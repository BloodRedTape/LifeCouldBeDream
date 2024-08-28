#pragma once

#include "http.hpp"
#include "notify.hpp"
#include <atomic>
#include <mutex>
#include <optional>

class DriverServer: public httplib::Server{
	const std::chrono::seconds NoPingFor = std::chrono::seconds(5);
private:
	std::atomic<std::chrono::steady_clock::time_point> m_LastUpdate = std::chrono::steady_clock::now();
	std::atomic<bool> m_DriverPresent = false;

	std::optional<bool> m_LastLightStatus;
	std::mutex m_NotifyMutex;
	std::vector<LightNotify> m_LightNotifies;
private:
	DriverServer();
public:

	bool IsLightPresent()const;

	bool IsDriverPresent()const;

	std::optional<bool> LightStatus()const;

	std::vector<LightNotify> CollectNotifies();

	static DriverServer& Get();
};
