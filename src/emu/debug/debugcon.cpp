// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcon.c

    Debugger console engine.

*********************************************************************/

#include "emu.h"
#include "debugcon.h"
#include "debugcpu.h"
#include "debugvw.h"
#include "textbuf.h"
#include "debugger.h"
#include <ctype.h>
#include <fstream>

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CONSOLE_BUF_SIZE    (1024 * 1024)
#define CONSOLE_MAX_LINES   (CONSOLE_BUF_SIZE / 20)

#define ERRORLOG_BUF_SIZE   (1024 * 1024)
#define ERRORLOG_MAX_LINES  (ERRORLOG_BUF_SIZE / 20)

/***************************************************************************

    Initialization and tear down

***************************************************************************/

debugger_console::debugger_console(running_machine &machine)
	: m_machine(machine)
	, m_console_textbuf(nullptr)
	, m_errorlog_textbuf(nullptr)
	, m_commandlist(nullptr)
{
	/* allocate text buffers */
	m_console_textbuf = text_buffer_alloc(CONSOLE_BUF_SIZE, CONSOLE_MAX_LINES);
	if (!m_console_textbuf)
		return;

	m_errorlog_textbuf = text_buffer_alloc(ERRORLOG_BUF_SIZE, ERRORLOG_MAX_LINES);
	if (!m_errorlog_textbuf)
		return;

	/* print the opening lines */
	printf("%s debugger version %s\n", emulator_info::get_appname(), emulator_info::get_build_version());
	printf("Currently targeting %s (%s)\n", m_machine.system().name, m_machine.system().type.fullname());

	/* request callback upon exiting */
	m_machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&debugger_console::exit, this));

	/* listen in on the errorlog */
	using namespace std::placeholders;
	m_machine.add_logerror_callback(std::bind(&debugger_console::errorlog_write_line, this, _1));

	/* register our own custom-command help */
	register_command("helpcustom", CMDFLAG_NONE, 0, 0, 0, std::bind(&debugger_console::execute_help_custom, this, _1, _2));
}


/*-------------------------------------------------
    exit - frees the console system
-------------------------------------------------*/

void debugger_console::exit()
{
	/* free allocated memory */
	if (m_console_textbuf)
	{
		text_buffer_free(m_console_textbuf);
	}
	m_console_textbuf = nullptr;

	if (m_errorlog_textbuf)
	{
		text_buffer_free(m_errorlog_textbuf);
	}
	m_errorlog_textbuf = nullptr;

	/* free the command list */
	m_commandlist = nullptr;
}



/***************************************************************************

    Command Handling

***************************************************************************/

/*------------------------------------------------------------
    execute_help_custom - execute the helpcustom command
------------------------------------------------------------*/

void debugger_console::execute_help_custom(int ref, const std::vector<std::string> &params)
{
	debug_command *cmd = m_commandlist;
	char buf[64];
	while (cmd)
	{
		if (cmd->flags & CMDFLAG_CUSTOM_HELP)
		{
			snprintf(buf, 63, "%s help", cmd->command);
			buf[63] = 0;
			char *temp_params[1] = { buf };
			internal_execute_command(true, 1, &temp_params[0]);
		}
		cmd = cmd->next;
	}
}


/*-------------------------------------------------
    trim_parameter - executes a
    command
-------------------------------------------------*/

void debugger_console::trim_parameter(char **paramptr, bool keep_quotes)
{
	char *param = *paramptr;
	size_t len = strlen(param);
	bool repeat;

	/* loop until all adornments are gone */
	do
	{
		repeat = false;

		/* check for begin/end quotes */
		if (len >= 2 && param[0] == '"' && param[len - 1] == '"')
		{
			if (!keep_quotes)
			{
				param[len - 1] = 0;
				param++;
				len -= 2;
			}
		}

		/* check for start/end braces */
		else if (len >= 2 && param[0] == '{' && param[len - 1] == '}')
		{
			param[len - 1] = 0;
			param++;
			len -= 2;
			repeat = true;
		}

		/* check for leading spaces */
		else if (len >= 1 && param[0] == ' ')
		{
			param++;
			len--;
			repeat = true;
		}

		/* check for trailing spaces */
		else if (len >= 1 && param[len - 1] == ' ')
		{
			param[len - 1] = 0;
			len--;
			repeat = true;
		}
	} while (repeat);

	*paramptr = param;
}


