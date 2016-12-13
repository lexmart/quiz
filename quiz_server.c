#include "stdio.h"
#include "shared.h"

#define PACKET_SIZE 2048
#define SERVER_PORT "5658"
#define SILENT_ERROR 1
#include "networking.h"

int main(int NumArguments, char *Arguments[])
{
	if (NumArguments >= 2)
	{
		char *NumPlayersString = Arguments[1];
		int NumPlayers = atoi(NumPlayersString);
		SOCKET *PlayerSockets = (SOCKET *)malloc(sizeof(SOCKET)*NumPlayers);

		NetworkStartup();
		SOCKET ListenSocket = Listen(SERVER_PORT);
		for (int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			PlayerSockets[PlayerIndex] = Accept(ListenSocket);
		}

		player Players[8] = { 0 };

		for (int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			SOCKET ClientSocket = PlayerSockets[PlayerIndex];

			char Buffer[PACKET_SIZE] = { 0 };
			int BytesReceived = Recieve(ClientSocket, Buffer, PACKET_SIZE);

			player *Player = &Players[PlayerIndex];
			strncpy(Player->Name, Buffer, ArrayCount(Player->Name));
			Player->Score = 0;

			printf("%s has joined\n", Buffer);
		}

		for (int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
		{
			SOCKET ClientSocket = PlayerSockets[PlayerIndex];
			Send(ClientSocket, (char *)Players, sizeof(Players));
		}

		for (int PlayerIndex = 0; PlayerIndex < NumPlayers; PlayerIndex++)
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