#include <bsl/file.hpp>
#include <bsl/log.hpp>
#include <chrono>
#include <boost/asio/ip/udp.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <optional>

DEFINE_LOG_CATEGORY(Driver)


boost::asio::io_context s_Context;

std::optional<boost::asio::ip::udp::endpoint> ResolveHostname(const std::string &hostname, int port) {
    using namespace boost::asio::ip;

    udp::resolver resolver(s_Context);
    udp::resolver::query query(udp::v4(), hostname, std::to_string(port));
    auto result = resolver.resolve(query);

    if(!result.size())
        return std::nullopt;
        
    return {*result.begin()};
}

bool LooksLikeDefaultGateway(const boost::asio::ip::udp::endpoint& endpoint) {
    auto str = endpoint.address().to_string();

    return boost::starts_with(str, "192.168");
}

bool IsValidResolution(std::optional<boost::asio::ip::udp::endpoint> endpoint) {
    if(!endpoint.has_value())
        return false;
    
    if(LooksLikeDefaultGateway(endpoint.value()))
        return false;

    return true;
}

int main() {
    using namespace boost::asio::ip;

    std::string hostname = "driver.dream.bloodredtape.com";
    int port = 44188;
    auto period = std::chrono::milliseconds(1000);

    udp::socket socket(s_Context);
    socket.open(udp::v4());

    std::optional<boost::asio::ip::udp::endpoint> endpoint;

    for (;;) {
        try {
            if(!IsValidResolution(endpoint))
                endpoint = ResolveHostname(hostname, port);

            if(!endpoint.has_value()){
                (LogDriver(Error, "Can't resolve %:%", hostname, port), EXIT_FAILURE);
                continue;
            }

            LogDriver(Display, "Resolved % to %", hostname, endpoint.value().address().to_string());
            std::string message;

            boost::system::error_code ec;
            socket.send_to(boost::asio::buffer(message), endpoint.value(), 0, ec);

            if (ec) {
                LogDriver(Error, "Failed to send packet: %", ec.message());
            } else {
                LogDriver(Display, "Packet sent successfully");
            }

        } catch (const std::exception& e) {
            LogDriver(Error, "Tick exception: %", e.what());
        }

        std::this_thread::sleep_for(period);
    }

    return 0;
}
