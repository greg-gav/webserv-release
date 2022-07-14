#include "CgiCore.hpp"

CgiCore::CgiCore(Client& client) {

	_client = client;
	_req = client.getRequest();
	_body = _req.body;
	_path = _req.path;
	char path[PATH_MAX];
	_wd = getcwd(path, PATH_MAX);
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["SERVER_SOFTWARE"] = "webserv/1.0";
    _env["REDIRECT_STATUS"] = "200"; // needed by php-cgi
	_env["SERVER_NAME"] = _client.getParentSocket()->getConf()->begin()->address;
	_env["SERVER_PORT"] = ToString(_client.getParentSocket()->getConf()->begin()->port);
	_env["CONTENT_LENGTH"] = ToString(_req.body.length());
	_env["CONTENT_TYPE"] = _req.getHeader("Content-Type");
    _env["QUERY_STRING"] = _req.query;
    _env["REMOTE_ADDR"] = _client.getRemoteAddr();
	_env["REQUEST_METHOD"] = SetMethodAsStr();
    _env["SCRIPT_NAME"] = _req.target;
	_env["PATH_INFO"] = _req.target;
	_env["REQUEST_URI"] = _req.target;
	_env["SCRIPT_FILENAME"] = getScriptName();
    _env["PATH_TRANSLATED"] = getPathTranslated();
    _env["UPLOAD_DIR"] = _wd + "/" + _client.getLocation()->uploadFolder;
    _env["REMOTE_HOST"] = ""; 
    _env["REMOTE_USER"] = ""; 
    _env["AUTH_TYPE"] = "";
	addHttpHeaders(_req.getAllHeaders());
} 

void CgiCore::addHttpHeaders(EnvMap& hm){
	for (EnvMapIter it = hm.begin(); it != hm.end(); ++it){
		std::string key = "HTTP_" + it->first;
		std::string value = it->second;
		std::replace(key.begin(), key.end(), '-', '_');
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		_env[key] = value;
	}
}

const char* CgiCore::SetMethodAsStr(){
	const char* methodGET = "GET";
	const char* methodPOST = "POST";
	
	switch (_req.method)
	{
	case M_GET:
		return methodGET;
	case M_POST:
		return methodPOST;
	default:
		return methodGET;
	}
}

char**		CgiCore::EnvAsArray() const {
    char** env = new char*[_env.size() + 1];
    int i = 0;
    for (EnvMapIter it = _env.begin(); it != _env.end(); ++it){
        std::string elem = it->first + "=" + it->second;
        env[i] = new char[elem.size() + 1];
        env[i] = strcpy(env[i], elem.c_str());
        ++i;
    }
    env[_env.size()] = NULL;
    return env;
}

char**		CgiCore::GetExecArgs(std::string cgiPath) const {
	char** args = new char*[3];
	args[0] = new char[cgiPath.length() + 1];
	args[0] = strcpy(args[0], cgiPath.c_str());
	args[1] = new char[getScriptName().length() + 1];
	args[1] = strcpy(args[1], getScriptName().c_str());
	args[2] = NULL;
    return args;
}

int CgiCore::ExecuteCgi(const std::string &scriptName, std::string& newBody) {
    char** env;
	char** args;

    try {
		env = EnvAsArray();
		if (hasEnding(scriptName, ".py"))
			args = GetExecArgs(std::string("/usr/bin/python3"));
		else if (hasEnding(scriptName, ".php"))
			args = GetExecArgs(std::string("./php-cgi"));
		else if (hasEnding(scriptName, ".bla")){
			args = GetExecArgs(std::string(_wd + "/YoupiBanane/cgi_test"));
		}
		else
			args = GetExecArgs(getScriptName().c_str());
	}
	catch (std::bad_alloc &e){
        std::cerr << e.what() << std::endl;
		return (CGI_ERROR);
    }

	int launchError = launchExecve(env, args, newBody);

	DeleteArgsArray(env);
	DeleteArgsArray(args);
	if (launchError)
		return CGI_ERROR;

    return CGI_OK;
}

int CgiCore::launchExecve(char** env, char** args, std::string& newBody){
    pid_t pid;
	int readfd[2];
	int writefd[2];
	
	if (pipe(readfd) == -1 || pipe(writefd) == -1){
		std::cerr << "Pipe error occured." << std::endl;
		return(CGI_ERROR);
	}
	pid = fork();
	if (pid == -1)
	{
		std::cerr << "Fork error occured." << std::endl;
		return(CGI_ERROR);
	}
	else if (pid == 0)
	{
		dup2(readfd[1], STDOUT_FILENO);
		dup2(writefd[0], STDIN_FILENO);
		close(readfd[0]);
		close(writefd[1]);
		close(readfd[1]);
		close(writefd[0]);
		if (chdir(getWorkingDirectory().c_str()))
			return (CGI_ERROR);
		if (execve(*args, args, env)) {
			return (CGI_ERROR);
		}
	}
	else
	{
		close(readfd[1]);
		close(writefd[0]);
		pid_t pid2 = fork();
		if (pid2 == -1)
		{
			std::cerr << "Fork error occured." << std::endl;
			return(CGI_ERROR);
		}
		if (pid2 == 0)
		{
			close(readfd[0]);
			write(writefd[1], _body.c_str(), _body.size());
			close(writefd[1]);
			exit(0);
		}
		else
		{
			close(writefd[1]);
			char	buffer[PIPE_BUF] = {0};
			ssize_t ret = 1;
			while (ret > 0)
			{
				memset(buffer, 0, PIPE_BUF);
				ret = read(readfd[0], buffer, PIPE_BUF - 1);
				newBody += buffer;
			}
			close(readfd[0]);
			waitpid(pid2, NULL, 0);
			waitpid(pid, NULL, 0);
		}
		
	}
	return CGI_OK;
}

std::string CgiCore::getPathTranslated(){
	char path[PATH_MAX];
	char* cwdResult = NULL;
	if (!_path.empty() && _path.at(0) == '.')
		_path.erase(0, 1);
	cwdResult = getcwd(path, PATH_MAX);
	std::string retPath;
	if (cwdResult == NULL)
		return retPath;
	retPath = cwdResult + _path;
	return retPath;
}

std::string       CgiCore::getWorkingDirectory(){
	char path[PATH_MAX];
	char* cwdResult = NULL;
	cwdResult = getcwd(path, PATH_MAX);
	std::string wd;
	if (cwdResult == NULL)
		return wd;
	wd = cwdResult + _path;
	if (wd.find_last_of('/') != std::string::npos)
		wd.erase(wd.find_last_of('/'));
	return wd;
}

std::string       CgiCore::getScriptName() const {
	if (_path.find_last_of('/') != std::string::npos) {
		std::string wd = _path.substr(_path.find_last_of('/') + 1);
		return wd;
	}
	return _path;
}

void DeleteArgsArray(char **arr){
	for (size_t i = 0; arr[i]; ++i)
		delete[] arr[i];
	delete[] arr;
}

CgiCore::~CgiCore() {}
