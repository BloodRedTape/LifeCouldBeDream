#include "dream.hpp"
#include "driver.hpp"
#include <bsl/log.hpp>

DEFINE_LOG_CATEGORY(DreamServer)

std::string CodeToJson(int code) {
	return nlohmann::json({{"Status", std::to_string(code)}}).dump();
}

DreamServer::DreamServer() {
	Super::Get ("/light/status", [&](const httplib::Request& req, httplib::Response& resp) {
		if (!IsDriverPresent()) {
			resp.status = httplib::StatusCode::NotFound_404;
			return;
		}

		resp.status = DriverServer::Get().IsLightPresent() 
			? httplib::StatusCode::OK_200 
			: httplib::StatusCode::Gone_410;
		resp.set_content(CodeToJson(resp.status), "application/json");
	});

	Super::Get ("/light/notifications", [&](const httplib::Request& req, httplib::Response& resp) {
		resp.status = 200;
		resp.set_content(nlohmann::json(CollectNotifies()).dump(), "application/json");
	});

	Super::Post("/driver/connect", [&](const httplib::Request& req, httplib::Response& resp) {
		SetDriverPresent(true);
		resp.status = 200;
		resp.set_content(CodeToJson(resp.status), "application/json");
	});

	Super::Post("/driver/disconnect", [&](const httplib::Request& req, httplib::Response& resp) {
		SetDriverPresent(false);
		resp.status = 200;
		resp.set_content(CodeToJson(resp.status), "application/json");
	});

	Super::Get ("/driver/status", [&](const httplib::Request& req, httplib::Response& resp) {
		resp.status = IsDriverPresent() 
			? httplib::StatusCode::OK_200 
			: httplib::StatusCode::Gone_410;
		resp.set_content(CodeToJson(resp.status), "application/json");
	});
}

void DreamServer::SetDriverPresent(bool is){
	m_IsDriverPresent = is;
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
