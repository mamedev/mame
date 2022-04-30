// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcon.h

    Debugger console engine.

*********************************************************************/

#ifndef MAME_EMU_DEBUG_DEBUGCON_H
#define MAME_EMU_DEBUG_DEBUGCON_H

#pragma once

#include "textbuf.h"

#include <functional>
#include <set>


/***************************************************************************
    CONSTANTS
***************************************************************************/

constexpr int MAX_COMMAND_PARAMS            = 128;

// flags for command parsing
constexpr u32 CMDFLAG_NONE                  = 0x0000;
constexpr u32 CMDFLAG_KEEP_QUOTES           = 0x0001;
constexpr u32 CMDFLAG_CUSTOM_HELP           = 0x0002;

// parameter separator macros
#define CMDPARAM_SEPARATOR                  "\0"
#define CMDPARAM_TERMINATOR                 "\0\0"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// CMDERR is an error code for command evaluation
class CMDERR
{
public:
	// values for the error code in a command error
	static constexpr u16 NONE               = 0;
	static constexpr u16 UNKNOWN_COMMAND    = 1;
	static constexpr u16 AMBIGUOUS_COMMAND  = 2;
	static constexpr u16 UNBALANCED_PARENS  = 3;
	static constexpr u16 UNBALANCED_QUOTES  = 4;
	static constexpr u16 NOT_ENOUGH_PARAMS  = 5;
	static constexpr u16 TOO_MANY_PARAMS    = 6;
	static constexpr u16 EXPRESSION_ERROR   = 7;

	// command error assembly/disassembly
	constexpr CMDERR(u16 c, u16 o) : m_error_class(c), m_error_offset(o) { }
	constexpr u16 error_class() const { return m_error_class; }
	constexpr u16 error_offset() const { return m_error_offset; }

	// assemble specific error conditions
	static constexpr CMDERR none()                      { return CMDERR(NONE, 0); }
	static constexpr CMDERR unknown_command(u16 x)      { return CMDERR(UNKNOWN_COMMAND, x); }
	static constexpr CMDERR ambiguous_command(u16 x)    { return CMDERR(AMBIGUOUS_COMMAND, x); }
	static constexpr CMDERR unbalanced_parens(u16 x)    { return CMDERR(UNBALANCED_PARENS, x); }
	static constexpr CMDERR unbalanced_quotes(u16 x)    { return CMDERR(UNBALANCED_QUOTES, x); }
	static constexpr CMDERR not_enough_params(u16 x)    { return CMDERR(NOT_ENOUGH_PARAMS, x); }
	static constexpr CMDERR too_many_params(u16 x)      { return CMDERR(TOO_MANY_PARAMS, x); }
	static constexpr CMDERR expression_error(u16 x)     { return CMDERR(EXPRESSION_ERROR, x); }

private:
	u16 m_error_class, m_error_offset;
};


class debugger_console
{
public:
	debugger_console(running_machine &machine);
	~debugger_console();

	// command handling
	CMDERR          execute_command(std::string_view command, bool echo);
	CMDERR          validate_command(std::string_view command);
	void            register_command(std::string_view command, u32 flags, int minparams, int maxparams, std::function<void (const std::vector<std::string> &)> &&handler);
	void            source_script(const char *file);
	void            process_source_file();

	// console management
	void vprintf(util::format_argument_pack<std::ostream> const &args);
	void vprintf(util::format_argument_pack<std::ostream> &&args);
	void vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> const &args);
	void vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> &&args);
	text_buffer &get_console_textbuf() const { return *m_console_textbuf; }

	// errorlog management
	void errorlog_write_line(const char *line);
	text_buffer &get_errorlog_textbuf() const { return *m_errorlog_textbuf; }

	// convenience templates
	template <typename Format, typename... Params>
	inline void printf(Format &&fmt, Params &&...args)
	{
		vprintf(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}
	template <typename Format, typename... Params>
	inline void printf_wrap(int wrapcol, Format &&fmt, Params &&...args)
	{
		vprintf_wrap(wrapcol, util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	device_t *get_visible_cpu() { return m_visiblecpu; }
	void set_visible_cpu(device_t *visiblecpu) { m_visiblecpu = visiblecpu; }
	symbol_table &visible_symtable();

	static std::string cmderr_to_string(CMDERR error);

private:
	void exit();

	void execute_help_custom(const std::vector<std::string> &params);
	void execute_condump(const std::vector<std::string>& params);

	[[nodiscard]] static std::string_view trim_parameter(std::string_view param, bool keep_quotes);
	CMDERR internal_execute_command(bool execute, std::vector<std::string_view> &params);
	CMDERR internal_parse_command(std::string_view command, bool execute);

	void print_core(std::string_view text);                   // core text output
	void print_core_wrap(std::string_view text, int wrapcol); // core text output

	struct debug_command
	{
		debug_command(std::string_view _command, u32 _flags, int _minparams, int _maxparams, std::function<void (const std::vector<std::string> &)> &&_handler);

		struct compare
		{
			using is_transparent = void;
			bool operator()(const debug_command &a, const debug_command &b) const;
			bool operator()(const char *a, const debug_command &b) const;
			bool operator()(const debug_command &a, const char *b) const;
		};

		std::string     command;
		const char *    params;
		const char *    help;
		std::function<void (const std::vector<std::string> &)> handler;
		u32             flags;
		int             minparams;
		int             maxparams;
	};

	running_machine &m_machine;

	// visible CPU device (the one that commands should apply to)
	device_t *m_visiblecpu;

	text_buffer_ptr m_console_textbuf;
	text_buffer_ptr m_errorlog_textbuf;

	std::set<debug_command, debug_command::compare> m_commandlist;

	std::unique_ptr<std::istream> m_source_file;        // script source file
	std::unique_ptr<emu_file> m_logfile;                // logfile for debug console output
};

#endif // MAME_EMU_DEBUG_DEBUGCON_H
