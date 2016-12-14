#include "stdio.h"
#include "shared.h"

#define PACKET_SIZE 2048
#define SERVER_PORT "5658"
#define SILENT_ERROR 0
#include "networking.h"

typedef struct
{
    SOCKET ServerSocket;
} network_state;

// TODO: game closes when socket dies or takes too long

internal void
ConnectToServer(network_state *Result, char *Name)
{
    NetworkStartup();
    SOCKET ServerSocket = Connect("10.91.7.2", SERVER_PORT);
    Result->ServerSocket = ServerSocket;
    u_long Mode = 1;
    ioctlsocket(ServerSocket, FIONBIO, &Mode);
    Send(ServerSocket, Name, (int)strlen(Name) + 1);
}

internal void
GetPlayerList(network_state *NetworkState, player *Players, int PlayersLength)
{
    while(Recieve(NetworkState->ServerSocket, (char *)Players, sizeof(player)*PlayersLength) <= 0);
    for(int PlayerIndex = 0; PlayerIndex < PlayersLength; PlayerIndex++)
    {
        player *Player = Players + PlayerIndex;
        if(Player->Name != 0)
        {
            printf("player: %s\n", Player->Name);
        }
    }
}

internal void
ReceiveQuestion(network_state *NetworkState, question *Question)
{
    while(Recieve(NetworkState->ServerSocket, (char *)Question, sizeof(question)) <= 0);
    
    printf("category: %s\n", Question->Category);
    printf("question: %s\n", Question->Question);
    printf("question: %s\n", Question->Answer);
}

internal void
SendChatMessage(network_state *NetworkState, chat_message *Message)
{
    Send(NetworkState->ServerSocket, (char *)Message, sizeof(chat_message));
}

internal b32
ReceiveChatMessage(network_state *NetworkState, chat_message *Message)
{
     int BytesRecieved = Recieve(NetworkState->ServerSocket, (char *)Message, sizeof(chat_message));
    
    if(BytesRecieved > 0)
    {
    printf("%d\n", BytesRecieved);
    }
    b32 Result = (BytesRecieved > 0);
    
    return Result;
}

#if 0
int main(int NumArguments, char *Arguments[])
{
    NetworkStartup();
    SOCKET ServerSocket = Connect("132.161.242.190", SERVER_PORT);
    
    if(NumArguments >= 2)
    {
        Send(ServerSocket, Arguments[1], (int)strlen(Arguments[1]) + 1);
        
        player Players[8] = {0};
        Recieve(ServerSocket, (char *)Players, sizeof(Players));
        
        for(int PlayerIndex = 0; PlayerIndex < ArrayCount(Players); PlayerIndex++)
        {
            player *Player = Players + PlayerIndex;
            if(Player->Name != 0)
            {
                printf("player: %s\n", Player->Name);
            }
        }
        
        question Question;
        Recieve(ServerSocket, (char *)&Question, sizeof(question));
        printf("category: %s\n", Question.Category);
        printf("question: %s\n", Question.Question);
        printf("question: %s\n", Question.Answer);
        
    closesocket(ServerSocket);
}
else
{
    printf("invalid number of parameters\n");
}
}
#endif