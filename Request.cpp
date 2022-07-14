#include "Request.hpp"

Request::Request()
:blankline(NOT_SET),
bodyType(0){}
Request::Request(const std::string& request)
:raw(request) {}
Request::~Request() {}
Request::Request(const Request& source) { *this = source; }

int Request::readLine(size_t& start)
{
	size_t end = raw.find("\r\n", start);
	size_t sep = raw.find(": ", start);
	
	if (end == std::string::npos || sep == std::string::npos || end < sep || end > blankline)
		return -1;
	std::pair<std::string, std::string> header;
	header.first = raw.substr(start, sep - start);
	header.second = raw.substr(sep + 2, end - sep - 2);
	_headers[header.first] = header.second;
	std::cout << header.first << ": " << header.second << std::endl;
	start = end + 2;
	return 0;
}



bool	Request::have(const std::string& name) const
{
	return _headers.find(name) != _headers.end();
}

const std::string&	Request::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	static const std::string nuls = "";
	
	if (it != _headers.end())
		return it->second;
	else
		return nuls;
}

void		Request::setPath(const std::string& path) { this->path = path; }

std::map<std::string, std::string>& Request::getAllHeaders(){
	return _headers;
}

void	Request::setQuery()
{
	size_t pos = target.find("?");
	if (pos != std::string::npos)
	{
		query = target.substr(pos + 1);
		target = target.substr(0, pos);
	}
}

void	Request::setPath(std::vector<ServerConfiguration::Location>::iterator mem)
{
	std::string temp_target, temp_root, temp_path;
	
	if (target.size() > 2 && target.back() == '/')
		target = target.substr(0, target.size() - 1);
	if (mem->root.back() == '/' && mem->root.size() > 2)
		mem->root = mem->root.substr(0, mem->root.size() - 1);
	if (mem->path.back() == '/' && mem->path.size() > 2)
		mem->path = mem->path.substr(0, mem->path.size() - 1);
		
	path.assign(mem->root);
	
	if (target == mem->path)
	{
		if (!mem->autoIndex)
			path.append("/" + mem->index);
	}
	else
	{
		if (mem->path.back() == '/')
			path.append("/" + target.substr(mem->path.length(), target.length() - mem->path.length()));
		else
			path.append(target.substr(mem->path.length(), target.length() - mem->path.length()));
	}
}

void Request::clear()
{
	method = 0;
	target.clear();
	target.shrink_to_fit();
	version.clear();
	version.shrink_to_fit();
	blankline = NOT_SET;
	raw.clear();
	raw.shrink_to_fit();
	path.clear();
	path.shrink_to_fit();
	body.clear();
	body.shrink_to_fit();
	bodyType = 0;
	query.clear();
	query.shrink_to_fit();
	status = 0;
	posChunk = 0;
	code = 0;
	contentLength = 0;
	_headers.clear();
}
