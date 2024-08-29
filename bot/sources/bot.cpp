#include "bot.hpp"
#include <bsl/log.hpp>
#include <bsl/file.hpp>
#include "http.hpp"
#include "driver.hpp"
#include "dream.hpp"

#undef SendMessage

DEFINE_LOG_CATEGORY(DreamBot)

static const char *Svet = (const char*)u8"🟢Свет есть!🟢";
static const char *NoSvet = (const char*)u8"🔴Свет закончился...🔴";

static const char *SvetNotify = (const char*)u8"🟢Свет!🟢";
static const char *NoSvetNotify = (const char*)u8"🔴No Свет?🔴";

DreamBot::DreamBot(const std::string &token, const std::string &server_endpoint):
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

void DreamBot::Tick() {
	try{
		std::vector<LightNotify> notifications = DreamServer::Get().CollectNotifies();

		for(const auto &notify: notifications)
			Broadcast(notify);
	} catch (const std::exception& e) {
		LogDreamBot(Error, "%", e.what());
	}
}

void DreamBot::Broadcast(const LightNotify &notify) {
	for(auto chat: m_Chats){
		SendMessage(chat, 0, notify.Change == LightChange::Up ? SvetNotify : NoSvetNotify);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

void DreamBot::OnStatus(TgBot::Message::Ptr message) {
	auto status = DreamServer::Get().LightStatus();

	if(!status.has_value())
		return (void)ReplyMessage(message, "Light status unknown, raspberry service is down");

	ReplyMessage(message, status.value() ? Svet : NoSvet);
}

void DreamBot::OnEnable(TgBot::Message::Ptr message) {
	if(std::count(m_Chats.begin(), m_Chats.end(), message->chat->id))
		return (void)ReplyMessage(message, (const char*)u8"Дурак? делу не поможет");

	m_Chats.push_back(message->chat->id);
	WriteEntireFile(ChatsFile, nlohmann::json(m_Chats).dump());

	return (void)ReplyMessage(message, (const char*)u8"Subscribed to light events!");
}

void DreamBot::OnDisable(TgBot::Message::Ptr message) {
	auto it = std::find(m_Chats.begin(), m_Chats.end(), message->chat->id);

	if(it == m_Chats.end())
		return (void)ReplyMessage(message, (const char*)u8"Ты за кого играешь?");
	
	std::swap(*it, m_Chats.back());
	m_Chats.pop_back();

	WriteEntireFile(ChatsFile, nlohmann::json(m_Chats).dump());

	return (void)ReplyMessage(message, (const char*)u8"Unsubscribed from light events!");
}

void DreamBot::OnDriverDisconnect(TgBot::Message::Ptr message) {
	if(message->from->username != "BloodRedTape")
		return;
	
	DreamServer::Get().SetDriverPresent(false);

	ReplyMessage(message, "Driver disconnected");
}

void DreamBot::OnDriverConnect(TgBot::Message::Ptr message) {
	if(message->from->username != "BloodRedTape")
		return;

	DreamServer::Get().SetDriverPresent(true);

	ReplyMessage(message, "Driver connected");
}

void DreamBot::OnDriverStatus(TgBot::Message::Ptr message) {
	if(message->from->username != "BloodRedTape")
		return;

	ReplyMessage(message, DreamServer::Get().IsDriverPresent() ? "Driver connected" : "Driver disconnected");
}