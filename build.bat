@echo off

set EntryFile=../chip8_main.c
set VariableFlags=

set WarningFlags=-nologo -WX -W4 -wd4201 -wd4189 -wd4505 -wd4238 -wd4100 -wd4026 -wd4244 -wd4996 -wd4127 -wd4101

REM -FC = Full path of source file in diagnostics (so emacs can parse errors/warning)
REM -Zi = Creates debug information for Visual Studio debugger (Do I need to turn this off is release builds?)
REM -LD = Build DLL file
REM -Od = Turn off all optimizations
REM -incremental:no = To stop annyoing full link message

set CompilerFlags=%WarningFlags% %VariableFlags% -FC -Zi -Od
set LinkerFlags=-incremental:no -LIBPATH:../chip8/SDL/lib/x64/SDL2.lib
set ExternalLibraries=User32.lib Gdi32.lib Winmm.lib

pushd build

REM cl %CompilerFlags% %EntryFile% %ExternalLibraries% -I SDL/include /link %LinkerFlags%

cl %CompilerFlags% ../quiz_sdl.c  /I W:\vq\SDL\include /link ws2_32.lib User32.lib /LIBPATH:W:\vq\SDL\lib\x64 SDL2.lib SDL2main.lib SDL2_ttf.lib /SUBSYSTEM:CONSOLE

cl %CompilerFlags% ../quiz_server.c /link ws2_32.lib User32.lib /SUBSYSTEM:CONSOLE
REM cl %CompilerFlags% ../quiz_client.c /link ws2_32.lib User32.lib /SUBSYSTEM:CONSOLE

popd