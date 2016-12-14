#include "stdio.h"
#include "shared.h"
#include "quiz_questions.c"

#define PACKET_SIZE 2048
#define SERVER_PORT "5658"
#define SILENT_ERROR 1
#include "networking.h"

// skip if everyone says "skip"? or maybe just keep the session short?

internal void
BroadcastMessage(SOCKET *PlayerSockets, int NumPlayers, chat_message *Message)
{
    for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
    {
        SOCKET ClientSocket = PlayerSockets[PlayerIndex];
        Send(ClientSocket, (char *)Message, sizeof(chat_message));
    }
}

int main(int NumArguments, char *Arguments[])
{
	if(NumArguments >= 2)
	{
		char *NumPlayersString = Arguments[1];
		int NumPlayers = atoi(NumPlayersString);
		SOCKET *PlayerSockets = (SOCKET *)malloc(sizeof(SOCKET)*NumPlayers);

		NetworkStartup();
		SOCKET ListenSocket = Listen(SERVER_PORT);
		for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			PlayerSockets[PlayerIndex] = Accept(ListenSocket);
              u_long Mode = 1;
            ioctlsocket(PlayerSockets[PlayerIndex], FIONBIO, &Mode);
		}

		player Players[8] = { 0 };

		for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			SOCKET ClientSocket = PlayerSockets[PlayerIndex];

			char Buffer[PACKET_SIZE] = { 0 };
			int BytesReceived = Recieve(ClientSocket, Buffer, PACKET_SIZE);

			player *Player = &Players[PlayerIndex];
			strncpy(Player->Name, Buffer, ArrayCount(Player->Name));
			Player->Score = 0;

			printf("%s has joined\n", Buffer);
		}

		for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			SOCKET ClientSocket = PlayerSockets[PlayerIndex];
			Send(ClientSocket, (char *)Players, sizeof(Players));
		}
        
        FILE *FileHandle = fopen("test.txt", "r");
        question Question = GenerateQuestion(FileHandle);
        
        for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
        {
            SOCKET ClientSocket = PlayerSockets[PlayerIndex];
            Send(ClientSocket, (char *)&Question, sizeof(question));
        }
        
        #if 1
        while(true)
        {
        for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
        {
            SOCKET ClientSocket = PlayerSockets[PlayerIndex];
            chat_message ChatMessage;
            int BytesRead = Recieve(ClientSocket, (char *)&ChatMessage, sizeof(chat_message));
            
            if(BytesRead > 0)
            {
                if(strlen(ChatMessage.Value) > 0)
                {
                    BroadcastMessage(PlayerSockets, NumPlayers, &ChatMessage);
            }
        }
        }
    }
    #endif
        
		for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			SOCKET ClientSocket = PlayerSockets[PlayerIndex];
			closesocket(ClientSocket);
		}

		closesocket(ListenSocket);
	}
	else
	{
		printf("Invalid number of arguments\n");
	}
}