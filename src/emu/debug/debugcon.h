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


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_COMMAND_LENGTH                  4096
#define MAX_COMMAND_PARAMS                  128

// flags for command parsing
constexpr u32 CMDFLAG_NONE                  = 0x0000;
constexpr u32 CMDFLAG_KEEP_QUOTES           = 0x0001;
constexpr u32 CMDFLAG_CUSTOM_HELP           = 0x0002;

/* parameter separator macros */
#define CMDPARAM_SEPARATOR                  "\0"
#define CMDPARAM_TERMINATOR                 "\0\0"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// CMDERR is an error code for command evaluation
struct CMDERR
{
	// values for the error code in a command error
	static constexpr u32 NONE               = 0;
	static constexpr u32 UNKNOWN_COMMAND    = 1;
	static constexpr u32 AMBIGUOUS_COMMAND  = 2;
	static constexpr u32 UNBALANCED_PARENS  = 3;
	static constexpr u32 UNBALANCED_QUOTES  = 4;
	static constexpr u32 NOT_ENOUGH_PARAMS  = 5;
	static constexpr u32 TOO_MANY_PARAMS    = 6;
	static constexpr u32 EXPRESSION_ERROR   = 7;

	u32 val;

	// command error assembly/disassembly
	constexpr CMDERR(u32 a, u32 b) : val((a << 16) | (b & 0xffff)) { }
	constexpr u32 ERROR_CLASS() const { return val >> 16; }
	constexpr u32 ERROR_OFFSET() const { return val & 0xffff; }

	// assemble specific error conditions
	static constexpr CMDERR MAKE_UNKNOWN_COMMAND(u32 x)    { return CMDERR(UNKNOWN_COMMAND, x); }
	static constexpr CMDERR MAKE_AMBIGUOUS_COMMAND(u32 x)  { return CMDERR(AMBIGUOUS_COMMAND, x); }
	static constexpr CMDERR MAKE_UNBALANCED_PARENS(u32 x)  { return CMDERR(UNBALANCED_PARENS, x); }
	static constexpr CMDERR MAKE_UNBALANCED_QUOTES(u32 x)  { return CMDERR(UNBALANCED_QUOTES, x); }
	static constexpr CMDERR MAKE_NOT_ENOUGH_PARAMS(u32 x)  { return CMDERR(NOT_ENOUGH_PARAMS, x); }
	static constexpr CMDERR MAKE_TOO_MANY_PARAMS(u32 x)    { return CMDERR(TOO_MANY_PARAMS, x); }
	static constexpr CMDERR MAKE_EXPRESSION_ERROR(u32 x)   { return CMDERR(EXPRESSION_ERROR, x); }
};


class debugger_console
{
public:
	debugger_console(running_machine &machine);

	// command handling
	CMDERR          execute_command(const std::string &command, bool echo);
	CMDERR          validate_command(const char *command);
	void            register_command(const char *command, u32 flags, int ref, int minparams, int maxparams, std::function<void(int, const std::vector<std::string> &)> handler);
	void            source_script(const char *file);
	void            process_source_file();

	// console management
	void vprintf(util::format_argument_pack<std::ostream> const &args);
	void vprintf(util::format_argument_pack<std::ostream> &&args);
	void vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> const &args);
	void vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> &&args);
	text_buffer &get_console_textbuf() const { return *m_console_textbuf; }

	/* errorlog management */
	void errorlog_write_line(const char *line);
	text_buffer &get_errorlog_textbuf() const { return *m_errorlog_textbuf; }

	/* convenience templates */
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

	void execute_help_custom(int ref, const std::vector<std::string> &params);
	void execute_condump(int ref, const std::vector<std::string>& params);

	void trim_parameter(char **paramptr, bool keep_quotes);
	CMDERR internal_execute_command(bool execute, int params, char **param);
	CMDERR internal_parse_command(const std::string &original_command, bool execute);

	void print_core(const char *text);                   // core text output
	void print_core_wrap(const char *text, int wrapcol); // core text output

	struct debug_command
	{
		debug_command(const char *_command, u32 _flags, int _ref, int _minparams, int _maxparams, std::function<void(int, const std::vector<std::string> &)> _handler);

		char            command[32];
		const char *    params;
		const char *    help;
		std::function<void(int, const std::vector<std::string> &)> handler;
		u32             flags;
		int             ref;
		int             minparams;
		int             maxparams;
	};

	running_machine &m_machine;

	// visible CPU device (the one that commands should apply to)
	device_t *m_visiblecpu;

	text_buffer_ptr m_console_textbuf;
	text_buffer_ptr m_errorlog_textbuf;

	std::forward_list<debug_command> m_commandlist;

	std::unique_ptr<std::istream> m_source_file;        // script source file
	std::unique_ptr<emu_file> m_logfile;                // logfile for debug console output
};

#endif // MAME_EMU_DEBUG_DEBUGCON_H
