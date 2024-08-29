#pragma once

#include "simple_bot.hpp"
#include "notify.hpp"
#include <optional>

class DreamBot: public SimpleBot{
	static constexpr const char *ChatsFile = "chats.json";
private:
	std::optional<bool> m_LastLightStatus;
	std::vector<std::int64_t> m_Chats;

	std::string m_ServerEndpoint;
public:
	DreamBot(const std::string &token, const std::string &server_endpoint);

	void Tick();

	void Broadcast(const LightNotify &notify);
	
	void OnStatus(TgBot::Message::Ptr message);

	void OnEnable(TgBot::Message::Ptr message);

	void OnDisable(TgBot::Message::Ptr message);

	void OnDriverDisconnect(TgBot::Message::Ptr message);

	void OnDriverConnect(TgBot::Message::Ptr message);

	void OnDriverStatus(TgBot::Message::Ptr message);
};
