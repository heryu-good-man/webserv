#ifndef SOCKET_HPP
# define SOCKET_HPP

class Socket;

#include <sstream>
#include <string>
#include "Request.hpp"
#include "Response.hpp"

class Socket
{
private:
	int			_socketFd;
	std::string	_buffer;
	bool		_readChecker;
	int			_bodyLen;
	int			_startIndex;
	std::string	_chunkedBuff;
	int			_endOfHeader;
	bool		_requestChecker;
	Request		_request;
	Response	_response;

	Socket();
public:
	Socket(int fd);
	Socket(const Socket &from);
	~Socket();
	Socket	&operator=(const Socket &rvalue);

	// ***** GETTOR ******
	int			getSocketFd() const;
	const std::string&	getBuffer() const;
	bool		getReadChecker() const;
	int			getBodyLen() const;
	int			getEndOfHeader() const;
	int			getStartIndex() const
	{
		return _startIndex;
	}
	const std::string&	getChunkedBuff() const
	{
		return _chunkedBuff;
	}
	Request&	getRequest();
	Response&	getResponse();
	bool		getRequestChecker();
	// ***** SETTER ******
	void		setReadChecker(bool b);
	void		setBodyLen(int bodyLen);
	void		setStartIndex(int num)
	{
		_startIndex = num;
	}
	void		setEndOfHeader(int num)
	{
		_endOfHeader = num;
	}
	void		addChunkedBuff(const std::string& str)
	{
		_chunkedBuff += str;
	}
	void		setBuff(const std::string& str)
	{
		_buffer = str;
	}
	void		setRequest(const std::string& buffer)
	{
		_request = Request(buffer);
	}

	// ***** memberFunction ******
	void		addStringToBuff(char *addBuff);
	void		addStringToBuff(std::string addBuff);
	void		clearBuffer();
	void		clearChunkBuffer()
	{
		_chunkedBuff.clear();
	}
	void		setRequestChecker(bool b)
	{
		_requestChecker = b;
	}
};

#endif
