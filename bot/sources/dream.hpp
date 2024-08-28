#pragma once

#include "http.hpp"
#include <optional>
#include "notify.hpp"

class DreamServer: public httplib::Server{
public:
	DreamServer();
};