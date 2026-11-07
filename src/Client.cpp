#include "Client.hpp"

#include "Router.hpp"
#include "RequestProcessor.hpp"

void Client::buildResponse() {
	Router router;
	RequestContext ctx = router.createContext(getParser().getRequest(), m_serverConfig);
	RequestProcessor processor;
	HttpResponse response = processor.process(ctx);

	setHttpResponse(response);
	writerSetup(response);

	std::cout << getHttpResponse() << std::endl;
}

void Client::buildParserErrorResponse() {
	Router router;
	RequestContext ctx = router.createErrorContext(m_serverConfig);
	RequestProcessor processor;
	HttpResponse response = processor.buildErrorResponse(getParser().getErrorStatus(), ctx);

	response.addHeader("Connection", "close");
	setHttpResponse(response);
	writerSetup(response);

	std::cout << getHttpResponse() << std::endl;
}
