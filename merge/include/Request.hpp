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
	void parseRequest();
	void parseStartLine();
	void parseHeader();
	void parseBody();
};

#endif
