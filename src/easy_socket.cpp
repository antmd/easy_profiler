/**
Lightweight profiler library for c++
Copyright(C) 2016  Sergey Yagovtsev

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
**/

#include "profiler/easy_socket.h"


#ifndef _WIN32
#include <strings.h>
#include <errno.h>

int EasySocket::bind(uint16_t portno)
{
    if(m_socket < 0 ) return -1;
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    return ::bind(m_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
}

void EasySocket::flush()
{
    if (m_socket){
        close(m_socket);  
    }
    if (m_replySocket != m_socket){
        close(m_replySocket);
    }
    m_socket = 0;
    m_replySocket = 0;
}
void EasySocket::init()
{
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
}

EasySocket::EasySocket()
{
    init();
}

EasySocket::~EasySocket()
{
    flush();
}

int EasySocket::send(const void *buf, size_t nbyte)
{
    if(m_replySocket <= 0)  return -1;
    int res = ::write(m_replySocket,buf,nbyte);
    checkResult(res);
    return res;
}

int EasySocket::receive(void *buf, size_t nbyte)
{
    if(m_replySocket <= 0) return -1;
    int res = ::read(m_replySocket,buf,nbyte);
    checkResult(res);
    if (res == 0){
        m_state = CONNECTION_STATE_DISCONNECTED;
    }
    return res;
}

int EasySocket::listen(int count)
{
    if(m_socket < 0 ) return -1;
    int res = ::listen(m_socket,count);
    checkResult(res);
    return res;
}

int EasySocket::accept()
{
    if(m_socket < 0 ) return -1;
    m_replySocket = ::accept(m_socket,nullptr,nullptr);

    checkResult(m_replySocket);

    return m_replySocket;
}

bool EasySocket::setAddress(const char *serv, uint16_t portno)
{
    server = gethostbyname(serv);
    if (server == NULL) {
        return false;
        //fprintf(stderr,"ERROR, no such host\n");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    return true;
}

int EasySocket::connect()
{
    if (server == NULL || m_socket <=0 ) {
        return -1;
        //fprintf(stderr,"ERROR, no such host\n");
    }
    int res = ::connect(m_socket,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if(res == 0){

        struct timeval tv;

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

        m_replySocket = m_socket;
    }
    return res;
}
#include <string.h>
void EasySocket::checkResult(int result)
{
    if(result >= 0){
        m_state = CONNECTION_STATE_SUCCESS;
        return;
    }else if(result == -1){
        //printf("Errno: %s\n", strerror(errno));
        switch(errno){
        case ECONNABORTED:
        case ECONNRESET:
            m_state = CONNECTION_STATE_DISCONNECTED;
            break;
        default:
            break;
        }

    }
}

#else

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

void EasySocket::checkResult(int result)
{
    if (result >= 0){
        m_state = CONNECTION_STATE_SUCCESS;
        return;
    }
    else if (result == -1){
        int error_code = WSAGetLastError();
        //printf("Errno: %s\n", strerror(errno));
        switch (error_code){
        case WSAECONNABORTED:
        case WSAECONNRESET:
            m_state = CONNECTION_STATE_DISCONNECTED;
            break;
        default:
            break;
        }

    }
}

void EasySocket::flush()
{
    if (m_socket){
        closesocket(m_socket);  
    }
    if (m_replySocket != m_socket){
        closesocket(m_replySocket);
    }
    m_socket = INVALID_SOCKET;
    m_replySocket = INVALID_SOCKET;
}
void EasySocket::init()
{
    if (wsaret == 0)
    {
        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);;
        if (m_socket == INVALID_SOCKET) {
            return; 
        }
    }
    
    u_long iMode = 0;//0 - blocking, 1 - non blocking
    ioctlsocket(m_socket, FIONBIO, &iMode);

    int opt = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
}

EasySocket::EasySocket()
{
    // socket
    WSADATA wsaData;
    wsaret = WSAStartup(0x101, &wsaData);
    init();
}

EasySocket::~EasySocket()
{
    flush();
    if (wsaret == 0)
        WSACleanup();
}

int EasySocket::send(const void *buf, size_t nbyte)
{
    if (m_replySocket <= 0){
        return -1;
    }

    int res = ::send(m_replySocket, (const char*)buf, nbyte, 0);
    checkResult(res);
    return res;
}

#include <stdio.h>

int EasySocket::receive(void *buf, size_t nbyte)
{
    if (m_replySocket <= 0){
        return -1;
    }

    int res = ::recv(m_replySocket, (char*)buf, nbyte, 0);
    checkResult(res);
    if (res == 0){
        m_state = CONNECTION_STATE_DISCONNECTED;
    }

    /**
    if (res == SOCKET_ERROR)
    {
        LPWSTR *s = NULL;
        int err = WSAGetLastError();
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&s, 0, NULL);
        printf("%S\n", s);
        LocalFree(s);
    }
    /**/
    return res;
}

bool EasySocket::setAddress(const char *serv, uint16_t portno)
{
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int iResult;
    char buffer[20] = {};
    _itoa(portno, buffer, 10);
    iResult = getaddrinfo(serv, buffer, &hints, &result);
    if (iResult != 0) {
        return false;
    }

    return true;
}

int EasySocket::connect()
{
    if (!m_socket || !result){
        return -1;
    }

    // Connect to server.
    auto iResult = ::connect(m_socket, result->ai_addr, (int)result->ai_addrlen);
    checkResult(iResult);
    if (iResult == SOCKET_ERROR) {
        return iResult;
    }
    /**/
    struct timeval tv;

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

    m_replySocket = m_socket;
    
    return iResult;
}


int EasySocket::listen(int count)
{
    if (m_socket < 0) return -1;
    int res = ::listen(m_socket, count);
    checkResult(res);
    return res;
}

int EasySocket::accept()
{
    if (m_socket < 0) return -1;
    m_replySocket = ::accept(m_socket, nullptr, nullptr);

    int send_buffer = 64 * 1024*1024;    // 64 MB
    int send_buffer_sizeof = sizeof(int);
    setsockopt(m_replySocket, SOL_SOCKET, SO_SNDBUF, (char*)&send_buffer, send_buffer_sizeof);

    //int flag = 1;
    //int result = setsockopt(m_replySocket,IPPROTO_TCP,TCP_NODELAY,(char *)&flag,sizeof(int));

    u_long iMode = 0;//0 - blocking, 1 - non blocking
    ioctlsocket(m_replySocket, FIONBIO, &iMode);

    return (int)m_replySocket;
}

int EasySocket::bind(uint16_t portno)
{
    if (m_socket < 0) return -1;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    auto res = ::bind(m_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (res == SOCKET_ERROR)
    {
        printf("bind failed with error %u\n", WSAGetLastError());
        return -1;
    }
    return res;
}


#endif