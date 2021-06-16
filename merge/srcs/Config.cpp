#include "Config.hpp"

Config::Config()
	: _servers()
{

}

Config::Config(const Config& other)
	: _servers(other._servers)
{

}

Config::~Config()
{

}

Config&	Config::operator=(const Config& rhs)
{
	if (this != &rhs)
	{
		_servers = rhs._servers;
	}
	return (*this);
}

Server	Config::getServer(size_t index) const
{
	if (0 <= index && index < _servers.size())
		return (_servers[index]);
	else
		throw std::runtime_error("index error: out_of_index by getServer");
}

std::vector<Server>	Config::getServers(void) const
{
	return (_servers);
}

/*
** config 파일을 파싱함
** config -> Server -> Location
*/
void	Config::parseConfig(const char* fileName)
{
	std::string fileContent = _getFileContent(fileName);

	size_t i = 0;
	while (1)
	{
		while (isSpace(fileContent[i])) // skip space
			i++;
		if (!fileContent[i]) // break condition
			break ;
		if (fileContent.compare(i, SERVER_STR_LENGTH, "server") == 0)
		{
			size_t serverContentSize = _getServerContentSize(fileContent, i);
			std::string serverContent = fileContent.substr(i, serverContentSize);

			// parse contents "server{...}"
			_servers.push_back(_parseServer(serverContent));
			i += serverContentSize;
		}
		else
			throw std::runtime_error("config error: is not \"server\"");
	}

	if (_isValidConfig() == false)
		throw std::runtime_error("config error: invalid configuration");
}

/*
** server {...}를 파싱함
** server123 {...} 이런 식이면 에러처리
** content로 location {...}이 들어올 때,
** content로 key value;가 들어올 때로 크게 나눔
*/
Server	Config::_parseServer(const std::string& content)
{
	Server newServer;

	size_t i = SERVER_STR_LENGTH;
	while (content[i] != '{')
	{
		if (!isSpace(content[i]))
			throw std::runtime_error("config error: don't \"server@!#@!#\"");
		i++;
	}

	size_t curPos = i + 1;
	while (1)
	{
		while(isSpace(content[curPos])) // skip space
			curPos++;
		if (!content[curPos] || content[curPos] == '}') // break condition
			break ;

		if (content.compare(curPos, LOCATION_STR_LENGTH, "location") == 0) // if location / {...}
		{
			size_t openPos = content.find("{", curPos);
			size_t closePos = content.find("}", curPos);
			if (openPos == std::string::npos || closePos == std::string::npos) // if invalid {} pair
				throw std::runtime_error("config error: invalid location parentheses");

			std::string locationContent = content.substr(curPos, closePos + 1 - curPos); // location {...}
			newServer._locations.push_back(_parseLocation(locationContent));
			curPos = closePos + 1;
		}
		else
		{
			size_t closePos = content.find(";", curPos); // key value ";"
			if (closePos == std::string::npos)
				throw std::runtime_error("config error: invalid semicolon in server");

			// get key-value by splitString
			std::string dataLine = content.substr(curPos, closePos - curPos);
			if (dataLine.find(" ") == std::string::npos)
				throw std::runtime_error("config error: No space key\" \"value");

			std::pair<std::string, std::string> splitedLine = splitString(dataLine, " ");
			std::string key = splitedLine.first;
			std::string value = splitedLine.second;
			newServer._data[key] = value;
			curPos = closePos + 1;
		}
	}

	if (newServer.setMemberData() == false)
		throw std::runtime_error("config error: invalid Server's key");
	return (newServer);
}

/*
** location {...}을 파싱함
** key value;에만 신경씀
*/
Location	Config::_parseLocation(const std::string& content)
{
	Location newLocation;
	// set location Path
	newLocation._path = _getLocationPath(content);

	// set location value
	size_t	curPos = content.find("{") + 1;
	while (1)
	{
		while (isSpace(content[curPos]))
			curPos++;
		if (!content[curPos] || content[curPos] == '}')
			break ;

		size_t endPos = content.find(";", curPos); // key value ";"
		if (endPos == std::string::npos)
			throw std::runtime_error("config error: invalid semicolon in location");

		// get key-value by splitString
		std::string dataLine = content.substr(curPos, endPos - curPos);
		std::pair<std::string, std::string> splitedLine = splitString(dataLine, " ");
		std::string key = splitedLine.first;
		std::string value = splitedLine.second;
		newLocation._data[key] = value;

		curPos = endPos + 1;
	}

	if (newLocation.setMemberData() == false)
		throw std::runtime_error("config error: invalid location's key");
	return (newLocation);
}

