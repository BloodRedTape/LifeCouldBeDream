#pragma once

#include "http.hpp"
#include <optional>
#include "notify.hpp"

class DreamServer: public httplib::Server{
	std::vector<LightNotify> m_LightNotifies;
	std::optional<bool> m_LastLightStatus;
public:
	DreamServer();
};