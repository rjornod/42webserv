#pragma once

#include <string>
#include <map>
#include <ctime>

class Session
{
	private:
		std::string m_id;
		std::map<std::string, std::string> m_data;
		std::time_t m_createdAt;
		std::time_t m_lastActivity;
		bool m_isEmpty;
	public:
		Session(const std::string& id);
		Session(bool isEmpty);

		const std::string& getId() const;
		void set(const std::string& key,
				const std::string& value);
		std::string get(const std::string& key) const;
		std::string buildHeaderValue();
		void updateActivity();
		bool isExpired(unsigned int timeout) const;
};
// TODO: insert return statement here