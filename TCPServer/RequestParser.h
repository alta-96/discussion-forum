#ifndef __REQUEST_PARSER_H
#define __REQUEST_PARSER_H
#include <string>
#include "TCPServer.h"


class RequestParser
{
public:
	static bool isExitRequested;

	void operator()(TCPServer& server, ReceivedSocketData& data) {
		parseRequest(server, data);
	}

private:
	void parseRequest(TCPServer& server, ReceivedSocketData& data);
};

#endif __REQUEST_PARSER_H
