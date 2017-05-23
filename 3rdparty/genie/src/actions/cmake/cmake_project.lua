--
-- _cmake.lua
-- Define the CMake action(s).
-- Copyright (c) 2015 Miodrag Milanovic
-- Modifications and additions in 2017 by Maurizio Petrarota
--

local cmake = premake.cmake
local tree = premake.tree

function cmake.list(value)
    if #value > 0 then
        return " " .. table.concat(value, " ")
    else
        return ""
    end
end

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

function cmake.customtasks(prj)
    local dirs = {}
    local tasks = {}
    for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
        for _, buildtask in ipairs(custombuildtask or {}) do
            table.insert(tasks, buildtask)
            local d = string.format("${CMAKE_CURRENT_SOURCE_DIR}/../%s", path.getdirectory(path.getrelative(prj.location, buildtask[2])))
            if not table.contains(dirs, d) then
                table.insert(dirs, d)
            end
        end
    end

    for _, v in ipairs(dirs) do
        _p('file(MAKE_DIRECTORY \"%s\")', v)
    end
    _p('')

    for _, buildtask in ipairs(tasks) do
        local deps = string.format("${CMAKE_CURRENT_SOURCE_DIR}/../%s ", path.getrelative(prj.location, buildtask[1]))
        local outputs = string.format("${CMAKE_CURRENT_SOURCE_DIR}/../%s ", path.getrelative(prj.location, buildtask[2]))
        local msg = ""

        for _, depdata in ipairs(buildtask[3] or {}) do
            deps = deps .. string.format("${CMAKE_CURRENT_SOURCE_DIR}/../%s ", path.getrelative(prj.location, depdata))
        end

        _p('add_custom_command(')
        _p(1, 'OUTPUT %s', outputs)
        _p(1, 'DEPENDS %s', deps)

        for _, cmdline in ipairs(buildtask[4] or {}) do
            if (cmdline:sub(1, 1) ~= "@") then
                local cmd = cmdline
                local num = 1
                for _, depdata in ipairs(buildtask[3] or {}) do
                    cmd = string.gsub(cmd, "%$%(" .. num .. "%)", string.format("${CMAKE_CURRENT_SOURCE_DIR}/../%s ", path.getrelative(prj.location, depdata)))
                    num = num + 1
                end

                cmd = string.gsub(cmd, "%$%(<%)", string.format("${CMAKE_CURRENT_SOURCE_DIR}/../%s ", path.getrelative(prj.location, buildtask[1])))
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

function cmake.dependencyRules(prj)
    local customdeps = {}
    for key, dependency in ipairs(prj.dependency or {}) do
        _p('add_custom_target(')
        local customname = string.format("customdep_%s_%s", premake.esc(prj.name), key)
        table.insert(customdeps, customname)
        _p(1, '%s', customname)
        for _, dep in ipairs(dependency or {}) do
            _p(1, 'DEPENDS \"${CMAKE_CURRENT_SOURCE_DIR}/../%s\"', premake.esc(path.getrelative(prj.location, dep[2])))
        end
        _p(')')
        _p('')
    end
    return customdeps
end

function cmake.includeRules(cfg)
    for _, v in ipairs(cfg.includedirs) do
        _p(1, 'include_directories(../%s)', premake.esc(v))
    end
end

function cmake.definesRules(cfg)
    for _, v in ipairs(cfg.defines) do
        _p(1, 'add_definitions(-D%s)', v)
    end
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
    _p('cmake_minimum_required(VERSION 2.8.4)')
    _p('')
    _p('project(%s)', premake.esc(prj.name))
    _p('set(')
    _p('source_list')
    cmake.files(prj)
    _p(')')
    _p('')

    local nativeplatform = iif(os.is64bit(), "x64", "x32")
    local cc = premake.gettool(prj)
    local platforms = premake.filterplatforms(prj.solution, cc.platforms, "Native")
    local configurations = {}

    cmake.removeCrosscompiler(platforms)


    -- TODO: Reduce the length of the generated code by aggregating common parts.
    for _, platform in ipairs(platforms) do
        for cfg in premake.eachconfig(prj, platform) do

            -- TODO: Extend support for 32-bit targets on 64-bit hosts
            if cfg.platform == nativeplatform then
                table.insert(configurations, cfg)
                _p('if(CMAKE_BUILD_TYPE MATCHES \"%s\")', cfg.name)

                -- add includes directories
                cmake.includeRules(cfg)

                -- add build defines
                cmake.definesRules(cfg)

                -- set CXX flags
                _p(1, 'set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} %s\")', cmake.list(table.join(cc.getcppflags(cfg), cc.getcflags(cfg), cc.getcxxflags(cfg), cfg.buildoptions, cfg.buildoptions_cpp)))

                -- set C flags
                _p(1, 'set(CMAKE_C_FLAGS \"${CMAKE_C_FLAGS} %s\")', cmake.list(table.join(cc.getcppflags(cfg), cc.getcflags(cfg), cfg.buildoptions, cfg.buildoptions_c)))

                _p('endif()')
                _p('')
            end
        end
    end

    -- force CPP if needed
    if (prj.options.ForceCPP) then
        _p('set_source_files_properties(${source_list} PROPERTIES LANGUAGE CXX)')
    end

    -- add custom tasks
    cmake.customtasks(prj)

    -- per-dependency build rules
    local customdeps = cmake.dependencyRules(prj)

    for _, cfg in ipairs(configurations) do
        _p('if(CMAKE_BUILD_TYPE MATCHES \"%s\")', cfg.name)

        if (prj.kind == 'StaticLib') then
            _p(1, 'add_library(%s STATIC ${source_list})', premake.esc(cfg.buildtarget.basename))
        end

        if (prj.kind == 'SharedLib') then
            _p(1, 'add_library(%s SHARED ${source_list})', premake.esc(cfg.buildtarget.basename))
        end
        if (prj.kind == 'ConsoleApp' or prj.kind == 'WindowedApp') then
            _p(1, 'add_executable(%s ${source_list})', premake.esc(cfg.buildtarget.basename))
            _p(1, 'target_link_libraries(%s%s%s)', premake.esc(cfg.buildtarget.basename), cmake.list(premake.esc(premake.getlinks(cfg, "siblings", "basename"))), cmake.list(cc.getlinkflags(cfg)))
        end

        for _, v in ipairs(customdeps) do
            _p(1, 'add_dependencies(%s %s)', premake.esc(cfg.buildtarget.basename), v)
        end
        _p('endif()')
        _p('')
    end
end
