#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <iostream>
# include <fstream>
# include <sys/stat.h>
# include "Request.hpp"
# include "Location.hpp"

# define FILE	1
# define DIR	2

class Response
{
public:
	Response(void);
	Response(const Response &ref);
	~Response(void);
	Response	&operator=(const Response &ref);
	Location	getMatchingLocation(const Server& server, const std::string& uri) const;
	std::string	getRealPath(const Location& location, const std::string& uri) const;
	int			getType(const std::string& realPath) const;
	std::string	isAllowedMethod(const Location& location, const std::string& method) const;
	void		response(const Server& server, const Request& request);
	void 		parseBody(const Server& server);

private:
	std::string		_ret;
	std::string		_body;
	int				_statusCode;
};

#endif
