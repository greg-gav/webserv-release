#ifndef serverConfiguration_hpp
#define serverConfiguration_hpp
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>

#define GET 1
#define POST 2
#define DELETE 4
#define PUT 8

class Client;
class ServerConfiguration
{
public:
	struct Location
	{
		std::string					path;
		std::string					root;
		std::string					index;
		int							allowMethods;
		bool						autoIndex;
		std::string					uploadFolder;
		size_t						maxBodySize;
	};
	struct OneServConf
	{
		std::string					serverName;
		std::string					address;
		int							port;
		std::string					portStr;
		std::string					cgi;
		std::vector<Location>		location;
	};
	ServerConfiguration(const char* file);
	~ServerConfiguration();

	void							init();
	const std::string&				getPath() const;
	std::map<std::string, std::vector<OneServConf> >& getServerMap();
private:
	enum 						_configEnum {ALLOW_METHODS, AUTOINDEX, CGI, INDEX, LISTEN, LOCATION, MAXSIZE, ROOT, SERVER, SERVER_NAME, UPLOAD_FOLDER};
	const char*					_confFileName;
	std::ifstream				_file;
	std::map<std::string, int>	_checker;
	std::vector<OneServConf>	_servConfObj;
	std::map<std::string, std::vector<OneServConf> >	_servers;
	std::string					_path;
	
	ServerConfiguration();
	/* opening */
	void						openConf();
	/* parsing */
	void						parseConf();
	void						fillingValues(const std::vector<std::string>&);
	std::vector<std::string>	errorComparator(std::string, const int);
	void						makeMap();
	
	/* filling */
	void						makeServer();
	void						makeServerName(const std::vector<std::string>&);
	void						makeLister(const std::vector<std::string>&);
	void						makeCGI(const std::vector<std::string>&);
	void						makeMethods(const std::vector<std::string>&);
	void						makeLocation(const std::vector<std::string>&);
	void						makeRoot(const std::vector<std::string>&);
	void						makeIndex(const std::vector<std::string>&);
	void						makeAutoIndex();
	void						makeUploadFolder(const std::vector<std::string>&);
	void						makeBodySize(const std::vector<std::string>&);
};

#endif
