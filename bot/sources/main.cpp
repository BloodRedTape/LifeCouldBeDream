#include <bsl/log.hpp>
#include <bsl/file.hpp>
#include <INIReader.h>
#include <tgbot/net/TgLongPoll.h>
#include "driver.hpp"
#include "dream.hpp"
#include "bot.hpp"

std::string HttpEndpoint(const std::string& hostname, int port) {
	return Format("http://%:%", hostname, port);
}

DEFINE_LOG_CATEGORY(Timer)

int main() {
	INIReader config("BotConfig.ini");

	if (config.ParseError()) {
		Println("Failed to parse condig");
		return EXIT_FAILURE;
	}

	std::string server_hostname = config.Get("Server", "Host", "localhost");
	int server_port = config.GetInteger("Server", "Port", 1488);

	std::string driver_hostname = config.Get("Driver", "Host", "localhost");
	int driver_port = config.GetInteger("Driver", "Port", 44188);

	std::string token = config.Get("Bot", "Token", "");

	std::string server_endpoint = HttpEndpoint(server_hostname, server_port);
	std::string driver_endpoint = HttpEndpoint(driver_hostname, driver_port);

	std::thread([server_hostname, server_port](){
		DreamServer::Get().listen(server_hostname, server_port);
	}).detach();

	std::thread([driver_port](){
		DriverServer::Get().Run(driver_port);
	}).detach();

	std::thread([server_endpoint](){
		auto period = std::chrono::milliseconds(1111);

		for (;;) {
			LogTimer(Display, "Tick, period: %", period.count());

			HttpPost(server_endpoint, "/timer/tick");

			std::this_thread::sleep_for(period);
		}
	}).detach();
	
	DreamBot bot(token, server_endpoint);

	TgBot::TgLongPoll poll(bot, 100, 1);

	while (true) {
		try{
			poll.start();
			bot.Tick();
		} catch (const std::exception &e) {
			Println("%", e.what());
		}
	}
}