#include "Request.hpp"

Request::Request()
{

}

Request::Request(const std::string& message): _message(message),  _queryString(""), _cgi_extension(""), _badRequset(false)
{
	size_t CRLF = _message.find("\r\n\r\n");
	_body = _message.substr(CRLF + 4);
	_message = _message.substr(0, CRLF + 4);
	// if (_message.substr(0, 3) != "GET")
		// std::cout << "_message:\n" << _message.substr(0, 20) << std::endl;
}

void Request::parseRequest()
{
	try
	{
		parseStartLine();
		parseHeader();
		// parseBody();
	}
	catch(int)
	{
		_badRequset = true;
	}
	catch(std::exception &e)
	{
		// std::cout << "error in parse Request" << std::endl;
	}

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
	// 재할당 수정@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
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
		if (_startLine[i] == "")
			throw BADREQUEST;
	}

	size_t posCGI;
	if ((posCGI = _startLine[1].find(".bla")) != _startLine[1].npos)
		_cgi_extension = std::string(".bla");
	else if ((posCGI = _startLine[1].find(".php")) != _startLine[1].npos)
		_cgi_extension = std::string(".php");

	if (_startLine[1].find("?") != _startLine[1].npos)
		_queryString = _startLine[1].substr(_startLine[1].find("?") + 1);
}

void Request::parseHeader()
{
	size_t pos;
	std::string headerLine;
	size_t posColon;
	std::string headerKey;
	std::string headerValue;

	if (_message == "")
		return ;
	while (1)
	{
		pos = _message.find("\r\n");
		// headerLine;
		headerLine = _message.substr(0, pos);
		// 재할당 수정@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		_message = _message.substr(pos + 2);
		// headerLine을 Key와 Value로 스플릿
		posColon = headerLine.find(":");
		// std::cout << "headerLine: " << headerLine << std::endl;
		if (posColon == headerLine.npos)
		{
			if (!headerLine.empty())
			{
				// std::cout << "*************************" << headerLine << std::endl;
				throw BADREQUEST;
			}
			break ;
		}
		headerKey = headerLine.substr(0, posColon);
		if (headerLine[posColon + 1] == ' ')
			posColon += 1;
		headerValue = headerLine.substr(posColon + 1);
		_headers.insert(std::pair<std::string, std::string>(headerKey, headerValue));
	}
}

void Request::parseBody()
{
	// if (_headers.find("content-length") != _headers.end())
	// {
	// 	_body = _message.substr(atoi(_headers["content-length"].c_str()));
	// }
	// else if (_headers.find("transfer-enconding") != _headers.end())
	// {
	// 	std::string chunkSizeStr;
	// 	std::string chunkedBody;
	// 	long int chunkSize;
	// 	size_t pos;
	// 	while (1)
	// 	{
	// 		pos = _message.find("\r\n");
	// 		chunkSizeStr = _message.substr(0, pos);
	// 		chunkSize = strtol(chunkSizeStr.c_str(), NULL, 16);
	// 		if (chunkSize != 0)
	// 		{
	// 			_message = _message.substr(pos + 2);
	// 			chunkedBody = _message.substr(0, chunkSize);
	// 			_body += chunkedBody;
	// 			_message = _message.substr(chunkSize + 1);
	// 		}
	// 		else
	// 			break ;
	// 	}
	// }
	// _body = _message;
}

const std::string* Request::getStartLine(void) const
{
	return _startLine;
}

const std::map<std::string, std::string>& Request::getHeaders(void) const
{
	return _headers;
}

const std::string& Request::getBody(void) const
{
	return _body;
}