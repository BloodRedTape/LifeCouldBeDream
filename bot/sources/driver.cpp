#include "driver.hpp"
#include <bsl/log.hpp>

DEFINE_LOG_CATEGORY(DriverServer)

DriverServer::DriverServer(){
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

bool DriverServer::IsLightPresent()const {
	return (std::chrono::steady_clock::now() - m_LastUpdate.load()) < std::chrono::seconds(8);
}

bool DriverServer::IsDriverPresent()const {
	return m_DriverPresent.load();
}

std::optional<bool> DriverServer::LightStatus()const {
	if(!IsDriverPresent())
		return std::nullopt;
	
	return {IsLightPresent()};
}

DriverServer& DriverServer::Get() {
	static DriverServer s_Server;

	return s_Server;
}