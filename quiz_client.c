#include "stdio.h"
#include "shared.h"

#define PACKET_SIZE 2048
#define SERVER_PORT "5658"
#define SILENT_ERROR 1
#include "networking.h"

/* game closes when socket dies or takes too long */

int main(int NumArguments, char *Arguments[])
{
    NetworkStartup();
    SOCKET ServerSocket = Connect("132.161.242.190", SERVER_PORT);
    
    if(NumArguments >= 2)
    {
        Send(ServerSocket, Arguments[1], (int)strlen(Arguments[1]) + 1);
        
        player Players[8] = {0};
        Recieve(ServerSocket, (char *)Players, sizeof(Players));
        printf("raw: %s\n", (char *)Players);
        for(int PlayerIndex = 0; PlayerIndex < ArrayCount(Players); PlayerIndex++)
        {
            player *Player = Players + PlayerIndex;
            if(Player->Name != 0)
            {
                printf("player: %s\n", Player->Name);
            }
        }
        
    closesocket(ServerSocket);
}
else
{
    printf("invalid number of parameters\n");
}
}