#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <map>
#include <mutex>
#include <random>
#include <sstream>

#include "TCPClient.h"

#define SERVER_PORT 12345
#define MAX_THREADS_PER_READER_WRITER 16

std::vector<std::thread> posters;
std::vector<std::thread> readers;

std::map<const std::string, const unsigned int> resultMap;

std::string serverAddress;
std::mutex threadMutex;
unsigned int duration = 0;
bool throttle = false;

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

unsigned int generateThreadSafeRandom(const unsigned int max)
{
	static thread_local std::mt19937 generator;
	return std::uniform_int_distribution<unsigned int>{5, max}(generator);
}

struct TopicMsgStruct
{
	std::string topic;
	std::string msg;

	TopicMsgStruct(const std::string& _topic, const std::string& _msg): topic(_topic), msg(_msg) {}
};
// Random topic/message generator for POST/READ requests
TopicMsgStruct generateTopicMsg(bool generateMessage)
{
	const int topicMaxLength = 70;
	const int messageMaxLength = 140;

	static const char availableChars[] =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"0123456789";
	std::string tempTopic, tempMessage;
	tempTopic.reserve(topicMaxLength);
	tempMessage.reserve(messageMaxLength);

	for (int i = 0; i < generateThreadSafeRandom(topicMaxLength); i++)
	{
		tempTopic += availableChars[generateThreadSafeRandom( sizeof(availableChars) - 1)];
	}

	if (generateMessage)
	{
		for (int i = 0; i < generateThreadSafeRandom(messageMaxLength); i++)
		{
			tempMessage += availableChars[generateThreadSafeRandom(sizeof(availableChars) - 1)];
		}
	}
	return{ tempTopic, tempMessage };
}


// The multi-threaded poster entry function for each poster thread
void MultiThreadedPosterFunction()
{
	TCPClient client(serverAddress, SERVER_PORT);
	unsigned int currentThreadPostReqCount = 0;
	auto startPoint = std::chrono::system_clock::now();
	auto threadId = std::this_thread::get_id();
	const auto postReq = std::string("POST@");

	client.OpenConnection();

	while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startPoint).count() <=
		duration)
	{
		TopicMsgStruct topicMsgStruct = generateTopicMsg(false);

		client.send(postReq + topicMsgStruct.topic + '#' + topicMsgStruct.msg);
		currentThreadPostReqCount++;
	}

	threadMutex.lock();

	std::stringstream key;
	key << "POST" << threadId;
	resultMap.insert({key.str(), currentThreadPostReqCount});

	threadMutex.unlock();

	client.CloseConnection();
}

// The multi-threaded reader entry function for each reader thread
void MultiThreadedReaderFunction()
{
	TCPClient client(serverAddress, SERVER_PORT);
	unsigned int currentThreadReadReqCount = 0;
	auto startPoint = std::chrono::system_clock::now();
	auto threadId = std::this_thread::get_id();
	const auto readReq = std::string("READ@");

	client.OpenConnection();

	while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startPoint).count() <=
		duration)
	{
		TopicMsgStruct topicMsgStruct = generateTopicMsg(false);

		client.send(readReq + topicMsgStruct.topic);
		currentThreadReadReqCount++;
	}

	threadMutex.lock();

	std::stringstream key;
	key << "READ" << threadId;
	resultMap.insert({ key.str(), currentThreadReadReqCount });

	threadMutex.unlock();

	client.CloseConnection();
}

void ProcessResultFindings(
	const std::vector<unsigned>& postResults,
	const std::vector<unsigned>& readResults)
{
	std::cout << "Post Results:" << std::endl;

	for (int i = 0; i < postResults.size(); i++)
	{
		std::cout << "\tPoster Thread " << (i + 1) << ":" << std::endl;
		std::cout << "\t\tTotal Requests: " << postResults[i] << std::endl;
	}

	std::cout << "Read Results:" << std::endl;
	for (int i = 0; i < readResults.size(); i++)
	{
		std::cout << "\tReader Thread " << (i + 1) << ":" << std::endl;
		std::cout << "\t\tTotal Requests: " << readResults[i] << std::endl;
	}
}

int main(int argc, char **argv)
{
	// Optimal max supported threads without significant performance loss
	const unsigned int maxThreads = std::thread::hardware_concurrency();

	if (argc != 6) { sendHelperMessage(); return 0; } // Handle invalid args

	// Parse cmd-line arguments
	serverAddress = argv[1];
	const unsigned int posterThreads = castThreadArgToUnsignedInt(argv[2], true);
	const unsigned int readerThreads = castThreadArgToUnsignedInt(argv[3], true);
	duration = castThreadArgToUnsignedInt(argv[4] ,false);
	throttle = castThreadArgToUnsignedInt(argv[5], false) > 0; // treat anything > 0 as true

	std::cout << "Running Test-Harness Client for " 
		<< duration << " seconds on " << posterThreads << " poster threads & "
		<< readerThreads << " reader threads. Throttle: " << throttle << std::endl;

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
			posters.emplace_back(&MultiThreadedPosterFunction);
		}
	}

	// Setup reader threads
	if (readerThreads > 0)
	{
		std::cout << "Setting up " << readerThreads << " reader threads..." << std::endl;

		for (unsigned int i = 0; i < readerThreads; i++)
		{
			posters.emplace_back(&MultiThreadedReaderFunction);
		}
	}

	// Cleanup threads
	for (auto &thread : posters){ thread.join(); }
	for (auto &thread : readers){ thread.join(); }

	// Process results
	std::vector<unsigned int> postResults;
	std::vector<unsigned int> readResults;

	for(std::pair<const std::string, const unsigned int>& result : resultMap)
	{
		if (result.first.find("POST"))
		{
			postResults.push_back(result.second);
		}

		if (result.first.find("READ"))
		{
			readResults.push_back(result.second);
		}
	}

	ProcessResultFindings(postResults, readResults);

	std::cout << "\npress enter to continue...";
	std::cin.get();
	return 0;
}
