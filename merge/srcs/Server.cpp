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

const Location&	Server::getLocation(size_t index) const
{
	if (0 <= index && index < _locations.size())
		return (_locations[index]);
	else
		throw std::runtime_error("index error: out_of_index by getServer");
}

const std::vector<Location>&	Server::getLocations(void) const
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

void	Server::checkSet(fd_set *readSet, fd_set *writeSet, fd_set *copyr, fd_set *copyw)
{
	std::vector<Socket>::iterator iter = _sockets.begin();
	// int i = 0;
	for (; iter != _sockets.end(); iter++)
	{
		// std::cout << i++ << "번째 소켓!!\n";
		if (FD_ISSET(iter->getSocketFd(), copyr))
		{
			if (_checkReadSetAndExit(iter, readSet, writeSet))
			{
				_sockets.erase(iter--);
				continue ;
			}
		}
		else if (FD_ISSET(iter->getSocketFd(), copyw) && iter->getReadChecker() == true)
		{
			if (_checkWriteSet(iter, readSet, writeSet))
			{
				_sockets.erase(iter--);
				continue ;
			}
		}
	}
}

// priavate
int	Server::_socketDisconnect(std::vector<Socket>::iterator iter, fd_set *readSet, fd_set *writeSet)
{
	// 소켓연결해제
	FD_CLR(iter->getSocketFd(), readSet);
	FD_CLR(iter->getSocketFd(), writeSet);
	close(iter->getSocketFd());
	return 1;
}

void	Server::_setReadEnd(std::vector<Socket>::iterator iter)
{
	std::string method = iter->getRequest().getStartLine()[0];
	if (method == "POST" || method == "PUT")
	{
		std::map<std::string, std::string>::const_iterator reqEnd = iter->getRequest().getHeaders().end();
		std::map<std::string, std::string>::const_iterator lenIter = iter->getRequest().getHeaders().find("Content-Length");
		std::map<std::string, std::string>::const_iterator encodingIter = iter->getRequest().getHeaders().find("Transfer-Encoding");
		char buff[MAXBUFF];
		std::stringstream ss(lenIter->second);
		int n;
		if (lenIter != reqEnd && encodingIter != reqEnd)
			throw 400;
		// content-length 확인
		else if (lenIter != reqEnd)
		{
			ss >> n;
			if (ss.fail())
				throw 400;
			iter->setBodyLen(n);
			// 여기 없애야 할거 같은데 나중에 생각하자
			if ((n = read(iter->getSocketFd(), buff, sizeof(buff))) != 0)
			{
				buff[n] = '\0';
				iter->addStringToBuff(buff);
				// 내가 원하는 만큼 버퍼에 가득 찼다
				if (iter->getBuffer().size() - (iter->getStartIndex()) <= static_cast<size_t>(iter->getBodyLen()))
					iter->setReadChecker(true);
			}
		}
		else if (encodingIter != reqEnd)
		{
			// chunked 체크해야한다.
			// 여기서 바로 읽어버리자.
			while ((n = iter->getBuffer().find("\r\n", iter->getStartIndex())) != -1)
			{
				std::string sizeLine = "";
				int chunkSizeStart = iter->getStartIndex();
				sizeLine = iter->getBuffer().substr(chunkSizeStart, n - chunkSizeStart);
				std::stringstream ss;
				ss << std::hex << sizeLine;
				int chunkSize;
				ss >> chunkSize;
				if (sizeLine != "0" && iter->getBuffer().find("\r\n", n + 2) != (size_t)-1)
				{
					iter->addChunkedBuff(iter->getBuffer().substr(n + 2, chunkSize));
					iter->setStartIndex(n + chunkSize + 4);
				}
				else if (sizeLine == "0" && iter->getBuffer().find("\r\n", n + 2) != (size_t)-1)
				{
					iter->setBuff(iter->getBuffer().substr(0, iter->getEndOfHeader()) + iter->getChunkedBuff());
					iter->setReadChecker(true);
					iter->setStartIndex(n + 4);
					iter->clearChunkBuffer();
					iter->setRequestChecker(false);
					break ;
				}
				else
					break ;
			}
		}
	}
	else if (method == "GET" || method == "DELETE" || method == "HEAD")
		iter->setReadChecker(true);
	else
		throw 405;
}

