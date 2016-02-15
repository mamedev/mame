@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

REM *****************************
REM *   Customization section   *
REM *****************************

REM use the /help option for generic usage information

REM Where is the source code located (the unpacked Lua source archive, toplevel dir)
SET SOURCETREE=.\

REM set the toolchain to either MS or GCC (allcaps), leave blank to autodetect
SET TOOLCHAIN=

REM set the compatibility flags, defaults to empty for 5.1, -DLUA_COMPAT_ALL for 5.2, 
REM and -DLUA_COMPAT_5_2 for 5.3, which are the same as the unix make files
REM This setting can be overridden with the --nocompat flag
SET COMPATFLAG=









REM **********************************
REM *   Nothing to customize below   *
REM **********************************

SET BATCHNAME=%~n0
SET SOURCE=%SOURCETREE%src\
SET LUA_H=%SOURCE%lua.h
SET CURDIR=%CD%

REM the following line ends with a TAB. DO NOT REMOVE IT!
SET TABCHAR=	
REM Define LF to contain a linefeed character
set ^"LFCHAR=^

^" The above empty line is critical. DO NOT REMOVE


REM Supported toolchains (allcaps)
SET TOOLCHAINS=MS GCC
REM Commands which, if exiting without error, indicate presence of the toolchain
SET CHECK_GCC=gcc --version
SET CHECK_MS=cl

REM **********************************
REM *   Check for help request       *
REM **********************************

SET HELPCMDS=help -help --help /help ? -? /?
for %%L in ("!LFCHAR!") do for /f %%a in ("!HELPCMDS: =%%~L!") do (
   if "%%a"=="%~1" (
      echo.
      echo Builds a standalone Lua installation. Supports Lua version 5.1, 5.2 and 5.3.
      echo Your compiler must be in the system path, and this "%BATCHNAME%.bat" file must be located
      echo in ".\etc\" in the unpacked Lua source archive.
      echo.
      echo USAGE etc\%BATCHNAME% [FLAG] [COMMAND] [...]
      echo ^(execute from the root of the unpacked archive^)
      echo.
      echo Commands;
      echo   clean          : cleans the source tree of build ^(intermediate^) files
      echo   install [path] : installs the build results into "path"
      echo   local          : installs into ".\local\" in the unpacked Lua source structure
      echo   [toolchain]    : uses a specific toolchain to build. If not provided then supported
      echo                    toolchains will be tested and the first available will be picked.
      echo                    Supported toolchains are: "%TOOLCHAINS%" ^(must use ALLCAPS^)
      echo.
      echo Flags;
      echo   --nocompat     : Specifies that no compatibility flags should be set when building.
      echo                    If not specified, the default compatibility flags will be used.
      echo.
      echo Example use;
      echo   set PATH=C:\path\to\your\compiler\;%%PATH%%
      echo   etc\%BATCHNAME% clean
      echo   etc\%BATCHNAME%
      echo   etc\%BATCHNAME% --nocompat GCC
      echo   etc\%BATCHNAME% install "C:\Program Files\Lua"
      echo.
      goto :EXITOK
   )
)

REM **********************************
REM *   Check commandline            *
REM **********************************

SET CMDOK=FALSE
if "%~1"=="" (
   SET CMDOK=TRUE
)
for %%a in (local install clean) do ( 
   if "%%a"=="%~1" (
      SET CMDOK=TRUE
   )
)
for %%a in (--nocompat) do ( 
   if "%%a"=="%~1" (
      SET NOCOMPAT=TRUE
      if "%~2"=="" (
         SET CMDOK=TRUE
      )
      SHIFT
   )
)
for %%a in (%TOOLCHAINS%) do ( 
   if "%%a"=="%~1" (
      SET CMDOK=TRUE
      SET TOOLCHAIN=%~1
   )
)
if NOT %CMDOK%==TRUE (
   echo.
   echo Unknown command or toolchain specified.
   goto :EXITERROR
)

REM **************************************
REM *   Check for cleaning               *
REM **************************************

if "%1"=="clean" (
   if NOT [%2]==[] (
      echo.
      echo ERROR: The clean command does not take extra parameters.
   ) else (
      echo Cleaning...
      if exist "%SOURCE%*.exe" del "%SOURCE%*.exe"
      if exist "%SOURCE%*.dll" del "%SOURCE%*.dll"
      if exist "%SOURCE%*.o" del "%SOURCE%*.o"
      if exist "%SOURCE%*.a" del "%SOURCE%*.a"
      if exist "%SOURCE%*.obj" del "%SOURCE%*.obj"
      if exist "%SOURCE%*.manifest" del "%SOURCE%*.manifest"
      if exist "%SOURCE%*.lib" del "%SOURCE%*.lib"
      echo Done.
   )
   goto :EXITOK
)

