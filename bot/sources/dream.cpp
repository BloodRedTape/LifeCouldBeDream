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
		resp.set_content(nlohmann::json(DriverServer::Get().CollectNotifies()).dump(), "application/json");
	});
}