int	Server::_checkReadSetAndExit(std::vector<Socket>::iterator iter, fd_set *readSet, fd_set *writeSet)
{
	// char	*buff = new char[MAXBUFF + 1];
	char	buff[MAXBUFF + 1];
	int		n;
	int		pos = 0;

	// if ((n = read(iter->getSocketFd(), buff, MAXBUFF)) != 0)
	if ((n = read(iter->getSocketFd(), buff, sizeof(buff))) != 0)
	{
		try
		{
			if (n == -1)
				return _socketDisconnect(iter, readSet, writeSet);
			buff[n] = '\0';
			// std::cout << "2345" << std::endl;
			// std::cout << "real2 buf size: " << iter->getBuffer().size() << std::endl;
			iter->addStringToBuff(buff);
			// std::cout << "real3 buf size: " << iter->getBuffer().size() << std::endl;
			pos = iter->getBuffer().find("\r\n\r\n");
			if (pos != -1 && iter->getRequestChecker() == true)
			{
				// std::cout << "reading...\n";x
				_setReadEnd(iter);
				// delete[] buff;
			}
			else if (pos != -1 && iter->getRequestChecker() == false)
			{
				iter->setRequest(iter->getBuffer());
				iter->getRequest().parseRequest();
				// std::cout << "before read\n";
				iter->setRequestChecker(true);
				iter->setStartIndex(pos + 4);
				iter->setEndOfHeader(pos + 4);
				_setReadEnd(iter);
				// delete[] buff;
				// if (iter->getRequest().getMethod() == "POST" || iter->getRequest().getMethod() == "PUT")
			}
			return 0;
		}
		catch(int code)
		{
			std::cout << "err code : " << code << std::endl;
			iter->clearBuffer();
			iter->clearChunkBuffer();
			// delete[] buff;
			return 1;
		}
	}
	else
	{
		std::cout << "break!!\n";
		_socketDisconnect(iter, readSet, writeSet);
		// delete[] buff;
		return 1;
	}
}


int		Server::_checkWriteSet(std::vector<Socket>::iterator iter, fd_set *readSet, fd_set *writeSet)
{
	// std::cout << "after read\n";
	iter->setRequestChecker(false);
	// std::cout << "============================BUFFER=============================\n";
	try{
		Request request(iter->getBuffer());
		if (iter->getBuffer().empty())
			return 0;
		request.parseRequest();
		
		Response tmp;
		tmp.response(*this, request);
		// std::cout << "============================RESPONSE BUFFER=============================\n";
		// std::cout << tmp.getResponse() << std::endl;
		// std::cout << "========================================================\n";

		size_t rest = tmp.getResponse().size();
		size_t writtenSize = 0;
		while (rest > 0)
		{
			size_t writeSize = rest < 65530 ? rest : 65530;
			int tmpSize = 0;
			if ((tmpSize = write(iter->getSocketFd(), tmp.getResponse().c_str() + writtenSize, writeSize)) <= 0)
			{
				if (tmpSize == -1)
					continue ;
				// std::cout << *(tmp.getResponse().c_str() + writtenSize) << std::endl;
				// std::cout << tmpSize << ":" << writeSize << std::endl;
				// std::cout << errno << std::endl;
				// std::cout << "ssibal" << std::endl;
				// return _socketDisconnect(iter, readSet, writeSet);
				(void)readSet;
				(void)writeSet;
			}
			rest -= tmpSize;
			writtenSize += tmpSize;
		}
	}
	catch(std::exception& e)
	{
		std::cout << "error in checkWriteSet\n";
	}

	// if (write(iter->getSocketFd(), tmp.getResponse().c_str(), tmp.getResponse().size()) != (ssize_t)tmp.getResponse().size())
	// {
	// 	std::cout << "???" << std::endl;
	// 	return _socketDisconnect(iter, readSet, writeSet);
	// }
	// std::cout << "...end write1..." << std::endl;
	// char buf[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: 100\r\nDate: Sun, 13 Jun 2021\r\n\r\nHello World AAA!!!\r\n";
	// write(iter->getSocketFd(), buf, strlen(buf));
	iter->setReadChecker(false);
	// 잠깐 추가. 나중에 소켓 초기화하는 함수를 따로 만들던가 해야지 원...
	iter->setStartIndex(0);
	iter->clearBuffer();
	return 0;
}
