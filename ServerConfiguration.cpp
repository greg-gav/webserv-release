#include "ServerConfiguration.hpp"
#include "supply.hpp"
#include <unistd.h>

ServerConfiguration::ServerConfiguration()
{
	
}

ServerConfiguration::ServerConfiguration(const char* file)
:_confFileName(file)
{
	_checker.insert(std::pair<std::string, int>("autoindex", 3));
	_checker.insert(std::pair<std::string, int>("upload_folder", 3));
	_checker.insert(std::pair<std::string, int>("index", 3));
	_checker.insert(std::pair<std::string, int>("root", 3));
	_checker.insert(std::pair<std::string, int>("location", 2));
	_checker.insert(std::pair<std::string, int>("allow_methods", 3));
	_checker.insert(std::pair<std::string, int>("cgi", 2));
	_checker.insert(std::pair<std::string, int>("listen", 2));
	_checker.insert(std::pair<std::string, int>("server_name", 2));
	_checker.insert(std::pair<std::string, int>("server", 1));
	_checker.insert(std::pair<std::string, int>("max_body_size", 3));

	_servConfObj.clear();
}

ServerConfiguration::~ServerConfiguration()
{
	_file.close();
}

const std::string&	ServerConfiguration::getPath() const
{
	return _path;
}

void	ServerConfiguration::init(){
	char path[PATH_MAX];
	_path = getcwd(path, PATH_MAX);
	if (_path.empty())
		throw std::runtime_error("Work directory");
	openConf();
	parseConf();
	makeMap();

}



void	ServerConfiguration::openConf(){
	_file.open(_confFileName);
	if (_file.fail())
		throw std::runtime_error("Open file exception");
	if (_file.eof())
		throw std::runtime_error("Error file exception");
}


void	ServerConfiguration::makeMap(){
	std::vector<OneServConf>::iterator	it;
	std::vector<OneServConf>::iterator	it_last;
	std::string							sample;
	std::string							checkingSample;
	std::vector<OneServConf>			tempServVec;

	while (!_servConfObj.empty()){
		it = _servConfObj.begin();
		if (it == it_last)
			break;
		sample = it->address + ":" + it->portStr;
		tempServVec.push_back(*it);
		_servConfObj.erase(it);
		it_last = _servConfObj.end();
		while(it != it_last){
			checkingSample = it->address + ":" + it->portStr;
			if (!checkingSample.compare(sample)){
				tempServVec.push_back(*it);
				_servConfObj.erase(it);
				it_last = _servConfObj.end();
			}
			else
				++it;
		}
		_servers.insert(std::pair<std::string, std::vector<OneServConf> > (sample, tempServVec));
		tempServVec.clear();
	}
}

void	ServerConfiguration::parseConf()
{
	std::string 				temp;
	std::vector<std::string>	words;
	int							level = 1;

	while(std::getline(_file, temp)){
		temp = trim(temp);
		if (temp.size() == 0)
			continue ;
		words = errorComparator(temp, level);
		fillingValues(words);
		if (!((words.end() - 1)->compare("{")))
		level++;
		else if (!((words.end() - 1)->compare("}")))
		level--;
}
	} 




void	ServerConfiguration::fillingValues(const std::vector<std::string>& words){
	std::map<std::string, int>::iterator	it = _checker.begin();
	std::map<std::string, int>::iterator	it_end = _checker.end();
	size_t									i = 0;
	
	while (it != it_end){
		if (!words[0].compare(it->first))
			break ;
		i++;
		it++;
	}
	switch (i)
	{
	case SERVER:
		makeServer();
		break;
	case SERVER_NAME:
		makeServerName(words);
		break;
	case LISTEN:
		makeLister(words);
		break;
	case CGI:
		makeCGI(words);
		break;
	case ALLOW_METHODS:
		makeMethods(words);
		break;
	case LOCATION:
		makeLocation(words);
		break;
	case ROOT:
		makeRoot(words);
		break;
	case INDEX:
		makeIndex(words);
		break;
	case AUTOINDEX:
		makeAutoIndex();
		break;
	case UPLOAD_FOLDER:
		makeUploadFolder(words);
		break;
	case MAXSIZE:
		makeBodySize(words);
		break;
	default:
		break;
	}
}

void	ServerConfiguration::makeUploadFolder(const std::vector<std::string>& words){
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;
	std::vector<Location>::iterator it_last_loc = it_last->location.end() - 1;
	std::string::const_iterator it = words[1].end();

	if (*(--it) != '/'){
		std::string newWord(words[1]);
		newWord += '/';
		it_last_loc->uploadFolder = newWord;
	}
	else
		it_last_loc->uploadFolder = words[1];

	DIR* dir = opendir(it_last_loc->uploadFolder.c_str());
	if (dir) {
    	closedir(dir);
	} 
	else
		throw std::runtime_error("Upload folder doesn't exist");
}

