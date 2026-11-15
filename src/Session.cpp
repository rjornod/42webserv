#include "Session.hpp"

Session::Session(const std::string &id) : m_id(id), m_isEmpty(false) {}

Session::Session(bool isEmpty) : m_id(""), m_isEmpty(isEmpty) {}

const std::string &Session::getId() const {
	return m_id;
}

void Session::set(const std::string &key, const std::string &value) {
	m_data[key] = value;
}

std::string Session::get(const std::string &key) const {
	auto it = m_data.find(key);
	if (it != m_data.end()) {
		return it->second;
	}
	return "";
}

#include <iostream>

std::string Session::buildHeaderValue() {
	std::string result = "SESSION_ID=" + m_id;
	if (m_isEmpty) {
		return result + "; token=deleted; path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT";
	}
	for (auto &[key, value] : m_data) {
		if (key == "SESSION_ID") {
			continue;
		}
		result += "; " + key + "=" + value;
	}
	return result;
}
