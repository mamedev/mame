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


class lua_engine::symbol_table_wrapper
{
public:
	symbol_table_wrapper(symbol_table_wrapper const &) = delete;

	symbol_table_wrapper(lua_engine &host, running_machine &machine, std::shared_ptr<symbol_table_wrapper> const &parent, device_t *device)
		: m_host(host)
		, m_table(machine, symbol_table::BUILTIN_GLOBALS, parent ? &parent->table() : nullptr, device)
		, m_parent(parent)
	{
	}

	symbol_table &table() { return m_table; }
	symbol_table const &table() const { return m_table; }
	std::shared_ptr<symbol_table_wrapper> const &parent() { return m_parent; }

	symbol_entry &add(char const *name) { return m_table.add(name, symbol_table::READ_WRITE); }
	symbol_entry &add(char const *name, u64 value) { return m_table.add(name, value); }
	symbol_entry &add(char const *name, sol::protected_function getter, std::optional<sol::protected_function> setter, std::optional<char const *> format)
	{
		symbol_table::setter_func setfun;
		if (setter)
		{
			setfun =
					[this, cbfunc = sol::protected_function(m_host.m_lua_state, *setter)] (u64 value)
					{
						auto status = m_host.invoke(cbfunc, value);
						if (!status.valid())
						{
							sol::error err = status;
							osd_printf_error("[LUA EROR] in symbol value setter callback: %s\n", err.what());
						}
					};
		}
		return m_table.add(
				name,
				[this, cbfunc = sol::protected_function(m_host.m_lua_state, getter)] () -> u64
				{
					auto status = m_host.invoke(cbfunc);
					if (status.valid())
					{
						auto result = status.get<std::optional<u64> >();
						if (result)
							return *result;

						osd_printf_error("[LUA EROR] invalid return from symbol value getter callback\n");
					}
					else
					{
						sol::error err = status;
						osd_printf_error("[LUA EROR] in symbol value getter callback: %s\n", err.what());
					}
					return 0;
				},
				std::move(setfun),
				(format && *format) ? *format : "");
	}
	symbol_entry *find(char const *name) const { return m_table.find(name); }
	symbol_entry *find_deep(char const *name) { return m_table.find_deep(name); }

	u64 value(const char *symbol) { return m_table.value(symbol); }
	void set_value(const char *symbol, u64 value) { m_table.set_value(symbol, value); }

	u64 read_memory(addr_space &space, offs_t address, int size, bool translate) { return m_table.read_memory(space.space, address, size, translate); }
	void write_memory(addr_space &space, offs_t address, u64 data, int size, bool translate) { m_table.write_memory(space.space, address, data, size, translate); }

private:
	lua_engine &m_host;
	symbol_table m_table;
	std::shared_ptr<symbol_table_wrapper> const m_parent;
};


class lua_engine::expression_wrapper
{
public:
	expression_wrapper(expression_wrapper const &) = delete;
	expression_wrapper &operator=(expression_wrapper const &) = delete;

	expression_wrapper(std::shared_ptr<symbol_table_wrapper> const &symtable)
		: m_expression(symtable->table())
		, m_symbols(symtable)
	{
	}

	void set_default_base(int base) { m_expression.set_default_base(base); }

	void parse(sol::this_state s, char const *string)
	{
		try
		{
			m_expression.parse(string);
		}
		catch (expression_error const &err)
		{
			sol::stack::push(s, err);
			lua_error(s);
		}
	}

	u64 execute(sol::this_state s)
	{
		try
		{
			return m_expression.execute();
		}
		catch (expression_error const &err)
		{
			sol::stack::push(s, err);
			lua_error(s);
			return 0; // unreachable - lua_error doesn't return
		}
	}

	bool is_empty() const { return m_expression.is_empty(); }
	char const *original_string() const { return m_expression.original_string(); }
	std::shared_ptr<symbol_table_wrapper> const &symbols() { return m_symbols; }

