--
-- jcdb_solution.lua
-- compile_commands.json functions.
-- Copyright (c) 2020 Johan Skoeld
--

premake.jcdb = {}

local premake = premake
local jcdb = premake.jcdb

local encode_chars = {
	[0x22] = '\\"',
	[0x5c] = "\\\\",
	[0x08] = "\\b",
	[0x0c] = "\\f",
	[0x0a] = "\\n",
	[0x0d] = "\\r",
	[0x09] = "\\t",
}

local function encode_string(s)
	local res = '"'

	for _, cp in utf8.codes(s) do
		if encode_chars[cp] then
			res = res..encode_chars[cp]
		elseif cp < 32 then
			res = res..string.format("\\u%04x", cp)
		else
			res = res..utf8.char(cp)
		end
	end

	return res..'"'
end

local function escape_cmdline_arg(s)
	if s:find("%s") then
		s = s:gsub("\\", "\\\\")
		s = s:gsub('"', '\\"')
		s = '"'..s..'"'
	end

	return s
end

local function list(tbl)
	return iif(#tbl > 0, " "..table.concat(tbl, " "), "")
end

local function build_command(cfg, cc, file)
	local cmdline = ""
	local function app(s) cmdline = cmdline..s end

	-- Compiler
	if path.iscfile(file) or path.isasmfile(file) then
		app(cc.cc)
	else
		app(cc.cxx)
	end

	-- Flags / Defines / Includes
	app(list(cc.getcppflags(cfg)))
	app(list(cc.getdefines(cfg.defines)))
	app(list(cc.getincludedirs(cfg.includedirs)))
	app(list(cc.getquoteincludedirs(cfg.userincludedirs)))
	app(list(cc.getsystemincludedirs(cfg.systemincludedirs)))

	-- Custom build options
	app(list(cc.getcflags(cfg)))

	if path.iscppfile(file) then
		app(list(cc.getcxxflags(cfg)))
	end

	if path.isasmfile(file) then
		app(list(cfg.buildoptions))
		app(list(cfg.buildoptions_asm))
	elseif path.isobjcfile(file) then
		local opts = iif(path.iscfile(file), cfg.buildoptions_objc, cfg.buildoptions_objcpp)
		app(list(cc.getobjcflags(cfg)))
		app(list(cfg.buildoptions))
		app(list(opts))
	elseif path.iscfile(file) then
		app(list(cfg.buildoptions))
		app(list(cfg.buildoptions_c))
	else
		app(list(cfg.buildoptions))
		app(list(cfg.buildoptions_cpp))
	end

	-- Forced includes
	if cfg.pchheader and not cfg.flags.NoPCH then
		-- No need to worry about gch files or anything. Using the pch directly
		-- should have the same behavior for our purposes.
		app(" -include ")
		app(escape_cmdline_arg(cfg.pchheader))
	end

	for _, i in ipairs(cfg.forcedincludes) do
		app(" -include ")
		app(escape_cmdline_arg(i))
	end

	-- Input / Output
	local base = path.trimdots(path.removeext(file))..".o"
	local output = path.join(cfg.objectsdir, base)
	app(" -o ")
	app(escape_cmdline_arg(output))
	app(" -c ")
	app(escape_cmdline_arg(file))

	return cmdline
end

function jcdb.generate_config(prj, cfg, cc)
	table.sort(cfg.files)

	local directory = path.getabsolute(prj.location)

	for _, file in ipairs(cfg.files) do
		if path.iscppfile(file) or path.isasmfile(file) then
			_p('  { "directory": %s,', encode_string(directory))
			_p('    "command": %s,', encode_string(build_command(cfg, cc, file)))
			_p('    "file": %s },', encode_string(file))
		end
	end
end

function jcdb.generate_project(prj)
	local cc = premake.gettool(prj)
	local platforms = premake.filterplatforms(prj.solution, cc.platforms, "Native")

	for _, platform in ipairs(platforms) do
		for cfg in premake.eachconfig(prj, platform) do
			jcdb.generate_config(prj, cfg, cc)
		end
	end
end

function jcdb.generate(sln)
	for _, prj in ipairs(sln.projects) do
		jcdb.generate_project(prj)
	end

	-- Remove the last comma as JSON doesn't permit trailing commas
	io.captured = io.captured:gsub(",%s$", "")

	-- Wrap in brackets
	io.captured = "["..io.eol..io.captured..io.eol.."]"
end

