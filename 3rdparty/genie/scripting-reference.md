# Scripting Reference

This section describes the functions and variables available to Premake
scripts.

Premake is built on Lua, so everything in the [Lua Reference Manual][1] applies
to a Premake script. Premake adds the ability to define solutions, projects,
and configurations, as well as functions useful for build configuration tasks.

You may also want to check out [LuaForge][2] for a wide assortment of Lua
add-on modules.

The `_ACTION` global variable stores the name of the action to be performed on
this execution run. As an example, if this command line was used to launch
Premake...


	$ premake4 vs2005


...then `_ACTION` will be set to "vs2005". If there is no action (for instance,
if the command was `premake4 /help`) this variable will be nil.

The `_ARGS` global variable stores any arguments to the current action. As an
example, if this command line was used to launch Premake...


	$ premake4 vs2005 alpha beta


...then `_ARGS[1]` will be set to "alpha" and `_ARGS[2]` to "beta". If there
are no arguments this array will be empty.

The **_OPTIONS** global variable lists the current set of command line options
and their values, if any. For more information, see [Command Line
Arguments][3].

The `_PREMAKE_COMMAND` global variable stores the full path to the Premake
executable.

Introduced in Premake 4.4.

The `_GENIE_VERSION` global variable stores the current GENie version, i.e.
62.

The `_GENIE_VERSION_STR` global variable stores the current GENie version with
git commit hash.

The `_SCRIPT` global variable stores the full path of the currently executing
script.

The `_WORKING_DIR` global variable stores the current working directory at the
time Premake was launched.

Introduced in Premake 4.4.

The **buildaction** function specifies how a file or set of files should be
treated during the compilation process. It is usually paired with a
configuration filter to select a file set. If no build action is specified for
a file a default action will be used, based on the file's extension.

Build actions are currently only supported for .NET projects, and not for C or
C++.

**Updated information is [available on the new Premake wiki][4]**.

### Applies To

Solutions, projects, and configurations.

### Parameters

_action_ is one of:

Compile : Treat the file as source code; compile and link it.

Embed : Embed the file into the target binary as a resource.

Copy : Copy the file to the target directory.

None : Do nothing with this file.

### Examples

Embed all PNG image files into the target binary.


	configuration "**.png" buildaction "Embed"

The **buildoptions** function passes arguments directly to the compiler command
line without translation.


	buildoptions { "options" }

If a project includes multiple calls to **buildoptions** the lists are
concatenated, in the order in which they appear in the script.

### Applies To

Solutions, projects, and configurations.

### Parameters

_options_ is a list of compiler flags and options, specific to a particular
compiler.

### Examples

Use `pkg-config` style configuration when building on Linux with GCC. Build
options are always compiler specific and should be targeted to a particular
toolset.


	configuration { "linux", "gmake" } buildoptions { "`wx-config --cxxflags`",
	"-ansi", "-pedantic" }

The **configuration** function limits the subsequent build settings to a
particular environment.


	configuration { "keywords" }

The **configuration** functions acts as a filter. Any settings that appear
after this function in the script will be applied only in those environments
that match all of the listed keywords. See below for some usage examples.

### Parameters

_keywords_ is a list of identifiers (see below). If all of these identifiers
are present in the current runtime environment, then the settings following the
**configuration** call will be applied. If any of the identifiers is not the
current environment the settings will be ignored.

The following table lists the available sources for keywords. Keywords are not
case-sensitive.

Configuration names : Any of the configuration names supplied to the
[configurations][5] function.

Action names : Any action name, such as **vs2005** or **gmake**. See the [Quick
Start][6] for a complete list.

Operating system names : Any of the operating system identifiers, such as
**windows** or **macosx**, as returned by [os.get][7].

Platform names : Any of the platform identifiers, such as **ps3** or
**xbox360**. See [platforms][8] for a complete list.

Command-line options : Any of the available command-line options or the option
values, whether built-in or custom to the project.

File names : Although currently very limited, some settings can be applied to
specific files.

In addition to the terms listed above, you may use the ***** and ******
wildcards to match more than one term or file. You may also use the modifiers
**not** and **or** to build more complex conditions. See the examples below for
more information.

### Return Value

The function returns the current configuration object; see [The Configuration
Block][9] below for more information on the structure of this object.

### Examples

Define a new symbol which applies only to debug builds; assumes a configuration
named "Debug" was defined as part of the solution.


	configuration "Debug" defines { "_DEBUG" }

Define a symbol only when targeting Visual Studio 2005.


	configuration "vs2005" defines { "VISUAL_STUDIO_2005" }

Wildcards can be used to match multiple terms. Define a symbol for all versions
of Visual Studio.


	configuration "vs*" defines { "VISUAL_STUDIO_2005" }

Although support is currently quite limited (only [buildaction][10] works so
far), you may also apply settings to a particular file or set of files. This
example sets the build action for all PNG image files.


	configuration "*.png" buildaction "Embed"

In the case of files you may also use the ****** wildcard, which will recurse
into subdirectories.


	configuration "**.png" buildaction "Embed"

If multiple keywords are specified, they will be treated as a logical AND. All
terms must be present for the block to be applied. This example will apply the
symbol only for debug builds on Mac OS X.


	configuration { "debug", "macosx" } defines { "DEBUG_MACOSX" }

Multiple terms must use Lua's curly bracket list syntax.

You can use the **or** modifier to match against multiple, specific terms.


	configuration "linux or macosx" defines { "LINUX_OR_MACOSX" }

You can also use **not** to apply the settings to all environments where the
identifier is not set.


	configuration "not windows" defines { "NOT_WINDOWS" }

Finally, you can reset the configuration filter and remove all active keywords
by passing the function an empty table.

### The Configuration Block

Each call to **configuration** function creates a new configuration block
object. Unless you really know what you are doing, you should treat this object
as read-only and use the Premake API to make any changes. The configuration
block object contains the following values:

buildaction : A build action.

buildoptions : A list of compiler options.

defines : A list of compiler symbols.

excludes : A list of excluded files.

files : A list of files.

flags : A list of build flags.

implibdir : The import library directory.

implibextension : The import library file extension.

implibname : The import library base file name.

implibprefix : The import library file name prefix.

implibsuffix : The import library file name suffix.

includedirs : A list of include file search directories.

keywords : A list of keywords associated with the block.

kind : The target kind.

libdirs : A list of library search directories.

linkoptions : A list of linker options.

links : A list of libraries or assemblies to link against.

objdir : The objects and intermediate files directory.

pchheader : The target file name for precompiled header support.

pchsource : The target source file name for precompiled header support.

prebuildcommands : A list of pre-build commands.

prelinkcommands : A list of pre-link commands.

postbuildcommands : A list of post-build commands.

resdefines : A list of symbols for the resource compiler.

resincludedirs : A list of include file search paths for the resource compiler.

resoptions : A list of resource compiler options.

startproject : The default startup project (for VS2005+)

targetdir : The target output directory.

targetextension : The target file extension.

targetname : The target base file name.

targetprefix : The target file name prefix.

targetsuffix : The target file name suffix.

