#include "Socket.hpp"

Socket::Socket() : _request(), _response()
{
	_readChecker = false;
	_bodyLen = 0;
	_endOfHeader = 0;
	_requestChecker = false;
}

Socket::Socket(int fd): _request(), _response()
{
	_readChecker = false;
	_socketFd = fd;
	_readChecker = false;
	_bodyLen = 0;
	_startIndex = 0;
	_chunkedBuff = "";
	_endOfHeader = 0;
	_requestChecker = false;
}

Socket::Socket(const Socket &from)
{
	*this = from;
}

Socket::~Socket()
{
}

Socket		&Socket::operator=(const Socket &rvalue)
{
	if (this != &rvalue)
	{
		_socketFd = rvalue._socketFd;
		_buffer = rvalue._buffer;
		_readChecker = rvalue._readChecker;
		_bodyLen = rvalue._bodyLen;
		_startIndex = rvalue._startIndex;
		_chunkedBuff = rvalue._chunkedBuff;
		_endOfHeader = rvalue._endOfHeader;
		_requestChecker = rvalue._requestChecker;
		_request = rvalue._request;
		_response = rvalue._response;
	}
	return *this;
}

// ***** GETTOR ******

int		Socket::getSocketFd() const
{
	return _socketFd;
}

const std::string&	Socket::getBuffer() const
{
	return _buffer;
}

bool		Socket::getReadChecker() const
{
	return _readChecker;
}

int			Socket::getBodyLen() const
{
	return _bodyLen;
}

int			Socket::getEndOfHeader() const
{
	return _endOfHeader;
}

Request&		Socket::getRequest()
{
	return (_request);
}

Response&		Socket::getResponse()
{
	return (_response);
}

bool		Socket::getRequestChecker()
{
	return (_requestChecker);
}

void		Socket::addStringToBuff(char *addBuff)
{
	_buffer += addBuff;
}
void		Socket::addStringToBuff(std::string addBuff)
{
	_buffer += addBuff;
}

void		Socket::clearBuffer()
{
	_buffer.clear();
}

void		Socket::setReadChecker(bool b)
{
	_readChecker = b;
}

void		Socket::setBodyLen(int bodyLen)
{
	_bodyLen = bodyLen;
}
