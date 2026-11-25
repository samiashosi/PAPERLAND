@echo off
REM Builds every .cpp under src\ (core, shared, chapters\*).
REM animation_backup.cpp lives in backup\ on purpose and is not compiled.
REM When you add a chapter, add its src\chapters\chapterN\*.cpp line below.
g++ -std=c++11 -O2 -Iinclude src\core\*.cpp src\shared\*.cpp src\chapters\chapter1\*.cpp -o PAPERLAND.exe -lfreeglut -lopengl32 -lglu32
if errorlevel 1 (echo BUILD FAILED) else (echo Built PAPERLAND.exe)
