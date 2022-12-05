#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <fstream>
#include <map>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <sys/stat.h>

#include "TCPClient.h"
#include "ThreadBarrier.h"

#define SERVER_PORT 12345
#define MAX_THREADS_PER_READER_WRITER 16


std::vector<std::thread> posters;
std::vector<std::thread> readers;

std::map<const std::string, std::vector<int>> resultMap;

std::string serverAddress;
std::mutex threadMutex;
ThreadBarrier* barrier;
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
	int currentThreadPostReqCount = 0;
	auto threadId = std::this_thread::get_id();
	const auto postReq = std::string("POST@");
	std::vector<int> secondBySecondThreadResult;
	int secondTracker = 1;

	client.OpenConnection();
	barrier->wait();
	
	auto startPoint = std::chrono::system_clock::now();
	auto runningPoint = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startPoint);

	while (runningPoint.count() <= duration)
	{
		if (runningPoint.count() >= secondTracker)
		{
			secondTracker++;
			secondBySecondThreadResult.push_back(currentThreadPostReqCount);
			currentThreadPostReqCount = 0;
		}

		runningPoint = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startPoint);

		if (throttle && currentThreadPostReqCount == 1000) { continue; }

		TopicMsgStruct topicMsgStruct = generateTopicMsg(false);

		client.send(postReq + topicMsgStruct.topic + '#' + topicMsgStruct.msg);
		currentThreadPostReqCount++;
	}

	threadMutex.lock();

	std::stringstream key;
	key << "POST" << threadId;
	resultMap.insert({key.str(), secondBySecondThreadResult});

	threadMutex.unlock();

	client.CloseConnection();
}

// The multi-threaded reader entry function for each reader thread
void MultiThreadedReaderFunction()
{
	TCPClient client(serverAddress, SERVER_PORT);
	int currentThreadReadReqCount = 0;
	auto threadId = std::this_thread::get_id();
	const auto readReq = std::string("READ@");
	std::vector<int> secondBySecondThreadResult;
	int secondTracker = 1;

	client.OpenConnection();
	barrier->wait();

	auto startPoint = std::chrono::system_clock::now();
	auto runningPoint = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startPoint);

	while (runningPoint.count() <= duration)
	{
		if (runningPoint.count() >= secondTracker)
		{
			secondTracker++;
			secondBySecondThreadResult.push_back(currentThreadReadReqCount);
			currentThreadReadReqCount = 0;
		}

		runningPoint = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startPoint);

		if (throttle && currentThreadReadReqCount == 1000) { continue; }

		TopicMsgStruct topicMsgStruct = generateTopicMsg(false);

		client.send(readReq + topicMsgStruct.topic);
		currentThreadReadReqCount++;
	}

	threadMutex.lock();

	std::stringstream key;
	key << "READ" << threadId;
	resultMap.insert({ key.str(), secondBySecondThreadResult });

	threadMutex.unlock();

	client.CloseConnection();
}

