#ifndef SUPPLY
#define SUPPLY

#include <vector>
#include <string>
#include <iostream>
#include <regex>

std::vector<std::string>	split(std::string string, const char delimiter){
	size_t						i = 0;
	std::vector<std::string>	list;
	size_t						pos = string.find(delimiter);

	while (pos != std::string::npos){
		list.push_back(string.substr(i, pos));
		string.erase(i, pos + 1);
		pos = string.find(delimiter);
	}
	
	list.push_back(string.substr(i, string.length()));

	return list;
}

bool isNumber(const std::string &str)
{
    return !str.empty() && (str.find_first_not_of("[0123456789]") == std::string::npos);
}


bool		validateIP(std::string ip){
	std::vector<std::string>	list = split(ip, '.');
	

	if (list.size() != 4)
		return true;
	for (int i = 0; i < 4; i++){
		
		if (!isNumber(list[i]) || atoi(list[i].c_str()) > 255 || atoi(list[i].c_str()) < 0) {
            return true;
        }
	}
	return false;
}


std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    str.erase(0, str.find_first_not_of(chars));
    return str;
}
 
std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}
 
std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    return ltrim(rtrim(str, chars), chars);
}

#endif
