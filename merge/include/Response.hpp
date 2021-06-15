#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <iostream>
# include <fstream>
# include "Request.hpp"
# include "Location.hpp"

class Response
{
public:
	Response(void);
	Response(const Response &ref);
	~Response(void);
	Response	&operator=(const Response &ref);
	Location	getLocation(const Server& server, const std::string& uri);
	std::string	isAllowedMethod(const Location& location, const std::string& method);
	void		response(const Server& server, const Request& request);
	void 		parseBody(const Server& server);

private:
	std::string		_ret;
	std::string		_body;
	int				_statusCode;
};

#endif
