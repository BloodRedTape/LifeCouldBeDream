﻿#define CPPHTTPLIB_OPENSSL_SUPPORT
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

enum class LightChange {
	None,
	Up,
	Down
};

struct LightNotify {
	LightChange Change;	
	std::int64_t UnixTime = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(LightNotify, Change, UnixTime)
};

class DreamBot: public SimpleBot{
	std::optional<bool> m_LastLightStatus;
	std::vector<std::int64_t> m_Chats;
	const char *ChatsFile = "chats.json";

	const char *DriverEndpoint = "http://driver.dream.bloodredtape.com";
	const char *ServerEndpoint = "http://dream.bloodredtape.com";
public:
	DreamBot(const std::string &token):
		SimpleBot(token)
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
		std::vector<LightNotify> notifications = HttpGetJson(ServerEndpoint, "/light/notifications");

		for(const auto &notify: notifications)
			Broadcast(notify);
	}

	void Broadcast(const LightNotify &notify) {
		for(auto chat: m_Chats){
			SendMessage(chat, 0, notify.Change == LightChange::Up ? SvetNotify : NoSvetNotify);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void OnStatus(TgBot::Message::Ptr message);

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

		auto status = HttpPostStatus(DriverEndpoint, "/driver/disconnect");

		ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
	}

	void OnDriverConnect(TgBot::Message::Ptr message) {
		if(message->from->username != "BloodRedTape")
			return;

		auto status = HttpPostStatus(DriverEndpoint, "/driver/connect");

		ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
	}

	void OnDriverStatus(TgBot::Message::Ptr message) {
		if(message->from->username != "BloodRedTape")
			return;

		auto status = HttpGetStatus(DriverEndpoint, "/driver/status");

		ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
	}
};

class DriverServer: public httplib::Server{
	volatile std::atomic<std::chrono::steady_clock::time_point> m_LastUpdate = std::chrono::steady_clock::now();
	volatile std::atomic<bool> m_DriverPresent;
public:
	DriverServer(){
		m_DriverPresent.store(false);

		Post("/driver/light/update", [&](const httplib::Request& req, httplib::Response& resp) {
			m_LastUpdate = std::chrono::steady_clock::now();

			resp.status = 200;
		});

		httplib::Server::Get("/driver/status", [&](const httplib::Request& req, httplib::Response& resp) {
			resp.status = IsDriverPresent() 
				? httplib::StatusCode::OK_200 
				: httplib::StatusCode::Gone_410;
		});

		Post("/driver/connect", [&](const httplib::Request& req, httplib::Response& resp) {
			m_LastUpdate = std::chrono::steady_clock::now();
			m_DriverPresent.store(true);
			resp.status = 200;
		});
		Post("/driver/disconnect", [&](const httplib::Request& req, httplib::Response& resp) {
			m_DriverPresent.store(false);
			resp.status = 200;
		});
	}

	bool IsLightPresent()const {
		return (std::chrono::steady_clock::now() - m_LastUpdate.load()) < std::chrono::seconds(8);
	}

	bool IsDriverPresent()const {
		return m_DriverPresent.load();
	}

	std::optional<bool> LightStatus()const {
		if(!IsDriverPresent())
			return std::nullopt;
		
		return {IsLightPresent()};
	}

	static DriverServer& Get() {
		static DriverServer s_Server;

		return s_Server;
	}
};

class DreamServer: public httplib::Server{
	std::vector<LightNotify> m_LightNotifies;
	std::optional<bool> m_LastLightStatus;
public:

	DreamServer() {

		Get ("/light/status", [&](const httplib::Request& req, httplib::Response& resp) {
			if (!DriverServer::Get().IsDriverPresent()) {
				resp.status = httplib::StatusCode::NotFound_404;
				return;
			}

			resp.status = DriverServer::Get().IsLightPresent() 
				? httplib::StatusCode::OK_200 
				: httplib::StatusCode::Gone_410;
		});

		Get ("/light/notifications", [&](const httplib::Request& req, httplib::Response& resp) {
			resp.status = 200;
			resp.set_content(nlohmann::json(m_LightNotifies).dump(), "application/json");
			m_LightNotifies.clear();
		});

		Post("/timer/tick", [&](const httplib::Request& req, httplib::Response& resp) {
			auto old_status = m_LastLightStatus;
			m_LastLightStatus = DriverServer::Get().LightStatus();
		
			if(old_status.has_value() && m_LastLightStatus.has_value() 
			&& old_status.value() != m_LastLightStatus.value())
				m_LightNotifies.push_back({m_LastLightStatus.value() ? LightChange::Up : LightChange::Down});

			resp.status = 200;
		});
	}


};

void DreamBot::OnStatus(TgBot::Message::Ptr message) {
	if(!DriverServer::Get().IsDriverPresent())
		return (void)ReplyMessage(message, "Unknown, raspberry is down");

	ReplyMessage(message, DriverServer::Get().IsLightPresent() ? Svet : NoSvet);
}

int main() {
	INIReader config("BotConfig.ini");

	if (config.ParseError()) {
		Println("Failed to parse condig");
		return EXIT_FAILURE;
	}

	std::string hostname = config.Get("Server", "Host", "localhost");
	int port = config.GetInteger("Server", "Port", 1488);

	std::string driver_hostname = config.Get("Driver", "Host", "localhost");
	int driver_port = config.GetInteger("Driver", "Port", 44188);

	std::string token = config.Get("Bot", "Token", "");

	std::string endpoint = Format("http://%:%", hostname, port);

	std::thread(
		[](std::string host, int port){
			DreamServer server;
			bool status = server.listen(host, port);

			Println("Status: %", status);
		},
		hostname,
		port
	).detach();

	std::thread(
		[](std::string host, int port){
			DriverServer server;
			bool status = server.listen(host, port);

			Println("Status: %", status);
		},
		driver_hostname,
		driver_port
	).detach();

	std::thread([](){
		httplib::Client client("http://dream.bloodredtape.com");

		for (;;) {
			client.Post("/timer/tick");

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}).detach();
	
	DreamBot bot(token);


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