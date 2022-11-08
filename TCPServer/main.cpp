#include <iostream>
#include <thread>

#include "MessageBoard.h"
#include "TCPServer.h"

#define DEFAULT_PORT 12345
#define MAX_MESSAGES_PER_TOPIC 1000000

bool terminateServer = false;

// Processing the POST Request
struct PostRequest
{
	PostRequest(const std::string& topic, const std::string& message)
	: topic(topic), message(message) {}
	const std::string topic, message;
};
PostRequest ProcessPostRequest(const std::string& request)
{
	int endOfTopic = request.find('#');
	std::string topic = request.substr(5, (endOfTopic - 1) - request.find('@'));
	std::string message = request.substr(endOfTopic + 1);

	return { topic, message };
}

// Processing the READ request
struct ReadRequest
{
	ReadRequest(const std::string& topic, const unsigned int& messageNumber)
		: topic(topic), messageNumber(messageNumber) {}
	const std::string topic; const unsigned int messageNumber;
};
ReadRequest ProcessReadRequest(const std::string& request)
{
	int endOfTopic = request.find('#');
	std::string topic = request.substr(5, (endOfTopic - 1) - request.find('@'));
	unsigned int messageNumber;

	// Try convert string to positive integer
	try
	{
		messageNumber = std::stoi(request.substr(endOfTopic + 1));
	}
	catch (...)
	{
		messageNumber = MAX_MESSAGES_PER_TOPIC;
	}
	

	return { topic, messageNumber };
}

// Incoming request parser - checks which request processor to call.
std::string ParseRequest(const std::string& incomingReq)
{
	if (incomingReq.find("POST@") != std::string::npos)
	{
		PostRequest postRequest = ProcessPostRequest(incomingReq);
		
		return MessageBoard::Post(postRequest.topic, postRequest.message);
	}

	if (incomingReq.find("LIST@") != std::string::npos)
	{
		// process list request
		return "0";
	}

	if (incomingReq.find("COUNT@") != std::string::npos)
	{
		// process count request
		return "0";
	}

	if (incomingReq.find("READ@") != std::string::npos)
	{
		ReadRequest readRequest = ProcessReadRequest(incomingReq);

		return MessageBoard::Read(readRequest.topic, readRequest.messageNumber);
	}

	return "";
}

// The multi-threaded server function. Created & detached for each client connection.
// Persists until connection is lost or client calls exit.
void HandleClientRequests(TCPServer* server, ReceivedSocketData&& data)
{
	while (data.socketAlive)
	{
		server->receiveData(data, 1);
		if (data.socketAlive)
		{
			if (data.request != "EXIT" || data.request != "exit")
			{
				data.reply = ParseRequest(data.request);
			}
			else
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

int main()
{
	TCPServer server(DEFAULT_PORT);

	std::cout << "Starting server. Send \"exit\" (without quotes) to terminate." << std::endl;

	while (!terminateServer)
	{
		ReceivedSocketData receivedData = server.accept();

		std::thread t(HandleClientRequests, &server, receivedData);
		t.detach();
	}
}