terms : The filter terms passed to the configuration function to create the
block (i.e. "Debug").

The **configurations** function specifies the set of build configurations, such
as "Debug" and "Release", for a solution.


	configurations { "names" }

A configuration encapsulates a collection of build settings, allowing the
developer to easily switch between them. "Debug" and "Release" are the most
common configurations, the former providing debugging information, and the
latter providing optimizations.

The list of configurations must be specified before any projects are defined,
and once a project has been defined the configuration list may no longer be
changed.

**Updated information is [available on the new Premake wiki][11].**

### Applies To

Solutions only.

### Parameters

_names_ is a list of configuration names. Spaces are allowed, but may make
using certain Premake features, such as a command-line configuration selection,
more difficult.

### Return Value

The function returns the current list of configuration names for the active
solution.

### Examples

Specify debug and release configurations for a solution.


	solution "MySolution" configurations { "Debug", "Release" }

Add additional configurations for a dynamic link library version.


	configurations { "Debug", "Release", "DebugDLL", "ReleaseDLL" }

You can retrieve the current list of configurations by calling the function
with no parameters.


	local cfgs = configurations()

The **debugargs** function specifies a list of arguments to pass to the
application when run under the debugger.

Note that this settings is not implemented for Xcode 3, which requires a
per-user configuration file in order to make it work.

