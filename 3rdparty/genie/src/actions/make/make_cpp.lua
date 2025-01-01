-- --
-- make_cpp.lua
-- Generate a C/C++ project makefile.
-- Copyright (c) 2002-2013 Jason Perkins and the Premake project
--

	premake.make.cpp = { }
	premake.make.override = { }
	premake.make.makefile_ignore = false

	local cpp = premake.make.cpp
	local make = premake.make

	function premake.make_cpp(prj)

		-- create a shortcut to the compiler interface
		local cc = premake.gettool(prj)

		-- build a list of supported target platforms that also includes a generic build
		local platforms = premake.filterplatforms(prj.solution, cc.platforms, "Native")

		-- output build configurations
		local action = premake.action.current()
		premake.gmake_cpp_header(prj, cc, platforms)
		premake.gmake_cpp_configs(prj, cc, platforms)
		table.sort(prj.allfiles)

		-- list object directories
		local objdirs = {}
		local additionalobjdirs = {}
		for _, file in ipairs(prj.allfiles) do
			if path.issourcefile(file) then
				objdirs[_MAKE.esc(path.getdirectory(path.trimdots(file)))] = 1
			end
		end

		for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
			for _, buildtask in ipairs(custombuildtask or {}) do
				additionalobjdirs[_MAKE.esc(path.getdirectory(path.getrelative(prj.location,buildtask[2])))] = 1
			end
		end

		_p('OBJDIRS := \\')
		_p('\t$(OBJDIR) \\')
		for dir, _ in iter.sortByKeys(objdirs) do
			_p('\t$(OBJDIR)/%s \\', dir)
		end
		for dir, _ in iter.sortByKeys(additionalobjdirs) do
			_p('\t%s \\', dir)
		end
		_p('')

		_p('RESOURCES := \\')
		for _, file in ipairs(prj.allfiles) do
			if path.isresourcefile(file) then
				_p('\t$(OBJDIR)/%s.res \\', _MAKE.esc(path.getbasename(file)))
			end
		end
		_p('')

		-- main build rule(s)
		_p('.PHONY: clean prebuild prelink')
		_p('')

		if os.is("MacOSX") and prj.kind == "WindowedApp" and not prj.options.SkipBundling then
			_p('all: $(OBJDIRS) $(TARGETDIR) prebuild prelink $(TARGET) $(dir $(TARGETDIR))PkgInfo $(dir $(TARGETDIR))Info.plist')
		else
			_p('all: $(OBJDIRS) $(TARGETDIR) prebuild prelink $(TARGET)')
		end
		_p('\t@:')
		_p('')

		if (prj.kind == "StaticLib" and prj.options.ArchiveSplit) then
			_p('define max_args')
			_p('\t$(eval _args:=)')
			_p('\t$(foreach obj,$3,$(eval _args+=$(obj))$(if $(word $2,$(_args)),$1 $(_args)$(EOL)$(eval _args:=)))')
			_p('\t$(if $(_args),$1 $(_args))')
			_p('endef')
			_p('')
			_p('define EOL')
			_p('')
			_p('')
			_p('endef')
			_p('')
		end

		-- target build rule
		_p('$(TARGET): $(GCH) $(OBJECTS) $(LIBDEPS) $(EXTERNAL_LIBS) $(RESOURCES) $(OBJRESP) $(LDRESP) | $(TARGETDIR) $(OBJDIRS)')

		if prj.kind == "StaticLib" then
			if prj.msgarchiving then
				_p('\t@echo ' .. prj.msgarchiving)
			else
				_p('\t@echo Archiving %s', prj.name)
			end
			if (not prj.archivesplit_size) then
				prj.archivesplit_size=200
			end
			if (not prj.options.ArchiveSplit) then
				_p('ifeq (posix,$(SHELLTYPE))')
				_p('\t$(SILENT) rm -f  $(TARGET)')
				_p('else')
				_p('\t$(SILENT) if exist $(subst /,\\\\,$(TARGET)) del $(subst /,\\\\,$(TARGET))')
				_p('endif')
				_p('\t$(SILENT) $(LINKCMD) $(LINKOBJS)' .. (os.is("MacOSX") and " 2>&1 > /dev/null | sed -e '/.o) has no symbols$$/d'" or ""))
			else
				_p('\t$(call RM,$(TARGET))')
				_p('\t@$(call max_args,$(LINKCMD),'.. prj.archivesplit_size ..',$(LINKOBJS))' .. (os.is("MacOSX") and " 2>&1 > /dev/null | sed -e '/.o) has no symbols$$/d'" or ""))
				_p('\t$(SILENT) $(LINKCMD_NDX)')
			end
		else
			if prj.msglinking then
				_p('\t@echo ' .. prj.msglinking)
			else
				_p('\t@echo Linking %s', prj.name)
			end
			_p('\t$(SILENT) $(LINKCMD)')
		end

		_p('\t$(POSTBUILDCMDS)')
		_p('')

		-- Create destination directories. Can't use $@ for this because it loses the
		-- escaping, causing issues with spaces and parenthesis
		_p('$(TARGETDIR):')
		premake.make_mkdirrule("$(TARGETDIR)")

		_p('$(OBJDIRS):')
		if (not prj.solution.messageskip) or (not table.contains(prj.solution.messageskip, "SkipCreatingMessage")) then
		_p('\t@echo Creating $(@)')
		end
		_p('\t-$(call MKDIR,$@)')
		_p('')

		-- Mac OS X specific targets
		if os.is("MacOSX") and prj.kind == "WindowedApp" and not prj.options.SkipBundling then
			_p('$(dir $(TARGETDIR))PkgInfo:')
			_p('$(dir $(TARGETDIR))Info.plist:')
			_p('')
		end

		-- clean target
		_p('clean:')
		if (not prj.solution.messageskip) or (not table.contains(prj.solution.messageskip, "SkipCleaningMessage")) then
			_p('\t@echo Cleaning %s', prj.name)
		end
		_p('ifeq (posix,$(SHELLTYPE))')
		_p('\t$(SILENT) rm -f  $(TARGET)')
		_p('\t$(SILENT) rm -rf $(OBJDIR)')
		_p('else')
		_p('\t$(SILENT) if exist $(subst /,\\\\,$(TARGET)) del $(subst /,\\\\,$(TARGET))')
		_p('\t$(SILENT) if exist $(subst /,\\\\,$(OBJDIR)) rmdir /s /q $(subst /,\\\\,$(OBJDIR))')
		_p('endif')
		_p('')

		-- custom build step targets
		_p('prebuild:')
		_p('\t$(PREBUILDCMDS)')
		_p('')

		_p('prelink:')
		_p('\t$(PRELINKCMDS)')
		_p('')

		-- precompiler header rule
		cpp.pchrules(prj)

		-- per-file build rules
		cpp.fileRules(prj, cc)

		-- per-dependency build rules
		cpp.dependencyRules(prj)

		for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
			for _, buildtask in ipairs(custombuildtask or {}) do
				local deps =  string.format("%s ",path.getrelative(prj.location,buildtask[1]))
				for _, depdata in ipairs(buildtask[3] or {}) do
					deps = deps .. string.format("%s ",path.getrelative(prj.location,depdata))
				end
				_p('%s: %s | $(TARGETDIR) $(OBJDIRS)'
					,path.getrelative(prj.location,buildtask[2])
					, deps
					)
				for _, cmdline in ipairs(buildtask[4] or {}) do
					local cmd = cmdline
					local num = 1
					for _, depdata in ipairs(buildtask[3] or {}) do
						cmd = string.gsub(cmd,"%$%(" .. num .."%)", string.format("%s ",path.getrelative(prj.location,depdata)))
						num = num + 1
					end
					cmd = string.gsub(cmd, "%$%(<%)", "$<")
					cmd = string.gsub(cmd, "%$%(@%)", "$@")

					_p('\t$(SILENT) %s',cmd)

				end
				_p('')
			end
		end

		-- include the dependencies, built by GCC (with the -MMD flag)
		_p('-include $(OBJECTS:%%.o=%%.d)')
		_p('ifneq (,$(PCH))')
			_p('  -include $(OBJDIR)/$(notdir $(PCH)).d')
			_p('  -include $(OBJDIR)/$(notdir $(PCH))_objc.d')
		_p('endif')
	end



