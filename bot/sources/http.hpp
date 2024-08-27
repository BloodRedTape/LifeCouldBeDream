#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT 1
#include <httplib.h>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <bsl/log.hpp>

DEFINE_LOG_CATEGORY(Http)

inline httplib::Client MakeSecureClient(const std::string& endpoint) {
	return httplib::Client(endpoint);
}

inline std::optional<std::string> HttpGet(const std::string& endpoint, const std::string &path, httplib::Headers headers = {}) {
    httplib::Client client = MakeSecureClient(endpoint);

	auto resp = client.Get(path, headers);

	if (!resp || resp.error() != httplib::Error::Success){
		LogHttp(Error, "GET Request to % failed with internal error %", endpoint + path, httplib::to_string(resp.error()));
		return std::nullopt;
	}

	if (resp->status != httplib::StatusCode::OK_200){
		LogHttp(Error, "GET Request to % failed with http status %", endpoint + path, resp->status);
		return std::nullopt;
	}

	return resp->body;
}

inline std::optional<httplib::StatusCode> HttpGetStatus(const std::string& endpoint, const std::string &path, httplib::Headers headers = {}) {
    httplib::Client client = MakeSecureClient(endpoint);

	auto resp = client.Get(path, headers);

	if (!resp || resp.error() != httplib::Error::Success){
		LogHttp(Error, "GET Request to % failed with internal error %", endpoint + path, httplib::to_string(resp.error()));
		return std::nullopt;
	}

	if (resp->status != httplib::StatusCode::OK_200){
		LogHttp(Error, "GET Request to % failed with http status %", endpoint + path, resp->status);
		return std::nullopt;
	}

	return {(httplib::StatusCode)resp->status};
}


inline nlohmann::json HttpGetJson(const std::string& endpoint, const std::string &path, httplib::Headers headers = {}) {
	auto res = HttpGet(endpoint, path, headers);

	if(!res.has_value())
		return {};

	return nlohmann::json::parse(res.value(), nullptr, false, false);
}

inline std::optional<std::string> HttpPost(const std::string& endpoint, const std::string &path, httplib::Headers headers = {}, const std::string &content = {}, const std::string &content_type = "text/plain") {
    httplib::Client client = MakeSecureClient(endpoint);

	auto resp = content.size() ? client.Post(path, headers, content, content_type) : client.Post(path, headers);

	if (!resp || resp.error() != httplib::Error::Success){
		LogHttp(Error, "POST Request to % failed with internal error %", endpoint + path, httplib::to_string(resp.error()));
		return std::nullopt;
	}

	if (resp->status != httplib::StatusCode::OK_200){
		LogHttp(Error, "POST Request to % failed with http status %", endpoint + path, resp->status);
		return std::nullopt;
	}

	return resp->body;
}

inline std::optional<httplib::StatusCode> HttpPostStatus(const std::string& endpoint, const std::string &path, httplib::Headers headers = {}, const std::string &content = {}, const std::string &content_type = "text/plain") {
    httplib::Client client = MakeSecureClient(endpoint);

	auto resp = content.size() ? client.Post(path, headers, content, content_type) : client.Post(path, headers);

	if (!resp || resp.error() != httplib::Error::Success){
		LogHttp(Error, "POST Request to % failed with internal error %", endpoint + path, httplib::to_string(resp.error()));
		return std::nullopt;
	}

	if (resp->status != httplib::StatusCode::OK_200){
		LogHttp(Error, "POST Request to % failed with http status %", endpoint + path, resp->status);
		return std::nullopt;
	}

	return {(httplib::StatusCode)resp->status};
}


inline nlohmann::json HttpPostJson(const std::string& endpoint, const std::string &path) {
	auto res = HttpPost(endpoint, path);

	if(!res.has_value())
		return {};

	return nlohmann::json::parse(res.value(), nullptr, false, false);
}

template<typename T>
std::optional<T> GetJsonProperty(const std::string &json_string, const std::string &property) {
	auto json = nlohmann::json::parse(json_string, nullptr, false, false);

	if(!json.count(property))
		return std::nullopt;

	return json[property].get<T>();
}

template<typename T>
std::optional<T> GetJsonObject(const std::string& json_string) {
	try {
		T object = nlohmann::json::parse(json_string);

		return object;
	} catch (...) {
		return std::nullopt;
	}
}

template<typename T>
T GetJsonPropertyOr(const std::string &json_string, const std::string &property, T value) {
	return GetJsonProperty<T>(json_string, property).value_or(value);
}