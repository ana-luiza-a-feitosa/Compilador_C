@echo off
REM Script de compilacao para Windows (MinGW/GCC)
REM Execute: build.bat

echo ==========================================
echo   COMPILANDO COMPILADOR C-MINUS
echo ==========================================
echo.

REM Verifica se o GCC esta instalado
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERRO: GCC nao encontrado!
    echo Instale MinGW ou configure o PATH
    pause
    exit /b 1
)

echo Limpando arquivos antigos...
if exist cminus.exe del cminus.exe
if exist *.o del *.o
if exist *.tm del *.tm

echo.
echo Compilando modulos...

echo - Compilando main.c...
gcc -Wall -Wextra -std=c99 -c main.c
if %ERRORLEVEL% NEQ 0 goto :error

echo - Compilando util.c...
gcc -Wall -Wextra -std=c99 -c util.c
if %ERRORLEVEL% NEQ 0 goto :error

echo - Compilando scan.c...
gcc -Wall -Wextra -std=c99 -c scan.c
if %ERRORLEVEL% NEQ 0 goto :error

echo - Compilando parse.c...
gcc -Wall -Wextra -std=c99 -c parse.c
if %ERRORLEVEL% NEQ 0 goto :error

echo - Compilando symtab.c...
gcc -Wall -Wextra -std=c99 -c symtab.c
if %ERRORLEVEL% NEQ 0 goto :error

echo - Compilando analyze.c...
gcc -Wall -Wextra -std=c99 -c analyze.c
if %ERRORLEVEL% NEQ 0 goto :error

echo - Compilando codegen.c...
gcc -Wall -Wextra -std=c99 -c codegen.c
if %ERRORLEVEL% NEQ 0 goto :error

echo.
echo Linkando executavel...
gcc -Wall -Wextra -std=c99 -o cminus.exe main.o util.o scan.o parse.o symtab.o analyze.o codegen.o
if %ERRORLEVEL% NEQ 0 goto :error

echo.
echo ==========================================
echo   COMPILACAO BEM-SUCEDIDA!
echo ==========================================
echo.
echo Executavel gerado: cminus.exe
echo.
echo Para testar, execute:
echo   cminus.exe test.cm
echo.
pause
exit /b 0

:error
echo.
echo ==========================================
echo   ERRO NA COMPILACAO!
echo ==========================================
echo.
pause
exit /b 1