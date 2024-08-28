#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <bsl/file.hpp>
#include <bsl/log.hpp>
#include <chrono>

auto CurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_time_t);
    
    return std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
}

DEFINE_LOG_CATEGORY(Driver)

int main() {
	httplib::Client http("http://driver.dream.bloodredtape.com");

	auto Period = std::chrono::milliseconds(1000);

	for (;;) {
		auto res = http.Post("/driver/light/update");
		
		if (!res || res->status != 200) {
			LogDriver(Error, "[%]: Failed to update status, %", CurrentTime(), to_string(res.error()));
		} else {
			LogDriver(Display, "[%]: Updated status", CurrentTime());
		}

		std::this_thread::sleep_for(Period);
	}
}