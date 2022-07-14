#include "NetSocket.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/event.h>
#include "Client.hpp"
#include "ServerConfiguration.hpp"
#include "CgiCore.hpp"
#include <filesystem>

void initServerSockets(std::map<int, NetSocket>& serverSockets, ServerConfiguration& conf)
{
	std::map<std::string, std::vector<ServerConfiguration::OneServConf> >::iterator it = conf.getServerMap().begin();
	std::map<std::string, std::vector<ServerConfiguration::OneServConf> >::iterator end = conf.getServerMap().end();
	while (it != end)
	{
		NetSocket soc;
		std::cout << "Socket opened: " << it->second.begin()->address<<":"<<it->second.begin()->port << std::endl;
		soc.initSocket(it->second.begin()->address, it->second.begin()->port);
		soc.setConf(&it->second);
		serverSockets.insert(std::pair<int, NetSocket>(soc.getSocketFd(), soc));
		++it;
	}
	std::cout << "Sockets init: OK" << std::endl;
}

int acceptClient(const int kq, const int socketFd,
				 std::map<int, Client>& clientBase,
				 std::map<int, NetSocket>& serverSockets)
{
	
	struct sockaddr_in	addr;
	int clientFd, ret;
	socklen_t len(sizeof(addr));
	
	clientFd = accept(socketFd, (struct sockaddr*) &addr, &len);
	if (clientFd == -1)
	{
		std::cout << "Accept FAIL" << std::endl;
		exit(1);
		return -1;
	}
	ret = addReadEvent(kq, clientFd);
	if (ret)
		return ret;
	fcntl(clientFd, F_SETFL, O_NONBLOCK);
	std::map<int, NetSocket>::iterator it = serverSockets.find(socketFd);
	if (it == serverSockets.end())
		return -1;
	Client newClient(clientFd, inet_ntoa(addr.sin_addr), addr.sin_port, it->second.getSocketFd());
	NetSocket* temp = &(it->second);
	newClient.setParentSocket(temp);//(&serverSockets[socketFd]);
	newClient.setRemote(&addr);
	clientBase[clientFd] = newClient;
	return 0;
}





int main(int argc, char* argv[])
{
	std::map<int, NetSocket>	serverSockets;
	std::map<int, Client>		clientBase;
	struct kevent				eventList[MAX_EVENTS];
	int							newEvents, kqFd;
	size_t ret;
	const char* arg;
	
	if (argc == 1)
		arg = "./web/default.conf";
	else if (argc == 2)
		arg = argv[1];
	else {
		std::cout << "Progamm must have only 1 argument as a configuration file" << std::endl;
		return 1;
	}
	ServerConfiguration conf(arg);
	try {
		conf.init();
        std::cout << "pwd: " << conf.getPath() << std::endl;
        
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return (0);
	}
	
	
	kqFd = kqueue();
	if (kqFd == -1)
	{
		std::cout << "KQUEUE ERROR" << std::endl;
		exit(1);
	}
	try
	{
		initServerSockets(serverSockets, conf);
		std::map<int, NetSocket>::iterator it = serverSockets.begin();
		while (it != serverSockets.end())
		{
			addReadEvent(kqFd, it->first);
			++it;
		}
		std::cout << "Events added." << std::endl;
		
		for(;;)
		{
			usleep(2000);
			newEvents = kevent(kqFd, //очередь
							   NULL, 0,  //здесь не нужно, список kevent структур отслеживаемых событий
							   eventList, MAX_EVENTS, //массив куда положить, список kevent структур произоше
							   NULL); //таймаут ожидания
			if (newEvents == -1)
			{
				std::cout << "KEVENT ERROR" << std::endl;
				exit(-1);
			}
			for (int i = 0; i < newEvents; i++)
			{
				std::cout << "[" << i + 1 << " of " << newEvents << "]" << std::endl;
				int evFd = static_cast<int>(eventList[i].ident);
				if (eventList[i].flags & EV_EOF)
				{
					close(evFd);
					std::cout << "Client disconnected." << std::endl;
				}
				else if (serverSockets.find(evFd) != serverSockets.end())
				{
					ret = acceptClient(kqFd, evFd, clientBase, serverSockets);
					if (ret)
					{
						std::cout << "ACCEPT ERROR" << std::endl;
						exit(static_cast<int>(ret));
					}
					std::cout << "Client accepted." << std::endl;
				}
				else if (eventList[i].filter == EVFILT_READ)
				{
					std::cout << "\nEVFILT_READ" << std::endl;
					Client&	client = clientBase[evFd];
					std::cout << "\tRecieving" << std::endl;
					if (client.recieveRequest(eventList[i].data))
					{
						clientBase.erase(evFd);
						close(evFd);
						continue;
					}
					std::cout << "\tStarting check" << std::endl;
					client.checkRequest();
					if (client.getRequestStatus() == REQUEST_ERROR)
					{
						std::cout << "Request error(fd): " << evFd << std::endl;
						clientBase.erase(evFd);
						close(evFd);
						continue;
					}
					client.processRequest(kqFd);
				}
				else if (eventList[i].filter == EVFILT_WRITE)
				{
					std::string response = clientBase[evFd].getResponse().raw;
					size_t r = send(evFd, response.c_str(), response.size(), 0);
					std::cout << ">>>Send: " << r << "/" << response.size() << std::endl;
					if (r < response.size())
					{
						clientBase[evFd].getResponse().raw = clientBase[evFd].getResponse().raw.substr(r);
						addWriteEvent(kqFd, evFd);
					}
					else
					{
						std::cout << "Served page to client, listening...:  "  << clientBase[evFd].getResponse()._body.size() << "Code " << clientBase[evFd].getResponse().getCode() <<std::endl;
						
						if (clientBase[evFd].isClose() || clientBase[evFd].getResponseBackup().find("Content-Length") == std::string::npos)
						{
							std::cout << "Client disconnected (close). " << evFd << std::endl;
							clientBase.erase(evFd);
							shutdown(evFd, SHUT_RDWR);
							close(evFd);
						}
						else
							clientBase[evFd].clearRequest();
					}
				}
				else
				{
					clientBase.erase(evFd);
					shutdown(evFd, SHUT_RDWR);
					close(evFd);
				}
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
