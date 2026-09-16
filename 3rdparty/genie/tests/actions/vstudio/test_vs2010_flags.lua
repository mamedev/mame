
T.vs2010_flags = { }
local vs10_flags = T.vs2010_flags
local sln, prj

function vs10_flags.setup()
	_ACTION = "vs2010"

	sln = solution "MySolution"
	configurations { "Debug" }
	platforms {}

	prj = project "MyProject"
	language "C++"
	kind "ConsoleApp"
	uuid "AE61726D-187C-E440-BD07-2556188A6565"
	includedirs{"foo/bar"}
end

function vs10_flags.teardown()
	sln = nil
	prj = nil
end

local function get_buffer()
	io.capture()
	premake.bake.buildconfigs()
	sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
	prj = premake.solution.getproject(sln, 1)
	premake.vs2010_vcxproj(prj)
	local buffer = io.endcapture()
	return buffer
end


function vs10_flags.sseSet()
	flags  {"EnableSSE"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<EnableEnhancedInstructionSet>StreamingSIMDExtensions</EnableEnhancedInstructionSet>')
end

function vs10_flags.sse2Set()
	flags  {"EnableSSE2"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<EnableEnhancedInstructionSet>StreamingSIMDExtensions2</EnableEnhancedInstructionSet>')
end

function vs10_flags.extraWarningNotSet_warningLevelIsThree()
	local buffer = get_buffer()
	test.string_contains(buffer,'<WarningLevel>Level3</WarningLevel>')
end

function vs10_flags.extraWarning_warningLevelIsFour()
	flags  {"ExtraWarnings"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<WarningLevel>Level4</WarningLevel>')
end

function vs10_flags.extraWarning_treatWarningsAsError_setToTrue()
	flags  {"FatalWarnings"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<TreatWarningAsError>true</TreatWarningAsError>')
end

function vs10_flags.floatFast_floatingPointModel_setToFast()
	flags  {"FloatFast"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<FloatingPointModel>Fast</FloatingPointModel>')
end

function vs10_flags.floatStrict_floatingPointModel_setToStrict()
	flags  {"FloatStrict"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<FloatingPointModel>Strict</FloatingPointModel>')
end

function vs10_flags.nativeWideChar_TreatWChar_tAsBuiltInType_setToTrue()
	flags  {"NativeWChar"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<TreatWChar_tAsBuiltInType>true</TreatWChar_tAsBuiltInType>')
end

function vs10_flags.nativeWideChar_TreatWChar_tAsBuiltInType_setToFalse()
	flags  {"NoNativeWChar"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<TreatWChar_tAsBuiltInType>false</TreatWChar_tAsBuiltInType>')
end

function vs10_flags.noExceptions_exceptionHandling_setToFalse()
	flags  {"NoExceptions"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<ExceptionHandling>false</ExceptionHandling>')
end

function vs10_flags.structuredExceptions_exceptionHandling_setToAsync()
	flags  {"SEH"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<ExceptionHandling>Async</ExceptionHandling>')
end

function vs10_flags.noFramePointer_omitFramePointers_setToTrue()
	flags  {"NoFramePointer"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<OmitFramePointers>true</OmitFramePointers>')
end


function vs10_flags.noRTTI_runtimeTypeInfo_setToFalse()
	flags  {"NoRTTI"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<RuntimeTypeInfo>false</RuntimeTypeInfo>')
end

function vs10_flags.callingconvention_fastcall()
	flags  {"FastCall"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<CallingConvention>FastCall</CallingConvention>')
end

function vs10_flags.callingconvention_stdcall()
	flags  {"StdCall"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<CallingConvention>StdCall</CallingConvention>')
end

function vs10_flags.optimizeSize_optimization_setToMinSpace()
	flags  {"OptimizeSize"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<Optimization>MinSpace</Optimization>')
end

function vs10_flags.optimizeSpeed_optimization_setToMaxSpeed()
	flags  {"OptimizeSpeed"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<Optimization>MaxSpeed</Optimization>')
end
function vs10_flags.optimizeSpeed_optimization_setToMaxSpeed()
	flags  {"Optimize"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<Optimization>Full</Optimization>')
end


local debug_string = "Symbols"
local release_string = "Optimize"
function vs10_flags.debugHasNoStaticRuntime_runtimeLibrary_setToMultiThreadedDebugDLL()
	flags {debug_string}
	local buffer = get_buffer()
	test.string_contains(buffer,'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>')
end

function vs10_flags.debugAndStaticRuntime_runtimeLibrary_setToMultiThreadedDebug()
	flags {debug_string,"StaticRuntime"}
	local buffer = get_buffer()
	test.string_contains(buffer,'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>')
end

function vs10_flags.releaseHasNoStaticRuntime_runtimeLibrary_setToMultiThreadedDLL()
	flags {release_string}
	local buffer = get_buffer()
	test.string_contains(buffer,'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>')
end

function vs10_flags.releaseAndStaticRuntime_runtimeLibrary_setToMultiThreaded()
	flags {release_string,"StaticRuntime"}
	local buffer = get_buffer()
	test.string_contains(buffer,'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>')
end

function vs10_flags.debugAndMinimalRebuildAndSymbols_minimalRebuild_setToFalse()
	flags {debug_string,"EnableMinimalRebuild"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<MinimalRebuild>true</MinimalRebuild>')
end

function vs10_flags.debugYetNotMinimalRebuild_minimalRebuild_setToTrue()
	flags {debug_string}

	local buffer = get_buffer()
	test.string_contains(buffer,'<MinimalRebuild>true</MinimalRebuild>')
end

function vs10_flags.release_minimalRebuild_setToFalse()
	flags {release_string}

	local buffer = get_buffer()
	test.string_contains(buffer,'<MinimalRebuild>false</MinimalRebuild>')
end

--there is not an option for /Z7 OldStyle
--/ZI is not compatible with /clr or x64_64
--minimal Rebuild requires /Zi in x86_64

function vs10_flags.symbols_32BitBuild_DebugInformationFormat_setToEditAndContinue()
	flags{"Symbols"}
	platforms{'x32'}
	local buffer = get_buffer()
	test.string_contains(buffer,'<DebugInformationFormat>EditAndContinue</DebugInformationFormat>')
end

function vs10_flags.symbols_64BitBuild_DebugInformationFormat_setToProgramDatabase()
	flags{"Symbols"}
	platforms{"x64"}
	local buffer = get_buffer()
	test.string_contains(buffer,'<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>')
end

function vs10_flags.symbolsAndNoEditAndContinue_DebugInformationFormat_setToProgramDatabase()
	flags{"Symbols","NoEditAndContinue"}

	local buffer = get_buffer()
	test.string_contains(buffer,'<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>')
end

function vs10_flags.symbolsAndRelease_DebugInformationFormat_setToProgramDatabase()
	flags{"Symbols",release_string}

	local buffer = get_buffer()
	test.string_contains(buffer,'<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>')
end

function vs10_flags.symbolsManaged_DebugInformationFormat_setToProgramDatabase()
	flags{"Symbols","Managed"}
	local buffer = get_buffer()
	test.string_contains(buffer,'<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>')
end

function vs10_flags.noSymbols_DebugInformationFormat_blockIsEmpty()
	local buffer = get_buffer()
	test.string_contains(buffer,'<DebugInformationFormat></DebugInformationFormat>')
end

function vs10_flags.noSymbols_bufferDoesNotContainprogramDataBaseFile()
	local buffer = get_buffer()
	test.string_does_not_contain(buffer,'<Link>.*<ProgramDataBaseFileName>.*</Link>')
end
function vs10_flags.symbols_bufferContainsprogramDataBaseFile()
	flags{"Symbols"}
	local buffer = get_buffer()
	test.string_contains(buffer,'<ClCompile>.*<ProgramDataBaseFileName>%$%(OutDir%)MyProject%.pdb</ProgramDataBaseFileName>.*</ClCompile>')
end


function vs10_flags.WithOutManaged_bufferContainsKeywordWin32Proj()
	local buffer = get_buffer()
	test.string_contains(buffer,'<PropertyGroup Label="Globals">.*<Keyword>Win32Proj</Keyword>.*</PropertyGroup>')
end

function vs10_flags.WithOutManaged_bufferDoesNotContainKeywordManagedCProj()
	local buffer = get_buffer()
	test.string_does_not_contain(buffer,'<PropertyGroup Label="Globals">.*<Keyword>ManagedCProj</Keyword>.*</PropertyGroup>')
end

T.vs2010_managedFlag = { }
local vs10_managedFlag = T.vs2010_managedFlag

local function vs10_managedFlag_setOnProject()
		local sln = solution "Sol"
		configurations { "Debug" }
		language "C++"
		kind "ConsoleApp"

		local prj = project "Prj"
			flags {"Managed"}

		return sln,prj
end


local function get_managed_buffer(sln,prj)
	io.capture()
	premake.bake.buildconfigs()
	sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
	prj = premake.solution.getproject(sln, 1)
	premake.vs2010_vcxproj(prj)
	local buffer = io.endcapture()
	return buffer
end

function vs10_managedFlag.setup()
end

function vs10_managedFlag.managedSetOnProject_CLRSupport_setToTrue()
	local sln, prj = vs10_managedFlag_setOnProject()
	local buffer = get_managed_buffer(sln,prj)

	test.string_contains(buffer,
			'<PropertyGroup Condition=".*" Label="Configuration">'
				..'.*<CLRSupport>true</CLRSupport>'
			..'.*</PropertyGroup>')
end

function vs10_managedFlag.globals_bufferContainsKeywordManagedCProj()
	local sln, prj = vs10_managedFlag_setOnProject()
	local buffer = get_managed_buffer(sln,prj)
	test.string_contains(buffer,'<PropertyGroup Label="Globals">.*<Keyword>ManagedCProj</Keyword>.*</PropertyGroup>')
end


function vs10_managedFlag.globals_bufferDoesNotContainKeywordWin32Proj()
	local sln, prj = vs10_managedFlag_setOnProject()
	local buffer = get_managed_buffer(sln,prj)
	test.string_does_not_contain(buffer,'<PropertyGroup Label="Globals">.*<Keyword>Win32Proj</Keyword>.*</PropertyGroup>')
end


function vs10_managedFlag.globals_FrameworkVersion_setToV4()
	local sln, prj = vs10_managedFlag_setOnProject()
	local buffer = get_managed_buffer(sln,prj)
	test.string_contains(buffer,'<PropertyGroup Label="Globals">.*<TargetFrameworkVersion>v4.0</TargetFrameworkVersion>.*</PropertyGroup>')
end


function vs10_managedFlag.withFloatFast_FloatingPointModelNotFoundInBuffer()
	local sln, prj = vs10_managedFlag_setOnProject()
	flags {"FloatStrict"}
	local buffer = get_managed_buffer(sln,prj)
	test.string_does_not_contain(buffer,'<FloatingPointModel>.*</FloatingPointModel>')
end

function vs10_managedFlag.debugWithStaticRuntime_flagIgnoredAndRuntimeSetToMDd()
	local sln, prj = vs10_managedFlag_setOnProject()
	flags {"Symbols","StaticRuntime"}
	local buffer = get_managed_buffer(sln,prj)
	test.string_contains(buffer,'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>')
end

function vs10_managedFlag.notDebugWithStaticRuntime_flagIgnoredAndRuntimeSetToMD()
	local sln, prj = vs10_managedFlag_setOnProject()
	flags {"StaticRuntime"}
	local buffer = get_managed_buffer(sln,prj)
	test.string_contains(buffer,'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>')
end

function vs10_managedFlag.noOptimisationFlag_basicRuntimeChecksNotFoundInBuffer()
	local sln, prj = vs10_managedFlag_setOnProject()
	local buffer = get_managed_buffer(sln,prj)
	test.string_does_not_contain(buffer,'<BasicRuntimeChecks>.*</BasicRuntimeChecks>')
end

function vs10_managedFlag.applictionWithOutWinMain_EntryPointSymbolNotFoundInBuffer()
	local sln, prj = vs10_managedFlag_setOnProject()
	local buffer = get_managed_buffer(sln,prj)
	test.string_does_not_contain(buffer,'<EntryPointSymbol>.*</EntryPointSymbol>')
end
