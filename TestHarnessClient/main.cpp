#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#include "TCPClient.h"

#define DEFAULT_PORT 12345

std::string GetPostRequest(std::string::size_type length);
void sendReadReq();

int main()
{
    std::vector<std::thread> post_req_threads;

    for (int i = 0; i < 2; i++) {
        post_req_threads.emplace_back(std::thread(&sendReadReq));
    }

    for (auto& t : post_req_threads)
    {
        t.join();
    }
}

void sendReadReq()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Spinning up POST Thread: " << std::this_thread::get_id() << std::endl;
    TCPClient client("localhost", DEFAULT_PORT);
    std::string request = "";

    auto start = std::chrono::system_clock::now();
    int secondsToRunTest = 10;
    int msgCount = 0;

    client.OpenConnection();

    while ((std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() <=
        secondsToRunTest))
    {
        std::string post_request = GetPostRequest(10);
        client.send(post_request);
        msgCount++;
    }
    client.send("EXIT");
    std::cout << "POST Thread: " << std::this_thread::get_id() << " Sent " << msgCount << " in "
        << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count()
        << "seconds " << std::endl;

    std::cin;
}


std::string random_string(std::string::size_type length)
{
    // Constant look-up table
    static auto& chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rg{ std::random_device{}() };
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;
    s.reserve(length);

    while (length--)
        s += chrs[pick(rg)];

    return s;
}

std::string GetPostRequest(std::string::size_type length)
{
    return std::string("POST@") + random_string(length) + std::string("#") + random_string(length);
}