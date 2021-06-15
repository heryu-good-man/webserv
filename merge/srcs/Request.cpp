#include "Request.hpp"

Request::Request(std::string message): _message(message)
{
}

void Request::parseRequest()
{
	if (_message == "")
		return ;
	parseStartLine();
	parseHeader();
	parseBody();

	// // 출력해보기
	// std::cout << "Startline\n";
	// std::cout << "Method: " << _startLine[0] << "$" << std::endl;
	// std::cout << "Location: " << _startLine[1] << std::endl;
	// std::cout << "Vesrion: " << _startLine[2] << std::endl;
	// // 출력해보기
	// std::cout << "Headers\n";
	// for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
	// 	std::cout << it->first << " : " << it->second << std::endl;
	// // 출력해보기
	// std::cout << "Body\n";
	// std::cout << _body << std::endl;
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
		if (pos != startLine.npos)
		{
			_startLine[i] = startLine.substr(0, pos);
			startLine = startLine.substr(pos + 1);
		}
		else
			_startLine[i] = startLine;
	}
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
		// headerLine을 Key와 Value로 스플릿
		posColon = headerLine.find(":");
		if (posColon == headerLine.npos)
			break ;
		headerKey = headerLine.substr(0, posColon);
		if (headerLine[posColon + 1] == ' ')
			posColon += 1;
		headerValue = headerLine.substr(posColon + 1);
		_headers.insert(std::pair<std::string, std::string>(headerKey, headerValue));
	}
}

void Request::parseBody()
{
	if (_headers.find("content-length") != _headers.end())
	{
		_body = _message.substr(atoi(_headers["content-length"].c_str()));
	}
	else if (_headers.find("transfer-enconding") != _headers.end())
	{
		std::string chunkSizeStr;
		std::string chunkedBody;
		long int chunkSize;
		size_t pos;
		while (1)
		{
			pos = _message.find("\r\n");
			chunkSizeStr = _message.substr(0, pos);
			chunkSize = strtol(chunkSizeStr.c_str(), NULL, 16);
			if (chunkSize != 0)
			{
				_message = _message.substr(pos + 2);
				chunkedBody = _message.substr(0, chunkSize);
				_body += chunkedBody;
				_message = _message.substr(chunkSize + 1);
			}
			else
				break ;
		}
	}
}

std::string* Request::getStartLine(void)
{
	return _startLine;
}

std::map<std::string, std::string> Request::getHeaders(void)
{
	return _headers;
}

std::string Request::getBody(void)
{
	return _body;
}