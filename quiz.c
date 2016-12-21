#ifndef QUIZ_C

#include "quiz.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <ctype.h>

internal void
DrawString(int X, int Y, int Width, int Height, char *String, TTF_Font *Font, SDL_Surface *ScreenSurface, SDL_Color ForegroundColor, SDL_Color BackgroundColor)
{
    int CharactersLeft = (int)strlen(String);
    int SourceCharIndex = 0;
    int StringPixelHeight = 0;
    SDL_Rect TextLocation = {0};
    TextLocation.x = X;
    TextLocation.y = Y;
    b32 Hyphenate = false;
    
    while(CharactersLeft)
    {
        char LineString[1024] = {0};
        int DestCharIndex = 0;
        
        if(Hyphenate)
        {
            LineString[DestCharIndex++] = '-';
        }
        
        int StringPixelWidth = 0;
        int LineHeight = 0;
        while(CharactersLeft > 0 && StringPixelWidth < Width)
        {
            LineString[DestCharIndex++] = String[SourceCharIndex++];
            TTF_SizeText(Font, LineString, &StringPixelWidth, &LineHeight);
            CharactersLeft--;
        }
        StringPixelHeight += LineHeight;
        
        char LastChar = String[DestCharIndex - 1];
        if(((LastChar >= 'a' && LastChar <= 'z') ||
           (LastChar >= 'A' && LastChar <= 'Z') ||
           (LastChar >= '0' && LastChar <= '9')) &&
           ((String[SourceCharIndex] >= 'a' && String[SourceCharIndex] <= 'z') ||
            (String[SourceCharIndex] >= 'a' && String[SourceCharIndex] <= 'z') ||
            (String[SourceCharIndex] >= 'a' && String[SourceCharIndex] <= 'z')))
        {
            Hyphenate = true;
        }
        
        if(StringPixelHeight < Height)
        {
        SDL_Surface* TextSurface = TTF_RenderText_Shaded(Font, LineString,
                                                         ForegroundColor, BackgroundColor);
        SDL_BlitSurface(TextSurface, 0, ScreenSurface, &TextLocation);
        SDL_FreeSurface(TextSurface);
        
        TextLocation.y = Y + StringPixelHeight;
        }
    }
}

internal memory_arena
InitMemoryArena(void *Base, int Bytes)
{
    memory_arena Result;
    Result.Base = Base;
    Result.BytesUsed = 0;
    Result.BytesTotal = Bytes;
    return Result;
}

internal bitmap
LoadSprite(char *FileName)
{
    bitmap Result = {0};
    int Components;
    Result.Memory = (u32 *)stbi_load(FileName, &Result.Width, &Result.Height, &Components, 0);
    
    int NumPixels = Result.Width*Result.Height;
    u32 *Pixel = Result.Memory;
    for(int PixelIndex = 0; PixelIndex < NumPixels; PixelIndex++)
    {
        u32 Alpha = (*Pixel & 0xFF000000) >> 24;
        u32 Blue = (*Pixel & 0x00FF0000) >> 16;
        u32 Green = (*Pixel & 0x0000FF00) >> 8;
        u32 Red = (*Pixel & 0x000000FF) >> 0;
        *Pixel++ = ((Alpha << 24) | (Red << 16) | (Green << 8) | Blue);
    }
    
    return Result;
}

internal void
DrawBitmap(screen *Screen, bitmap *Bitmap, int MinX, int MinY)
{
    int MaxX = MinX + Bitmap->Width - 1;
    int MaxY = MinY + Bitmap->Height - 1;
    
    u32 *ScreenRow = Screen->Memory + MinY*Screen->Width + MinX;
    u32 *BitmapPixel = Bitmap->Memory;
    for(int Y = MinY; Y <= MaxY; Y++)
    {
        u32 *ScreenPixel = ScreenRow;
        for(int X = MinX; X <= MaxX; X++)
        {
            if((X >= 0) && (X < Screen->Width) && (Y >= 0) && (Y < Screen->Height))
            {
                int BitmapAlpha = (*BitmapPixel & ((u32)0xFF << 24));
                if(BitmapAlpha)
                {
                    *ScreenPixel = *BitmapPixel;
                }
                ScreenPixel++;
                BitmapPixel++;
            }
        }
        ScreenRow += Screen->Width;
    }
}

