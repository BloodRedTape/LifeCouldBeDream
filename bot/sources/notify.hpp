#pragma once

#include <nlohmann/json.hpp>

enum class LightChange {
	None,
	Up,
	Down
};

struct LightNotify {
	LightChange Change = LightChange::None;	
	std::int64_t UnixTime = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(LightNotify, Change, UnixTime)
};