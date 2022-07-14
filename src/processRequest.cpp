#include "CgiCore.hpp"
#include <dirent.h>

static const std::string getExt(const std::string& path)
{
	size_t ext = path.rfind('.');
	size_t slash = path.rfind('/');
	std::string res;
	
	if (ext == std::string::npos || (slash != std::string::npos && ext < slash))
		return res;
	res = path.substr(ext + 1);
	return res;
}

static std::string	autoIndex(const std::string& dirName, const std::string& target)
{
	DIR				*dir;
	struct dirent	*entry;
	std::string		value;

	dir = opendir(dirName.c_str());
	if (!dir)
		return "";
	value.assign("<html>\n<head>\n<meta charset=\"utf-8\">\n"
			"<title>Directory Listing</title>\n</head>\n<body>\n<h1>"
			+ target + "</h1>\n<ul>");
	while ((entry = readdir(dir)) != NULL)
	{
		value.append("<li><a href=\"");
		value.append(target);
		if (value.back() != '/')
			value.append("/");
		value.append(entry->d_name);
		if(entry->d_type == DT_DIR)
			value.append("/");
		value.append("\"> ");
		value.append(entry->d_name);
		if(entry->d_type == DT_DIR)
			value.append("/");
		value.append("</a></li>\n");
	}
	value.append("</ul></body></html>");
	closedir(dir);
	return value;
}

static bool isKnownType(const std::string& type)
{
	std::string types;
	types = "_png_jpg_html_cgi_py_php_bla_";
	
	if (types.find(type) != std::string::npos)
		return true;
	return false;
}

void fileToStr(const std::string& file, std::string& content)
{
	std::ifstream fstream;
	fstream.open(file.c_str());
	if (fstream.fail())
		throw std::runtime_error(file + ": Open file exception");
	if (fstream.eof())
		throw std::runtime_error(file + ": Error file exception");
	std::stringstream strStream;
	strStream << fstream.rdbuf();
	content = strStream.str();
	fstream.close();
}

static int strToFile(const std::string& file, std::string& content)
{
	std::ofstream ofs(file, std::ofstream::trunc);
	if (ofs.fail() || ofs.eof())
		return 500;
	ofs << content;
	ofs.close();
	return 201;
}

int Client::processRequest(int kq)
{
	std::cout << "Process request: " << getRequestStatus() << std::endl;
	if (getRequestStatus() == REQUEST_OK)
	{
		int code;
		code = _req.parseRequest();
		std::cout << "Code: " << code << std::endl;
		if (code)
		{
			_res.setCode(code);
			_res.construct();
			_req.clear();
			addWriteEvent(kq, getFd());
			return 0;
		}
		setHost();
		setLocation();
		Client::iterL mem = getLocation();
		if (mem == getHost()->location.end())
		{
			std::cout << "Loc not found"<< std::endl;
			_res.setCode(404);
			_res.construct();
			_req.clear();
			addWriteEvent(kq, getFd());
			return 0;
		}
		_req.setQuery();
		_req.setPath(mem);
		std::cout << "Path: " << _req.path << std::endl;
		//managing cookies
		if (_req.have("Cookie"))
			getSession().processCookie(_req.getHeader("Cookie"));
		else
			getSession().newSession();
		
		_res.addCookies(getSession().getCookieResponse());
		if (!(_req.method & mem->allowMethods))
		{
			std::cout << "Session: BAD" << std::endl;
			_res.setCode(405);
			_res.construct();
			_req.clear();
			addWriteEvent(kq, getFd());
			return 0;
		}
		if (_req.method == M_GET || _req.method == M_POST)
		{
			std::string body;
			if (isDir(_req.path))
			{
				if (mem->autoIndex)
				{
					body = autoIndex(_req.path, _req.target);
					_res.setCode(200, body);
				}
				else
					_res.setCode(403);
			}
			else if (isFileExist(_req.path) && !isOpenable(_req.path))
				_res.setCode(403);
			else if (isFileExist(_req.path) && isOpenable(_req.path)
					 && (getExt(_req.path).length() == 0 || !isKnownType(getExt(_req.path))))
			{
				_res.setContentType("application/octet-stream");
				fileToStr(_req.path, body);
				_res.setCode(200, body);
			}
			else
			{
				try
				{
					if (!isFileExist(_req.path))
					{
						if (_req.method == M_GET)
							_res.setCode(404);
						else
						{
							if (isDir(_req.path))
								_res.setCode(500);
							else if (isFileExist(_req.path) && !isOpenable(_req.path))
								_res.setCode(500);
							else
							{
								strToFile(_req.path, _req.body);
								_res.setCode(201);
							}
						}
					}
					else if (hasEnding(_req.path, ".cgi") || hasEnding(_req.path, ".py")
						|| hasEnding(_req.path, ".php") || hasEnding(_req.path, ".bla")){
						CgiCore cgi(*this);
						int cgiFailure = cgi.ExecuteCgi(_req.path, body);
						if (!isFileExist(_req.path) || !isOpenable(_req.path))
							_res.setCode(404);
						else if (cgiFailure)
							_res.setCode(500);
						else
							_res.setCode(200, body);
					}
					else {
						if (_req.body.size() > mem->maxBodySize)
						{
							_res.setCode(413);
						}
						else
						{
							fileToStr(_req.path, body);
							_res.setCode(200, body);
						}
					}
				}
				catch (std::exception& e)
				{
					_res.setCode(404);
				}
			}
			_res.construct();
			_req.clear();
			addWriteEvent(kq, getFd());
		}
		else if (_req.method == M_PUT)
		{
			if (isDir(_req.path))
				_res.setCode(500);
			else if (isFileExist(_req.path) && !isOpenable(_req.path))
				_res.setCode(500);
			else
			{
				strToFile(_req.path, _req.body);
				_res.setCode(201);
			}
			_res.construct();
			_req.clear();
			addWriteEvent(kq, getFd());
		}
		else if (_req.method == M_DELETE)
		{
			if (!isFileExist(_req.path))
				_res.setCode(404);
			else if (isDir(_req.path))
				_res.setCode(403);
			else if (isFileExist(_req.path) && !isOpenable(_req.path))
				_res.setCode(403);
			else
			{
				if (std::remove(_req.path.c_str()))
					_res.setCode(500);
				else
					_res.setCode(204);
			}
			_res.construct();
			_req.clear();
			addWriteEvent(kq, getFd());
		}
	}
	else if (getRequestStatus() == REQUEST_INCOMPLETE)
		return 0;
	else if (getRequestStatus() == REQUEST_INCOMPLETE)
		return 0;
	else
	{
		_res.setCode(getRequestStatus());
		_res.construct();
		_req.clear();
		addWriteEvent(kq, getFd());
		return 0;
	}
	return 0;
}