--
-- Write the makefile header
--

	function premake.gmake_cpp_header(prj, cc, platforms)
		_p('# %s project makefile autogenerated by GENie', premake.action.current().shortname)
		_p('')

		_p('.SUFFIXES:') -- Delete the default suffix rules.
		_p('')

		-- set up the environment
		_p('ifndef config')
		_p('  config=%s', _MAKE.esc(premake.getconfigname(prj.solution.configurations[1], platforms[1], true)))
		_p('endif')
		_p('')

		_p('ifndef verbose')
		_p('  SILENT = @')
		_p('endif')
		_p('')

		-- identify the shell type
		_p('SHELLTYPE := msdos')
		_p('ifeq (,$(ComSpec)$(COMSPEC))')
		_p('  SHELLTYPE := posix')
		_p('endif')
		_p('ifeq (/bin,$(findstring /bin,$(SHELL)))')
		_p('  SHELLTYPE := posix')
		_p('endif')
		_p('ifeq (/bin,$(findstring /bin,$(MAKESHELL)))')
		_p('  SHELLTYPE := posix')
		_p('endif')
		_p('')

		_p('ifeq (posix,$(SHELLTYPE))')
		_p('  MKDIR = $(SILENT) mkdir -p "$(1)"')
		_p('  COPY  = $(SILENT) cp -fR "$(1)" "$(2)"')
		_p('  RM    = $(SILENT) rm -f "$(1)"')
		_p('else')
		_p('  MKDIR = $(SILENT) mkdir "$(subst /,\\\\,$(1))" 2> nul || exit 0')
		_p('  COPY  = $(SILENT) copy /Y "$(subst /,\\\\,$(1))" "$(subst /,\\\\,$(2))"')
		_p('  RM    = $(SILENT) del /F "$(subst /,\\\\,$(1))" 2> nul || exit 0')
		_p('endif')
		_p('')

		_p('CC  = %s', cc.cc)
		_p('CXX = %s', cc.cxx)
		_p('AR  = %s', cc.ar)
		_p('')

		_p('ifndef RESCOMP')
		_p('  ifdef WINDRES')
		_p('    RESCOMP = $(WINDRES)')
		_p('  else')
		_p('    RESCOMP = %s', cc.rc or 'windres')
		_p('  endif')
		_p('endif')
		_p('')

		if (not premake.make.makefile_ignore) then
			_p('MAKEFILE = %s', _MAKE.getmakefilename(prj, true))
			_p('')
		end
	end

