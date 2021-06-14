#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include "Server.hpp"
#include "Config.hpp"


int main(int argc, char** argv, char** envp)
{
	(void)envp;
	(void)argc;
	fd_set	readSet;
	fd_set	writeSet;
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
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

	int maxFd;
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
		if (i->listenSelf(5) == -1)
		{
			std::cout << "listen 실패!!\n";
			return -1;
		}
		FD_SET(i->getListenSocket(), &readSet);
		// manager._fdManager[i->getListenSocket()] = *i;
		maxFd = i->getListenSocket();
	}
	fd_set copyRead;
	fd_set copyWrite;
	while (1)
	{
		copyRead = readSet;
		copyWrite = writeSet;

		// std::cout << "wait req!!\n";
		if (select(maxFd + 1, &copyRead, &copyWrite, NULL, NULL) == -1)
		{
			std::cout << "select Fail!!!!\n";
			exit (-1);
		}
		// std::cout << "recieve req!!\n";

		// 각 server의 listenSocket확인
		// manager에 서버를 넣어줌.
		for (std::vector<Server>::iterator iter = servers.begin(); iter != servers.end(); iter++)
		{
			// if (FD_ISSET(iter->getListenSocket(), &readSet))
			// 	manager._fdManager[iter->acceptSocket(&readSet, &writeSet)] = *iter;
			if (FD_ISSET(iter->getListenSocket(), &copyRead))
			{
				int tmp;

				std::cout << "accept!!\n";
				tmp = iter->acceptSocket(&readSet, &writeSet);
				if (tmp > maxFd)
					maxFd = tmp;
			}
			// 입력 버퍼에 들어온게 있는지 확인해본다.
			// && 출력할게 있는지 확인한다.
			iter->checkSet(&readSet, &writeSet, &copyRead, &copyWrite);
			usleep(10);
		}
	}
}