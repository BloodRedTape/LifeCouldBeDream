#include "dream.hpp"
#include "driver.hpp"
#include <bsl/log.hpp>
#include <bsl/file.hpp>

DEFINE_LOG_CATEGORY(DreamServer)

std::string StringJson(const std::string &str) {
	return nlohmann::json(str).dump();
}

DreamServer::DreamServer() {
	
	try{
		DreamState state = nlohmann::json::parse(ReadEntireFile(DreamStateFile), nullptr, false, false);
		m_IsDriverPresent = state.IsDriverPresent;
	}catch (const std::exception& e) {
		LogDreamServer(Error, "%", e.what());
	}


	Super::Get ("/light", [&](const httplib::Request& req, httplib::Response& resp) {
		auto status = LightStatus();

		auto status_string = status.has_value() 
			? (status.value() 
				? "On"
				: "Off")
			: "Unknown";

		auto json = nlohmann::json({{"Status", status_string}});

		resp.status = httplib::StatusCode::OK_200;
		resp.set_content(json.dump(), "application/json");
	});

	Super::Get ("/light/notifications", [&](const httplib::Request& req, httplib::Response& resp) {
		resp.status = 200;
		resp.set_content(nlohmann::json(CollectNotifies()).dump(), "application/json");
	});

	Super::Post("/driver/connect", [&](const httplib::Request& req, httplib::Response& resp) {
		SetDriverPresent(true);
		resp.status = 200;
		resp.set_content(StringJson("Done"), "application/json");
	});

	Super::Post("/driver/disconnect", [&](const httplib::Request& req, httplib::Response& resp) {
		SetDriverPresent(false);
		resp.status = 200;
		resp.set_content(StringJson("Done"), "application/json");
	});

	Super::Get ("/driver", [&](const httplib::Request& req, httplib::Response& resp) {
		resp.status = httplib::StatusCode::OK_200;

		auto json = nlohmann::json({{"Status", IsDriverPresent() ? "Connected" : "Disconnected"}});

		resp.set_content(json.dump(), "application/json");
	});
}

void DreamServer::SetDriverPresent(bool is){
	m_IsDriverPresent = is;

	DreamState state;
	state.IsDriverPresent = m_IsDriverPresent;
	
	WriteEntireFile(DreamStateFile, nlohmann::json(state).dump());
}

bool DreamServer::IsDriverPresent() const{
	return m_IsDriverPresent;
}

std::optional<bool> DreamServer::LightStatus()const {
	if(!IsDriverPresent())
		return std::nullopt;
	
	return {DriverServer::Get().IsLightPresent()};
}

std::vector<LightNotify> DreamServer::CollectNotifies(){
	std::vector<LightNotify> result;

	{
		std::scoped_lock lock(m_LightNotifiesLock);

		result = std::move(m_LightNotifies);
	}

	return result;
}

void DreamServer::Tick(){
	auto old_status = m_LastLightStatus;
	m_LastLightStatus = LightStatus();

	if (old_status.has_value() && m_LastLightStatus.has_value() && old_status.value() != m_LastLightStatus.value()) {
		LogDreamServer(Info, "Notify generated");

		std::scoped_lock lock(m_LightNotifiesLock);
		m_LightNotifies.push_back({ m_LastLightStatus.value() ? LightChange::Up : LightChange::Down, 0 });
	}
}

DreamServer& DreamServer::Get(){
	static DreamServer s_Server;

	return s_Server;
}
