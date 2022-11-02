#ifndef __RECEIVEDSOCKETDATA_H
#define __RECEIVEDSOCKETDATA_H

#include <winsock2.h>
#include <string>

class ReceivedSocketData
{
public:
	SOCKET ClientSocket;
	std::string request;
	std::string reply;
	bool socketAlive;

	ReceivedSocketData()
	{
		ClientSocket = INVALID_SOCKET;
		socketAlive = false;
	};

	ReceivedSocketData(const ReceivedSocketData &rhs)
	{
		this->ClientSocket = rhs.ClientSocket;
		this->reply = rhs.reply;
		this->request = rhs.request;
		socketAlive = true;
	}

	~ReceivedSocketData() { };

	ReceivedSocketData& operator = (const ReceivedSocketData &rhs)
	{
		this->ClientSocket = rhs.ClientSocket;
		this->reply = rhs.reply;
		this->request = rhs.request;
		socketAlive = true;
		return *this;
	}

};

#endif //__RECEIVEDSOCKETDATA_H
