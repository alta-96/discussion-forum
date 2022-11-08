#include "MessageBoard.h"

// Static init
std::shared_mutex MessageBoard::mbMutex;
std::unordered_map<std::string, std::vector<std::string>>* MessageBoard::messageBoard
{
	new std::unordered_map<std::string, std::vector<std::string>>
};

// POST to the messageBoard 
/* 
 * Because of the nature of the data-structure used to store messages (map);
 * If the topic does not exist in the map,
 * when attempting to index via it with the topic as the key,
 * the topic will automatically be created if key not already present in map.
 */
std::string MessageBoard::Post(const std::string& topic, const std::string& messageBody)
{
	const std::lock_guard<std::shared_mutex> mbLock(mbMutex);
	auto truncatedTopic = topic;
	auto truncatedMessageBody = messageBody;

	// Truncate topic or message to first 140 characters
	if (topic.length() > 140) { truncatedTopic = topic.substr(MAX_POST_LENGTH); }
	if (messageBody.length() > 140) { truncatedTopic = messageBody.substr(MAX_POST_LENGTH); }

	(*messageBoard)[truncatedTopic].push_back(truncatedMessageBody);
	return std::to_string((*messageBoard)[truncatedTopic].size());
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



























//MessageBoard* MessageBoard::GetInstance()
//{
//	{
//		if (messageBoardInstance == nullptr)
//		{
//			const std::lock_guard<std::mutex> lock(mbMutex);
//			messageBoardInstance = new MessageBoard();
//			messageBoard = new std::unordered_map<std::string, std::vector<std::string>>();
//		}
//		return messageBoardInstance;
//	}
//}
//
//std::string MessageBoard::Post(const std::string& topic, const std::string& messageBody)
//{
//	(*messageBoard).insert(std::pair<std::string, std::vector<std::string>>{topic, std::vector<std::string>{messageBody}});
//
//	return std::to_string(static_cast<int>((*messageBoard)[topic].size()) - 1);
//}
//
//std::string MessageBoard::List()
//{
//	std::string list;
//	for (const auto& topic : *messageBoard)
//	{
//		list.append(topic.first + "#");
//	}
//	return list;
//}
//
//std::string MessageBoard::Read(const std::string topic, unsigned int messageIndex)
//{
//	/*if(messageBoard->find(topic) != messageBoard->end())
//	{
//		if (((*messageBoard)[topic].size()) > messageIndex)
//		{
//			return (*messageBoard)[topic][messageIndex].message;
//		}
//	}*/
//	return"";
//}
