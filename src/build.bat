@echo off

ctime -begin stats.ctm

pushd ..\bin



set CompilerFlags=-nologo -Od -MTd -fp:fast -fp:except- -EHa- -Gm- -GR- -Zi -Zo -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4701 -wd4456 -wd4706 -wd4204 -wd4101 -wd4311 -FC
set LinkerFlags=-incremental:no -opt:ref kernel32.lib Gdi32.lib User32.lib Shell32.lib Winmm.lib

REM cl %CompilerFlags% ..\src\compress.c /link %LinkerFlags%

cl %CompilerFlags% -Feriddick_tool.exe ..\src\win32.c /link %LinkerFlags%
set ErrorLevel=%ERRORLEVEL%

del *.obj

popd

ctime -end stats.ctm %ErrorLevel%
