#include "Response.hpp"

void fileToStr(const std::string& file, std::string& content);
std::map<int, std::string> createMap()
{
	std::map<int, std::string> codes;
	codes[200] = "OK";
	codes[201] = "Created";
	codes[204] = "No Content";
	codes[400] = "Bad request";
	codes[403] = "Forbidden";
	codes[404] = "Not Found";
	codes[405] = "Method No Allowed";
	codes[411] = "Length Required";
	codes[413] = "Payload Too Large";
	codes[500] = "Internal Server Error";
	return codes;
}

void	Response::setCode(int code, const std::string& body)
{
	_body.clear();
	_body.shrink_to_fit();
	_status.clear();
	_status.shrink_to_fit();
	static std::map<int, std::string> codes(createMap());
	_code = code;
	if (_code >= 300)
	{
		try {
			fileToStr("./web/dist/HTTP" + std::to_string(_code) + ".html", _body);
		} catch (std::exception) {
			_body = "Error " + std::to_string(_code);
		}
	}
	else if (_code == 201 || _code == 204)
		_body = "";
	else
		_body = body;
	_status = codes[_code];
}

void		Response::setContentType(const std::string& type)
{
	_contentType = type;
}

void		Response::construct()
{
	raw.clear();
	_responseBackup.clear();
	raw.assign("HTTP/1.1 " + std::to_string(_code) + " " + _status + "\n");
	raw.append(_cookie);
	if (_body.find("\r\n\r\n") != std::string::npos){ // if cgi body contains part of head
		raw.append(_body, 0, _body.find("\r\n\r\n") + 2);
		_body.erase(0, _body.find("\r\n\r\n") + 4);
	}
	if (_status != "Created")
		raw.append("Content-Length: " + std::to_string(_body.length()) + "\n");
	if (_contentType.size())
		raw.append("Content-Type: " + _contentType + "\n");//application/octet-stream
	raw.append("\n" + _body);
	_responseBackup = raw;
}

std::string		Response::getBody(){
	return _body;
}

int		Response::getCode(){
	return _code;
}

void Response::addCookies(std::string& cookie){
	_cookie = cookie;
}

const std::string& Response::getContentType()
{
	return _contentType;
}

void Response::clear()
{
	_code = 0;
	_status.clear();
	_status.shrink_to_fit();
	_body.clear();
	_body.shrink_to_fit();
	raw.clear();
	raw.shrink_to_fit();
	_contentType.clear();
	_contentType.shrink_to_fit();
	_cookie.clear();
	_cookie.shrink_to_fit();
	_responseBackup.clear();
	_responseBackup.shrink_to_fit();
}
