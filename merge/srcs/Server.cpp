#include "Server.hpp"

const std::string		DEFAULT_HOST = "localhost";		// 변경 필요
const size_t			DEFAULT_PORT = 8080;
const std::string		DEFAULT_SERVER_NAME = "localhost";
const std::string		DEFAULT_ERROR_PAGE = "";		// 변경 필요


Server::Server()
	: _listenSocket()
	, _sAddr()
	, _cAddr()
	, _sockets()
	, _data()
	, _locations()
	, _host(DEFAULT_HOST)
	, _port(DEFAULT_PORT)
	, _serverName(DEFAULT_SERVER_NAME)
	, _errorPage(DEFAULT_ERROR_PAGE)
{

}

Server::Server(const Server& other)
	: _listenSocket(other._listenSocket)
	, _sAddr(other._sAddr)
	, _cAddr(other._cAddr)
	, _sockets(other._sockets)
	, _data(other._data)
	, _locations(other._locations)
	, _host(other._host)
	, _port(other._port)
	, _serverName(other._serverName)
	, _errorPage(other._errorPage)
{

}

Server::~Server()
{

}

Server& Server::operator=(const Server& rhs)
{
	if (this != &rhs)
	{
		_listenSocket = rhs._listenSocket;
		_sAddr = rhs._sAddr;
		_cAddr = rhs._cAddr;
		_sockets = rhs._sockets;
		_data = rhs._data;
		_locations = rhs._locations;
		_host = rhs._host;
		_port = rhs._port;
		_serverName = rhs._serverName;
		_errorPage = rhs._errorPage;
	}
	return (*this);
}

Location	Server::getLocation(size_t index) const
{
	if (0 <= index && index < _locations.size())
		return (_locations[index]);
	else
		throw std::runtime_error("index error: out_of_index by getServer");
}

std::vector<Location>	Server::getLocations(void) const
{
	return (_locations);
}

Server::Key	Server::_getKeyNumber(const std::string& key) const
{
	if (key == "listen")
		return (LISTEN);
	else if (key == "server_name")
		return (SERVER_NAME);
	else if (key == "error_page")
		return (ERROR_PAGE);
	else
		return (NONE);
}

bool	Server::setMemberData(void)
{
	for (std::map<std::string, std::string>::const_iterator it = _data.begin();
		it != _data.end(); ++it)
	{
		Key what = _getKeyNumber(it->first);
		switch (what)
		{
		case LISTEN:
			_setListen(it->second);
			break ;
		case SERVER_NAME:
			_setServerName(it->second);
			break ;
		case ERROR_PAGE:
			_setErrorPage(it->second);
			break ;
		default:
			return (false);
		}
	}
	return (true);
}

void	Server::_setListen(const std::string& value)
{
	std::string tmpValue = value;
	if (tmpValue.find(" ") != std::string::npos)
	{
		std::pair<std::string, std::string> splitedValue = splitString(tmpValue, " ");
		_port = atoi(splitedValue.first.c_str());
		_host = splitedValue.second;
	}
	else
	{
		_port = atoi(tmpValue.c_str());
	}
}

void	Server::_setServerName(const std::string& value)
{
	_serverName = value;
}

void	Server::_setErrorPage(const std::string& value)
{
	_errorPage = value;
}

void	Server::print(void)
{
	std::cout << "###################Server####################\n";
	std::cout << "_host: " << _host << std::endl;
	std::cout << "_port: " << _port << std::endl;
	std::cout << "_serverName: " << _serverName << std::endl;
	std::cout << "_errorPage: " << _errorPage << std::endl;
	for (std::vector<Location>::iterator it = _locations.begin(); it != _locations.end(); ++it)
	{
		it->print();
	}
	std::cout << "\n#########################################\n\n";
}

void	Server::setAddress(void)
{
	// listensocket 할당.???
	// sAddr 초기화
	memset(&_sAddr, 0, sizeof(_sAddr));
	_sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_sAddr.sin_family = AF_INET;
	_sAddr.sin_port = htons(_port);
}

int	Server::getListenSocket() const
{
	return _listenSocket;
}

void	Server::setListenSocket()
{
	_listenSocket = socket(PF_INET, SOCK_STREAM, 0);
}
int	Server::bindSelf()
{
	return bind(_listenSocket, (struct sockaddr *)&_sAddr, sizeof(_sAddr));
}

int	Server::listenSelf(int backLog)
{
	return listen(_listenSocket, backLog);
}

// server의 accept를 진행하는 부분이다.
// 새로 받은 socket을 리턴한다.
int	Server::acceptSocket(fd_set *readSet, fd_set *writeSet)
{
	int len;
	int tmpSock = accept(_listenSocket, (struct sockaddr *) &_cAddr, (socklen_t *)&len);
	fcntl(tmpSock, F_SETFL, O_NONBLOCK);
	_sockets.push_back(Socket(tmpSock));
	FD_SET(tmpSock, readSet);
	FD_SET(tmpSock, writeSet);
	return tmpSock;
}

// server의 client소켓을 확인해보자.
void	Server::checkSet(fd_set *readSet, fd_set *writeSet, fd_set *copyr, fd_set *copyw)
{
	// iterater 너무 길어!!!!!!!!
	std::vector<Socket>::iterator iter = _sockets.begin();
	std::vector<Socket>::iterator endIter = _sockets.end();
	for (; iter != endIter; iter++)
	{
		if (FD_ISSET(iter->getSocketFd(), copyr))
		{
			if (_checkReadSetAndExit(iter, readSet, writeSet))
				continue ;
		}
		// write하는 부분
		// send버퍼를 사용할 수 있고 버퍼에 어떠한 문자가 있는지 확인해본다.
		// 원래 따로 만드려고 했는데 (역할에 따라 함수를 분리하기 위해) 그러면 socket을 한번 더 돌아야 하기 때문에(비효율적) 합쳤다.
		else if (FD_ISSET(iter->getSocketFd(), copyw) && iter->getReadChecker() == true)
			_checkWriteSet(iter);
	}
}

int	Server::_checkReadSetAndExit(std::vector<Socket>::iterator iter, fd_set *readSet, fd_set *writeSet)
{
	char	buff[MAXBUFF];
	int	n;
	if ((n = read(iter->getSocketFd(), buff, sizeof(buff))) != 0)
	{
		std::cout << "read!!\n";
		buff[n] = '\0';
		iter->addStringToBuff(buff);
		iter->setReadChecker(true);
		return 0;
	}
	else
	{
		std::cout << "break!!\n";
		std::cout << "EOF\n";
		FD_CLR(iter->getSocketFd(), readSet);
		FD_CLR(iter->getSocketFd(), writeSet);
		close(iter->getSocketFd());
		_sockets.erase(iter);
		return 1;
	}
}


void	Server::_checkWriteSet(std::vector<Socket>::iterator iter)
{
	// Request request(iter->getBuffer());
	// request.parseRequest();
	// std::cout << iter->getBuffer();
	std::cout << "write!!\n";
	write(iter->getSocketFd(), iter->getBuffer().c_str(), iter->getBuffer().size() + 1);
	iter->setReadChecker(false);
	iter->clearBuffer();
}