REM **************************************************
REM *   Fetch the Lua version from the source code   *
REM **************************************************

Echo.
Echo Checking source code to extract Lua version...
IF NOT EXIST %LUA_H% (
   Echo Cannot locate Lua header file; %LUA_H%
   goto :EXITERROR
)

findstr /R /C:"#define[ %TABCHAR%][ %TABCHAR%]*LUA_VERSION_MAJOR"  %LUA_H% > NUL
if NOT %ERRORLEVEL%==0 (
   rem ECHO We've got a Lua version 5.1
   rem findstr /R /C:"#define[ %TABCHAR%][ %TABCHAR%]*LUA_VERSION[ %TABCHAR%]"  %LUA_H%
   SET LUA_VER=5.1
) else (
   rem ECHO We've got a Lua version 5.2+
   rem findstr /R /C:"#define[ %TABCHAR%][ %TABCHAR%]*LUA_VERSION_MAJOR[ %TABCHAR%]"  %LUA_H%
   rem findstr /R /C:"#define[ %TABCHAR%][ %TABCHAR%]*LUA_VERSION_MINOR[ %TABCHAR%]"  %LUA_H%

   for /F "delims=" %%a in ('findstr /R /C:"#define[ %TABCHAR%][ %TABCHAR%]*LUA_VERSION_MAJOR[ %TABCHAR%]"  %LUA_H%') do set LUA_MAJOR=%%a
   SET LUA_MAJOR=!LUA_MAJOR:#define=!
   SET LUA_MAJOR=!LUA_MAJOR:LUA_VERSION_MAJOR=!
   SET LUA_MAJOR=!LUA_MAJOR: =!
   SET LUA_MAJOR=!LUA_MAJOR:%TABCHAR%=!
   SET LUA_MAJOR=!LUA_MAJOR:"=!
   SET LUA_MAJOR=!LUA_MAJOR:~0,1!

   for /F "delims=" %%a in ('findstr /R /C:"#define[ %TABCHAR%][ %TABCHAR%]*LUA_VERSION_MINOR[ %TABCHAR%]"  %LUA_H%') do set LUA_MINOR=%%a
   SET LUA_MINOR=!LUA_MINOR:#define=!
   SET LUA_MINOR=!LUA_MINOR:LUA_VERSION_MINOR=!
   SET LUA_MINOR=!LUA_MINOR: =!
   SET LUA_MINOR=!LUA_MINOR:%TABCHAR%=!
   SET LUA_MINOR=!LUA_MINOR:"=!
   SET LUA_MINOR=!LUA_MINOR:~0,1!

   SET LUA_VER=!LUA_MAJOR!.!LUA_MINOR!
)
SET LUA_SVER=!LUA_VER:.=!

Echo Lua version found: %LUA_VER%
Echo.

REM **************************************
REM *   Set some Lua version specifics   *
REM **************************************

REM FILES_CORE; files for Lua core (+lauxlib, needed for Luac)
REM FILES_LIB; files for Lua standard libraries
REM FILES_DLL; vm files to be build with dll option
REM FILES_OTH; vm files to be build without dll, for static linking

if %LUA_SVER%==51 (
   set FILES_CORE=lapi lcode ldebug ldo ldump lfunc lgc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lauxlib
   set FILES_LIB=lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib linit
   set FILES_DLL=lua 
   set FILES_OTH=luac print
   set INSTALL_H=lauxlib.h lua.h luaconf.h lualib.h ..\etc\lua.hpp
)
if %LUA_SVER%==52 (
   set FILES_CORE=lapi lcode lctype ldebug ldo ldump lfunc lgc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lauxlib
   set FILES_LIB=lbaselib lbitlib lcorolib ldblib liolib lmathlib loslib lstrlib ltablib loadlib linit
   set FILES_DLL=lua
   set FILES_OTH=luac
   set INSTALL_H=lauxlib.h lua.h lua.hpp luaconf.h lualib.h
   if "%COMPATFLAG%"=="" (
      set COMPATFLAG=-DLUA_COMPAT_ALL
   )
)
if %LUA_SVER%==53 (
   set FILES_CORE=lapi lcode lctype ldebug ldo ldump lfunc lgc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lauxlib
   set FILES_LIB=lbaselib lbitlib lcorolib ldblib liolib lmathlib loslib lstrlib ltablib lutf8lib loadlib linit
   set FILES_DLL=lua
   set FILES_OTH=luac
   set INSTALL_H=lauxlib.h lua.h lua.hpp luaconf.h lualib.h
   if "%COMPATFLAG%"=="" (
      set COMPATFLAG=-DLUA_COMPAT_5_2
   )
)