	void set_symbols(std::shared_ptr<symbol_table_wrapper> const &symtable)
	{
		m_expression.set_symbols(symtable->table());
		m_symbols = symtable;
	}

private:
	parsed_expression m_expression;
	std::shared_ptr<symbol_table_wrapper> m_symbols;
};


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

	auto expression_error_type = emu.new_usertype<expression_error>(
			"expression_error",
			sol::no_constructor);
	expression_error_type["code"] = sol::property(
			[] (expression_error const &err) { return int(err.code()); });
	expression_error_type["offset"] = sol::property(&expression_error::offset);
	expression_error_type[sol::meta_function::to_string] = &expression_error::code_string;

	auto symbol_table_type = emu.new_usertype<symbol_table_wrapper>(
			"symbol_table",
			sol::call_constructor, sol::factories(
				[this] (running_machine &machine)
				{ return std::make_shared<symbol_table_wrapper>(*this, machine, nullptr, nullptr); },
				[this] (std::shared_ptr<symbol_table_wrapper> const &parent, device_t *device)
				{ return std::make_shared<symbol_table_wrapper>(*this, parent->table().machine(), parent, device); },
				[this] (std::shared_ptr<symbol_table_wrapper> const &parent)
				{ return std::make_shared<symbol_table_wrapper>(*this, parent->table().machine(), parent, nullptr); },
				[this] (device_t &device)
				{ return std::make_shared<symbol_table_wrapper>(*this, device.machine(), nullptr, &device); }));
	symbol_table_type.set_function("set_memory_modified_func",
			[this] (symbol_table_wrapper &st, sol::object cb)
			{
				if (cb == sol::lua_nil)
					st.table().set_memory_modified_func(nullptr);
				else if (cb.is<sol::protected_function>())
					st.table().set_memory_modified_func([this, cbfunc = sol::protected_function(m_lua_state, cb)] () { invoke(cbfunc); });
				else
					osd_printf_error("[LUA ERROR] must call set_memory_modified_func with function or nil\n");
			});
	symbol_table_type["add"] = sol::overload(
			static_cast<symbol_entry &(symbol_table_wrapper::*)(char const *)>(&symbol_table_wrapper::add),
			static_cast<symbol_entry &(symbol_table_wrapper::*)(char const *, u64)>(&symbol_table_wrapper::add),
			static_cast<symbol_entry &(symbol_table_wrapper::*)(char const *, sol::protected_function, std::optional<sol::protected_function>, std::optional<char const *>)>(&symbol_table_wrapper::add),
			[] (symbol_table_wrapper &st, char const *name, sol::protected_function getter, sol::lua_nil_t, char const *format) -> symbol_entry &
			{
				return st.add(name, getter, std::nullopt, format);
			},
			[] (symbol_table_wrapper &st, char const *name, sol::protected_function getter, std::optional<sol::protected_function> setter) -> symbol_entry &
			{
				return st.add(name, getter, setter, nullptr);
			},
			[] (symbol_table_wrapper &st, char const *name, sol::protected_function getter, char const *format) -> symbol_entry &
			{
				return st.add(name, getter, std::nullopt, format);
			},
			[] (symbol_table_wrapper &st, char const *name, sol::protected_function getter) -> symbol_entry &
			{
				return st.add(name, getter, std::nullopt, nullptr);
			},
			[this] (symbol_table_wrapper &st, char const *name, int minparams, int maxparams, sol::protected_function execute) -> symbol_entry &
			{
				return st.table().add(
						name,
						minparams,
						maxparams,
						[this, cb = sol::protected_function(m_lua_state, execute)] (int numparams, u64 const *paramlist) -> u64
						{
							// TODO: C++20 will make this obsolete
							class helper
							{
							private:
								u64 const *b, *e;
							public:
								helper(int n, u64 const *p) : b(p), e(p + n) { }
								auto begin() const { return b; }
								auto end() const { return e; }
							};

							auto status(invoke(cb, sol::as_args(helper(numparams, paramlist))));
							if (status.valid())
							{
								auto result = status.get<std::optional<u64> >();
								if (result)
									return *result;

								osd_printf_error("[LUA EROR] invalid return from symbol execute callback\n");
							}
							else
							{
								sol::error err = status;
								osd_printf_error("[LUA EROR] in symbol execute callback: %s\n", err.what());
							}
							return 0;
						});
			});
	symbol_table_type.set_function("find", &symbol_table_wrapper::find);
	symbol_table_type.set_function("find_deep", &symbol_table_wrapper::find_deep);
	symbol_table_type.set_function("value", &symbol_table_wrapper::value);
	symbol_table_type.set_function("set_value", &symbol_table_wrapper::set_value);
	symbol_table_type.set_function("memory_value",
			[] (symbol_table_wrapper &st, char const *name, char const *space, u32 offset, int size, bool disable_se)
			{
				expression_space const es = s_expression_space_parser(space);
				return st.table().memory_value(name, es, offset, size, disable_se);
			});
	symbol_table_type.set_function("set_memory_value",
			[] (symbol_table_wrapper &st, char const *name, char const *space, u32 offset, int size, u64 value, bool disable_se)
			{
				expression_space const es = s_expression_space_parser(space);
				st.table().set_memory_value(name, es, offset, size, value, disable_se);
			});
	symbol_table_type.set_function("read_memory", &symbol_table_wrapper::read_memory);
	symbol_table_type.set_function("write_memory", &symbol_table_wrapper::write_memory);
	symbol_table_type["entries"] = sol::property([] (symbol_table_wrapper const &st) { return standard_tag_object_ptr_map<symbol_entry>(st.table().entries()); });
	symbol_table_type["parent"] = sol::property(&symbol_table_wrapper::parent);


	auto parsed_expression_type = emu.new_usertype<expression_wrapper>(
			"parsed_expression",
			sol::call_constructor, sol::initializers(
				[] (expression_wrapper &wrapper, std::shared_ptr<symbol_table_wrapper> const &symbols)
				{
					new (&wrapper) expression_wrapper(symbols);
				},
				[] (expression_wrapper &wrapper, sol::this_state s, std::shared_ptr<symbol_table_wrapper> const &symbols, char const *expression, int base)
				{
					new (&wrapper) expression_wrapper(symbols);
					wrapper.set_default_base(base);
					wrapper.parse(s, expression);
				},
				[] (expression_wrapper &wrapper, sol::this_state s, std::shared_ptr<symbol_table_wrapper> const &symbols, char const *expression)
				{
					new (&wrapper) expression_wrapper(symbols);
					wrapper.parse(s, expression);
				}));
	parsed_expression_type.set_function("set_default_base", &expression_wrapper::set_default_base);
	parsed_expression_type.set_function("parse", &expression_wrapper::parse);
	parsed_expression_type.set_function("execute", &expression_wrapper::execute);
	parsed_expression_type["is_empty"] = sol::property(&expression_wrapper::is_empty);
	parsed_expression_type["original_string"] = sol::property(&expression_wrapper::original_string);
	parsed_expression_type["symbols"] = sol::property(&expression_wrapper::symbols, &expression_wrapper::set_symbols);


	auto symbol_entry_type = sol().registry().new_usertype<symbol_entry>("symbol_entry", sol::no_constructor);
	symbol_entry_type["name"] = sol::property(&symbol_entry::name);
	symbol_entry_type["format"] = sol::property(&symbol_entry::format);
	symbol_entry_type["is_function"] = sol::property(&symbol_entry::is_function);
	symbol_entry_type["is_lval"] = sol::property(&symbol_entry::is_lval);
	symbol_entry_type["value"] = sol::property(&symbol_entry::value, &symbol_entry::set_value);


	auto debugger_type = sol().registry().new_usertype<debugger_manager>("debugger", sol::no_constructor);
	debugger_type.set_function("command", [] (debugger_manager &debug, std::string const &cmd) { debug.console().execute_command(cmd, false); });
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
	device_debug_type.set_function("step",
			[] (device_debug &dev, sol::object num)
			{
				int steps = 1;
				if (num.is<int>())
					steps = num.as<int>();
				dev.single_step(steps);
			});
	device_debug_type.set_function("go", &device_debug::go);
	device_debug_type.set_function("bpset",
			[] (device_debug &dev, offs_t address, char const *cond, char const *act)
			{
				int result(dev.breakpoint_set(address, cond, act));
				dev.device().machine().debug_view().update_all(DVT_DISASSEMBLY);
				dev.device().machine().debug_view().update_all(DVT_BREAK_POINTS);
				return result;
			});
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
	device_debug_type.set_function("bpenable", &do_breakpoint_enable<true>);
	device_debug_type.set_function("bpdisable", &do_breakpoint_enable<false>);
	device_debug_type.set_function("bplist",
			[this] (device_debug &dev)
			{
				sol::table table = sol().create_table();
				for (auto const &bpp : dev.breakpoint_list())
					table[bpp.second->index()] = sol::make_reference(sol(), bpp.second.get());
				return table;
			});
	device_debug_type.set_function("wpset",
			[] (device_debug &dev, addr_space &sp, std::string const &type, offs_t addr, offs_t len, char const *cond, char const *act)
			{
				read_or_write const wptype = s_read_or_write_parser(type);
				int result(dev.watchpoint_set(sp.space, wptype, addr, len, cond, act));
				dev.device().machine().debug_view().update_all(DVT_WATCH_POINTS);
				return result;
			});
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
	device_debug_type.set_function("wpenable", &do_watchpoint_enable<true>);
	device_debug_type.set_function("wpdisable", &do_watchpoint_enable<false>);
	device_debug_type.set_function("wplist",
			[this] (device_debug &dev, addr_space &sp)
			{
				sol::table table = sol().create_table();
				for (auto &wpp : dev.watchpoint_vector(sp.space.spacenum()))
					table[wpp->index()] = sol::make_reference(sol(), wpp.get());
				return table;
			});


	auto breakpoint_type = sol().registry().new_usertype<debug_breakpoint>("breakpoint", sol::no_constructor);
	breakpoint_type["index"] = sol::property(&debug_breakpoint::index);
	breakpoint_type["enabled"] = sol::property(
			&debug_breakpoint::enabled,
			[] (debug_breakpoint &bp, bool val)
			{
				if (bp.enabled() != val)
				{
					bp.setEnabled(val);
					bp.debugInterface()->device().machine().debug_view().update_all(DVT_DISASSEMBLY);
					bp.debugInterface()->device().machine().debug_view().update_all(DVT_BREAK_POINTS);
				}
			});
	breakpoint_type["address"] = sol::property(&debug_breakpoint::address);
	breakpoint_type["condition"] = sol::property(&debug_breakpoint::condition);
	breakpoint_type["action"] = sol::property(&debug_breakpoint::action);


	auto watchpoint_type = sol().registry().new_usertype<debug_watchpoint>("watchpoint", sol::no_constructor);
	watchpoint_type["index"] = sol::property(&debug_watchpoint::index);
	watchpoint_type["enabled"] = sol::property(
			&debug_watchpoint::enabled,
			[] (debug_watchpoint &wp, bool val)
			{
				if (wp.enabled() != val)
				{
					wp.setEnabled(val);
					wp.debugInterface()->device().machine().debug_view().update_all(DVT_WATCH_POINTS);
				}
			});
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
