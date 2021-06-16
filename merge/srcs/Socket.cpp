#include "Socket.hpp"

Socket::Socket()
{
	_readChecker = false;
	_bodyLen = 0;
}

Socket::Socket(int fd)
{
	_socketFd = fd;
	_bodyLen = 0;
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
		_buffer = rvalue._buffer;
		_readChecker = rvalue._readChecker;
		_socketFd = rvalue._socketFd;
		_bodyLen = rvalue._bodyLen;
	}
	return *this;
}

// ***** GETTOR ******

int		Socket::getSocketFd() const
{
	return _socketFd;
}

std::string	Socket::getBuffer() const
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


// ***** memberFunction ******
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

// ***** SETTER ******
void		Socket::setReadChecker(bool b)
{
	_readChecker = b;
}


void		Socket::setBodyLen(int bodyLen)
{
	_bodyLen = bodyLen;
}