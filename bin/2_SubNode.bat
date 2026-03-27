@echo off
setlocal EnableDelayedExpansion

:: 실행할 exe 이름 (필요시 수정)
set "exefile=iapl_chord.exe"

:: 콘솔 한글 깨짐 방지
chcp 65001 >nul

:: 사용법:
::   run_many_iapl.bat [count] [base_port]
:: 예) run_many_iapl.bat 5 5000    -> 5000,5001,5002,5003,5004
:: 인자를 주지 않으면 프롬프트로 입력받음.

if "%~1"=="" (
    set /p count=Input number of instacne:
) else (
    set "count=%~1"
)

if "%~2"=="" (
    set "base=6000"
) else (
    set "base=%~2"
)

rem 입력 검증: count가 숫자인지 간단 체크
for /f "delims=0123456789" %%x in ("%count%") do (
    echo error: must be number.
    pause
    exit /b 1
)

if %count% LEQ 0 (
    echo error: more than 1 .
    pause
    exit /b 1
)

echo Starting %count% instances of %exefile% starting at port %base%...
echo.

rem 0부터 count-1까지 반복
for /L %%i in (1,1,%count%-1) do (
    set /a port=%base%+%%i
    echo [%date% %time%] Starting instance %%i on port !port! ...
    start "" /d ".\" %exefile% c !port!
)

echo.
echo End.
pause
endlocal
exit /b 0
