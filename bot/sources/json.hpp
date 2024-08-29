#pragma once

#include <nlohmann/json.hpp>
#include <bsl/file.hpp>

template<typename T>
class JsonFile: public T {
	std::string m_Filename;	
public:
	JsonFile(const char *filename):
		m_Filename(filename)
	{
		std::string content = ReadEntireFile(ChatsFile);
	
		try{
			Data() = nlohmann::json::parse(content, nullptr, false).get<T>();
		} catch (...) {

		}
	}
	
	void Save()const{
		WriteEntireFile(m_Filename, nlohmann::json(Data()).dump());
	}

	T& Data(){
		return *((T*)this);
	}

	const T& Data()const{
		return *((T*)this);
	}
};

