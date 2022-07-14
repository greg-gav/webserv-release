#ifndef SESSION_HPP
#define SESSION_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <vector>

class Session
{
public:
    static std::map<std::string, int> Sessions;
    void			processCookie(const std::string& cookie);
    void			setId(std::string& id);
    std::string&	getId();
    int				getCount();
    void			newSession();
    std::string&	getCookieResponse();
private:
    std::string _rawCookieRequest;
    std::string _rawCookieResponse;
	std::string	_id;
    int         _count;
    
    std::string getNewId();
    std::string generateCookie();

    template <typename T>
  	std::string ToString ( T Number )
  	{
    	std::ostringstream ss;
    	ss << Number;
    	return ss.str();
  	}
};

std::vector<std::string> split(const std::string& s, char seperator);

#endif
