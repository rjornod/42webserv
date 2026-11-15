#include "SessionManager.hpp"

#include <cstdlib>
#include <ctime>

SessionManager::SessionManager() : m_emptySession(true) {
    std::srand(std::time(NULL));
}

Session& SessionManager::createSession() {
    std::string id;

    for (int i = 0; i < 32; ++i)
        id += "0123456789abcdef"[std::rand() % 16];

    Session session(id);
    m_sessions.insert(std::make_pair(id, session));

    return m_sessions.find(id)->second;
}

Session &SessionManager::getSession(const std::string &id) {
  auto it = m_sessions.find(id);
  if (it == m_sessions.end()) {
	return createSession();
  }
  return it->second;
}

Session &SessionManager::getSessionFromRequest(
    const std::unordered_map<std::string, std::string> &cookies) {
	auto cit = cookies.find("SESSION_ID");
	auto &session = (cit == cookies.end()) ? createSession() : getSession(cit->second);
	for (auto &[key, value] : cookies) {
		if (key == "SESSION_ID") {
			continue;
		}
		session.set(key, value);
	}
	return session;
}

Session &SessionManager::getEmptySession() {
  return m_emptySession;
}

void SessionManager::deleteSession(const std::string& id) {
    m_sessions.erase(id);
}

void SessionManager::cleanExpiredSessions() {
	// todo cleanup
}
