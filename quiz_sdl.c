#ifndef QUIZ_SDL_C

#include "stdio.h"
#include "stdlib.h"
#include "SDL.h"
#include "SDL_TTF.h"

#define PACKET_SIZE 2048
#define SERVER_PORT "5658"
#define SILENT_ERROR 0

#include "shared.h"
#include "quiz_questions.c"
#include "quiz_client.c"
#include "quiz.c"

int main(int argc, char *argv[])
{
    int ScreenWidth = 800;
    int ScreenHeight = 400;
    
    if(SDL_Init(SDL_INIT_VIDEO) >= 0)
    {
        SDL_Window *Window = SDL_CreateWindow("Alex Trebek Simulator", 
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN);
        
        if(Window)
        {
            SDL_Surface *ScreenSurface = SDL_GetWindowSurface(Window);
            
            int MemoryInBytes = MegaBytes(256);
            void *Memory = calloc(MemoryInBytes, 1);
            
            screen Screen =  {0};
            Screen.Memory = ScreenSurface->pixels;
            Screen.Width = ScreenSurface->w;
            Screen.Height = ScreenSurface->h;
            
            TTF_Init();
            
            game_state *GameState = (game_state *)Memory;
            GameState->ScreenSurface = ScreenSurface;
            GameState->Font = TTF_OpenFont("Liberation-Mono.TTF", 13);
            GameState->Arena = InitMemoryArena((u8 *)Memory + sizeof(game_state), 
                                               MemoryInBytes - sizeof(game_state));
            
            b32 Running = true;
            while(Running)
            {
                keyboard Keyboard = {0};
                key_press *ReversedInput = 0;
                
                SDL_Event Event;
                while(SDL_PollEvent(&Event) != 0)
                {
                    switch(Event.type)
                    {
                        case SDL_QUIT:
                        {
                            Running = false;
                        } break;
                        
                        case SDL_KEYDOWN:
                        {
                            int KeyValue = Event.key.keysym.sym;
                            if((Event.key.keysym.mod & KMOD_LSHIFT) || (Event.key.keysym.mod & KMOD_RSHIFT))
                            {
                                if(KeyValue >= 'a' && KeyValue <= 'z') { KeyValue = toupper(KeyValue); }
                                else if(KeyValue == '1') { KeyValue = '!'; }
                                else if(KeyValue == '/') { KeyValue = '?'; }
                            }
                            key_press *NewKeyPress = CreateKeyPress(&Keyboard, &GameState->Arena, KeyValue, ReversedInput);
                            NewKeyPress->Next = ReversedInput;
                            ReversedInput = NewKeyPress;
                        } break;
                    }
                }
                
                // TODO: UpdateAndRender  must clear the input stream each frame
                Assert(Keyboard.Input == 0);
                
                while(ReversedInput)
                {
                    key_press *PreviousKeyboardInput = Keyboard.Input;
                    Keyboard.Input = ReversedInput;
                    ReversedInput = ReversedInput->Next;
                    Keyboard.Input->Next = PreviousKeyboardInput;
                }
                
                UpdateAndRender(Memory, MemoryInBytes, &Screen, &Keyboard);
                
                SDL_UpdateWindowSurface(Window);
                SDL_Delay(10); // TODO: Actually synchronize game taking into account how long loop took
            }
            
            SDL_DestroyWindow(Window);
            SDL_Quit();
        }
        else
        {
            printf("failed to create window, error = %s\n", SDL_GetError());
        }
    }
    else
    {
        printf("failed to init window, error = %s\n", SDL_GetError());
    }
    
    return 0;
}

#define QUIZ_SDL_C
#endif