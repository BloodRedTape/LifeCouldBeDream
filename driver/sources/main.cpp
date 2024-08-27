#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <bsl/file.hpp>
#include <bsl/log.hpp>

int main() {
	httplib::Client http("http://dream.bloodredtape.com");

	auto res = http.Post("/driver/connect");

	auto Period = std::chrono::milliseconds(1000);

	for (;;) {
		auto res = http.Post("/light/status");
		
		if (!res || res->status != 200) {
			Println("Failed to update status, %", to_string(res.error()));
		}

		std::this_thread::sleep_for(Period);
	}
}