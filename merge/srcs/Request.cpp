#include "Request.hpp"

Request::Request(std::string message): _message(message) {}

void Request::parseRequest()
{
	if (_message == "")
		return ;
	parseStartLine();
	parseHeader();
	parseBody();
}

void Request::parseStartLine()
{
	size_t pos;
	std::string startLine;

	pos = _message.find("\r\n");
	startLine = _message.substr(0, pos);
	_message = _message.substr(pos + 2);
	// startLine을 공백 단위로 스플릿
	for (size_t i = 0; i < 3; i++)
	{
		pos = startLine.find(" ");
		_startLine[i] = startLine.substr(0, pos);
		startLine = startLine.substr(pos + 1);
	}

	// 출력해보기
	// std::cout << "Startline\n";
	// std::cout << "Method: " << _startLine[0] << std::endl;
	// std::cout << "Location: " << _startLine[1] << std::endl;
	// std::cout << "Vesrion: " << _startLine[2] << std::endl<< std::endl;
}

void Request::parseHeader()
{
	size_t pos;
	std::string headerLine;
	size_t posColon;
	std::string headerKey;
	std::string headerValue;

	while (1)
	{
		pos = _message.find("\r\n");
		// headerLine;
		headerLine = _message.substr(0, pos);
		_message = _message.substr(pos + 2);
		posColon = headerLine.find(":");
		// headerLine을 Key와 Value로 스플릿
		headerKey = headerLine.substr(0, posColon);
		if (headerLine[posColon + 1] == ' ')
			posColon += 1;
		headerValue = headerLine.substr(posColon + 1);
		if (headerKey.size() != 0 && headerValue.size() != 0)
			_headers.insert(std::pair<std::string, std::string>(headerKey, headerValue));
		else
			break ;
	}

	// 출력해보기
	std::cout << "headers\n";
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
		std::cout << it->first << " : " << it->second << std::endl;
	std::cout << std::endl;
}

void Request::parseBody()
{
	if (_headers.find("content-length") == _headers.end())
	{
		_body = _message.substr(atoi(_headers["content-length"].c_str()));
	}
	else if (_headers.find("transfer-enconding") == _headers.end())
	{

	}
	else
	{
		std::cout << "empty body\n";
		return ;
	}
}