/*-------------------------------------------------
    internal_execute_command - executes a
    command
-------------------------------------------------*/

CMDERR debugger_console::internal_execute_command(bool execute, int params, char **param)
{
	debug_command *cmd, *found = nullptr;
	int i, foundcount = 0;
	char *p, *command;
	size_t len;

	/* no params is an error */
	if (params == 0)
		return CMDERR_NONE;

	/* the first parameter has the command and the real first parameter; separate them */
	for (p = param[0]; *p && isspace(u8(*p)); p++) { }
	for (command = p; *p && !isspace(u8(*p)); p++) { }
	if (*p != 0)
	{
		*p++ = 0;
		for ( ; *p && isspace(u8(*p)); p++) { }
		if (*p != 0)
			param[0] = p;
		else
			params = 0;
	}
	else
	{
		params = 0;
		param[0] = nullptr;
	}

	/* search the command list */
	len = strlen(command);
	for (cmd = m_commandlist; cmd != nullptr; cmd = cmd->next)
		if (!strncmp(command, cmd->command, len))
		{
			foundcount++;
			found = cmd;
			if (strlen(cmd->command) == len)
			{
				foundcount = 1;
				break;
			}
		}

	/* error if not found */
	if (!found)
		return MAKE_CMDERR_UNKNOWN_COMMAND(0);
	if (foundcount > 1)
		return MAKE_CMDERR_AMBIGUOUS_COMMAND(0);

	/* NULL-terminate and trim space around all the parameters */
	for (i = 1; i < params; i++)
		*param[i]++ = 0;

	/* now go back and trim quotes and braces and any spaces they reveal*/
	for (i = 0; i < params; i++)
		trim_parameter(&param[i], found->flags & CMDFLAG_KEEP_QUOTES);

	/* see if we have the right number of parameters */
	if (params < found->minparams)
		return MAKE_CMDERR_NOT_ENOUGH_PARAMS(0);
	if (params > found->maxparams)
		return MAKE_CMDERR_TOO_MANY_PARAMS(0);

	/* execute the handler */
	if (execute)
	{
		std::vector<std::string> params_vec(param, param + params);
		found->handler(found->ref, params_vec);
	}
	return CMDERR_NONE;
}


/*-------------------------------------------------
    internal_parse_command - parses a command
    and either executes or just validates it
-------------------------------------------------*/

