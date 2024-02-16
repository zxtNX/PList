@echo off
SETLOCAL

IF NOT EXIST build (mkdir build)
cd build

REM Détecter le système d'exploitation
IF "%OS%"=="Windows_NT" (
    REM Windows détecté
    REM Utiliser un générateur par défaut pour Windows, par exemple Ninja ou nmake
    SET GENERATOR=Ninja
) ELSE (
    echo "Ce script est conçu pour fonctionner uniquement sous Windows."
    exit /b
)

REM Configuration CMake
cmake -G %GENERATOR% %ARCH% -DCMAKE_BUILD_TYPE=Release ..

REM Construction
cmake --build .

cd ..
ENDLOCAL
pause
