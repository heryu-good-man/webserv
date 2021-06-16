#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <iostream>
# include <fstream>
# include <sstream>
# include <sys/stat.h>
# include <cstdlib>
# include <dirent.h>
# include "Request.hpp"
# include "Location.hpp"

# define FILE		1
# define DIRECTORY	2

# define HTML	1
# define TEXT	2

class Response
{
public:
	Response(void);
	Response(const Response &ref);
	~Response(void);
	Response	&operator=(const Response &ref);

	const std::string&	getRet(void) const;

	void		response(const Server& server, const Request& request);


private:
	std::string		_ret;
	std::string		_body;
	int				_statusCode;

	std::string		_contentType;


	void		_responseGET(const Location& location, const std::string& realPath);

	void		_isValidHTTPVersion(const std::string& httpVersion) const;
	std::string	_isAllowedMethod(const Location& location, const std::string& method) const;

	Location	_getMatchingLocation(const Server& server, const std::string& uri) const;
	std::string	_getRealPath(const Location& location, const std::string& uri) const;
	int			_getType(const std::string& realPath) const;
	int			_getTypeMIME(const std::string& fileName) const;

	void		_setBodyFromFile(const std::string& fileName, const Location& location);
	void		_setBodyFromDir(const std::string& dirPath, const Location& location);

	void 		_setAutoIndex(const std::string& dirPath);
};

#endif
