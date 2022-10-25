#include "RequestParser.h"

void RequestParser::parseRequest(TCPServer& server, ReceivedSocketData& data)
{
	if (data.request.find("exit") != std::string::npos || data.request.find("EXIT") != std::string::npos)
	{
		data.reply = "Shutting down...";

		server.sendReply(data);
		server.closeClientSocket(data);
		isExitRequested = true;
	} else
	{
		server.sendReply(data);
	}
}
