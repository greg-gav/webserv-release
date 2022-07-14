#ifndef Request_hpp
#define Request_hpp
#include <iostream>
#include <map>
#include <vector>
#include <cstdio>
#include "ServerConfiguration.hpp"
#include "fileUtils.hpp"
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <climits>
#include <cerrno>
#define M_GET 1
#define M_POST 2
#define M_DELETE 4
#define M_PUT 8
#define M_WRONG -1
#define M_NULL 0
#define NOT_SET std::string::npos
#define REQUEST_OK 0
#define REQUEST_INCOMPLETE 2
#define REQUEST_ERROR -1
#define NOT_SET std::string::npos
#define FIXED 1
#define CHUNKED 2
#define MAX_HEADER_SIZE 4096

struct Request
{
	Request();
	Request(const std::string& request);
	Request(const Request& source);
	~Request();
	
	int readLine(size_t& pos);

	void				setPath(const std::string& path);
	void				setQuery();
	void				setPath(std::vector<ServerConfiguration::Location>::iterator mem);
	
	const std::string&					getHeader(const std::string& key) const;
	std::map<std::string, std::string>& getAllHeaders();
	bool								have(const std::string& name) const;
	void								clear();
	int									parseRequest();
	int									checkStartLine();
	void	checkRequestBody();
	void	check();
	void	checkRequestHeaders();
	size_t	findCaseInsensetiveHeader(const size_t end, ssize_t& pos, const std::string& header);
	size_t	findCaseInsensetive(const size_t end, ssize_t& pos, const std::string& header);
	int									method;
	std::string							target;
	std::string							version;
	size_t								blankline;
	std::string							raw;
	std::string							path;
	std::string							body;
	int									bodyType;
	std::string							query;
	int									status;
	size_t								posChunk;
	int									code;
	int									contentLength;
private:
	int									parseStartLine();
	std::map<std::string, std::string>	_headers;
};

#endif /* Request_hpp */
