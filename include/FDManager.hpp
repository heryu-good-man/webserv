#ifndef FDMANAGER_HPP
# define FDMANAGER_HPP

class FDManager;
# include <iostream>
# include <vector>
# include <map>
# include <unistd.h>
# include <fcntl.h>
# include <exception>
# include "Response.hpp"

# define NOT_SET		0
# define SET			1
# define READING		2
# define WRITING		3
# define ENABLE_WRITE	4
# define END			5

# define ERROR			0
# define SUCCESS		1
# define MORE			2

class FDManager
{
public:
	static FDManager&	instance(void);
	~FDManager();
	void	destroyInstance(void);

	void	clearFD(void);
	void	resetMaxFD(void);
	void	addFD(int fd);
	void	removeFD(int fd);
	void	setFD(int fd, bool isReadSet, bool isWriteSet);
	void	unsetFD(int fd, bool isReadSet, bool isWriteSet);
	void	unsetFileFD(int fd, bool isReadSet, bool isWriteSet);

	int		getMaxFD(void);
	fd_set	getReadSet(void);
	fd_set	getWriteSet(void);
	const std::vector<int>&	getReadFileFDs(void);
	const std::vector<int>&	getWriteFileFDs(void);

	void	addReadFileFD(int fd, Response* response);
	void	addWriteFileFD(int fd, const std::string& data, Response* response);
	int		readFile(int fd);
	int		writeFile(int fd);
	std::string	getResult(int fd);

private:
	static FDManager*	_instance;

	std::vector<int>					_allFDs;
	int									_maxFD;
	fd_set								_readSet;
	fd_set								_writeSet;

	std::map<int, Response*>			_matchFDResponse;

	std::vector<int>					_readFileFDs;
	std::map<int, std::string>			_result;

	std::vector<int>					_writeFileFDs;
	std::map<int, size_t>				_written;
	std::map<int, std::string>			_data;


	FDManager();
	FDManager(const FDManager& other);
	FDManager&	operator=(const FDManager& rhs);

};

#endif
