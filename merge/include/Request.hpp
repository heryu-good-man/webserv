#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <map>

class Request
{
private:
	std::string _message;
	std::string _startLine[3];
	std::map<std::string, std::string> _headers;
	std::string _body;
public:
	Request(std::string);
	void parseRequest(void);
	void parseStartLine(void);
	void parseHeader(void);
	void parseBody(void);

	/* getter */
	std::string* getStartLine(void);
	std::map<std::string, std::string> getHeaders(void);
	std::string getBody(void);
};

#endif
