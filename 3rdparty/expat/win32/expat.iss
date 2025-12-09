; Basic setup script for the Inno Setup installer builder.  For more
; information on the free installer builder, see www.jrsoftware.org.
;
; This script was originally contributed by Tim Peters.
; It was designed for Inno Setup 2.0.19 but works with later versions as well.
;
;                          __  __            _
;                       ___\ \/ /_ __   __ _| |_
;                      / _ \\  /| '_ \ / _` | __|
;                     |  __//  \| |_) | (_| | |_
;                      \___/_/\_\ .__/ \__,_|\__|
;                               |_| XML parser
;
; Copyright (c) 2001      Tim Peters <tim.peters@gmail.com>
; Copyright (c) 2001-2005 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
; Copyright (c) 2006-2017 Karl Waclawek <karl@waclawek.net>
; Copyright (c) 2007-2025 Sebastian Pipping <sebastian@pipping.org>
; Copyright (c) 2022      Johnny Jazeix <jazeix@gmail.com>
; Copyright (c) 2024      Dag-Erling Smørgrav <des@des.dev>
; Licensed under the MIT license:
;
; Permission is  hereby granted,  free of charge,  to any  person obtaining
; a  copy  of  this  software   and  associated  documentation  files  (the
; "Software"),  to  deal in  the  Software  without restriction,  including
; without  limitation the  rights  to use,  copy,  modify, merge,  publish,
; distribute, sublicense, and/or sell copies of the Software, and to permit
; persons  to whom  the Software  is  furnished to  do so,  subject to  the
; following conditions:
;
; The above copyright  notice and this permission notice  shall be included
; in all copies or substantial portions of the Software.
;
; THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
; EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
; NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
; DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
; OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
; USE OR OTHER DEALINGS IN THE SOFTWARE.

#define expatVer "2.7.1"

[Setup]
AppName=Expat
AppId=expat
AppVersion={#expatVer}
AppVerName=Expat {#expatVer}
AppCopyright=Copyright © 1997-2025 Thai Open Source Software Center, Clark Cooper, and the Expat maintainers
AppPublisher=The Expat Developers
AppPublisherURL=https://libexpat.github.io/
AppSupportURL=https://libexpat.github.io/
AppUpdatesURL=https://libexpat.github.io/
UninstallDisplayName=Expat XML Parser {#expatVer}
VersionInfoVersion={#expatVer}
OutputBaseFilename=expat-win32bin-{#expatVer}

DefaultDirName={pf}\Expat {#expatVer}
UninstallFilesDir={app}\Uninstall

Compression=lzma
SolidCompression=yes
SourceDir=..
OutputDir=win32
DisableStartupPrompt=yes
AllowNoIcons=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes

[Files]
Flags: ignoreversion; Source: win32\bin\Release\xmlwf.exe;  DestDir: "{app}\Bin"
Flags: ignoreversion; Source: win32\MANIFEST.txt;           DestDir: "{app}"
Flags: ignoreversion; Source: AUTHORS;                      DestDir: "{app}"; DestName: AUTHORS.txt
Flags: ignoreversion; Source: Changes;                      DestDir: "{app}"; DestName: Changes.txt
Flags: ignoreversion; Source: COPYING;                      DestDir: "{app}"; DestName: COPYING.txt
Flags: ignoreversion; Source: README.md;                    DestDir: "{app}"; DestName: README.txt
Flags: ignoreversion; Source: doc\*.html;                   DestDir: "{app}\Doc"
Flags: ignoreversion; Source: doc\*.css;                    DestDir: "{app}\Doc"
Flags: ignoreversion; Source: doc\*.xml;                    DestDir: "{app}\Source\doc"
Flags: ignoreversion; Source: win32\bin\Release\*.dll;      DestDir: "{app}\Bin"
Flags: ignoreversion; Source: win32\bin\Release\*.lib;      DestDir: "{app}\Bin"
Flags: ignoreversion; Source: win32\version.rc.cmake;       DestDir: "{app}\Source\win32"
Flags: ignoreversion; Source: win32\README.txt;             DestDir: "{app}\Source"
Flags: ignoreversion; Source: AUTHORS;                      DestDir: "{app}\Source"
Flags: ignoreversion; Source: Changes;                      DestDir: "{app}\Source"
Flags: ignoreversion; Source: CMake.README;                 DestDir: "{app}\Source"
Flags: ignoreversion; Source: CMakeLists.txt;               DestDir: "{app}\Source"
Flags: ignoreversion; Source: ConfigureChecks.cmake;        DestDir: "{app}\Source"
Flags: ignoreversion; Source: expat.pc.cmake;               DestDir: "{app}\Source"
Flags: ignoreversion; Source: expat_config.h.cmake;         DestDir: "{app}\Source"
Flags: ignoreversion; Source: run.sh.in;                    DestDir: "{app}\Source"
Flags: ignoreversion; Source: cmake\expat-config.cmake.in;  DestDir: "{app}\Source\cmake"
Flags: ignoreversion; Source: fuzz\*.c;                     DestDir: "{app}\Source\fuzz"
Flags: ignoreversion; Source: lib\*.c;                      DestDir: "{app}\Source\lib"
Flags: ignoreversion; Source: lib\*.h;                      DestDir: "{app}\Source\lib"
Flags: ignoreversion; Source: lib\*.def.cmake;              DestDir: "{app}\Source\lib"
Flags: ignoreversion; Source: examples\*.c;                 DestDir: "{app}\Source\examples"
Flags: ignoreversion; Source: tests\*.c;                    DestDir: "{app}\Source\tests"
Flags: ignoreversion; Source: tests\*.cpp;                  DestDir: "{app}\Source\tests"
Flags: ignoreversion; Source: tests\*.h;                    DestDir: "{app}\Source\tests"
Flags: ignoreversion; Source: tests\README.md;              DestDir: "{app}\Source\tests"
Flags: ignoreversion; Source: tests\benchmark\*.c;          DestDir: "{app}\Source\tests\benchmark"
Flags: ignoreversion; Source: tests\benchmark\README.txt;   DestDir: "{app}\Source\tests\benchmark"
Flags: ignoreversion; Source: xmlwf\*.c*;                   DestDir: "{app}\Source\xmlwf"
Flags: ignoreversion; Source: xmlwf\*.h;                    DestDir: "{app}\Source\xmlwf"

[Messages]
WelcomeLabel1=Welcome to the Expat XML Parser Setup Wizard
WelcomeLabel2=This will install [name/ver] on your computer.%n%nExpat is an XML parser with a C-language API, and is primarily made available to allow developers to build applications which use XML using a portable API and fast implementation.%n%nIt is strongly recommended that you close all other applications you have running before continuing. This will help prevent any conflicts during the installation process.
