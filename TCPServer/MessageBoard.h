#ifndef __MESSAGE_BOARD_H
#define __MESSAGE_BOARD_H
#include <unordered_map>


class MessageBoard
{
public:
	static bool Post();
	static bool List();
	static bool Count();
	static bool Read();
	static bool Exit();

private:
	std::unordered_map<std::string, std::string[]> messageBoard;
};

#endif __MESSAGE_BOARD_H
