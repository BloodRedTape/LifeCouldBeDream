#include "driver.hpp"
#include <bsl/log.hpp>

DEFINE_LOG_CATEGORY(DriverServer)

bool DriverServer::IsLightPresent()const {
	auto since_last_update = std::chrono::steady_clock::now() - m_LastUpdate.load();
	LogDriverServer(Display, "IsLightPresent: SinceLastUpdate: %s", (since_last_update.count()/(1000 * 1000))/1000.f);

	return since_last_update < NoPingFor;
}

void DriverServer::Run(int port){
    try {
        boost::asio::io_context io_context;
        boost::asio::ip::udp::socket socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));

        for(;;) {
            char buffer[1024];
            boost::asio::ip::udp::endpoint remote_endpoint;

            std::size_t length = socket.receive_from(boost::asio::buffer(buffer), remote_endpoint);

            m_LastUpdate = std::chrono::steady_clock::now();
            
            LogDriverServer(Display, "Got update");
        }
    } catch (std::exception& e) {
        LogDriverServer(Error, "Run failed with: %", e.what());
    }
}

DriverServer& DriverServer::Get() {
	static DriverServer s_Server;

	return s_Server;
}