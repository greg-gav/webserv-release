#include "fileUtils.hpp"

bool	isDir(const std::string& file)
{
	struct stat	st;

	if (stat(file.c_str(), &st) == -1)
		return false;
	return S_ISDIR(st.st_mode);
}

bool	isFileExist(const std::string& file)
{
	int ret = access(file.c_str(), F_OK);
	if (ret == 0)
		return true;
	else if (errno == EACCES)
		return true;
	else
		return false;
}

size_t	getFileSize(const std::string& file)
{
	struct stat	buf;

	if (stat(file.c_str(), &buf) == -1)
		return -1;
	return buf.st_size;
}

bool	isOpenable(const std::string& file)
{
	return (access(file.c_str(), R_OK | W_OK) == 0);
}
