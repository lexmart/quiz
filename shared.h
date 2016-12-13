#include <stdint.h>
#include <limits.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float r32;
typedef double r64;

#include <stdbool.h>
typedef int b32;

#define global_variable static
#define local_persist static
#define internal static

#define KiloBytes(Num) ((Num)*1024)
#define MegaBytes(Num) (KiloBytes((Num))*1024)
#define GigaBytes(Num) (MegaBytes((Num))*1024)

#define ArrayCount(Array) (sizeof(Array)/(sizeof((Array)[0])))

#define Assert(Expression) if(!(Expression)) { *((int *)0) = 0; }

#define InvalidCodePath Assert(!"Invalid code path");

/* NOTE: Data structures shared between platform and game */

typedef struct
{
    u32 *Memory;
    int Width;
    int Height;
} screen;

typedef struct key_press
{
    char Value;
    struct key_press *Next;
} key_press;

typedef struct
{
    // NOTE: Linked list of keys pressed during frame in the order they were pressed
    key_press *Input;
    key_press *FreeKeys;
} keyboard;

typedef struct player
{
    int Score;
    char Name[64];
} player;

typedef struct chat_message
{
    char Value[1024];
    struct chat_message *Next;
} chat_message;

typedef enum
{
    PacketType_PlayerList,
} packet_type;