#pragma once
#include <iostream>

// TO DO: figure out if this is the best way to check for the allowed directives in the config file
enum class LocationDirectiveType {
	Root,
	AutoIndex,
	MaxBodySize,
	Index,
	ErrorPage,
	LimitExcept,
	UploadStore,
	Return,
	Unknown
};

LocationDirectiveType locationDirectiveFromString(const std::string& name) {
	if (name == "root") 									return LocationDirectiveType::Root;
	if (name == "auto_index")							return LocationDirectiveType::AutoIndex;
	if (name == "client_max_body_size") 	return LocationDirectiveType::MaxBodySize;
	if (name == "index") 									return LocationDirectiveType::Index;
	if (name == "error_page") 						return LocationDirectiveType::ErrorPage;
	if (name == "limit_except") 						return LocationDirectiveType::LimitExcept;
	if (name == "upload_store")						return LocationDirectiveType::UploadStore;
	if (name == "return") 								return LocationDirectiveType::Return;
	return LocationDirectiveType::Unknown;
}