--
-- Write a block of configuration settings.
--

	local function is_excluded(prj, cfg, file)
		if table.icontains(prj.excludes, file) then
			return true
		end

		if table.icontains(cfg.excludes, file) then
			return true
		end

		return false
	end

	function premake.gmake_cpp_configs(prj, cc, platforms)
		for _, platform in ipairs(platforms) do
			for cfg in premake.eachconfig(prj, platform) do
				premake.gmake_cpp_config(prj, cfg, cc)
			end
		end
	end

	function premake.gmake_cpp_config(prj, cfg, cc)
		_p('ifeq ($(config),%s)', _MAKE.esc(cfg.shortname))

		-- if this platform requires a special compiler or linker, list it here
		cpp.platformtools(cfg, cc)

		local targetDir = _MAKE.esc(cfg.buildtarget.directory)

		_p('  ' .. (table.contains(premake.make.override,"OBJDIR") and "override " or "") ..    'OBJDIR              = %s', _MAKE.esc(cfg.objectsdir))
		_p('  ' .. (table.contains(premake.make.override,"TARGETDIR") and "override " or "") .. 'TARGETDIR           = %s', iif(targetDir == "", ".", targetDir))
		_p('  ' .. (table.contains(premake.make.override,"TARGET") and "override " or "") ..    'TARGET              = $(TARGETDIR)/%s', _MAKE.esc(cfg.buildtarget.name))
		_p('  DEFINES            +=%s', make.list(_MAKE.escquote(cc.getdefines(cfg.defines))))

		local id  = make.list(cc.getincludedirs(cfg.includedirs));
		local uid = make.list(cc.getquoteincludedirs(cfg.userincludedirs))
		local sid = make.list(cc.getsystemincludedirs(cfg.systemincludedirs))

		if id ~= "" then
			_p('  INCLUDES           +=%s', id)
		end

		if uid ~= "" then
			_p('  INCLUDES           +=%s', uid)
		end

		if sid ~= "" then
			_p('  INCLUDES           +=%s', sid)
		end

		-- set up precompiled headers
		cpp.pchconfig(cfg)

		-- CPPFLAGS, CFLAGS, CXXFLAGS, and RESFLAGS
		cpp.flags(cfg, cc)

		-- write out libraries, linker flags, and the link command
		cpp.linker(prj, cfg, cc)

		table.sort(cfg.files)

		-- add objects for compilation, and remove any that are excluded per config.
		if cfg.flags.UseObjectResponseFile then
			_p('  OBJRESP             = $(OBJDIR)/%s_objects', prj.name)
		else
			_p('  OBJRESP             =')
		end
		_p('  OBJECTS := \\')
		for _, file in ipairs(cfg.files) do
			if path.issourcefile(file) then
				-- check if file is excluded.
				if not is_excluded(prj, cfg, file) then
					-- if not excluded, add it.
					_p('\t$(OBJDIR)/%s.o \\'
						, _MAKE.esc(path.trimdots(path.removeext(file)))
						)
				end
			end
		end
		_p('')

		_p('  define PREBUILDCMDS')
		if #cfg.prebuildcommands > 0 then
			_p('\t@echo Running pre-build commands')
			_p('\t%s', table.implode(cfg.prebuildcommands, "", "", "\n\t"))
		end
		_p('  endef')

		_p('  define PRELINKCMDS')
		if #cfg.prelinkcommands > 0 then
			_p('\t@echo Running pre-link commands')
			_p('\t%s', table.implode(cfg.prelinkcommands, "", "", "\n\t"))
		end
		_p('  endef')

		_p('  define POSTBUILDCMDS')
		if #cfg.postbuildcommands > 0 then
			_p('\t@echo Running post-build commands')
			_p('\t%s', table.implode(cfg.postbuildcommands, "", "", "\n\t"))
		end
		_p('  endef')

		-- write out config-level makesettings blocks
		make.settings(cfg, cc)

		_p('endif')
		_p('')
	end


