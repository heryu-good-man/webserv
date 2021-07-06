#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "Config.hpp"
#include "FDManager.hpp"
#include "Server.hpp"

void	handleSignal(int signal)
{
	std::cout << "\nSERVER OFF" << std::endl;
	
	close(0);
	close(1);
	close(2);
	for (int i = 3; i < OPEN_MAX + 1; i++)
	{
		int tmp = dup(i);
		if (tmp != -1)
		{
			close(tmp);
			close(i);
		}
	}
	exit(signal);
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "invalid argument(Usage ./webserv [CONFIG_FILE])" << std::endl;
		return (1);
	}

	signal(SIGINT, handleSignal);
	Config conf;
	try
	{
		conf.parseConfig(argv[1]);
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return (1);
	}

	std::vector<Server> servers = conf.getServers();
	size_t serversSize = servers.size();
	for (size_t i = 0; i < serversSize; i++)
	{
		std::cout << "OPEN PORT : " << servers[i]._port << std::endl;
	}

	std::vector<Server>::iterator endI = servers.end();
	for (std::vector<Server>::iterator i = servers.begin(); i != endI; i++)
	{
		i->setListenSocket();
		i->setAddress();
		if (i->bindSelf() == -1)
		{
			std::cout << "BIND FAIL" << std::endl;
			return -1;
		}
		if (i->listenSelf(1000) == -1)
		{
			std::cout << "LISTEN FAIL" << std::endl;
			return -1;
		}
		FDManager::instance().setFD(i->getListenSocket(), true, false);
	}

	while (1)
	{
		usleep(50);

		fd_set copyRead = FDManager::instance().getReadSet();
		fd_set copyWrite = FDManager::instance().getWriteSet();
		if (select(FDManager::instance().getMaxFD() + 1, &copyRead, &copyWrite, NULL, NULL) == -1)
		{
			std::cout << "SELECT FAIL" << std::endl;
			FDManager::instance().clearFD();
			exit (1);
		}

		try 
		{
			for (std::vector<Server>::iterator iter = servers.begin(); iter != servers.end(); iter++)
			{
				if (FD_ISSET(iter->getListenSocket(), &copyRead))
					iter->acceptSocket();
				else
					iter->checkSet(&copyRead, &copyWrite);
			}

			for (std::vector<int>::const_iterator iter = FDManager::instance().getReadFileFDs().begin();
					iter != FDManager::instance().getReadFileFDs().end(); )
			{
				if (FD_ISSET(*iter, &copyRead))
				{
					int ret = FDManager::instance().readFile(*iter);
					if (ret == ERROR)
						throw std::runtime_error("FDManager::readFile FAIL");
					else if (ret == MORE)
						++iter;
					else  // success
						;
				}
				else
					++iter;
			}

			for (std::vector<int>::const_iterator iter = FDManager::instance().getWriteFileFDs().begin();
					iter != FDManager::instance().getWriteFileFDs().end(); )
			{
				if (FD_ISSET(*iter, &copyWrite))
				{
					int ret = FDManager::instance().writeFile(*iter);
					if (ret == ERROR)
						throw std::runtime_error("FDManager::writeFile FAIL");
					else if (ret == MORE)
						++iter;
					else  // success
						;
				}
				else
					++iter;
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
