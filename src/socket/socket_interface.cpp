/*
 * socket_interface.cpp
 *
 *  Created on: 6 Oct 2016
 *      Author: Oscar Crespo
 */

#include "socket_interface.h"

#include <unistd.h>
#include <cstring>

#include <arpa/inet.h>


#include "../connection/connection_types.h"

const int Socket::MAX_CONNEXTION_LISTEN = 5;

using connection::SConnection;
using connection::EError;
using connection::ESocketType;
using connection::EConnexionType;
using connection::ECloseType;

void copy_socket_struct(sockaddr_in &to,const sockaddr_in &from)
{
    to.sin_family = from.sin_family;
    to.sin_addr = from.sin_addr;
    to.sin_port = from.sin_port;
    memcpy(to.sin_zero,from.sin_zero,sizeof(from.sin_zero));
}


Socket& Socket::operator=(const Socket &rhs)
{

    if (this == &rhs) return *this;

    socketId_ = rhs.socketId_;
    copy_socket_struct(address_,rhs.address_);

    return *this;
}

SConnection::SConnection():
		size_(sizeof(struct sockaddr_in)),
        connection_(-1)
{

}

SConnection::SConnection(const SConnection &obj):
		size_(obj.size_),
        connection_(obj.connection_)
{
    copy_socket_struct(this->connectionInfo_,obj.connectionInfo_);
}
SConnection& SConnection::operator=(const SConnection &rhs)
{
    if(this == &rhs) return *this;

    this->connection_ = rhs.connection_;

    copy_socket_struct(this->connectionInfo_,rhs.connectionInfo_);

    return *this;
}


Socket::Socket():
        size_(sizeof(address_)),
        socketId_{-1}
{
	bzero(&address_, sizeof(address_));
}

Socket::Socket(const Socket &obj):
        socketId_{obj.socketId_}
{
    copy_socket_struct(address_,obj.address_);
    size_ = sizeof(address_);
}

Socket::~Socket()
{

}

EError Socket::convert_error() const
{

    switch errno
    {
    case EACCES:
        return EError::ACCES;
        break;
    case EADDRINUSE:
        return EError::ADDRINUSE;
        break;
    case EADDRNOTAVAIL:
        return EError::ADDRNOTAVAIL;
        break;
    case EBADF:
        return EError::BADF;
        break;
    case EFAULT:
        return EError::FAULT;
        break;
    case EINVAL:
        return EError::INVAL;
        break;
    case ENOTSOCK:
        return EError::NOTSOCK;
        break;
    case EIO:
        return EError::IO;
        break;
    case EISDIR:
        return EError::ISDIR;
        break;
    case ELOOP:
        return EError::LOOP;
        break;
    case ENAMETOOLONG:
        return EError::NAMETOOLONG;
        break;
    case ENOENT:
        return EError::NOENT;
        break;
    case ENOTDIR:
        return EError::NOTDIR;
        break;
    case EROFS:
        return EError::ROFS;
        break;
    case EMFILE:
        return EError::MFILE;
        break;
    case ENFILE:
        return EError::NFILE;
        break;
    case ENOBUFS:
        return EError::NOBUFS;
        break;
    case EPROTONOSUPPORT:
        return EError::PROTONOSUPPORT;
        break;
    case EPROTOTYPE:
        return EError::PROTOTYPE;
        break;
    case EOPNOTSUPP:
        return EError::OPNOTSUPP;
        break;
    case EAGAIN:
        return EError::AGAIN;
        break;
    case ECONNRESET:
        return EError::CONNRESET;
        break;
    case EINTR:
        return EError::INTR;
        break;
    case ENOTCONN:
        return EError::NOTCONN;
        break;
    case ETIMEDOUT:
        return EError::TIMEDOUT;
        break;
    case ENOMEM:
        return EError::NOMEM;
        break;
    case EDESTADDRREQ:
        return EError::DESTADDRREQ;
        break;
    case EMSGSIZE:
        return EError::MSGSIZE;
        break;
    case EPIPE:
        return EError::PIPE;
        break;
    case ENETDOWN:
        return EError::NETDOWN;
        break;
    case ENETUNREACH:
        return EError::NETUNREACH;
        break;

    }

    return EError::UNKNOWN;
}

