#include <iostream>
#include <regex>
#include <thread>

#include "MessageBoard.h"
#include "TCPServer.h"

#define DEFAULT_PORT 12345

bool terminateServer = false;

struct PostRequest
{
	PostRequest(const std::string& topic, const std::string& message)
	: topic(topic), message(message) {}
	const std::string topic, message;
};

struct ReadRequest
{
	ReadRequest(const std::string& topic, const unsigned int& messageNumber)
	: topic(topic), messageNumber(messageNumber) {}
	const std::string topic; const unsigned int messageNumber;
};

void ParseIncomingRequest(TCPServer* server, ReceivedSocketData&& data);
std::string Parse(const std::string& incomingReq);
PostRequest ProcessPostRequest(const std::string& request);
ReadRequest ProcessReadRequest(const std::string& request);

int main()
{
	TCPServer server(DEFAULT_PORT);
	
	std::cout << "Starting server. Send \"exit\" (without quotes) to terminate." << std::endl;

	while (!terminateServer)
	{
		ReceivedSocketData receivedData = server.accept();

		std::thread t(ParseIncomingRequest, &server, receivedData);
		t.detach();
		
	}
}

void ParseIncomingRequest(TCPServer* server, ReceivedSocketData&& data)
{
	while (data.socketAlive)
	{
		server->receiveData(data, 0);
		if (data.socketAlive)
		{
			if (data.request != "EXIT" || data.request != "exit")
			{
				data.reply = Parse(data.request);
			} else
			{
				data.socketAlive = false;
				server->closeClientSocket(data);
				terminateServer = true;
				ExitThread(0);
			}

			if (data.socketAlive) { server->sendReply(data); }
		}
	}
}

PostRequest ProcessPostRequest(const std::string& request)
{
	int endOfTopic = request.find('#');
	std::string topic = request.substr(5, (endOfTopic - 1) - request.find('@'));
	std::string message = request.substr(endOfTopic + 1);

	return { topic, message };
}

ReadRequest ProcessReadRequest(const std::string& request)
{
	int endOfTopic = request.find('#');
	std::string topic = request.substr(5, (endOfTopic - 1) - request.find('@'));
	unsigned int messageNumber = std::stoi(request.substr(endOfTopic + 1));

	return { topic, messageNumber };
}

std::string Parse(const std::string& incomingReq)
{
	if (incomingReq.find("POST@") != std::string::npos)
	{
		PostRequest postRequest = ProcessPostRequest(incomingReq);
		
		return "0";
	}

	if (incomingReq.find("READ") != std::string::npos)
	{
		ReadRequest readRequest = ProcessReadRequest(incomingReq);
		return "0";
	}

	if (incomingReq.find("LIST") != std::string::npos)
	{
		// process list
		return "0";
	}

	return "";
}