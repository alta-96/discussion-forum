#include "MessageBoard.h"

// Static init
std::shared_mutex MessageBoard::mbMutex;
std::unordered_map<std::string, std::vector<std::string>>* MessageBoard::messageBoard
{
	new std::unordered_map<std::string, std::vector<std::string>>
};

// POST to the messageBoard 
std::string MessageBoard::Post(const std::string& topic, const std::string& messageBody)
{
	const std::lock_guard<std::shared_mutex> mbLock(mbMutex);
	std::string truncatedTopic = topic;
	std::string truncatedMessageBody = messageBody;

	// Truncate topic or message to first 140 characters
	if (topic.length() > 140) { truncatedTopic = topic.substr(MAX_POST_LENGTH); }
	if (messageBody.length() > 140) { truncatedTopic = messageBody.substr(MAX_POST_LENGTH); }

	// Check to see if the max messages for a given topic has been reached
	if (messageBoard->find(topic) != messageBoard->end()) {
		if ((*messageBoard)[topic].size() >= MAX_MESSAGES_PER_TOPIC) {
			return "Limit of " 
				+ std::to_string(MAX_MESSAGES_PER_TOPIC) 
				+ " messages reached for topic: " + topic
				+ ". Please create a new topic";
		}
	}

	// Topic will automatically be created if key not already present in map.
	(*messageBoard)[truncatedTopic].push_back(truncatedMessageBody);
	return std::to_string((*messageBoard)[truncatedTopic].size() - 1);
}

// READ from the messageBoard 
std::string MessageBoard::Read(const std::string topic, unsigned int messageNumber)
{
	const std::lock_guard<std::shared_mutex> mbLock(mbMutex);

	// If topic doesn't exist
	if (messageBoard->find(topic) == messageBoard->end()) { return ""; }

	// If requested message number is out of scope
	if (((*messageBoard)[topic].size() - 1) < messageNumber) { return ""; }

	return (*messageBoard)[topic].at(messageNumber);
}

std::string MessageBoard::List() {
	const std::lock_guard<std::shared_mutex> mbLock(mbMutex);

	std::vector<std::string> messageBoardKeys;
	std::string response = "";
	
	for (auto key : *messageBoard) { messageBoardKeys.push_back("@" + key.first); }
	for (auto& key : messageBoardKeys) { 
		response += key;
		if (&key != &messageBoardKeys.back()) { response += "#"; }
	}

	return response;
}

std::string MessageBoard::Count(const std::string topic) {
	const std::lock_guard<std::shared_mutex> mbLock(mbMutex);

	std::string truncatedTopic = topic;
	if (topic.length() > 140) { truncatedTopic = topic.substr(MAX_POST_LENGTH); }

	// If topic exists
	if (messageBoard->find(topic) != messageBoard->end()) {
		return std::to_string((*messageBoard)[truncatedTopic].size());
	}

	return "0";
}