internal void
DrawUserInterface(game_state *GameState, screen *Screen)
{    
    DrawBitmap(Screen, &GameState->Bitmaps.Trebek, 0, 0);
    int TrebekBottomY = GameState->Bitmaps.Trebek.Height;
    
    int DrawPlayersX = Screen->Width - GameState->Bitmaps.Players.Width;
    DrawBitmap(Screen, &GameState->Bitmaps.Players, DrawPlayersX, TrebekBottomY);
    
    DrawBitmap(Screen, &GameState->Bitmaps.Board, 0, TrebekBottomY);
}

internal b32
IsValidDrawCharacter(char Char)
{
    b32 Result = (((Char >= 'a') && (Char <= 'z')) || ((Char >= 'A') && (Char <= 'Z')) ||
                  ((Char >= '0') && (Char <= '9')) || (Char == '!') || (Char == '?') ||
                  (Char == ' ') || (Char == '\''));
    
    return Result;
}

internal void
DrawTopic(game_state *GameState, screen *Screen, char *Topic)
{
    int PadY = 5;
    
    int DrawX = 115;
    int DrawY = PadY + GameState->Bitmaps.Trebek.Height;
    int Width = GameState->Bitmaps.Board.Width - DrawX;
    int Height = GameState->Bitmaps.Board.Height - PadY;
    
    SDL_Color ForegroundColor = {211, 160, 129};
    SDL_Color BackgroundColor = {2, 2, 122};
    DrawString(DrawX, DrawY, Width, Height, Topic, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);
}

internal void
DrawQuestion(game_state *GameState, screen *Screen, char *Question)
{
    int PadY = 5;
    
    int DrawX = 10;
    int DrawY = PadY + 2*GameState->Bitmaps.Trebek.Height;
    int Width = GameState->Bitmaps.Board.Width - 2*DrawX;
    int Height = GameState->Bitmaps.Board.Height;
    
    SDL_Color ForegroundColor = {211, 160, 129};
    SDL_Color BackgroundColor = {2, 2, 122};
    DrawString(DrawX, DrawY, Width, Height, Question, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);
}

internal void
DrawPlayers(game_state *GameState, screen *Screen)
{
    int StartY = GameState->Bitmaps.Trebek.Height + 30;
    int DrawX = GameState->Bitmaps.Board.Width;
    int PadX = 10;
    int PadY = 8;
    int NameTextPadX = 10;
    int ScoreTextPadX = 128 + 14;
    
    SDL_Color ForegroundColor = {211, 160, 129};
    SDL_Color BackgroundColor = {2, 2, 122};
    
    for(int PlayerIndex = 0; PlayerIndex < GameState->Players.NumPlayers; PlayerIndex++)
    {
        player *Player = GameState->Players.Contents + PlayerIndex;
        
        int DrawY = StartY + (PlayerIndex*GameState->Bitmaps.Player.Height) + + PlayerIndex*PadY;
        DrawBitmap(Screen, &GameState->Bitmaps.Player, DrawX + PadX, DrawY);
        
        DrawString(DrawX + NameTextPadX, DrawY + PadY, GameState->Bitmaps.Player.Width, GameState->Bitmaps.Player.Height, 
                   Player->Name, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);

        char ScoreText[16];
        sprintf(ScoreText, "%03d", Player->Score);
        DrawString(ScoreTextPadX + DrawX, DrawY + PadY, GameState->Bitmaps.Player.Width - ScoreTextPadX, GameState->Bitmaps.Player.Height, ScoreText, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);
    }
}

internal void
DrawInputLine(game_state *GameState, screen *Screen)
{
    int DrawX = 10;
    int DrawY = GameState->Bitmaps.Board.Height + GameState->Bitmaps.Trebek.Height - 40;
    int Width = GameState->Bitmaps.Board.Width - 2*DrawX;
    int Height = 17;

    SDL_Color ForegroundColor = {211, 160, 129};
    SDL_Color BackgroundColor = {3, 3, 183};
    DrawString(DrawX, DrawY, Width, Height, GameState->CurInputLine, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);
}

internal void
AddPlayer(game_state *GameState, char *Name)
{
    player *Player = GameState->Players.Contents + GameState->Players.NumPlayers++;
    
    int ActualNameLength = (int)strlen(Name);
    for(int CharIndex = 0; CharIndex < ActualNameLength; CharIndex++)
    {
        Name[CharIndex] = (char)tolower(Name[CharIndex]);
    }
    
    int MaxNameLength = 8;
    strncpy(Player->Name, Name, MaxNameLength);
    
    Player->Score = 0;
}

