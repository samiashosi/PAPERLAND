@echo off
REM Builds every .cpp in src\ (animation_backup.cpp lives in backup\ on purpose).
g++ -std=c++11 -O2 src\*.cpp -o PAPERLAND.exe -lfreeglut -lopengl32 -lglu32
if errorlevel 1 (echo BUILD FAILED) else (echo Built PAPERLAND.exe)
