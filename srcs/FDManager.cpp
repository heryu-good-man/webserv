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

		_data.erase(_matchSocket[fd]);
		_written.erase(_matchSocket[fd]);
	}
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

void	FDManager::addReadFileFD(int fd, Response* response, bool isCGI)
{
	_isCGI[fd] = isCGI;

	_matchSocket[fd] = response->getSocketNum();
	_matchCondition[response->getSocketNum()] = READING;

	_readFileFDs.push_back(fd);
	_result[response->getSocketNum()] = "";

	setFD(fd, true, false);
}

void	FDManager::addWriteFileFD(int fd, const std::string& data, Response* response, bool isCGI)
{
	_isCGI[fd] = isCGI;

	_matchSocket[fd] = response->getSocketNum();
	_matchCondition[response->getSocketNum()] = WRITING;
	
	_writeFileFDs.push_back(fd);
	_data.insert(std::make_pair(response->getSocketNum(), data));
	_written.insert(std::make_pair(response->getSocketNum(), 0));

	setFD(fd, false, true);
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
		_matchCondition[_matchSocket[fd]] = SET;
		unsetFileFD(fd, true, false);
		_isCGI.erase(fd);
		return (SUCCESS);
	}
	else				// need more
	{
		buf[ret] = '\0';
		_result[_matchSocket[fd]] += buf;
		return (MORE);
	}
}

int		FDManager::writeFile(int fd)
{
	size_t writtenSize = _written[_matchSocket[fd]];
	size_t rest = _data[_matchSocket[fd]].size() - writtenSize;
	size_t writeSize = rest < 65530 ? rest : 65530;
	int ret = write(fd, _data[_matchSocket[fd]].c_str() + writtenSize, writeSize);
	if (ret == -1)				// error
	{
		unsetFileFD(fd, false, true);
		return (ERROR);
	}
	_written[_matchSocket[fd]] += ret;
	if (_written[_matchSocket[fd]] == _data[_matchSocket[fd]].size())	// end
	{
		if (_isCGI[fd])
			_matchCondition[_matchSocket[fd]] = CGI_READ;
		else
			_matchCondition[_matchSocket[fd]] = SET;
			// _matchFDResponse[fd]->setCondition(SET);
		unsetFileFD(fd, false, true);
		_isCGI.erase(fd);
		return (SUCCESS);
	}
	else						// need more
	{
		return (MORE);
	}
}

std::string	FDManager::getResult(int socket)
{
	std::string tmp = _result[socket];
	_result.erase(socket);
	return (tmp);
}

int		FDManager::getConditionBySocket(int socket)
{
	return (_matchCondition[socket]);
}

int		FDManager::getConditionByFD(int fd)
{
	return (_matchCondition[_matchSocket[fd]]);
}

void	FDManager::setConditionBySocket(int socket, int condition)
{
	_matchCondition[socket] = condition;
}

void	FDManager::setConditionByFD(int fd, int condition)
{
	_matchCondition[_matchSocket[fd]] = condition;
}