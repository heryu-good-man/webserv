#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <string>

class Socket
{
private:
	int		_socketFd;
	std::string	_buffer;
	bool		_readChecker;
	Socket();
public:
	Socket(int fd);
	Socket(const Socket &from);
	~Socket();
	Socket	&operator=(const Socket &rvalue);

	// ***** GETTOR ******
	int		getSocketFd() const;
	std::string	getBuffer() const;
	bool		getReadChecker() const;
	// ***** SETTER ******
	void		setReadChecker(bool b);


	// ***** memberFunction ******
	void		addStringToBuff(char *addBuff);
	void		clearBuffer();
};

#endif