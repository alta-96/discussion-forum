#include <iostream>
#include <regex>
#include <thread>

#include "MessageBoard.h"
#include "TCPServer.h"

#define DEFAULT_PORT 12345
bool terminateServer = false;

struct PostRequest
{
	std::string _topic;
	std::string _msg;
	PostRequest(std::string topic, std::string msg) : _topic(std::move(topic)), _msg(std::move(msg)) {}
};

void ParseIncomingRequest(TCPServer* server, ReceivedSocketData&& data);
std::string Parse(const std::string& incomingReq);
PostRequest ProcessPostRequest(const std::string& request);

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
			}

			if (data.socketAlive) { server->sendReply(data); }
		}
	}

	server->closeClientSocket(data);
	ExitThread(0);
}

PostRequest ProcessPostRequest(const std::string& request)
{
	int endOfTopic = request.find('#');
	std::string topic = request.substr(5, (endOfTopic - 1) - request.find('@'));
	std::string message = request.substr(endOfTopic + 1);

	return {topic, message};
}

std::string Parse(const std::string& incomingReq)
{
	if (incomingReq.find("POST@") != std::string::npos)
	{
		PostRequest postRequest = ProcessPostRequest(incomingReq);


		// process post
		return "0";
	}

	if (incomingReq.find("READ") != std::string::npos)
	{
		// process read
		return "0";
	}

	if (incomingReq.find("LIST") != std::string::npos)
	{
		// process list
		return "0";
	}

	return "";
}




/*std::string request = data.request;

if (request.find("EXIT") != std::string::npos || request.find("exit") != std::string::npos)
{
	data.socketAlive = false;
}

if (request.find("POST") != std::string::npos)
{
	int count = 0;
	PostRequest req = PostRequest::parse(data.request);
	if (req.valid)
	{
		count++;
		data.reply = std::to_string(count);
	}
}*/
