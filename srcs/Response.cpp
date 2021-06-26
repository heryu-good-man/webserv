#ifndef RESPONSE_COMPILE
# define RESPONSE_COMPILE
#endif

#include "Response.hpp"

Response::Response(void)
	: _ret()
	, _body()
	, _statusCode(200)
	, _condition(NOT_SET)
	, _writtenSize(0)
	, _socketNum(-1)
{

}

Response::Response(const Response &ref)
{
	if (this != &ref)
		*this = ref;
}

Response::~Response(void)
{

}

Response		&Response::operator=(const Response &ref)
{
	if (this != &ref)
	{
		_ret = ref._ret;
		_body = ref._body;
		_statusCode = ref._statusCode;
		_condition = ref._condition;
		_writtenSize = ref._writtenSize;
		_socketNum = ref._socketNum;
		_cgi = ref._cgi;
	}
	return *this;
}

void    Response::response(const Server& server, const Request& request)
{
	try
	{
		_ret = "";
		_statusCode = 200;
		_isValidHTTPVersion(request.getHTTPVersion());
		_location = _getMatchingLocation(server, request.getURI());
		_requestMethod = _isAllowedMethod(_location, request.getMethod());
		_path = _getRealPath(_location, request.getURI());
		_isCGI = _isCGIRequest(_location, request.getCGIextension(), _path);
		if (_location.getReturn() != "")
			_responseRedirect(_location);
		else if ((_requestMethod == "GET" || _requestMethod == "POST") && _isCGI == true)
			_responseWithCGI(_location, _path, request);
		else if (_requestMethod == "GET")
			_responseGET(_location, _path, request, true);
		else if (_requestMethod == "HEAD")
			_responseGET(_location, _path, request, false);
		else if (_requestMethod == "DELETE")
			_responseDELETE(_location, _path);
		else if (_requestMethod == "PUT" || _requestMethod == "POST")
			_responsePUTorPOST(_location, _path, request);
	}
	catch(int code)
	{
		_statusCode = code;
		std::cout << code << std::endl;
		_ret = makeErrorResponse(server, request.getMethod());
	}
	// std::cout << _statusCode << std::endl;
}

std::string	Response::makeErrorResponse(const Server& server, const std::string& method)
{
	setStatusMap();

	std::map<int, std::string>* StatusMap = getStatusMap();
	std::string statusText = StatusMap->find(_statusCode)->second;
	std::string status = std::to_string(_statusCode) + " " + statusText;
	std::string body;
	const std::string errorPagePath = server.getErrorPage(_statusCode);
	if (!errorPagePath.empty() && _getType(errorPagePath) == TYPE_FILE)
		body = _readFile(errorPagePath);
	else
		body = status;

	std::string ret = "HTTP/1.1 ";
	ret += status;
	ret += "\r\nContent-Type: text/html";
	ret += "\r\nContent-Length: ";
	ret += std::to_string(body.size());
	ret += "\r\n\r\n";
	if (method != "HEAD")
		ret += body;

	unsetStatusMap();
	return (ret);
}

std::string		Response::_readFile(const std::string& fileName)
{
	if (_condition == NOT_SET)
	{
		_fd = open(fileName.c_str(), O_RDONLY);
		if (_fd == -1)
			std::cout << "open error" << std::endl;
		FDManager::instance().addReadFileFD(_fd, this, false); // reading
	}
	else if (_condition == SET)
	{
		return (FDManager::instance().getResult(_fd));
	}
	return (std::string(""));
}

void	Response::_writeFile(const std::string& fileName, const Request& req)
{
	if (_condition == NOT_SET)
	{
		const std::string& method = req.getMethod();
		if (method == "PUT")
			_fd = open(fileName.c_str(), O_WRONLY | O_TRUNC, 0666);
		else if (method == "POST")
			_fd = open(fileName.c_str(), O_WRONLY | O_APPEND, 0666);
		FDManager::instance().addWriteFileFD(_fd, req.getBody(), this, false); // writing
	}
	else if (_condition == SET)
	{
		;
	}
}

