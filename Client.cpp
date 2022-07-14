#include "Client.hpp"

Client::Client() {}
Client::~Client() {}
Client::Client(const int fd, const std::string& ip, const int port, const int serverListenFd)
:_fd(fd),
_addr(std::pair<std::string, int>(ip, port)),
_serverListenFd(serverListenFd)
{
}
Client::Client(const Client& src)
{
	*this = src;
}


void	Client::setFd(const int fd) { _fd = fd; }
void	Client::setAddress(const std::pair<std::string, int>& address) { _addr = address; }

void	Client::appendRequest(const char *str, const long len)
{
	_req.raw.append(str, len);
	while (_req.raw.compare(0, 2, "\r\n") == 0)
		_req.raw.erase(0, 2);
}

const std::string& Client::getRequestRaw() { return _req.raw; }
Client::iter Client::getHost() { return _host; }
Client::iterL Client::getLocation() { return _location; }

void Client::setHost()
{
	iter it = getParentSocket()->getConf()->begin();
	iter end = getParentSocket()->getConf()->end();
	std::string host;
	
	if (_req.have("Host"))
	{
		host = _req.getHeader("Host");
		if (host.find(':') != std::string::npos)
			host = host.substr(0, host.find(':'));
		while (it != end)
		{
			if (it->serverName == host)
				break;
			++it;
		}
		if (it == end)
			it = getParentSocket()->getConf()->begin();
	}
	_host = it;
}


void Client::setLocation()
{
	iterL itL = _host->location.begin();
	iterL endL = _host->location.end();
	iterL mem = endL;
	std::string reqTarget = _req.target;
	size_t len = 0;
	while (itL != endL)
	{
		if (reqTarget.compare(0, itL->path.length(), itL->path) == 0 && len < itL->path.length())
		{
			if ((reqTarget.length() > itL->path.length() && (reqTarget[itL->path.length()] == '/' || itL->path == "/"))
				|| (reqTarget.length() == itL->path.length()))
			{
				len = itL->path.length();
				mem = itL;
			}
		}
		++itL;
	}
	_location = mem;
}



void Client::checkRequest() { _req.check(); }

int Client::recieveRequest(size_t requestLen)
{
	long ret;
	size_t total = requestLen;
	size_t r_size = 0;
	char buffer2[501];
	while (1)
	{
		if (total > 500)
			r_size = 500;
		else
			r_size = total;
		if (r_size == 0)
			return 0;
		total -= r_size;
		ret = recv(_fd, buffer2, r_size, 0);
		if (ret < 1)
			return 0;
		else
		{
			buffer2[ret] = 0;
			appendRequest(buffer2, ret);
			total += ret;
		}
	}
	return -1;
}

ssize_t		Client::getEmptyLine() const { return _req.blankline; }
void		Client::setEmptyLine(ssize_t emptyLine) { _req.blankline = emptyLine; }

void	Client::clearRequest()
{
	_req.clear();
	_res.clear();
	_req.blankline = NOT_SET;
	_req.bodyType = 0;
}

int	Client::getRequestStatus() const { return _req.status; }
int	Client::getContentLength() const { return _req.contentLength; }
const std::string& Client::getContentType() const {return _req.getHeader("Content-Type"); }

int		Client::getFd() const
{
	return _fd;
}

void	Client::setParentSocket(NetSocket* soc) { _parentSocket = soc; }
NetSocket*	Client::getParentSocket() const { return _parentSocket; }

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool Client::isClose()
{
	return _req.getHeader("Connection") == "close";
}

void	Client::setRemote(struct sockaddr_in*	sockaddr)
{
	_remoteAddress = inet_ntoa(sockaddr->sin_addr);
	_remotePort = sockaddr->sin_port;
}

const std::string& Client::getRemoteAddr() const { return _remoteAddress; }
int Client::getRemotePort() const { return _remotePort; }
Session& Client::getSession() { return _session; }
Request& Client::getRequest() { return _req; }
Response&	Client::getResponse() {return _res; }

const std::string&	Client::getResponseBackup() const {return _res._responseBackup; }


int readLine(size_t& start, const std::string& body, Client::BodyPart& part, const size_t& blank)
{
	size_t end = body.find("\r\n", start);
	size_t sep = body.find(": ", start);
	
	if (end == std::string::npos || sep == std::string::npos || end < sep || end > blank)
		return -1;
	
	std::pair<std::string, std::string> header;
	header.first = body.substr(start, sep - start);
	header.second = body.substr(sep + 2, end - sep - 2);
	part.headers[header.first] = header.second;
	start = end + 2;
	return 0;
}

int getPart(const std::string& body, const std::string& boundary, std::vector<Client::BodyPart>& parts, size_t& fin)
{
	static size_t start;
	size_t blank, end;
	Client::BodyPart bodyPart;
	int ret;
	size_t temp;
	
	temp = start;
	start = body.find(boundary, start);
	if (start == fin)
	{
		start = 0;
		return 1;
	}
	if (temp == 0 && start == std::string::npos)
	{
		start = 0;
		return 400;
	}
		
	else if (temp != 0 && start == std::string::npos)
	{
		start = 0;
		return 1;
	}
	start += boundary.length() + 2;
	end = body.find(boundary, start);
	if (end == std::string::npos)
	{
		start = 0;
		return 400;
	}
	blank = body.find("\r\n\r\n", start);
	if (blank == std::string::npos || blank > end)
	{
		start = 0;
		return 400;
	}
	do
	{
		ret = readLine(start, body, bodyPart, blank);
	}
	while (ret != -1);
	bodyPart.content = body.substr(blank + 4, end - blank - 4);
	parts.push_back(bodyPart);
	start = end;
	return 0;
}

int getFileName(const std::string& header, std::string& fileName)
{
	size_t start, end;
	start = header.find("filename=\"");
	
	if (start == std::string::npos)
		return -1;
	start += 10;
	end = header.find("\"", start);
	if (end == std::string::npos)
		return -1;
	fileName = header.substr(start, end - start);
	return 0;
}
