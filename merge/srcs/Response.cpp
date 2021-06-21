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

std::string	Response::makeErrorResponse(const std::string& req)
{
	setStatusMap();
	std::map<int, std::string>* StatusMap = getStatusMap();
	std::string statusText = StatusMap->find(_statusCode)->second;
	std::string status = std::to_string(_statusCode) + " " + statusText;
	std::string ret = "HTTP/1.1 ";
	ret += status;
	ret += "\r\nContent-Type: text/html";
	ret += "\r\nContent-Length: ";
	ret += std::to_string(status.size());
	
	// if (req != "HEAD")
	// 	ret += "\r\n\r\n";
	// else
	// 	ret += "\r\n";
	ret += "\r\n\r\n";
	if (req != "HEAD")
		ret += status;
	unsetStatusMap();
	return (ret);
}

void	Response::_makeFile(const std::string& path, const Request& req)
{
	std::fstream file;

	std::string method = req.getMethod();
	if (method == "PUT")
		file.open(path, std::ios_base::out | std::ios_base::trunc);
	else if (method == "POST")
		file.open(path, std::ios_base::out | std::ios_base::app);
	if (!file.is_open())
		throw 500;
	// std::cout << "!!!!!!!!!!!!!!!!!!!!!!1" << req.getBody() << std::endl;
	file << req.getBody();
	file.close();
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
			_makeFile(tmpPath + indexPages[0], request);
		}
	}
	else if (type == TYPE_FILE)
		_makeFile(path, request);
	else
	{
		if (path.back() == '/')
			throw 500;
		std::string previousPath = path.substr(0, path.rfind("/"));
		if (_getType(previousPath) == TYPE_DIR)  // if there is directory, make new file
		{
			_statusCode = 201;
			_makeFile(path, request);
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

bool	Response::_isCGI(const Location& location, const std::string& CGIExtention)
{
	if (location.getCGI() == CGIExtention)
		return true;
	else
		return false;
}

void    Response::response(const Server& server, const Request& request)
{
	try
	{
		// if (request.isBadRequest() == true)
		//  throw 400 ; // 400 bad request
		_isValidHTTPVersion(request.getHTTPVersion());
		std::string URI = request.getURI();
		Location location = _getMatchingLocation(server, URI);
		std::string requestMethod = _isAllowedMethod(location, request.getMethod());
		std::string realPath = _getRealPath(location, URI);
		bool isCGI = _isCGI(location, request.getCGIextension());
		// request안에 있는 _cgi_extension을 location 블락 안에 있는 CGI랑 비교하는 게 필요할듯
		if (location.getReturn() != "")
			_responseRedirect(location);
		else if ((requestMethod == "GET" || requestMethod == "POST") && isCGI == true)
			_responseWithCGI(location, realPath, request);
		else if (requestMethod == "GET")
			_responseGET(location, realPath, request, true);
		else if (requestMethod == "HEAD")
			_responseGET(location, realPath, request, false);
		else if (requestMethod == "DELETE")
			_responseDELETE(location, realPath);
		else if (requestMethod == "PUT" || requestMethod == "POST")
		 	_responsePUTorPOST(location, realPath, request);
		// else if (requestMethod == "POST")
		//  responsePOST(server, request); // localhost/a/b/c/d
		// else
		//  return ; // 501 status
	}
	catch(int code)
	{
		_statusCode = code;
		std::cout << code << std::endl;
		_ret = makeErrorResponse(request.getMethod());
	}
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
	int type = _getType(realPath);
	if (type == TYPE_FILE)
		_setBodyFromFile(realPath);
	else if (type == TYPE_DIR)
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
	std::ifstream ifs;
	ifs.open(fileName, std::ios_base::in);
	if (!ifs.is_open())
		throw 500;
	std::ostringstream oss;
	char buf[1024];
	while (1)
	{
		ifs.getline(buf, 1024);
		if (ifs.fail())
		{
			ifs.clear();
			break ;
		}
		oss << buf;
		if (!ifs.eof())
			oss << "\r\n";
	}
	ifs.close();
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
		return (_setBodyFromFile(dirPath + (*it)));

	// autoindex
	if (location.getAutoIndex() == true)
		return (_setBodyFromAutoIndex(request, dirPath));

	// else 403 status
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
		if (indexPage == "")
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
	std::vector<std::string>	envVal;
	char **env;

	envVal.push_back("REQUEST_METHOD=" + request.getMethod());
	envVal.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVal.push_back("GATEWAY_INTERFACE=CGI/1.1");
	//get일 때만
	if (request.getMethod() == "GET")
	{
		env = new char*[10];
		envVal.push_back("QUERY_STRING=" + request.getQueryString());
		std::cout << "query string: " << request.getQueryString() << std::endl;
	}
	// post 일 때만
	else
	{
		env = new char*[11];
		size_t size = request.getBody().size();
		envVal.push_back("CONTENT_LENGTH=" + std::to_string(size));
		envVal.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
	}
	std::string path3 = path.substr(0, path.find("?"));
	std::string path2 = "/Users/mijeong/subject/webserv/merge" + path3.substr(1);
	envVal.push_back("PATH_INFO=" + path2);
	envVal.push_back("SCRIPT_FILENAME=" + path2);
	envVal.push_back("PATH_TRANSLATED=" + path2);
	envVal.push_back("SCRIPT_NAME=" + path2);
	envVal.push_back("REQUEST_URI=" + path2);
	envVal.push_back("REDIRECT_STATUS=200");

	for (size_t i = 0; i < envVal.size(); i++)
	{
		env[i] = strdup(envVal[i].c_str());
		std::cout << "env : " << env[i] << std::endl;
	}
	env[envVal.size()] = NULL;

	int fd[2];
    pipe(fd);
	int originfd[2];
	originfd[1] = dup(1);
	originfd[0] = dup(0);
    // int tmp_fd;
    pid_t pid = fork();
    if (pid == 0)
    {
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);
        int file_fd = open("./tmp.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        dup2(file_fd, 1);
        close(file_fd);
		std::cout << location.getCGIPath().c_str() << std::endl;
		execve(location.getCGIPath().c_str(), NULL, env);
    }
    else
    {
		if (request.getMethod() == "GET")
			write(fd[1], request.getQueryString().c_str(), request.getQueryString().size());
		if (request.getMethod() == "POST")
			write(fd[1], request.getBody().c_str(), request.getBody().size());
		// if (request.getMethod() == "GET")
        // 	write(fd[1], request.getQueryString().c_str(), request.getQueryString().size());
		// if (request.getMethod() == "POST")
        // 	write(fd[1], request.getBody().c_str(), request.getBody().size());
        close(fd[1]);
        close(fd[0]);
        waitpid(-1, NULL, 0);
    }
	dup2(originfd[1], 1);
	dup2(originfd[0], 0);
}
