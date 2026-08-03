function Invoke-CmdScript($scriptName)
{
	$cmdLine = """$scriptName"" $args & set"
	& $Env:SystemRoot\system32\cmd.exe /c $cmdLine |
	select-string '^([^=]*)=(.*)$' | foreach-object {
		$varName = $_.Matches[0].Groups[1].Value
		$varValue = $_.Matches[0].Groups[2].Value
		set-item Env:$varName $varValue
	}
}

$sources = @("src/pugixml.cpp") + (Get-ChildItem -Path "tests/*.cpp" -Exclude "fuzz_*.cpp")
$failed = $FALSE

foreach ($vs in @($args[0]))
{
	foreach ($arch in @($args[1]))
	{
		Write-Output "# Setting up VS$vs $arch"

		if ($vs -eq 15) {
			$vsdevcmdarch = if ($arch -eq "x64") { "amd64" } else { "x86" }
			Invoke-CmdScript "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat" "-arch=$vsdevcmdarch"
		}
		elseif ($vs -eq 19) {
			$vsdevcmdarch = if ($arch -eq "x64") { "amd64" } else { "x86" }
			Invoke-CmdScript "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" "-arch=$vsdevcmdarch"
		}
		elseif ($vs -eq 22) {
			$vsdevcmdarch = if ($arch -eq "x64") { "amd64" } else { "x86" }
			Invoke-CmdScript "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" "-arch=$vsdevcmdarch"
		}
		elseif ($vs -eq 26) {
			$vsdevcmdarch = if ($arch -eq "x64") { "amd64" } else { "x86" }
			Invoke-CmdScript "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" "-arch=$vsdevcmdarch"
		}
		else
		{
			Invoke-CmdScript "C:\Program Files (x86)\Microsoft Visual Studio $vs.0\VC\vcvarsall.bat" $arch
		}

		if (! $?) { throw "Error setting up VS$vs $arch" }

		$cxx = if ($vs -ge 19) { "/std:c++17" } else { "" }

		foreach ($defines in @($args[2]))
		{
			$target = "tests_vs${vs}_${arch}_${defines}"
			$deflist = if ($defines -eq "standard") { "" } else { "/D$defines" }

			Add-AppveyorTest $target -Outcome Running

			Write-Output "# Building $target.exe"
			& cmd /c "cl.exe /MP /Fe$target.exe /EHsc /W4 /WX $cxx $deflist $sources 2>&1" | Tee-Object -Variable buildOutput

			if ($?)
			{
				Write-Output "# Running $target.exe"

				$sw = [Diagnostics.Stopwatch]::StartNew()

				& .\$target | Tee-Object -Variable testOutput

				if ($?)
				{
					Write-Output "# Passed"

					Update-AppveyorTest $target -Outcome Passed -StdOut ($testOutput | out-string) -Duration $sw.ElapsedMilliseconds
				}
				else
				{
					Write-Output "# Failed"

					Update-AppveyorTest $target -Outcome Failed -StdOut ($testOutput | out-string) -ErrorMessage "Running failed"

					$failed = $TRUE
				}
			}
			else
			{
				Write-Output "# Failed to build"

				Update-AppveyorTest $target -Outcome Failed -StdOut ($buildOutput | out-string) -ErrorMessage "Compilation failed"

				$failed = $TRUE
			}
		}
	}
}

if ($failed) { throw "One or more build steps failed" }

Write-Output "# End"
