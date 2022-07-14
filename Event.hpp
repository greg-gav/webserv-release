#ifndef Event_hpp
#define Event_hpp
#include <sys/event.h>
#include <iostream>

int	addReadEvent(const int kq, const int socketFd);
int addWriteEvent(const int kq, const int socketFd);
#endif
