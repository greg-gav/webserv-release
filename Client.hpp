#ifndef Client_hpp
#define Client_hpp

#include <iostream>
#include <map>
#include <vector>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <sys/socket.h>
#include "ServerConfiguration.hpp"
#include "Event.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "NetSocket.hpp"
#include "fileUtils.hpp"
#include "Session.hpp"

#define REQUEST_OK 0
#define REQUEST_INCOMPLETE 2
#define REQUEST_ERROR -1
#define NOT_SET std::string::npos
#define FIXED 1
#define CHUNKED 2

class Session;
class Client
{
public:
	struct BodyPart
	{
		std::map<std::string, std::string>	headers;
		std::string							content;
	};
						Client();
						Client(const int fd, const std::string& ip, const int port, const int serverListenFd);
						Client(const Client& src);
						~Client();
	
	void				setFd(const int fd);
	void				setAddress(const std::pair<std::string, int>& address);
	void				checkRequest();
	void				parseRequest();
	int					recieveRequest(size_t requestLen);
	
	int					getFd() const;
	int					getRequestStatus() const;
	ssize_t				getEmptyLine() const;
	int					getContentLength() const;
	const std::string& 	getContentType() const;
	const std::string&	getRequestRaw();
	void				setEmptyLine(ssize_t emptyLine);
	void				clearRequest();
	Session&			getSession();
	void				setParentSocket(NetSocket* soc);
	NetSocket*			getParentSocket() const;
	bool				isClose();
	void				setRemote(struct sockaddr_in*	sockaddr);
	
	const std::string&	getRemoteAddr() const;
	int 				getRemotePort() const;
	typedef std::vector<ServerConfiguration::OneServConf>::iterator iter;
	typedef std::vector<ServerConfiguration::Location>::iterator iterL;
	void				setHost();
	void				setLocation();
	iter				getHost();
	iterL				getLocation();
	Request&			getRequest();
	Response&			getResponse();
	const std::string&	getResponseBackup() const;
	int					processRequest(int kq);
private:
	void				appendRequest(const char *str, const long len);
	Request						_req;
	Response					_res;
	int							_fd;
	std::pair<std::string, int>	_addr;
	int							_serverListenFd;
	NetSocket*					_parentSocket;

	Session						_session;
	std::string					_remoteAddress;
	int							_remotePort;
	iter						_host;
	iterL						_location;
	
};
int processRequest(int kq, Client& cl);
bool hasEnding (std::string const &fullString, std::string const &ending);
void fileToStr(const std::string& file, std::string& content);

#endif
