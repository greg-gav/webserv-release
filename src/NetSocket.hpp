//
//  Socket.hpp
//  Socket
//
//  Created by Евгений on 22.02.2022.
//

#ifndef NETSOCKET_HPP
# define NETSOCKET_HPP
# include <iostream>
# include <sys/socket.h> //socket
# include <netinet/in.h> //IPPROTO_TCP
# include <arpa/inet.h> //inet_addr
# include <sys/types.h>

# include <map>
# include <vector>
# include <fcntl.h>
# include <unistd.h>
# include <cstring>
# include <sys/event.h>
# define SOCKET_ERROR -1
# define MAX_EVENTS 1000
# ifndef SOCK_NONBLOCK
#  define SOCK_NONBLOCK O_NONBLOCK
# endif
#include "ServerConfiguration.hpp"

class NetSocket
{
public:
	NetSocket();
	~NetSocket();
	
	void	initSocket(const std::string& ip, const int& port);
	int		getSocketFd() const;
	void	socket(int domain, int type, int protocol);
	void	setsockopt(int level, int option_name);
	void	bind(const std::string& ip, const int& port);
	void	listen(int backlog);
	
	void	setConf(std::vector<ServerConfiguration::OneServConf>* conf);
	std::vector<ServerConfiguration::OneServConf>* getConf() const;
	
	class SocketException : public std::exception
	{
	public:
		SocketException(const std::string& error);
		virtual					~SocketException() throw() {};
		virtual const char*		what(void) const throw();
	private:
		std::string _errorMsg;
	};
private:
	int _socketFd;
	std::vector<ServerConfiguration::OneServConf>* _conf;
	
};

#endif /* Socket_hpp */
