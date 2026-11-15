#pragma once

#include <string>
#include <map>

class Session
{
	private:
		std::string m_id;
		std::map<std::string, std::string> m_data;
		bool m_isEmpty;
	public:
		Session(const std::string& id);
		Session(bool isEmpty);

		const std::string& getId() const;
		void set(const std::string& key,
				const std::string& value);
		std::string get(const std::string& key) const;
		void clear();
		std::string buildHeaderValue();
};
