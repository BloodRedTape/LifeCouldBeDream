#pragma once

#include "http.hpp"
#include <optional>
#include <atomic>
#include <mutex>
#include "notify.hpp"

struct DreamState{
	bool IsDriverPresent = false;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(DreamState, IsDriverPresent)
};

class DreamServer: public httplib::Server{
	using Super = httplib::Server;

	const char *DreamStateFile = "dream_state.json";
private:
	std::atomic<bool> m_IsDriverPresent = false;
	std::optional<bool> m_LastLightStatus;

	std::mutex m_LightNotifiesLock;
	std::vector<LightNotify> m_LightNotifies;
private:
	DreamServer();
public:

	void SetDriverPresent(bool is);

	bool IsDriverPresent()const;

	std::optional<bool> LightStatus()const;
	
	std::vector<LightNotify> CollectNotifies();

	void Tick();

	static DreamServer &Get();
};