#include "stdio.h"

#define PACKET_SIZE 2048
#define SERVER_PORT "5658"
#define SILENT_ERROR 1

#include "networking.h"
#include "shared.h"
#include "quiz_questions.c"

// skip if everyone says "skip"? or maybe just keep the session short?

/*
TODO
Implement commands such as 
!skip - skips current question (we'll probably have long period like 45sec/question)
Maybe have GenerateQuestion jump to new category once its done with category stream?
*/

internal void
BroadcastPacket(SOCKET *PlayerSockets, int NumPlayers, packet *Packet)
{
    for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
    {
        SOCKET PlayerSocket = PlayerSockets[PlayerIndex];
        Send(PlayerSocket, Packet);
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

        player_list Players = {0};
        Players.NumPlayers = NumPlayers;

		for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			SOCKET ClientSocket = PlayerSockets[PlayerIndex];

            packet Packet = {0};
            while(Recieve(ClientSocket, &Packet) <= 0)
            {
                Sleep(10);
            }
            Assert(Packet.PacketType == PacketType_Name);

            int MaxPlayerNameLength = 14;
			player *Player = &Players.Contents[PlayerIndex];
			strncpy(Player->Name, Packet.Contents, MaxPlayerNameLength);
			Player->Score = 0;

			printf("%s has joined\n", Player->Name);
		}

        packet PlayerListPacket = BuildPacket(PacketType_PlayerList, (char *)&Players, sizeof(Players));
        BroadcastPacket(PlayerSockets, NumPlayers, &PlayerListPacket);
        
        #if 0
		for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			SOCKET ClientSocket = PlayerSockets[PlayerIndex];
			Send(ClientSocket, (char *)Players, sizeof(Players));
		}
        #endif
        
        FILE *FileHandle = fopen("test.txt", "r");
        question Question = GenerateQuestion(FileHandle);
        Question = GenerateQuestion(FileHandle);
        Question = GenerateQuestion(FileHandle);
        
        packet QuestionPacket = BuildPacket(PacketType_Question, (char *)&Question, sizeof(question));
        
        for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
        {
            SOCKET ClientSocket = PlayerSockets[PlayerIndex];
            Send(ClientSocket, &QuestionPacket);
        }
        
        while(true)
        {
        for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
        {
            SOCKET ClientSocket = PlayerSockets[PlayerIndex];
            packet ReceivedPacket = {0};
            int BytesRead = Recieve(ClientSocket, &ReceivedPacket);
            
            if(BytesRead > 0)
            {
                switch(ReceivedPacket.PacketType)
                {
                    case PacketType_ChatMessage:
                    {
                        printf("received chat message\n");
                        chat_message *ChatMessage = (chat_message *)&ReceivedPacket.Contents;
                        
                        if(strlen(ChatMessage->Value) > 0)
                        {
                            BroadcastPacket(PlayerSockets, NumPlayers, &ReceivedPacket);
                            
                            if(!strcmp(ChatMessage->Value, Question.Answer))
                            {
                                chat_message ResponseMessage;
                                strncpy(ResponseMessage.Value, "Correct", ArrayCount(ResponseMessage.Value));
                                
                                packet CorrectResponsePacket = BuildPacket(PacketType_ChatMessage,
                                                                           (char *)&ResponseMessage,
                                                                           sizeof(chat_message));
                                BroadcastPacket(PlayerSockets, NumPlayers, &CorrectResponsePacket);
                            }
                        }
                    } break;
                }
        }
        }
    }
        
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