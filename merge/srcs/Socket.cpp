#include "Socket.hpp"

Socket::Socket()
{
	_readChecker = false;
}

Socket::Socket(int fd)
{
	_readChecker = false;
	_socketFd = fd;
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


// ***** memberFunction ******
void		Socket::addStringToBuff(char *addBuff)
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