void	Response::_responsePUTorPOST(const Location& location, const std::string& path, const Request& request)
{
	if (request.getBody().size() > location.getClientBodySize())
		throw 413;
	if (location.getUploadEnable() == false)
		throw 403;

	int type = _getType(path);
	if (type == TYPE_DIR) // dir
	{
		const std::vector<std::string>& indexPages = location.getIndexPages();
		if (indexPages.size() == 0)
			throw 500;
		else
		{
			std::string tmpPath = path;
			if (tmpPath.back() != '/')
				tmpPath += "/";
			if (_getType(tmpPath + indexPages[0]) == TYPE_NONE) // if there is no file, make new file
				_statusCode = 201;
			_writeFile(tmpPath + indexPages[0], request);
		}
	}
	else if (type == TYPE_FILE)
		_writeFile(path, request);
	else
	{
		if (path.back() == '/')
			throw 500;
		std::string previousPath = path.substr(0, path.rfind("/"));
		if (_getType(previousPath) == TYPE_DIR)  // if there is directory, make new file
		{
			_statusCode = 201;
			_writeFile(path, request);
		}
		else
			throw 404;
	}
	if (_statusCode == 200)
		_body = "200 OK";
	else if (_statusCode == 201)
		_body = "201 Created";
	_writeStartLine();
	_contentType = "text/plain";
	_writeHeaders();
	_writeBody();
}

bool	Response::_isCGIRequest(const Location& location, const std::string& CGIExtension, std::string& path)
{
	if (_getType(path) == TYPE_DIR)
	{
		std::string indexPage = _getIndexPage(path, location);
		if (!indexPage.empty())
		{
			std::string extension = indexPage.substr(indexPage.rfind("."));
			if (extension == location.getCGI())
			{
				path = indexPage;
				return (true);
			}
		}
	}
	if (!CGIExtension.empty() && location.getCGI() == CGIExtension)
		return true;

	return false;
}

void	Response::_writeStartLine(void)
{
	_ret = "";
	_ret += "HTTP/1.1 ";
	if (_statusCode == 200) // need mapping statusCode map
		_ret += "200 OK\r\n";
	else if (_statusCode == 201)
		_ret += "201 Create\r\n";
}

void	Response::_writeHeaders(void)
{
	// Server
	_ret += "Server: webserv\r\n";

	// Content-Length
	_ret += "Content-Length: ";
	_ret += std::to_string(_body.size());
	_ret += "\r\n";

	// Content-Type
	_ret += "Content-Type: ";
	_ret += _contentType;
	_ret += "\r\n";

	// Date

	_ret += "\r\n";
}

void	Response::_writeBody(void)
{
	_ret += _body;
}

void	Response::_responseRedirect(const Location& location)
{
	const std::string& value = location.getReturn();
	std::pair<std::string, std::string> p = splitString(value, " ");
	if (p.first == "301")
	{
		_ret = "";
		_ret += "HTTP/1.1 301 Moved Permanantly\r\n";
		_ret += "Server: webserv\r\n";
		_ret += "Location: " + p.second + "\r\n";
		_ret += "\r\n";
	}
	else
		throw 500;
}

void	Response::_responseGET(const Location& location, const std::string& realPath, const Request& request, bool isGET)
{
	_type = _getType(realPath);
	if (_type == TYPE_FILE)
		_setBodyFromFile(realPath);
	else if (_type == TYPE_DIR)
		_setBodyFromDir(realPath, location, request);
	else	// not found
		throw 404;
	_writeStartLine();
	_writeHeaders();
	if (isGET)
		_writeBody();
}

void	Response::_isValidHTTPVersion(const std::string& httpVersion) const
{
	if (httpVersion != "HTTP/1.1")
		throw 505;
}

/*
** 해당 location에서 허용되는 methods 인지 파악
** 기본값은 모두 허용임
*/
std::string Response::_isAllowedMethod(const Location& location, const std::string& requestMethod) const
{
	if (requestMethod != "GET" && requestMethod != "HEAD" &&
		requestMethod != "POST" && requestMethod != "PUT" && requestMethod != "DELETE")
		throw 501;
	std::vector<std::string> allowedMethods = location.getMethods();
	size_t methodSize = allowedMethods.size();
	size_t i = 0;
	while (i < methodSize)
	{
		if (requestMethod == allowedMethods[i])
			break ;
		++i;
	}
	if (i == allowedMethods.size())
		throw 405; // 405 Not Allowed method
	return (requestMethod);
}

