#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <vector>
# include <string>
# include <cstring>
# include <iostream>
# include <unistd.h>
# include <fcntl.h>
# include <sstream>
# include <netinet/in.h>
# include <sys/socket.h>
# include <exception>
# include "Socket.hpp"
# include "Utils.hpp"
# include "Config.hpp"
# include "Location.hpp"
# include "Request.hpp"
# include "Response.hpp"

class Server
{
	friend int main(int, char**, char**);
	friend class Config;
public:
	enum Key {
		LISTEN, SERVER_NAME, ERROR_PAGE,
		NONE
	};

	Server();
	Server(const Server& other);
	~Server();
	Server& operator=(const Server& rhs);

	// ***********************GETTER************************
	int								getListenSocket() const;
	const Location&					getLocation(size_t index) const;
	const std::vector<Location>&	getLocations(void) const;

	bool					setMemberData(void);

	void					print(void);

	void					setListenSocket(void);
	void					setAddress(void);
	// err시 -1 리턴.
	int						bindSelf(void);
	int						listenSelf(int backLog);
	// server의 accept를 진행하는 부분이다.
	// 새로 받은 socket을 리턴한다.
	int						acceptSocket(fd_set *readSet, fd_set *writeSet);

	// server의 client소켓을 확인해보자.
	void					checkSet(fd_set *readSet, fd_set *writeSet, fd_set *copyr, fd_set *copyw);

private:
	// basicFrame
	int		_listenSocket;
	struct 	sockaddr_in _sAddr;
	struct	sockaddr_in _cAddr;
	// 읽었는지 확인하는 bool이 필요함. buffer도 함께 가지고 있다.
	// std::map<int, std::pair<std::string, bool> >	_sockets;
	std::vector<Socket> _sockets;

	// config data
	std::map<std::string, std::string>	_data;
	std::vector<Location>				_locations;
	std::string							_host;
	size_t								_port;
	std::string							_serverName;
	std::string							_errorPage;

	Key		_getKeyNumber(const std::string& key) const;
	void	_setListen(const std::string& value);
	void	_setServerName(const std::string& value);
	void	_setErrorPage(const std::string& value);


	// cluster socket이 닫히면 return 1
	int		_checkReadSetAndExit(std::vector<Socket>::iterator& iter, fd_set *readSet, fd_set *writeSet);
	int		_checkWriteSet(std::vector<Socket>::iterator& iter, fd_set *readSet, fd_set *writeSet);
	int		_socketDisconnect(std::vector<Socket>::iterator& iter, fd_set *readSet, fd_set *writeSet);
	size_t	_checkRN(std::string buff);
	void	_setReadEnd(std::vector<Socket>::iterator& iter);
};

#endif