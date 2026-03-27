@ECHO off
TITLE Stop Sub-Node
 

setlocal

SET exefile=iapl_chord.exe
taskkill /IM %exefile% /t /f
: PAUSE
echo END

exit /b 0