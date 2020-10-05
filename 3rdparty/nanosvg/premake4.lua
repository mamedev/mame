
local action = _ACTION or ""

solution "nanosvg"
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"native", "x64", "x32"}
  
	project "example1"
		kind "ConsoleApp"
		language "C++"
		files { "example/example1.c", "example/*.h", "src/*.h" }
		includedirs { "example", "src" }
		targetdir("build")
	 
		configuration { "linux" }
			 links { "X11","Xrandr", "rt", "GL", "GLU", "pthread", "glfw" }

		configuration { "windows" }
			 links { "glu32","opengl32", "gdi32", "winmm", "user32" }

		configuration { "macosx" }
			links { "glfw3" }
			linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}    

	project "example2"
		kind "ConsoleApp"
		language "C++"
		files { "example/example2.c", "example/*.h", "src/*.h" }
		includedirs { "example", "src" }
		targetdir("build")
	 
		configuration { "linux" }
			 links { "X11","Xrandr", "rt", "pthread" }

		configuration { "windows" }
			 links { "winmm", "user32" }

		configuration { "macosx" }
			linkoptions { "-framework Cocoa", "-framework IOKit" }

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}    
