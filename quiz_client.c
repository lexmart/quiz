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
    printf("10.91.7.2");
    SOCKET ServerSocket = Connect("10.91.7.2", SERVER_PORT);
    printf("10.91.7.2");
    Result->ServerSocket = ServerSocket;
    u_long Mode = 1;
    ioctlsocket(ServerSocket, FIONBIO, &Mode);
    Sleep(250);
    
    packet Packet = BuildPacket(PacketType_Name, Name, (int)strlen(Name));
    Send(ServerSocket, &Packet);
}

#if 0
internal void
GetPlayerList(network_state *NetworkState, player *Players, int PlayersLength)
{
#if 0
    int BytesReceived = -1;
    while(BytesReceived <= 0)
    {
        BytesReceived = Recieve(NetworkState->ServerSocket, (char *)Players, sizeof(player)*PlayersLength);
        Sleep(10);
    }
    for(int PlayerIndex = 0; PlayerIndex < PlayersLength; PlayerIndex++)
    {
        player *Player = Players + PlayerIndex;
        if(Player->Name != 0)
        {
            printf("player: %s\n", Player->Name);
        }
    }
#else
    
    packet PlayerListPacket = {0};
    while(Recieve(NetworkState->ServerSocket, &PlayerListPacket) <= 0)
    {
        Sleep(50);
    }
    
    #endif
}

internal void
ReceiveQuestion(network_state *NetworkState, question *Question)
{
    while(Recieve(NetworkState->ServerSocket, (char *)Question, sizeof(question)) <= 0)
    {
        Sleep(10);
    }
    
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

#endif

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