--
-- Platform support
--

	function cpp.platformtools(cfg, cc)
		local platform = cc.platforms[cfg.platform]
		if platform.cc then
			_p('  CC         = %s', platform.cc)
		end
		if platform.cxx then
			_p('  CXX        = %s', platform.cxx)
		end
		if platform.ar then
			_p('  AR         = %s', platform.ar)
		end
	end


--
-- Configurations
--

	function cpp.flags(cfg, cc)

		if cfg.pchheader and not cfg.flags.NoPCH then
			_p('  FORCE_INCLUDE      += -include $(OBJDIR)/$(notdir $(PCH))')
			_p('  FORCE_INCLUDE_OBJC += -include $(OBJDIR)/$(notdir $(PCH))_objc')
		end

		if #cfg.forcedincludes > 0 then
			_p('  FORCE_INCLUDE      += -include %s'
					,_MAKE.esc(table.concat(cfg.forcedincludes, ";")))
		end

		_p('  ALL_CPPFLAGS       += $(CPPFLAGS) %s $(DEFINES) $(INCLUDES)', table.concat(cc.getcppflags(cfg), " "))

		_p('  ALL_ASMFLAGS       += $(ASMFLAGS) $(CFLAGS) $(ALL_CPPFLAGS) $(ARCH)%s', make.list(table.join(cc.getcflags(cfg), cfg.buildoptions, cfg.buildoptions_asm)))
		_p('  ALL_CFLAGS         += $(CFLAGS) $(ALL_CPPFLAGS) $(ARCH)%s', make.list(table.join(cc.getcflags(cfg), cfg.buildoptions, cfg.buildoptions_c)))
		_p('  ALL_CXXFLAGS       += $(CXXFLAGS) $(CFLAGS) $(ALL_CPPFLAGS) $(ARCH)%s', make.list(table.join(cc.getcflags(cfg), cc.getcxxflags(cfg), cfg.buildoptions, cfg.buildoptions_cpp)))
		_p('  ALL_OBJCFLAGS      += $(CFLAGS) $(ALL_CPPFLAGS) $(ARCH)%s', make.list(table.join(cc.getcflags(cfg), cc.getobjcflags(cfg), cfg.buildoptions, cfg.buildoptions_objc)))
		_p('  ALL_OBJCPPFLAGS    += $(CXXFLAGS) $(CFLAGS) $(ALL_CPPFLAGS) $(ARCH)%s', make.list(table.join(cc.getcflags(cfg), cc.getcxxflags(cfg), cc.getobjcflags(cfg), cfg.buildoptions, cfg.buildoptions_objcpp)))

		_p('  ALL_RESFLAGS       += $(RESFLAGS) $(DEFINES) $(INCLUDES)%s',
		        make.list(table.join(cc.getdefines(cfg.resdefines),
		                                cc.getincludedirs(cfg.resincludedirs), cfg.resoptions)))
	end


