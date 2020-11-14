function Run-Command([string]$cmd)
{
	Invoke-Expression $cmd
	if ($LastExitCode) { exit $LastExitCode }
}

function Force-Copy([string]$from, [string]$to)
{
	Write-Host $from "->" $to
	New-Item -Force $to | Out-Null
	Copy-Item -Force $from $to
	if (! $?) { exit 1 }
}

function Build-Version([string]$vs, [string]$toolset, [string]$linkage)
{
	$prjsuffix = if ($linkage -eq "static") { "_static" } else { "" }
	$cfgsuffix = if ($linkage -eq "static") { "Static" } else { "" }

	foreach ($configuration in "Debug","Release")
	{
		Run-Command "msbuild pugixml_$vs$prjsuffix.vcxproj /t:Rebuild /p:Configuration=$configuration /p:Platform=x86 /v:minimal /nologo"
		Run-Command "msbuild pugixml_$vs$prjsuffix.vcxproj /t:Rebuild /p:Configuration=$configuration /p:Platform=x64 /v:minimal /nologo"

		Force-Copy "$vs/Win32_$configuration$cfgsuffix/pugixml.lib" "nuget/build/native/lib/Win32/$toolset/$linkage/$configuration/pugixml.lib"
		Force-Copy "$vs/x64_$configuration$cfgsuffix/pugixml.lib" "nuget/build/native/lib/x64/$toolset/$linkage/$configuration/pugixml.lib"
	}
}

Push-Location
$scriptdir = Split-Path $MyInvocation.MyCommand.Path
cd $scriptdir

Force-Copy "../src/pugiconfig.hpp" "nuget/build/native/include/pugiconfig.hpp"
Force-Copy "../src/pugixml.hpp" "nuget/build/native/include/pugixml.hpp"

if ($args[0] -eq 2019){
	Build-Version "vs2019" "v142" "dynamic"
	Build-Version "vs2019" "v142" "static"

} elseif ($args[0] -eq 2017){
	Build-Version "vs2017" "v141" "dynamic"
	Build-Version "vs2017" "v141" "static"

	Build-Version "vs2015" "v140" "dynamic"
	Build-Version "vs2015" "v140" "static"

	Build-Version "vs2013" "v120" "dynamic"
	Build-Version "vs2013" "v120" "static"

} elseif($args[0] -eq 2015){
	Build-Version "vs2015" "v140" "dynamic"
	Build-Version "vs2015" "v140" "static"

	Build-Version "vs2013" "v120" "dynamic"
	Build-Version "vs2013" "v120" "static"

} elseif($args[0] -eq 2013){
	Build-Version "vs2013" "v120" "dynamic"
	Build-Version "vs2013" "v120" "static"
}

Run-Command "nuget pack nuget"

Pop-Location
