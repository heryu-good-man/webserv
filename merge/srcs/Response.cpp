#include "Response.hpp"

Response::Response(void)
	: _ret()
	, _body()
	, _statusCode(200)
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
	}
	return *this;
}

std::string	Response::makeErrorResponse()
{
	setStatusMap();
	std::map<int, std::string>* StatusMap = getStatusMap();
	std::string statusText = StatusMap->find(_statusCode)->second;
	std::string status = std::to_string(_statusCode) + " " + statusText;
	std::string ret = "HTTP/1.1 ";
	ret += status;
	ret += "\r\nContent-Type: text/html\r\nContent-Length: ";
	ret += std::to_string(status.size());
	ret += "\r\n\r\n";
	ret += status;
	unsetStatusMap();
	return (ret);
}

void	Response::response(const Server& server, const Request& request)
{
	try
	{
		// if (request.isBadRequest() == true)
		// 	throw 400 ; // 400 bad request
		_isValidHTTPVersion(request.getHTTPVersion());
		Location location = _getMatchingLocation(server, request.getURI());
		// if there is return in location, redirection response
		std::string requestMethod = _isAllowedMethod(location, request.getMethod());
		std::string realPath = _getRealPath(location, request.getURI());

		if (requestMethod == "GET")
			_responseGET(location, realPath, request);
		// else if (requestMethod == "HEAD")
		// 	responseHEAD(server, request);
		// else if (requestMethod == "PUT")
		// 	responsePUT(server, request);
		// else if (requestMethod == "POST")
		// 	responsePOST(server, request); // localhost/a/b/c/d
		// else if (requestMethod == "DELETE")
		// 	responseDELETE(server, request);
		// else
		// 	return ; // 501 status
	}
	catch(int code)
	{
		_statusCode = code;
		std::cout << code << std::endl;
		_ret = makeErrorResponse();
	}
}

void	Response::_writeStartLine(void)
{
	_ret = "";
	_ret += "HTTP/1.1 ";
	if (_statusCode == 200) // need mapping statusCode map
		_ret += "200 OK\r\n";
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

void	Response::_responseGET(const Location& location, const std::string& realPath, const Request& request)
{
	if (_getType(realPath) == FILE)
		_setBodyFromFile(realPath, location);
	else // directory
		_setBodyFromDir(realPath, location, request);
	_writeStartLine();
	_writeHeaders();
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
		throw 404;
	if (S_ISDIR(statBuf.st_mode))
		return (DIRECTORY);
	else
		return (FILE);
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

void	Response::_setBodyFromFile(const std::string& fileName, const Location& location)
{
	std::ifstream ifs;
	ifs.open(fileName, std::ios_base::in);
	if (!ifs.is_open())
		throw 500;

	std::ostringstream oss;
	size_t clientBodySize = location.getClientBodySize();
	size_t curSize = 0;
	char buf[1024];
	while (curSize < clientBodySize)
	{
		ifs.getline(buf, 1024);
		if (ifs.fail())
		{
			ifs.clear();
			break ;
		}
		oss << buf;
		curSize += strlen(buf);
		if (!ifs.eof())
			oss << "\r\n";
	}
	ifs.close();

	if (curSize > clientBodySize)
		_body = oss.str().substr(0, clientBodySize);
	else
		_body = oss.str();

	if (_getTypeMIME(fileName) == HTML)
		_contentType = "text/html";
	else
		_contentType = "text/plain";
}

void	Response::_setBodyFromDir(const std::string& path, const Location& location, const Request& request)
{
	std::string dirPath = path;
	if (dirPath.back() != '/')
		dirPath += "/";

	// search indexpages
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
		return (_setBodyFromFile(dirPath + (*it), location));

	// autoindex
	if (location.getAutoIndex() == true)
		return (_setBodyFromAutoIndex(request, dirPath));

	// else 403 status
	throw 403;
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