internal void
UpdatePlayerScore(game_state *GameState, char *Name, int NewScore)
{
    for(int PlayerIndex = 0; PlayerIndex < GameState->Players.NumPlayers; PlayerIndex++)
    {
        player *Player = GameState->Players.Contents + PlayerIndex;
        if(!strcmp(Player->Name, Name))
        {
            Player->Score = NewScore;
        }
    }
}

internal void
SortPlayersByScore(game_state *GameState)
{
    // NOTE: Just selection sort, number of players is small.
    
    for(int SelectionIndex = 0; SelectionIndex < GameState->Players.NumPlayers; SelectionIndex++)
    {
        player *SelectionPlayer = GameState->Players.Contents + SelectionIndex;
        
        player *NextHighestPlayer = SelectionPlayer;
        
        for(int TestIndex = SelectionIndex + 1; TestIndex < GameState->Players.NumPlayers; TestIndex++)
        {
            player *TestPlayer = GameState->Players.Contents + TestIndex;
            if(TestPlayer->Score > NextHighestPlayer->Score)
            {
                NextHighestPlayer = TestPlayer;
            }
        }
        
            player TempPlayer = *NextHighestPlayer;
            *NextHighestPlayer = *SelectionPlayer;
            *SelectionPlayer = TempPlayer;
    }
}

internal chat_message *
CreateMessage(game_state *GameState, memory_arena *Arena, char *Value, chat_message *Next)
{
    chat_message *Result = 0;
    if(GameState->FreeMessages)
    {
        Result = GameState->FreeMessages;
        GameState->FreeMessages = GameState->FreeMessages->Next;
    }
    else if(Arena)
    {
        Result = PushStruct(chat_message, Arena);
    }
    
    if(Result)
    {
        strncpy(Result->Value, Value, ArrayCount(Result->Value));
        Result->Next = Next;
    }
    
    return Result;
}

 internal key_press *
CreateKeyPress(keyboard *Keyboard, memory_arena *MemoryArena, char Value, key_press *Next)
{
    key_press *Result = 0;
    
    if(Keyboard->FreeKeys)
    {
        Result = Keyboard->FreeKeys;
        Keyboard->FreeKeys = Keyboard->FreeKeys->Next;
    }
    else if(MemoryArena)
    {
        Result = PushStruct(key_press, MemoryArena);
    }
    
    if(Result)
    {
    Result->Value = Value;
    Result->Next = Next;
    }
    
    return Result;
}

internal void
ProcessKeyboardInput(game_state *GameState, network_state *NetworkState, keyboard *Keyboard, memory_arena *Arena)
{
    while(Keyboard->Input)
    {
        key_press *CurKeyPress = Keyboard->Input;
        char KeyPressed = CurKeyPress->Value;
        
        if((GameState->InputLineLength < ArrayCount(GameState->CurInputLine)) && 
           IsValidDrawCharacter(KeyPressed))
        {
            GameState->CurInputLine[GameState->InputLineLength++] = KeyPressed;
            GameState->CurInputLine[GameState->InputLineLength] = 0;
        }
        else if((GameState->InputLineLength > 0) && (KeyPressed == '\b'))
        {
            GameState->CurInputLine[--GameState->InputLineLength] = 0;
        }
        else if((KeyPressed == '\r') && (NetworkState != 0))
        {
            chat_message *NewMessage = CreateMessage(GameState, Arena, GameState->CurInputLine, 0);
            
            packet_type PacketType = PacketType_ChatMessage;
            if(!strcmp(GameState->CurInputLine, "!skip"))
            {
                PacketType = PacketType_SkipVote;
            }
            
            packet MessagePacket = BuildPacket(PacketType, (char *)NewMessage, sizeof(chat_message));
            Send(NetworkState->ServerSocket, &MessagePacket);
            
            GameState->InputLineLength = 0;
            GameState->CurInputLine[0] = 0;
        }
        else if((KeyPressed == '\r') && (NetworkState == 0))
        {
            NetworkState = PushStruct(network_state, Arena);
            ConnectToServer(NetworkState, GameState->CurInputLine);
            GameState->NetworkState = NetworkState;
            
            GameState->InputLineLength = 0;
            GameState->CurInputLine[0] = 0;
        }

        Keyboard->Input = Keyboard->Input->Next;
        CurKeyPress->Next = Keyboard->FreeKeys;
        Keyboard->FreeKeys = CurKeyPress;
    }
}

