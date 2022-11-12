#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "TCPClient.h"

#define SERVER_PORT 12345
#define MAX_THREADS_PER_READER_WRITER 16

// The helper message is returned when invalid number of args are provided
void sendHelperMessage()
{
	std::cout << std::right
	<< "\nRequires at least 5 arguments to run!\n"
	<< "Usage: \nTestHarnessClient.exe server_ip number_of_poster_threads number_of_reader_threads time_duration throttle(0|1) \n\n"
	<< std::setw(25) << "server_ip" << std::setw(20) << " - IP of server (localhost or 127.0.0.1 if running locally)\n"
	<< std::setw(25) << "number_of_poster_threads" << std::setw(20) << " - The number of threads performing POST operations\n"
	<< std::setw(25) << "number_of_reader_threads" << std::setw(20) << " - The number of threads performing READ operations\n"
	<< std::setw(25) << "time_duration" << std::setw(20) << " - How long you want the Test-Harness to run for (seconds)\n"
	<< std::setw(25) << "throttle(0|1)" << std::setw(20) << " - 0: no throttling\n"
	<< std::setw(25)<< "" << std::setw(20) << " - 1: throttle to 1,000 requests per second" << std::endl;
}

// Casts the cmd-line parsed argument to unsigned integers
unsigned int castThreadArgToUnsignedInt(char* str, bool isThread)
{
	unsigned int parsedStringToUInt = static_cast<unsigned int>(std::stoull(str));

	// If cmd-line arg is a thread count, limit to max threads
	if (isThread && parsedStringToUInt > MAX_THREADS_PER_READER_WRITER)
	{
		parsedStringToUInt = MAX_THREADS_PER_READER_WRITER;
	}

	return parsedStringToUInt;
}

// The multi-threaded poster entry function for each poster thread
void MultiThreadedPosterFunction(
	const std::string& serverAddress,
	unsigned int runDuration,
	bool throttle)
{
	TCPClient client(serverAddress, SERVER_PORT);
	unsigned int currentThreadPostReqCount = 0;

	client.OpenConnection();


	client.CloseConnection();
}

// The multi-threaded reader entry function for each reader thread
void MultiThreadedReaderFunction(
	const std::string& serverAddress,
	unsigned int runDuration,
	bool throttle)
{
	TCPClient client(serverAddress, SERVER_PORT);
	unsigned int currentThreadReadReqCount = 0;

	client.OpenConnection();


	client.CloseConnection();
}

int main(int argc, char **argv)
{
	// Optimal max supported threads without significant performance loss
	const unsigned int maxThreads = std::thread::hardware_concurrency();

	std::vector<std::thread> posters;
	std::vector<std::thread> readers;
	
	std::cout << "Test-Harness Client" << std::endl;
	if (argc < 6) { sendHelperMessage(); return 0; } // Handle invalid args

	// Parse cmd-line arguments
	const std::string serverAddress = argv[1];
	const unsigned int posterThreads = castThreadArgToUnsignedInt(argv[3], true);
	const unsigned int readerThreads = castThreadArgToUnsignedInt(argv[4], true);
	const unsigned int duration = castThreadArgToUnsignedInt(argv[5] ,false);
	const bool throttle = castThreadArgToUnsignedInt(argv[6], false) > 0; // treat anything > 0 as true

	// Hardware concurrency limit warning
	if ((posterThreads + readerThreads) > maxThreads)
	{
		std::cout << "\n[WARN]: " << std::endl;
		std::cout << "You are running the Test-Harness with a total of "
				  << posterThreads + readerThreads << " threads." << std::endl;

		std::cout << "Your system optimally performs at " << maxThreads << std::endl;
		std::cout << "This will massively hinder performance...\n " << std::endl;
	}

	// Setup poster threads
	if (posterThreads > 0)
	{
		std::cout << "Setting up " << posterThreads << " poster threads..." << std::endl;

		for (unsigned int i = 0; i < posterThreads; i++)
		{
			posters.emplace_back(&MultiThreadedPosterFunction, &serverAddress, duration, throttle);
		}
	}

	// Setup reader threads
	if (readerThreads > 0)
	{
		std::cout << "Setting up " << readerThreads << " reader threads..." << std::endl;

		for (unsigned int i = 0; i < readerThreads; i++)
		{
			posters.emplace_back(&MultiThreadedReaderFunction, &serverAddress, duration, throttle);
		}
	}

	// Cleanup threads
	for (auto &thread : posters){ thread.join(); }
	for (auto &thread : readers){ thread.join(); }

	std::cout << "\npress enter to continue...";
	std::cin.get();
	return 0;
}
