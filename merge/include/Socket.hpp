#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <string>

class Socket
{
private:
	int			_socketFd;
	std::string	_buffer;
	bool		_readChecker;
	int			_bodyLen;
	int			_startIndex;
	std::string	_chunkedBuff;

	Socket();
public:
	Socket(int fd);
	Socket(const Socket &from);
	~Socket();
	Socket	&operator=(const Socket &rvalue);

	// ***** GETTOR ******
	int			getSocketFd() const;
	std::string	getBuffer() const;
	bool		getReadChecker() const;
	int			getBodyLen() const;
	int			getStartIndex() const
	{
		return _startIndex;
	}
	std::string	getChunkedBuff() const
	{
		return _chunkedBuff;
	}
	// ***** SETTER ******
	void		setReadChecker(bool b);
	void		setBodyLen(int bodyLen);
	void		setStartIndex(int num)
	{
		_startIndex = num;
	}
	void		setChunkedBuff(std::string str)
	{
		_chunkedBuff = str;
	}

	// ***** memberFunction ******
	void		addStringToBuff(char *addBuff);
	void		addStringToBuff(std::string addBuff);
	void		clearBuffer();
	void		clearChunkBuffer()
	{
		_chunkedBuff.clear();
	}
};

#endif