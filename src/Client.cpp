#include "Client.hpp"

#include "Router.hpp"
#include "RequestProcessor.hpp"
#include "PipeEnd.hpp"

void Client::startCgi(std::unique_ptr<CgiRunner> cgiRunner) {
	m_cgiRunner = std::shared_ptr<CgiRunner>(cgiRunner.release());
	spawn(std::make_unique<PipeEnd>(m_cgiRunner->getInFd(), PipeEnd::PipeType::IN, m_cgiRunner));
	spawn(std::make_unique<PipeEnd>(m_cgiRunner->getOutFd(), PipeEnd::PipeType::OUT, m_cgiRunner));
	spawn(std::make_unique<PipeEnd>(m_cgiRunner->getErrFd(), PipeEnd::PipeType::ERR, m_cgiRunner));
}

void Client::buildResponse() {
	Router router;
	RequestContext ctx = router.createContext(getParser().getRequest(), m_serverConfig);
	RequestProcessor processor;
	HttpResponse response = processor.process(ctx);

	std::cout << response << std::endl;

	if (response.isCgi()) {
		startCgi(response.getCgiRunner());
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
