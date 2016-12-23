#include "stdio.h"

#define PACKET_SIZE 2048
#define SERVER_PORT "7937"
#define SILENT_ERROR 1

#include "networking.h"
#include "shared.h"
#include "quiz_questions.c"
#include "time.h"

/*
TODO
!skip - skips current question (we'll probably have long period like 45sec/question)
Maybe have GenerateQuestion jump to new category once its done with category stream?

add timer to question

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
    srand(time(0));
    
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
            while(TryRecievePacket(ClientSocket, &Packet) <= 0)
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
        
        r32 CorrectAnswerScore = 0.8f;
        char WelcomeMessage[512];
        sprintf(WelcomeMessage, "Alex Trebek: Welcome to Jeopardy, I'm your host, Alex Trebrek.");
        packet WelcomeMessagePacket = BuildPacket(PacketType_ChatMessage, (char *)WelcomeMessage, (int)strlen(WelcomeMessage));
        BroadcastPacket(PlayerSockets, NumPlayers, &WelcomeMessagePacket);
        sprintf(WelcomeMessagePacket.Contents, "Alex Trebek: Good luck.", CorrectAnswerScore);
        BroadcastPacket(PlayerSockets, NumPlayers, &WelcomeMessagePacket);
        
        packet PlayerListPacket = BuildPacket(PacketType_PlayerList, (char *)&Players, sizeof(Players));
        BroadcastPacket(PlayerSockets, NumPlayers, &PlayerListPacket);
        
        FILE *FileHandle = fopen("test.txt", "r");
        SkipQuestion(FileHandle);
        question Question = GenerateQuestion(FileHandle);
        Question = GenerateQuestion(FileHandle);
        Question = GenerateQuestion(FileHandle);
        
        packet QuestionPacket = BuildPacket(PacketType_Question, (char *)&Question, sizeof(question));
        BroadcastPacket(PlayerSockets, NumPlayers, &QuestionPacket);
        
        b32 *SkipVotes = malloc(sizeof(b32)*NumPlayers);
        memset(SkipVotes, 0, sizeof(b32)*NumPlayers);
        
        while(true)
        {
            Sleep(10);
            
            int NumSkipVotes = 0;
            
            for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
            {
                packet RecievedPacket = {0};
                SOCKET PlayerSocket = PlayerSockets[PlayerIndex];
                player *Player = &Players.Contents[PlayerIndex];
                int BytesRecieved = TryRecievePacket(PlayerSocket, &RecievedPacket);
                NumSkipVotes += SkipVotes[PlayerIndex];
                
                if(BytesRecieved > 0)
                {
                    if(RecievedPacket.PacketType == PacketType_ChatMessage)
                    {
                        printf("received chat message\n");
                        chat_message *ChatMessage = (chat_message *)&RecievedPacket.Contents;
                        
                        if(strlen(ChatMessage->Value) > 0)
                        {
                            chat_message PlayerMessage = {0};
                            sprintf(&PlayerMessage.Value[0], "%s: %s", Player->Name, RecievedPacket.Contents);
                            packet PlayerMessagePacket = 
                                BuildPacket(PacketType_ChatMessage, (char *)&PlayerMessage, sizeof(chat_message));
                            BroadcastPacket(PlayerSockets, NumPlayers, &PlayerMessagePacket);
                            
                            r32 AnswerScore = 1.0f - NormalizedDistance(ChatMessage->Value, &Question);
                            
                            if(AnswerScore >= CorrectAnswerScore)
                            {
                                chat_message ResponseMessage;
                                sprintf(ResponseMessage.Value, "Alex Trebek: %s wins the round (answer score was %.2f)", 
                                        Player->Name, AnswerScore);
                                
                                packet CorrectResponsePacket = BuildPacket(PacketType_Winner,
                                                                           (char *)&ResponseMessage,
                                                                           sizeof(CorrectResponsePacket));
                                BroadcastPacket(PlayerSockets, NumPlayers, &CorrectResponsePacket);
                                
                                Question = GenerateQuestion(FileHandle);
                                QuestionPacket = BuildPacket(PacketType_Question, (char *)&Question, sizeof(question));
                                BroadcastPacket(PlayerSockets, NumPlayers, &QuestionPacket);
                                
                                Player->Score++;
                                PlayerListPacket = BuildPacket(PacketType_PlayerList, (char *)&Players, sizeof(Players));
                                BroadcastPacket(PlayerSockets, NumPlayers, &PlayerListPacket);
                                
                                memset(SkipVotes, 0, sizeof(b32)*NumPlayers);
                            }
                        }
                    }
                }
                else if(RecievedPacket.PacketType == PacketType_SkipVote)
                {
                    chat_message PlayerMessage = {0};
                    sprintf(&PlayerMessage.Value[0], "%s: !skip", Player->Name);
                    packet PlayerMessagePacket = 
                        BuildPacket(PacketType_ChatMessage, (char *)&PlayerMessage, sizeof(chat_message));
                    BroadcastPacket(PlayerSockets, NumPlayers, &PlayerMessagePacket);
                    
                    SkipVotes[PlayerIndex] = true;
                }
            }
            
            if(NumSkipVotes == NumPlayers)
            {
                Question = GenerateQuestion(FileHandle);
                QuestionPacket = BuildPacket(PacketType_Question, (char *)&Question, sizeof(question));
                BroadcastPacket(PlayerSockets, NumPlayers, &QuestionPacket);
            }
        }
        
        #if 0
        while(true)
        {
        for(int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
        {
            player *Player = &Players.Contents[PlayerIndex];
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
                            chat_message PlayerMessage = {0};
                            sprintf(&PlayerMessage.Value[0], "%s: %s", Player->Name, ReceivedPacket.Contents);
                            packet PlayerMessagePacket = 
                                BuildPacket(PacketType_ChatMessage, (char *)&PlayerMessage, sizeof(chat_message));
                            BroadcastPacket(PlayerSockets, NumPlayers, &PlayerMessagePacket);
                            
                            r32 AnswerScore = 1.0f - NormalizedDistance(ChatMessage->Value, &Question);
                            
                            if(AnswerScore >= CorrectAnswerScore)
                            {
                                chat_message ResponseMessage;
                                sprintf(ResponseMessage.Value, "Alex Trebek: %s wins the round (answer score was %.2f)", 
                                        Player->Name, AnswerScore);
                                
                                packet CorrectResponsePacket = BuildPacket(PacketType_Winner,
                                                                           (char *)&ResponseMessage,
                                                                           sizeof(CorrectResponsePacket));
                                BroadcastPacket(PlayerSockets, NumPlayers, &CorrectResponsePacket);
                                
                                Question = GenerateQuestion(FileHandle);
                                QuestionPacket = BuildPacket(PacketType_Question, (char *)&Question, sizeof(question));
                                BroadcastPacket(PlayerSockets, NumPlayers, &QuestionPacket);
                                
                                Player->Score++;
                                PlayerListPacket = BuildPacket(PacketType_PlayerList, (char *)&Players, sizeof(Players));
                                BroadcastPacket(PlayerSockets, NumPlayers, &PlayerListPacket);
                            }
                            else
                            {
                                #if 0
                                chat_message ResponseMessage;
                                sprintf(ResponseMessage.Value, "Alex Trebek: %s has score %.2f", 
                                        ReceivedPacket.Contents, AnswerScore);
                                
                                packet IncorrectResponsePacket = BuildPacket(PacketType_ChatMessage,
                                                                             (char *)&ResponseMessage,
                                                                             sizeof(ResponseMessage));
                                BroadcastPacket(PlayerSockets, NumPlayers, &IncorrectResponsePacket);
                                #endif
                            }
                        }
                    } break;
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