CMDERR debugger_console::internal_parse_command(const std::string &original_command, bool execute)
{
	char command[MAX_COMMAND_LENGTH], parens[MAX_COMMAND_LENGTH];
	char *params[MAX_COMMAND_PARAMS] = { nullptr };
	CMDERR result;
	char *command_start;
	char *p, c = 0;

	/* make a copy of the command */
	strcpy(command, original_command.c_str());

	/* loop over all semicolon-separated stuff */
	for (p = command; *p != 0; )
	{
		int paramcount = 0, parendex = 0;
		bool foundend = false, instring = false, isexpr = false;

		/* find a semicolon or the end */
		for (params[paramcount++] = p; !foundend; p++)
		{
			c = *p;
			if (instring)
			{
				if (c == '"' && p[-1] != '\\')
					instring = false;
			}
			else
			{
				switch (c)
				{
					case '"':   instring = true; break;
					case '(':
					case '[':
					case '{':   parens[parendex++] = c; break;
					case ')':   if (parendex == 0 || parens[--parendex] != '(') return MAKE_CMDERR_UNBALANCED_PARENS(p - command); break;
					case ']':   if (parendex == 0 || parens[--parendex] != '[') return MAKE_CMDERR_UNBALANCED_PARENS(p - command); break;
					case '}':   if (parendex == 0 || parens[--parendex] != '{') return MAKE_CMDERR_UNBALANCED_PARENS(p - command); break;
					case ',':   if (parendex == 0) params[paramcount++] = p; break;
					case ';':   if (parendex == 0) foundend = true; break;
					case '-':   if (parendex == 0 && paramcount == 1 && p[1] == '-') isexpr = true; *p = c; break;
					case '+':   if (parendex == 0 && paramcount == 1 && p[1] == '+') isexpr = true; *p = c; break;
					case '=':   if (parendex == 0 && paramcount == 1) isexpr = true; *p = c; break;
					case 0:     foundend = true; break;
					default:    *p = tolower(u8(c)); break;
				}
			}
		}

		/* check for unbalanced parentheses or quotes */
		if (instring)
			return MAKE_CMDERR_UNBALANCED_QUOTES(p - command);
		if (parendex != 0)
			return MAKE_CMDERR_UNBALANCED_PARENS(p - command);

		/* NULL-terminate if we ended in a semicolon */
		p--;
		if (c == ';') *p++ = 0;

		/* process the command */
		command_start = params[0];

		/* allow for "do" commands */
		if (tolower(u8(command_start[0])) == 'd' && tolower(u8(command_start[1])) == 'o' && isspace(u8(command_start[2])))
		{
			isexpr = true;
			command_start += 3;
		}

		/* if it smells like an assignment expression, treat it as such */
		if (isexpr && paramcount == 1)
		{
			try
			{
				u64 expresult;
				parsed_expression expression(m_machine.debugger().cpu().get_visible_symtable(), command_start, &expresult);
			}
			catch (expression_error &err)
			{
				return MAKE_CMDERR_EXPRESSION_ERROR(err);
			}
		}
		else
		{
			result = internal_execute_command(execute, paramcount, &params[0]);
			if (result != CMDERR_NONE)
				return MAKE_CMDERR(CMDERR_ERROR_CLASS(result), command_start - command);
		}
	}
	return CMDERR_NONE;
}


/*-------------------------------------------------
    execute_command - execute a command string
-------------------------------------------------*/

CMDERR debugger_console::execute_command(const std::string &command, bool echo)
{
	CMDERR result;

	/* echo if requested */
	if (echo)
		printf(">%s\n", command.c_str());

	/* parse and execute */
	result = internal_parse_command(command, true);

	/* display errors */
	if (result != CMDERR_NONE)
	{
		if (!echo)
			printf(">%s\n", command.c_str());
		printf(" %*s^\n", CMDERR_ERROR_OFFSET(result), "");
		printf("%s\n", cmderr_to_string(result).c_str());
	}

	/* update all views */
	if (echo)
	{
		m_machine.debug_view().update_all();
		m_machine.debugger().refresh_display();
	}
	return result;
}


/*-------------------------------------------------
    validate_command - validate a command string
-------------------------------------------------*/

CMDERR debugger_console::validate_command(const char *command)
{
	return internal_parse_command(command, false);
}


/*-------------------------------------------------
    register_command - register a command handler
-------------------------------------------------*/

void debugger_console::register_command(const char *command, u32 flags, int ref, int minparams, int maxparams, std::function<void(int, const std::vector<std::string> &)> handler)
{
	assert_always(m_machine.phase() == machine_phase::INIT, "Can only call register_command() at init time!");
	assert_always((m_machine.debug_flags & DEBUG_FLAG_ENABLED) != 0, "Cannot call register_command() when debugger is not running");

	debug_command *cmd = auto_alloc_clear(m_machine, <debug_command>());

	/* fill in the command */
	strcpy(cmd->command, command);
	cmd->flags = flags;
	cmd->ref = ref;
	cmd->minparams = minparams;
	cmd->maxparams = maxparams;
	cmd->handler = handler;

	/* link it */
	cmd->next = m_commandlist;
	m_commandlist = cmd;
}


//-------------------------------------------------
//  source_script - specifies a debug command
//  script to execute
//-------------------------------------------------

