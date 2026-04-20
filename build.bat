@echo off
setlocal

set TARGET=wol.exe

where cl >nul 2>&1
if %errorlevel% == 0 (
    echo Building with MSVC...
    cl /nologo /O2 /W3 /Fe:%TARGET% wol.c ws2_32.lib
    if %errorlevel% neq 0 exit /b %errorlevel%
    del /f /q wol.obj >nul 2>&1
    goto done
)

where gcc >nul 2>&1
if %errorlevel% == 0 (
    echo Building with GCC...
    gcc -O2 -Wall -Wextra -o %TARGET% wol.c -lws2_32
    if %errorlevel% neq 0 exit /b %errorlevel%
    goto done
)

echo Error: neither cl (MSVC) nor gcc (MinGW) found in PATH. >&2
exit /b 1

:done
echo Built: %TARGET%
endlocal
