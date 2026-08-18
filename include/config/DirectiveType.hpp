#pragma once
#include <string_view>

enum class DirectiveType {
	LISTEN,
	NAME,
	ROOT,
	INDEX,
	ERRORPAGE,
	LOCATION,
	AUTOINDEX,
	MAXBODYSIZE,
	CGIHANDLER,
	UNKNOWN
};

DirectiveType directiveFromString(std::string_view name) {
	if (name == "listen") 							return DirectiveType::LISTEN;
	if (name == "server_name")					return DirectiveType::NAME;
	if (name == "root") 								return DirectiveType::ROOT;
	if (name == "index") 								return DirectiveType::INDEX;
	if (name == "error_page") 					return DirectiveType::ERRORPAGE;
	if (name == "location") 						return DirectiveType::LOCATION;
	if (name == "autoindex") 						return DirectiveType::AUTOINDEX;
	if (name == "client_max_body_size") return DirectiveType::MAXBODYSIZE;
	if (name == "cgi_handler") 					return DirectiveType::CGIHANDLER;
	return DirectiveType::UNKNOWN;
}

