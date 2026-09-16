--
-- _cmake.lua
-- Define the CMake action(s).
-- Copyright (c) 2015 Miodrag Milanovic
-- Modifications and additions in 2017 by Maurizio Petrarota
--

local cmake = premake.cmake
local tree = premake.tree

-- Split multi-word flags (e.g. "-Wall -Wextra") into individual items
local function splitflags(flags)
	local result = {}
	for _, flag in ipairs(flags) do
		for word in flag:gmatch("%S+") do
			table.insert(result, word)
		end
	end
	return result
end

--
-- MSVC flag translation tables (parallel to gcc.lua tables)
--

local msvc_cflags =
{
    EnableSSE        = "/arch:SSE",
    EnableSSE2       = "/arch:SSE2",
    EnableAVX        = "/arch:AVX",
    EnableAVX2       = "/arch:AVX2",
    PedanticWarnings = "/W4",
    ExtraWarnings    = "/W4",
    FatalWarnings    = "/WX",
    FloatFast        = "/fp:fast",
    FloatStrict      = "/fp:strict",
    Optimize         = "/O2",
    OptimizeSize     = "/O1",
    OptimizeSpeed    = "/Ox",
    Symbols          = "/Zi",
}

local msvc_cxxflags =
{
    Cpp14        = "/std:c++14",
    Cpp17        = "/std:c++17",
    Cpp20        = "/std:c++20",
    CppLatest    = "/std:c++latest",
    NoRTTI       = "/GR-",
    NoBufferSecurityCheck = "/GS-",
}

function cmake.getmsvc_cflags(cfg)
    return table.translate(cfg.flags, msvc_cflags)
end

function cmake.getmsvc_cxxflags(cfg)
    local result = table.translate(cfg.flags, msvc_cxxflags)
    if cfg.flags.NoExceptions then
        table.insert(result, "/EHs-c-")
    end
    -- Always enable conformance modes for MSVC
    table.insert(result, "/Zc:__cplusplus")
    table.insert(result, "/Zc:preprocessor")
    return result
end

-- MSVC_VERSION ranges for version-specific CMake conditions
local msvc_version_ranges = {
    vs2013 = { 1800, 1900 },
    vs2015 = { 1900, 1910 },
    vs2017 = { 1910, 1920 },
    vs2019 = { 1920, 1930 },
    vs2022 = { 1930, 1940 },
    vs2026 = { 1940, 1950 },
}

local function msvc_condition(matched_keyword)
    local range = msvc_version_ranges[matched_keyword]
    if range then
        return string.format("MSVC AND MSVC_VERSION GREATER_EQUAL %d AND MSVC_VERSION LESS %d", range[1], range[2])
    end
    return "MSVC"
end

-- Check if a keyword matches the "vs" action pattern.
-- Keywords are stored as Lua patterns after path.wildcards() and :lower() transformation.
-- Accepts transformed forms of: vs*, vs20*, vs2022, vs2019
-- Rejects transformed forms of: vs*-clang, vs*-orbis (contain %-suffix)
local function is_vs_action_keyword(keyword)
    -- Must start with "vs", then only digits and wildcard patterns [^/]* or .*
    -- Must NOT contain %-  (transformed from "-", indicates user-defined variant)
    if not keyword:match("^vs") then
        return false
    end
    if keyword:find("%%%-") then
        return false
    end
    -- After "vs", only digits, wildcard chars [^/]*, .*, or Lua pattern escapes
    local rest = keyword:sub(3)
    -- Strip known wildcard patterns
    rest = rest:gsub("%[%^/%]%*", "")  -- [^/]* from single *
    rest = rest:gsub("%.%*", "")        -- .* from **
    -- What remains should be only digits (or empty)
    return rest:match("^%d*$") ~= nil
end

