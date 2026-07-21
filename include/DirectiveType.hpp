#pragma once
#include <iostream>

// TO DO: figure out if this is the best way to check for the allowed directives in the config file
enum class DirectiveType {
	Listen,
	Name,
	Root,
	Index,
	ErrorPage,
	Location,
	AutoIndex,
	MaxBodySize,
	Unknown
};

DirectiveType directiveFromString(const std::string& name) {
	if (name == "listen") 							return DirectiveType::Listen;
	if (name == "server_name")					return DirectiveType::Name;
	if (name == "root") 								return DirectiveType::Root;
	if (name == "index") 								return DirectiveType::Index;
	if (name == "error_page") 					return DirectiveType::ErrorPage;
	if (name == "location") 						return DirectiveType::Location;
	if (name == "autoindex") 						return DirectiveType::AutoIndex;
	if (name == "client_max_body_size") return DirectiveType::MaxBodySize;
	return DirectiveType::Unknown;
}

