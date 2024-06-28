#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#undef SendMessage
#include <bsl/log.hpp>
#include <bsl/file.hpp>
#include <INIReader.h>
#include "simple_bot.hpp"
#include <tgbot/net/TgLongPoll.h>
#include <nlohmann/json.hpp>


const char *Section = "Bot";
static std::chrono::steady_clock::time_point s_LastUpdate = std::chrono::steady_clock::now();
static httplib::Server s_Server;

class DreamBot: public SimpleBot{
	bool m_LastLightStatus = true;
	std::vector<std::int64_t> m_Chats;
	const char *ChatsFile = "chats.json";
public:
	DreamBot(const std::string &token):
		SimpleBot(token)
	{
		OnCommand("light_status", this, &DreamBot::OnStatus);
		OnCommand("light_enable", this, &DreamBot::OnEnable);
		OnCommand("light_disable", this, &DreamBot::OnDisable);

		std::string content = ReadEntireFile(ChatsFile);
		
		try{
			m_Chats = nlohmann::json::parse(content, nullptr, false).get<decltype(m_Chats)>();
		} catch (...) {

		}
	}

	void Tick() {
		auto old_status = m_LastLightStatus;
		m_LastLightStatus = IsLightPresent();

		if(old_status != m_LastLightStatus)
			OnUpdate();
	}

	void OnUpdate() {
		for(auto chat: m_Chats){
			SendMessage(chat, 0, m_LastLightStatus ? "Svet?" : "No Svet?");
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void OnStatus(TgBot::Message::Ptr message) {
		ReplyMessage(message, IsLightPresent() ? "Svet." : "No Svet.");
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

	bool IsLightPresent()const {
		return (std::chrono::steady_clock::now() - s_LastUpdate) < std::chrono::seconds(3);
	}
};

int main() {
	INIReader config("BotConfig.ini");

	if (config.ParseError()) {
		Println("Failed to parse condig");
		return EXIT_FAILURE;
	}


	s_Server.Post("/light_status", [](const httplib::Request& req, httplib::Response& resp) {
		Println("Updated");
		s_LastUpdate = std::chrono::steady_clock::now();
	});
	
	std::thread server_thread(
		[](std::string host, int port){
			bool status = s_Server.listen(host, port);

			Println("Status: %", status);
		},
		config.Get(Section, "Host", "localhost"),
		config.GetInteger(Section, "Post", 1488)
	);
	
	DreamBot bot(
		config.Get(Section, "Token", "")
	);


	TgBot::TgLongPoll poll(bot, 100, 2);
	while (true) {
		poll.start();
		bot.Tick();
	}
}