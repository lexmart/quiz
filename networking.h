#ifndef NETWORKING_H

#include "memory.h"

typedef enum
{
    PacketType_PlayerList,
    PacketType_ChatMessage,
    PacketType_Winner,
    PacketType_Question,
    PacketType_Name
} packet_type;

typedef struct
{
    packet_type PacketType;
    char Contents[PACKET_SIZE];
    int BytesUsed;
} packet;

 packet
BuildPacket(packet_type PacketType, char *SourceBytes, int SourceLength)
{
    packet Packet = {0};
    Packet.PacketType = PacketType;
    Packet.BytesUsed = SourceLength;
    memcpy(Packet.Contents, SourceBytes, SourceLength);
    
    return Packet;
}

#include "winsock2.h"
#include "Ws2tcpip.h"
#include "windows.h"

#define Error(Message) if(SILENT_ERROR) { printf(Message); exit(0); } else { printf(Message); MessageBox(0, Message, 0, MB_OK); exit(0); }

void
NetworkStartup()
{
    WSADATA WsaData;
    if(WSAStartup(MAKEWORD(2,2), &WsaData) != 0)
    {
        Error("NetworkStartup");
    }
}

SOCKET
Connect(char *Address, char *Port)
{
    SOCKET Result = INVALID_SOCKET;
    
    struct addrinfo *AddrInfo;
    struct addrinfo Hints;
    
    ZeroMemory(&Hints, sizeof(Hints));
    Hints.ai_family = AF_UNSPEC;
    Hints.ai_socktype = SOCK_STREAM;
    Hints.ai_protocol = IPPROTO_TCP;
    
    getaddrinfo(Address, Port, &Hints, &AddrInfo);
    
        Result = socket(AddrInfo->ai_family, AddrInfo->ai_socktype, AddrInfo->ai_protocol);
        if(Result != INVALID_SOCKET)
        {
            int debug = connect(Result, AddrInfo->ai_addr, (int)AddrInfo->ai_addrlen);
            if(debug != 0)
            {
                Result = INVALID_SOCKET;
                Error("Connect->could not connect");
            }
            else
            {
                printf("successfully connected\n");
            }
    }
    
    if(Result == INVALID_SOCKET)
    {
        Error("Connect->getaddrinfo");
    }
    
    freeaddrinfo(AddrInfo);
    return Result;
}

SOCKET
Listen(char *Port)
{
    printf("Attempting to start listening...\n");
    
    struct addrinfo *AddrInfo;
    struct addrinfo Hints;
    
    ZeroMemory(&Hints, sizeof(Hints));
    Hints.ai_family = AF_INET;
    Hints.ai_socktype = SOCK_STREAM;
    Hints.ai_protocol = IPPROTO_TCP;
    Hints.ai_flags = AI_PASSIVE;
    
    if(getaddrinfo(NULL, Port, &Hints, &AddrInfo) != 0)
    {
        Error("Listen->getaddrinfo");
    }
    
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(AddrInfo->ai_family, AddrInfo->ai_socktype, AddrInfo->ai_protocol);
    
    if(ListenSocket == INVALID_SOCKET)
    {
        Error("Listen->Could not create ListenSocket");
    }
    
    if(bind(ListenSocket, AddrInfo->ai_addr, (int)AddrInfo->ai_addrlen) == SOCKET_ERROR)
    {
        Error("Listen->Could not bind");
    }
    freeaddrinfo(AddrInfo);
    
    printf("Listening on port %s\n", Port);
    
    if(listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR)
    {
        Error("Listen->Could not listen");
    }
    
    return ListenSocket;
}

SOCKET
Accept(SOCKET ListenSocket)
{
    SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
    if(ClientSocket == INVALID_SOCKET)
    {
        Error("Accept->Could not accept");
    }
    printf("Accepted\n");
    return ClientSocket;
}

void
Send(SOCKET Socket, packet *Packet)
{
    if(send(Socket, (char *)Packet, sizeof(packet), 0) == SOCKET_ERROR)
    {
        Error("send failed\n");
    }
}

int
Recieve(SOCKET Socket, packet *Packet)
{
    int Bytes = recv(Socket, (char *)Packet, sizeof(packet), 0);
    
    return Bytes;
}

#define NETWORKING_H
#endif