// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcon.cpp

    Debugger console engine.

*********************************************************************/

#include "emu.h"
#include "debugcon.h"

#include "debugcpu.h"
#include "debugvw.h"
#include "textbuf.h"

#include "debugger.h"

#include "corestr.h"

#include <cctype>
#include <fstream>
#include <iterator>


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
	, m_visiblecpu(nullptr)
	, m_console_textbuf(nullptr)
	, m_errorlog_textbuf(nullptr)
	, m_logfile(nullptr)
{
	/* allocate text buffers */
	m_console_textbuf = text_buffer_alloc(CONSOLE_BUF_SIZE, CONSOLE_MAX_LINES);
	if (!m_console_textbuf)
		return;

	m_errorlog_textbuf = text_buffer_alloc(ERRORLOG_BUF_SIZE, ERRORLOG_MAX_LINES);
	if (!m_errorlog_textbuf)
		return;

	/* due to initialization order, @machine is holding our debug.log handle */
	m_logfile = machine.steal_debuglogfile();

	/* print the opening lines */
	printf("%s debugger version %s\n", emulator_info::get_appname(), emulator_info::get_build_version());
	printf("Currently targeting %s (%s)\n", m_machine.system().name, m_machine.system().type.fullname());

	/* request callback upon exiting */
	m_machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&debugger_console::exit, this));

	/* listen in on the errorlog */
	using namespace std::placeholders;
	m_machine.add_logerror_callback(std::bind(&debugger_console::errorlog_write_line, this, _1));

	/* register our own custom-command help */
	register_command("helpcustom", CMDFLAG_NONE, 0, 0, std::bind(&debugger_console::execute_help_custom, this, _1));
	register_command("condump", CMDFLAG_NONE, 1, 1, std::bind(&debugger_console::execute_condump, this, _1));

	/* first CPU is visible by default */
	for (device_t &device : device_enumerator(m_machine.root_device()))
	{
		auto *cpu = dynamic_cast<cpu_device *>(&device);
		if (cpu)
		{
			m_visiblecpu = cpu;
			break;
		}
	}
}


/*-------------------------------------------------
    exit - frees the console system
-------------------------------------------------*/

void debugger_console::exit()
{
	// free allocated memory
	m_console_textbuf.reset();
	m_errorlog_textbuf.reset();

	// free the command list
	m_commandlist.clear();

	// close the logfile, if any
	m_logfile.reset();
}



/***************************************************************************

    Command Handling

***************************************************************************/

inline bool debugger_console::debug_command::compare::operator()(const debug_command &a, const debug_command &b) const
{
	return core_stricmp(a.command.c_str(), b.command.c_str()) < 0;
}

inline bool debugger_console::debug_command::compare::operator()(const char *a, const debug_command &b) const
{
	return core_stricmp(a, b.command.c_str()) < 0;
}

inline bool debugger_console::debug_command::compare::operator()(const debug_command &a, const char *b) const
{
	return core_stricmp(a.command.c_str(), b) < 0;
}


debugger_console::debug_command::debug_command(const char *_command, u32 _flags, int _minparams, int _maxparams, std::function<void (const std::vector<std::string> &)> &&_handler)
	: command(_command), params(nullptr), help(nullptr), handler(std::move(_handler)), flags(_flags), minparams(_minparams), maxparams(_maxparams)
{
}


/*------------------------------------------------------------
    execute_help_custom - execute the helpcustom command
------------------------------------------------------------*/

void debugger_console::execute_help_custom(const std::vector<std::string> &params)
{
	char buf[64];
	for (const debug_command &cmd : m_commandlist)
	{
		if (cmd.flags & CMDFLAG_CUSTOM_HELP)
		{
			snprintf(buf, 63, "%s help", cmd.command.c_str());
			buf[63] = 0;
			char *temp_params[1] = { buf };
			internal_execute_command(true, 1, &temp_params[0]);
		}
	}
}

/*------------------------------------------------------------
    execute_condump - execute the condump command
------------------------------------------------------------*/

