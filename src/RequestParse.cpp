#include "Request.hpp"

/*
 parseStartLine
 parseRequest
 */

int Request::parseStartLine()
{
	using	std::string;
	size_t	sep, sep2, end;
	
	sep = raw.find(' ');
	end = raw.find("\r\n");
	if (sep == string::npos || end == string::npos || sep > end)
		return 400;
	if (raw.compare(0, sep, "GET") == 0)
		method = M_GET;
	else if (raw.compare(0, sep, "POST") == 0)
		method = M_POST;
	else if (raw.compare(0, sep, "PUT") == 0)
		method = M_PUT;
	else if (raw.compare(0, sep, "DELETE") == 0)
		method = M_DELETE;
	else
	{
		method = M_WRONG;
		return 405;
	}
	while (raw[sep] == ' ')
		++sep;
	sep2 = raw.find(' ', sep);
	if (sep2 == string::npos || sep2 > end)
		return 400;
	target = raw.substr(sep, sep2 - sep);
	while (raw[sep2] == ' ')
		++sep2;
	version = raw.substr(sep2, end - sep2);
	sep = raw.find("\r\n", sep) + 2;
	return 0;
}

int	Request::parseRequest()
{
	int ret = parseStartLine();
	if (ret)
		return ret;
	blankline = raw.find("\r\n\r\n");
	size_t pos = raw.find("\r\n") + 2;

	while (readLine(pos) != -1);
	
	if (method == M_POST || method == M_PUT)
	{
		std::map<std::string, std::string>::iterator it = _headers.begin();
		std::map<std::string, std::string>::iterator end = _headers.end();
		while (it != end)
		{
			std::cout << ">" << it->first << ": " << it->second << "<" << std::endl;
			++it;
		}
		
		std::map<std::string, std::string>::iterator itM;
		itM = _headers.find("Transfer-Encoding");
		if (itM != _headers.end() && itM->second.find("chunked") != std::string::npos)
		{
			size_t start = raw.find("\r\n\r\n") + 2;
			size_t end;
			std::string chunk;
			do
			{
				start = raw.find("\r\n", start) + 2;
				start = raw.find("\r\n", start) + 2;
				end = raw.find("\r\n", start);
				chunk = raw.substr(start, end - start);
				body.append(chunk);
			}
			while (chunk.size());
		}
		else
		{
			itM = _headers.find("Content-Length");
			if (itM != _headers.end())
			{
				size_t len = atoi(itM->second.c_str());
				body = raw.substr(blankline + 4, len);
				if (len == 0)
					return 411;
			}
			else
				return 400;
		}
	}
	return 0;
}
