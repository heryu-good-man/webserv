#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <map>

# define BADREQUEST 400

class Request
{
public:
	Request();
	Request(const std::string&);
	void parseRequest(void);
	void parseStartLine(void);
	void parseHeader(void);
	void parseBody(void);

	/* getter */
	const std::string* getStartLine(void) const;
	const std::map<std::string, std::string>& getHeaders(void) const;
	const std::string& getBody(void) const;

	const std::string& getMethod(void) const
	{
		return (getStartLine()[0]);
	}
	const std::string& getURI(void) const
	{
		return (getStartLine()[1]);
	}
	const std::string& getHTTPVersion(void) const
	{
		return (getStartLine()[2]);
	}
	// 만들기.
	const std::string& getCGIextension(void) const
	{
		return _cgi_extension;
	}
	const std::string& getQueryString(void) const
	{
		return _queryString;
	}
	bool isBadRequest(void) const
	{
		return _badRequset;
	}
	void	addBody(std::string body)
	{
		_body += body;
	}
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