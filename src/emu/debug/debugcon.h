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

/* flags for command parsing */
#define CMDFLAG_NONE                        (0x0000)
#define CMDFLAG_KEEP_QUOTES                 (0x0001)
#define CMDFLAG_CUSTOM_HELP                 (0x0002)

/* values for the error code in a command error */
#define CMDERR_NONE                         (0)
#define CMDERR_UNKNOWN_COMMAND              (1)
#define CMDERR_AMBIGUOUS_COMMAND            (2)
#define CMDERR_UNBALANCED_PARENS            (3)
#define CMDERR_UNBALANCED_QUOTES            (4)
#define CMDERR_NOT_ENOUGH_PARAMS            (5)
#define CMDERR_TOO_MANY_PARAMS              (6)
#define CMDERR_EXPRESSION_ERROR             (7)

/* parameter separator macros */
#define CMDPARAM_SEPARATOR                  "\0"
#define CMDPARAM_TERMINATOR                 "\0\0"



/***************************************************************************
    MACROS
***************************************************************************/

/* command error assembly/disassembly macros */
#define CMDERR_ERROR_CLASS(x)               ((x) >> 16)
#define CMDERR_ERROR_OFFSET(x)              ((x) & 0xffff)
#define MAKE_CMDERR(a,b)                    (((a) << 16) | ((b) & 0xffff))

/* macros to assemble specific error conditions */
#define MAKE_CMDERR_UNKNOWN_COMMAND(x)      MAKE_CMDERR(CMDERR_UNKNOWN_COMMAND, (x))
#define MAKE_CMDERR_AMBIGUOUS_COMMAND(x)    MAKE_CMDERR(CMDERR_AMBIGUOUS_COMMAND, (x))
#define MAKE_CMDERR_UNBALANCED_PARENS(x)    MAKE_CMDERR(CMDERR_UNBALANCED_PARENS, (x))
#define MAKE_CMDERR_UNBALANCED_QUOTES(x)    MAKE_CMDERR(CMDERR_UNBALANCED_QUOTES, (x))
#define MAKE_CMDERR_NOT_ENOUGH_PARAMS(x)    MAKE_CMDERR(CMDERR_NOT_ENOUGH_PARAMS, (x))
#define MAKE_CMDERR_TOO_MANY_PARAMS(x)      MAKE_CMDERR(CMDERR_TOO_MANY_PARAMS, (x))
#define MAKE_CMDERR_EXPRESSION_ERROR(x)     MAKE_CMDERR(CMDERR_EXPRESSION_ERROR, (x))


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* CMDERR is an error code for command evaluation */
typedef u32 CMDERR;

class debugger_console
{
public:
	debugger_console(running_machine &machine);

	/* command handling */
	CMDERR          execute_command(const std::string &command, bool echo);
	CMDERR          validate_command(const char *command);
	void            register_command(const char *command, u32 flags, int ref, int minparams, int maxparams, std::function<void(int, const std::vector<std::string> &)> handler);
	void            source_script(const char *file);
	void            process_source_file();

	/* console management */
	void            vprintf(util::format_argument_pack<std::ostream> const &args);
	void            vprintf(util::format_argument_pack<std::ostream> &&args);
	void            vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> const &args);
	void            vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> &&args);
	text_buffer *   get_console_textbuf() const { return m_console_textbuf; }

	/* errorlog management */
	void            errorlog_write_line(const char *line);
	text_buffer *   get_errorlog_textbuf() const { return m_errorlog_textbuf; }

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

	static std::string cmderr_to_string(CMDERR error);

private:
	void exit();

	void execute_help_custom(int ref, const std::vector<std::string> &params);

	void trim_parameter(char **paramptr, bool keep_quotes);
	CMDERR internal_execute_command(bool execute, int params, char **param);
	CMDERR internal_parse_command(const std::string &original_command, bool execute);

	struct debug_command
	{
		debug_command * next;
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

	text_buffer     *m_console_textbuf;
	text_buffer     *m_errorlog_textbuf;

	debug_command   *m_commandlist;

	std::unique_ptr<std::istream> m_source_file;        // script source file
};

#endif // MAME_EMU_DEBUG_DEBUGCON_H
