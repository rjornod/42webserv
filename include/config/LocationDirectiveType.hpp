#pragma once
#include <string_view>

enum class LocationDirectiveType {
	ROOT,
	AUTOINDEX,
	MAXBODYSIZE,
	INDEX,
	ERRORPAGE,
	ALLOWEDMETHODS,
	UPLOADSTORE,
	RETURN,
	CGIHANDLER,
	UNKNOWN
};

LocationDirectiveType locationDirectiveFromString( std::string_view name) {
	if (name == "root") 									return LocationDirectiveType::ROOT;
	if (name == "autoindex")							return LocationDirectiveType::AUTOINDEX;
	if (name == "client_max_body_size") 	return LocationDirectiveType::MAXBODYSIZE;
	if (name == "index") 									return LocationDirectiveType::INDEX;
	if (name == "error_page") 						return LocationDirectiveType::ERRORPAGE;
	if (name == "allowed_methods") 				return LocationDirectiveType::ALLOWEDMETHODS;
	if (name == "upload_store")						return LocationDirectiveType::UPLOADSTORE;
	if (name == "return") 								return LocationDirectiveType::RETURN;
	if (name == "cgi_handler") 						return LocationDirectiveType::CGIHANDLER;
	return LocationDirectiveType::UNKNOWN;
}