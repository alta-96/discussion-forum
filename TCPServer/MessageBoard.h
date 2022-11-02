#ifndef __MESSAGE_BOARD_H
#define __MESSAGE_BOARD_H
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

// Singleton MessageBoard class

namespace MessageBoardSingleton
{
    class MessageBoard {
    private:
        static MessageBoard* messageBoardInstance;
        static std::mutex mbMutex;

        //// each post has a unique hash associated with it to validate existence.
        //struct PostStruct {
        //    unsigned int hash;
        //    std::string message;
        //    PostStruct(const unsigned int hash, const std::string msg) : hash(hash), message(msg) {}
        //};

        static std::unordered_map<std::string, std::vector<std::string>>* messageBoard;

    public:
        // Prevent singleton access race-condition.
        static MessageBoard* GetInstance();

        static std::string Post(const std::string& topic, const std::string& messageBody);
        static std::string List();
        static unsigned int Count(const std::string topic);
        static std::string Read(const std::string topic, unsigned int messageIndex);
    };
}

#endif __MESSAGE_BOARD_H