/*
** 파일내용을 한 줄씩 읽어와서 주석과 양옆 공백을 제거한 후 합쳐서 string으로 리턴
*/
std::string	Config::_getFileContent(const char* fileName) const
{
	std::ifstream ifs;
	ifs.open(fileName, std::ios_base::in);
	if (!ifs.is_open())
		throw std::runtime_error("file open error: invalid file name");

	std::ostringstream oss;
	std::string fileLine;
	while (1)
	{
		std::getline(ifs, fileLine);
		if (ifs.fail())
		{
			ifs.clear();
			break ;
		}
		deleteComments(fileLine);			// 주석 제거
		removeContinuousSpace(fileLine);	// 공백은 무조건 1개로
		trimSpace(fileLine);				// 양쪽 공백 제거
		if (fileLine != "")					// 빈문자열 아니면 넣기
			oss << fileLine << " ";
	}
	ifs.close();
	return (oss.str());
}

/*
** server {}에서 길이를 구함
*/
size_t	Config::_getServerContentSize(const std::string& content, size_t startIndex) const
{
	size_t serverContentSize = SIZE_T_MAX;
	size_t contentSize = content.size();
	std::stack<bool> parenthesesChecker;
	for (size_t i = startIndex; i < contentSize; ++i)
	{
		if (content[i] == '{')
			parenthesesChecker.push(true);					// 여는 괄호면 푸쉬 해놓기
		else if (content[i] == '}')							// 닫는 괄호일 때 판단하기
		{
			if (parenthesesChecker.empty())					// 닫는 괄호인데 쌍에 맞는 괄호가 없다면
				break ;
			else
			{
				parenthesesChecker.pop();					// 비어있지는 않음
				if (parenthesesChecker.empty())				// 닫는 괄호가 짝이 맞았다면
				{
					serverContentSize = i - startIndex + 1; // server {} 닫힌 것
					break ;
				}
			}
		}
	}

	if (serverContentSize == SIZE_T_MAX)
		throw std::runtime_error("config error: invalid parentheses");
	else
		return (serverContentSize);
}

/*
** location의 path를 리턴
** 중간에 invalid하면 예외던지기
*/
std::string	Config::_getLocationPath(const std::string& content) const
{
	if (!isSpace(content[LOCATION_STR_LENGTH])) // location123 {...} 식으로 들어온 경우
		throw std::runtime_error("config error: don't \"location@!#@!#\"");

	std::string locationPath;
	locationPath = content.substr(LOCATION_STR_LENGTH, content.find("{") - LOCATION_STR_LENGTH);
	trimSpace(locationPath);

	if (locationPath.size() == 0) // location {} 으로 바로 들어왔을 경우
		throw std::runtime_error("config error: need a location path");
	size_t i = 0;
	while (locationPath[i])
	{
		if (locationPath[i] == ' ') // space가 있을 경우
			throw std::runtime_error("config error: there is space in location path");
		i++;
	}
	if (locationPath.back() != '/')
		locationPath += "/";
	return (locationPath);
}

bool	Config::_isValidConfig(void) const
{
	// No server
	size_t serverCount = _servers.size();
	if (serverCount == 0)
		return (false);

	// duplicated Checker in Servers
	std::map<size_t, int> portChecker;
	std::map<std::string, int> hostChecker;
	std::map<std::string, int> nameChecker;
	for (size_t i = 0; i < serverCount; ++i)
	{
		if (portChecker[_servers[i]._port] == 0)		// port 중복
			portChecker[_servers[i]._port] = 1;
		else
			return (false);

		if (hostChecker[_servers[i]._host] == 0)		// host 중복
			hostChecker[_servers[i]._host] = 1;
		else
			return (false);

		if (nameChecker[_servers[i]._serverName] == 0)	// server name 중복
			nameChecker[_servers[i]._serverName] = 1;
		else
			return (false);

		// No Location
		size_t locationCount = _servers[i]._locations.size();
		if (locationCount == 0)
			return (false);

		// duplicated path Checker in Locations
		std::map<std::string, int> locationPathChecker;
		for (size_t j = 0; j < locationCount; ++j)
		{
			if (locationPathChecker[_servers[i]._locations[j]._path] == 0)
				locationPathChecker[_servers[i]._locations[j]._path] = 1;
			else
				return (false);
		}
	}
	return (true);
}
