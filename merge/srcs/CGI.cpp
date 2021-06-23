#include "CGI.hpp"

CGI::CGI()
    : _env()
{

}

CGI::CGI(const CGI& other)
    : _env(other._env)
{

}

CGI::~CGI()
{

}

CGI&    CGI::operator=(const CGI& rhs)
{
    if (this != &rhs)
    {
        _env = rhs._env;
    }
    return (*this);
}

void    CGI::setEnv(const Request& request, const std::string path)
{
    std::vector<std::string>	envVal;

	envVal.push_back("REQUEST_METHOD=" + request.getMethod());
	envVal.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVal.push_back("GATEWAY_INTERFACE=CGI/1.1");
	if (request.getMethod() == "GET") // get
	{
		_env = new char*[12];
		envVal.push_back("QUERY_STRING=" + request.getQueryString());
	}
	else // post
	{
		_env = new char*[13];
		size_t size = request.getBody().size();
		envVal.push_back("CONTENT_LENGTH=" + std::to_string(size));
		envVal.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
	}
	std::string path2 = path.substr(0, path.find("?"));

	envVal.push_back("PATH_INFO=" + path2);
	envVal.push_back("SCRIPT_FILENAME=" + path2);
	envVal.push_back("PATH_TRANSLATED=" + path2);
	envVal.push_back("SCRIPT_NAME=" + path2);
	envVal.push_back("REQUEST_URI=" + path2);
	envVal.push_back("REDIRECT_STATUS=200");
    envVal.push_back("HTTP_X_SECRET_HEADER_FOR_TEST=1");

	for (size_t i = 0; i < envVal.size(); i++)
		_env[i] = strdup(envVal[i].c_str());
	_env[envVal.size()] = NULL;
}

void    CGI::execCGI(const Request& request, const Location& location)
{
    int fd[2];
    pipe(fd);
	int originfd[2];
	originfd[0] = dup(0);
    originfd[1] = dup(1);
    // int tmp_fd;
    pid_t pid = fork();
    if (pid == 0)
    {
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);
        // std::cout << "before open\n";
        int file_fd = open("./tmp.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        // std::cout << "after open\n";
        dup2(file_fd, 1);
        close(file_fd);
		execve(location.getCGIPath().c_str(), NULL, _env);
        exit(1);
    }
    else
    {
        if (request.getMethod() == "POST")
        {
            size_t rest = request.getBody().size();
            size_t writtenSize = 0;
            // std::cout << "before cgi write\n";
            std::cout << "********request : " << request.getBody().size() << std::endl;
            while (rest != 0)
            {
                size_t writeSize = rest < 65530 ? rest : 65530;
                int tmpSize = 0;
                if ((tmpSize = write(fd[1], request.getBody().c_str() + writtenSize, writeSize)) <= 0)
                {
                    std::cout << "ssibal cgi" << std::endl;
                }
                rest -= tmpSize;
                writtenSize += tmpSize;
            }
            // std::cout << "after cgi write\n";
        }
        close(fd[1]);
        close(fd[0]);
        waitpid(-1, NULL, 0);
    }
	dup2(originfd[1], 1);
	dup2(originfd[0], 0);
	close(originfd[1]);
	close(originfd[0]);
    _clearEnv();
}

void    CGI::_clearEnv(void)
{
    size_t i = 0;
	while (_env[i])
    {
		free(_env[i]);
        ++i;
    }
	delete[] _env;
}