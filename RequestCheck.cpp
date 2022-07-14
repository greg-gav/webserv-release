#include "Request.hpp"

/*
 my_stoul
 checkStartLine
 caseInsensetiveCompare
 findCaseInsensetiveHeader
 findCaseInsensetive
 checkRequestHeaders
 strIsNum
 checkRequestBody
 check
*/

static unsigned long my_stoul (std::string const& str, size_t *idx = 0, int base = 10) {
	char *endp;
	unsigned long value = strtoul(str.c_str(), &endp, base);
	if (endp == str.c_str()) {
		throw std::invalid_argument("my_stoul");
	}
	if (value == ULONG_MAX && errno == ERANGE) {
		throw std::out_of_range("my_stoul");
	}
	if (idx) {
		*idx = endp - str.c_str();
	}
	return value;
}

int Request::checkStartLine()
{
	size_t pos = 0;
	size_t temp = pos;
	while (isupper(raw[pos]))
		++pos;
	if (pos == temp)
		return 1;
	temp = pos;
	while (raw[pos] == ' ')
		++pos;
	if (pos == temp)
		return 1;
	temp = pos;
	while (isprint(raw[pos]) && raw[pos] != ' ')
		++pos;
	if (pos == temp)
		return 1;
	temp = pos;
	while (raw[pos] == ' ')
		++pos;
	if (pos == temp)
		return 1;
	if (raw.compare(pos, 8, "HTTP/1.1") != 0)
		return 1;
	pos += 8;
	if (raw.compare(pos, 2, "\r\n") != 0)
		return 1;
	return 0;
}

static int caseInsensetiveCompare(const std::string& s1, size_t offset, const std::string& s2)
{
	for (size_t i = 0; i < s2.length(); i++)
	{
		if (tolower(s1[i + offset]) != tolower(s2[i]))
			return 0;
	}
	return 1;
}

size_t Request::findCaseInsensetiveHeader(const size_t end, ssize_t& pos, const std::string& header)
{
	std::string needle = "\r\n";
	needle.append(header);
	needle.append(":");
	for (size_t i = pos; i < end; i++)
	{
		if (raw[i] == '\r')
		{
			if (caseInsensetiveCompare(raw, i, needle))
			{
				i = i + needle.length();
				return i;
			}
		}
	}
	return std::string::npos;
}

size_t Request::findCaseInsensetive(const size_t end, ssize_t& pos, const std::string& header)
{
	std::string needle = header;

	for (size_t i = pos; i < end; i++)
	{
		if (raw[i] == needle[0])
		{
			if (caseInsensetiveCompare(raw, i, needle))
			{
				i = i + needle.length();
				return i;
			}
		}
	}
	return std::string::npos;
}

void Request::checkRequestHeaders()
{
	ssize_t pos = -1;
	
	if (blankline == NOT_SET)
	{
		blankline = raw.find("\r\n\r\n");
		if (blankline == NOT_SET)
		{
			if (raw.size() > MAX_HEADER_SIZE)
			{
				clear();
				status = REQUEST_ERROR;
				code = 431;
			}
			else
			{
				std::cout << "INCOMPLETE." << std::endl;
				status = REQUEST_INCOMPLETE;
			}
			return;
		}
		else if (checkStartLine())
		{
			clear();
			status = REQUEST_ERROR;
			code = 400;
			return;
		}
		posChunk = blankline + 4;
	}
	ssize_t pos0 = 0;
	pos = findCaseInsensetiveHeader(blankline, pos0, "transfer-encoding");
	if (pos != -1)
	{
		pos = findCaseInsensetive(raw.length(), pos, "chunked");
		if (pos != -1)
		{
			bodyType = CHUNKED;
			status = REQUEST_INCOMPLETE;
			return;
		}
	}
	
	pos = -1;
	pos = findCaseInsensetiveHeader(blankline, pos0, "content-length");
	if (pos != -1)
	{
		size_t pos2 = raw.find_first_of('\r', pos);
		if (pos2 != std::string::npos)
		{
			contentLength = atoi(raw.substr(pos, pos2 - pos).c_str());
			bodyType = FIXED;
			status = REQUEST_INCOMPLETE;
		}
		else
		{
			clear();
			status = REQUEST_ERROR;
			code = 400;
		}
		return;
	}

	if (raw.size() > static_cast<size_t>(blankline) + 4)
	{
		status = REQUEST_ERROR;
		code = 400;
		clear();
		return;
	}
	status = REQUEST_OK;
}

static bool strIsNum(const std::string& str)
{
	if (str.length() == 0)
		return false;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!(isdigit(str[i]) || (str[i] >= 'a' && str[i] <= 'f') || (str[i] >= 'A' && str[i] <= 'F')))
			return false;
	}
	return true;
}

void		Request::checkRequestBody()
{
	if (bodyType == FIXED)
	{
		std::cout << "ReqSiz = " << raw.size() << " EmL = " << (blankline + 4) << " _ConL = " << contentLength << std::endl;
		if (raw.size() < (static_cast<size_t>(blankline) + 4) + contentLength)
		{
			status = REQUEST_INCOMPLETE;
		}
		else if (raw.size() == (static_cast<size_t>(blankline) + 4) + contentLength )
			status = REQUEST_OK;
		else
			status = REQUEST_ERROR;
	}
	else
	{
		size_t	start, end;
		size_t	chunkSize;
		std::string line;
		while (1)
		{
			start = posChunk;
			end = raw.find("\r\n", start);
			if (end == std::string::npos)
			{
				std::cout << "INCOMPLETE: Size end not found" << std::endl;
				status = REQUEST_INCOMPLETE;
				return;
			}
			line = raw.substr(start, end - start);
			if (strIsNum(line) == false)
			{
				std::cout << "ERROR: Size not a number" << std::endl;
				status = REQUEST_ERROR;
				return;
			}
			chunkSize = my_stoul(line, NULL, 16);
			start = end + 2;
			end = raw.find("\r\n", start);
			if (end == std::string::npos)
			{
				std::cout << "INCOMPLETE: Size end not found" << std::endl;
				status = REQUEST_INCOMPLETE;
				return;
			}
			if (end - start != chunkSize)
			{
				std::cout << "ERROR: Content size wrong" << std::endl;
				status = REQUEST_ERROR;
				return;
			}
			if (chunkSize == 0)
			{
				status = REQUEST_OK;
				return;
			}
			posChunk = end + 2;
		}
	}
}

void Request::check()
{
	if (bodyType == 0)
	{
		checkRequestHeaders();
		if (bodyType != 0)
			checkRequestBody();
	}
	else
		checkRequestBody();
}