void ProcessResultFindings(const std::map<const std::string, std::vector<int>>& map)
{
	std::cout << "\n\n\n------------------------------------------------------------------" << std::endl;
	std::cout << "------------------------- R E S U L T S --------------------------" << std::endl;
	std::cout << "------------------------------------------------------------------" << std::endl;
	std::cout << "\nTest-Harness results after running for a total of " << duration << " seconds..." << std::endl;
	unsigned int postThreadCount = 0;
	unsigned int readThreadCount = 0;

	unsigned int runningTotalPostReqs = 0;
	unsigned int runningTotalReadReqs = 0;

	// create file if not already exists else find a free name for version of run.
	const std::string resultsFullFileName = "test-harness-results-v";
	unsigned int runVersion = 0;
	struct stat buf;

	// While file name taken, find a free file name for writing results of this run...
	while(stat((resultsFullFileName + std::to_string(runVersion) + ".full" + ".csv").c_str(), &buf) != -1) { runVersion++; }

	std::ofstream fullResultsFile;
	std::ofstream minResultsFile;
	
	fullResultsFile.open(resultsFullFileName + std::to_string(runVersion) + ".full" + ".csv");
	minResultsFile.open(resultsFullFileName + std::to_string(runVersion) + ".min" + ".txt");

	fullResultsFile << ",SECONDS\n,";

	for (int i = 0; i < duration; i++)
	{
		fullResultsFile << i << "s,";
	}

	for (auto& result : resultMap)
	{
		if (result.first.find("POST") != std::string::npos)
		{
			const unsigned int totalPostRequestsForCurrentThread = std::accumulate(result.second.begin(), result.second.end(), 0);
			runningTotalPostReqs += totalPostRequestsForCurrentThread;
			std::cout << "\nPOST thread " << postThreadCount << " sent:" << " [id - " << result.first.substr(4) << "]" << std::endl;

			fullResultsFile << "\nPOST-" << postThreadCount << ",";
			minResultsFile << "\nPOST-" << postThreadCount << "\t" << totalPostRequestsForCurrentThread;

			for(int i = 0; i < result.second.size(); i++)
			{
				std::cout << "\tSecond " << i << ": " << result.second[i] << " requests." << std::endl;
				fullResultsFile << result.second[i] << ",";
			}

			fullResultsFile << "\n";
			postThreadCount++;

			std::cout << "\t------------------------" << std::endl;

			std::cout << "\tTotal: " << totalPostRequestsForCurrentThread << "." << std::endl;
			fullResultsFile << "Total," << totalPostRequestsForCurrentThread << "\n";
			minResultsFile << "\nTOTAL\t" << totalPostRequestsForCurrentThread << "\n";

			std::cout << "\tAverage: " << 
				static_cast<double>(totalPostRequestsForCurrentThread) / duration << " per second." << std::endl;
			fullResultsFile << "Avg," << static_cast<double>(totalPostRequestsForCurrentThread) / duration;
			minResultsFile << "AVG\t" << static_cast<double>(totalPostRequestsForCurrentThread) / duration << "\n";
		}

		if (result.first.find("READ") != std::string::npos)
		{
			const unsigned int totalReadRequestsForCurrentThread = std::accumulate(result.second.begin(), result.second.end(), 0);
			runningTotalReadReqs += totalReadRequestsForCurrentThread;
			std::cout << "\nREAD thread " << readThreadCount << " sent:" << " [id - " << result.first.substr(4) << "]" << std::endl;
		
			fullResultsFile << "\nREAD-" << readThreadCount << ",";
			minResultsFile << "\nREAD-" << readThreadCount << "\t" << totalReadRequestsForCurrentThread;

			for (int i = 0; i < result.second.size(); i++)
			{
				std::cout << "\tSecond " << i << ": " << result.second[i] << " requests." << std::endl;
				fullResultsFile << result.second[i] << ",";
			}

			fullResultsFile << "\n";
			readThreadCount++;

			std::cout << "\t------------------------" << std::endl;

			std::cout << "\tTotal: " << totalReadRequestsForCurrentThread << "." << std::endl;
			fullResultsFile << "Total," << totalReadRequestsForCurrentThread << "\n";
			minResultsFile << "\nTOTAL\t" << totalReadRequestsForCurrentThread << "\n";

			std::cout << "\tAverage: " << 
				static_cast<double>(totalReadRequestsForCurrentThread) / duration << " per second." << std::endl;
			fullResultsFile << "Avg," << static_cast<double>(totalReadRequestsForCurrentThread) / duration;
			minResultsFile << "AVG\t" << static_cast<double>(totalReadRequestsForCurrentThread) / duration << "\n";
		}
	}

	std::cout << "\nOverall:" << std::endl;
	std::cout << "\tTotal requests: " << runningTotalPostReqs + runningTotalReadReqs << "." << std::endl;
	std::cout << "\tTotal POST requests: "  << runningTotalPostReqs << "." << std::endl;
	std::cout << "\tTotal READ requests: "  << runningTotalReadReqs << "." << std::endl;

	std::cout << "\n\tTotal average requests: " << 
		static_cast<double>(runningTotalPostReqs + runningTotalReadReqs) / duration << " per second." << std::endl;
	std::cout << "\tTotal average POST requests: " << static_cast<double>(runningTotalPostReqs) / duration << " per second." << std::endl;
	std::cout << "\tTotal average READ requests: " << static_cast<double>(runningTotalReadReqs) / duration << " per second." << std::endl;

	fullResultsFile.close();
	minResultsFile.close();
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

	barrier = new ThreadBarrier(posterThreads + readerThreads);

	std::cout << "Running Test-Harness Client for " 
		<< duration << " seconds on " << posterThreads << " poster threads & "
		<< readerThreads << " reader threads. Throttling: " << (throttle == 0 ? "disabled" : "enabled") << std::endl;

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
		for (unsigned int i = 0; i < posterThreads; i++)
		{
			posters.emplace_back(&MultiThreadedPosterFunction);
		}
	}

	// Setup reader threads
	if (readerThreads > 0)
	{
		for (unsigned int i = 0; i < readerThreads; i++)
		{
			readers.emplace_back(&MultiThreadedReaderFunction);
		}
	}

	// Cleanup threads
	for (auto &thread : posters){ thread.join(); }
	for (auto &thread : readers){ thread.join(); }

	// Process results
	ProcessResultFindings(resultMap);

	std::cout << "\npress enter to continue...";
	std::cin.get();
	return 0;
}
