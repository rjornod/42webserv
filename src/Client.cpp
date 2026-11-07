#include "Client.hpp"

#include "Router.hpp"
#include "RequestProcessor.hpp"

void Client::buildResponse() {
	Router router;
	RequestContext ctx = router.createContext(getParser().getRequest(), m_serverConfig);
	RequestProcessor processor;
	HttpResponse response = processor.process(ctx);

	std::cout << response << std::endl;

	if (response.isCgi()) {
		// TODO connect to Cgi Runner
	} else {
		writerSetup(std::move(response));
	}
}

void Client::buildParserErrorResponse() {
	Router router;
	RequestContext ctx = router.createErrorContext(m_serverConfig);
	RequestProcessor processor;
	HttpResponse response = processor.buildErrorResponse(getParser().getErrorStatus(), ctx);

	response.addHeader("Connection", "close");

	std::cout << response << std::endl;
	writerSetup(std::move(response));
}
