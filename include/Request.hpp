#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <map>

# define BADREQUEST 400

class Request;
class Request
{
public:
	Request();
	Request(const std::string&);
	void parseRequest(void);
	void parseStartLine(void);
	void parseHeader(void);

	/* getter */
	const std::string* 							getStartLine(void) const;
	const std::map<std::string, std::string>&	getHeaders(void) const;
	const std::string& 							getBody(void) const;

	const std::string& 							getMethod(void) const;
	const std::string&	getURI(void) const;
	const std::string&	getHTTPVersion(void) const;
	// 만들기.
	const std::string&	getCGIextension(void) const;
	const std::string&	getQueryString(void) const;
	bool 				isBadRequest(void) const;
	void				addBody(std::string body);
private:
	std::string _message;
	std::string _startLine[3];
	std::string _queryString;
	std::string _cgi_extension;
	std::map<std::string, std::string> _headers;
	std::string _body;
	bool		_badRequset;
	// int			_bodyLen;
};

#endif

// 105
// sl 2
// h 3
// bo 100
