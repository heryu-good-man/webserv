#include "Request.hpp"

Request::Request()
{

}

Request::Request(const std::string& message): _message(message),  _queryString(""), _cgi_extension(""), _badRequset(false)
{
	size_t CRLF = _message.find("\r\n\r\n");
	_body = _message.substr(CRLF + 4);
	_message = _message.substr(0, CRLF + 4);
}

void Request::parseRequest()
{
	try
	{
		parseStartLine();
		parseHeader();
	}
	catch(int)
	{
		_badRequset = true;
	}
	catch(std::exception &e)
	{
		std::cout << "error in parse Request" << std::endl;
	}
}

void Request::parseStartLine()
{
	size_t pos;
	std::string startLine;

	pos = _message.find("\r\n");
	startLine = _message.substr(0, pos);
	_message = _message.substr(pos + 2);
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
		headerLine = _message.substr(0, pos);
		_message = _message.substr(pos + 2);
		posColon = headerLine.find(":");
		if (posColon == headerLine.npos)
		{
			if (!headerLine.empty())
			{
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

const std::string&	Request::getMethod(void) const
{
	return (getStartLine()[0]);
}

const std::string& Request::getURI(void) const
{
	return (getStartLine()[1]);
}

const std::string& Request::getHTTPVersion(void) const
{
	return (getStartLine()[2]);
}

const std::string& Request::getCGIextension(void) const
{
	return _cgi_extension;
}

const std::string& Request::getQueryString(void) const
{
	return _queryString;
}

bool Request::isBadRequest(void) const
{
	return _badRequset;
}

void	Request::addBody(std::string body)
{
	_body += body;
}