#pragma once

#include "http.hpp"
#include <atomic>
#include <optional>

class DriverServer: public httplib::Server{
	volatile std::atomic<std::chrono::steady_clock::time_point> m_LastUpdate = std::chrono::steady_clock::now();
	volatile std::atomic<bool> m_DriverPresent;
public:
	DriverServer();

	bool IsLightPresent()const;

	bool IsDriverPresent()const;

	std::optional<bool> LightStatus()const;

	static DriverServer& Get();
};