--
-- Linker settings, including the libraries to link, the linker flags,
-- and the linker command.
--

	function cpp.linker(prj, cfg, cc)
		local libdeps
		local lddeps

		if #cfg.wholearchive > 0 then
			libdeps = {}
			lddeps  = {}

			for _, linkcfg in ipairs(premake.getlinks(cfg, "siblings", "object")) do
				local linkpath = path.rebase(linkcfg.linktarget.fullpath, linkcfg.location, cfg.location)

				if table.icontains(cfg.wholearchive, linkcfg.project.name) then
					lddeps = table.join(lddeps, cc.wholearchive(linkpath))
				else
					table.insert(lddeps, linkpath)
				end

				table.insert(libdeps, linkpath)
			end

			libdeps = make.list(_MAKE.esc(libdeps))
			lddeps  = make.list(_MAKE.esc(lddeps))
		else
			libdeps = make.list(_MAKE.esc(premake.getlinks(cfg, "siblings", "fullpath")))
			lddeps  = libdeps
		end

		_p('  ALL_LDFLAGS        += $(LDFLAGS)%s', make.list(table.join(cc.getlibdirflags(cfg), cc.getldflags(cfg), cfg.linkoptions)))
		_p('  LIBDEPS            +=%s', libdeps)
		_p('  LDDEPS             +=%s', lddeps)

		if cfg.flags.UseLDResponseFile then
			_p('  LDRESP              = $(OBJDIR)/%s_libs', prj.name)
			_p('  LIBS               += @$(LDRESP)%s', make.list(cc.getlinkflags(cfg)))
		else
			_p('  LDRESP              =')
			_p('  LIBS               += $(LDDEPS)%s', make.list(cc.getlinkflags(cfg)))
		end

		_p('  EXTERNAL_LIBS      +=%s', make.list(cc.getlibfiles(cfg)))
		_p('  LINKOBJS            = %s', (cfg.flags.UseObjectResponseFile and "@$(OBJRESP)" or "$(OBJECTS)"))

		if cfg.kind == "StaticLib" then
			if (not prj.options.ArchiveSplit) then
				_p('  LINKCMD             = $(AR) %s $(TARGET)', make.list(cc.getarchiveflags(prj, cfg, false)))
			else
				_p('  LINKCMD             = $(AR) %s $(TARGET)', make.list(cc.getarchiveflags(prj, cfg, false)))
				_p('  LINKCMD_NDX         = $(AR) %s $(TARGET)', make.list(cc.getarchiveflags(prj, cfg, true)))
			end
		else
			local tool = iif(cfg.language == "C", "CC", "CXX")
			local startgroup = ''
			local endgroup = ''
			if (cfg.flags.LinkSupportCircularDependencies) then
				startgroup = '-Wl,--start-group '
				endgroup   = ' -Wl,--end-group'
			end
			_p('  LINKCMD             = $(%s) -o $(TARGET) $(LINKOBJS) $(RESOURCES) $(ARCH) $(ALL_LDFLAGS) %s$(LIBS)%s', tool, startgroup, endgroup)
		end
	end


