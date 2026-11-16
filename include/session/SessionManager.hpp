#pragma once

#include "Session.hpp"
#include <map>
#include <unordered_map>
#include <string>

class SessionManager
{
private:
    std::map<std::string, Session> m_sessions;
	Session m_emptySession;
public:
	SessionManager();
    Session& createSession();
	Session& getSession(const std::string& id);
    Session& getSessionFromRequest(const std::unordered_map<std::string, std::string>& cookies);
	Session& getEmptySession();
    void deleteSession(const std::string& id);
};
