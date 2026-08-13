#pragma once
#include <string_view>

enum class DirectiveType {
	Listen,
	Name,
	Root,
	Index,
	ErrorPage,
	Location,
	AutoIndex,
	MaxBodySize,
	CGIHandler,
	Unknown
};

DirectiveType directiveFromString(std::string_view name) {
	if (name == "listen") 							return DirectiveType::Listen;
	if (name == "server_name")					return DirectiveType::Name;
	if (name == "root") 								return DirectiveType::Root;
	if (name == "index") 								return DirectiveType::Index;
	if (name == "error_page") 					return DirectiveType::ErrorPage;
	if (name == "location") 						return DirectiveType::Location;
	if (name == "autoindex") 						return DirectiveType::AutoIndex;
	if (name == "client_max_body_size") return DirectiveType::MaxBodySize;
	if (name == "cgi_handler") 					return DirectiveType::CGIHandler;
	return DirectiveType::Unknown;
}