internal void
UpdateAndRender(void *Memory, int MemoryInBytes, screen *Screen, keyboard *Keyboard)
{
    game_state *GameState = (game_state *)Memory;
    memory_arena *Arena = &GameState->Arena;
    
    if(!GameState->Initialized)
    {
        GetEditDistance("gumbo", "gambol");
        
        GameState->Initialized = true;
        
        GameState->Bitmaps.Trebek = LoadSprite("data/trebek.png");
        GameState->Bitmaps.Players = LoadSprite("data/players.png");
        GameState->Bitmaps.Player = LoadSprite("data/player.png");
        GameState->Bitmaps.Board = LoadSprite("data/board.png");
        
        FILE *FileHandle = fopen("test.txt", "r");
        char CurChar = 'a';
        int NumLines = 0;
        while(CurChar != EOF)
        {
            CurChar= fgetc(FileHandle);
            if(CurChar == '\n')
            {
                NumLines++;
            }
        }
        fclose(FileHandle);
        
         GameState->Messages = CreateMessage(GameState, Arena, "Enter name...", 0);
    }
    
    network_state *NetworkState = (network_state *)GameState->NetworkState;
    
    if(NetworkState != 0)
    {
    packet IncomingPacket = {0};
    
    int BytesRecieved = TryRecievePacket(NetworkState->ServerSocket, &IncomingPacket);
    while(BytesRecieved > 0)
    {
        printf("recived %d\n", BytesRecieved);
        
        switch(IncomingPacket.PacketType)
        {
            case PacketType_PlayerList:
            {
                memcpy(&GameState->Players, IncomingPacket.Contents, sizeof(GameState->Players));
            } break;
            case PacketType_ChatMessage:
            {
                char *RawMessage = ((chat_message *)IncomingPacket.Contents)->Value;
                chat_message *NewMessage = CreateMessage(GameState, Arena, RawMessage, GameState->Messages);
                GameState->Messages = NewMessage;
            } break;
            case PacketType_Winner:
            {
                char *RawMessage = ((chat_message *)IncomingPacket.Contents)->Value;
                chat_message *NewMessage = CreateMessage(GameState, Arena, RawMessage, GameState->Messages);
                GameState->Messages = NewMessage;
                
                chat_message *NewRound = CreateMessage(GameState, Arena, "--------------------------------", GameState->Messages);
                GameState->Messages = NewRound;
            };
            case PacketType_Question:
            {
                question *NewQuestion = (question *)&IncomingPacket.Contents;
                memcpy(&GameState->CurrentQuestion, NewQuestion, sizeof(question));
            } break;
    }
    
    BytesRecieved = TryRecievePacket(NetworkState->ServerSocket, &IncomingPacket);
}
}
    
    SortPlayersByScore(GameState);
    
    ProcessKeyboardInput(GameState, NetworkState, Keyboard, Arena);
    
    DrawUserInterface(GameState, Screen);
    DrawPlayers(GameState, Screen);
    DrawTopic(GameState, Screen, GameState->CurrentQuestion.Category);
    DrawInputLine(GameState, Screen);
    
    int DrawX = 10;
    int DrawY = GameState->Bitmaps.Board.Height + GameState->Bitmaps.Trebek.Height - 70;
    int MessageHeight = 17;
    int MessageWidth = GameState->Bitmaps.Board.Width - 2*DrawX;
    chat_message *CurMessage = GameState->Messages;
    SDL_Color ForegroundColor = {211, 160, 129};
    SDL_Color BackgroundColor = {3, 3, 183};
    
    int MinY = 150 + GameState->Bitmaps.Trebek.Height;
    while(CurMessage && (DrawY > MinY))
    {
        DrawString(DrawX, DrawY, MessageWidth, MessageHeight, CurMessage->Value, GameState->Font, GameState->ScreenSurface,
                 ForegroundColor, BackgroundColor);
        CurMessage = CurMessage->Next;
        DrawY -= MessageHeight;
    }
    
    DrawQuestion(GameState, Screen, GameState->CurrentQuestion.Question);
}

#define QUIZ_C
#endif