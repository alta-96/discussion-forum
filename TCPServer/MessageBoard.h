#ifndef __MESSAGE_BOARD_H
#define __MESSAGE_BOARD_H
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

// Singleton MessageBoard class

class MessageBoard {

private:
    static std::unordered_map<std::string, std::vector<std::string>>* messageBoard;

public:
    static std::string Post(const std::string& topic, const std::string& messageBody);
    static std::string Read(const std::string topic, unsigned int messageIndex);
    /*
    static std::string List();
    static unsigned int Count(const std::string topic);
    */
};


#endif __MESSAGE_BOARD_H