#ifndef CGI_COMPILE
#define CGI_COMPILE
#endif

#include "CGI.hpp"

CGI::CGI()
	: _env(NULL), _path(), _PID(0)
{
}

CGI::CGI(const CGI &other)
{
	if (this != &other)
		*this = other;
}

CGI::~CGI()
{
	if (_env != NULL)
	{
		_clearEnv();
	}
}

CGI &CGI::operator=(const CGI &rhs)
{
	if (this != &rhs)
	{
		if (_env != NULL)
			_clearEnv();

		if (rhs._env != NULL)
		{
			size_t i = 0;
			while (rhs._env[i])
				++i;
			_env = new char *[i + 1];
			while (rhs._env[i])
			{
				_env[i] = strdup(rhs._env[i]);
				i++;
			}
			_env[i] = NULL;
		}
		else
			_env = rhs._env;
		_path = rhs._path;
		_PID = rhs._PID;
	}
	return (*this);
}

void CGI::setEnv(const Request &request, const std::string path)
{
	std::vector<std::string> envVal;

	envVal.push_back("REQUEST_METHOD=" + request.getMethod());
	envVal.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVal.push_back("GATEWAY_INTERFACE=CGI/1.1");
	if (request.getMethod() == "GET") // get
	{
		_env = new char *[12];
		envVal.push_back("QUERY_STRING=" + request.getQueryString());
	}
	else // post
	{
		_env = new char *[13];
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
	if (request.getHeaders().find("X-Secret-Header-For-Test") != request.getHeaders().end())
	{
		std::string tmp = "HTTP_X_SECRET_HEADER_FOR_TEST=";
		tmp += request.getHeaders().find("X-Secret-Header-For-Test")->second;
		envVal.push_back(tmp);
	}

	for (size_t i = 0; i < envVal.size(); i++)
		_env[i] = strdup(envVal[i].c_str());
	_env[envVal.size()] = NULL;
}

void CGI::execCGI(const Request &request, const Location &location, Response *response, const std::string &path)
{
	int fd[2];
	int originfd[2];

	pipe(fd);
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(fd[1], F_SETFL, O_NONBLOCK);

	originfd[0] = dup(0);
	originfd[1] = dup(1);

	pid_t pid = fork();
	if (pid == 0)
	{
		close(fd[1]);
		dup2(fd[0], 0);
		close(fd[0]);
		int file_fd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		if (file_fd == -1)
		{
			std::runtime_error("Response: cgi child open FAIL");
		}
		dup2(file_fd, 1);
		close(file_fd);
		execve(location.getCGIPath().c_str(), NULL, _env);
		exit(1);
	}
	else
	{
		_PID = pid;
		FDManager::instance().addWriteFileFD(fd[1], request.getBody(), response, true);
		close(fd[0]);
		dup2(originfd[1], 1);
		dup2(originfd[0], 0);
		close(originfd[1]);
		close(originfd[0]);
		_clearEnv();
	}
}

void CGI::setPath(std::string path)
{
	_path = path;
}
const std::string &CGI::getPath(void) const
{
	return (_path);
}

void CGI::setPID(pid_t pid)
{
	_PID = pid;
}

pid_t CGI::getPID(void) const
{
	return (_PID);
}

void CGI::_clearEnv(void)
{
	size_t i = 0;
	while (_env[i])
	{
		free(_env[i]);
		++i;
	}
	delete[] _env;
	_env = NULL;
}
