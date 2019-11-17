# Scripting Reference

## Table of Contents

* Predefined variables
    * [_ACTION](#_action)
    * [_ARGS](#_args)
    * [_OPTIONS](#_options)
    * [_PREMAKE_COMMAND](#_premake_command)
    * [_PREMAKE_VERSION](#_premake_version)
    * [_SCRIPT](#_script)
    * [_WORKING_DIR](#_working_dir)
* Build script functions
    * [buildaction](#buildactionaction)
    * [buildoptions](#buildoptionsoptions)
    * [configuration](#configurationkeywords)
    * [configurations](#configurationsnames)
    * [custombuildtask](#custombuildtasktask)
    * [debugcmd](#debugcmdcmd)
    * [debugargs](#debugargsargs)
    * [debugdir](#debugdirpath)
    * [defines](#definessymbols)
    * [dependency](#dependencyfiles)
    * [deploymentoptions](#deploymentoptionsoptions)
    * [excludes](#excludesfiles)
    * [files](#filesfiles)
    * [flags](#flagsflags)
    * [framework](#frameworkversion)
    * [group](#groupname)
    * [imageoptions](#imageoptionsoptions)
    * [imagepath](#imagepathpath)
    * [implibdir](#implibdir)
    * [implibextension](#implibextensionextension)
    * [implibname](#implibnamename)
    * [implibprefix](#implibprefixprefix)
    * [implibsuffix](#implibsuffixsuffix)
    * [include](#includedirectory)
    * [includedirs](#includedirspaths)
    * [kind](#kindkind)
    * [language](#languagelang)
    * [libdirs](#libdirspaths)
    * [linkoptions](#linkoptionsoptions)
    * [links](#linksreferences)
    * [location](#locationpath)
    * [messageskip](#messageskipoptions)
    * [newaction](#newactiondescription)
    * [newoption](#newoptionsdescription)
    * [nopch](#nopch)
    * [objdir](#objdirpath)
    * [options](#optionsoptions)
    * [pchheader](#pchheaderfile)
    * [pchsource](#pchsourcefile)
    * [platforms](#platformsidentifiers)
    * [postbuildcommands](#postbuildcommandscommands)
    * [postcompiletasks](#postcompiletaskstasks)
    * [prebuildcommands](#prebuildcommandscommands)
    * [prelinkcommands](#prelinkcommandscommands)
    * [project](#projectname)
    * [removefiles](#removefilesfiles)
    * [removeflags](#removeflagsflags)
    * [removelinks](#removelinkslinks)
    * [removeplatforms](#removeplatformsplatforms)
    * [resdefines](#resdefinessymbols)
    * [resincludedirs](#resincludedirspaths)
    * [resoptions](#resoptionsoptions)
    * [solution](#solutionname)
    * [startproject](#startprojectname)
    * [systemincludedirs](#systemincludedirspaths)
    * [targetdir](#targetdirpath)
    * [targetextension](#targetextensionext)
    * [targetname](#targetnamename)
    * [targetprefix](#targetprefixprefix)
    * [targetsubdir](#targetsubdirpath)
    * [targetsuffix](#targetsuffixsuffix)
    * [userincludedirs](#userincludedirspaths)
    * [uuid](#uuidprojectuuid)
    * [vpaths](#vpathsgroup--pattern)
    * [xcodeprojectopts](#xcodeprojectoptskey--value-)
    * [xcodetargetopts](#xcodetargetoptskey--value-)
    * [xcodescriptphases](#xcodescriptphasescmd-inputpaths-)
    * [xcodecopyresources](#xcodecopyresourcestargetpath-inputfiles-)
    * [xcodecopyframeworks](#xcodecopyframeworksinputframeworks-)
    * [wholearchive](#wholearchivereferences)
* Utility functions
    * [iif](#iifcondition-trueval-falseval)
    * [os.chdir](#oschdirpath)
    * [os.copyfile](#oscopyfilesource-destination)
    * [os.findlib](#osfindliblibname)
    * [os.get](#osget)
    * [os.getcwd](#osgetcwd)
    * [os.getversion](#osgetversion)
    * [os.is](#osisid)
    * [os.is64bit](#osis64bit)
    * [os.isdir](#osisdirpath)
    * [os.isfile](#osisfilepath)
    * [os.matchdirs](#osmatchdirspattern)
    * [os.matchfiles](#osmatchfilespatterns)
    * [os.mkdir](#osmkdirpath)
    * [os.outputof](#osoutputofcommand)
    * [os.pathsearch](#ospathsearchfname-paths)
    * [os.rmdir](#osrmdirpath)
    * [os.stat](#osstatpath)
    * [os.uuid](#osuuidname)
    * [path.getabsolute](#pathgetabsolutepath)
    * [path.getbasename](#pathgetbasenamepath)
    * [path.getdirectory](#pathgetdirectorypath)
    * [path.getdrive](#pathgetdrivepath)
    * [path.getextension](#pathgetextension)
    * [path.getname](#pathgetnamepath)
    * [path.getrelative](#pathgetrelativesrc-dest)
    * [path.isabsolute](#pathisabsolutepath)
    * [path.iscfile](#pathiscfilepath)
    * [path.isSourceFile](#pathiscppfilepath)
    * [path.isresourcefile](#pathisresourcefilepath)
    * [path.join](#pathjoinleading-trailing)
    * [path.rebase](#pathrebasepath-oldbase-newbase)
    * [path.translate](#pathtranslatepath-newsep)
    * [printf](#printfformat-args)
    * [string.endswith](#stringendswithhaystack-needle)
    * [string.explode](#stringexplodestr-pattern)
    * [string.findlast](#stringfindlaststr-pattern-plain)
    * [string.startswith](#stringstartswithhaystack-needle)
    * [table.contains](#tablecontainsarray-value)
    * [table.implode](#tableimplodearray-before-after-between)
* Additional information
    * [Wildcards](#wildcards)

---

## Predefined Variables

Each of the following variables is available for use in any GENie script.

### _ACTION

Name of the action to be performed on this execution run.

`$ genie vs2005`

produces

`_ACTION: "vs2005"`

[Back to top](#table-of-contents)

---
### _ARGS

Any arguments to the current action.

`$ genie vs2015 alpha beta`

produces

`_ARGS[0]: "alpha"`
`_ARGS[1]: "beta"`

[Back to top](#table-of-contents)

---
### _OPTIONS

Current set of command line options and their values, if any.

`$ genie vs2015 --gfxapi=directx`

produces

`_OPTIONS['gfxapi']: "directx"`

**Note:** Options may be registered with [newoption](#newoption) to fully integrate them into the CLI.

[Back to top](#table-of-contents)

---
### _PREMAKE_COMMAND

Full path to the GENie (Premake) executable.

[Back to top](#table-of-contents)

---
### _PREMAKE_VERSION

GENie (Premake) version.

[Back to top](#table-of-contents)

---
### _SCRIPT

Full path to the currently executing script.

[Back to top](#table-of-contents)

---
### _WORKING_DIR

Current working directory.

[Back to top](#table-of-contents)

---

## Build script functions

### buildaction(_action_)
Specifies what action should be performed on a set of files during compilation. Usually paired with a configuration filter to select a file set. If no build action is specified for a file, a default action will be used (chosen based on the file's extension).

**Scope:** solutions, projects, configurations

**Note:** only supported for .NET projects, and not for C or C++.

#### Arguments
_action_ - the action to be performed. One of:

* "Compile" - treat the file as source code: compile and run it
* "Embed" - embed the file into the target binary as a resource
* "Copy" - copy the file to the target directory
* "None" - do nothing with this file

#### Examples
Embed all PNGs into the target binary

```lua
configuration "**.png"
    buildaction "Embed"
```

[Back to top](#table-of-contents)

---
### buildoptions({_options_...})
Passes arguments direction to the compiler command line. Multiple calls in a project will be concatenated in order.

**Scope:** solutions, projects, configurations

You may also use one of these functions to configure buildoptions for each individual file extension:

* `buildoptions_asm` for .asm files
* `buildoptions_c` for .c files
* `buildoptions_cpp` for .cpp files
* `buildoptions_objc` for .m files
* `buildoptions_objcpp` for .mm files
* `buildoptions_swift` for .swift files
* `buildoptions_vala` for .vala files

#### Arguments
_options_ - list of compiler flags

#### Examples
Add some GCC-specific options

```lua
configuration {"linux", "gmake"}
    buildoptions {"-ansi", "-pedantic"}
```

[Back to top](#table-of-contents)

---
### configuration({_keywords_...})
Limits subsequent build settings to a particular environment. Acts as a filter, only applying settings that appear after this function if the environment matches the keywords.

#### Arguments
_keywords_ - list of identifiers to compare to the current runtime environment

Possible values:

* Configuration names - configuration names passed to [configurations](#configurations)
* Action names - "vs2015", "gmake", etc.
* Operating system names - "windows", "macosx", etc.
* Platform names - "ps3", "xbox360", etc.
* Command-line options - either built-in or custom
* File names - very limited, but some settings can be applied to specific files

You may also use "*" and "**" wildcards, as well as "not" and "or".

#### Return
Current configuration object with the following fields:

* _buildaction_       - build action.
* _buildoptions_      - list of compiler options.
* _defines_           - list of compiler symbols.
* _excludes_          - list of excluded files.
* _files_             - list of files.
* _flags_             - list of build flags.
* _implibdir_         - import library directory.
* _implibextension_   - import library file extension.
* _implibname_        - import library base file name.
* _implibprefix_      - import library file name prefix.
* _implibsuffix_      - import library file name suffix.
* _includedirs_       - list of include file search directories.
* _keywords_          - list of keywords associated with the block.
* _kind_              - target kind.
* _libdirs_           - list of library search directories.
* _linkoptions_       - list of linker options.
* _links_             - list of libraries or assemblies to link against.
* _objdir_            - objects and intermediate files directory.
* _pchheader_         - target file name for precompiled header support.
* _pchsource_         - target source file name for precompiled header support.
* _prebuildcommands_  - list of pre-build commands.
* _prelinkcommands_   - list of pre-link commands.
* _postbuildcommands_ - list of post-build commands.
* _resdefines_        - list of symbols for the resource compiler.
* _resincludedirs_    - list of include file search paths for the resource compiler.
* _resoptions_        - list of resource compiler options.
* _targetdir_         - target output directory.
* _targetextension_   - target file extension.
* _targetname_        - target base file name.
* _targetprefix_      - target file name prefix.
* _targetsuffix_      - target file name suffix.
* _terms_             - filter terms passed to the configuration function to create the block (i.e. "Debug").

#### Examples
Define debug symbol for debug configurations

```lua
configuration "Debug"
    defines { "DEBUG" }
```

Define a symbol based on a wildcard

```lua
configuration "vs*"
    defines { "VISUAL_STUDIO_2005" }
```

Define a symbol based on an "or"

```lua
configuration "linux or macosx"
    defines { "LINUX_OR_MACOSX" }
```

Define a symbol based on a "not"

```lua
configuration "not windows"
    defines { "NOT_WINDOWS" }
```

Reset the configuration filter

```lua
configuration {}
```

#### Caveats

- Argument chaining:
  `configuration` can take multiple arguments, e.g.,
  ```lua
  configuration {"StaticLib", "xcode*", "osx or ios*"}
  ```
  These arguments will be combined as an `AND` clause,
  i.e. if one of the keywords does _not_ match the actual configuration terms,
  the following settings will not be applied.

- Condition evaluation:
  The arguments are **not** evaluated as Lua. They are merely regex-matched against the configuration terms.
  The implications of this are that parentheses have no effect outside of regular expression groups.
  A condition like `"not (osx or ios*)"` will not be equivalent to `{"not osx", "not ios*}"`.
  Furthermore, a condition like `"not osx or ios*"` will be evaluated as the negation of `"osx or ios*"`.

- `and` is **not** a valid keyword for configuration combinations.
  However, several keywords will be combined as an `AND` clause.

- Limits of Lua's regular expressions:
  Each passed keyword is matched against each configuration terms from the project/solution type being built
  using [Lua's regular expression mechanism](https://www.lua.org/manual/5.3/manual.html#6.4).
  This means that keyword matching is subject to the same limits as regular Lua regex matching.
  This implies that regexes like `"(osx|ios)"` do not work.

- Wildcard expansion:
  Wildcards will get expanded following the same rules as paths.
  Similarly, special characters such as `()` will get escaped (i.e. converted to `%(%)`) before being matched.
  This means that `"not (osx or ios*)"` will in fact get expanded to `"not %(osx or ios[^/]*)"` and then checked as
  `not` _result of_ `"%(osx or ios[^/]*)"`, which in turn gets broken down to `"%(osx"` and `"ios[^/]*)"`.

- `"win*"` matchings:
  Intuitively, the configuration keyword to match "Windows" ("Win32", "Win64" or "WinCE") configuration would be
  `"win*"`. However **`"win*"` also matches "WindowedApp"**. Prefer using the term `"vs*"` to check for configurations
  targeting Windows.

[Back to top](#table-of-contents)

---
### configurations({_names_...})
Defines a set of build configurations, such as "Debug" and "Release". Must be specified before any projects are defined, so can't be called after a project has been defined.

**Scope:** solutions

#### Arguments
_names_ - list of configuration names

#### Return Value
When called with no arguments - list of current configuration names

#### Examples
Specify configurations for a solution

```lua
solution "MySolution"
    configurations { "Debug", "Release" }
```

Add additional configurations

```lua
configurations{ "Debug", "Release", "DebugDLL", "ReleaseDLL" }
```

Retrieve current list of configurations

```lua
local cfgs = configurations()
```

[Back to top](#table-of-contents)

---
### custombuildtask({*input_file*, *output_file*, {*dependency*, ...}, {*command*, ...}}, ...)
Defines custom build task for specific input file, that generates output file, there can be additional dependencies, and 
for rule listed commands are executed.

**Scope:** solutions, projects, configurations

#### Arguments
*input_file* - source file that should be "compiled" with custom task
*output_file* - generated file name
*dependency* - additional dependencies, that can be used as parameters to commands
*command* - command list, special functions in commands are :
    $(<) - input file  
    $(@) - output file  
    $(1) - $(9) - additional dependencies

#### Examples

```lua
custombuildtask {
        { ROOT_DIR .. "version.txt" , GEN_DIR .. "src/version.inc",   { ROOT_DIR .. "version.py" }, {"@echo Generating version.inc file...", "python $(1) $(<) > $(@)" }},
    }
```

[Back to top](#table-of-contents)

---
### debugcmd(cmd)
Specifies a command to execute when running under the debugger instead of the build target.

**Note:** In Visual Studio, this can be overridden by a per-user config file (e.g. ProjectName.vcxproj.MYDOMAIN-MYUSERNAME.user).

**Scope:** solutions, projects, configurations

#### Arguments
_cmd_ - the command to execute when starting with the debugger

#### Examples

```lua
configuration 'TestConfig'
    debugcmd 'D:\\Apps\\Test.exe'
```

[Back to top](#table-of-contents)

---
### debugargs({_args_...})
Specifies a list of arguments to pas to the application when run under the debugger.

**Note:** In Visual Studio, this can be overridden by a per-user config file (e.g. ProjectName.vcxproj.MYDOMAIN-MYUSERNAME.user).

**Scope:** solutions, projects, configurations

#### Arguments
_args_ - list of arguments to pass to the executable while debugging

#### Examples

```lua
configuration "Debug"
    debugargs { "--append", "somefile.txt" }
```

[Back to top](#table-of-contents)

---
### debugdir(_path_)
Sets the working directory for the integrated debugger.

**Note:** In Visual Studio, this can be overridden by a per-user config file (e.g. ProjectName.vcxproj.MYDOMAIN-MYUSERNAME.user).

**Scope:** solutions, projects, configurations

#### Arguments
_path_ - path to the working directory, relative to the currently-executing script file

#### Examples

```lua
configuration "Debug"
    debugdir "bin/debug"
```

[Back to top](#table-of-contents)

---
### defines({_symbols_...})
Adds preprocessor or compiler symbols to the project. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_symbols_ - list of symbols

#### Examples
Define two new symbols

```lua
defines { "DEBUG", "TRACE" }
```

Assign a symbol value

```lua
defines { "CALLSPEC=__dllexport" }
```

[Back to top](#table-of-contents)

---
### dependency({*main_file*, *depending_of*} ...)
GMAKE specific. Adds dependency between source file and any other file.

**Scope:** solutions, projects, configurations

#### Arguments
*main_file* - name of source file that depends of other file
*depending_of* - name of dependency file

#### Examples

```lua
dependency { { ROOT_DIR .. "src/test.c", ROOT_DIR .. "verion.txt" } }
```

[Back to top](#table-of-contents)

---
### deploymentoptions({_options_...})
Passes arguments directly to the deployment tool command line. Multiple calls are concatenated.

**Note:** Currently only supported for Xbox 360 targets.

**Scope:** solutions, projects, configurations

#### Arguments
_options_ - list of arguments

[Back to top](#table-of-contents)

---
### excludes({_files_...})
Excludes files from the project. This is different from [removefiles](#removefilesfiles) in that it may keep them in the project (Visual Studio) while still excluding them from the build. Multiple calls are concatenated.

**Note:** May be set on the solution, project, or configuration, but only project-level file lists are currently supported.

**Scope:** solutions, projects, configurations

#### Arguments
_files_ - List of files to exclude. Paths should be relative to the currently-executing script file and may contain [wildcards](#wildcards).

#### Examples
Add all c files in a directory, then exclude a specific file

```lua
files { "*.c" }
excludes { "a_file.c" }
```

Add an entire directory of C files, then exclude one directory

```lua
files { "*.c" }
excludes { "tests/*.c" }
```

[Back to top](#table-of-contents)

---
### files({_files_...})
Adds files to a project. Multiple calls are concatenated.

**Note:** May be set on the solution, project, or configuration, but only project-level file lists are currently supported.

**Scope:** solutions, projects, configurations

#### Arguments
_files_ - List of files to include. Paths should be relative to the currently-executing script file and may contain [wildcards](#wildcards).

#### Examples
Add two files to the current project

```lua
files { "hello.cpp", "goodbye.cpp" }
```

Add all C++ files from the "src/" directory to the project

```lua
files { "src/*.cpp" }
```

Add all C++ files from the "src/" directory and any subdirectories

```lua
files { "src/**.cpp" }
```

[Back to top](#table-of-contents)

---
### flags({_flags_...})
Specifies build flags to modify the compiling or linking process. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_flags_ - List of flag names from list below. Names are case-insensitive and ignored if not supported on a platform.

* _C7DebugInfo_ - Enables C7 compatible debug info for MSVC builds.
* _EnableSSE, EnableSSE2, EnableAVX, EnableAVX2_ - Enable SSE/AVX instruction sets
* _ExtraWarnings_ - Sets compiler's max warning level.
* _FatalWarnings_ - Treat warnings as errors.
* _FloatFast_ - Enable floating point optimizations at the expense of accuracy.
* _FloatStrict_ - Improve floating point consistency at the expense of performance.
* _FullSymbols_ - Use together with _Symbols_ to generate full debug symbols with Visual Studio.
* _LinkSupportCircularDependencies_ - Enables the linker to iterate over provided libs in order to resolve circular dependencies (make and ninja only).
* _Managed_ - Enable Managed C++ (.NET).
* _MinimumWarnings_ - - Sets compiler's minimum warning level (Visual Studio only).
* _MFC_ - Enable support for Microsoft Foundation Classes.
* _NativeWChar, NoNativeWChar_ - Toggle support for the wchar data type.
* _No64BitChecks_ - Disable 64-bit portability warnings.
* _NoBufferSecurityCheck_ - Turns off Visual Studio 'Security Check' option. Can give up to 10% performance improvement.
* _NoEditAndContinue_ - Disable support for Visual Studio's Edit-and-Continue feature.
* _NoExceptions_ - Disable C++ exception support.
* _NoFramePointer_ - Disable the generation of stack frame pointers.
* _NoIncrementalLink_ - Disable support for Visual Studio's incremental linking feature.
* _NoImportLib_ - Prevent the generation of an import library for a Windows DLL.
* _NoManifest_ - Prevent the generation of a manifest for Windows executables and shared libraries.
* _NoMultiProcessorCompilation_ - Disables Visual Studio's and FastBuild's multiprocessor compilation.
* _NoRuntimeChecks_ - Disable Visual Studio's Basic Runtime Checks in Debug builds.
* _EnableMinimalRebuild_ - Enable Visual Studio's minimal rebuild feature.
* _NoPCH_ - Disable precompiled headers.
* _NoRTTI_ - Disable C++ runtime type information.
* _NoWinMD_ - Disables Generation of Windows Metadata.
* _NoWinRT_ - Disables Windows RunTime Extension for project.
* _ObjcARC_ - Enable automatic reference counting for Object-C and Objective-C++.
* _Optimize_ - Perform a balanced set of optimizations.
* _OptimizeSize_ - Optimize for the smallest file size.
* _OptimizeSpeed_ - Optimize for the best performance.
* _PedanticWarnings_ - Enables the pedantic warning flags.
* _SEH_ - Enable structured exception handling.
* _SingleOutputDir_ - Allow source files in the same project to have the same name.
* _StaticRuntime_ - Perform a static link against the standard runtime libraries.
* _Symbols_ - Generate debugging information.
* _Unicode_ - Enable Unicode strings. If not specified, the default toolset behavior is used.
* _Unsafe_ - Enable the use of unsafe code in .NET applications.
* _UseFullPaths_ - Enable absolute paths for `__FILE__`. 
* _UseLDResponseFile_ - Enable use of response file (aka @file) for linking lib dependencies (make only).
* _UseObjectResponseFile_ - Enable use of response file (aka @file) for linking objects (make only).
* _UnsignedChar_ - Force `char`s to be `unsigned` by default.
* _WinMain_ - Use WinMain() as the entry point for Windows applications, rather than main().

**Note:** When not set, options will default to the tool default.

Additional tool-specific arguments can be passed with [`buildoptions`](#buildoptions) or [`linkoptions`](#linkoptions)

#### Examples
Enable debugging symbols in the Debug configuration and optimize the Release configuration

```lua
configuration "Debug"
    flags { "Symbols" }

configuration "Release"
    flags { "OptimizeSpeed", "No64BitChecks" }
```

[Back to top](#table-of-contents)

---
### framework(_version_)
Specifies a .NET framework version.

**Note:** Currently only applied to Visual Studio 2005+ and GNU Makefiles using Mono.

**Scope:** solutions, projects

#### Arguments
_version_ - one of the following:

* 1.0
* 1.1
* 2.0
* 3.0
* 3.5
* 4.0

#### Examples
Use the .NET 3.0 framework

```lua
framework "3.0"
```

[Back to top](#table-of-contents)

---
### group(_name_)
Creates a solution folder for Visual Studio solutions.

**Scope:** solutions

#### Arguments
_name_ - the name of the solution folder

#### Examples

```lua
solution "MySolution"
    group "MyGroup1"
        project "Project1"
        -- ...
        project "Project2"
        -- ...
    group "MyGroup2"
        project "Project3"
        -- ...
```

[Back to top](#table-of-contents)

---
### imageoptions({_options_...})
Passes arguments directly to the image tool command line without translation. Multiple calls are concatenated.

**Scope:** solutions, project, configurations

#### Arguments
_options_ - list of image tools flags and options

[Back to top](#table-of-contents)

---
### imagepath(_path_)
Sets the file name of the deployment image produced by the build

**Scope:** solutions, projects, configurations

#### Arguments
_path_ - the full path for the image file, relative to the currently-executing script

[Back to top](#table-of-contents)

---
### implibdir(_path_)
Specifies the import library output directory. Import libraries are generated for Windows DLL projects. By default, the generated files will place the import library in the same directory as the compiled binary.

**Scope:** solutions, projects, configurations

#### Arguments
_path_ - the output directory for the library, relative to the currently-executing script file

#### Examples

```lua
implibdir "../Libraries"
```

[Back to top](#table-of-contents)

---
### implibextension(_extension_)
Specifies the import library file extension. Import libraries are generated for Windows DLL projects. By default, the toolset static library file extension will be used (`.lib` with Windows tools, `.a` with GNU tools).

**Scope:** solutions, projects, configurations

#### Arguments
_extension_ - the extension, including the leading dot

[Back to top](#table-of-contents)

---
### implibname(_name_)
Specifies the import library base file name. Import libraries are generated for Windows DLL projects. By default the target name will be used as the import library file name.

**Scope:** solutions, projects, configurations

#### Arguments
_name_ - new base file name

[Back to top](#table-of-contents)

---
### implibprefix(_prefix_)
Specifies the import library file name prefix. Import libraries are generated for Windows DLL projects. By default the system naming convention will be used (no prefix on Windows, `lib` prefix on other systems).

**Scope:** solutions, projects, configurations

#### Arguments
_prefix_ - new file name prefix

#### Examples

```lua
implibprefix "plugin"
```

The prefix may also be set to an empty string for no prefix

```lua
implibprefix ""
```

[Back to top](#table-of-contents)

---
### implibsuffix(_suffix_)
Specifies the file name suffix for the import library base file name. Import libraries are generated for Windows DLL projects.

**Scope:** solutions, projects, configurations

#### Arguments
_suffix_ - the new filename suffix

#### Examples

```lua
-- Add "-d" to debug versions of files
configuration "Debug"
    implibsuffix "-d"
```

[Back to top](#table-of-contents)

---
### include(_directory_)
Includes a file named `premake4.lua` from the specified directory. This allows you to specify each project in its own file, and easily include them into a solution.

#### Arguments
_directory_ - path to the included directory, relative to the currently-executing script file.

#### Return Value
Any values returned by the script are passed through to the caller

#### Examples

```lua
-- runs "src/MyApplication/premake4.lua"
include "src/MyApplication"

-- runs "src/MyLibrary/premake4.lua"
include "src/MyLibrary"
```

[Back to top](#table-of-contents)

---
### includedirs({_paths_...})
Specifies include file search paths. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_paths_ - list of include file search directories, relative to the currently-executing script file.

#### Examples
Define two include file search paths

```lua
includedirs { "../lua/include", "../zlib" }
```

You can also use [wildcards](#wildcards) to match multiple directories.

```lua
includedirs { "../includes/**" }
```

[Back to top](#table-of-contents)

---
### kind(_kind_)
Sets the kind of binary object being created by the project, such as a console or windowed application.

**Scope:** solutions, projects, configurations

#### Arguments
_kind_ - project kind identifier. One of:

* _ConsoleApp_ - console executable
* _WindowedApp_ - application that runs in a desktop window. Does not apply on Linux.
* _StaticLib_ - static library
* _SharedLib_ - shared library or DLL
* _Bundle_ - Xcode: Cocoa Bundle, everywhere else: alias to _SharedLib_

#### Examples

```lua
kind "ConsoleApp"
```

You can also set different kinds for each configuration. **Not supported by XCode.**

```lua
solution "MySolution"
    configurations { "DebugLib", "ReleaseLib", "DebugDLL", "ReleaseDLL" }

project "MyProject"
    configuration "*Lib"
        kind "StaticLib"

    configuration "*DLL"
        kind "SharedLib"
```

[Back to top](#table-of-contents)

---
### language(_lang_)
Sets the programming language used by a project. GENie currently supports C, C++, C# and Vala. Not all languages are supported by all of the generators. For instance, SharpDevelop does not currently support C or C++ development, and Code::Blocks does not support the .NET languages (C#, managed C++).

**Scope:** solutions, projects

#### Arguments
_lang_ - language identifier string ("C", "C++", "C#" or "Vala"). Case insensitive.

#### Examples

```lua
language "C++"
```

[Back to top](#table-of-contents)

---
### libdirs({_paths_...})
Specifies the library search paths. Library search directories are not well supported by the .NET tools. Visual Studio will change relative paths to absolute, making it difficult to share the generated project. MonoDevelop and SharpDevelop do not support search directories at all, using only the GAC. In general, it is better to include the full (relative) path to the assembly in links instead. C/C++ projects do not have this limitation.

Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_paths_ - list of library search directories, relative to the currently-executing script file

#### Examples

```lua
libdirs { "../lua/libs", "../zlib" }
```

You can also use [wildcards](#wildcards) to match multiple directories.

```lua
libdirs { "../libs/**" }
```

[Back to top](#table-of-contents)

---
### linkoptions({_options_...})
Passes arguments to the linker command line. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_options_ - list of flags and options to pass

#### Examples
Use `pkg-config`-style configuration when building on Linux with GCC.

```lua
configuration { "linux", "gmake" }
    linkoptions { "`wx-config --libs`"}
```

[Back to top](#table-of-contents)

---
### links({_references_...})
Specifies a list of libraries and projects to link against. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_references_ - list of library and project names

When linking against another project in the same solution, specify the project name here, rather than the library name. GENie will figure out the correct library to link against for the current configuration and will also create a dependency between the projects to ensure proper build order.

When linking against system libraries, do not include any prefix or file extension. GENie will use the appropriate naming conventions for the current platform.

#### Examples
Link against some system libraries

```lua
configuration "windows"
    links { "user32", "gdi32" }

configuration "linux"
    links { "m", "png" }

configuration "macosx"
--- OS X frameworks need the extension to be handled properly
    links { "Cocoa.framework", "png" }
```

In a solution with two projects, link the library into the executable. Note that the project name is used to specify the link. GENie will automatically figure out the correct library file name and directory and create a project dependency.

```lua
solution "MySolution"
    configurations { "Debug", "Release" }
    language "C++"

    project "MyExecutable"
        kind "ConsoleApp"
        files "**.cpp"
        links { "MyLibrary" }

    project "MyLibrary"
        kind "SharedLib"
        files "**.cpp"
```

You may also create links between non-library projects. In this case, GENie will generate a build dependency (the linked project will build first) but not an actual link. In this example, MyProject uses a build dependency to ensure that MyTool gets built first. It then uses MyTool as part of its build process

```lua
solution "MySolution"
    configurations { "Debug", "Release" }
    language "C++"

    project "MyProject"
        kind "ConsoleApp"
        files "**.cpp"
        links { "MyTool" }
        prebuildcommands { "MyTool --dosomething" }

    project "MyTool"
        kind "ConsoleApp"
        files "**.cpp"
```

[Back to top](#table-of-contents)

---
### location(_path_)
Sets the destination directory for a generated solution or project file. By default, project files are generated into the same directory as the script that defines them.

**Note:** Does not automatically propagate to the contained projects. Projects will use their default location unless explicitly overridden.

**Scope:** solutions, projects

#### Arguments
_path_ - directory into which files should be generated, relative to the currently-executing script file.

#### Examples

```lua
solution "MySolution"
    location "../build"
```

If you plan to build with multiple tools from the same source tree, you might want to split up the project files by toolset. The _ACTION global variable contains the current toolset identifier, as specified on the command line. Note that Lua syntax requires parentheses around the function parameters in this case.

```lua
location ("../build/" .. _ACTION)
```

[Back to top](#table-of-contents)

---
### messageskip(_options_)
Skips certain messages in ninja and Makefile generated projects.

**Scope:** solutions

#### Arguments
_options_ - one or several of "SkipCreatingMessage", "SkipBuildingMessage", "SkipCleaningMessage"

#### Examples

```lua
messageskip { "SkipCreatingMessage", "SkipBuildingMessage", "SkipCleaningMessage" }
```

[Back to top](#table-of-contents)

---
### newaction(_description_)
Registers a new command-line action argument.

#### Arguments
_description_ - a table describing the new action with the following fields:

* _trigger_ - string identifier of the action; what the user would type on the command line
* _description_ - short description of the action, to be displayed in the help text
* _execute_ - Function to be executed when the action is fired

#### Examples

```lua
newaction {
    trigger     = "install",
    description = "Install the software",
    execute     = function()
        os.copyfile("bin/debug/myprogram", "/usr/local/bin/myprogram")
    end
}
```

[Back to top](#table-of-contents)

---
### newoption(_description_)
Registers a new command-line option argument.

**Scope:** solutions, projects, configurations

#### Arguments
_description_ - a table describing the new option with the following fields:

* _trigger_ - string identifier of the option; what the user would type on the command line
* _description_ - short description of the option, to be displayed in the help text
* _value_ - (optional) provides a hint to the user as to what type of data is expected
* _allowed_ - (optional) list of key-value pairs listing the allowed values for the option

#### Examples

```lua
newoption {
    trigger     = "gfxapi",
    value       = "API",
    description = "Choose a particular 3D API for rendering",
    allowed     = {
        { "opengl",   "OpenGL" },
        { "direct3d", "Direct3D (Windows only)"},
        { "software", "Software Renderer" }
    }
}
```

[Back to top](#table-of-contents)

---
### nopch({_files_...})
Sets sources files added with the [`files`](#files) function, to not use the precompiled header. Multiple calls are concatenated.

**Note:** May be set on the solution, project, or configuration, but only project-level file lists are currently supported.

**Scope:** solutions, projects, configurations

#### Arguments
_files_ - List of files to not use the precompiled header. Paths should be relative to the currently-executing script file and may contain [wildcards](#wildcards).

#### Examples
Add all c files in a directory, then set a specific file to not use precompiled headers.

```lua
files { "*.c" }
nopch { "a_file.c" }
```

Add an entire directory of C files, then set one directory to not use precompiled headers

```lua
files { "*.c" }
nopch { "tests/*.c" }
```

[Back to top](#table-of-contents)

---
### objdir(_path_)
Sets an object and intermediate file directory for a project. By default, object and intermediate files are stored in a directory named "obj" in the same directory as the project.

**Scope:** solutions, projects, configurations

#### Arguments
_path_ - directory where the object and intermediate files should be stored, relative to the currently-executing script file.

#### Examples

```lua
project "MyProject"
    objdir "objects"
```

Set object directories per configuration

```lua
configuration "Debug"
    objdir "../obj_debug"

configuration "Release"
    objdir "../obj_release"
```

[Back to top](#table-of-contents)

---
### options({_options_...})
Specifies build flags to modify the compiling or linking process. This differs from `flags` in
that these are set per project rather than per configuration.

**Scope:** solutions, projects

#### Arguments
_options_ - List of option names from list below. Names are case-insensitive and ignored if not supported on a platform.

* _ArchiveSplit_ - Split arguments to the gmake archiver across multiple invocations, if there are too many of them.
* _ForceCPP_ - Force compiling source as C++ despite the file extension suggesting otherwise.
* _SkipBundling_ - Disable generating bundles for Apple platforms.
* _XcodeLibrarySchemes_ - Generate XCode schemes for libraries too. (By default schemes are only created for runnable apps.)
* _XcodeSchemeNoConfigs_ - Generate a single scheme per project, rather than one per project config.

[Back to top](#table-of-contents)

---
### pchheader(_file_)
Sets the main header file for precompiled header support.

**Scope:** projects

#### Arguments
_file_ - name of the header file, as it is specified in your `#include` statements

#### Examples

```lua
pchheader "afxwin.h"
pchsource "afxwin.cpp"
```

[Back to top](#table-of-contents)

---
### pchsource(_file_)
Sets the main source file for precompiled header support. Only used by Visual Studio.

**Scope:** projects

#### Arguments
_file_ - name of the source file, relative to the currently-executing script file

#### Examples

```lua
pchheader "afxwin.h"
pchsource "afxwin.cpp"
```

[Back to top](#table-of-contents)

---
### platforms({_identifiers_...})
Specifies a set of target hardware platforms for a solution.

_Platform support is a new, experimental feature. The syntax and behavior described here might change as we sort out the details_

**Scope:** solutions

#### Arguments
_identifiers_ - list of hardware platform specifiers from this list:

* _Native_ - general build not targeting any particular platform. If your project can be built in a generic fashion, you should include this as the first platform option
* _x32_ - 32-bit environment
* _x64_ - 64-bit environment
* _Universal_ - OS X universal binary, target both 32- and 64-bit versions of x86 and PPC. Automated dependency generation must be turned off, and always do a clean build. Not supported by Visual Studio.
* _Universal32_ - like _Universal_ above, but targeting only 32-bit platforms
* _Universal64_ - like _Universal_ above, but targeting only 64-bit platforms
* _PS3_ - Playstation 3
* _WiiDev_ - Wii
* _Xbox360_ - Xbox 360 compiler and linker under Visual Studio
* _PowerPC_ - PowerPC processors
* _ARM_ - ARM-based processors
* _Orbis_ - Playstation 4
* _Durango_ - Xbox One

#### Return Value
Current list of target platforms for the active solution

#### Examples
Generic build, as well as OS X Universal build

```lua
solution "MySolution"
    configurations { "Debug", "Release" }
    platforms { "native", "universal" }
```

Prove 32- and 64-bit specific build targets. No generic build is provided so one of these two platforms must always be used. Do this only if your software requires knowledge of the underlying architecture at build time; otherwise, include _native_ to provide a generic build.

```lua
solution "MySolution"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
```

You can retrieve the current list of platforms by calling the function with no parameters

```lua
local p = platforms()
```

Once you have defined a list of platforms, you may use those identifiers to set up configuration filters and apply platform-specific settings.

```lua
configuration "x64"
    defines "IS_64BIT"

-- You can also mix platforms with other configuration selectors
configuration { "Debug", "x64" }
    defines "IS_64BIT_DEBUG"
```

[Back to top](#table-of-contents)

---
### postbuildcommands({_commands_...})
Specifies shell commands to run after build is finished

**Scope:** solutions, projects, configurations

#### Arguments
_commands_ - one or more shell commands

#### Examples

```lua
configuration "windows"
    postbuildcommands { "copy default.config bin\\project.config" }

configuration "not windows"
    postbuildcommands { "cp default.config bin/project.config" }
```

[Back to top](#table-of-contents)

---
### postcompiletasks({_commands_...})
Specifies shell commands to run after compile of file is finished
(GMAKE specific)

**Scope:** solutions, projects, configurations

#### Arguments
_commands_ - one or more shell commands

#### Examples

```lua
    postcompiletasks { "rm $(@:%.o=%.d)" }
```

[Back to top](#table-of-contents)

---
### prebuildcommands({_commands_...})
Specifies shell commands to run before each build

**Scope:** solutions, projects, configurations

#### Arguments
_commands_ - one or more shell commands

#### Examples

```lua
configuration "windows"
    prebuildcommands { "copy default.config bin\\project.config" }

configuration "not windows"
    prebuildcommands { "cp default.config bin/project.config" }
```

[Back to top](#table-of-contents)

---
### prelinkcommands({_commands_...})
Specifies shell commands to run after source files have been compiled, but before the link step

**Scope:** solutions, projects, configurations

#### Arguments
_commands_ - one or more shell commands

#### Examples

```lua
configuration "windows"
    prelinkcommands { "copy default.config bin\\project.config" }

configuration "not windows"
    prelinkcommands { "cp default.config bin/project.config" }
```

[Back to top](#table-of-contents)


---
### project(_name_)
Creates a new project and makes it active. Projects contain all of the settings necessary to build a single binary target, and are synonymous with a Visual Studio Project. These settings include the list of source code files, the programming language used by those files, compiler flags, include directories, and which libraries to link against.

Every project belongs to a solution.

#### Arguments
_name_ - a unique name for the project. If a project with the given name already exists, it is made active and returned. The project name will be used as the file name of the generated solution file.

#### Return Value
The active project object.

#### The `project` Object
Every project is represented in Lua as a table of key-value pairs. You should treat this object as read-only and use the GENie API to make any changes.

* _basedir_  - directory where the project was originally defined. Root for relative paths.
* _blocks_   - list of configuration blocks
* _language_ - project language, if set
* _location_ - output directory for generated project file
* _name_     - name of the project
* _solution_ - solution which contains the project
* _uuid_     - unique identifier

#### Examples
Create a new project named "MyProject". Note that a solution must exist to contain the project. The indentation is for readability and is optional.

```lua
solution "MySolution"
    configurations { "Debug", "Release" }

    project "MyProject"
```

You can retrieve the currently active project by calling `project` with no parameters.

```lua
local prj = project()
```

You can retrieve the list of projects associated with a solution using the `projects` field of the solution object, which may then be iterated over.

```lua
local prjs = solution().projects
for i, prj in ipairs(prjs) do
    print(prj.name)
end
```

[Back to top](#table-of-contents)

---
### removefiles({_files_...})
Removes files from the project. This is different from [excludes](#excludesfiles) in that it completely removes them from the project, not only from the build. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_files_ - list of files to remove.

[Back to top](#table-of-contents)

---
### removeflags({_flags_...})
Removes flags from the flag list.

**Scope:** solutions, projects, configurations

#### Arguments
_flags_ - list of flags to remove from the flag list. They must be valid flags.

[Back to top](#table-of-contents)

---
### removelinks({_references_...})

Removes flags from the flag list.

**Scope:** solutions, projects, configurations

#### Arguments
_references_ - list of libraries and project names to remove from the links list.

[Back to top](#table-of-contents)

---
### removeplatforms({_platforms_...})

Removes platforms from the platform list.

**Scope:** solutions, projects, configurations

#### Arguments
_platforms_ - list of platforms to remove from the platforms list.

[Back to top](#table-of-contents)

---
### resdefines({_symbols_...})
Specifies preprocessor symbols for the resource compiler. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_symbols_ - list of symbols to be defined

#### Examples

```lua
resdefines { "DEBUG", "TRACE" }
```

```lua
resdefines { "CALLSPEC=__dllexport" }
```

[Back to top](#table-of-contents)

---
### resincludedirs({_paths_...})
Specifies the include file search paths for the resource compiler. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_paths_ - list of include file search directories, relative to the currently executing script file

#### Examples

```lua
resincludedirs { "../lua/include", "../zlib" }
```

May use wildcards

```lua
resincludedirs { "../includes/**" }
```

[Back to top](#table-of-contents)

---
### resoptions({_options_...})
Passes arguments directly to the resource compiler. Multiple calls are concatenated.

**Scope:** solutions, projects, configurations

#### Arguments
_options_ - list of resource compiler flags and options

#### Examples

```lua
configuration { "linux", "gmake" }
    resoptions { "`wx-config --cxxflags`", "-ansi", "-pedantic" }
```

[Back to top](#table-of-contents)

---
### solution(_name_)
Creates a new solution and makes it active. Solutions are the top-level objects in a GENie build script, and are synonymous with a Visual Studio solution. Each solution contains one or more projects, which in turn contain the settings to generate a single binary target.

#### Arguments
_name_ - unique name for the solution. If a solution with the given name already exists, it is made active and returned. This value will be used as the file name of the generated solution file.

#### Return Value
The active `solution` object.

#### The `solution` Object
Represented as a Lua table key-value pairs, containing the following values. You should treat this object as read-only and use the GENie API to make any changes.

* _basedir_        - directory where the original project was defined; acts as a root for relative paths
* _configurations_ - list of valid configuration names
* _blocks_         - list of configuration blocks
* _language_       - solution language, if set
* _location_       - output directory for the generated solution file
* _name_           - name of the solution
* _platforms_      - list of target platforms
* _projects_       - list of projects contained by the solution

#### Examples

```lua
solution "MySolution"
```

You can retrieve the currently active solution object by calling `solution` with no parameters.

```lua
local sln = solution()
```

You can use the global variable `_SOLUTIONS` to list out all of the currently defined solutions.

```lua
for i, sln in ipairs(_SOLUTIONS) do
    print(sln.name)
end
```

[Back to top](#table-of-contents)

---
###  startproject(_name_)
Sets the start (default) project for the solution. Works for VS, QBS and Xcode.

**Scope:** solutions

#### Arguments
_name_ - name of the project to set as the start project.

### Examples

```lua
solution "MySolution"
    startproject "MyProjectFoo"
    -- [...]

project "MyProjectFoo"
-- [...]

project "MyProjectBar"
-- [...]
```

[Back to top](#table-of-contents)

---
### systemincludedirs({_paths_...})
Specifies the system include file search paths. Multiple calls are concatenated.

For clang/gcc, it maps to setting the include directory using the `-isystem` option.

On the other build systems, it behaves like [includedirs](#includedirspaths),
but is always searched after directories specified using includedirs.

**Scope:** solutions, projects, configurations

#### Arguments
_paths_ - list of system include file search directories, relative to the currently-executing script file.

#### Examples
Define two include file search paths

```lua
systemincludedirs { "../lua/include", "../zlib" }
```

You can also use [wildcards](#wildcards) to match multiple directories.

```lua
systemincludedirs { "../includes/**" }
```

[Back to top](#table-of-contents)

---
### targetdir(_path_)
Sets the destination directory for the compiled binary target. By default, generated project files will place their compiled output in the same directory as the script.

**Scope:** solutions, projects, configurations

#### Arguments
_path_ - file system path to the directory where the compiled target file should be stored, relative to the currently executing script file.

#### Examples

```lua
project "MyProject"

    configuration "Debug"
        targetdir "bin/debug"

    configuration "Release"
        targetdir "bin/release"
```

[Back to top](#table-of-contents)

---
### targetextension(_ext_)
Specifies the file extension for the compiled binary target. By default, the project will use the system's normal naming conventions: ".exe" for Windows executables, ".so" for Linux shared libraries, etc.

**Scope:** solutions, projects, configurations

#### Arguments
_ext_ - new file extension, including leading dot

#### Examples

```lua
targetextension ".zmf"
```

[Back to top](#table-of-contents)

---
### targetname(_name_)
Specifies the base file name for the compiled binary target. By default, the project name will be used as the file name of the compiled binary target.

**Scope:** solutions, projects, configurations

#### Arguments
_name_ - new base file name

#### Examples

```lua
targetname "mytarget"
```

[Back to top](#table-of-contents)

---
### targetprefix(_prefix_)
Specifies the file name prefix for the compiled binary target. By default, system naming conventions will be used: "lib" for POSIX libraries (e.g. "libMyProject.so") and no prefix elsewhere.

**Scope:** solutions, projects, configurations

#### Arguments
_prefix_ - new file name prefix

#### Examples

```lua
targetprefix "plugin"
```

The prefix may also be set to an empty string for no prefix

```lua
targetprefix ""
```

[Back to top](#table-of-contents)

---
### targetsubdir(_path_)
Sets a subdirectory inside the target directory for the compiled binary target.

**Scope:** solutions, projects, configurations

#### Arguments
_path_ - name of the subdirectory.

[Back to top](#table-of-contents)

---
### targetsuffix(_suffix_)
Specifies a file name suffix for the compiled binary target.

**Scope:** solutions, projects, configurations

#### Arguments
_suffix_ - new filename suffix

#### Examples

```lua
--- Add "-d" to debug versions of files
configuration "Debug"
    targetsuffix "-d"
```

[Back to top](#table-of-contents)

---
### uuid(_projectuuid_)
Sets the UUID for a project. GENie automatically assigns a UUID to each project, which is used by the Visual Studio generators to identify the project within a solution. This UUID is essentially random and will change each time the project file is generated. If you are storing the generated Visual Studio project files in a version control system, this will create a lot of unnecessary deltas. Using the `uuid` function, you can assign a fixed UUID to each project which never changes.

**Scope:** projects

#### Arguments
_projectuuid_ - UUID for the current project

### Return Value
Current project UUID or `nil` if no UUID has been set

#### Examples

```lua
uuid "XXXXXXXX-XXXX-XXXX-XXXXXXXXXXXX"
```

[Back to top](#table-of-contents)

---
### vpaths({[_group_] = {_pattern_...}})
Places files into groups for "virtual paths", rather than mirroring the filesystem. This allows you to, for instance, put all header files in a group called "Headers", no matter where they appeared in the source tree.

**Note:** May be set on the solution, project, or configuration, but only project-level file lists are currently supported.

**Scope:** solutions, projects, configurations

#### Arguments
Table of values, where keys (_groups_) are strings and values (_pattern_) are lists of file system patterns.

_group_   - name for the new group  
_pattern_ - file system pattern for matching file names

#### Examples

Place all header files into a virtual path called "Headers". Any directory information is removed, "src/lua/lua.h" will appear in the IDE as "Headers/lua.h"

```lua
vpaths { ["Headers"] = "**.h" }
```

You may specify multiple file patterns using table syntax

```lua
vpaths {
    ["Headers"] = { "**.h", "**.hxx", "**.hpp" }
}
```

It is also possible to include the file's path in the virtual group. Using this rule, "src/lua/lua.h" will appear in the IDE as "Headers/src/lua/lua.h".

```lua
vpaths { ["Headers/*"] = "**.h" }
```

Any directory information explicitly provided in the pattern will be removed from the replacement. Using this rule, "src/lua/lua.h" will appear in the IDE as "Headers/lua/lua.h".

```lua
vpaths { ["Headers/*"] = "src/**.h" }
```

You can also use virtual paths to remove extra directories from the IDE. Using this rule, "src/lua/lua.h" will appear in the IDE as "lua/lua.h".

```lua
vpaths { ["*"] = "src" }
```

You may specify more than one rule at a time

```lua
vpaths {
    ["Headers"]   = "**.h",
    ["Sources/*"] = {"**.c", "**.cpp"},
    ["Docs"]      = "**.txt"
}
```

[Back to top](#table-of-contents)

---
### xcodeprojectopts({[_key_] = _value_, ...})
#### XCode only
Sets XCode project options in the generated project files. [List of options.](https://gist.github.com/tkersey/39b4fe69e14b859889ffadccb009e397)

#### Arguments
_key_ - Name of the option to set
_value_ - Value to set it to

#### Examples

```lua
xcodeprojectopts {
    ENABLE_BITCODE = "NO",
    GCC_ENABLE_TRIGRAPHS = "YES",
}
```

[Back to top](#table-of-contents)

---
### xcodetargetopts({[_key_] = _value_, ...})
#### XCode only
Sets XCode target options in the generated project files. [List of options.](https://gist.github.com/tkersey/39b4fe69e14b859889ffadccb009e397)

#### Arguments
_key_ - Name of the option to set
_value_ - Value to set it to

#### Examples

```lua
xcodetargetopts {
    ALWAYS_SEARCH_USER_PATHS = "YES",
}
```

[Back to top](#table-of-contents)

---
### xcodescriptphases({{_cmd_, {_inputpaths_, ...}}})
#### XCode only
Adds a script phase to the generated XCode project file.
One tag can contain several commands with different inputpaths.

#### Arguments
_cmd_ - The actual command to run. (This can be a shell script file or direct shell code).
_inputpaths_ - The paths passed to the command

#### Examples
_Building shader files_

```lua
xcodescriptphases {
    {"shaderc_xcode.sh", {
        os.matchfiles("**.shader")}
    },
}
```

_Copying, trimming and signing frameworks by relying on [carthage](https://github.com/Carthage/Carthage)_

```lua
xcodescriptphases {
    {"carthage copy-frameworks", {
        os.matchdirs("**.frameworks")}
    },
}
```

#### Caveats
- Script phases are added in their order of declaration inside the project,
  and in their order of declaration inside the tag.
- The input paths are used as passed to the tag.
  If relative paths are required, you have to rebase them beforehand using `path.getrelative()`.
- For commands/scripts: You can iterate over the input paths using the following XCode variables:
  `${SCRIPT_INPUT_FILE_COUNT}`: The number of input paths provided to the script
  `${SCRIPT_INPUT_FILE_0}` ...: The input paths at index 0 and so on.
  **NOTE**: You can construct the indexed variable as in the example below:
```bash
for (( i = 0; i < ${SCRIPT_INPUT_FILE_COUNT}; ++i )); do
    varname=SCRIPT_INPUT_FILE_$i
    echo ${!varname}
done
```

[Back to top](#table-of-contents)

---
### xcodecopyresources({{_targetpath_, {_inputfiles_, ...}}})
#### XCode only
Adds a 'Copy Files' phase to the generated XCode project file.
One tag can contain several target paths with different input files.

#### Arguments
_targetpath_ - The target path relative to the _Resource_ folder in the resulting `.app` structure.
_inputfiles_ - The input files to be copied.

#### Examples

```lua
xcodecopyresources {
    { ".", {
        "GameResources", -- a folder
    }},
    { "shaders", {
         os.matchfiles("**.shader"), -- sparse files
    }},
}
```

#### Caveats
- The target path is only handled as relative to the _Resource_ folder. No other folder can be indicated at the moment.
  If you need support for other targets, please file an issue on Github.
- `xcodecopyresources` can only be set _per project_, not _per configuration_.


[Back to top](#table-of-contents)

---
### xcodecopyframeworks({_inputframeworks_, ...})
#### XCode only
Adds a 'Copy Files' phase to the generated XCode project file that will copy and sign the provided frameworks.

#### Arguments
_inputframeworks_ - A list of frameworks to be copied to the `.app` structure, with the `SignOnCopy` flag set.

#### Examples

```lua
links { -- frameworks have to be linked with the .app first
    "GTLR.framework",
    "BGFX.framework",
}
xcodecopyframeworks {
    "GTLR.framework",
    "BGFX.framework",
}
```

#### Caveats
- Frameworks must be known to the project to be copyable: set the link dependency accordingly using `links {}`.
- `xcodecopyframeworks` can only be set _per project_, not _per configuration_.

[Back to top](#table-of-contents)

---

---
### wholearchive({_references_...})
Specifies a list of libraries to link without stripping unreferenced object files. The libraries must have already been added using `links`, and the same identifier must be specified.

**Scope:** solutions, projects, configurations

#### Arguments
_references_ - list of library and project names

#### Examples

```lua
project "static_lib"
    kind "StaticLib"

project "console_app"
    kind "ConsoleApp"
    links { "static_lib" }
    wholearchive { "static_lib" }
```

#### References
* [Clang documentation](https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang2-force-load)
* [GNU documentation](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_3.html#IDX183)
* [Microsoft documentation](https://docs.microsoft.com/en-us/cpp/build/reference/wholearchive-include-all-library-object-files?view=vs-2017)

[Back to top](#table-of-contents)


---
## Utility functions

### iif(_condition_, _trueval_, _falseval_)
Implements an immediate `if` clause, returning one of two possible values.

#### Arguments
_condition_ - logical condition to test  
_trueval_ - value to return if _condition_ evaluates to `true`  
_falseval_ - value to return if _condition_ evaluates to `false`

#### Examples

```lua
result = iif(os.is("windows"), "is windows", "is not windows")
```

Note that all expressions are evaluated before the condition is checked. The following expression cannot be implemented with an `iif` because it may try to concatenate a string value.

```lua
result = iif(x -= nil, "x is " .. x, "x is nil")
```

[Back to top](#table-of-contents)

---
### os.chdir(_path_)
Changes the working directory

#### Arguments
_path_ - path to the new working directory

#### Return Value
`true` if successful, otherwise `nil` and an error message

[Back to top](#table-of-contents)

---
### os.copyfile(_source_, _destination_)
Copies a file from one location to another.

#### Arguments
_source_ - file system path to the file to be copied
_destination_ - path to the copy location

#### Return Value
`true` if successful, otherwise `nil` and an error message

[Back to top](#table-of-contents)

---
### os.findlib(_libname_)
Scans the well-known system locations looking for a binary file.

#### Arguments
_libname_ - name of the library to locate. May be specified with (libX11.so) or without (X11) system-specified decorations.

#### Return Value
The path containing the library file, if found. Otherwise, `nil`.

[Back to top](#table-of-contents)

---
### os.get()
Identifies the currently-targeted operating system.

#### Return Value
One of "bsd", "linux", "macosx", "solaris", or "windows"

**Note:** This function returns the OS being targeted, which is not necessarily the same as the OS on which GENie is being run.

#### Example

```lua
if os.get() == "windows" then
    -- do something windows-specific
end
```

[Back to top](#table-of-contents)

---
### os.getcwd()
Gets the current working directory.

#### Return Value
The current working directory

[Back to top](#table-of-contents)

---
### os.getversion()
Retrieves version information for the host operating system

**Note:** Not implemented for all platforms. On unimplemented platforms, will return `0` for all version numbers, and the platform name as the description.

#### Return Value
Table containing the following key-value pairs:

| Key          | Value                                        |
| ------------ | -------------------------------------------- |
| majorversion | major version number                         |
| minorversion | minor version number                         |
| revision     | bug fix release or service pack number       |
| description  | human-readable description of the OS version |


#### Examples

```lua
local ver = os.getversion()
print(string.format(" %d.%d.%d (%s)",
    ver.majorversion, ver.minorversion, ver.revision,
    ver.description))

-- On Windows XP: "5.1.3 (Windows XP)"
-- On OSX: "10.6.6 (Mac OS X Snow Leopard)"
```

[Back to top](#table-of-contents)

---
### os.is(_id_)
Checks the current operating system identifier against a particular value

#### Arguments
_id_ - one of "bsd", "linux", "macosx", "solaris", or "windows"

**Note:** This function returns the OS being targeted, which is not necessarily the same as the OS on which GENie is being run.

#### Return Value
`true` if the supplied _id_ matches the current operating system identifier, `false` otherwise.

[Back to top](#table-of-contents)

---
### os.is64bit()
Determines if the host is using a 64-bit processor.

#### Return Value
`true` if the host system has a 64-bit processor
`false` otherwise

#### Examples

```lua
if os.is64bit() then
    print("This is a 64-bit system")
else
    print("This is NOT a 64-bit system")
end
```

[Back to top](#table-of-contents)

---
### os.isdir(_path_)
Checks for the existence of a directory.

#### Arguments
_path_ - the file system path to check

#### Return Value
`true` if a matching directory is found  
`false` if there is no such file system path, or if the path points to a file

[Back to top](#table-of-contents)

---
### os.isfile(_path_)
Checks for the existence of a file.

#### Arguments
_path_ - the file system path to check

#### Return Value
`true` if a matching file is found  
`false` if there is no such file system path or if the path points to a directory instead of a file

[Back to top](#table-of-contents)

---
### os.matchdirs(_pattern_)
Performs a wildcard match to locate one or more directories.

#### Arguments
_pattern_ - file system path to search. May [wildcard](#wildcard) patterns.

#### Return Value
List of directories which match the specified pattern. May be empty.

#### Examples

```lua
matches = os.matchdirs("src/*")     -- non-recursive match
matches = os.matchdirs("src/**")    -- recursive match
matches = os.matchdirs("src/test*") -- may also match partial name
```

[Back to top](#table-of-contents)

---
### os.matchfiles(_patterns_)
Performs a wildcard match to locate one or more directories.

#### Arguments
_pattern_ - file system path to search. May contain [wildcard](#wildcard) patterns.

#### Return Value
List of files which match the specified pattern. May be empty.

#### Examples

```lua
matches = os.matchfiles("src/*.c")  -- non-recursive match
matches = os.matchfiles("src/**.c") -- recursive match
```

[Back to top](#table-of-contents)

---
### os.mkdir(_path_)
Creates a new directory.

#### Arguments
_path_ - path to be created

#### Return Value
`true` if successful  
`nil` and an error message otherwise

[Back to top](#table-of-contents)

---
### os.outputof(_command_)
Runs a shell command and returns the output.

#### Arguments
_command_ - shell command to run

#### Return Value
The output of the command

#### Examples

```lua
-- Get the ID for the host processor architecture
local proc = os.outputof("uname -p")
```

[Back to top](#table-of-contents)

---
### os.pathsearch(_fname_, _paths..._)
description

**Scope:** solutions, projects, configurations

#### Arguments
_fname_ - name of the file being searched, followed by one or more path sets to be searched  
_paths_ - the match format of the PATH environment variable: a colon-delimited list of path. On Windows, you may use a semicolon-delimited list if drive letters might be included

#### Return Value
Path to the directory which contains the file, if found
`nil` otherwise

#### Examples

```lua
local p = os.pathsearch("mysystem.config", "./config:/usr/local/etc:/etc")
```

[Back to top](#table-of-contents)

---
### os.rmdir(_path_)
Removes an existing directory as well as any files or subdirectories it contains.

#### Arguments
_path_ - file system path to be removed

#### Return Value
`true` if successful  
`nil` and an error message otherwise

[Back to top](#table-of-contents)

---
### os.stat(_path_)
Retrieves information about a file.

#### Arguments
_path_ - path to file for which to retrieve information

#### Return Value
Table of values:

| Key   | Value                   |
| ----- | ----------------------- |
| mtime | Last modified timestamp |
| size  | File size in bytes      |

[Back to top](#table-of-contents)

---
### userincludedirs({_paths_...})
Specifies the user include file search paths. Multiple calls are concatenated.

For XCode, it maps to setting the USER INCLUDE SEARCH PATH. 

For clang/gcc, it maps to setting the include directory using the iquote option.

On the other build systems, it behaves like [includedirs](#includedirspaths).

**Scope:** solutions, projects, configurations

#### Arguments
_paths_ - list of user include file search directories, relative to the currently-executing script file.

#### Examples
Define two include file search paths

```lua
userincludedirs { "../lua/include", "../zlib" }
```

You can also use [wildcards](#wildcards) to match multiple directories.

```lua
userincludedirs { "../includes/**" }
```

[Back to top](#table-of-contents)

---
### os.uuid(_name_)
Returns a Universally Unique Identifier

#### Arguments
_name_ - (optional) string to be hashed

#### Return Value
A new UUID, a string value with the format `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`, generated from _name_ if it is provided, otherwise generated from random data

[Back to top](#table-of-contents)

---
### path.getabsolute(_path_)
Converts relative path to absolute path

#### Arguments
_path_ - the relative path to be converted

#### Return Value
New absolute path, calculated from the current working directory

[Back to top](#table-of-contents)

---
### path.getbasename(_path_)
Extracts base file portion of a path, with the directory and extension removed.

#### Arguments
_path_ - path to be split

#### Return Value
Base name portion of the path

[Back to top](#table-of-contents)

---
### path.getdirectory(_path_)
Extracts directory portion of a path, with file name removed

#### Arguments
_path_ - path to be split

#### Return Value
Directory portion of the path

[Back to top](#table-of-contents)

---
### path.getdrive(_path_)
Returns drive letter portion of a path

#### Arguments
_path_ - path to be split

#### Return Value
Drive letter portion of the path, or `nil`

[Back to top](#table-of-contents)

---
### path.getextension(_path_)
Returns file extension portion of a path

#### Arguments
_path_ - path to be split

#### Return Value
File extension portion of the path, or an empty string

[Back to top](#table-of-contents)

---
### path.getname(_path_)
Returns file name and extension, removes directory information.

#### Arguments
_path_ - path to be split

#### Return Value
File name and extension without directory information

[Back to top](#table-of-contents)

---
### path.getrelative(_src_, _dest_)
Computes relative path from one directory to another.

#### Arguments
_src_ - originating directory  
_dest_ - target directory

#### Return Value
Relative path from _src_ to _dest_

[Back to top](#table-of-contents)

---
### path.isabsolute(_path_)
Returns whether or not a path is absolute.

#### Arguments
_path_ - path to check

#### Return Value
`true` if path is absolute  
`false` otherwise

[Back to top](#table-of-contents)

---
### path.iscfile(_path_)
Determines whether file is a C source code file, based on extension.

#### Arguments
_path_ - path to check

#### Return Value
`true` if path uses a C file extension  
`false` otherwise

[Back to top](#table-of-contents)

---
### path.isSourceFile(_path_)
Determines whether a file is a C++ source code file, based on extension.

#### Arguments
_path_ - path to check

#### Return Value
`true` if path uses a C++ file extension  
`false` otherwise

[Back to top](#table-of-contents)

---
### path.isresourcefile(_path_)
Determines whether a path represends a Windows resource file, based on extension.

#### Arguments
_path_ - path to check

#### Return Value
`true` if path uses a well-known Windows resource file extension  
`false` otherwise

[Back to top](#table-of-contents)

---
### path.join(_leading_, _trailing_)
Joins two path portions together into a single path.

**Note:** if _trailing_ is an absolute path, then _leading_ is ignored and the absolute path is returned.

#### Arguments
_leading_ - beginning portion of the path  
_trailing_ - ending portion of the path

#### Return Value
Merged path

#### Examples

```lua
-- returns "MySolution/MyProject"
p = path.join("MySolution", "MyProject")

-- returns "/usr/bin", because the trailing path is absolute
p = path.join("MySolution", "/usr/bin")

-- tokens are assumed to be absolute. This returns `${ProjectDir}`
p = path.join("MySolution", "$(ProjectDir)")
```

[Back to top](#table-of-contents)

---
### path.rebase(_path_, _oldbase_, _newbase_)
Takes a relative path and makes it relative to a different location.

#### Arguments
_path_ - path to be modified  
_oldbase_ - original base directory, from which _path_ is relative  
_newbase_ - the new base directory, from where the resulting path should be relative

#### Return Value
Rebased path

[Back to top](#table-of-contents)

---
### path.translate(_path_, _newsep_)
Converts the separators in a path.

#### Arguments
_path_ - path to modify  
_newsep_ - new path separator. Defaults to current environment default.

#### Return Value
Modified path

[Back to top](#table-of-contents)
---

### printf(_format_, _args_...)
Prints a formatted string

#### Arguments
_format_ - formatting string, containing C `printf()` formatting codes  
_args_ - arguments to be substituted into the format string

[Back to top](#table-of-contents)

---
### string.endswith(_haystack_, _needle_)
Checks if the given _haystack_ string ends with _needle_.

#### Arguments
_haystack_ - string to search within  
_needle_   - string to check ending of _haystack_ against

#### Return Value
`true`  - _haystack_ ends with _needle_  
`false` - _haystack_ does not end with _needle_

[Back to top](#table-of-contents)

---
### string.explode(_str_, _pattern_)
Breaks a string into an array of strings, formed by splitting _str_ on _pattern_.

#### Arguments
_str_     - string to be split  
_pattern_ - separator pattern at which to split; may use Lua's pattern matching syntax

#### Return Value
List of substrings

[Back to top](#table-of-contents)

---
### string.findlast(_str_, _pattern_, _plain_)
Finds the last instance of a pattern within a string.

#### Arguments
_str_     - string to be searched  
_pattern_ - pattern to search for; may use Lua's pattern matching syntax  
_plain_   - whether or not plain string comparison should be used (rather than pattern-matching)

#### Return Value
The matching pattern, if found, or `nil`

[Back to top](#table-of-contents)

---
### string.startswith(_haystack_, _needle_)
Checks if the given _haystack_ starts with _needle_.

#### Arguments
_haystack_ - string to search within  
_needle_   - string to check start of _haystack_ against

#### Return Value
`true`  - _haystack_ starts with _needle_  
`false` - _haystack_ does not start with _needle_

[Back to top](#table-of-contents)

---
### table.contains(_array_, _value_)
Determines if a _array_ contains _value_.

#### Arguments
_array_ - table to test for _value_  
_value_ - _value_ being tested for

#### Return Value
`true`  - _array_ contains _value_  
`false` - _array_ does not contain _value_

[Back to top](#table-of-contents)

---
### table.implode(_array_, _before_, _after_, _between_)
Merges an array of items into a single formatted string.

#### Arguments
_array_   - table to be converted into a string  
_before_  - string to be inserted before each item  
_after_   - string to be inserted after each item  
_between_ - string to be inserted between each item

#### Return Value
Formatted string

[Back to top](#table-of-contents)

---

## Additional information

### Wildcards

In some places, wildcards may be used in string values passed to a function. Usually, these strings represent paths. There are two types of wildcards:

* `*` - matches files within a single directory
* `**` - matches files recursively in any child directory
