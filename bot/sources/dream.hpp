#pragma once

#include "http.hpp"
#include <optional>
#include <atomic>
#include "notify.hpp"

class DreamServer: public httplib::Server{
	using Super = httplib::Server;
private:
	std::atomic<bool> m_IsDriverPresent = false;
	std::optional<bool> m_LastLightStatus;
	std::vector<LightNotify> m_LightNotifies;
private:
	DreamServer();
public:

	bool IsDriverPresent()const;

	std::optional<bool> LightStatus()const;
	
	std::vector<LightNotify> CollectNotifies();

	static DreamServer &Get();
};