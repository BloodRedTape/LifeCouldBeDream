#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#undef SendMessage
#include <bsl/log.hpp>
#include <bsl/file.hpp>
#include <INIReader.h>
#include "simple_bot.hpp"
#include <tgbot/net/TgLongPoll.h>
#include "http.hpp"

const char *Section = "Bot";

static const char *Svet = (const char*)u8"🟢Свет есть!🟢";
static const char *NoSvet = (const char*)u8"🔴Свет закончился...🔴";

static const char *SvetNotify = (const char*)u8"🟢Свет!🟢";
static const char *NoSvetNotify = (const char*)u8"🔴No Свет?🔴";

class DreamBot: public SimpleBot{
	std::optional<bool> m_LastLightStatus;
	std::vector<std::int64_t> m_Chats;
	const char *ChatsFile = "chats.json";

	std::string m_ServerEndpoint;


public:
	DreamBot(const std::string &token, const std::string &server_endpoint):
		SimpleBot(token),
		m_ServerEndpoint(server_endpoint)
	{
		OnCommand("light_status", this, &DreamBot::OnStatus);
		OnCommand("light_enable", this, &DreamBot::OnEnable);
		OnCommand("light_disable", this, &DreamBot::OnDisable);
		OnCommand("driver_disconnect", this, &DreamBot::OnDriverDisconnect);
		OnCommand("driver_connect", this, &DreamBot::OnDriverConnect);
		OnCommand("driver_status", this, &DreamBot::OnDriverStatus);

		std::string content = ReadEntireFile(ChatsFile);
		
		try{
			m_Chats = nlohmann::json::parse(content, nullptr, false).get<decltype(m_Chats)>();
		} catch (...) {

		}
	}

	void Tick() {
		auto old_status = m_LastLightStatus;
		m_LastLightStatus = IsLightPresent();
		
		if(old_status.has_value())
			Println("OldStatus: (%)", old_status.value());
		else
			Println("OldStatus: ()");

		if(m_LastLightStatus.has_value())
			Println("LastStatus: (%)", m_LastLightStatus.value());
		else
			Println("LastStatus: ()");


		if(old_status.has_value() && m_LastLightStatus.has_value() 
		&& old_status.value() != m_LastLightStatus.value())
			OnUpdate();
	}

	void OnUpdate() {
		if(!m_LastLightStatus.has_value())
			return;

		for(auto chat: m_Chats){
			SendMessage(chat, 0, m_LastLightStatus.value() ? SvetNotify : NoSvetNotify);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void OnStatus(TgBot::Message::Ptr message) {
		ReplyMessage(message, IsLightPresent() ? Svet : NoSvet);
	}
	void OnEnable(TgBot::Message::Ptr message) {
		if(std::count(m_Chats.begin(), m_Chats.end(), message->chat->id))
			return;

		m_Chats.push_back(message->chat->id);
		WriteEntireFile(ChatsFile, nlohmann::json(m_Chats).dump());
	}

	void OnDisable(TgBot::Message::Ptr message) {
		ReplyMessage(message, "Hehe, undisableable");
	}

	void OnDriverDisconnect(TgBot::Message::Ptr message) {
		if(message->from->username != "BloodRedTape")
			return;

		auto status = HttpPostStatus(m_ServerEndpoint, "/driver/disconnect");

		ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
	}

	void OnDriverConnect(TgBot::Message::Ptr message) {
		if(message->from->username != "BloodRedTape")
			return;

		auto status = HttpPostStatus(m_ServerEndpoint, "/driver/connect");

		ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
	}

	void OnDriverStatus(TgBot::Message::Ptr message) {
		if(message->from->username != "BloodRedTape")
			return;

		auto status = HttpGetStatus(m_ServerEndpoint, "/driver/status");

		ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
	}

	std::optional<bool> IsLightPresent()const {
		auto status = HttpGetStatus(m_ServerEndpoint, "/light/status");

		if(!status.has_value())
			return std::nullopt;

		if(status.value() == httplib::OK_200)
			return true;
		if(status.value() == httplib::Gone_410)
			return false;

		return std::nullopt;
	}
};


class DreamServer: public httplib::Server{
	std::chrono::steady_clock::time_point m_LastUpdate = std::chrono::steady_clock::now();
	bool m_DriverPresent = false;
public:

	DreamServer() {
		Post("/light/status", [&](const httplib::Request& req, httplib::Response& resp) {
			m_LastUpdate = std::chrono::steady_clock::now();

			resp.status = 200;
		});

		Get ("/light/status", [&](const httplib::Request& req, httplib::Response& resp) {
			if (!IsDriverPresent()) {
				resp.status = httplib::StatusCode::NotFound_404;
				return;
			}

			resp.status = IsLightPresent() 
				? httplib::StatusCode::OK_200 
				: httplib::StatusCode::Gone_410;
		});

		Get ("/driver/status", [&](const httplib::Request& req, httplib::Response& resp) {
			resp.status = IsDriverPresent() 
				? httplib::StatusCode::OK_200 
				: httplib::StatusCode::Gone_410;
		});

		Post("/driver/connect", [&](const httplib::Request& req, httplib::Response& resp) {
			m_LastUpdate = std::chrono::steady_clock::now();
			m_DriverPresent = true;
			resp.status = 200;
		});
		Post("/driver/disconnect", [&](const httplib::Request& req, httplib::Response& resp) {
			m_DriverPresent = false;
			resp.status = 200;
		});
	}

	bool IsLightPresent()const {
		return (std::chrono::steady_clock::now() - m_LastUpdate) < std::chrono::seconds(3);
	}

	bool IsDriverPresent()const {
		return m_DriverPresent;
	}
};

int main() {
	INIReader config("BotConfig.ini");

	if (config.ParseError()) {
		Println("Failed to parse condig");
		return EXIT_FAILURE;
	}

	std::string hostname = config.Get("Server", "Host", "localhost");
	int port = config.GetInteger("Server", "Port", 1488);
	std::string token = config.Get("Bot", "Token", "");

	std::string endpoint = Format("http://%:%", hostname, port);

	std::thread server_thread(
		[](std::string host, int port){
			DreamServer server;
			bool status = server.listen(host, port);

			Println("Status: %", status);
		},
		hostname,
		port
	);
	
	DreamBot bot(token, endpoint);


	TgBot::TgLongPoll poll(bot, 100, 1);
	while (true) {
		try{
			poll.start();
			bot.Tick();
		} catch (const std::exception &e) {
			Println("%", e.what());
		}
	}
}