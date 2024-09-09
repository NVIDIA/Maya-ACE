@echo off

set "BASE_DIR=%~dp0"
set PATH=%PATH%;%BASE_DIR%deps\premake

premake5.exe vs2022 --solution-name=tests-aceclient --aceclient_log_level=1
MSBuild.exe /NoWarn:MSB8003,MSB8005 .\_compiler\tests-aceclient.sln /p:Configuration=release /p:Platform=x64 /verbosity:minimal /m /t:tests-aceclient

premake5.exe vs2022 --solution-name=maya-ace --aceclient_log_level=1
MSBuild.exe /NoWarn:MSB8003,MSB8005 .\_compiler\maya-ace.sln /p:Configuration=release /p:Platform=x64 /verbosity:minimal /m /t:mace