/*
** uri와 서버의 location path와 비교
** 가장 유사한 location을 리턴함
** uri /a/ 인 경우 -> /a/부터 찾고, 없으면 location /a 매칭임
** uri /a  인 경우 -> /a/를 찾지 못함, /a은 당연히 매칭임
*/
Location	Response::_getMatchingLocation(const Server& server, const std::string& uri) const
{
	const std::vector<Location>& locations = server.getLocations();
	size_t locationSize = locations.size();
	size_t samePathSize = 0;
	size_t searchIndex = 0;
	std::string tmpURI = uri;
	if (uri.back() != '/')
		tmpURI += "/";
	size_t uriSize = tmpURI.size();
	for (size_t i = 0; i < locationSize; ++i)
	{
		const std::string locationPath = locations[i].getPath();
		const size_t pathSize = locationPath.size();
		if (uriSize >= pathSize) // uri길이가 크거나 같을 때 찾음
		{
			if (tmpURI.find(locationPath) != std::string::npos && samePathSize < pathSize) // uri에 locationPath가 있을 경우, 가장 큰 것
			{
				samePathSize = pathSize;
				searchIndex = i;
			}
		}
	}
	if (samePathSize == 0)
		throw 404; // 404 Not Found
	return (Location(server.getLocation(searchIndex)));
}

/*
** location root가 /var/www이고 path가 /a/b일 때
** uri가 /a/b/c 라면 -> /var/www/c 를 반환하게 함
*/
std::string	Response::_getRealPath(const Location& location, const std::string& uri) const
{
	std::string realPath = uri;
	const std::string locationPath = location.getPath();
	const std::string rootDir = location.getRoot();
	if (locationPath.back() == '/')		// location /a/b/ 같은 경우
	{
		if (rootDir.back() == '/')	// root /var/www/ 일 때
			realPath.replace(0, locationPath.size(), rootDir);
		else						// root /var/www 일 때
			realPath.replace(0, locationPath.size() - 1, rootDir);
	}
	else 								// location /a/b 같은 경우
	{
		if (rootDir.back() == '/')	// root /var/www/ 일 때
			realPath.replace(0, locationPath.size(), rootDir.substr(0, rootDir.size() - 1));
		else						// root /var/www 일 때
			realPath.replace(0, locationPath.size(), rootDir);
	}
	return (realPath);
}

/*
** stat으로 FILE인지 DIR인지 파악함
** stat을 실패하는 건 -> 경로를 찾을 수 없는 것 -> 404
*/
int		Response::_getType(const std::string& realPath) const
{
	struct stat statBuf;
	if (stat(realPath.c_str(), &statBuf) == -1)
		return (TYPE_NONE);
	if (S_ISDIR(statBuf.st_mode))
		return (TYPE_DIR);
	else
		return (TYPE_FILE);
}

int		Response::_getTypeMIME(const std::string& fileName) const
{
	size_t findExtension = fileName.rfind(".");
	if (findExtension == std::string::npos)
		return (TEXT);

	std::string extension = fileName.substr(findExtension + 1);
	if (extension == "html" || extension == "htm")
		return (HTML);
	else
		return (TEXT);
}

void    Response::_setBodyFromFile(const std::string& fileName)
{
	if (_getTypeMIME(fileName) == HTML)
		_contentType = "text/html";
	else
		_contentType = "text/plain";

	_body = _readFile(fileName);
}

void	Response::_setBodyFromDir(const std::string& path, const Location& location, const Request& request)
{
	std::string dirPath = path;
	if (dirPath.back() != '/')
		dirPath += "/";

	// search indexpages
	std::string indexPage = _getIndexPage(dirPath, location);
	if (!indexPage.empty())
		return (_setBodyFromFile(indexPage));

	// autoindex
	if (location.getAutoIndex() == true)
		return (_setBodyFromAutoIndex(request, dirPath));

	// else 404 status
	throw 404;
}

