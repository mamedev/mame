// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/debugvw.h"
#include "debug/points.h"
#include "debug/textbuf.h"
#include "debugger.h"


namespace {

struct wrap_textbuf
{
	wrap_textbuf(text_buffer const &buf) : textbuf(buf) { }

	std::reference_wrapper<const text_buffer> textbuf;
};


template <bool Enable>
sol::object do_breakpoint_enable(device_debug &dev, sol::this_state s, sol::object index)
{
	if (index == sol::lua_nil)
	{
		dev.breakpoint_enable_all(Enable);
		dev.device().machine().debug_view().update_all(DVT_DISASSEMBLY);
		dev.device().machine().debug_view().update_all(DVT_BREAK_POINTS);
		return sol::lua_nil;
	}
	else if (index.is<int>())
	{
		bool result(dev.breakpoint_enable(index.as<int>(), Enable));
		if (result)
		{
			dev.device().machine().debug_view().update_all(DVT_DISASSEMBLY);
			dev.device().machine().debug_view().update_all(DVT_BREAK_POINTS);
		}
		return sol::make_object(s, result);
	}
	else
	{
		osd_printf_error("[LUA ERROR] must call bpenable with integer or nil\n");
		return sol::lua_nil;
	}
}


template <bool Enable>
sol::object do_watchpoint_enable(device_debug &dev, sol::this_state s, sol::object index)
{
	if (index == sol::lua_nil)
	{
		dev.watchpoint_enable_all(Enable);
		dev.device().machine().debug_view().update_all(DVT_WATCH_POINTS);
		return sol::lua_nil;
	}
	else if (index.is<int>())
	{
		bool result(dev.watchpoint_enable(index.as<int>(), Enable));
		if (result)
			dev.device().machine().debug_view().update_all(DVT_WATCH_POINTS);
		return sol::make_object(s, result);
	}
	else
	{
		osd_printf_error("[LUA ERROR] must call wpenable with integer or nil");
		return sol::lua_nil;
	}
}

} // anonymous namespace


