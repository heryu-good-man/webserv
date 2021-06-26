#ifndef RESPONSE_HPP
# define RESPONSE_HPP

class Response;
# include <string>
# include <iostream>
# include <fstream>
# include <sstream>
# include <sys/stat.h>
# include <cstdlib>
# include <dirent.h>
# include <map>
# include "Location.hpp"
# include "FDManager.hpp"
# include "Request.hpp"
# include "CGI.hpp"
# include "Server.hpp"

# define TYPE_FILE	1
# define TYPE_DIR	2
# define TYPE_NONE	3

# define HTML	1
# define TEXT	2

class Response
{
public:
	Response(void);
	Response(const Response &ref);
	~Response(void);
	Response &operator=(const Response &ref);

	void	response(const Server& server, const Request& request);
	void	setStatusMap()
	{
		_statusMap = new std::map<int, std::string>;
		_statusMap->insert(std::pair<int, std::string>(200, "OK"));
		_statusMap->insert(std::pair<int, std::string>(201, "Created"));
		_statusMap->insert(std::pair<int, std::string>(301, "Moved Permanantly"));
		_statusMap->insert(std::pair<int, std::string>(400, "Bad Request"));
		_statusMap->insert(std::pair<int, std::string>(403, "Forbidden"));
		_statusMap->insert(std::pair<int, std::string>(404, "Not found"));
		_statusMap->insert(std::pair<int, std::string>(405, "Not Allowed Method"));
		_statusMap->insert(std::pair<int, std::string>(413, "Payload Too Large"));
		_statusMap->insert(std::pair<int, std::string>(500, "Internal Server Error"));
		_statusMap->insert(std::pair<int, std::string>(501, "Not Implemented"));
		_statusMap->insert(std::pair<int, std::string>(502, "Bad Gateway"));
		_statusMap->insert(std::pair<int, std::string>(503, "Service Unavailable"));
		_statusMap->insert(std::pair<int, std::string>(505, "HTTP Version Not Supported"));
	}
	void	unsetStatusMap(void)
	{
		_statusMap->clear();
		delete _statusMap;
	}
	std::map<int, std::string>* getStatusMap(void)
	{
		return _statusMap;
	}
	std::string makeErrorResponse(const Server& server, const std::string& req);
	const std::string& getMessage(void) const
	{
		return _ret;
	}
	int	getCondition(void) const
	{
		return (_condition);
	}
	void setCondition(int condition)
	{
		_condition = condition;
	}
	size_t getWrittenSize(void) const
	{
		return (_writtenSize);
	}
	void setWrittenSize(size_t size)
	{
		_writtenSize = size;
	}
	int getSocketNum(void) const
	{
		return (_socketNum);
	}
	void setSocketNum(int num)
	{
		_socketNum = num;
	}
	const CGI& getCGI(void) const
	{
		return (_cgi);
	}

private:
	std::string					_ret;
	std::string					_body;
	int							_statusCode;
	std::map<int, std::string>*	_statusMap;
	std::string					_contentType;
	Location					_location;
	std::string					_requestMethod;
	std::string					_path;
	bool						_isCGI;
	int							_type;
	int							_fd;
	CGI							_cgi;
	int							_condition;
	size_t						_writtenSize;
	int							_socketNum;


	void		_responseRedirect(const Location& location);
	void		_responseGET(const Location&, const std::string&, const Request&, bool);
	void 		_responseDELETE(const Location& location, const std::string& path);
	void		_responsePUTorPOST(const Location& location, const std::string& path, const Request& request);
	void		_responseWithCGI(const Location& location, const std::string& path, const Request& request);
	// void		_responsePOSTwithCGI(const Location& location, const std::string& path, const Request& request);

	void		_writeFile(const std::string& fileName, const Request& req);
	std::string	_readFile(const std::string& fileName);
	void		_isValidHTTPVersion(const std::string& httpVersion) const;
	std::string	_isAllowedMethod(const Location& location, const std::string& method) const;
	bool		_isCGIRequest(const Location& location, const std::string& CGIExtension, std::string& path);

	Location	_getMatchingLocation(const Server& server, const std::string& uri) const;
	std::string	_getRealPath(const Location& location, const std::string& uri) const;
	int			_getType(const std::string& realPath) const;
	int			_getTypeMIME(const std::string& fileName) const;
	std::string _getIndexPage(const std::string& path, const Location& location);
	void		_setBodyFromFile(const std::string& fileName);
	void		_setBodyFromDir(const std::string& dirPath, const Location&, const Request&);
	void 		_setBodyFromAutoIndex(const Request& request, const std::string& dirPath);

	void		_writeStartLine(void);
	void		_writeHeaders(void);
	void		_writeBody(void);

};

#endif