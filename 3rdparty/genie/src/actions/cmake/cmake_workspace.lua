--
-- _cmake.lua
-- Define the CMake action(s).
-- Copyright (c) 2015 Miodrag Milanovic
--

function premake.cmake.workspace(sln)
	_p('cmake_minimum_required(VERSION 2.8.4)')
	_p('')
	for i,prj in ipairs(sln.projects) do
		local name = premake.esc(prj.name)
		_p('add_subdirectory(%s)', name)
	end
end