void lua_engine::initialize_debug(sol::table &emu)
{

	static const enum_parser<read_or_write, 4> s_read_or_write_parser =
	{
		{ "r", read_or_write::READ },
		{ "w", read_or_write::WRITE },
		{ "rw", read_or_write::READWRITE },
		{ "wr", read_or_write::READWRITE }
	};


	auto debugger_type = sol().registry().new_usertype<debugger_manager>("debugger", sol::no_constructor);
	debugger_type["command"] = [] (debugger_manager &debug, std::string const &cmd) { debug.console().execute_command(cmd, false); };
	debugger_type["consolelog"] = sol::property([] (debugger_manager &debug) { return wrap_textbuf(debug.console().get_console_textbuf()); });
	debugger_type["errorlog"] = sol::property([](debugger_manager &debug) { return wrap_textbuf(debug.console().get_errorlog_textbuf()); });
	debugger_type["visible_cpu"] = sol::property(
			[](debugger_manager &debug) { return debug.console().get_visible_cpu(); },
			[](debugger_manager &debug, device_t &dev) { debug.console().set_visible_cpu(&dev); });
	debugger_type["execution_state"] = sol::property(
			[] (debugger_manager &debug) { return debug.cpu().is_stopped() ? "stop" : "run"; },
			[] (debugger_manager &debug, std::string const &state)
			{
				if (state == "stop")
					debug.cpu().set_execution_stopped();
				else
					debug.cpu().set_execution_running();
			});


/* wrap_textbuf library (requires debugger to be active)
 *
 * manager:machine():debugger().consolelog
 * manager:machine():debugger().errorlog
 *
 * log[index] - get log entry
 * #log - entry count
 */

	sol().registry().new_usertype<wrap_textbuf>("text_buffer", "new", sol::no_constructor,
			"__metatable", [](){},
			"__newindex", [](){},
			"__index", [](wrap_textbuf &buf, int index) { return text_buffer_get_seqnum_line(buf.textbuf, index - 1); },
			"__len", [](wrap_textbuf &buf) { return text_buffer_num_lines(buf.textbuf) + text_buffer_line_index_to_seqnum(buf.textbuf, 0) - 1; });


	auto device_debug_type = sol().registry().new_usertype<device_debug>("device_debug", sol::no_constructor);
	device_debug_type["step"] =
		[] (device_debug &dev, sol::object num)
		{
			int steps = 1;
			if (num.is<int>())
				steps = num.as<int>();
			dev.single_step(steps);
		};
	device_debug_type["go"] = &device_debug::go;
	device_debug_type["bpset"] =
		[] (device_debug &dev, offs_t address, char const *cond, char const *act)
		{
			int result(dev.breakpoint_set(address, cond, act));
			dev.device().machine().debug_view().update_all(DVT_DISASSEMBLY);
			dev.device().machine().debug_view().update_all(DVT_BREAK_POINTS);
			return result;
		};
	device_debug_type["bpclear"] = sol::overload(
			[] (device_debug &dev, int index)
			{
				bool result(dev.breakpoint_clear(index));
				if (result)
				{
					dev.device().machine().debug_view().update_all(DVT_DISASSEMBLY);
					dev.device().machine().debug_view().update_all(DVT_BREAK_POINTS);
				}
				return result;
			},
			[] (device_debug &dev)
			{
				dev.breakpoint_clear_all();
				dev.device().machine().debug_view().update_all(DVT_DISASSEMBLY);
				dev.device().machine().debug_view().update_all(DVT_BREAK_POINTS);
			});
	device_debug_type["bpenable"] = &do_breakpoint_enable<true>;
	device_debug_type["bpdisable"] = &do_breakpoint_enable<false>;
	device_debug_type["bplist"] =
		[this] (device_debug &dev)
		{
			sol::table table = sol().create_table();
			for (auto const &bpp : dev.breakpoint_list())
				table[bpp.second->index()] = sol::make_reference(sol(), *bpp.second);
			return table;
		};
	device_debug_type["wpset"] =
		[] (device_debug &dev, addr_space &sp, std::string const &type, offs_t addr, offs_t len, char const *cond, char const *act)
		{
			read_or_write wptype = s_read_or_write_parser(type);
			int result(dev.watchpoint_set(sp.space, wptype, addr, len, cond, act));
			dev.device().machine().debug_view().update_all(DVT_WATCH_POINTS);
			return result;
		};
	device_debug_type["wpclear"] = sol::overload(
			[] (device_debug &dev, int index)
			{
				bool result(dev.watchpoint_clear(index));
				if (result)
					dev.device().machine().debug_view().update_all(DVT_WATCH_POINTS);
				return result;
			},
			[] (device_debug &dev)
			{
				dev.watchpoint_clear_all();
				dev.device().machine().debug_view().update_all(DVT_WATCH_POINTS);
			});
	device_debug_type["wpenable"] = &do_watchpoint_enable<true>;
	device_debug_type["wpdisable"] = &do_watchpoint_enable<false>;
	device_debug_type["wplist"] =
		[this] (device_debug &dev, addr_space &sp)
		{
			sol::table table = sol().create_table();
			for (auto &wpp : dev.watchpoint_vector(sp.space.spacenum()))
				table[wpp->index()] = sol::make_reference(sol(), *wpp);
			return table;
		};


	auto breakpoint_type = sol().registry().new_usertype<debug_breakpoint>("breakpoint", sol::no_constructor);
	breakpoint_type["index"] = sol::property(&debug_breakpoint::index);
	breakpoint_type["enabled"] = sol::property(&debug_breakpoint::enabled);
	breakpoint_type["address"] = sol::property(&debug_breakpoint::address);
	breakpoint_type["condition"] = sol::property(&debug_breakpoint::condition);
	breakpoint_type["action"] = sol::property(&debug_breakpoint::action);


	auto watchpoint_type = sol().registry().new_usertype<debug_watchpoint>("watchpoint", sol::no_constructor);
	watchpoint_type["index"] = sol::property(&debug_watchpoint::index);
	watchpoint_type["enabled"] = sol::property(&debug_watchpoint::enabled);
	watchpoint_type["type"] = sol::property(
			[] (debug_watchpoint &wp) -> char const *
			{
				switch (wp.type())
				{
				case read_or_write::READ:
					return "r";
				case read_or_write::WRITE:
					return "w";
				case read_or_write::READWRITE:
					return "rw";
				default: // huh?
					return "";
				}
			});
	watchpoint_type["address"] = sol::property(&debug_watchpoint::address);
	watchpoint_type["length"] = sol::property(&debug_watchpoint::length);
	watchpoint_type["condition"] = sol::property(&debug_watchpoint::condition);
	watchpoint_type["action"] = sol::property(&debug_watchpoint::action);

}
