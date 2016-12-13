#ifndef QUIZ_C

#include "quiz.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <ctype.h>

internal void
DrawText(int X, int Y, int Width, int Height, char *String, TTF_Font *Font, SDL_Surface *ScreenSurface, SDL_Color ForegroundColor, SDL_Color BackgroundColor)
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
        if((LastChar >= 'a' && LastChar <= 'z') || 
           (LastChar >= 'A' && LastChar <= 'Z') ||
           (LastChar >= '0' && LastChar <= '9'))
        {
            Hyphenate = true;
        }
        
        if(StringPixelHeight < Height)
        {
        SDL_Surface* TextSurface = TTF_RenderText_Shaded(Font, LineString,
                                                         ForegroundColor, BackgroundColor);
        SDL_BlitSurface(TextSurface, 0, ScreenSurface, &TextLocation);
        SDL_FreeSurface(TextSurface);
        
        TextLocation.y = StringPixelHeight;
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
LoadBitmap(char *FileName)
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
    //Assert((MinX >= 0) && (MinY >= 0) && (MaxX < Screen->Width) && (MaxY < Screen->Height));
    
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
                  (Char == ' '));
    
    return Result;
}

// TODO: No paramter, see below
internal void
DrawTopic(game_state *GameState, screen *Screen, char *Topic)
{
    int PadY = 5;
    
    int DrawX = 115;
    int DrawY = PadY + GameState->Bitmaps.Trebek.Height;
    int Width = GameState->Bitmaps.Board.Width - DrawX;
    int Height = GameState->Bitmaps.Board.Height - PadY;
    
    SDL_Color ForegroundColor = {223, 96, 14};
    SDL_Color BackgroundColor = {2, 2, 122};
    DrawText(DrawX, DrawY, Width, Height, Topic, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);
}

// TODO: No parameter, just draw question from game_state
internal void
DrawQuestion(game_state *GameState, screen *Screen, char *Question)
{
    int PadY = 5;
    
    int DrawX = 10;
    int DrawY = PadY + 2*GameState->Bitmaps.Trebek.Height;
    int Width = GameState->Bitmaps.Board.Width;
    int Height = GameState->Bitmaps.Board.Height;
    
    SDL_Color ForegroundColor = {223, 96, 14};
    SDL_Color BackgroundColor = {2, 2, 122};
    DrawText(DrawX, DrawY, Width, Height, Question, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);
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
    
    SDL_Color ForegroundColor = {223, 96, 14};
    SDL_Color BackgroundColor = {2, 2, 122};
    
    for(int PlayerIndex = 0; PlayerIndex < GameState->NumPlayers; PlayerIndex++)
    {
        player *Player = GameState->Players + PlayerIndex;
        
        int DrawY = StartY + (PlayerIndex*GameState->Bitmaps.Player.Height) + + PlayerIndex*PadY;
        DrawBitmap(Screen, &GameState->Bitmaps.Player, DrawX + PadX, DrawY);
        
        DrawText(DrawX + NameTextPadX, DrawY + PadY, GameState->Bitmaps.Player.Width, GameState->Bitmaps.Player.Height, Player->Name, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);

        char ScoreText[16];
        sprintf(ScoreText, "%03d", Player->Score);
        DrawText(ScoreTextPadX + DrawX, DrawY + PadY, GameState->Bitmaps.Player.Width - ScoreTextPadX, GameState->Bitmaps.Player.Height, ScoreText, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);
    }
}

internal void
DrawInputLine(game_state *GameState, screen *Screen)
{
    int DrawX = 10;
    int DrawY = GameState->Bitmaps.Board.Height + GameState->Bitmaps.Trebek.Height - 40;
    int Width = GameState->Bitmaps.Board.Width - 2*DrawX;
    int Height = 15;

    SDL_Color ForegroundColor = {223, 96, 14};
    SDL_Color BackgroundColor = {3, 3, 183};
    DrawText(DrawX, DrawY, Width, Height, GameState->CurInputLine, GameState->Font, GameState->ScreenSurface, ForegroundColor, BackgroundColor);
}

internal void
AddPlayer(game_state *GameState, char *Name)
{
    player *Player = GameState->Players + GameState->NumPlayers++;
    
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
    for(int PlayerIndex = 0; PlayerIndex < GameState->NumPlayers; PlayerIndex++)
    {
        player *Player = GameState->Players + PlayerIndex;
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
    
    for(int SelectionIndex = 0; SelectionIndex < GameState->NumPlayers; SelectionIndex++)
    {
        player *SelectionPlayer = GameState->Players + SelectionIndex;
        
        player *NextHighestPlayer = SelectionPlayer;
        
        for(int TestIndex = SelectionIndex + 1; TestIndex < GameState->NumPlayers; TestIndex++)
        {
            player *TestPlayer = GameState->Players + TestIndex;
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
ProcessKeyboardInput(game_state *GameState, keyboard *Keyboard, memory_arena *Arena)
{
    if(Keyboard->Input)
    {
        int Stub = 0;
    }
    
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
        else if(KeyPressed == '\r')
        {
            // TODO: Move this code. This shoulding be pushing CurInputLine up to the server, not putting in message queue
            chat_message *NewMessage = CreateMessage(GameState, Arena, GameState->CurInputLine, 0);
            NewMessage->Next = GameState->Messages;
            GameState->Messages = NewMessage;
            
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
        GameState->Initialized = true;
        
        GameState->Bitmaps.Trebek = LoadBitmap("data/trebek.png");
        GameState->Bitmaps.Players = LoadBitmap("data/players.png");
        GameState->Bitmaps.Player = LoadBitmap("data/player.png");
        GameState->Bitmaps.Board = LoadBitmap("data/board.png");
        
        AddPlayer(GameState, "Chaz");
        AddPlayer(GameState, "Clay");
        AddPlayer(GameState, "Lex");
    }
    
    UpdatePlayerScore(GameState, "lex", 45);
    UpdatePlayerScore(GameState, "clay", 69);
    UpdatePlayerScore(GameState, "chaz", 45150);
    SortPlayersByScore(GameState);
    
    ProcessKeyboardInput(GameState, Keyboard, Arena);
    
    DrawUserInterface(GameState, Screen);
    DrawPlayers(GameState, Screen);
    DrawQuestion(GameState, Screen, "who was the first person to walk on the moon ?");
    DrawTopic(GameState, Screen, "this is an open question to the ladies");
    DrawInputLine(GameState, Screen);
    
    int DrawX = 10;
    int DrawY = GameState->Bitmaps.Board.Height + GameState->Bitmaps.Trebek.Height - 70;
    int MessageHeight = 15;
    int MessageWidth = GameState->Bitmaps.Board.Width - 2*DrawX;
    chat_message *CurMessage = GameState->Messages;
    SDL_Color ForegroundColor = {223, 96, 14};
    SDL_Color BackgroundColor = {3, 3, 183};
    
    int MinY = 150 + GameState->Bitmaps.Trebek.Height;
    while(CurMessage && (DrawY > MinY))
    {
        DrawText(DrawX, DrawY, MessageWidth, MessageHeight, CurMessage->Value, GameState->Font, GameState->ScreenSurface,
                 ForegroundColor, BackgroundColor);
        CurMessage = CurMessage->Next;
        DrawY -= MessageHeight;
    }
}

#define QUIZ_C
#endif