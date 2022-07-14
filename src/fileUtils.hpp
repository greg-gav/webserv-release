#ifndef fileUtils_h
#define fileUtils_h
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

bool	isDir(const std::string& file);
bool	isFileExist(const std::string& file);
size_t	getFileSize(const std::string& file);
bool	isOpenable(const std::string& file);

#endif
