#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <INIReader.h>
#include <bsl/file.hpp>
#include <bsl/log.hpp>

int main() {
	INIReader config("DriverConfig.ini");

	if (config.ParseError()) {
		Println("Failed to parse condig");
		return EXIT_FAILURE;
	}

	const char *Section = "Driver";
	
	httplib::Client http(
		config.Get(Section, "Host", ""),
		config.GetInteger(Section, "Port", 0)
	);

	int Period = config.GetInteger(Section, "Period", 1000);

	for (;;) {
		auto res = http.Post("/light_status");
		
		if (!res || res->status != 200) {
			Println("Failed to update status, %", to_string(res.error()));
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(Period));
	}
}