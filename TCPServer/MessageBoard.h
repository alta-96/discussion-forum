#ifndef __MESSAGE_BOARD_H
#define __MESSAGE_BOARD_H
#include <string>
#include <unordered_map>
#include <vector>

// Singleton MessageBoard class
class MessageBoard {
private:
    std::unordered_map<std::string, std::vector<std::string>> messageBoard;
    MessageBoard();  
    
public:
    static MessageBoard& GetInstance() {
        static MessageBoard instance;
        return instance;
    }

    unsigned int Post(const std::string topic, const std::string messageBody);
    std::string List() const;
    unsigned int Count(const std::string topic);
    std::string Read(const std::string topic, unsigned int messageIndex);
};
#endif __MESSAGE_BOARD_H