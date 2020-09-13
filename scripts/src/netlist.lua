-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   netlist.lua
--
--   Rules for building netlist cores
--
---------------------------------------------------------------------------

project "netlist"
	uuid "665ef8ac-2a4c-4c3e-a05f-fd1e5db11de9"
	kind (LIBTYPE)

	if _OPTIONS["targetos"]=="windows" then
		configuration { "mingw* or vs*" }
		defines {
			"UNICODE",
			"_UNICODE",
			"_WIN32_WINNT=0x0501",
			"WIN32_LEAN_AND_MEAN",
			"NOMINMAX",
		}
	end

	addprojectflags()

	defines {
		"__STDC_CONSTANT_MACROS",
		"NL_USE_ACADEMIC_SOLVERS=0",
	}

	includedirs {
	--	MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/netlist",
	}

	files {
		MAME_DIR .. "src/lib/netlist/nl_base.cpp",
		MAME_DIR .. "src/lib/netlist/nl_base.h",
		MAME_DIR .. "src/lib/netlist/nl_config.h",
		MAME_DIR .. "src/lib/netlist/nl_errstr.h",
		MAME_DIR .. "src/lib/netlist/nl_dice_compat.h",
		MAME_DIR .. "src/lib/netlist/nl_factory.cpp",
		MAME_DIR .. "src/lib/netlist/nl_factory.h",
		MAME_DIR .. "src/lib/netlist/nl_interface.h",
		MAME_DIR .. "src/lib/netlist/nl_parser.cpp",
		MAME_DIR .. "src/lib/netlist/nl_parser.h",
		MAME_DIR .. "src/lib/netlist/nl_setup.cpp",
		MAME_DIR .. "src/lib/netlist/nl_setup.h",
		MAME_DIR .. "src/lib/netlist/nl_types.h",
		MAME_DIR .. "src/lib/netlist/core/analog.h",
		MAME_DIR .. "src/lib/netlist/core/base_objects.h",
		MAME_DIR .. "src/lib/netlist/core/core_device.h",
		MAME_DIR .. "src/lib/netlist/core/device.h",
		MAME_DIR .. "src/lib/netlist/core/device_macros.h",
		MAME_DIR .. "src/lib/netlist/core/devices.h",
		MAME_DIR .. "src/lib/netlist/core/exec.h",
		MAME_DIR .. "src/lib/netlist/core/logic_family.h",
		MAME_DIR .. "src/lib/netlist/core/logic.h",
		MAME_DIR .. "src/lib/netlist/core/nets.h",
		MAME_DIR .. "src/lib/netlist/core/object_array.h",
		MAME_DIR .. "src/lib/netlist/core/param.h",
		MAME_DIR .. "src/lib/netlist/core/setup.h",
		MAME_DIR .. "src/lib/netlist/core/state_var.h",
		MAME_DIR .. "src/lib/netlist/plib/pconfig.h",
		MAME_DIR .. "src/lib/netlist/plib/palloc.h",
		MAME_DIR .. "src/lib/netlist/plib/pchrono.h",
		MAME_DIR .. "src/lib/netlist/plib/pgsl.h",
		MAME_DIR .. "src/lib/netlist/plib/penum.h",
		MAME_DIR .. "src/lib/netlist/plib/pexception.cpp",
		MAME_DIR .. "src/lib/netlist/plib/pexception.h",
		MAME_DIR .. "src/lib/netlist/plib/pfunction.cpp",
		MAME_DIR .. "src/lib/netlist/plib/pfunction.h",
		MAME_DIR .. "src/lib/netlist/plib/pfmtlog.cpp",
		MAME_DIR .. "src/lib/netlist/plib/pfmtlog.h",
		MAME_DIR .. "src/lib/netlist/plib/plists.h",
		MAME_DIR .. "src/lib/netlist/plib/pdynlib.cpp",
		MAME_DIR .. "src/lib/netlist/plib/pdynlib.h",
		MAME_DIR .. "src/lib/netlist/plib/pmain.cpp",
		MAME_DIR .. "src/lib/netlist/plib/pmain.h",
		MAME_DIR .. "src/lib/netlist/plib/pmath.h",
		MAME_DIR .. "src/lib/netlist/plib/pmempool.h",
		MAME_DIR .. "src/lib/netlist/plib/pmulti_threading.h",
		MAME_DIR .. "src/lib/netlist/plib/pomp.h",
		MAME_DIR .. "src/lib/netlist/plib/poptions.cpp",
		MAME_DIR .. "src/lib/netlist/plib/poptions.h",
		MAME_DIR .. "src/lib/netlist/plib/ppmf.h",
		MAME_DIR .. "src/lib/netlist/plib/ppreprocessor.cpp",
		MAME_DIR .. "src/lib/netlist/plib/ppreprocessor.h",
		MAME_DIR .. "src/lib/netlist/plib/prandom.h",
		MAME_DIR .. "src/lib/netlist/plib/pstate.h",
		MAME_DIR .. "src/lib/netlist/plib/pstonum.h",
		MAME_DIR .. "src/lib/netlist/plib/pstring.cpp",
		MAME_DIR .. "src/lib/netlist/plib/pstring.h",
		MAME_DIR .. "src/lib/netlist/plib/pstrutil.h",
		MAME_DIR .. "src/lib/netlist/plib/pstream.h",
		MAME_DIR .. "src/lib/netlist/plib/ptime.h",
		MAME_DIR .. "src/lib/netlist/plib/ptimed_queue.h",
		MAME_DIR .. "src/lib/netlist/plib/ptokenizer.cpp",
		MAME_DIR .. "src/lib/netlist/plib/ptokenizer.h",
		MAME_DIR .. "src/lib/netlist/plib/ptypes.h",
		MAME_DIR .. "src/lib/netlist/plib/putil.cpp",
		MAME_DIR .. "src/lib/netlist/plib/putil.h",
		MAME_DIR .. "src/lib/netlist/tools/nl_convert.cpp",
		MAME_DIR .. "src/lib/netlist/tools/nl_convert.h",
		MAME_DIR .. "src/lib/netlist/analog/nld_bjt.cpp",
		MAME_DIR .. "src/lib/netlist/analog/nld_bjt.h",
		MAME_DIR .. "src/lib/netlist/analog/nld_generic_models.h",
		MAME_DIR .. "src/lib/netlist/analog/nld_mosfet.cpp",
		MAME_DIR .. "src/lib/netlist/analog/nld_mosfet.h",
		MAME_DIR .. "src/lib/netlist/analog/nlid_fourterm.cpp",
		MAME_DIR .. "src/lib/netlist/analog/nlid_fourterm.h",
		MAME_DIR .. "src/lib/netlist/analog/nld_fourterm.h",
		MAME_DIR .. "src/lib/netlist/analog/nld_switches.cpp",
		MAME_DIR .. "src/lib/netlist/analog/nld_switches.h",
		MAME_DIR .. "src/lib/netlist/analog/nlid_twoterm.cpp",
		MAME_DIR .. "src/lib/netlist/analog/nlid_twoterm.h",
		MAME_DIR .. "src/lib/netlist/analog/nld_twoterm.h",
		MAME_DIR .. "src/lib/netlist/analog/nld_opamps.cpp",
		MAME_DIR .. "src/lib/netlist/analog/nld_opamps.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_solver.cpp",
		MAME_DIR .. "src/lib/netlist/solver/nld_solver.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_matrix_solver.cpp",
		MAME_DIR .. "src/lib/netlist/solver/nld_matrix_solver.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_direct.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_direct1.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_direct2.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_sor.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_sor_mat.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_gmres.h",
		MAME_DIR .. "src/lib/netlist/solver/mat_cr.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_sm.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_w.h",
		MAME_DIR .. "src/lib/netlist/solver/nld_ms_direct_lu.h",
		MAME_DIR .. "src/lib/netlist/solver/vector_base.h",
		MAME_DIR .. "src/lib/netlist/devices/net_lib.cpp",
		MAME_DIR .. "src/lib/netlist/devices/net_lib.h",
		MAME_DIR .. "src/lib/netlist/devices/nld_9316_base.hxx",
		MAME_DIR .. "src/lib/netlist/devices/nlid_truthtable.h",
		MAME_DIR .. "src/lib/netlist/devices/nlid_system.h",
		MAME_DIR .. "src/lib/netlist/devices/nlid_proxy.h",

		MAME_DIR .. "src/lib/netlist/generated/static_solvers.cpp",
		MAME_DIR .. "src/lib/netlist/generated/nld_devinc.h",
		MAME_DIR .. "src/lib/netlist/generated/lib_entries.hxx",
		MAME_DIR .. "src/lib/netlist/generated/nlm_modules_lib.cpp",
	}

	dofile(MAME_DIR .. "src/lib/netlist/generated/mame_netlist.lua")
	