#include "MessageBoard.h"

namespace MessageBoardSingleton
{
	MessageBoard* MessageBoard::messageBoardInstance{ nullptr };
	std::mutex MessageBoard::mbMutex;
	std::unordered_map<std::string, std::vector<std::string>>* MessageBoard::messageBoard
	{
		new std::unordered_map<std::string, std::vector<std::string>>
	};

	MessageBoard* MessageBoard::GetInstance()
	{
		{
			if (messageBoardInstance == nullptr)
			{
				const std::lock_guard<std::mutex> lock(mbMutex);
				messageBoardInstance = new MessageBoard();
				messageBoard = new std::unordered_map<std::string, std::vector<std::string>>();
			}
			return messageBoardInstance;
		}
	}

	std::string MessageBoard::Post(const std::string& topic, const std::string& messageBody)
	{
		(*messageBoard).insert(std::pair<std::string, std::vector<std::string>>{topic, std::vector<std::string>{messageBody}});

		return std::to_string(static_cast<int>((*messageBoard)[topic].size()) - 1);
	}

	std::string MessageBoard::List()
	{
		std::string list;
		for (const auto& topic : *messageBoard)
		{
			list.append(topic.first + "#");
		}
		return list;
	}

	std::string MessageBoard::Read(const std::string topic, unsigned int messageIndex)
	{
		/*if(messageBoard->find(topic) != messageBoard->end())
		{
			if (((*messageBoard)[topic].size()) > messageIndex)
			{
				return (*messageBoard)[topic][messageIndex].message;
			}
		}*/
		return"";
	}
}