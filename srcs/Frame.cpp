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
	std::cout << "signal  : " << signal << std::endl;
	
	close(0);
	close(1);
	close(2);
	for (int i = 3; i < OPEN_MAX + 1; i++)
	{
		// dup을 하면 fd가 늘어나서 fd가 가득 찬 경우에는 문제가 생길 수도 있지만 나중에 생각.
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
	}

	// ServerManager manager;
	// manager._fdManager[3] = server1;
	std::vector<Server> servers = conf.getServers();
	size_t serversSize = servers.size();
	for (size_t i = 0; i < serversSize; i++)
	{
		std::cout << "PORT : " << servers[i]._port << std::endl;
	}

	// 각 테스트 서버 bind, listen, setListenSocket readSet에다 넣기.
	std::vector<Server>::iterator endI = servers.end();
	for (std::vector<Server>::iterator i = servers.begin(); i != endI; i++)
	{
		i->setListenSocket();
		i->setAddress();
		if (i->bindSelf() == -1)
		{
			std::cout << "bind 실패!!\n";
			return -1;
		}
		if (i->listenSelf(1000) == -1)
		{
			std::cout << "listen 실패!!\n";
			return -1;
		}
		FDManager::instance().setFD(i->getListenSocket(), true, false);
	}
	fd_set copyRead;
	fd_set copyWrite;
	while (1)
	{
		usleep(50);
		copyRead = FDManager::instance().getReadSet();
		copyWrite = FDManager::instance().getWriteSet();

		if (select(FDManager::instance().getMaxFD() + 1, &copyRead, &copyWrite, NULL, NULL) == -1)
		{
			std::cout << "select Fail!!!!\n";
			FDManager::instance().clearFD();
			exit (-1);
		}

		for (std::vector<Server>::iterator iter = servers.begin(); iter != servers.end(); iter++)
		{
			if (FD_ISSET(iter->getListenSocket(), &copyRead))
				iter->acceptSocket();
			else
				iter->checkSet(&copyRead, &copyWrite);
			// usleep(50);
		}

		for (std::vector<int>::const_iterator iter = FDManager::instance().getReadFileFDs().begin();
				iter != FDManager::instance().getReadFileFDs().end(); )
		{
			if (FD_ISSET(*iter, &copyRead))
			{
				int ret = FDManager::instance().readFile(*iter);
				if (ret == ERROR)
					throw std::runtime_error("FDManager::readFile error");
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
					throw std::runtime_error("FDManager::writeFile error");
				else if (ret == MORE)
					++iter;
				else  // success
					;
			}
			else
				++iter;
		}
	}
}
