REM Batch script to build Inno Setup installer for libexpat for Windows
REM Call from parent directory, e.g.: cmd /c win32\build_expat_iss.bat
REM                          __  __            _
REM                       ___\ \/ /_ __   __ _| |_
REM                      / _ \\  /| '_ \ / _` | __|
REM                     |  __//  \| |_) | (_| | |_
REM                      \___/_/\_\ .__/ \__,_|\__|
REM                               |_| XML parser
REM
REM Copyright (C) 2019 Expat development team
REM Licensed under the MIT license:
REM
REM Permission is  hereby granted,  free of charge,  to any  person obtaining
REM a  copy  of  this  software   and  associated  documentation  files  (the
REM "Software"),  to  deal in  the  Software  without restriction,  including
REM without  limitation the  rights  to use,  copy,  modify, merge,  publish,
REM distribute, sublicense, and/or sell copies of the Software, and to permit
REM persons  to whom  the Software  is  furnished to  do so,  subject to  the
REM following conditions:
REM
REM The above copyright  notice and this permission notice  shall be included
REM in all copies or substantial portions of the Software.
REM
REM THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
REM EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
REM MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
REM NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
REM DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
REM OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
REM USE OR OTHER DEALINGS IN THE SOFTWARE.

SET GENERATOR=Visual Studio 15 2017

REM Read by msbuild!
SET CONFIGURATION=RelWithDebInfo

REM Where Inno Setup expects build results
SET BINDIR=win32\bin\Release


MD %BINDIR% || EXIT /b 1


MD build_shared_char || EXIT /b 1
CD build_shared_char || EXIT /b 1
    cmake -G"%GENERATOR%" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DEXPAT_MSVC_STATIC_CRT=ON -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_TOOLS=OFF .. || EXIT /b 1
    msbuild /m expat.sln || EXIT /b 1
    DIR %CONFIGURATION% || EXIT /b 1
    CD .. || EXIT /b 1
COPY build_shared_char\%CONFIGURATION%\libexpat.dll %BINDIR%\ || EXIT /b 1
COPY build_shared_char\%CONFIGURATION%\libexpat.lib %BINDIR%\ || EXIT /b 1


MD build_static_char || EXIT /b 1
CD build_static_char || EXIT /b 1
    cmake -G"%GENERATOR%" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DEXPAT_MSVC_STATIC_CRT=ON -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_SHARED_LIBS=OFF .. || EXIT /b 1
    msbuild /m expat.sln || EXIT /b 1
    DIR %CONFIGURATION% || EXIT /b 1
    CD .. || EXIT /b 1
COPY build_static_char\%CONFIGURATION%\libexpatMT.lib %BINDIR%\ || EXIT /b 1
COPY build_static_char\xmlwf\%CONFIGURATION%\xmlwf.exe %BINDIR%\ || EXIT /b 1


MD build_shared_wchar_t || EXIT /b 1
CD build_shared_wchar_t || EXIT /b 1
    cmake -G"%GENERATOR%" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DEXPAT_MSVC_STATIC_CRT=ON -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_TOOLS=OFF -DEXPAT_CHAR_TYPE=wchar_t .. || EXIT /b 1
    msbuild /m expat.sln || EXIT /b 1
    DIR %CONFIGURATION% || EXIT /b 1
    CD .. || EXIT /b 1
COPY build_shared_wchar_t\%CONFIGURATION%\libexpatw.dll %BINDIR%\ || EXIT /b 1
COPY build_shared_wchar_t\%CONFIGURATION%\libexpatw.lib %BINDIR%\ || EXIT /b 1


MD build_static_wchar_t || EXIT /b 1
CD build_static_wchar_t || EXIT /b 1
    cmake -G"%GENERATOR%" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DEXPAT_MSVC_STATIC_CRT=ON -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_TOOLS=OFF -DEXPAT_SHARED_LIBS=OFF -DEXPAT_CHAR_TYPE=wchar_t .. || EXIT /b 1
    msbuild /m expat.sln || EXIT /b 1
    DIR %CONFIGURATION% || EXIT /b 1
    CD .. || EXIT /b 1
COPY build_static_wchar_t\%CONFIGURATION%\libexpatwMT.lib %BINDIR%\ || EXIT /b 1


DIR %BINDIR% || EXIT /b 1
iscc win32\expat.iss || EXIT /b 1
