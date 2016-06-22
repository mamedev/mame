premake.ninja = { }

local p = premake

newaction
{
	-- Metadata for the command line and help system
	trigger     = "ninja",
	shortname   = "ninja",
	description = "Ninja is a small build system with a focus on speed",
	module      = "ninja",

	-- The capabilities of this action
	valid_kinds     = {"ConsoleApp", "WindowedApp", "SharedLib", "StaticLib"},
	valid_languages = {"C", "C++"},
	valid_tools     = {
		cc = { "gcc" }
	},

	-- Solution and project generation logic
	onsolution = function(sln)
		io.eol    = "\r\n"
		io.indent = "\t"
		io.escaper(p.ninja.esc)
		p.generate(sln, "Makefile", p.ninja.generate_solution)
		io.indent = "  "
		p.ninja.generate_ninja_builds(sln)
	end,
	
	onproject = function(prj)
		io.eol    = "\r\n"
		io.indent = "  "
		io.escaper(p.ninja.esc)
		p.ninja.generate_project(prj)
	end,
	
	oncleansolution = function(sln)
		for _,name in ipairs(sln.configurations) do
			premake.clean.file(sln, p.ninja.get_solution_name(sln, name))
		end
	end,
	
	oncleanproject = function(prj)
		-- TODO
	end,
	
	oncleantarget = function(prj)
		-- TODO
	end,
}