#include "stdio.h"
#include "shared.h"

typedef struct
{
    SOCKET ServerSocket;
} network_state;

// TODO: game closes when socket dies or takes too long

internal void
ConnectToServer(network_state *Result, char *Name)
{
    NetworkStartup();
    SOCKET ServerSocket = Connect("2600:8801:2a80:15a:501c:db3:efb4:ba90", SERVER_PORT);
    Result->ServerSocket = ServerSocket;
    u_long Mode = 1;
    ioctlsocket(ServerSocket, FIONBIO, &Mode);
    Sleep(250);
    
    packet Packet = BuildPacket(PacketType_Name, Name, (int)strlen(Name));
    Send(ServerSocket, &Packet);
}