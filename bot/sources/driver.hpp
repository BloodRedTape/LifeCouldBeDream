#pragma once

#include "http.hpp"
#include "notify.hpp"
#include <atomic>
#include <mutex>
#include <optional>
#include <boost/asio/ip/udp.hpp>

class DriverServer{
	const std::chrono::seconds NoPingFor = std::chrono::seconds(5);
private:
	std::atomic<std::chrono::steady_clock::time_point> m_LastUpdate = std::chrono::steady_clock::now();
private:
	DriverServer() = default;
public:
	bool IsLightPresent()const;

	void Run(int port);

	static DriverServer& Get();
};
