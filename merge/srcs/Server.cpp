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

// server의 client소켓을 확인해보자.
void	Server::checkSet(fd_set *readSet, fd_set *writeSet, fd_set *copyr, fd_set *copyw)
{
	// iterater 너무 길어!!!!!!!!
	std::vector<Socket>::iterator iter = _sockets.begin();
	for (; iter != _sockets.end(); iter++)
	{
		if (FD_ISSET(iter->getSocketFd(), copyr))
		{
			if (_checkReadSetAndExit(iter, readSet, writeSet))
			{
				_sockets.erase(iter--);
				continue ;
			}
		}
		// write하는 부분
		// send버퍼를 사용할 수 있고 버퍼에 어떠한 문자가 있는지 확인해본다.
		// 원래 따로 만드려고 했는데 (역할에 따라 함수를 분리하기 위해) 그러면 socket을 한번 더 돌아야 하기 때문에(비효율적) 합쳤다.
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

// 정리해주실분....
void	Server::_setReadEnd(std::vector<Socket>::iterator iter, size_t pos)
{
	Request request(iter->getBuffer());

	request.parseRequest();
	std::string method = request.getStartLine()[0];
	// ㅇ겨ㅣ기는 감
	if (method == "POST" || method == "PUT")
	{
		std::map<std::string, std::string> requestHeader = request.getHeaders();
		std::map<std::string, std::string>::const_iterator reqEnd = requestHeader.end();
		std::map<std::string, std::string>::const_iterator lenIter = requestHeader.find("Content-Length");
		std::map<std::string, std::string>::const_iterator encodingIter = requestHeader.find("Transfer-Encoding");
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
			while ((n = read(iter->getSocketFd(), buff, sizeof(buff))) != 0)
			{
				buff[n] = '\0';
				iter->addStringToBuff(buff);
				// 내가 원하는 만큼 버퍼에 가득 찼다
				if (iter->getBuffer().size() - (pos + 4) <= static_cast<size_t>(iter->getBodyLen()))
				{
					request.parseBody();
					iter->setReadChecker(true);
					break ;
				}
			}
		}
		else if (encodingIter != reqEnd)
		{
			// chunked 체크해야한다.
			// 여기서 바로 읽어버리자.
			if ((n = iter->getBuffer().find("\r\n\r\n", pos + 4)) != -1)
			{
				// 딱 청크드 데이터만 있음
				if (!iter->getChunkedBuff().empty())
					iter->setChunkedBuff(iter->getBuffer().substr(pos + 4, n));
				int endIndex = 0;
				while (1)
				{
					std::string oneLine;
					endIndex = iter->getChunkedBuff().find("\r\n", iter->getStartIndex()) - iter->getStartIndex();
					// \r\n이 없다?? 그럼 
					if (endIndex == -1)
						break ;
					// 한줄 자르고
					int	chunckSize;
					std::stringstream ss2(oneLine);
					ss2 << std::hex << oneLine;
					ss2 >> chunckSize;
					std::cout << "chunckSize: " << chunckSize << std::endl;
					if (ss2.fail())
						throw 400;
					// 일단 숫자 0다음에 \r\n을 한번만 받아도 종료되게끔 만들겠습니다.
					if (chunckSize == 0)
					{
						iter->setReadChecker(true);
						iter->clearChunkBuffer();
						return ;
					}
					// endIndex에 2를 더해야 읽은 문자 갯수에 \r\n을 포함가능하다. 그리고 세어야할 문자열 뒤에 \r\n은 빼고 세어야 하기 때문에 -2를 해줬다.
					// 더 읽어야함.
					if (chunckSize > static_cast<int>(iter->getChunkedBuff().size()) - (endIndex + 2) - 2)
						break ;
					// 다 읽었으면 chunkSize만큼 잘라내서 버퍼에 저장하자.
					iter->addStringToBuff(iter->getChunkedBuff().substr(endIndex + 2, chunckSize));
					// startindex잡는건 한줄 읽고 나서.
					iter->setStartIndex(endIndex + 2);
				}
			}
		}
	}
	// POST, PUT외에 다른 method가 들어왔을 경우...
	else if (method == "GET" || method == "DELETE" || method == "HEAD")
		iter->setReadChecker(true);
	else
		throw 405;
}


int	Server::_checkReadSetAndExit(std::vector<Socket>::iterator iter, fd_set *readSet, fd_set *writeSet)
{
	char	buff[MAXBUFF];
	int		n;
	int		pos;
	
	if ((n = read(iter->getSocketFd(), buff, sizeof(buff))) != 0)
	{
		try
		{
			if (n == -1)
				return _socketDisconnect(iter, readSet, writeSet);
			std::cout << "read!!\n";
			buff[n] = '\0';
			iter->addStringToBuff(buff);
			// \r\n\r\n이 있는지 확인
			if ((pos = iter->getBuffer().find("\r\n\r\n")) != -1)
			{
				_setReadEnd(iter, pos);
			}
			return 0;
		}
		catch(int code)
		{
			std::cout << "err code : " << code << std::endl;
			iter->clearBuffer();
			iter->clearChunkBuffer();
			return 0;
		}
	}
	else
	{
		std::cout << "break!!\n";
		_socketDisconnect(iter, readSet, writeSet);
		return 1;
	}
}


int		Server::_checkWriteSet(std::vector<Socket>::iterator iter, fd_set *readSet, fd_set *writeSet)
{
	std::cout << "write!!\n";

	Request request(iter->getBuffer());
	if (iter->getBuffer().empty())
		return 0;
	// request.parseRequest();를 언제 해야할까??
	request.parseRequest();
	Response tmp;
	tmp.response(*this, request);
	if (write(iter->getSocketFd(), iter->getBuffer().c_str(), iter->getBuffer().size() + 1) == -1)
		return _socketDisconnect(iter, readSet, writeSet);
	// char buf[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: 100\r\nDate: Sun, 13 Jun 2021\r\n\r\nHello World AAA!!!\r\n";
	// write(iter->getSocketFd(), buf, strlen(buf));
	iter->setReadChecker(false);
	// 잠깐 추가. 나중에 소켓 초기화하는 함수를 따로 만들던가 해야지 원...
	iter->setStartIndex(0);
	iter->clearBuffer();
	return 0;
}
