#ifndef CGI_HPP
# define CGI_HPP

class CGI;
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include "Request.hpp"
#include "Location.hpp"
#include "FDManager.hpp"

class CGI
{
public:
	CGI();
	CGI(const CGI& other);
	virtual ~CGI();
	CGI&    operator=(const CGI& rhs);

	void    setEnv(const Request& request, const std::string path);
	void    execCGI(const Request& request, const Location& location, Response* response, const std::string& path);

	void               setPath(std::string path);
	const std::string& getPath(void) const;

	void               setPID(pid_t pid);
	pid_t              getPID(void) const;



private:
	char**      _env;
	std::string _path;
	pid_t       _PID;

	void    _clearEnv(void);

};

#endif
// 1. 디렉터리 요청: index index.php
// 2. 해당 파일이 있어야 함
