#ifndef CGI_HPP
#define CGI_HPP

#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <map>
#include <unistd.h>
#include <sstream>
#include "Client.hpp"

#define CGI_OK 0
#define CGI_ERROR 1

class CgiCore
{
public:
    ~CgiCore();
    CgiCore(Client&); //setup env
    int ExecuteCgi(const std::string &scriptName, std::string& body); //return status
private:
	typedef std::map<std::string, std::string> EnvMap;
	typedef std::map<std::string, std::string>::const_iterator EnvMapIter;
    CgiCore();
    CgiCore(CgiCore&);
    CgiCore&    operator=(CgiCore&);
    char**		EnvAsArray() const;
    char**		GetExecArgs(std::string cgiPath) const;
    const char* SetMethodAsStr();
    std::string getPathTranslated();
    std::string getWorkingDirectory();
    std::string getScriptName() const;
    int         launchExecve(char** env, char** args, std::string&);
    void        addHttpHeaders(EnvMap&);

    EnvMap		_env;
    std::string	_body;
    std::string _path;
    Client      _client;
    Request     _req;
	std::string	_wd;

	template <typename T>
  	std::string ToString ( T Number )
  	{
    	std::ostringstream ss;
    	ss << Number;
    	return ss.str();
  	}
};

void DeleteArgsArray(char **arr);

#endif
