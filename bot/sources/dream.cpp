#include "dream.hpp"
#include "driver.hpp"
#include <bsl/log.hpp>

DEFINE_LOG_CATEGORY(DreamServer)

DreamServer::DreamServer() {
	Super::Get ("/light/status", [&](const httplib::Request& req, httplib::Response& resp) {
		LogDreamServer(Display, "Driver: %, Light: %", IsDriverPresent(), DriverServer::Get().IsLightPresent());

		if (!IsDriverPresent()) {
			resp.status = httplib::StatusCode::NotFound_404;
			return;
		}

		resp.status = DriverServer::Get().IsLightPresent() 
			? httplib::StatusCode::OK_200 
			: httplib::StatusCode::Gone_410;
	});

	Super::Get ("/light/notifications", [&](const httplib::Request& req, httplib::Response& resp) {
		resp.status = 200;
		resp.set_content(nlohmann::json(CollectNotifies()).dump(), "application/json");
	});

	Post("/driver/connect", [&](const httplib::Request& req, httplib::Response& resp) {
		m_IsDriverPresent = true;	
		resp.status = 200;
	});

	Post("/driver/disconnect", [&](const httplib::Request& req, httplib::Response& resp) {
		m_IsDriverPresent = false;	
		resp.status = 200;
	});

	Super::Get ("/driver/status", [&](const httplib::Request& req, httplib::Response& resp) {
		resp.status = IsDriverPresent() 
			? httplib::StatusCode::OK_200 
			: httplib::StatusCode::Gone_410;
	});


	Post("/timer/tick", [&](const httplib::Request& req, httplib::Response& resp) {
		auto old_status = m_LastLightStatus;
		m_LastLightStatus = LightStatus();

		LogDreamServer(Display, "/timer/tick: %", m_LastLightStatus.has_value() ? Format("(%)", m_LastLightStatus.value()) : "()");

		if (old_status.has_value() && m_LastLightStatus.has_value()
			&& old_status.value() != m_LastLightStatus.value()) {
			LogDreamServer(Info, "Notify generated");
			m_LightNotifies.push_back({ m_LastLightStatus.value() ? LightChange::Up : LightChange::Down, 0 });
		}

		resp.status = 200;
	});
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
	std::vector<LightNotify> result = std::move(m_LightNotifies);

	return result;
}

DreamServer& DreamServer::Get(){
	static DreamServer s_Server;

	return s_Server;
}
