#include "NetSocket.hpp"


NetSocket::NetSocket(){ }

NetSocket::~NetSocket(){ }


void NetSocket::socket(int domain, int type, int protocol)
{
	_socketFd = ::socket(domain, type, protocol);
	if (SOCKET_ERROR == _socketFd)
	{
		std::string s = "socket: " + std::string(strerror(errno));
		throw SocketException(s.c_str());
	}
}

void NetSocket::setsockopt(int level, int option_name)
{
	int enable = 1;
	if (SOCKET_ERROR == ::setsockopt(_socketFd, level, option_name, &enable, sizeof(enable)))
	{
		std::string s = "setsockopt: " + std::string(strerror(errno));
		throw SocketException(s.c_str());
	}
}

void NetSocket::bind(const std::string& ip, const int& port)
{
	struct sockaddr_in _addr;
	
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(port); // host to network bytes short
	_addr.sin_addr.s_addr = inet_addr(ip.c_str()); // or htonl(INADDR_ANY);
	if (SOCKET_ERROR == ::bind(_socketFd, (struct sockaddr*)&_addr, sizeof(_addr)))
	{
		std::string s = "bind: " + std::string(strerror(errno));
		throw SocketException(s.c_str());
	}
}

void NetSocket::listen(int backlog)
{
	if (SOCKET_ERROR == ::listen(_socketFd, backlog))
	{
		std::string s = "listen: " + std::string(strerror(errno));
		throw SocketException(s.c_str());
	}
}

void NetSocket::initSocket(const std::string& ip, const int& port)
{
	socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // | SOCK_NONBLOCK
	setsockopt(SOL_SOCKET, SO_REUSEADDR);
	bind(ip, port);
	listen(SOMAXCONN);
}

int NetSocket::getSocketFd() const
{
	return _socketFd;
}

NetSocket::SocketException::SocketException(const std::string& error)
:_errorMsg("NetSocket." + error) {}

const char*		NetSocket::SocketException::what(void) const throw() { return _errorMsg.c_str(); }

void	NetSocket::setConf(std::vector<ServerConfiguration::OneServConf>* conf)
{
	_conf = conf;
}

std::vector<ServerConfiguration::OneServConf>*	NetSocket::getConf() const
{
	return _conf;
}