connection::EError Socket::create_socket(ESocketType socketType,EConnexionType connexionType) noexcept
{
    socketId_ =  socket(static_cast<int>(connexionType),static_cast<int>(socketType),0);

    if(socketId_ < -1)
    {
        return convert_error();
    }
    return connection::EError::NO_ERROR;
}

void Socket::set_socket_info(EConnexionType connexionType,uint16_t port,uint32_t address) noexcept
{
    address_.sin_family = static_cast<int>(connexionType);
    address_.sin_addr.s_addr = htonl(address);
    address_.sin_port = htons(port);
}

void Socket::set_socket_info(EConnexionType connexionType,uint16_t port,const char* address) noexcept
{
    address_.sin_family = static_cast<int>(connexionType);
    address_.sin_addr.s_addr = inet_addr(address);
    address_.sin_port = htons(port);
}

EError Socket::bind_socket() noexcept
{
    if( bind(socketId_,(struct sockaddr *) &address_, sizeof(address_) ) == -1)
    {
        return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::listen_socket(int maxConnexions) noexcept
{
    if(listen(socketId_,maxConnexions) != 0)
    {
        return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::accept_connexion(SConnection &connexion) noexcept
{
    socklen_t clilen {sizeof(connexion.connectionInfo_)};
    connexion.connection_ = accept(socketId_,(struct sockaddr *) &connexion.connectionInfo_,&clilen);
    if(connexion.connection_ < -1)
    {
        return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::connect_to() noexcept
{
	 if(connect(socketId_,(struct sockaddr *) &address_,sizeof(address_)) < 0){
        return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::recceive_message(uint8_t *buffer,int size,int &sizeRead, int flags) const noexcept
{
    sizeRead = recv(socketId_,buffer,size,flags);
    if(sizeRead < 0)
    {
       return convert_error();
    }

    return EError::NO_ERROR;
}


EError Socket::recceive_message(const SConnection &connexion,uint8_t *buffer,int size,int &sizeRead, int flags) const noexcept
{
    sizeRead = recv(connexion.connection_,buffer,size,flags);
    if(sizeRead < 0)
    {
       return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::recceive_from_message(SConnection &connexion,uint8_t *buffer,int size,int &sizeRead, int flags) noexcept
{
    sizeRead = recvfrom(socketId_,buffer,size,flags,(struct sockaddr *) &connexion.connectionInfo_,&connexion.size_);
    if(sizeRead < 0)
    {
       return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::send_message(const uint8_t *buffer,int size,int &sizeSent, int flags)const noexcept
{
    sizeSent = send(socketId_,buffer,size,flags);
    if(sizeSent < 0)
    {
       return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::send_message(const SConnection &connexion,const uint8_t *buffer,int size,int &sizeSent, int flags)const noexcept
{
    sizeSent = send(connexion.connection_,buffer,size,flags);
    if(sizeSent < 0)
    {
       return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::send_to_message(const SConnection &connexion,const uint8_t *buffer,int size,int &sizeSent) noexcept
{
    sizeSent = sendto(socketId_,buffer,size,0,(struct sockaddr *) &connexion.connectionInfo_,connexion.size_);
    if(sizeSent < 0)
    {
       return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::close_connexion(const SConnection &connexion,ECloseType close) noexcept
{
    if(shutdown(connexion.connection_,static_cast<int>(close)) < 0)
    {
        return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::disconnect_socket() noexcept
{
    if(shutdown(socketId_,static_cast<int>(ECloseType::STOP_ALL)) < 0)
    {
        return convert_error();
    }

    return EError::NO_ERROR;
}

EError Socket::close_socket() noexcept
{
    if(close(socketId_) < 0)
    {
        return convert_error();
    }

    return EError::NO_ERROR;
}



