#ifndef __INPUT_REQUEST_PARSER_H
#define __INPUT_REQUEST_PARSER_H
#include <iostream>

#include "MessageBoard.h"
#include "ReceivedSocketData.h"
#include "TCPServer.h"


class InboundRequestParser
{
public:
	void operator()(TCPServer* server, const ReceivedSocketData* data) { ParseIncomingRequest(server, *data); }

private:
	void ParseIncomingRequest(TCPServer* server, ReceivedSocketData data);
	bool Parse(ReceivedSocketData* data);
};

#endif __INPUT_REQUEST_PARSER_H