void Response::_setBodyFromAutoIndex(const Request& request, const std::string& dirPath)
{
	std::string requestURI = request.getURI();
	if (requestURI.back() != '/')
		requestURI += "/";
	std::string former =
	"<html>\n<head><title>Index of /</title></head>\n<body bgcolor=\"white\">\n<h1>Index of /</h1>\n<hr><pre>\n";
	std::string latter = "</pre><hr></body>\n</html>";
	std::string prefix = "<a href=\"";
	std::string suffix = "</a>\n";
	std::ostringstream oss;

	oss << former;
	DIR *pDir = opendir(dirPath.c_str());
	if (pDir == NULL)
		throw 500;
	struct dirent *dir_ent;
	while ((dir_ent = readdir(pDir)) != NULL)
	{
		oss << prefix;
		oss << requestURI << dir_ent->d_name;
		if (dir_ent->d_type == DT_DIR)
			oss << "/";
		oss << "\">";
		oss << dir_ent->d_name;
		if (dir_ent->d_type == DT_DIR)
			oss <<  "/";
		oss << suffix;
	}
	closedir(pDir);
	oss << latter;
	_body = oss.str();

	_contentType = "text/html";
}

void Response::_responseDELETE(const Location& location, const std::string& path)
{
	int type = _getType(path);
	if (type == TYPE_DIR)
	{
		// search index -> 없으면 404 error
		std::string indexPage = _getIndexPage(path, location);
		if (indexPage.empty())
			throw 404;
		// 있으면 삭제
		remove(indexPage.c_str());
	}
	else if (type == TYPE_FILE)
	{
		remove(path.c_str());
	}
	else // not found
		throw 404;
	_body = "200 Delete";
	_writeStartLine();
	_contentType = "text/plain";
	_writeHeaders();
	_writeBody();
}

std::string Response::_getIndexPage(const std::string& path, const Location& location)
{
	std::string dirPath = path;
	if (dirPath.back() != '/')
		dirPath += "/";
	const std::vector<std::string> indexPages = location.getIndexPages();
	std::vector<std::string>::const_iterator it = indexPages.begin();
	std::vector<std::string>::const_iterator endIter = indexPages.end();
	while (it != endIter)
	{
		DIR *pDir = opendir(dirPath.c_str());
		if (pDir == NULL)
			throw 500;
		struct dirent *dirEnt;
		bool found = false;
		while ((dirEnt = readdir(pDir)) != NULL)
		{
			if (dirEnt->d_name == *it)
			{
				found = true;
				break ;
			}
		}
		closedir(pDir);
		if (found)
			break ;
		++it;
	}
	if (it != endIter)
		return (dirPath + (*it));
	else
		return (std::string());
}


void		Response::_responseWithCGI(const Location& location, const std::string& path, const Request& request)
{
	std::cout << "..cgi..." << std::endl;
	if (_condition == NOT_SET)
	{
		_cgi.setEnv(request, path);
		_cgi.setPath("./tmp" + std::to_string(_socketNum));
		_cgi.execCGI(request, location, this, _cgi.getPath()); // fork -> write(QUERY) -> read
	}
	if (_condition == CGI_READ)
	{
		waitpid(_cgi.getPID(), NULL, 0);
		std::cout << "get or cgi_read" << std::endl;
		_fd = open(_cgi.getPath().c_str(), O_RDONLY);
		if (_fd == -1)
		{
			std::cout << errno << std::endl;
			std::cout << "망가짐" << std::endl;
		}
		FDManager::instance().addReadFileFD(_fd, this, true);
	}
	if (_condition == SET)
	{
		remove(_cgi.getPath().c_str());
		std::string fileContent = FDManager::instance().getResult(_fd);
		size_t findCRLF = fileContent.find("\r\n\r\n");
		size_t contentLength = fileContent.size() - (findCRLF + 4);

		std::cout << "findCRLF: " << findCRLF << std::endl;
		std::cout << "contentLength: " << contentLength << std::endl;

		std::string startLine = "HTTP/1.1 200 OK\r\n";
		std::string header = fileContent.substr(0, findCRLF);
		_ret = "";
		_ret += startLine;
		_ret += header;
		_ret += "\r\n";
		_ret += "Content-Length: " + std::to_string(contentLength);
		_ret += "\r\n\r\n";
		_ret += fileContent.substr(findCRLF + 4);
	}

	// int fd = open("./tmp.txt", O_RDONLY);
	// std::string fileContent;
	// char buf[30001];
	// int ret = 0;
	// while ((ret = read(fd, buf, 30000)) > 0)
	// {
	// 	buf[ret] = '\0';
	// 	fileContent += buf;
	// }
	// close(fd);
	// remove("./tmp.txt");


}
