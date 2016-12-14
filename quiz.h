#ifndef QUIZ_H

#include "stb_image.h"

typedef struct
{
    u32 *Memory;
    int Width;
    int Height;
} bitmap;

typedef struct
{
    void *Base;
    int BytesUsed;
    int BytesTotal;
} memory_arena;

void *PushBytes(memory_arena *Arena, int NumBytes)
{
    int BytesLeft = Arena->BytesTotal - Arena->BytesUsed;
    Assert(BytesLeft >= NumBytes);
    void *Result = (u8 *)Arena->Base + Arena->BytesUsed;
    Arena->BytesUsed += NumBytes;
    return Result;
}
#define PushStruct(type, Arena) (type *)PushBytes(Arena, sizeof(type))

typedef struct
{
    bitmap Trebek;
    bitmap Players;
    bitmap Player;
    bitmap Board;
    bitmap Font;
    bitmap Chars[128];
} bitmaps;

typedef struct
{
    b32 Initialized;
    
    SDL_Surface *ScreenSurface;
    TTF_Font *Font;
    
    memory_arena Arena;
    
    bitmaps Bitmaps;
    
    int NumPlayers;
    player Players[8];
    
    char CurInputLine[1024];
    int InputLineLength;
    
    chat_message *Messages;
    chat_message *FreeMessages;
    
    question Q;
    
    void *NetworkState;
} game_state;

#define QUIZ_H
#endif