--
-- Precompiled header support
--

	function cpp.pchconfig(cfg)

		-- If there is no header, or if PCH has been disabled, I can early out

		if not cfg.pchheader or cfg.flags.NoPCH then
			return
		end

		-- Visual Studio requires the PCH header to be specified in the same way
		-- it appears in the #include statements used in the source code; the PCH
		-- source actual handles the compilation of the header. GCC compiles the
		-- header file directly, and needs the file's actual file system path in
		-- order to locate it.

		-- To maximize the compatibility between the two approaches, see if I can
		-- locate the specified PCH header on one of the include file search paths
		-- and, if so, adjust the path automatically so the user doesn't have
		-- add a conditional configuration to the project script.

		local pch = cfg.pchheader
		for _, incdir in ipairs(cfg.includedirs) do

			-- convert this back to an absolute path for os.isfile()
			local abspath = path.getabsolute(path.join(cfg.project.location, incdir))

			local testname = path.join(abspath, pch)
			if os.isfile(testname) then
				pch = path.getrelative(cfg.location, testname)
				break
			end
		end

		_p('  PCH                 = %s', _MAKE.esc(pch))
		_p('  GCH                 = $(OBJDIR)/$(notdir $(PCH)).gch')
		_p('  GCH_OBJC            = $(OBJDIR)/$(notdir $(PCH))_objc.gch')

	end


	function cpp.pchrules(prj)
		_p('ifneq (,$(PCH))')
		_p('$(GCH): $(PCH) $(MAKEFILE) | $(OBJDIR)')
		if prj.msgprecompile then
			_p('\t@echo ' .. prj.msgprecompile)
		else
			_p('\t@echo $(notdir $<)')
		end

		local cmd = iif(prj.language == "C", "$(CC) $(ALL_CFLAGS) -x c-header", "$(CXX) $(ALL_CXXFLAGS) -x c++-header")
		_p('\t$(SILENT) %s $(DEFINES) $(INCLUDES) -o "$@" -c "$<"', cmd)

		_p('')

		_p('$(GCH_OBJC): $(PCH) $(MAKEFILE) | $(OBJDIR)')
		if prj.msgprecompile then
			_p('\t@echo ' .. prj.msgprecompile)
		else
			_p('\t@echo $(notdir $<)')
		end

		local cmd = iif(prj.language == "C", "$(CC) $(ALL_OBJCFLAGS) -x objective-c-header", "$(CXX) $(ALL_OBJCPPFLAGS) -x objective-c++-header")
		_p('\t$(SILENT) %s $(DEFINES) $(INCLUDES) -o "$@" -c "$<"', cmd)

		_p('endif')
		_p('')
	end


