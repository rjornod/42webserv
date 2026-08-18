#pragma once

enum class ClientState {
	ReadingRequest,
	RequestFinished,
	BuildingResponse,
	SendingResponse
};