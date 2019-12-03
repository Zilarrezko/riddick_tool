@echo off

pushd ..\bin

cl -nologo ..\src\frame_script.c /LD /link User32.lib /EXPORT:frame_limit

popd
