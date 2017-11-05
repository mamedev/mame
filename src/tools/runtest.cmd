@rem ----------------------------------------------------
@rem MAME Testing script
@rem (Windows only at the moment, sorry!)
@rem
@rem Initial setup of the script:
@rem
@rem    1. Create a fresh directory mametest/
@rem    2. Copy this script into it (mametest/runtest.cmd)
@rem    3. Copy a mame.ini with your ROM paths into it
@rem        (mametest/mame.ini)
@rem    4. Copy a transparent crosshair cursor into it
@rem        (mametest/cross.png)
@rem
@rem How to run a test:
@rem
@rem    1. Create a new subdirectory mametest/version/
@rem    2. Copy mame.exe into it (mametest/version/mame.exe)
@rem    3. Open a command prompt to mametest/version
@rem    4. Run "..\runtest"
@rem    5. Wait for all the tests to complete
@rem       (Note one window will be opened for each CPU)
@rem
@rem How to generate a report:
@rem
@rem    1. Concatenate the summary files together:
@rem         copy summary*.log summary-final.log
@rem    1. Open a command prompt to mametest.
@rem    2. Make sure you have run tests for at least two
@rem        versions (mametest/ver1 and mametest/ver2)
@rem    3. Create an output directory (mametest/report)
@rem    4. Run "regrep report ver1\summary-final.log ver2\summary-final.log"
@rem    5. Upload the report directory to mamedev.org :)
@rem    6. Differing files are printed to stdout; redirect
@rem        to create a list that can be run again via
@rem        this script
@rem ----------------------------------------------------

@echo off
@setlocal ENABLEDELAYEDEXPANSION

@rem ----------------------------------------------------
@rem We require mame.exe to be present
@rem ----------------------------------------------------

if not exist mame.exe (
@echo Missing mame.exe!
@goto :eof
)

@rem ----------------------------------------------------
@rem By default we generate our own list; however, a list
@rem can be specified by an alternate parameter. If a
@rem parameter is given, we leave the existing log and
@rem snap directories intact; otherwise, we delete them
@rem and start fresh.
@rem ----------------------------------------------------

set LIST=gamelist.txt
set SUMMARY=summary.log
if "%1"=="" (
@echo Generating full list
mame -ls >%LIST%
@echo Deleting old data
if exist log rmdir /s/q log
if exist snap rmdir /s/q snap
if exist %SUMMARY% del %SUMMARY%
) else (
set LIST=%1
@echo Re-testing %1
)

@rem ----------------------------------------------------
@rem If we have a %2 parameter, then this is a sublaunch
@rem and we should go right to the execution.
@rem ----------------------------------------------------

if not "%2"=="" (
set SUMMARY=summary%2.log
goto :sublaunch
)

@rem ----------------------------------------------------
@rem Always delete all cfg, nvram, and diff files.
@rem ----------------------------------------------------

if exist cfg rmdir /s/q cfg
if exist nvram rmdir /s/q nvram
if exist diff rmdir /s/q diff

@rem ----------------------------------------------------
@rem Make sure we use transparent crosshairs.
@rem ----------------------------------------------------

if not exist artwork mkdir artwork
copy /y ..\cross.png artwork\cross0.png
copy /y ..\cross.png artwork\cross1.png
copy /y ..\cross.png artwork\cross2.png
copy /y ..\cross.png artwork\cross3.png

@rem ----------------------------------------------------
@rem If we don't yet have a summary.log, create a new one.
@rem ----------------------------------------------------

if not exist %SUMMARY% (
mame -help >%SUMMARY%
echo @@@@@dir=%CD%>>%SUMMARY%
)

@rem ----------------------------------------------------
@rem Create the log directory and a starting timestamp.
@rem ----------------------------------------------------

if not exist log mkdir log
echo @@@@@start=%TIME%>>%SUMMARY%

@rem ----------------------------------------------------
@rem Iterate over processors and sublaunch an entry for
@rem each one.
@rem ----------------------------------------------------

for /l %%c in (1,1,%NUMBER_OF_PROCESSORS%) do (
set /a CPU="%%c - 1"
@echo call %0 %LIST% !CPU!
start %0 %LIST% !CPU!
)
goto :eof



@rem ----------------------------------------------------
@rem Iterate over drivers in the log, extracting the
@rem source filename as well, and passing both to runone.
@rem ----------------------------------------------------

:sublaunch
set CPU=0
for /f "tokens=1-5 delims=/ " %%i in (%LIST%) do (
set /a CPU="(!CPU! + 1) %% %NUMBER_OF_PROCESSORS%"
if not "!CPU!"=="%2" (
@rem do nothing
) else if not "%%m"=="" (
call :runone %%i %%m
) else if not "%%l"=="" (
call :runone %%i %%l
) else if not "%%k"=="" (
call :runone %%i %%k
) else (
call :runone %%i %%j
)
)

@rem ----------------------------------------------------
@rem Add a final timestamp and we're done.
@rem ----------------------------------------------------

echo @@@@@stop=%TIME%>>%SUMMARY%
goto :eof



@rem ----------------------------------------------------
@rem runone: Execute a single game for 30 seconds and
@rem output the results to the summary.log.
@rem ----------------------------------------------------

:runone
@echo Testing %1 (%2)...
echo.>>%SUMMARY%
mame %1 -str 30 -watchdog 300 -nodebug -nothrottle -inipath .. -video none -sound none 1>log\%1.txt 2>log\%1.err
if %errorlevel% equ 100 (
echo @@@@@driver=%1: Exception>>%SUMMARY%
type log\%1.err >>%SUMMARY%
) else if %errorlevel% equ 5 (
@rem Do nothing -- game does not exist in this build
) else if %errorlevel% equ 3 (
echo @@@@@driver=%1: Fatal error>>%SUMMARY%
type log\%1.err >>%SUMMARY%
) else if %errorlevel% equ 2 (
echo @@@@@driver=%1: Missing files>>%SUMMARY%
type log\%1.err >>%SUMMARY%
) else if %errorlevel% equ 1 (
echo @@@@@driver=%1: Failed validity check>>%SUMMARY%
type log\%1.err >>%SUMMARY%
) else if %errorlevel% equ 0 (
echo @@@@@driver=%1: Success>>%SUMMARY%
) else (
echo @@@@@driver=%1: Unknown error %errorlevel%>>%SUMMARY%
)
echo @@@@@source=%2>>%SUMMARY%
goto :eof
