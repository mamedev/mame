--
-- GENie - Project generator tool
-- https://github.com/bkaradzic/GENie#license
--

premake.ninja = { }

local p = premake

newaction
{
	-- Metadata for the command line and help system
	trigger     = "ninja",
	shortname   = "ninja",
	description = "Generate Ninja build files",
	module      = "ninja",

	-- The capabilities of this action
	valid_kinds     = {"ConsoleApp", "WindowedApp", "StaticLib", "SharedLib", "Bundle"},
	valid_languages = {"C", "C++", "Swift"},
	valid_tools     = {
		cc    = { "gcc" },
		swift = { "swift" },
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