In Visual Studio, this file can be overridden by a per-user configuration file
(such as **ProjectName.vcproj.MYDOMAIN-MYUSERNAME.user**). Removing this file
(which is done by Premake's clean action) will restore the default settings.

Added in Premake 4.4.

### Applies To

Solutions, projects, and configurations.

### Parameters

_args_ is a Lua list of arguments to provide to the executable while debugging.

### Examples


	configuration "Debug" debugargs { "--append", "somefile.txt" }

The **debugdir** function sets the working directory for the integrated
debugger.

Note that this settings is not implemented for Xcode 3, which requires a
per-user configuration file in order to make it work.

In Visual Studio, this file can be overridden by a per-user configuration file
(such as **ProjectName.vcproj.MYDOMAIN-MYUSERNAME.user**). Removing this file
(which is done by Premake's clean action) will restore the default settings.

Added in Premake 4.4.

### Applies To

Solutions, projects, and configurations.

### Parameters

_path_ is the path to the working directory, relative to the currently
executing script file.

### Examples


	configuration "Debug" debugdir "bin/debug"

The **defines** function adds preprocessor or compiler symbols to a project.

If a project includes multiple calls to **defines** the lists are concatenated,
in the order in which they appear in the script.

### Applies To

Solutions, projects, and configurations.

### Parameters

_symbols_ specifies a list of symbols to be defined.

### Examples

Define two new symbols in the current project.


	defines { "DEBUG", "TRACE" }

Symbols may also assign values.


	defines { "CALLSPEC=__dllexport" }

The **deploymentoptions** function passes arguments directly to the deployment
tool command line without translation.


	deploymentoptions { "options" }

If a project includes multiple calls to **deploymentoptions** the lists are
concatenated, in the order in which they appear in the script.

Deployment options are currently only supported for Xbox 360 targets.

### Applies To

Solutions, projects, and configurations.

### Parameters

_options_ is a list of deployment tools flags and options.

The **excludes** function removes files, added with the [files][12] function,
from a project.

If a project includes multiple calls to **excludes** the lists are
concatenated.

### Applies To

Files may be set on the solution, project, and configuration level. However,
only project-level file lists are currently supported.

### Parameters

_file_list_ specifies one or more file patterns, separated by commas. File
paths should be specified relative to the location of the script file. File
patterns may contain the ***** wildcard to match against files in the current
directory, or the ****** wildcard to perform a recursive match.

### Examples

Add all C files in a directory, and then exclude a specific file.


	files { "*.c" } excludes { "a_file.c" }

Add an entire directory tree of C files, and then exclude one particular
directory.


	files { "**.c" } excludes { "tests/*.c" }

The **files** function adds files to a project.

If a project includes multiple calls to **files** the lists are concatenated,
in the order in which they appear in the script.

### Applies To

Files may be set on the solution, project, and configuration level. However,
only project-level file lists are currently supported.

### Parameters

_file_list_ specifies one or more file patterns, separated by commas. File
paths should be specified relative to the location of the script file. File
patterns may contain the ***** wildcard to match against files in the current
directory, or the ****** wildcard to perform a recursive match.

If a wildcard matches more files than you would like, you may filter the list
using the [excludes][13] function.

### Examples

Add two files to the current project.


	files { "hello.cpp", "goodbye.cpp" }

Add all C++ files from the **src/** directory to the project.

Add all C++ files from the **src/** directory, and any subdirectories.

The **flags** function specifies build flags to modify the compiling or linking
process.

If a project includes multiple calls to **flags** the lists are concatenated,
in the order in which they appear in the script.

**Updated information is [available on the new Premake wiki][14]**.

### Applies To

Solutions, projects, and configurations.

### Parameters

_flags_list_ is a list of string flag names; see below for a list of valid
flags. The flag values are **not** case-sensitive. Flags that are not supported
by a particular platform or toolset are ignored.

EnableSSE, EnableSSE2 : Use the SSE instruction sets for floating point math.

EnableMinimalRebuild : Enable Visual Studio's [minimal rebuild feature][15]. This will also disable multiprocessor compilation.

ExtraWarnings : Sets the compiler's maximum warning level.

FatalWarnings : Treat warnings as errors.

FloatFast : Enable floating point optimizations at the expense of accuracy.

FloatStrict : Improve floating point consistency at the expense of performance.

Managed : Enable Managed C++ (.NET).

MFC : Enable support for Microsoft Foundation Classes.

NativeWChar, NoNativeWChar : Enable or disable support for the **wchar** data
type. If no flag is specified, the toolset default will be used.

No64BitChecks : Disable 64-bit portability warnings.

NoEditAndContinue : Disable support for Visual Studio's Edit-and-Continue
feature.

NoExceptions : Disable C++ exception support.

NoFramePointer : Disable the generation of stack frame pointers.

NoIncrementalLink : Disable support for Visual Studio's incremental linking
feature.

NoImportLib : Prevent the generation of an import library for a Windows DLL.

NoManifest : Prevent the generation of a manifest for Windows executables and
shared libraries.

NoMultiProcessorCompilation : Disable multiprocessor compilation in Visual Studio. 

NoPCH : Disable precompiled header support. If not specified, the toolset
default behavior will be used. Also see [pchheader][16] and [pchsource][17].

NoRTTI : Disable C++ runtime type information.

Optimize : Perform a balanced set of optimizations.

OptimizeSize : Optimize for the smallest file size.

OptimizeSpeed : Optimize for the best performance.

SEH : Enable [structured exception handling][18].

StaticRuntime : Perform a static link against the standard runtime libraries.

Symbols : Generate debugging information.

Unicode : Enable Unicode strings. If not specified, the default toolset
behavior is used.

Unsafe : Enable the use of unsafe code in .NET applications.

UnsignedChar : Force char type to be unsigned.

WinMain : Use **WinMain()** as the program entry point for Windows
applications, rather than the default **main()**.

If the existing flags are not sufficient, you can also pass tool-specific
arguments directly to the compiler or linker using [buildoptions][19] and
[linkoptions][20].

### Examples

A common configuration: enable debugging symbols in the Debug configuration,
and optimize the Release configuration.


	configuration "Debug" flags { "Symbols" } &nbsp; configuration "Release"
	flags { "Optimize" }

You can specify multiple flags.


	flags { "Symbols", "ExtraWarnings", "FatalWarnings" }

The **framework** function selects a .NET framework version.

This value currently is only applied to Visual Studio 2005 or later, and GNU
makefiles using Mono. If no framework is specified the toolset default is used.

### Applies To

Solutions and projects.

### Parameters

_version_ is one of:

### Examples

Use the .NET 3.0 Framework.

The **iif** function implements an immediate "if" clause, returning one of two
possible values.


	result = iif(condition, trueval, falseval)

### Parameters

_condition_ is the logical condition to test. _trueval_ is the value to return
if the condition evaluates to true, _falseval_ if the condition evaluates
false.

### Return Value

_trueval_ is the condition evaluates true, _falseval_ otherwise.

### Examples


	result = iif(os.is("windows"), "is windows", "is not windows")

Note that all expressions are evaluated before the condition is checked; the
following expression can not be implemented with an immediate if because it may
try to concatenate a string value.


	result = iif(x ~= nil, "x is " .. x, "x is nil")

The **imageoptions** function passes arguments directly to the image tool
command line without translation.


	imageoptions { "options" }

If a project includes multiple calls to **imageoptions** the lists are
concatenated, in the order in which they appear in the script.

Image options are currently only supported for Xbox 360 targets.

### Applies To

Solutions, projects, and configurations.

### Parameters

_options_ is a list of image tools flags and options.

The **imagepath** function sets the file name of the deployment image produced
by the build.

This value is currently only used by the Xbox 360.

### Applies To

Solutions, projects, and configurations.

### Parameters

_path_ is the full path for the image file, relative to the currently executing
script file.

### See Also

[imageoptions][21]

The **implibdir** function specifies the import library output directory.
Import libraries are generated for Windows DLL projects.

By default, the generated project files will place the import library in the
same directory as the compiled binary. The **implibdir** function allows you to
change this location.

### Applies To

Solutions, projects, and configurations.

### Parameters

_path_ is the output directory for the library, relative to the currently
executing script file.

### See Also

[implibname][22] [implibextension][23] [implibprefix][24] [implibsuffix][25]

### Examples

The **implibextension** function specifies the import library file extension.
Import libraries are generated for Windows DLL projects.

By default, the toolset static library file extension will be used (**.lib**
with Windows tools, **.a** with GNU tools). The **implibextension** function
allows you to change this default.

### Applies To

Solutions, projects, and configurations.

### Parameters

_ext_ is the new file extension, including the leading dot.

### See Also

[implibname][22] [implibdir][26] [implibprefix][24] [implibsuffix][25]

### Examples

The **implibname** function specifies the import library base file name. Import
libraries are generated for Windows DLL projects.

By default, the [target name][27] will be used as the import library file name.
The **implibname** function allows you to change this default.

### Applies To

Solutions, projects, and configurations.

### Parameters

_name_ is the new base file name.

### See Also

[implibdir][26] [implibextension][23] [implibprefix][24] [implibsuffix][25]

### Examples

The **implibprefix** function specifies the import library file name prefix.
Import libraries are generated for Windows DLL projects.

By default, the system naming convention will be used: no prefix on Windows, a
prefix of "lib" (as in **libMyProject.a**) on other systems. The
**implibprefix** function allows you to change this default.

### Applies To

Solutions, projects, and configurations.

### Parameters

_prefix_ is the new file name prefix.

### See Also

[implibname][22] [implibdir][26] [implibextension][23] [implibsuffix][25]

### Examples

The prefix may also be set to an empty string for no prefix.

The **implibsuffix** function specifies a file name suffix for the import
library base file name. Import libraries are generated for Windows DLL
projects.

### Applies To

Solutions, projects, and configurations.

### Parameters

_suffix_ is the new filename suffix.

### See Also

[implibname][22] [implibdir][26] [implibprefix][24] [implibextension][23]

### Examples


	-- Add "-d" to debug versions of files configuration "Debug" implibsuffix
	"-d"

The **include** function looks for a file named **premake4.lua** in a specified
directory, and includes it in the current script.

This is equivalent to:


	dofile "directory/premake4.lua"

This allows you to specify each project in its own **premake4.lua** file, and
then easily include them into a solution, or multiple solutions.

### Parameters

_directory_ is the path to the included directory, relative to the currently
executing script file. The directory must contain a **premake4.lua** file, or
an error will occur. If you wish to call a file other than **premake4.lua**,
use the [dofile][28] function instead.

### Return Value

Any values returned by the included script are passed through to the caller.

### Examples


	-- runs "src/MyApplication/premake4.lua" include "src/MyApplication" &nbsp;
	-- runs "src/MyLibrary/premake4.lua" include "src/MyLibrary"

The **includedirs** function specifies the include file search paths.

If a project includes multiple calls to **includedirs** the lists are
concatenated, in the order in which they appear in the script.

### Applies To

Solutions, projects, and configurations.

### Parameters

_paths_ specifies a list of include file search directories. Paths should be
specified relative to the currently running script file.

### Examples

Define two include file search paths.


	includedirs { "../lua/include", "../zlib" }

You can also use wildcards to match multiple directories. The ***** will match
against a single directory, ****** will recurse into subdirectories as well.


	includedirs { "../includes/**" }

The **kind** function sets the kind of binary object being created by the
project, such as a console or windowed application, or a shared or static
library.

**Updated information is [available on the new Premake wiki][29]**.

### Applies To

Solutions, projects, and configurations.

### Parameters

_kind_ is the project kind identifier, and must be one of:

ConsoleApp : A console, or command-line, executable.

WindowedApp : An application that runs in a desktop window. This distinction
does not apply on Linux, but is important on Windows and Mac OS X.

SharedLib : A shared library, or DLL.

StaticLib : A static library.

### Examples

Set the project to generate a console executable.

Set the project to generate a shared library (DLL).

You can also set different kinds for each configuration. **This is not
supported by Xcode.**


	solution "MySolution" configurations { "DebugLib", "ReleaseLib",
	"DebugDLL", "ReleaseDLL" } &nbsp; project "MyProject" configuration "*Lib"
	kind "StaticLib" &nbsp; configuration "*DLL" kind "SharedLib"

The **language** function sets the programming language used by a project.

Premake currently supports **C**, **C++**, and **C#**. Not all languages are
supported by all of the generators; for instance, SharpDevelop does not
(currently) support C or C++ development, and Code::Blocks does not support the
.NET languages (C#, Managed C++).

### Applies To

Solutions and projects.

### Parameters

_lang_ is the language identifier. This is a string value, set to one of **C**,
**C++**, or **C#**. The value is not case sensitive.

### Examples

Set the project language to C++.

Set the project language to C#

The **libdirs** function specifies the library search paths.

Library search directories are not well supported by the .NET tools. Visual
Studio will change relative paths to absolute, making it difficult to share the
generated project. MonoDevelop and SharpDevelop do not support search
directories at all, using only the GAC. In general, it is better to include the
full (relative) path to the assembly in [links][30] instead. C/C++ projects do
not have this limitation.

If a project includes multiple calls to **libdirs** the lists are concatenated,
in the order in which they appear in the script.

### Applies To

Solutions, projects, and configurations.

### Parameters

_paths_ specifies a list of library search directories. Paths should be
specified relative to the currently running script file.

### Examples

Define two library file search paths.


	libdirs { "../lua/libs", "../zlib" }

You can also use wildcards to match multiple directories. The ***** will match
against a single directory, ****** will recurse into subdirectories as well.

The **linkoptions** function passes arguments directly to the linker command
line without translation.


	linkoptions { "options" }

If a project includes multiple calls to **linkoptions** the lists are
concatenated, in the order in which they appear in the script.

### Applies To

Solutions, projects, and configurations.

### Parameters

_options_ is a list of linker flags and options, specific to a particular
linker.

### Examples

Use `pkg-config` style configuration when building on Linux with GCC. Build
options are always linker specific and should be targeted to a particular
toolset.


	configuration { "linux", "gmake" } linkoptions { "`wx-config --libs`" }

The **links** function specifies a list of libraries and projects to link
against.

If a project includes multiple calls to **links** the lists are concatenated,
in the order in which they appear in the script.

### Applies To

Solutions, projects, and configurations.

### Parameters

_references_ is a list of library and project names.

When linking against another project in the same solution, specify the project
name here, rather than the library name. Premake will figure out the correct
library to link against for the current configuration, and will also create a
dependency between the projects to ensure a proper build order.

When linking against system libraries, do not include any prefix or file
extension. Premake will use the appropriate naming conventions for the current
platform.

### Examples

Link against some system libraries.


	configuration "windows" links { "user32", "gdi32" } &nbsp; configuration
	"linux" links { "m", "png" } &nbsp; configuration "macosx" -- OS X
	frameworks need the extension to be handled properly links {
	"Cocoa.framework", "png" }

In a solution with two projects, link the library into the executable. Note
that the project name is used to specify the link; Premake will automatically
figure out the correct library file name and directory and create a project
dependency.


	solution "MySolution" configurations { "Debug", "Release" } language "C++"
	&nbsp; project "MyExecutable" kind "ConsoleApp" files "**.cpp" links {
	"MyLibrary" } &nbsp; project "MyLibrary" kind "SharedLib" files "**.cpp"

You may also create links between non-library projects. In this case, Premake
will generate a build dependency (the linked project will build first), but not
an actual link. In this example, MyProject uses a build dependency to ensure
that MyTool gets built first. It then uses MyTool as part of its build process.


	solution "MySolution" configurations { "Debug", "Release" } language "C++"
	&nbsp; project "MyProject" kind "ConsoleApp" files "**.cpp" links {
	"MyTool" } prebuildcommands { "MyTool --dosomething" } &nbsp; project
	"MyTool" kind "ConsoleApp" files "**.cpp"

The **location** function sets the destination directory for a generated
solution or project file.

By default, solution and project files are generated into the same directory as
the script that defines them. The **location** function allows you to change
this location.

Note that unlike other values, **location** does not automatically propagate to
the contained projects. Projects will use their default location unless
explicitly overridden.

### Applies To

Solutions and projects.

### Parameters

_path_ is the directory where the generated files should be stored, specified
relative to the currently executing script file.

### Examples

Set the destination directory for a solution. Setting the location for a
project works the same way.


	solution "MySolution" location "../build"

If you plan to build with multiple tools from the same source tree you might
want to split up the project files by toolset. The [_ACTION][31] global
variable contains the current toolset identifier, as specified on the command
line. Note that Lua syntax requires parenthesis around the function parameters
in this case.


	location ("../build/" .. _ACTION)

The **newaction** function registers a new command-line action argument. For
more information, see [Command Line Arguments][3].

### Parameters

_description_ is a table describing the new action. It may contain the
following fields:

trigger : The string identifier of the action; what the user would type on the
command line.

description : A short description of the action, to be displayed in the help
text.

execute : A function to be executed when the action is fired.

### See Also

[Command Line Arguments][3]

### Examples

Register a new action to install the software project.


	newaction { trigger     = "install", description = "Install the software",
	execute     = function () os.copyfile("bin/debug/myprogram",
	"/usr/local/bin/myprogram") end }

The **newoption** function registers a new command-line option argument. For
more information, see [Command Line Arguments][3].

### Parameters

_description_ is a table describing the new option. It may contain the
following fields:

trigger : The string identifier of the option; what the user would type on the
command line.

description : A short description of the option, to be displayed in the help
text.

value : Optional. If the option needs a value, provides a hint to the user what
type of data is expected.

allowed : Optional. A list of key-value pairs listing the allowed values for
the option.

### See Also

[Command Line Arguments][3]

### Examples

Register a new option to select a rendering API for a 3D application.


	newoption { trigger     = "gfxapi", value       = "API", description =
	"Choose a particular 3D API for rendering", allowed = { { "opengl",
	"OpenGL" }, { "direct3d",  "Direct3D (Windows only)" }, { "software",
	"Software Renderer" } } }

The **objdir** function sets object and intermediate file directory for a
project.

By default, object and intermediate files are stored in a directory named "obj"
in the same directory as the project. The **objdir** function allows you to
change this location.

**Updated information is [available on the new Premake wiki][32]**.

### Applies To

Solutions, projects, and configurations.

### Parameters

_path_ is the directory where the object and intermediate files should be
stored, specified relative to the currently executing script file.

### Examples

Set an object directory for a project.


	project "MyProject" objdir "objects"

Set object directories per configuration.


	configuration "Debug" objdir "../obj_debug" &nbsp; configuration "Release"
	objdir "../obj_release"

The **os.chdir** function changes the current working directory.

### Parameters

_path_ is the file system path to the new working directory.

### Return Value

**True** if successful, otherwise **nil** and an error message.

The **os.copyfile** function copies a file from one location to another.


	os.copyfile("source", "destination")

### Parameters

_Source_ is the file system path to the file to be copied. _Destination_ is the
path to the copy location.

### Return Value

**True** if successful, otherwise **nil** and an error message.

The **os.findlib** function scans the well-known system locations looking for a
library file.


	p = os.findlib("libname")

### Parameters

_libname_ is name of the library to locate. It may be specified with
(libX11.so) or without (X11) system-specific decorations.

### Return Value

The path containing the library file, if found. Otherwise, nil.

The **os.get** function identifies the currently targeted operating system.

### Parameters

None.

### Return Value

An operating system identifier, one of **bsd**, **linux**, **macosx**,
**solaris**, or **windows**.

Note that this function returns the OS being targeted, which is not necessarily
the same as the OS on which Premake is being run. If you are running on Mac OS
X and generating Visual Studio project files, the identifier is "Windows",
since that is the OS being targeted by the Visual Studio action.

### Example


	if os.get() == "windows" then -- do something Windows-specific end

The **os.getcwd** function gets the current working directory.

### Parameters

None.

### Return Value

The current working directory.

Retrieve version information for the host operating system.

Introduced in **Premake 4.4**, this function has not yet been implemented for
all platforms. On platforms where this has not been implemented, it will return
zero for all version numbers, and the platform name as the description.

### Return Value

A table, containing the following key-value pairs:

majorversion The major version number

minorversion The minor version number

revision The bug fix release or service pack number

description A human-readable description of the OS version

### Examples


	local ver = os.getversion() print(string.format(" %d.%d.%d (%s)",
	ver.majorversion, ver.minorversion, ver.revision, ver.description)) &nbsp;
	-- On Windows XP: "5.1.3 (Windows XP)" -- On OS X,: "10.6.6 (Mac OS X Snow
	Leopard)"

The **os.is** function checks the current operating system identifier against a
particular value.

### Parameters

_id_ is one of the operating system identifiers **bsd**, **linux**, **macosx**,
**solaris**, or **windows**.

Note that this function tests against the OS being targeted, which is not
necessarily the same as the OS on which Premake is being run. If you are
running on Mac OS X and generating Visual Studio project files, the identifier
is "Windows", since that is the OS being targeted by the Visual Studio action.

### Return Value

True if the supplied ID matches the current operating system identifier, false
otherwise.

The **os.is64bit** function determines if the host is using a 64-bit processor.

Introduced in **Premake 4.4**.

### Return Value

**True** if the host system has a 64-bit processor, **false** otherwise.

### Examples


	if os.is64bit() then print("This is a 64-bit system") else print("This is
	NOT a 64-bit system") end

The **os.isdir** function checks for the existence of directory.

### Parameters

_Path_ is the file system path to check.

### Return Value

**True** if a matching directory is found; **false** is there is no such file
system path, or if the path points to a file instead of a directory.

The **os.isfile** function checks for the existence of file.

### Parameters

_Path_ is the file system path to check.

### Return Value

**True** if a matching file is found; **false** is there is no such file system
path, or if the path points to a directory instead of a file.

The **os.matchdirs** function performs a wildcard match to locate one or more
directories.


	matches = os.matchdirs("pattern")

### Parameters

_patterm_ is the file system path to search. It may contain single
(non-recursive) or double (recursive) asterisk wildcard patterns.

### Return Value

A list of directories which match the specified pattern. May be empty.

### Example


	matches = os.matchdirs("src/*")      -- non-recursive match matches =
	os.matchdirs("src/**")     -- recursive match &nbsp; matches =
	os.matchdirs("src/test*")  -- may also match partial names

The **os.matchdirs** function performs a wildcard match to locate one or more
directories.


	matches = os.matchfiles("pattern")

### Parameters

_patterm_ is the file system path to search. It may contain single
(non-recursive) or double (recursive) asterisk wildcard patterns.

### Return Value

A list of files which match the specified pattern. May be empty.

### Example


	matches = os.matchfiles("src/*.c")   -- non-recursive match matches =
	os.matchfiles("src/**.c")  -- recursive match

The **os.mkdir** function creates a new directory.

### Parameters

_Path_ is the file system path to be created.

### Return Value

**True** if successful, otherwise **nil** and an error message.

The **os.outputof** function runs a shell command and returns the output.


	result = os.outputof("command")

### Parameters

_command_ is a shell command to run.

### Return Value

The output of the command.

### Examples


	-- Get the ID for the host processor architecture local proc =
	os.outputof("uname -p")

The **os.pathsearch** function searches a collection of paths for a particular
file.


	p = os.pathsearch("fname", "path1", ...)

### Parameters

_fname_ is the name of the file being searched. This is followed by one or more
path sets to be searched.

Path sets match the format of the PATH environment variable: a colon-delimited
list of paths. On Windows, you may use a semicolon-delimited list if drive
letters might be included.

### Return Value

The path to the directory which contains the file, if found. Otherwise, nil.

### Examples


	local p = os.pathsearch("mysystem.config", "./config:/usr/local/etc:/etc")

The **os.rmdir** function removes an existing directory as well as any files or
subdirectories it contains.

### Parameters

_path_ is the file system path to be removed.

### Return Value

**True** if successful, otherwise **nil** and an error message.

The **os.stat** function retrieves information about a file.

Added in Premake 4.4.

### Parameters

_path_ is the filesystem path to file for which to retrieve information.

### Return Value

A table of values:

mtime : Last modified timestamp

size : The file size in bytes

The **os.uuid** function returns a [Universally Unique Identifier][33].

### Parameters

None.

### Return Value

A new UUID, a string value with the format
**74CFC033-FA4D-4B1E-A871-7DC48FA36769**.

The **path.getabsolute** function converts a relative path an absolute path.


	p = path.getabsolute("path")

### Parameters

_path_ is the relative path to be converted. It does not need to actually exist
on the file system.

### Return Value

A new absolute path, calculated from the current working directory.

The **path.getbasename** function returns the base file portion of a path, with
the directory and file extension removed.


	p = path.getbasename("path")

### Parameters

_path_ is the file system path to be split.

### Return Value

The base name portion of the supplied path, with any directory and file
extension removed.

The **path.getdirectory** function returns the directory portion of a path,
with any file name removed.


	p = path.getdirectory("path")

### Parameters

_path_ is the file system path to be split.

### Return Value

The directory portion of the path, with any file name removed. If the path does
not include any directory information, the "." (single dot) current directory
is returned.

The **path.getdrive** function returns the drive letter portion of a path, if
present.


	p = path.getdrive("path")

### Parameters

_path_ is the file system path to be split.

### Return Value

The drive letter portion of the path, if one is specified. Otherwise, nil.

The **path.getextension** function returns the file extension portion of a
path.


	p = path.getextension("path")

### Parameters

_path_ is the file system path to be split.

### Return Value

The file extension portion of the path, or an empty string if no extension is
present.

The **path.getname** function returns the file name and extension, with any
directory information removed.

### Parameters

_path_ is the file system path to be split.

### Return Value

The file name and extension, with no directory information.

The **path.getrelative** function computes a relative path from one directory
to another.


	p = path.getrelative("src", "dest")

### Parameters

_src_ is the originating directory, _dest_ is the target directory. Both may be
specified as absolute or relative.

### Return Value

A relative path from _src_ to _dest_.

The **path.isabsolute** function returns true if the specified path is an
absolute path.

### Parameters

_path_ is the file system path to check.

### Return Value

**True** if the specified path is absolute, **false** otherwise.

The **path.iscfile** function returns true if the specified path represents a C
source code file, based on its file extension.

### Parameters

_path_ is the file system path to check.

### Return Value

**True** if the path matches a well-known C file extension, **false**
otherwise.

The **path.iscppfile** function returns true if the specified path represents a
C++ source code file, based on its file extension.

### Parameters

_path_ is the file system path to check.

### Return Value

**True** if the path matches a well-known C++ file extension, **false**
otherwise.

The **path.isresourcefile** function returns true if the specified path
represents a Windows resource file, based on its file extension.


	path.isresourcefile("path")

### Parameters

_path_ is the file system path to check.

### Return Value

**True** if the path matches a well-known Windows resource file extension,
**false** otherwise.

The **path.join** function joins two path portions together into a single path.


	path.join("leading", "trailing")

If _trailing_ is an absolute path, then the leading portion is ignored, and the
absolute path is returned instead (see below for examples).

### Parameters

_leading_ is the beginning portion of the path; _trailing_ is the ending
portion.

### Return Value

A merged path.

### Examples


	-- returns "MySolution/MyProject" p = path.join("MySolution", "MyProject")
	&nbsp; -- returns "/usr/bin", because the trailing path is absolute p =
	path.join("MySolution", "/usr/bin") &nbsp; -- tokens are assumed to be
	absolute; this returns "$(ProjectDir)" p = path.join("MySolution",
	"$(ProjectDir)")

The **path.rebase** function takes a relative path and makes it relative to a
different location.


	path.rebase("path", "oldbase", "newbase")

### Parameters

_path_ is the relative path to conver. _oldbase_ is the original base
directory, from with _path_ is relative. _newbase_ is the new base directory,
from where the resulting path should be relative.

### Return Value

The rebased path.

The **path.translate** function converts the file separators in a path.


	path.translate("path", "newsep")

### Parameters

_path_ is the file system path to translate. _newsep_ is the new path
separator.

If _newsep_ is nil, the native path separator for the current environment will
be used.

### Return Value

The translated path.

The **pchheader** function sets the main header file for precompiled header
support.

If no header file is set, the toolset default settings will be used.

**Updated information is [available on the new Premake wiki][34]**.

### Applies To

Projects only.

### Parameters

_file_ is the name of the header file, as it is specified in your source file
**#include** statements.

### See Also

[pchsource][17]

### Examples


	pchheader "afxwin.h" pchsource "afxwin.cpp"

The **pchsource** function sets the main source file for precompiled header
support. This is only used by Visual Studio.

If no source file is set, the toolset default settings will be used.

**Updated information is [available on the new Premake wiki][35]**.

### Applies To

Projects only.

### Parameters

_file_ is the name of the source file, specified relative to the currently
executing script file.

### See Also

[pchheader][16]

### Examples


	pchheader "afxwin.h" pchsource "afxwin.cpp"

_Platform support is a new, experimental feature which will be introduced in
**Premake 4.1**. The syntax and behavior described here might change as we sort
out the details._

The **platforms** function specifies a set of target hardware platforms for a
solution. This is an optional setting; if it is not provided the toolset's
default behavior will be used.


	platforms { "identifiers" }

Please see the [Platforms section of the user guide][36] for a lot more
information on platforms and how they are used by Premake.

**Updated information is [available on the new Premake wiki][37].**

### Applies To

Solutions only.

### Parameters

_identifiers_ is a list of hardware platform identifiers, and may include any
of the following.

Native : A general build not targeting any particular platform; uses the
default build behavior of the compiler. If your project can be built in a
generic fashion, you should include this as the first platform option.

x32 : Target a 32-bit environment.

x64 : Target a 64-bit environment

Universal : Create a Mac OS X universal binary, targeting both 32- and 64-bit
versions of x86 and PPC. Note that in order to target multiple architectures,
automated dependency generation must be turned off. You should always do a
clean build when creating a universal target. Universal builds are not
supported by Visual Studio.

Universal32 : Like Universal above, but targeting only 32-bit platforms.

Universal64 : Like Universal above, but targeting only 64-bit platforms.

PS3 : Target the Playstation 3.

Xbox360 : Target the Xbox 360 compiler and linker under Visual Studio; ignored
elsewhere.

Not all platforms are supported on all systems, unsupported platforms will be
silently ignored. Some targets require extra configuration of the build tools
on the client machine in order to support cross-compilation.

### Return Value

The function returns the current list of target platforms for the active
solution.

### Examples

Provide a generic build that will work anywhere, as well as a Mac OS X
Universal build.


	solution "MySolution" configurations { "Debug", "Release" } platforms {
	"native", "universal" }

Provide 32- and 64-bit specific build targets. No generic build is provided, so
one of these two platforms must always be used. Do this only if your software
requires knowledge of the underlying architecture at build time, otherwise
include "native" to provide a generic build.


	solution "MySolution" configurations { "Debug", "Release" } platforms {
	"x32", "x64" }

You can retrieve the current list of platforms by calling the function with no
parameters.

Once you have defined a list of platforms, you may use those identifiers to set
up [configuration filters][38] and apply platform-specific settings.


	configuration "x64" defines "IS_64BIT" &nbsp; -- You can also mix platforms
	with other configuration selectors configuration { "Debug", "x64" } defines
	"IS_64BIT_DEBUG"

The **postbuildcommands** function specifies shell commands to run after build
is finished.


	postbuildcommands { "commands" }

### Applies To

Solutions, projects, and configurations.

### Parameters

_commands_ is one or more shell commands. These commands will be passed to the
shell exactly as entered, including path separators and the like.

### See Also

[prebuildcommands][39] [prelinkcommands][40]

### Examples


	configuration "windows" postbuildcommands { "copy default.config
	bin\project.config" } &nbsp; configuration "not windows" postbuildcommands
	{ "cp default.config bin/project.config" }

The **prebuildcommands** function specifies shell commands to run before each
build.


	prebuildcommands { "commands" }

### Applies To

Solutions, projects, and configurations.

### Parameters

_commands_ is one or more shell commands. These commands will be passed to the
shell exactly as entered, including path separators and the like.

### See Also

[prelinkcommands][40] [postbuildcommands][41]

### Examples


	configuration "windows" prebuildcommands { "copy default.config
	bin\project.config" } &nbsp; configuration "not windows" prebuildcommands {
	"cp default.config bin/project.config" }

The **prelinkcommands** function specifies shell commands to run after the
source files have been compiled, but before the link step.


	prelinkcommands { "commands" }

### Applies To

Solutions, projects, and configurations.

### Parameters

_commands_ is one or more shell commands. These commands will be passed to the
shell exactly as entered, including path separators and the like.

### See Also

[prebuildcommands][39] [postbuildcommands][41]

### Examples


	configuration "windows" prelinkcommands { "copy default.config
	bin\project.config" } &nbsp; configuration "not windows" prelinkcommands {
	"cp default.config bin/project.config" }

The **printf** performs like its C counterpart, printing a formatted string.

It is equivalent to this Lua code:


	print(string.format(format, unpack(arg))

### Parameters

_format_ is a formatting string containing C **printf()** style formatting
codes. It is followed by a list of arguments to be substituted into the format
string.

### Return Value

None.

The **project** function creates a new project and makes it active.

Projects contain all of the settings necessary to build a single binary target,
and are synonymous with a Visual Studio project. These settings include the
list of source code files, the programming language used by those files,
compiler flags, include directories, and which libraries to link against.

Every project belongs to a solution.

### Parameters

_name_ is a unique name for the project. If a project with the given name
already exists, it is made active and returned. The project name will be used
as the file name of the generated solution file.

### Return Value

The function returns the active project object; see [The Project Object][42]
below for more information on the structure of this object.

### See Also

[solution][43]

### Examples

Create a new project named "MyProject". Note that a solution must exist to
contain the project. The indentation is for readability and is optional.


	solution "MySolution" configurations { "Debug", "Release" } &nbsp; project
	"MyProject"

You can retrieve the currently active project object by calling **project**
with no parameters.

You can retrieve the list of projects associated with a solution using the
**projects** field on the solution object, which may then be iterated over.


	local prjs = solution().projects for i, prj in ipairs(prjs) do
	print(prj.name) end

### The Project Object

Each project is represented in Lua as a table of key-value pairs. Unless you
really know what you are doing, you should treat this object as read-only, and
use the Premake API to make any changes.

The project object contains the following values.

basedir : The directory where the project was original defined; acts as a root
for relative paths.

blocks : A list of configuration blocks.

language : The project language, if set.

location : The output directory for the generated project file.

name : The name of the project.

solution : The solution which contains the project.

uuid : The project's unique identifier.

The **resdefines** function specifies preprocessor symbols for the resource
compiler.

If a project includes multiple calls to **resdefines** the lists are
concatenated, in the order in which they appear in the script.

### Applies To

Solutions, projects, and configurations.

### Parameters

_symbols_ specifies a list of symbols to be defined.

### Examples

Define two new symbols in the current project.


	resdefines { "DEBUG", "TRACE" }

Symbols may also assign values.


	resdefines { "CALLSPEC=__dllexport" }

The **resincludedirs** function specifies the include file search paths for the
resource compiler.


	resincludedirs { "paths" }

If a project includes multiple calls to **resincludedirs** the lists are
concatenated, in the order in which they appear in the script.

### Applies To

Solutions, projects, and configurations.

### Parameters

_paths_ specifies a list of include file search directories. Paths should be
specified relative to the currently running script file.

### Examples

Define two include file search paths.


	resincludedirs { "../lua/include", "../zlib" }

You can also use wildcards to match multiple directories. The ***** will match
against a single directory, ****** will recurse into subdirectories as well.


	resincludedirs { "../includes/**" }

The **resoptions** function passes arguments directly to the resource compiler
command line without translation.

If a project includes multiple calls to **resoptions** the lists are
concatenated, in the order in which they appear in the script.

### Applies To

Solutions, projects, configurations

### Parameters

_options_ is a list of resource compiler flags and options, specific to a
particular compiler.

### Examples

Use `pkg-config` style configuration when building on Linux with GCC. Build
options are always compiler specific and should be targeted to a particular
toolset.


	configuration { "linux", "gmake" } resoptions { "`wx-config --cxxflags`",
	"-ansi", "-pedantic" }

The **solution** function creates a new solution and makes it active.

Solutions are the top-level objects in a Premake build script, and are
synonymous with a Visual Studio solution. Each solution contains one or more
projects, which it turn contain the settings to generate a single binary
target.

### Parameters

_name_ is a unique name for the solution. If a solution with the given name
already exists, it is made active and returned. The solution name will be used
as the file name of the generated solution file.

### Return Value

The function returns the active solution object; see [The Solution Object][42]
below for more information on the structure of this object.

### Examples

Create a new solution named "MySolution".

You can retrieve the currently active solution object by calling **solution**
with no parameters.

You can the global variable **_SOLUTIONS** to list out all of the currently
defined solutions.


	for i, sln in ipairs(_SOLUTIONS) do print(sln.name) end

### The Solution Object

Each solution is represented in Lua as a table of key-value pairs. Unless you
really know what you are doing, you should treat this object as read-only, and
use the Premake API to make any changes.

The solution object contains the following values.

basedir : The directory where the project was original defined; acts as a root
for relative paths.

configurations : The list of valid configuration names.

blocks : A list of configuration blocks.

language : The solution language, if set.

location : The output directory for the generated solution file.

name : The name of the solution.

platforms : A list of target platforms.

projects : A list of projects contained by the solution.

The **string.endswith** function returns true if the given string ends with the
provided sequence.


	string.endswith("haystack", "needle")

### Parameters

_haystack_ is the string to check. _needle_ is the ending sequence to check
against.

### Return Value

**True** if _haystack_ ends with _needle_.

The **string.explode** function returns an array of strings, each of which is a
substring of _s_ formed by splitting on boundaries formed by _pattern_.


	string.explode("str", "pattern")

### Parameters

_str_ is the string to be split. _pattern_ is the separator pattern at which to
split; it may use Lua's pattern matching syntax.

### Return Value

A list of substrings.

The **string.findlast** function finds the last instance of a pattern within a
string.


	string.endswith("str", "pattern", plain)

### Parameters

_str_ is the string to be searched. _pattern_ is the pattern to search for; it
may use Lua's pattern matching syntax. If _plain_ is true, no pattern matching
will be performed (faster).

### Return Value

The matching pattern, if found, or nil if there were no matches.

The **string.startswith** function returns true if the given string starts with
the provided sequence.


	string.startswith("haystack", "needle")

### Parameters

_haystack_ is the string to check. _needle_ is the starting sequence to check
against.

### Return Value

**True** if _haystack_ starts with _needle_.

The **table.contains** function determines if an array contains a particular
value.


	table.contains(arr, value)

### Parameters

_arr_ is the array to test; _value_ is the value being tested for.

### Return Value

True if the array contains the value, false otherwise.

The **table.implode** function merges an array of items into a single,
formatted string.


	table.implode(arr, "before", "after", "between")

### Parameters

_arr_ is the array to be converted into a string. _before_ is a string to be
inserted before each item. _after_ is a string to be inserted after each item.
_between_ is a string to be inserted between items.

### Return Value

The formatted string.

The **targetdir** function sets the destination directory for the compiled
binary target.

By default, the generated project files will place their compiled output in the
same directory as the script. The **targetdir** function allows you to change
this location.

### Applies To

Solutions, projects, and configurations.

### Parameters

_path_ is the file system path to the directory where the compiled target file
should be stored. It is specified relative to the currently executing script
file.

### See Also

[targetname][27] [targetextension][44] [targetprefix][45]

### Examples

This project separates its compiled output by configuration type.


	project "MyProject" &nbsp; configuration "Debug" targetdir "bin/debug"
	&nbsp; configuration "Release" targetdir "bin/release"

The **targetextension** function specifies the file extension for the compiled
binary target.

By default, the project will use the system's normal naming conventions:
**.exe** for Windows executables, **.so** for Linux shared libraries, and so
on. The **targetextension** function allows you to change this default.

### Applies To

Solutions, projects, and configurations.

### Parameters

_ext_ is the new file extension, including the leading dot.

### See Also

[targetname][27] [targetdir][46] [targetprefix][45] [targetsuffix][47]

### Examples

The **targetname** function specifies the base file name for the compiled
binary target.

By default, the project name will be used as the file name of the compiled
binary target. A Windows executable project named "MyProject" will produce a
binary named **MyProject.exe**. The **targetname** function allows you to
change this default.

### Applies To

Solutions, projects, and configurations.

### Parameters

_name_ is the new base file name.

### See Also

[targetdir][46] [targetextension][44] [targetprefix][45] [targetsuffix][47]

### Examples

The **targetprefix** function specifies the file name prefix for the compiled
binary target.

By default, the system naming convention will be used: a "lib" prefix for POSIX
libraries (as in **libMyProject.so**), and no prefix elsewhere. The
**targetprefix** function allows you to change this default.

### Applies To

Solutions, projects, and configurations.

### Parameters

_prefix_ is the new file name prefix.

### See Also

[targetname][27] [targetdir][46] [targetextension][44] [targetsuffix][47]

### Examples

The prefix may also be set to an empty string for no prefix.

The **targetsuffix** function specifies a file name suffix for the compiled
binary target.

### Applies To

Solutions, projects, and configurations.

### Parameters

_suffix_ is the new filename suffix.

### See Also

[targetname][27] [targetdir][46] [targetprefix][45] [targetextension][44]

### Examples


	-- Add "-d" to debug versions of files configuration "Debug" targetsuffix
	"-d"

The **uuid** function sets the [Universally Unique Identifier][33] (UUID) for a
project.

UUIDs are synonymous (for Premake's purposes) with [Globally Unique
Identifiers][48] (GUID).

Premake automatically assigns a UUID to each project, which is used by the
Visual Studio generators to identify the project within a solution. This UUID
is essentially random and will change each time the project file is generated.
If you are storing the generated Visual Studio project files in a version
control system, this will create a lot of unnecessary deltas. Using the
**uuid** function, you can assign a fixed UUID to each project which never
changes, removing the randomness from the generated projects.

### Applies To

Projects only.

### Parameters

_project_uuid_ is the UUID for the current project. It should take the form
"01234567-ABCD-ABCD-ABCD-0123456789AB" (see the examples below for some real
UUID values). You can use the Visual Studio [guidgen][49] tool to create new
UUIDs, or [this website][50], or run Premake once to generate Visual Studio
files and copy the assigned UUIDs.

### Returns

The current project UUID, or **nil** if no UUID has been set.

### Examples

Set the UUID for a current project.


	uuid "BE2461B7-236F-4278-81D3-F0D476F9A4C0"

The **vpaths** function places files into groups or "virtual paths", rather
than the default behavior of mirroring the filesystem in IDE-based projects. So
you could, for instance, put all header files in a group called "Headers", no
matter where they appeared in the source tree.


	vpaths { ["group"] = "pattern(s)" }

Note that _Lua tables do not maintain any ordering between key-value pairs_, so
there is no precedence between the supplied rules. That is, you can't write a
rule that rewrites the results of an earlier rule, since there is no guarantee
in which order the rules will run.

Added in Premake 4.4.

### Applies To

Virtual paths, like files, may be set on the solution, project, and
configuration level. However, only project-level file lists are currently
supported.

### Parameters

A list of key/value pairs, specified with Lua's standard syntax, which map file
patterns to the group in which they should appear. See the examples below for a
more complete explanation.

### Examples

Place all header files into a virtual path called "Headers". Any directory
information is removed, so a path such as **src/lua/lua.h** will appear in the
IDE as **Headers/lua.h**.


	vpaths { ["Headers"] = "**.h" }

You may also specify multiple file patterns using the table syntax.


	vpaths { ["Headers"] = { "**.h", "**.hxx", "**.hpp" } }

It is also possible to include the file's path in the virtual group. Using the
same example as above, this rule will appear in the IDE as
**Headers/src/lua/lua.h**.


	vpaths { ["Headers/*"] = "**.h" }

Any directory information explicitly provided in the pattern will be removed
from the replacement. This rule will appear in the IDE as
**Headers/lua/lua.h**.


	vpaths { ["Headers/*"] = "src/**.h" }

You can also use virtual paths to remove extra directories from the IDE. For
instance, this rule will cause the previous example to appear as **lua/lua.h**,
removing the **src** part of the path from _all_ files.

And of course, you can specify more than one rule at a time.


	vpaths { ["Headers"] = "**.h", ["Sources/*"] = {"**.c", "**.cpp"}, ["Docs"]
	= "**.txt" }

[1]: http://www.lua.org/manual/5.1/
[2]: http://luaforge.net/ 
[3]: http://industriousone.com/command-line-arguments
[4]: https://bitbucket.org/premake/premake-stable/wiki/buildaction
[5]: http://industriousone.com/configurations
[6]: http://industriousone.com/premake/quick-start
[7]: http://industriousone.com/osget
[8]: http://industriousone.com/platforms-0
[9]: http://industriousone.com#block
[10]: http://industriousone.com/buildaction
[11]: https://bitbucket.org/premake/premake-dev/wiki/configurations
[12]: http://industriousone.com/files
[13]: http://industriousone.com/excludes
[14]: https://bitbucket.org/premake/premake-stable/wiki/flags
[15]: http://msdn.microsoft.com/en-us/library/kfz8ad09(VS.80).aspx
[16]: http://industriousone.com/pchheader
[17]: http://industriousone.com/pchsource
[18]: http://www.microsoft.com/msj/0197/exception/exception.aspx
[19]: http://industriousone.com/buildoptions
[20]: http://industriousone.com/linkoptions
[21]: http://industriousone.com/imageoptions
[22]: http://industriousone.com/implibname
[23]: http://industriousone.com/implibextension
[24]: http://industriousone.com/implibprefix
[25]: http://industriousone.com/implibsuffix
[26]: http://industriousone.com/implibdir
[27]: http://industriousone.com/targetname
[28]: http://www.lua.org/manual/5.1/manual.html#pdf-dofile
[29]: https://bitbucket.org/premake/premake-stable/wiki/kind
[30]: http://industriousone.com/links
[31]: http://industriousone.com/action
[32]: https://bitbucket.org/premake/premake-dev/wiki/objdir
[33]: http://en.wikipedia.org/wiki/UUID
[34]: https://bitbucket.org/premake/premake-stable/wiki/pchheader
[35]: https://bitbucket.org/premake/premake-stable/wiki/pchsource
[36]: http://industriousone.com/platforms
[37]: https://bitbucket.org/premake/premake-dev/wiki/platforms
[38]: http://industriousone.com/configuration
[39]: http://industriousone.com/prebuildcommands
[40]: http://industriousone.com/prelinkcommands
[41]: http://industriousone.com/postbuildcommands
[42]: http://industriousone.com#object
[43]: http://industriousone.com/solution
[44]: http://industriousone.com/targetextension
[45]: http://industriousone.com/targetprefix
[46]: http://industriousone.com/targetdir
[47]: http://industriousone.com/targetsuffix
[48]: http://en.wikipedia.org/wiki/Globally_Unique_Identifier
[49]: http://msdn2.microsoft.com/en-us/library/ms241442(VS.80).aspx
[50]: http://www.famkruithof.net/uuid/uuidgen
