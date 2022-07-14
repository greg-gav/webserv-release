#ifndef Response_hpp
#define Response_hpp
#include <iostream>
#include <map>

struct Response
{

	void		setCode(int code, const std::string& body = "Hello world!");
	void		construct();
	std::string getBody();
	int 		getCode();
	void		setContentType(const std::string& type);
	const std::string& getContentType();
	void		addCookies(std::string&);
	void		clear();

	int			_code;
	std::string	_status;
	std::string _body;
	std::string raw;
	std::string _responseBackup;
	std::string _contentType;
	std::string _cookie;
};

#endif