void	ServerConfiguration::makeAutoIndex(){
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;
	std::vector<Location>::iterator it_last_loc = it_last->location.end() - 1;

	it_last_loc->autoIndex = true;

}

void	ServerConfiguration::makeLocation(const std::vector<std::string>& words){
	
	Location	newLocation;
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;

	if (words.size() != 3)
		throw std::runtime_error("Location path error");
	newLocation.path = words[1];
	newLocation.autoIndex = false;
	newLocation.allowMethods = 0;
	newLocation.maxBodySize = -1;
	it_last->location.push_back(newLocation);
}

void	ServerConfiguration::makeServer(){
	OneServConf	newServ;
	_servConfObj.push_back(newServ);
}

void	ServerConfiguration::makeBodySize(const std::vector<std::string>& words){
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;
	std::vector<Location>::iterator it_last_loc = it_last->location.end() - 1;

	it_last_loc->maxBodySize = atoi(words[1].c_str());

}

void	ServerConfiguration::makeRoot(const std::vector<std::string>& words){
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;
	std::vector<Location>::iterator it_last_loc = it_last->location.end() - 1;

	it_last_loc->root = words[1];

}

void	ServerConfiguration::makeIndex(const std::vector<std::string>& words){
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;
	std::vector<Location>::iterator it_last_loc = it_last->location.end() - 1;

	it_last_loc->index = words[1];
}


void	ServerConfiguration::makeServerName(const std::vector<std::string>& words){
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;
	
	it_last->serverName = words[1];
}

void	ServerConfiguration::makeLister(const std::vector<std::string>& words){
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;
	size_t 								pos;
	std::string							word(words[1]);

	pos = word.find(":");
	
	if (pos == std::string::npos)
		throw std::runtime_error("Incorrect ip or port is missing");
	it_last->address = word.substr(0, pos);
	
	it_last->port = atoi(word.substr(pos + 1, word.length() - pos).c_str());
	it_last->portStr = word.substr(pos + 1, word.length() - pos);
	if (validateIP(it_last->address))
		throw std::runtime_error("Incorrect ip addres");
	if (it_last->port < 1 || it_last->port > 65535){
		throw std::runtime_error("Incorrect port in ip address");
	}
}

void	ServerConfiguration::makeCGI(const std::vector<std::string>& words){
	std::vector<OneServConf>::iterator it_last = _servConfObj.end() - 1;

	it_last->cgi = words[1];
}

void	ServerConfiguration::makeMethods(const std::vector<std::string>& words){
	std::vector<Location>::iterator it_last	= ((_servConfObj.end() - 1)->location.end() - 1);
	std::vector<std::string>::const_iterator it_methods = words.begin() + 1;
	
	size_t	gFlag = 0;
	size_t	pFlag = 0;
	size_t	dFlag = 0;
	size_t	ptFlag = 0;

	while (it_methods != words.end()){
		if (!it_methods->compare("GET") && ++gFlag)
			it_last->allowMethods |= 1;
		else if (!it_methods->compare("POST") && ++pFlag)
			it_last->allowMethods |= 2;
		else if (!it_methods->compare("DELETE") && ++dFlag)
			it_last->allowMethods |= 4;
		else if (!it_methods->compare("PUT") && ++ptFlag)
			it_last->allowMethods |= 8;
		else
			throw std::runtime_error("Invalid allow method");
		it_methods++;
	}


	if (gFlag > 1 || pFlag > 1 || dFlag > 1 || ptFlag > 1){
	
		throw std::runtime_error("Incorrect allow methods");
	}
}

std::vector<std::string> ServerConfiguration::errorComparator(std::string key, const int level){
	std::map<std::string, int>::iterator 	it;
	std::vector<std::string>				words;
	std::string								temp;
	std::istringstream 						newStream(key);
	
    while (std::getline(newStream, temp, ' ')) {
        words.push_back(temp);
    }
	it = _checker.find(words[0]);
	if (it == _checker.end() || level != it->second){
		if (words.size() == 1 && !words[0].compare("}"))
			return words;
		throw std::runtime_error("Incorrect config file A.");
	}
	else if (!words[0].compare("autoindex")){
			if (words.size() != 1)
				throw std::runtime_error("Incorrect config file B.");
			return words;
	}
	else if (words.size() != 2){
		if (!words[0].compare("location") && words.size() == 3)
			return words;
		else if (!words[0].compare("allow_methods") && words.size() <= 5 && words.size() >= 2)
			return words;
		throw std::runtime_error("Incorrect config file C.");
	}
	
	return words;
}
	
std::map<std::string, std::vector<ServerConfiguration::OneServConf> >& ServerConfiguration::getServerMap()
{
	return _servers;
}


