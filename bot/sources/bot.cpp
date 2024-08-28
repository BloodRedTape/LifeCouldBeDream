#include "bot.hpp"
#include <bsl/log.hpp>
#include <bsl/file.hpp>
#include "http.hpp"
#include "driver.hpp"

#undef SendMessage

DEFINE_LOG_CATEGORY(DreamBot)

static const char *Svet = (const char*)u8"🟢Свет есть!🟢";
static const char *NoSvet = (const char*)u8"🔴Свет закончился...🔴";

static const char *SvetNotify = (const char*)u8"🟢Свет!🟢";
static const char *NoSvetNotify = (const char*)u8"🔴No Свет?🔴";

DreamBot::DreamBot(const std::string &token, const std::string &server_endpoint, const std::string &driver_endpoint):
	SimpleBot(token),
	m_ServerEndpoint(server_endpoint),
	m_DriverEndpoint(driver_endpoint)
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
		std::vector<LightNotify> notifications = HttpGetJson(m_ServerEndpoint, "/light/notifications");

		for(const auto &notify: notifications)
			Broadcast(notify);
	} catch (const std::exception& e) {
		Println("%", e.what());
	}
}

void DreamBot::Broadcast(const LightNotify &notify) {
	for(auto chat: m_Chats){
		SendMessage(chat, 0, notify.Change == LightChange::Up ? SvetNotify : NoSvetNotify);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

void DreamBot::OnStatus(TgBot::Message::Ptr message) {
	if(!DriverServer::Get().IsDriverPresent())
		return (void)ReplyMessage(message, "Unknown, raspberry is down");

	ReplyMessage(message, DriverServer::Get().IsLightPresent() ? Svet : NoSvet);
}

void DreamBot::OnEnable(TgBot::Message::Ptr message) {
	if(std::count(m_Chats.begin(), m_Chats.end(), message->chat->id))
		return;

	m_Chats.push_back(message->chat->id);
	WriteEntireFile(ChatsFile, nlohmann::json(m_Chats).dump());
}

void DreamBot::OnDisable(TgBot::Message::Ptr message) {
	ReplyMessage(message, "Hehe, undisableable");
}

void DreamBot::OnDriverDisconnect(TgBot::Message::Ptr message) {
	if(message->from->username != "BloodRedTape")
		return;

	auto status = HttpPostStatus(m_DriverEndpoint, "/driver/disconnect");

	ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
}

void DreamBot::OnDriverConnect(TgBot::Message::Ptr message) {
	if(message->from->username != "BloodRedTape")
		return;

	auto status = HttpPostStatus(m_DriverEndpoint, "/driver/connect");

	ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
}

void DreamBot::OnDriverStatus(TgBot::Message::Ptr message) {
	if(message->from->username != "BloodRedTape")
		return;

	auto status = HttpGetStatus(m_DriverEndpoint, "/driver/status");

	ReplyMessage(message, status.has_value() ? Format("Status: %", (int)status.value()) : "Failed");
}