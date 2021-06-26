#ifndef FDMANAGER_COMPILE
# define FDMANAGER_COMPILE
#endif
#include "FDManager.hpp"

FDManager* FDManager::_instance = NULL;

FDManager::FDManager()
	: _maxFD(0)
{
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
}

FDManager::~FDManager()
{
	destroyInstance();
}

FDManager&	FDManager::instance(void)
{
	if (_instance == NULL)
	{
		_instance = new FDManager();
	}
	return (*_instance);
}

void	FDManager::destroyInstance(void)
{
	if (_instance == NULL)
		delete _instance;
}

void	FDManager::clearFD(void)
{
	for (std::vector<int>::iterator iter = _readFileFDs.begin(); iter != _readFileFDs.end(); )
		unsetFileFD(*iter, true, false);
	for (std::vector<int>::iterator iter = _writeFileFDs.begin(); iter != _writeFileFDs.end(); )
		unsetFileFD(*iter, false, true);
	for (std::vector<int>::iterator iter = _allFDs.begin(); iter != _allFDs.end(); )
		unsetFD(*iter, true, true);
}

void	FDManager::resetMaxFD(void)
{
	_maxFD = 0;
	for (std::vector<int>::iterator it = _allFDs.begin(); it != _allFDs.end(); ++it)
	{
		if (*it > _maxFD)
			_maxFD = *it;
	}
	// std::cout << "maxFD : " << _maxFD << std::endl;
}

void	FDManager::addFD(int fd)
{
	_allFDs.push_back(fd);
	resetMaxFD();
}

void	FDManager::removeFD(int fd)
{
	close(fd);
	std::vector<int>::iterator it;
	for (it = _allFDs.begin(); it != _allFDs.end(); ++it)
	{
		if (*it == fd)
			break ;
	}
	if (it != _allFDs.end())
		_allFDs.erase(it);
	resetMaxFD();
}

void	FDManager::setFD(int fd, bool isReadSet, bool isWriteSet)
{
	fcntl(fd, F_SETFL, O_NONBLOCK);
	addFD(fd);
	if (isReadSet)
		FD_SET(fd, &_readSet);
	if (isWriteSet)
		FD_SET(fd, &_writeSet);
}

void	FDManager::unsetFD(int fd, bool isReadSet, bool isWriteSet)
{
	removeFD(fd);
	if (isReadSet)
		FD_CLR(fd, &_readSet);
	if (isWriteSet)
		FD_CLR(fd, &_writeSet);
}

void	FDManager::unsetFileFD(int fd, bool isReadSet, bool isWriteSet)
{
	unsetFD(fd, isReadSet, isWriteSet);
	if (isReadSet)
	{
		std::vector<int>::iterator it;
		for (it = _readFileFDs.begin(); it != _readFileFDs.end(); ++it)
		{
			if (*it == fd)
				break ;
		}
		if (it != _readFileFDs.end())
			_readFileFDs.erase(it);
	}
	if (isWriteSet)
	{
		std::vector<int>::iterator it;
		for (it = _writeFileFDs.begin(); it != _writeFileFDs.end(); ++it)
		{
			if (*it == fd)
				break ;
		}
		if (it != _writeFileFDs.end())
			_writeFileFDs.erase(it);

		_data.erase(fd);
		_written.erase(fd);
	}
	_matchFDResponse.erase(fd);
}

int		FDManager::getMaxFD(void)
{
	return (_maxFD);
}

fd_set	FDManager::getReadSet(void)
{
	return (_readSet);
}

fd_set	FDManager::getWriteSet(void)
{
	return (_writeSet);
}

const std::vector<int>&	FDManager::getReadFileFDs(void)
{
	return (_readFileFDs);
}

const std::vector<int>&	FDManager::getWriteFileFDs(void)
{
	return (_writeFileFDs);
}

void	FDManager::addReadFileFD(int fd, Response* response)
{
	_matchFDResponse[fd] =  response;
	_readFileFDs.push_back(fd);

	setFD(fd, true, false);
	response->setCondition(READING);
}

void	FDManager::addWriteFileFD(int fd, const std::string& data, Response* response)
{
	_matchFDResponse.insert(std::make_pair(fd, response));
	_writeFileFDs.push_back(fd);
	_data.insert(std::make_pair(fd, data));
	_written.insert(std::make_pair(fd, 0));

	setFD(fd, false, true);

	response->setCondition(WRITING);
}

int		FDManager::readFile(int fd)
{
	char buf[65531];
	int ret = read(fd, buf, 65530);
	if (ret == -1)		// error
	{
		unsetFileFD(fd, true, false);
		return (ERROR);
	}
	else if (ret == 0)	// end
	{
		_matchFDResponse[fd]->setCondition(SET);
		unsetFileFD(fd, true, false);
		return (SUCCESS);
	}
	else				// need more
	{
		buf[ret] = '\0';
		_result[fd] += buf;
		return (MORE);
	}
}

int		FDManager::writeFile(int fd)
{
	size_t writtenSize = _written[fd];
	size_t writeSize = _data[fd].size() - writtenSize;
	int ret = write(fd, _data[fd].c_str() + writtenSize, writeSize);

	if (ret == -1)				// error
	{
		unsetFileFD(fd, false, true);
		return (ERROR);
	}
	else if (static_cast<size_t>(ret) == writeSize)	// end
	{
		_matchFDResponse[fd]->setCondition(SET);
		unsetFileFD(fd, false, true);
		return (SUCCESS);
	}
	else						// need more
	{
		_written[fd] += ret;
		return (MORE);
	}
}

std::string	FDManager::getResult(int fd)
{
	std::string tmp = _result[fd];
	_result.erase(fd);
	return (tmp);
}
