#ifndef CGI_HPP
# define CGI_HPP

#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include "Request.hpp"
#include "Location.hpp"

class CGI
{
public:
    CGI();
    CGI(const CGI& other);
    virtual ~CGI();
    CGI&    operator=(const CGI& rhs);

    void    setEnv(const Request& request, const std::string path);
    void    execCGI(const Request& request, const Location& location);


private:
    char**  _env;

    void    _clearEnv(void);

};

#endif
// 1. 디렉터리 요청: index index.php
// 2. 해당 파일이 있어야 함