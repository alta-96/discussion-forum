#include <iostream>
#include <thread>
#include <vector>

#include "InboundRequestParser.h"
#include "TCPServer.h"

#define DEFAULT_PORT 12345

int main()
{
	TCPServer server(DEFAULT_PORT);
	ReceivedSocketData receivedData;

	std::vector<std::thread> serverThreads;

	std::cout << "Starting server. Send \"exit\" (without quotes) to terminate." << std::endl;

	while (true)
	{
		receivedData = server.accept();

		std::cout << "Connected to client: " << std::to_string(receivedData.ClientSocket) << std::endl;

		serverThreads.emplace_back(InboundRequestParser(), &server, &receivedData);

		for (auto& thread : serverThreads)
		{
			if (thread.joinable()) { thread.detach(); } // Allow the threads to destruct themselves once the client calls exit.
		}
	}
}