--
-- Build command for a single file.
--

	function cpp.fileRules(prj, cc)
		local platforms = premake.filterplatforms(prj.solution, cc.platforms, "Native")

		_p('ifneq (,$(OBJRESP))')
		_p('$(OBJRESP): $(OBJECTS) | $(TARGETDIR) $(OBJDIRS)')
		_p('\t$(SILENT) echo $^')
		_p('\t$(SILENT) echo $^ > $@')
		_p('endif')
		_p('')

		_p('ifneq (,$(LDRESP))')
		_p('$(LDRESP): $(LDDEPS) | $(TARGETDIR) $(OBJDIRS)')
		_p('\t$(SILENT) echo $^')
		_p('\t$(SILENT) echo $^ > $@')
		_p('endif')
		_p('')

		table.sort(prj.allfiles)
		for _, file in ipairs(prj.allfiles or {}) do
			if path.issourcefile(file) then
				if (path.isobjcfile(file)) then
					_p('$(OBJDIR)/%s.o: %s $(GCH_OBJC) $(MAKEFILE) | $(OBJDIR)/%s'
						, _MAKE.esc(path.trimdots(path.removeext(file)))
						, _MAKE.esc(file)
						, _MAKE.esc(path.getdirectory(path.trimdots(file)))
						)
				else
					_p('$(OBJDIR)/%s.o: %s $(GCH) $(MAKEFILE) | $(OBJDIR)/%s'
						, _MAKE.esc(path.trimdots(path.removeext(file)))
						, _MAKE.esc(file)
						, _MAKE.esc(path.getdirectory(path.trimdots(file)))
						)
				end
				if (path.isobjcfile(file) and prj.msgcompile_objc) then
					_p('\t@echo ' .. prj.msgcompile_objc)
				elseif prj.msgcompile then
					_p('\t@echo ' .. prj.msgcompile)
				else
					_p('\t@echo $(notdir $<)')
				end
				if (path.isobjcfile(file)) then
					if (path.iscfile(file)) then
						_p('\t$(SILENT) $(CXX) $(ALL_OBJCFLAGS) $(FORCE_INCLUDE_OBJC) -o "$@" -c "$<"')
					else
						_p('\t$(SILENT) $(CXX) $(ALL_OBJCPPFLAGS) $(FORCE_INCLUDE_OBJC) -o "$@" -c "$<"')
					end
				elseif (path.isasmfile(file)) then
					_p('\t$(SILENT) $(CC) $(ALL_ASMFLAGS) -o "$@" -c "$<"')
				else
					cpp.buildcommand(path.iscfile(file) and not prj.options.ForceCPP, "o")
				end
				for _, task in ipairs(prj.postcompiletasks or {}) do
					_p('\t$(SILENT) %s', task)
					_p('')
				end

				_p('')
			elseif (path.getextension(file) == ".rc") then
				_p('$(OBJDIR)/%s.res: %s', _MAKE.esc(path.getbasename(file)), _MAKE.esc(file))
				if prj.msgresource then
					_p('\t@echo ' .. prj.msgresource)
				else
					_p('\t@echo $(notdir $<)')
				end
				_p('\t$(SILENT) $(RESCOMP) $< -O coff -o "$@" $(ALL_RESFLAGS)')
				_p('')
			end
		end
	end

	function cpp.dependencyRules(prj)
		for _, dependency in ipairs(prj.dependency or {}) do
			for _, dep in ipairs(dependency or {}) do
				if (dep[3]==nil or dep[3]==false) then
					_p('$(OBJDIR)/%s.o: %s'
						, _MAKE.esc(path.trimdots(path.removeext(path.getrelative(prj.location, dep[1]))))
						, _MAKE.esc(path.getrelative(prj.location, dep[2]))
						)
				else
					_p('%s: %s'
						, _MAKE.esc(dep[1])
						, _MAKE.esc(path.getrelative(prj.location, dep[2]))
						)
				end
				_p('')
			end
		end
	end


	function cpp.buildcommand(iscfile, objext)
		local flags = iif(iscfile, '$(CC) $(ALL_CFLAGS)', '$(CXX) $(ALL_CXXFLAGS)')
		_p('\t$(SILENT) %s $(FORCE_INCLUDE) -o "$@" -c "$<"', flags, objext)
	end