void debugger_console::source_script(const char *file)
{
	// close any existing source file
	m_source_file.reset();

	// open a new one if requested
	if (file != nullptr)
	{
		auto source_file = std::make_unique<std::ifstream>(file, std::ifstream::in);
		if (source_file->fail())
		{
			if (m_machine.phase() == machine_phase::RUNNING)
				printf("Cannot open command file '%s'\n", file);
			else
				fatalerror("Cannot open command file '%s'\n", file);
		}
		else
		{
			m_source_file = std::move(source_file);
		}
	}
}


//-------------------------------------------------
//  process_source_file - executes commands from
//  a source file
//-------------------------------------------------

void debugger_console::process_source_file()
{
	std::string buf;

	// loop until the file is exhausted or until we are executing again
	while (m_machine.debugger().cpu().is_stopped()
			&& m_source_file
			&& std::getline(*m_source_file, buf))
	{
		// strip out comments (text after '//')
		size_t pos = buf.find("//");
		if (pos != std::string::npos)
			buf.resize(pos);

		// strip whitespace
		strtrimrightspace(buf);

		// execute the command
		if (!buf.empty())
			execute_command(buf, true);
	}

	if (m_source_file && !m_source_file->good())
	{
		if (!m_source_file->eof())
			printf("I/O error, script processing terminated\n");
		m_source_file.reset();
	}
}



/***************************************************************************

    Error Handling

***************************************************************************/

/*-------------------------------------------------
    cmderr_to_string - return a friendly string
    for a given command error
-------------------------------------------------*/

std::string debugger_console::cmderr_to_string(CMDERR error)
{
	int offset = CMDERR_ERROR_OFFSET(error);
	switch (CMDERR_ERROR_CLASS(error))
	{
		case CMDERR_UNKNOWN_COMMAND:        return "unknown command";
		case CMDERR_AMBIGUOUS_COMMAND:      return "ambiguous command";
		case CMDERR_UNBALANCED_PARENS:      return "unbalanced parentheses";
		case CMDERR_UNBALANCED_QUOTES:      return "unbalanced quotes";
		case CMDERR_NOT_ENOUGH_PARAMS:      return "not enough parameters for command";
		case CMDERR_TOO_MANY_PARAMS:        return "too many parameters for command";
		case CMDERR_EXPRESSION_ERROR:       return string_format("error in assignment expression: %s",
																 expression_error(static_cast<expression_error::error_code>(offset)).code_string());
		default:                            return "unknown error";
	}
}



/***************************************************************************

    Console Management

***************************************************************************/

/*-------------------------------------------------
    vprintf - vprintfs the given arguments using
    the format to the debug console
-------------------------------------------------*/

void debugger_console::vprintf(util::format_argument_pack<std::ostream> const &args)
{
	text_buffer_print(m_console_textbuf, util::string_format(args).c_str());

	/* force an update of any console views */
	m_machine.debug_view().update_all(DVT_CONSOLE);
}

void debugger_console::vprintf(util::format_argument_pack<std::ostream> &&args)
{
	text_buffer_print(m_console_textbuf, util::string_format(std::move(args)).c_str());

	/* force an update of any console views */
	m_machine.debug_view().update_all(DVT_CONSOLE);
}


/*-------------------------------------------------
    vprintf_wrap - vprintfs the given arguments
    using the format to the debug console
-------------------------------------------------*/

void debugger_console::vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> const &args)
{
	text_buffer_print_wrap(m_console_textbuf, util::string_format(args).c_str(), wrapcol);

	/* force an update of any console views */
	m_machine.debug_view().update_all(DVT_CONSOLE);
}

void debugger_console::vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> &&args)
{
	text_buffer_print_wrap(m_console_textbuf, util::string_format(std::move(args)).c_str(), wrapcol);

	/* force an update of any console views */
	m_machine.debug_view().update_all(DVT_CONSOLE);
}


/*-------------------------------------------------
    errorlog_write_line - writes a line to the
    errorlog ring buffer
-------------------------------------------------*/

void debugger_console::errorlog_write_line(const char *line)
{
	if (m_errorlog_textbuf)
	{
		text_buffer_print(m_errorlog_textbuf, line);
	}

	/* force an update of any log views */
	m_machine.debug_view().update_all(DVT_LOG);
}
