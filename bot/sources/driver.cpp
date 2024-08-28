#include "driver.hpp"
#include <bsl/log.hpp>

DEFINE_LOG_CATEGORY(DriverServer)

DriverServer::DriverServer(){
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
		m_DriverPresent = true;
		resp.status = 200;
	});
	Post("/driver/disconnect", [&](const httplib::Request& req, httplib::Response& resp) {
		m_DriverPresent = false;
		resp.status = 200;
	});

	Post("/driver/tick", [&](const httplib::Request& req, httplib::Response& resp) {
		auto old_status = m_LastLightStatus;
		m_LastLightStatus = LightStatus();

		LogDriverServer(Display, "LightStatus: %", m_LastLightStatus.has_value() ? Format("(%)", m_LastLightStatus.value()) : "()");
	
		if(old_status.has_value() && m_LastLightStatus.has_value() 
		&& old_status.value() != m_LastLightStatus.value()){ 
			LogDriverServer(Info, "Notify generated");
			m_LightNotifies.push_back({m_LastLightStatus.value() ? LightChange::Up : LightChange::Down, 0});
		}

		resp.status = 200;
	});
}

bool DriverServer::IsLightPresent()const {
	return (std::chrono::steady_clock::now() - m_LastUpdate.load()) < NoPingFor;
}

bool DriverServer::IsDriverPresent()const {
	return m_DriverPresent;
}

std::optional<bool> DriverServer::LightStatus()const {
	if(!IsDriverPresent())
		return std::nullopt;
	
	return {IsLightPresent()};
}

std::vector<LightNotify> DriverServer::CollectNotifies() {
	std::vector<LightNotify> notifies;

	{
		std::scoped_lock lock(m_NotifyMutex);
		notifies = std::move(m_LightNotifies);
	}

	return notifies;
}

DriverServer& DriverServer::Get() {
	static DriverServer s_Server;

	return s_Server;
}