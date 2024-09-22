#ifndef SOCKET_TOKEN_H_
#define SOCKET_TOKEN_H_
#include <OCTCommu.h>

#ifdef os_is_win
#include <wtypes.h>
#include <winsock2.h>
#endif /// !

namespace oct_commu
{
	class SocketToken
	{
	public:
		SocketToken()
		{
#ifdef os_is_win
			WORD wVersionRequested = MAKEWORD(2, 2);
			WSADATA wsaData;
			WSAStartup(wVersionRequested, &wsaData);

#endif /// !os_is_win

		}
		~SocketToken()
		{
#ifdef os_is_win
			WSACleanup();
#endif /// !os_is_win
		}
	};
}

#endif /// !SOCKET_TOKEN_H_