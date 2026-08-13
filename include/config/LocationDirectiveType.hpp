#pragma once
#include <string_view>

enum class LocationDirectiveType {
	Root,
	AutoIndex,
	MaxBodySize,
	Index,
	ErrorPage,
	AllowedMethods,
	UploadStore,
	Return,
	CGIHandler,
	Unknown
};

LocationDirectiveType locationDirectiveFromString( std::string_view name) {
	if (name == "root") 									return LocationDirectiveType::Root;
	if (name == "autoindex")							return LocationDirectiveType::AutoIndex;
	if (name == "client_max_body_size") 	return LocationDirectiveType::MaxBodySize;
	if (name == "index") 									return LocationDirectiveType::Index;
	if (name == "error_page") 						return LocationDirectiveType::ErrorPage;
	if (name == "allowed_methods") 				return LocationDirectiveType::AllowedMethods;
	if (name == "upload_store")						return LocationDirectiveType::UploadStore;
	if (name == "return") 								return LocationDirectiveType::Return;
	if (name == "cgi_handler") 						return LocationDirectiveType::CGIHandler;
	return LocationDirectiveType::Unknown;
}