#include "dream.hpp"
#include "driver.hpp"
#include <bsl/log.hpp>

DEFINE_LOG_CATEGORY(DreamServer)

DreamServer::DreamServer() {
	Get ("/light/status", [&](const httplib::Request& req, httplib::Response& resp) {
		LogDreamServer(Display, "Driver: %, Light: %", DriverServer::Get().IsDriverPresent(), DriverServer::Get().IsLightPresent());

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

		LogDreamServer(Display, "LightStatus: %", m_LastLightStatus.has_value() ? Format("(%)", m_LastLightStatus.value()) : "()");
	
		if(old_status.has_value() && m_LastLightStatus.has_value() 
		&& old_status.value() != m_LastLightStatus.value())
			m_LightNotifies.push_back({m_LastLightStatus.value() ? LightChange::Up : LightChange::Down, 0});

		resp.status = 200;
	});
}

