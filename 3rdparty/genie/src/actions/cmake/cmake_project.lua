--
-- _cmake.lua
-- Define the CMake action(s).
-- Copyright (c) 2015 Miodrag Milanovic
--

local cmake = premake.cmake
local tree = premake.tree

function cmake.files(prj)
	local tr = premake.project.buildsourcetree(prj)
	tree.traverse(tr, {
		onbranchenter = function(node, depth)
		end,
		onbranchexit = function(node, depth)
		end,
		onleaf = function(node, depth)
			_p(1, '../%s', node.cfg.name)
		end,
	}, true, 1)
end

function premake.cmake.project(prj)
	io.indent = "  "
	_p('cmake_minimum_required(VERSION 2.8.4)')
	_p('')
	_p('project(%s)', premake.esc(prj.name))
	_p('set(')
	_p('source_list')
	cmake.files(prj)
	_p(')')

	local platforms = premake.filterplatforms(prj.solution, premake[_OPTIONS.cc].platforms, "Native")
	for i = #platforms, 1, -1 do
		if premake.platforms[platforms[i]].iscrosscompiler then
			table.remove(platforms, i)
		end
	end 
	
	for _, platform in ipairs(platforms) do
		for cfg in premake.eachconfig(prj, platform) do
			for _,v in ipairs(cfg.includedirs) do
				_p('include_directories(../%s)', premake.esc(v))
			end
		end
	end

	if (prj.kind=='StaticLib') then
		_p('add_library(%s STATIC ${source_list})',premake.esc(prj.name))
	end
	if (prj.kind=='SharedLib') then
		_p('add_library(%s SHARED ${source_list})',premake.esc(prj.name))
	end
	if (prj.kind=='ConsoleApp') then
		_p('add_executable(%s ${source_list})',premake.esc(prj.name))
	end
	if (prj.kind=='WindowedApp') then
		_p('add_executable(%s ${source_list})',premake.esc(prj.name))
	end
end
