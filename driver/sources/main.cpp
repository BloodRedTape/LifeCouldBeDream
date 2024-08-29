#include <bsl/file.hpp>
#include <bsl/log.hpp>
#include <chrono>
#include <boost/asio/ip/udp.hpp>

DEFINE_LOG_CATEGORY(Driver)


boost::asio::io_context s_Context;

std::optional<boost::asio::ip::udp::endpoint> GetEndpoint(const std::string &hostname, int port) {
    using namespace boost::asio::ip;

    udp::resolver resolver(s_Context);
    udp::resolver::query query(udp::v4(), hostname, std::to_string(port));
    auto result = resolver.resolve(query);

    if(!result.size())
        return std::nullopt;
        
    return {*result.begin()};
}

int main() {
    using namespace boost::asio::ip;

    std::string hostname = "driver.dream.bloodredtape.com";
    int port = 44188;
    auto period = std::chrono::milliseconds(1000);


    auto endpoint = GetEndpoint(hostname, port);

    if(!endpoint.has_value())
        return (LogDriver(Error, "Can't resolve %:%", hostname, port), EXIT_FAILURE);

    LogDriver(Display, "Resolved % to %", hostname, endpoint.value().address().to_string());

    udp::socket socket(s_Context);
    socket.open(udp::v4());

    for (;;) {
        std::string message;

        boost::system::error_code ec;
        socket.send_to(boost::asio::buffer(message), endpoint.value(), 0, ec);

        if (ec) {
            LogDriver(Error, "Failed to send packet: %", ec.message());
        } else {
            LogDriver(Display, "Packet sent successfully");
        }

        std::this_thread::sleep_for(period);
    }

    return 0;
}
