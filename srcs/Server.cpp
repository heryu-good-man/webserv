#include "Server.hpp"

const std::string		DEFAULT_HOST = "localhost";		// 변경 필요
const size_t			DEFAULT_PORT = 8080;
const std::string		DEFAULT_SERVER_NAME = "localhost";

Server::Server()
	: _listenSocket()
	, _sAddr()
	, _cAddr()
	, _sockets()
	, _locations()
	, _host(DEFAULT_HOST)
	, _port(DEFAULT_PORT)
	, _serverName(DEFAULT_SERVER_NAME)
	, _errorPages()
{

}

Server::Server(const Server& other)
	: _listenSocket(other._listenSocket)
	, _sAddr(other._sAddr)
	, _cAddr(other._cAddr)
	, _sockets(other._sockets)
	, _locations(other._locations)
	, _host(other._host)
	, _port(other._port)
	, _serverName(other._serverName)
	, _errorPages(other._errorPages)
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
		_locations = rhs._locations;
		_host = rhs._host;
		_port = rhs._port;
		_serverName = rhs._serverName;
		_errorPages = rhs._errorPages;
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

const std::string&				Server::getErrorPage(int code) const
{
	static const std::string none = "";
	std::map<std::string, std::string>::const_iterator it = _errorPages.find(std::to_string(code));
	if (it != _errorPages.end())
		return (it->second);
	else
		return (none);
}

size_t							Server::getPort(void) const
{
	return (_port);
}

const std::string&				Server::getHost(void) const
{
	return (_host);
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
	std::pair<std::string, std::string> p = splitString(value, " ");

	_errorPages[p.first] = p.second;
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
void	Server::acceptSocket(void)
{
	int len;
	int tmpSock = accept(_listenSocket, (struct sockaddr *) &_cAddr, (socklen_t *)&len);
	FDManager::instance().setFD(tmpSock, true, true);
	_sockets.push_back(Socket(tmpSock));
	std::cout << "accept socket fd: " << tmpSock << std::endl;
}

void	Server::checkSet(fd_set *copyr, fd_set *copyw)
{
	std::vector<Socket>::iterator iter = _sockets.begin();
	// int i = 0;
	for (; iter != _sockets.end(); iter++)
	{
		// std::cout << i++ << "번째 소켓!!\n";
		if (FD_ISSET(iter->getSocketFd(), copyr))
		{
			if (_checkReadSetAndExit(iter))
			{
				_sockets.erase(iter--);
				continue ;
			}
		}
		else if (FD_ISSET(iter->getSocketFd(), copyw) && iter->getReadChecker() == true)
		{
			if (_checkWriteSet(iter))
			{
				_sockets.erase(iter--);
				continue ;
			}
		}
	}
}

// priavate
int	Server::_socketDisconnect(std::vector<Socket>::iterator& iter)
{
	// 소켓연결해제
	FDManager::instance().unsetFD(iter->getSocketFd(), true, true);
	return 1;
}

void	Server::_setReadEnd(std::vector<Socket>::iterator& iter)
{
	std::string method = iter->getRequest().getStartLine()[0];
	if (method == "POST" || method == "PUT")
	{
		std::map<std::string, std::string>::const_iterator reqEnd = iter->getRequest().getHeaders().end();
		std::map<std::string, std::string>::const_iterator lenIter = iter->getRequest().getHeaders().find("Content-Length");
		std::map<std::string, std::string>::const_iterator encodingIter = iter->getRequest().getHeaders().find("Transfer-Encoding");
		int n = 0;
		if (lenIter != reqEnd && encodingIter != reqEnd)
			throw 400;
		// content-length 확인
		else if (lenIter != reqEnd)
		{
			char buff[MAXBUFF + 1];
			std::stringstream contentSize(lenIter->second);
			contentSize >> n;
			if (contentSize.fail())
				throw 400;
			iter->setBodyLen(n);
			// 여기 없애야 할거 같은데 나중에 생각하자
			if ((n = read(iter->getSocketFd(), buff, MAXBUFF)) != 0)
			{
				buff[n] = '\0';
				iter->addStringToBuff(buff);
				// 내가 원하는 만큼 버퍼에 가득 찼다
				std::cout << "buffer size : " << iter->getBuffer().size() << std::endl;
				std::cout << "startindex : " << iter->getStartIndex() << std::endl;
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

int	Server::_checkReadSetAndExit(std::vector<Socket>::iterator& iter)
{
	// char	*buff = new char[MAXBUFF + 1];
	char	buff[MAXBUFF + 1];
	int		n;
	int		pos = 0;

	// if ((n = read(iter->getSocketFd(), buff, MAXBUFF)) != 0)
	if ((n = read(iter->getSocketFd(), buff, MAXBUFF)) != 0)
	{
		try
		{
			if (n == -1)
				return _socketDisconnect(iter);
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
		_socketDisconnect(iter);
		// delete[] buff;
		return 1;
	}
}

int		Server::_checkWriteSet(std::vector<Socket>::iterator& iter)
{
	std::cout << "fd: " << iter->getSocketFd() << ": condition: " << FDManager::instance().getConditionBySocket(iter->getSocketFd()) << std::endl;
	iter->setRequestChecker(false);
	int condition = FDManager::instance().getConditionBySocket(iter->getSocketFd());
	if (condition == NOT_SET)
	{
		Request request(iter->getBuffer());
		request.parseRequest();

		iter->getResponse().setSocketNum(iter->getSocketFd());
		iter->getResponse().response(*this, request);
		if (FDManager::instance().getConditionBySocket(iter->getSocketFd()) == NOT_SET)
		{
			condition = ENABLE_WRITE;
			FDManager::instance().setConditionBySocket(iter->getSocketFd(), ENABLE_WRITE);
		}
	}
	if (condition == CGI_READ)
	{
		Request request(iter->getBuffer());
		request.parseRequest();

		iter->getResponse().response(*this, request);
	}
	if (condition == SET)
	{
		Request request(iter->getBuffer());
		request.parseRequest();

		iter->getResponse().response(*this, request);

		condition = ENABLE_WRITE;
		FDManager::instance().setConditionBySocket(iter->getSocketFd(), ENABLE_WRITE);
	}
	if (condition == ENABLE_WRITE)
	{
		// write
		const std::string& message = iter->getResponse().getMessage();
		size_t writtenSize = iter->getResponse().getWrittenSize();
		size_t restSize = message.size() - writtenSize;
		size_t writeSize = restSize < 65530 ? restSize : 65530;

		int ret = write(iter->getSocketFd(), message.c_str() + writtenSize, writeSize);
		if (ret == -1)
		{
			FDManager::instance().unsetFD(iter->getSocketFd(), true, true);
			throw std::runtime_error("Response write error");
		}
		iter->getResponse().setWrittenSize(writtenSize + ret);
		if (iter->getResponse().getWrittenSize() == message.size())
		{
			condition = NOT_SET;
			FDManager::instance().setConditionBySocket(iter->getSocketFd(), NOT_SET);
			iter->getResponse().setWrittenSize(0);

			iter->setReadChecker(false);
			iter->setStartIndex(0);
			iter->clearBuffer();
		}
	}
	return 0;
}