void debugger_console::execute_condump(const std::vector<std::string>& params)
{
	std::string filename = params[0];
	const char* mode;

	/* replace macros */
	strreplace(filename, "{game}", m_machine.basename());

	mode = "w";
	/* opening for append? */
	if (filename.length() >= 2 && filename[0] == '>' && filename[1] == '>')
	{
		mode = "a";
		filename = filename.substr(2);
	}

	FILE* f = fopen(filename.c_str(), mode);
	if (!f)
	{
		printf("Error opening file '%s'\n", filename);
		return;
	}

	for (std::string_view line_info : text_buffer_lines(*m_console_textbuf))
	{
		fwrite(line_info.data(), sizeof(char), line_info.length(), f);
		fputc('\n', f);
	}

	fclose(f);
	printf("Wrote console contents to '%s'\n", filename);
}

//-------------------------------------------------
//  visible_symtable - return the locally-visible
//  symbol table
//-------------------------------------------------

symbol_table &debugger_console::visible_symtable()
{
	return m_visiblecpu->debug()->symtable();
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
	// no params is an error
	if (params == 0)
		return CMDERR::none();

	// the first parameter has the command and the real first parameter; separate them
	char *p, *command;
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

	// search the command list
	size_t const len = strlen(command);
	auto const found = m_commandlist.lower_bound(command);

	// error if not found
	if ((m_commandlist.end() == found) || core_strnicmp(command, found->command.c_str(), len))
		return CMDERR::unknown_command(0);
	if (found->command.length() > len)
	{
		auto const next = std::next(found);
		if ((m_commandlist.end() != next) && !core_strnicmp(command, next->command.c_str(), len))
			return CMDERR::ambiguous_command(0);
	}

	// NUL-terminate and trim space around all the parameters
	for (int i = 1; i < params; i++)
		*param[i]++ = 0;

	// now go back and trim quotes and braces and any spaces they reveal
	for (int i = 0; i < params; i++)
		trim_parameter(&param[i], found->flags & CMDFLAG_KEEP_QUOTES);

	// see if we have the right number of parameters
	if (params < found->minparams)
		return CMDERR::not_enough_params(0);
	if (params > found->maxparams)
		return CMDERR::too_many_params(0);

	// execute the handler
	if (execute)
	{
		std::vector<std::string> params_vec(param, param + params);
		found->handler(params_vec);
	}
	return CMDERR::none();
}


/*-------------------------------------------------
    internal_parse_command - parses a command
    and either executes or just validates it
-------------------------------------------------*/

CMDERR debugger_console::internal_parse_command(const std::string &original_command, bool execute)
{
	char command[MAX_COMMAND_LENGTH], parens[MAX_COMMAND_LENGTH];
	char *params[MAX_COMMAND_PARAMS] = { nullptr };
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
					case ')':   if (parendex == 0 || parens[--parendex] != '(') return CMDERR::unbalanced_parens(p - command); break;
					case ']':   if (parendex == 0 || parens[--parendex] != '[') return CMDERR::unbalanced_parens(p - command); break;
					case '}':   if (parendex == 0 || parens[--parendex] != '{') return CMDERR::unbalanced_parens(p - command); break;
					case ',':   if (parendex == 0) params[paramcount++] = p; break;
					case ';':   if (parendex == 0) foundend = true; break;
					case '-':   if (parendex == 0 && paramcount == 1 && p[1] == '-') isexpr = true; *p = c; break;
					case '+':   if (parendex == 0 && paramcount == 1 && p[1] == '+') isexpr = true; *p = c; break;
					case '=':   if (parendex == 0 && paramcount == 1) isexpr = true; *p = c; break;
					case 0:     foundend = true; break;
					default:    *p = c; break;
				}
			}
		}

		/* check for unbalanced parentheses or quotes */
		if (instring)
			return CMDERR::unbalanced_quotes(p - command);
		if (parendex != 0)
			return CMDERR::unbalanced_parens(p - command);

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
				parsed_expression(visible_symtable(), command_start).execute();
			}
			catch (expression_error &err)
			{
				return CMDERR::expression_error(err);
			}
		}
		else
		{
			const CMDERR result = internal_execute_command(execute, paramcount, &params[0]);
			if (result.error_class() != CMDERR::NONE)
				return CMDERR(result.error_class(), command_start - command);
		}
	}
	return CMDERR::none();
}


/*-------------------------------------------------
    execute_command - execute a command string
-------------------------------------------------*/

