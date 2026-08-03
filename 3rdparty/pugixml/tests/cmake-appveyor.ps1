$failed = $FALSE

$vs = $args[0]
$vsy = $args[1]

$target = "cmake_vs${vs}_${vsy}"

Add-AppveyorTest $target -Outcome Running

mkdir $target
pushd $target

try
{
	Write-Output "# Setting up CMake VS$vs"
	& cmake .. -G "Visual Studio $vs $vsy" | Tee-Object -Variable cmakeOutput

	$sw = [Diagnostics.Stopwatch]::StartNew()

	if ($?)
	{
		Write-Output "# Building"

		& cmake --build . | Tee-Object -Variable cmakeOutput

		if ($?)
		{
			Write-Output "# Passed"

			Update-AppveyorTest $target -Outcome Passed -StdOut ($cmakeOutput | out-string) -Duration $sw.ElapsedMilliseconds
		}
		else
		{
			Write-Output "# Failed to build"

			Update-AppveyorTest $target -Outcome Failed -StdOut ($cmakeOutput | out-string) -ErrorMessage "Building failed"

			$failed = $TRUE
		}
	}
	else
	{
		Write-Output "# Failed to configure"

		Update-AppveyorTest $target -Outcome Failed -StdOut ($cmakeOutput | out-string) -ErrorMessage "Configuration failed"

		$failed = $TRUE
	}
}
finally
{
	popd
}

if ($failed) { throw "One or more build steps failed" }

Write-Output "# End"
