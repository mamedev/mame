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
#include "debug/express.h"
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

	static const enum_parser<expression_space, 15> s_expression_space_parser =
	{
		{ "p", EXPSPACE_PROGRAM_LOGICAL }, { "lp", EXPSPACE_PROGRAM_LOGICAL }, { "pp", EXPSPACE_PROGRAM_PHYSICAL },
		{ "d", EXPSPACE_DATA_LOGICAL },    { "ld", EXPSPACE_DATA_LOGICAL },    { "pd", EXPSPACE_DATA_PHYSICAL },
		{ "i", EXPSPACE_IO_LOGICAL },      { "li", EXPSPACE_IO_LOGICAL },      { "pi", EXPSPACE_IO_PHYSICAL },
		{ "3", EXPSPACE_OPCODE_LOGICAL },  { "l3", EXPSPACE_OPCODE_LOGICAL },  { "p3", EXPSPACE_OPCODE_PHYSICAL },
		{ "r", EXPSPACE_PRGDIRECT },
		{ "o", EXPSPACE_OPDIRECT },
		{ "m", EXPSPACE_REGION }
	};

	auto const do_add_symbol = [this] (symbol_table &st, char const *name, sol::protected_function getter, std::optional<sol::protected_function> setter, std::optional<char const *> format)
	{
		symbol_table::setter_func setfun;
		if (setter)
			setfun = [this, cbfunc = std::move(*setter)] (u64 value) { invoke(cbfunc, value); };
		st.add(
				name,
				[this, cbfunc = std::move(getter)] () -> u64
				{
					auto result = invoke(cbfunc).get<sol::optional<u64> >();
					if (result)
					{
						return *result;
					}
					else
					{
						osd_printf_error("[LUA EROR] invalid return from symbol value getter callback\n");
						return 0;
					}
				},
				std::move(setfun),
				(format && *format) ? *format : "");
	};

	auto symbol_table_type = emu.new_usertype<symbol_table>(
			"symbol_table",
			sol::call_constructor, sol::factories(
				[] (running_machine &machine, symbol_table *parent, device_t *device) { return std::make_unique<symbol_table>(machine, parent, device); },
				[] (running_machine &machine, symbol_table *parent) { return std::make_unique<symbol_table>(machine, parent); },
				[] (running_machine &machine, device_t &device) { return std::make_unique<symbol_table>(machine, nullptr, &device); },
				[] (running_machine &machine) { return std::make_unique<symbol_table>(machine); }));
	symbol_table_type["set_memory_modified_func"] =
			[this] (symbol_table &st, sol::object cb)
			{
				if (cb == sol::lua_nil)
					st.set_memory_modified_func(nullptr);
				else if (cb.is<sol::protected_function>())
					st.set_memory_modified_func([this, cbfunc = cb.as<sol::protected_function>()] () { invoke(cbfunc); });
				else
					osd_printf_error("[LUA ERROR] must call set_memory_modified_func with function or nil\n");
			};
	symbol_table_type["add"] = sol::overload(
			[] (symbol_table &st, char const *name) { st.add(name, symbol_table::READ_WRITE); },
			static_cast<void (symbol_table::*)(char const *, u64)>(&symbol_table::add),
			do_add_symbol,
			[do_add_symbol] (symbol_table &st, char const *name, sol::protected_function getter, sol::lua_nil_t, char const *format)
			{
				do_add_symbol(st, name, getter, std::nullopt, format);
			},
			[do_add_symbol] (symbol_table &st, char const *name, sol::protected_function getter, std::optional<sol::protected_function> setter)
			{
				do_add_symbol(st, name, getter, setter, nullptr);
			},
			[do_add_symbol] (symbol_table &st, char const *name, sol::protected_function getter, char const *format)
			{
				do_add_symbol(st, name, getter, std::nullopt, format);
			},
			[do_add_symbol] (symbol_table &st, char const *name, sol::protected_function getter)
			{
				do_add_symbol(st, name, getter, std::nullopt, nullptr);
			},
			[this] (symbol_table &st, char const *name, int minparams, int maxparams, sol::protected_function execute)
			{
				st.add(
						name,
						minparams,
						maxparams,
						[this, cbref = sol::reference(execute)] (int numparams, u64 const *paramlist) -> u64
						{
							sol::stack_reference traceback(m_lua_state, -sol::stack::push(m_lua_state, sol::default_traceback_error_handler));
							cbref.push();
							sol::stack_aligned_stack_handler_function func(m_lua_state, -1, traceback);
							for (int i = 0; numparams > i; ++i)
								lua_pushinteger(m_lua_state, paramlist[i]);
							auto result = func(sol::stack_count(numparams)).get<sol::optional<u64> >();
							traceback.pop();
							return result ? *result : 0;
						});
			});
	symbol_table_type["find"] = &symbol_table::find;
	symbol_table_type["find_deep"] = &symbol_table::find_deep;
	symbol_table_type["value"] = &symbol_table::value;
	symbol_table_type["set_value"] = &symbol_table::set_value;
	symbol_table_type["memory_value"] =
			[] (symbol_table &st, char const *name, char const *space, u32 offset, int size, bool disable_se)
			{
				expression_space const es = s_expression_space_parser(space);
				return st.memory_value(name, es, offset, size, disable_se);
			};
	symbol_table_type["set_memory_value"] =
			[] (symbol_table &st, char const *name, char const *space, u32 offset, int size, u64 value, bool disable_se)
			{
				expression_space const es = s_expression_space_parser(space);
				st.set_memory_value(name, es, offset, size, value, disable_se);
			};
	//symbol_table_type["read_memory"] = &symbol_table::read_memory; crashes if you try to use it, need to work out why
	//symbol_table_type["write_memory"] = &symbol_table::write_memory; crashes if you try to use it, need to work out why
	symbol_table_type["entries"] = sol::property([] (symbol_table const &st) { return standard_tag_object_ptr_map<symbol_entry>(st.entries()); });
	symbol_table_type["parent"] = sol::property(&symbol_table::parent);


	auto parsed_expression_type = emu.new_usertype<parsed_expression>(
			"parsed_expression",
			sol::call_constructor, sol::factories(
				[] (symbol_table &symtable) { return std::make_unique<parsed_expression>(symtable); },
				[] (symbol_table &symtable, char const *expression, int default_base) { return std::make_unique<parsed_expression>(symtable, expression, default_base); },
				[] (symbol_table &symtable, char const *expression) { return std::make_unique<parsed_expression>(symtable, expression); },
				[] (parsed_expression const &src) { return std::make_unique<parsed_expression>(src); }));
	parsed_expression_type["set_default_base"] = &parsed_expression::set_default_base;
	parsed_expression_type["parse"] = [] (parsed_expression &e, char const *string) { e.parse(string); };
	parsed_expression_type["execute"] = &parsed_expression::execute;
	parsed_expression_type["is_empty"] = sol::property(&parsed_expression::is_empty);
	parsed_expression_type["original_string"] = sol::property(&parsed_expression::original_string);
	parsed_expression_type["symbols"] = sol::property(&parsed_expression::symbols, &parsed_expression::set_symbols);


	auto symbol_entry_type = sol().registry().new_usertype<symbol_entry>("symbol_entry", sol::no_constructor);
	symbol_entry_type["name"] = sol::property(&symbol_entry::name);
	symbol_entry_type["format"] = sol::property(&symbol_entry::format);
	symbol_entry_type["is_function"] = sol::property(&symbol_entry::is_function);
	symbol_entry_type["is_lval"] = sol::property(&symbol_entry::is_lval);
	symbol_entry_type["value"] = sol::property(&symbol_entry::value, &symbol_entry::set_value);


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
			read_or_write const wptype = s_read_or_write_parser(type);
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
