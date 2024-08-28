#pragma once

#include "http.hpp"
#include <atomic>
#include <optional>

class DriverServer: public httplib::Server{
	const std::chrono::seconds NoPingFor = std::chrono::seconds(5);
private:
	std::atomic<std::chrono::steady_clock::time_point> m_LastUpdate = std::chrono::steady_clock::now();
	std::atomic<bool> m_DriverPresent = false;
private:
	DriverServer();
public:

	bool IsLightPresent()const;

	bool IsDriverPresent()const;

	std::optional<bool> LightStatus()const;

	static DriverServer& Get();
};
