#include "Session.hpp"

std::map<std::string, int> Session::Sessions;

void Session::processCookie(const std::string& cookie){
    std::vector<std::string> vec = split(cookie, ' ');
    std::pair<std::string, int> pair("", 0);
	for(std::vector<std::string>::iterator it = vec.begin(); it!= vec.end(); it++){
        std::string key, value;
        size_t positionOfEquals = it->find("=");
		if (positionOfEquals == std::string::npos)
			continue;
        key = it->substr(0, positionOfEquals);
        if(positionOfEquals != std::string::npos)
            value = it->substr(positionOfEquals + 1);
        if (key == "ID")
			pair.first = value;
        else if (key == "COUNT")
			pair.second = atoi(value.c_str());
    }
    if (pair.first != ""){
        std::map<std::string, int>::iterator it = Sessions.find(pair.first);
        if (it != Sessions.end())
            pair.second = ++Sessions[pair.first];
        else {
            pair.second++;
        }
    }
    else {
        pair.first = getNewId();
        pair.second = 1;
    }
    Sessions.insert(pair);
    _id = pair.first;
    _count = pair.second;
    _rawCookieResponse = generateCookie();
}

void Session::newSession(){
    _id = getNewId();
    _count = 1;
    Sessions[_id] = _count;
    _rawCookieResponse = generateCookie();
}

void Session::setId(std::string& id){
    _id = id;
}

std::string& Session::getId(){
    return _id;
}

int Session::getCount(){
    return _count;
}

std::string& Session::getCookieResponse(){
    return _rawCookieResponse;
}

std::vector<std::string> split(const std::string& s, char seperator)
{
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos - prev_pos) );
        if (substring.back() == ';')
            substring.pop_back();
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos-prev_pos));
    return output;
}

std::string Session::getNewId(){
   static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    int len = 8;
    tmp_s.reserve(len);
    srand (static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return tmp_s;
}

std::string Session::generateCookie(){
    std::string res =
    "Set-Cookie: ID=" + _id + "; Path=/; Max-Age=2592000;\n"
    "Set-Cookie: COUNT=" + ToString(_count) + "; Path=/; Max-Age=2592000;\n";
    return res;
}
