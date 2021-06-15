#include "Response.hpp"

Response::Response(void)
{
	_statusCode = 200;
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
/*
1. http 버전확인
 -> invalid하면?? -> 505

2. uri parsing
	- 1. locations 비교.
	- 2. location에 맞는 메소드 확인
	- 3. 파일 디렉터리 구분
	- 디렉터리 (없으면 상태 만들어)
		-> 1. 오토인덱스
		-> 2. index page
	- 파일 (없으면 상태 만들어)
		-> 1. content-length, content-type
	- 4. Content-Languag
3. 응답메시지 작성.
*/

void	_isValidHTTPVersion(const std::string& httpVersion)
{
	if (httpVersion != "HTTP/1.1")
		throw 505;
}

// void Response::parseBody(const Server& server)
// {

// }

// void	Response::responseGET(const Server& server, const Request& request)
// {

// }

// void	Response::responseHEAD(const Server& server, const Request& request)
// {

// }

// void	Response::responsePUT(const Server& server, const Request& request)
// {

// }

// void	Response::responsePOST(const Server& server, const Request& request)
// {

// }

// void	Response::responseDELETE(const Server& server, const Request& request)
// {

// }

/**
 *  location /{}
 * 	location /a{}
 *  location /a/b {
 * 		root /var/www;    /a/b/c/d/index.html -> /var/www/c/d/index.html
 * 	} 유사성 높은 것부터 찾음 될듯ㅎ
*/
Location	Response::getLocation(const Server& server, const std::string& uri)
{
	const std::vector<Location>& locations = server.getLocations();
	size_t locationSize = locations.size();
	size_t uriSize = uri.size();
	size_t samePathSize = 0;
	size_t searchIndex = 0;
	for (size_t i = 0; i < locationSize; ++i)
	{
		const std::string locationPath = locations[i].getPath();
		const size_t pathSize = locationPath.size();
		if (uriSize >= pathSize) // uri길이가 크거나 같을 때 찾음
		{
			if (uri.find(locationPath) != std::string::npos && samePathSize < pathSize) // uri에 locationPath가 있을 경우, 가장 큰 것
			{
				samePathSize = pathSize;
				searchIndex = i;
			}
		}
	}
	if (samePathSize == 0)
		throw 404;
	return (Location(server.getLocation(searchIndex)));
}

std::string Response::isAllowedMethod(const Location& location, const std::string& requestMethod)
{
	std::vector<std::string> allowedMethods = location.getMethods();
	size_t i = 0;
	while (i < allowedMethods.size())
	{
		if (requestMethod == allowedMethods[i])
			break ;
		++i;
	}
	std::cout << "requestMethod: " << requestMethod << std::endl;
	if (i == allowedMethods.size())
		throw 405; // 405 Not Allowed method
	return (requestMethod);
}

void	Response::response(const Server& server, const Request& request)
{
	try
	{
		// if (request.isBadRequest() == true)
		// 	throw 400 ; // 400 bad request
		_isValidHTTPVersion(request.getHTTPVersion());
		Location location = getLocation(server, request.getURI());
		std::cout << "location path: " << location.getPath() << std::endl;
		std::string requestMethod = isAllowedMethod(location, request.getMethod());

		// if (requestMethod == "GET")
		// 	responseGET(server, request);
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
	}

}
