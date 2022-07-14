#include "Event.hpp"

int	addReadEvent(const int kq, const int socketFd)
{
	struct kevent	monEvent;

	memset(&monEvent, 0, sizeof(monEvent));
	EV_SET(&monEvent, socketFd, EVFILT_READ, EV_ADD, 0, 0, 0);
	if (kevent(kq, &monEvent, 1, NULL, 0, NULL) == -1)
		return -1;
	return 0;
}

int addWriteEvent(const int kq, const int socketFd)
{
	struct kevent	monEvent;

	memset(&monEvent, 0, sizeof(monEvent));
	EV_SET(&monEvent, socketFd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, 0);
	return kevent(kq, &monEvent, 1, NULL, 0, NULL);
}