CMDERR debugger_console::execute_command(const std::string &command, bool echo)
{
	/* echo if requested */
	if (echo)
		printf(">%s\n", command);

	/* parse and execute */
	const CMDERR result = internal_parse_command(command, true);

	/* display errors */
	if (result.error_class() != CMDERR::NONE)
	{
		if (!echo)
			printf(">%s\n", command);
		printf(" %*s^\n", result.error_offset(), "");
		printf("%s\n", cmderr_to_string(result));
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

void debugger_console::register_command(const char *command, u32 flags, int minparams, int maxparams, std::function<void (const std::vector<std::string> &)> &&handler)
{
	if (m_machine.phase() != machine_phase::INIT)
		throw emu_fatalerror("Can only call debugger_console::register_command() at init time!");
	if (!(m_machine.debug_flags & DEBUG_FLAG_ENABLED))
		throw emu_fatalerror("Cannot call debugger_console::register_command() when debugger is not running");

	auto const ins = m_commandlist.emplace(command, flags, minparams, maxparams, std::move(handler));
	if (!ins.second)
		osd_printf_error("error: Duplicate debugger command %s registered\n", command);
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
		buf = strtrimrightspace(buf);

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
	const int offset = error.error_offset();
	switch (error.error_class())
	{
		case CMDERR::UNKNOWN_COMMAND:       return "unknown command";
		case CMDERR::AMBIGUOUS_COMMAND:     return "ambiguous command";
		case CMDERR::UNBALANCED_PARENS:     return "unbalanced parentheses";
		case CMDERR::UNBALANCED_QUOTES:     return "unbalanced quotes";
		case CMDERR::NOT_ENOUGH_PARAMS:     return "not enough parameters for command";
		case CMDERR::TOO_MANY_PARAMS:       return "too many parameters for command";
		case CMDERR::EXPRESSION_ERROR:      return string_format("error in assignment expression: %s",
																 expression_error(static_cast<expression_error::error_code>(offset)).code_string());
		default:                            return "unknown error";
	}
}



/***************************************************************************

    Console Management

***************************************************************************/


//-------------------------------------------------
//  print_core - write preformatted text
//  to the debug console
//-------------------------------------------------

void debugger_console::print_core(std::string_view text)
{
	text_buffer_print(*m_console_textbuf, text);
	if (m_logfile)
		m_logfile->write(text.data(), text.length());
}

//-------------------------------------------------
//  print_core_wrap - write preformatted text
//  to the debug console, with wrapping
//-------------------------------------------------

void debugger_console::print_core_wrap(std::string_view text, int wrapcol)
{
	// FIXME: look into honoring wrapcol for the logfile
	text_buffer_print_wrap(*m_console_textbuf, text, wrapcol);
	if (m_logfile)
		m_logfile->write(text.data(), text.length());
}

//-------------------------------------------------
//  vprintf - vprintfs the given arguments using
//  the format to the debug console
//-------------------------------------------------

void debugger_console::vprintf(util::format_argument_pack<std::ostream> const &args)
{
	print_core(util::string_format(args));

	// force an update of any console views
	m_machine.debug_view().update_all(DVT_CONSOLE);
}

void debugger_console::vprintf(util::format_argument_pack<std::ostream> &&args)
{
	print_core(util::string_format(std::move(args)));

	// force an update of any console views
	m_machine.debug_view().update_all(DVT_CONSOLE);
}


//-------------------------------------------------
//  vprintf_wrap - vprintfs the given arguments
//  using the format to the debug console
//-------------------------------------------------

void debugger_console::vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> const &args)
{
	print_core_wrap(util::string_format(args), wrapcol);

	// force an update of any console views
	m_machine.debug_view().update_all(DVT_CONSOLE);
}

void debugger_console::vprintf_wrap(int wrapcol, util::format_argument_pack<std::ostream> &&args)
{
	print_core_wrap(util::string_format(std::move(args)), wrapcol);

	// force an update of any console views
	m_machine.debug_view().update_all(DVT_CONSOLE);
}


//-------------------------------------------------
//  errorlog_write_line - writes a line to the
//  errorlog ring buffer
//-------------------------------------------------

void debugger_console::errorlog_write_line(const char *line)
{
	if (m_errorlog_textbuf)
		text_buffer_print(*m_errorlog_textbuf, line);

	// force an update of any log views
	m_machine.debug_view().update_all(DVT_LOG);
}
