#include "InboundRequestParser.h"
void InboundRequestParser::ParseIncomingRequest(TCPServer* server, ReceivedSocketData data)
{
	unsigned int socketIndex = (unsigned int)data.ClientSocket;
	bool terminateConnection = false;

	do {
		server->receiveData(data, 1);
		
		terminateConnection = Parse((&data));
		server->sendReply(data);
			
	} while (!terminateConnection);

	server->closeClientSocket(data);
}


bool InboundRequestParser::Parse(ReceivedSocketData* data)
{
	std::string request = data->request;

	if (request.find("EXIT") != std::string::npos || request.find("exit") != std::string::npos)
	{
		std::cout << "Terminating client " << data->ClientSocket << " - Bye Bye!" << std::endl;
		return true;
	}

	if (request.find("POST") != std::string::npos)
	{
		data->reply = "You just did a post!";
	}

	if (request.find("LIST") != std::string::npos)
	{
		data->reply = "You just did a list";
	}

	if (request.find("COUNT") != std::string::npos)
	{
		data->reply = "You just did a count";
	}

	if (request.find("READ") != std::string::npos)
	{
		data->reply = "You just did a read";
	}

	return false;
}