if "%NOCOMPAT%"=="TRUE" (
   set COMPATFLAG=
)

SET FILES_BASE=%FILES_DLL% %FILES_CORE% %FILES_LIB%

if "%FILES_BASE%"=="" (
   Echo Unknown Lua version; %LUA_VER%
   goto :EXITERROR
)

REM *********************************
REM *   Check available toolchain   *
REM *********************************

if [%TOOLCHAIN%]==[] (
   Echo Testing for MS...
   %CHECK_MS%
   IF !ERRORLEVEL!==0 SET TOOLCHAIN=MS
)
if [%TOOLCHAIN%]==[] (
   Echo Testing for GCC...
   %CHECK_GCC%
   IF !ERRORLEVEL!==0 SET TOOLCHAIN=GCC
)
if [%TOOLCHAIN%]==[] (
   Echo No supported toolchain found ^(please make sure it is in the system path^)
   goto :EXITERROR
)

REM ***************************
REM *   Configure toolchain   *
REM ***************************

if %TOOLCHAIN%==GCC (
   echo Using GCC toolchain...
   SET OBJEXT=o
   SET LIBFILE=liblua%LUA_SVER%.a
)
if %TOOLCHAIN%==MS (
   echo Using Microsoft toolchain...
   SET OBJEXT=obj
   SET LIBFILE=lua%LUA_SVER%.lib
)
echo.

REM **************************************
REM *   Check for installing             *
REM **************************************

if "%1"=="install" (
   if "%~2"=="" (
      echo.
      echo ERROR: The install command requires a path where to install to.
      goto :EXITERROR
   )
   SET TARGETPATH=%~2
)
if "%1"=="local" (
   if NOT "%~2"=="" (
      echo.
      echo ERROR: The local command does not take extra parameters.
      goto :EXITERROR
   )
   SET TARGETPATH=%SOURCETREE%local
)
if NOT "%TARGETPATH%"=="" (
   mkdir "%TARGETPATH%\bin"
   mkdir "%TARGETPATH%\include"
   mkdir "%TARGETPATH%\lib\lua\%LUA_VER%"
   mkdir "%TARGETPATH%\man\man1"
   mkdir "%TARGETPATH%\share\lua\%LUA_VER%"
   copy "%SOURCE%lua.exe" "%TARGETPATH%\bin"
   copy "%SOURCE%luac.exe" "%TARGETPATH%\bin"
   copy "%SOURCE%lua%LUA_SVER%.dll" "%TARGETPATH%\bin"
   for %%a in (%INSTALL_H%) do ( copy "%SOURCE%%%a" "%TARGETPATH%\include" )
   copy "%SOURCE%%LIBFILE%" "%TARGETPATH%\lib"
   copy "%SOURCETREE%doc\lua.1" "%TARGETPATH%\man\man1"
   copy "%SOURCETREE%doc\luac.1" "%TARGETPATH%\man\man1"

   echo Installation completed in "%TARGETPATH%".
   goto :EXITOK
)

REM ***********************
REM *   Compile sources   *
REM ***********************
goto :after_compile_function
:compile_function
   REM Params: %1 is filelist (must be quoted)
   REM Return: same list, with the object file extension included, will be stored in global OBJLIST

   for %%a in (%~1) do (
      SET FILENAME=%%a
      if %TOOLCHAIN%==GCC (
         SET COMPCMD=gcc -O2 -Wall !EXTRAFLAG! !COMPATFLAG! -c -o !FILENAME!.%OBJEXT% !FILENAME!.c
      )
      if %TOOLCHAIN%==MS (
         SET COMPCMD=cl /nologo /MD /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE !COMPATFLAG! !EXTRAFLAG! !FILENAME!.c
      )
      echo !COMPCMD!
      !COMPCMD!
      SET OBJLIST=!OBJLIST! !FILENAME!.%OBJEXT%
   )