-- Extract settings from project blocks gated behind action-name keywords.
-- These blocks don't get baked when _ACTION="cmake", so we manually scan for them.
-- match_fn: function(keyword) -> bool, tests if a keyword matches the action family
-- condition_fn: maps matched keyword to a CMake condition string
-- Returns an array of { condition = "...", settings = { includedirs, defines, ... } }
-- grouped by condition, generic conditions first.
function cmake.getplatform_settings(prj, cfgname, match_fn, condition_fn)
    local groups = {}

    -- Get the raw project object (prj may be a baked config)
    local rawprj = prj.project or prj

    -- Scan both solution-level and project-level blocks
    local block_sources = {}
    if rawprj.solution and rawprj.solution.blocks then
        table.insert(block_sources, rawprj.solution.blocks)
    end
    if rawprj.blocks then
        table.insert(block_sources, rawprj.blocks)
    end
    for _, blocks in ipairs(block_sources) do
        for _, blk in ipairs(blocks) do
            local matched_alt = nil
            local has_config_keyword = false
            local config_ok = true

            for _, kw in ipairs(blk.keywords) do
                -- Check each alternative in "or"-separated keywords
                for _, alt in ipairs(kw:explode(" or ")) do
                    alt = alt:match("^%s*(.-)%s*$") -- trim
                    if match_fn(alt) then
                        matched_alt = alt
                    end
                    -- Check if this keyword is a config filter
                    if cfgname and cfgname ~= "" then
                        if alt:lower() == cfgname:lower() then
                            has_config_keyword = true
                        end
                    end
                end
            end

            -- If cfgname specified, block must either match it or not mention any known config
            if cfgname and cfgname ~= "" then
                local block_has_config_filter = false
                for _, kw in ipairs(blk.keywords) do
                    for _, alt in ipairs(kw:explode(" or ")) do
                        alt = alt:match("^%s*(.-)%s*$")
                        local al = alt:lower()
                        if al == "debug" or al == "release" or al == "profile" or al == "development" then
                            block_has_config_filter = true
                        end
                    end
                end
                if block_has_config_filter and not has_config_keyword then
                    config_ok = false
                end
            end

            if matched_alt and config_ok then
                local condition = condition_fn(matched_alt)
                if not groups[condition] then
                    groups[condition] = {
                        includedirs = {},
                        defines = {},
                        buildoptions = {},
                        linkoptions = {},
                        links = {},
                    }
                end
                local g = groups[condition]
                for _, v in ipairs(blk.includedirs or {}) do
                    if path.isabsolute(v) then
                        v = path.getrelative(rawprj.location, v)
                    end
                    if not table.contains(g.includedirs, v) then
                        table.insert(g.includedirs, v)
                    end
                end
                for _, v in ipairs(blk.defines or {}) do
                    if not table.contains(g.defines, v) then
                        table.insert(g.defines, v)
                    end
                end
                for _, v in ipairs(blk.buildoptions or {}) do
                    if not table.contains(g.buildoptions, v) then
                        table.insert(g.buildoptions, v)
                    end
                end
                for _, v in ipairs(blk.linkoptions or {}) do
                    if not table.contains(g.linkoptions, v) then
                        table.insert(g.linkoptions, v)
                    end
                end
                for _, v in ipairs(blk.links or {}) do
                    if not table.contains(g.links, v) then
                        table.insert(g.links, v)
                    end
                end
            end
        end
    end

    -- Convert to sorted array (generic conditions first, then specific)
    local result = {}
    for condition, settings in pairs(groups) do
        table.insert(result, { condition = condition, settings = settings })
    end
    table.sort(result, function(a, b) return #a.condition < #b.condition end)
    return result
end

-- Flatten grouped results into a single settings table
local function flatten_platform_groups(groups)
    local result = {
        includedirs = {},
        defines = {},
        buildoptions = {},
        linkoptions = {},
        links = {},
    }
    for _, group in ipairs(groups) do
        for _, v in ipairs(group.settings.includedirs) do
            if not table.contains(result.includedirs, v) then table.insert(result.includedirs, v) end
        end
        for _, v in ipairs(group.settings.defines) do
            if not table.contains(result.defines, v) then table.insert(result.defines, v) end
        end
        for _, v in ipairs(group.settings.buildoptions) do
            if not table.contains(result.buildoptions, v) then table.insert(result.buildoptions, v) end
        end
        for _, v in ipairs(group.settings.linkoptions) do
            if not table.contains(result.linkoptions, v) then table.insert(result.linkoptions, v) end
        end
        for _, v in ipairs(group.settings.links) do
            if not table.contains(result.links, v) then table.insert(result.links, v) end
        end
    end
    return result
end

function cmake.getmsvc_settings(prj, cfgname)
    return cmake.getplatform_settings(prj, cfgname, is_vs_action_keyword, msvc_condition)
end


local function is_excluded(prj, cfg, file)
    if table.icontains(prj.excludes, file) then
        return true
    end

    if table.icontains(cfg.excludes, file) then
        return true
    end

    return false
end

function cmake.excludedFiles(prj, cfg, src)
    for _, v in ipairs(src) do
        if (is_excluded(prj, cfg, v)) then
            _p(1, 'list(REMOVE_ITEM source_list ../../%s)', v)
        end

    end
end

function cmake.commonExcludedFiles(prj, configurations, src)
    for _, v in ipairs(src) do
        local excludedCount = 0
        for _, cfg in ipairs(configurations) do
            if is_excluded(prj, cfg, v) then
                excludedCount = excludedCount + 1
            end
        end
        if excludedCount == #configurations then
            _p('list(REMOVE_ITEM source_list ../../%s)', v)
        end
    end
end

function cmake.list(value)
    if #value > 0 then
        return " " .. table.concat(value, " ")
    else
        return ""
    end
end

function cmake.listWrapped(value, prefix, postfix)
    if #value > 0 then
        return prefix .. table.concat(value, postfix .. prefix) .. postfix
    else
        return ""
    end
end

function cmake.files(prj)
    local ret = {}
    local tr = premake.project.buildsourcetree(prj)
    tree.traverse(tr, {
        onbranchenter = function(node, depth)
        end,
        onbranchexit = function(node, depth)
        end,
        onleaf = function(node, depth)
            assert(node, "unexpected empty node")
            if node.cfg then
                table.insert(ret, node.cfg.name)
                _p(1, '../../%s', node.cfg.name)
            end
        end,
    }, true, 1)

    return ret
end

function cmake.header(prj)
    _p('# %s project autogenerated by GENie', premake.action.current().shortname)
    _p('# https://github.com/bkaradzic/GENie')
    _p('cmake_minimum_required(VERSION %s)', premake.cmake.cmake_minimum_version)
    if os.is("windows") then
        -- Add support for CMP0091, see https://cmake.org/cmake/help/latest/policy/CMP0091.html
        _p('cmake_policy(SET CMP0091 NEW)')
    end
    _p('')
    _p('project(%s)', premake.esc(prj.name))
end

function cmake.customtasks(prj)
    local dirs = {}
    local tasks = {}
    for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
        for _, buildtask in ipairs(custombuildtask or {}) do
            table.insert(tasks, buildtask)
            local d = string.format("${CMAKE_CURRENT_SOURCE_DIR}/../../%s", path.getdirectory(path.getrelative(prj.location, buildtask[2])))
            if not table.contains(dirs, d) then
                table.insert(dirs, d)
                _p('file(MAKE_DIRECTORY \"%s\")', d)
            end
        end
    end
    _p('')

    for _, buildtask in ipairs(tasks) do
        local deps = string.format("${CMAKE_CURRENT_SOURCE_DIR}/../../%s ", path.getrelative(prj.location, buildtask[1]))
        local outputs = string.format("${CMAKE_CURRENT_SOURCE_DIR}/../../%s ", path.getrelative(prj.location, buildtask[2]))
        local msg = ""

        for _, depdata in ipairs(buildtask[3] or {}) do
            deps = deps .. string.format("${CMAKE_CURRENT_SOURCE_DIR}/../../%s ", path.getrelative(prj.location, depdata))
        end

        _p('add_custom_command(')
        _p(1, 'OUTPUT %s', outputs)
        _p(1, 'DEPENDS %s', deps)

        for _, cmdline in ipairs(buildtask[4] or {}) do
            if (cmdline:sub(1, 1) ~= "@") then
                local cmd = cmdline
                local num = 1
                for _, depdata in ipairs(buildtask[3] or {}) do
                    cmd = string.gsub(cmd, "%$%(" .. num .. "%)", string.format("${CMAKE_CURRENT_SOURCE_DIR}/../../%s ", path.getrelative(prj.location, depdata)))
                    num = num + 1
                end

                cmd = string.gsub(cmd, "%$%(<%)", string.format("${CMAKE_CURRENT_SOURCE_DIR}/../../%s ", path.getrelative(prj.location, buildtask[1])))
                cmd = string.gsub(cmd, "%$%(@%)", outputs)

                _p(1, 'COMMAND %s', cmd)
            else
                msg = cmdline
            end
        end
        _p(1, 'COMMENT \"%s\"', msg)
        _p(')')
        _p('')
    end
end

function cmake.depRules(prj)
    local maintable = {}
    for _, dependency in ipairs(prj.dependency) do
        for _, dep in ipairs(dependency) do
            if path.issourcefile(dep[1]) then
                local dep1 = premake.esc(path.getrelative(prj.location, dep[1]))
                local dep2 = premake.esc(path.getrelative(prj.location, dep[2]))
                if not maintable[dep1] then maintable[dep1] = {} end
                table.insert(maintable[dep1], dep2)
            end
        end
    end

    for key, _ in pairs(maintable) do
        local deplist = {}
        local depsname = string.format('%s_deps', path.getname(key))

        for _, d2 in pairs(maintable[key]) do
            table.insert(deplist, d2)
        end
        _p('set(')
        _p(1, depsname)
        for _, v in pairs(deplist) do
            _p(1, '${CMAKE_CURRENT_SOURCE_DIR}/../../%s', v)
        end
        _p(')')
        _p('')
        _p('set_source_files_properties(')
        _p(1, '"${CMAKE_CURRENT_SOURCE_DIR}/../../%s"', key)
        _p(1, 'PROPERTIES OBJECT_DEPENDS \"${%s}\"', depsname)
        _p(')')
        _p('')
    end
end

function cmake.commonRules(conf, kind)
    local Dupes = {}
    local t2 = {}
    for _, cfg in ipairs(conf) do
        local cfgd = iif(kind == "includes", cfg.includedirs, cfg.defines)
        for _, v in ipairs(cfgd) do
            if(t2[v] == #conf - 1) then
                table.insert(Dupes, v)
            end
            if not t2[v] then
                t2[v] = 1
            else
                t2[v] = t2[v] + 1
            end
        end
    end
    return Dupes
end

function cmake.grouppath(prj)
    if prj.group == nil then
        return nil
    end
    local parts = {}
    local grp = prj.group
    while grp ~= nil do
        table.insert(parts, 1, grp.name)
        grp = grp.parent
    end
    return table.concat(parts, "/")
end

function cmake.removeCrosscompiler(platforms)
    for i = #platforms, 1, -1 do
        if premake.platforms[platforms[i]].iscrosscompiler then
            table.remove(platforms, i)
        end
    end
end

function cmake.project(prj)
    io.indent = "  "
    cmake.header(prj)
    _p('set(')
    _p('source_list')
    local source_files = cmake.files(prj)
    _p(')')
    _p('')

    local nativeplatform = iif(os.is64bit(), "x64", "x32")
    local cc = premake.gettool(prj)
    local platforms = premake.filterplatforms(prj.solution, cc.platforms, "Native")

    cmake.removeCrosscompiler(platforms)

    local configurations = {}

    for _, platform in ipairs(platforms) do
        for cfg in premake.eachconfig(prj, platform) do
            -- TODO: Extend support for 32-bit targets on 64-bit hosts
            if cfg.platform == nativeplatform then
                table.insert(configurations, cfg)
            end
        end
    end

    local commonIncludes = cmake.commonRules(configurations, "includes")
    local commonDefines = cmake.commonRules(configurations, "defines")

    _p('')

    -- file exclusions (unconditional for files excluded in all configs)
    cmake.commonExcludedFiles(prj, configurations, source_files)



    -- force CPP if needed
    if (prj.options.ForceCPP) then
        _p('set_source_files_properties(${source_list} PROPERTIES LANGUAGE CXX)')
    end

    -- add custom tasks
    cmake.customtasks(prj)

    -- per-dependency build rules
    cmake.depRules(prj)

    local targetname = premake.esc(prj.name)

    if (prj.kind == 'StaticLib') then
        _p('add_library(%s STATIC ${source_list})', targetname)
    end

    if (prj.kind == 'SharedLib') then
        _p('add_library(%s SHARED ${source_list})', targetname)
    end

    if (prj.kind == 'ConsoleApp' or prj.kind == 'WindowedApp') then
        _p('add_executable(%s ${source_list})', targetname)

        local linklibs = {}
        for _, cfg in ipairs(configurations) do
            for _, link in ipairs(premake.getlinks(cfg, "siblings", "object")) do
                local lname = link.project.name
                if not table.contains(linklibs, lname) then
                    table.insert(linklibs, lname)
                end
            end
        end

        --  sibling project link dependencies (compiler-agnostic)
        if #linklibs > 0 then
            _p('target_link_libraries(%s PRIVATE%s)', targetname, cmake.list(premake.esc(linklibs)))
        end

        -- GCC/Clang-specific link flags
        local libdirs = cmake.listWrapped(premake.esc(premake.getlinks(configurations[1], "all", "directory")), " -L\"../../", "\"")
        local gcc_linkflags = table.join(configurations[1].linkoptions, cc.getldflags(configurations[1]), cc.getlinkflags(configurations[1]))
        if #libdirs > 0 or #gcc_linkflags > 0 then
            _p('if(CMAKE_CXX_COMPILER_ID MATCHES \"GNU|Clang\")')
            _p(1, 'target_link_libraries(%s PRIVATE%s%s)', targetname, libdirs, cmake.list(gcc_linkflags))
            _p('endif()')
        end
    end

    -- common includes (target-scoped)
    for _, v in ipairs(commonIncludes) do
        _p('target_include_directories(%s PRIVATE ../../%s)', targetname, v)
    end

    -- common defines (target-scoped)
    for _, v in ipairs(commonDefines) do
        _p('target_compile_definitions(%s PRIVATE %s)', targetname, v)
    end

    -- per-config GCC/Clang compiler flags (target-scoped)
    for _, cfg in ipairs(configurations) do
        local cxx_flags = splitflags(table.join(cc.getcflags(cfg), cc.getcxxflags(cfg), cfg.buildoptions, cfg.buildoptions_cpp))
        local c_flags = splitflags(table.join(cc.getcflags(cfg), cfg.buildoptions, cfg.buildoptions_c))
        if #cxx_flags > 0 then
            _p('target_compile_options(%s PRIVATE $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU,Clang>,$<CONFIG:%s>>:%s>)', targetname, cfg.name, table.concat(cxx_flags, ';'))
        end
        if #c_flags > 0 then
            _p('target_compile_options(%s PRIVATE $<$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:GNU,Clang>,$<CONFIG:%s>>:%s>)', targetname, cfg.name, table.concat(c_flags, ';'))
        end
    end

    -- per-config MSVC compile options (using target_compile_options for multi-config generator support)
    for _, cfg in ipairs(configurations) do
        local msvc_c = cmake.getmsvc_cflags(cfg)
        local msvc_cxx = cmake.getmsvc_cxxflags(cfg)
        -- also get MSVC-specific buildoptions from vs*-gated config blocks (flattened across versions)
        local msvc_settings = flatten_platform_groups(cmake.getmsvc_settings(prj, cfg.name))
        -- filter out buildoptions already covered by the flag tables
        local all_flags = table.join(msvc_c, msvc_cxx)
        local msvc_buildopts = {}
        for _, v in ipairs(msvc_settings.buildoptions) do
            if not table.contains(all_flags, v) then
                table.insert(msvc_buildopts, v)
            end
        end

        if #msvc_c > 0 or #msvc_cxx > 0 or #msvc_buildopts > 0 then
            _p('target_compile_options(%s PRIVATE', targetname)
            if #msvc_cxx > 0 then
                _p(1, '$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>,$<CONFIG:%s>>:%s>', cfg.name, table.concat(msvc_cxx, ';'))
            end
            local msvc_common = table.join(msvc_c, msvc_buildopts)
            if #msvc_common > 0 then
                _p(1, '$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:%s>>:%s>', cfg.name, table.concat(msvc_common, ';'))
            end
            _p(')')
        end
    end

    -- MSVC-specific includes, defines, links, and link options from vs*-gated blocks
    -- Version-specific settings (e.g. vs2019-only) get their own MSVC_VERSION conditions
    local msvc_groups = cmake.getmsvc_settings(prj)
    for _, group in ipairs(msvc_groups) do
        local s = group.settings
        if #s.includedirs > 0 or #s.defines > 0 or #s.links > 0 or #s.buildoptions > 0 or #s.linkoptions > 0 then
            _p('if(%s)', group.condition)
            for _, v in ipairs(s.includedirs) do
                _p(1, 'target_include_directories(%s PRIVATE ../../%s)', targetname, v)
            end
            for _, v in ipairs(s.defines) do
                _p(1, 'target_compile_definitions(%s PRIVATE %s)', targetname, v)
            end
            if #s.buildoptions > 0 then
                _p(1, 'target_compile_options(%s PRIVATE %s)', targetname, table.concat(splitflags(s.buildoptions), ';'))
            end
            if #s.links > 0 then
                _p(1, 'target_link_libraries(%s PRIVATE%s)', targetname, cmake.list(s.links))
            end
            if #s.linkoptions > 0 then
                _p(1, 'target_link_options(%s PRIVATE%s)', targetname, cmake.list(s.linkoptions))
            end
            _p('endif()')
        end
    end

    -- project group (solution folder)
    local folder = cmake.grouppath(prj)
    if folder then
        _p('set_target_properties(%s PROPERTIES FOLDER "%s")', targetname, folder)
    end

    -- per-config includes and defines
    for _, cfg in ipairs(configurations) do
        for _, v in ipairs(cfg.includedirs) do
            if not table.icontains(commonIncludes, v) then
                _p('target_include_directories(%s PRIVATE $<$<CONFIG:%s>:../../%s>)', targetname, cfg.name, v)
            end
        end
        for _, v in ipairs(cfg.defines) do
            if not table.icontains(commonDefines, v) then
                _p('target_compile_definitions(%s PRIVATE $<$<CONFIG:%s>:%s>)', targetname, cfg.name, v)
            end
        end
    end

    -- debugdir → VS_DEBUGGER_WORKING_DIRECTORY (for executables)
    if prj.kind == 'ConsoleApp' or prj.kind == 'WindowedApp' then
        local debugdirs = {}
        for _, cfg in ipairs(configurations) do
            if cfg.debugdir then
                debugdirs[cfg.name] = cfg.debugdir
            end
        end
        -- Check if all configs share the same debugdir
        local common_debugdir = nil
        local all_same = true
        for _, dir in pairs(debugdirs) do
            if common_debugdir == nil then
                common_debugdir = dir
            elseif dir ~= common_debugdir then
                all_same = false
                break
            end
        end
        if common_debugdir then
            if all_same then
                _p('set_target_properties(%s PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../%s")', targetname, common_debugdir)
            else
                for cfgname, dir in pairs(debugdirs) do
                    _p('set_target_properties(%s PROPERTIES $<$<CONFIG:%s>:VS_DEBUGGER_WORKING_DIRECTORY> "${CMAKE_CURRENT_SOURCE_DIR}/../../%s")', targetname, cfgname, dir)
                end
            end
        end
    end

    _p('')
end