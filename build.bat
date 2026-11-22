@echo off
setlocal
where cmake >nul 2>nul
if errorlevel 1 (
  echo CMake was not found. Install Visual Studio 2022 with Desktop development with C++ and CMake.
  pause
  exit /b 1
)
cmake -S . -B build -A x64
if errorlevel 1 goto failed
cmake --build build --config Release
if errorlevel 1 goto failed
echo.
echo Build complete. Look for build\Release\LuauDesk.exe
pause
exit /b 0
:failed
echo.
echo Build failed. Check the error messages above.
pause
exit /b 1