goto :eof
:after_compile_function

CD %SOURCE%
REM Traverse the 4 lists of source files

for %%b in (CORE LIB DLL OTH) do (
   SET LTYPE=%%b
   SET OBJLIST=
   if !LTYPE!==OTH (
      REM OTH is the only list of files build without DLL option
      SET EXTRAFLAG=
   ) else (
      SET EXTRAFLAG=-DLUA_BUILD_AS_DLL
   )
   if !LTYPE!==CORE SET FILELIST=%FILES_CORE%
   if !LTYPE!==LIB SET FILELIST=%FILES_LIB%
   if !LTYPE!==DLL SET FILELIST=%FILES_DLL%
   if !LTYPE!==OTH SET FILELIST=%FILES_OTH%

   echo Now compiling !LTYPE! file set...
   call:compile_function "!FILELIST!"

   if !LTYPE!==CORE SET FILES_CORE_O=!OBJLIST!
   if !LTYPE!==LIB SET FILES_LIB_O=!OBJLIST!
   if !LTYPE!==DLL SET FILES_DLL_O=!OBJLIST!
   if !LTYPE!==OTH SET FILES_OTH_O=!OBJLIST!
)


REM ****************************
REM *   Link GCC based files   *
REM ****************************

if %TOOLCHAIN%==GCC (
   REM Link the LuaXX.dll file
   SET LINKCMD=gcc -shared -o lua%LUA_SVER%.dll %FILES_CORE_O% %FILES_LIB_O%
   echo !LINKCMD!
   !LINKCMD!

   REM strip from LuaXX.dll
   SET RANCMD=strip --strip-unneeded lua%LUA_SVER%.dll
   echo !RANCMD!
   !RANCMD!

   REM Link the Lua.exe file
   SET LINKCMD=gcc -o lua.exe -s lua.%OBJEXT% lua%LUA_SVER%.dll -lm
   echo !LINKCMD!
   !LINKCMD!

   REM create lib archive
   SET LIBCMD=ar rcu liblua%LUA_SVER%.a %FILES_CORE_O% %FILES_LIB_O%
   echo !LIBCMD!
   !LIBCMD!

   REM Speedup index using ranlib
   SET RANCMD=ranlib liblua%LUA_SVER%.a
   echo !RANCMD!
   !RANCMD!

   REM Link Luac.exe file
   SET LINKCMD=gcc -o luac.exe  %FILES_OTH_O% liblua%LUA_SVER%.a -lm
   echo !LINKCMD!
   !LINKCMD!

)


REM ****************************
REM *   Link MS based files    *
REM ****************************

if %TOOLCHAIN%==MS (
   REM Link the LuaXX.dll file, and LuaXX.obj
   SET LINKCMD=link /nologo /DLL /out:lua%LUA_SVER%.dll %FILES_CORE_O% %FILES_LIB_O%
   echo !LINKCMD!
   !LINKCMD!

   REM handle dll manifest
   if exist lua%LUA_SVER%.dll.manifest (
      SET MANICMD=mt /nologo -manifest lua%LUA_SVER%.dll.manifest -outputresource:lua%LUA_SVER%.dll;2
      echo !MANICMD!
      !MANICMD!
   )

   REM Link Lua.exe
   SET LINKCMD=link /nologo /out:lua.exe lua.%OBJEXT% lua%LUA_SVER%.lib
   echo !LINKCMD!
   !LINKCMD!

   REM handle manifest
   if exist lua.exe.manifest (
      SET MANICMD=mt /nologo -manifest lua.exe.manifest -outputresource:lua.exe
      echo !MANICMD!
      !MANICMD!
   )

   REM Link Luac.exe
   SET LINKCMD=link /nologo /out:luac.exe %FILES_OTH_O% %FILES_CORE_O%
   echo !LINKCMD!
   !LINKCMD!

   REM handle manifest
   if exist luac.exe.manifest (
      SET MANICMD=mt /nologo -manifest luac.exe.manifest -outputresource:luac.exe
      echo !MANICMD!
      !MANICMD!
   )
)

CD %CURDIR%

REM ****************************
REM *   Finished building      *
REM ****************************

echo.
echo Build completed.
goto :EXITOK

:EXITOK
exit /B 0

:EXITERROR
echo For help try; etc\%BATCHNAME% /help
exit /B 1
