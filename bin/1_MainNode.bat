@echo off
setlocal EnableDelayedExpansion

set "exefile=iapl_chord.exe"

:: exe daemon
start /d ".\"  %exefile% c 6000

PAUSE
endlocal
exit /b 0
