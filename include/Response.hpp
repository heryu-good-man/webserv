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
	const std::string& getMessage(void) const;
	size_t	getWrittenSize(void) const;
	void	setWrittenSize(size_t size);
	int		getSocketNum(void) const;
	void	setSocketNum(int num);


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
	size_t						_writtenSize;
	int							_socketNum;


	void		_responseRedirect(const Location& location);
	void		_responseGET(const Location&, const std::string&, const Request&, bool);
	void 		_responseDELETE(const Location& location, const std::string& path);
	void		_responsePUTorPOST(const Location& location, const std::string& path, const Request& request);
	void		_responseWithCGI(const Location& location, const std::string& path, const Request& request);

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
	std::map<int, std::string>* _getStatusMap(void);
	const CGI& _getCGI(void) const;

	void		_writeStartLine(void);
	void		_writeHeaders(void);
	void		_writeBody(void);

	void		_setStatusMap(void);
	void		_unsetStatusMap(void);
	std::string _makeErrorResponse(const Server& server, const std::string& req);
};

#endif
