// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
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
#include "fileio.h"
#include "main.h"

#include "corestr.h"

#include <cctype>
#include <fstream>
#include <iterator>
#include <locale>
#include <regex>


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

debugger_console::~debugger_console()
{
}


//-------------------------------------------------
//  exit - frees the console system
//-------------------------------------------------

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
	return a.command < b.command;
}

inline bool debugger_console::debug_command::compare::operator()(const char *a, const debug_command &b) const
{
	return strcmp(a, b.command.c_str()) < 0;
}

inline bool debugger_console::debug_command::compare::operator()(const debug_command &a, const char *b) const
{
	return strcmp(a.command.c_str(), b) < 0;
}


debugger_console::debug_command::debug_command(std::string_view _command, u32 _flags, int _minparams, int _maxparams, std::function<void (const std::vector<std::string_view> &)> &&_handler)
	: command(_command), params(nullptr), help(nullptr), handler(std::move(_handler)), flags(_flags), minparams(_minparams), maxparams(_maxparams)
{
}


//------------------------------------------------------------
//  execute_help_custom - execute the helpcustom command
//------------------------------------------------------------

void debugger_console::execute_help_custom(const std::vector<std::string_view> &params)
{
	for (const debug_command &cmd : m_commandlist)
	{
		if (cmd.flags & CMDFLAG_CUSTOM_HELP)
		{
			std::string buf = cmd.command + " help";
			std::vector<std::string_view> temp_params = { buf };
			internal_execute_command(true, temp_params);
		}
	}
}

/*------------------------------------------------------------
    execute_condump - execute the condump command
------------------------------------------------------------*/

void debugger_console::execute_condump(const std::vector<std::string_view>& params)
{
	std::string filename(params[0]);
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

symbol_table &debugger_console::visible_symtable() const
{
	return m_visiblecpu->debug()->symtable();
}



//-------------------------------------------------
//  trim_parameter - trim spaces and quotes around
//  a command parameter
//-------------------------------------------------

std::string_view debugger_console::trim_parameter(std::string_view param, bool keep_quotes)
{
	std::string_view::size_type len = param.length();
	bool repeat;

	// loop until all adornments are gone
	do
	{
		repeat = false;

		// check for begin/end quotes
		if (len >= 2 && param[0] == '"' && param[len - 1] == '"')
		{
			if (!keep_quotes)
			{
				param = param.substr(1, len - 2);
				len -= 2;
			}
		}

		// check for start/end braces
		else if (len >= 2 && param[0] == '{' && param[len - 1] == '}')
		{
			param = param.substr(1, len - 2);
			len -= 2;
			repeat = true;
		}

		// check for leading spaces
		else if (len >= 1 && param[0] == ' ')
		{
			param.remove_prefix(1);
			len--;
			repeat = true;
		}

		// check for trailing spaces
		else if (len >= 1 && param[len - 1] == ' ')
		{
			param.remove_suffix(1);
			len--;
			repeat = true;
		}
	} while (repeat);

	return param;
}


//-------------------------------------------------
//  internal_execute_command - executes a
//  command
//-------------------------------------------------

CMDERR debugger_console::internal_execute_command(bool execute, std::vector<std::string_view> &params)
{
	// no params is an error
	if (params.empty())
		return CMDERR::none();

	// the first parameter has the command and the real first parameter; separate them
	std::string_view command_param = params[0];
	std::string_view::size_type pos = 0;
	while (pos < command_param.length() && !isspace(u8(command_param[pos])))
		pos++;
	const std::string command(strmakelower(command_param.substr(0, pos)));
	while (pos < command_param.length() && isspace(u8(command_param[pos])))
		pos++;
	if (pos == command_param.length() && params.size() == 1)
		params.clear();
	else
		params[0].remove_prefix(pos);

	// search the command list
	auto const found = m_commandlist.lower_bound(command.c_str());

	// error if not found
	if (m_commandlist.end() == found || std::string_view(command) != std::string_view(found->command).substr(0, command.length()))
		return CMDERR::unknown_command(0);
	if (found->command.length() > command.length())
	{
		auto const next = std::next(found);
		if (m_commandlist.end() != next && std::string_view(command) == std::string_view(next->command).substr(0, command.length()))
			return CMDERR::ambiguous_command(0);
	}

	// now go back and trim quotes and braces and any spaces they reveal
	for (std::string_view &param : params)
		param = trim_parameter(param, found->flags & CMDFLAG_KEEP_QUOTES);

	// see if we have the right number of parameters
	if (params.size() < found->minparams)
		return CMDERR::not_enough_params(0);
	if (params.size() > found->maxparams)
		return CMDERR::too_many_params(0);

	// execute the handler
	if (execute)
		found->handler(params);
	return CMDERR::none();
}


//-------------------------------------------------
//  internal_parse_command - parses a command
//  and either executes or just validates it
//-------------------------------------------------

CMDERR debugger_console::internal_parse_command(std::string_view command, bool execute)
{
	std::string_view::size_type pos = 0;
	std::string_view::size_type len = command.length();

	while (pos < len)
	{
		std::string parens;
		std::vector<std::string_view> params;
		bool foundend = false, instring = false, isexpr = false;

		// skip leading spaces
		while (pos < len && isspace(u8(command[pos])))
			pos++;
		std::string_view::size_type startpos = pos;

		// find a semicolon or the end
		for (params.push_back(command.substr(pos)); !foundend && pos < len; pos++)
		{
			char c = command[pos];
			if (instring)
			{
				if (c == '"' && command[pos - 1] != '\\')
					instring = false;
			}
			else
			{
				switch (c)
				{
					case '"':   instring = true; break;
					case '(':
					case '[':
					case '{':   parens.push_back(c); break;
					case ')':   if (parens.empty() || parens.back() != '(') return CMDERR::unbalanced_parens(pos); parens.pop_back(); break;
					case ']':   if (parens.empty() || parens.back() != '[') return CMDERR::unbalanced_parens(pos); parens.pop_back(); break;
					case '}':   if (parens.empty() || parens.back() != '{') return CMDERR::unbalanced_parens(pos); parens.pop_back(); break;
					case ',':   if (parens.empty()) { params.back().remove_suffix(len - pos); params.push_back(command.substr(pos + 1)); } break;
					case ';':   if (parens.empty()) { params.back().remove_suffix(len - pos); foundend = true; } break;
					case '-':   if (parens.empty() && params.size() == 1 && pos > 0 && command[pos - 1] == '-') isexpr = true; break;
					case '+':   if (parens.empty() && params.size() == 1 && pos > 0 && command[pos - 1] == '+') isexpr = true; break;
					case '=':   if (parens.empty() && params.size() == 1) isexpr = true; break;
					default:    break;
				}
			}
		}

		// check for unbalanced parentheses or quotes
		if (instring)
			return CMDERR::unbalanced_quotes(pos);
		if (!parens.empty())
			return CMDERR::unbalanced_parens(pos);

		// process the command
		std::string_view command_or_expr = params[0];

		// allow for "do" commands
		if (command_or_expr.length() > 3 && tolower(u8(command_or_expr[0])) == 'd' && tolower(u8(command_or_expr[1])) == 'o' && isspace(u8(command_or_expr[2])))
		{
			isexpr = true;
			command_or_expr.remove_prefix(3);
		}

		// if it smells like an assignment expression, treat it as such
		if (isexpr && params.size() == 1)
		{
			try
			{
				parsed_expression expr(visible_symtable(), command_or_expr);
				if (execute)
					expr.execute();
			}
			catch (expression_error &err)
			{
				return CMDERR::expression_error(err);
			}
		}
		else
		{
			const CMDERR result = internal_execute_command(execute, params);
			if (result.error_class() != CMDERR::NONE)
				return CMDERR(result.error_class(), startpos);
		}
	}
	return CMDERR::none();
}


//-------------------------------------------------
//  execute_command - execute a command string
//-------------------------------------------------

CMDERR debugger_console::execute_command(std::string_view command, bool echo)
{
	// echo if requested
	if (echo)
		printf(">%s\n", command);

	// parse and execute
	const CMDERR result = internal_parse_command(command, true);

	// display errors
	if (result.error_class() != CMDERR::NONE)
	{
		if (!echo)
			printf(">%s\n", command);
		printf(" %*s^\n", result.error_offset(), "");
		printf("%s\n", cmderr_to_string(result));
	}

	// update all views
	if (echo)
	{
		m_machine.debug_view().update_all();
		m_machine.debugger().refresh_display();
	}
	return result;
}


//-------------------------------------------------
//  validate_command - validate a command string
//-------------------------------------------------

CMDERR debugger_console::validate_command(std::string_view command)
{
	return internal_parse_command(command, false);
}


/*-------------------------------------------------
    register_command - register a command handler
-------------------------------------------------*/

void debugger_console::register_command(std::string_view command, u32 flags, int minparams, int maxparams, std::function<void (const std::vector<std::string_view> &)> &&handler)
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
			source_file->imbue(std::locale::classic());
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
		case CMDERR::EXPRESSION_ERROR:      return string_format(std::locale::classic(), "error in assignment expression: %s",
																 expression_error(static_cast<expression_error::error_code>(offset)).code_string());
		default:                            return "unknown error";
	}
}


//**************************************************************************
//  PARAMETER VALIDATION HELPERS
//**************************************************************************

namespace {

template <typename T>
inline std::string_view::size_type find_delimiter(std::string_view str, T &&is_delim)
{
	unsigned parens = 0;
	for (std::string_view::size_type i = 0; str.length() > i; ++i)
	{
		if (str[i] == '(')
		{
			++parens;
		}
		else if (parens)
		{
			if (str[i] == ')')
				--parens;
		}
		else if (is_delim(str[i]))
		{
			return i;
		}
	}
	return std::string_view::npos;
}

} // anonymous namespace


/// \brief Validate parameter as a Boolean value
///
/// Validates a parameter as a Boolean value.  Fixed strings and
/// expressions evaluating to numeric values are recognised.  The result
/// is unchanged for an empty string.
/// \param [in] param The parameter string.
/// \param [in,out] result The default value on entry, and the value of
///   the parameter interpreted as a Boolean on success.  Unchanged if
///   the parameter is an empty string.
/// \return true if the parameter is a valid Boolean value or an empty
///   string, or false otherwise.
bool debugger_console::validate_boolean_parameter(std::string_view param, bool &result)
{
	// nullptr parameter does nothing and returns no error
	if (param.empty())
		return true;

	// evaluate the expression; success if no error
	using namespace std::literals;
	bool const is_true = util::streqlower(param, "true"sv);
	bool const is_false = util::streqlower(param, "false"sv);

	if (is_true || is_false)
	{
		result = is_true;
		return true;
	}

	// try to evaluate as a number
	u64 val;
	if (!validate_number_parameter(param, val))
		return false;

	result = val != 0;
	return true;
}


/// \brief Validate parameter as a numeric value
///
/// Parses the parameter as an expression and evaluates it as a number.
/// \param [in] param The parameter string.
/// \param [out] result The numeric value of the expression on success.
///   Unchanged on failure.
/// \return true if the parameter is a valid expression that evaluates
///   to a numeric value, or false otherwise.
bool debugger_console::validate_number_parameter(std::string_view param, u64 &result)
{
	// evaluate the expression; success if no error
	try
	{
		result = parsed_expression(visible_symtable(), param).execute();
		return true;
	}
	catch (expression_error const &error)
	{
		// print an error pointing to the character that caused it
		printf("Error in expression: %s\n", param);
		printf("                     %*s^", error.offset(), "");
		printf("%s\n", error.code_string());
		return false;
	}
}

bool debugger_console::validate_number_parameter(std::string_view param, offs_t &result)
{
	u64 res;
	bool ret = validate_number_parameter(param, res);
	result = res;
	return ret;
}


/// \brief Validate parameter as a device
///
/// Validates a parameter as a device identifier and retrieves the
/// device on success.  A string corresponding to the tag of a device
/// refers to that device; an empty string refers to the current CPU
/// with debugger focus; any other string is parsed as an expression
/// and treated as an index of a device implementing
/// #device_execute_interface and #device_state_interface, and exposing
/// a generic PC base value.
/// \param [in] param The parameter string.
/// \param [out] result A pointer to the device on success, or unchanged
///   on failure.
/// \return true if the parameter refers to a device in the current
///   system, or false otherwise.
bool debugger_console::validate_device_parameter(std::string_view param, device_t *&result)
{
	// if no parameter, use the visible CPU
	if (param.empty())
	{
		device_t *const current = m_visiblecpu;
		if (current)
		{
			result = current;
			return true;
		}
		else
		{
			printf("No valid CPU is currently selected\n");
			return false;
		}
	}

	// next look for a tag match
	std::string_view relative = param;
	device_t &base = get_device_search_base(relative);
	device_t *device = base.subdevice(strmakelower(relative));
	if (device)
	{
		result = device;
		return true;
	}

	// then evaluate as an expression; on an error assume it was a tag
	u64 cpunum;
	try
	{
		cpunum = parsed_expression(visible_symtable(), param).execute();
	}
	catch (expression_error &)
	{
		printf("Unable to find device '%s'\n", param);
		return false;
	}

	// attempt to find by numerical index
	device = get_cpu_by_index(cpunum);
	if (device)
	{
		result = device;
		return true;
	}
	else
	{
		// if out of range, complain
		printf("Invalid CPU index %u\n", cpunum);
		return false;
	}
}


/// \brief Validate a parameter as a CPU
///
/// Validates a parameter as a CPU identifier.  Uses the same rules as
/// #validate_device_parameter to identify devices, but additionally
/// checks that the device is a "CPU" for the debugger's purposes.
/// \param [in] The parameter string.
/// \param [out] result The device on success, or unchanged on failure.
/// \return true if the parameter refers to a CPU-like device in the
///   current system, or false otherwise.
bool debugger_console::validate_cpu_parameter(std::string_view param, device_t *&result)
{
	// first do the standard device thing
	device_t *device;
	if (!validate_device_parameter(param, device))
		return false;

	// check that it's a "CPU" for the debugger's purposes
	device_execute_interface const *execute;
	if (device->interface(execute))
	{
		result = device;
		return true;
	}

	printf("Device %s is not a CPU\n", device->name());
	return false;
}


/// \brief Validate a parameter as an address space identifier
///
/// Validates a parameter as an address space identifier.  Uses the same
/// rules as #validate_device_parameter to identify devices.  If the
/// default address space number is negative, the first address space
/// exposed by the device will be used as the default.
/// \param [in] The parameter string.
/// \param [inout] spacenum The default address space index.  If negative,
///   the first address space exposed by the device (i.e. the address
///   space with the lowest index) will be used as the default and
///   returned
/// \param [out] mintf Memory interface of the device when found
/// \return true if the parameter refers to an address space in the
///   current system, or false otherwise.
bool debugger_console::validate_device_space_parameter(std::string_view param, int &spacenum, device_memory_interface *&mintf)
{
	device_t *device;
	std::string spacename;
	if (param.empty())
	{
		// if no parameter, use the visible CPU
		device = m_visiblecpu;
		if (!device)
		{
			printf("No valid CPU is currently selected\n");
			return false;
		}
	}
	else
	{
		// look for a tag match on the whole parameter value
		std::string_view relative = param;
		device_t &base = get_device_search_base(relative);
		device = base.subdevice(strmakelower(relative));

		// if that failed, treat the last component as an address space
		if (!device)
		{
			auto const delimiter = relative.find_last_of(":^");
			bool const found = std::string_view::npos != delimiter;
			if (!found || (':' == relative[delimiter]))
			{
				spacename = strmakelower(relative.substr(found ? (delimiter + 1) : 0));
				relative = relative.substr(0, !found ? 0 : !delimiter ? 1 : delimiter);
				if (!relative.empty())
					device = base.subdevice(strmakelower(relative));
				else if (m_visiblecpu)
					device = m_visiblecpu;
				else
					device = &m_machine.root_device();
			}
		}
	}

	// if still no device found, evaluate as an expression
	if (!device)
	{
		u64 cpunum;
		try
		{
			cpunum = parsed_expression(visible_symtable(), param).execute();
		}
		catch (expression_error const &)
		{
			// parsing failed - assume it was a tag
			printf("Unable to find device '%s'\n", param);
			return false;
		}

		// attempt to find by numerical index
		device = get_cpu_by_index(cpunum);
		if (!device)
		{
			// if out of range, complain
			printf("Invalid CPU index %u\n", cpunum);
			return false;
		}
	}

	// ensure the device implements the memory interface
	if (!device->interface(mintf))
	{
		printf("No memory interface found for device %s\n", device->name());
		return false;
	}

	// fall back to supplied default space if appropriate
	if (spacename.empty() && (0 <= spacenum))
	{
		if (mintf->has_logical_space(spacenum))
			return true;
		else
		{
			printf("No matching memory space found for device '%s'\n", device->tag());
			return false;
		}
	}

	// otherwise find the specified space or fall back to the first populated space
	for (int i = 0; mintf->max_space_count() > i; ++i)
	{
		if (mintf->has_logical_space(i) && (spacename.empty() || (mintf->logical_space_config(i)->name() == spacename)))
		{
			spacenum = i;
			return true;
		}
	}

	// report appropriate error message
	if (spacename.empty())
		printf("No memory spaces found for device '%s'\n", device->tag());
	else
		printf("Memory space '%s' not found found for device '%s'\n", spacename, device->tag());
	return false;
}


/// \brief Validate a parameter as a target address
///
/// Validates a parameter as an numeric expression to use as an address
/// optionally followed by a colon and a device identifier.  If the
/// device identifier is not presnt, the current CPU with debugger focus
/// is assumed.  See #validate_device_parameter for information on how
/// device parameters are interpreted.
/// \param [in] The parameter string.
/// \param [in] spacenum The default address space index.  If negative,
///   the first address space exposed by the device (i.e. the address
///   space with the lowest index) will be used as the default.
/// \param [out] space The address space on success, or unchanged on
///   failure.
/// \param [out] addr The address on success, or unchanged on failure.
/// \return true if the address is a valid expression evaluating to a
///   number and the address space is found, or false otherwise.
bool debugger_console::validate_target_address_parameter(std::string_view param, int &spacenum, device_memory_interface *&mintf, u64 &addr)
{
	// check for the device delimiter
	std::string_view::size_type const devdelim = find_delimiter(param, [] (char ch) { return ':' == ch; });
	std::string_view device;
	if (devdelim != std::string::npos)
		device = param.substr(devdelim + 1);

	// parse the address first
	offs_t addrval;
	if (!validate_number_parameter(param.substr(0, devdelim), addrval))
		return false;

	// find the logical space
	if (!validate_device_space_parameter(device, spacenum, mintf))
		return false;

	// set the address now that we have the interface
	addr = addrval;
	return true;
}


/// \brief Validate a parameter as an address in a memory region or share
///
/// Validates a parameter as a address with memory region or share tag.
/// \param [in] The parameter string.
/// \param [out] addr The address on success, or unchanged on failure.
/// \param [out] region The region if the parameter refers to a memory
///   region, or unchanged otherwise.
/// \param [out] share The share if the parameter refers to a share, or
///   unchanged otherwise.
/// \return true if the parameter refers to an address in a memory
///   region or share in the current system, or false otherwise.
bool debugger_console::validate_address_with_memory_parameter(std::string_view param, u64 &addr, memory_region *&region, memory_share *&share)
{
	std::string str(param);
	std::regex re("^([^:]+)(:.+)\\.([ms])$");
	std::smatch m;
	if (std::regex_match(str, m, re))
	{
		if ('m' == m[3])
			validate_memory_region_parameter(m.str(2), region);
		else if ('s' == m[3])
			validate_memory_share_parameter(m.str(2), share);
		else
			return false;

		validate_number_parameter(m.str(1), addr);

		return true;
	}

	return false;
}


/// \brief Validate a parameter as a memory region
///
/// Validates a parameter as a memory region tag and retrieves the
/// specified memory region.
/// \param [in] The parameter string.
/// \param [out] result The memory region on success, or unchanged on
///   failure.
/// \return true if the parameter refers to a memory region in the
///   current system, or false otherwise.
bool debugger_console::validate_memory_region_parameter(std::string_view param, memory_region *&result)
{
	auto const &regions = m_machine.memory().regions();
	std::string_view relative = param;
	device_t &base = get_device_search_base(relative);
	auto const iter = regions.find(base.subtag(strmakelower(relative)));
	if (regions.end() != iter)
	{
		result = iter->second.get();
		return true;
	}
	else
	{
		printf("No matching memory region found for '%s'\n", param);
		return false;
	}
}


/// \brief Validate a parameter as a memory share
///
/// Validates a parameter as a memory share tag and retrieves the
/// specified memory share.
/// \param [in] The parameter string.
/// \param [out] result The memory share on success, or unchanged on
///   failure.
/// \return true if the parameter refers to a memory share in the
///   current system, or false otherwise.
bool debugger_console::validate_memory_share_parameter(std::string_view param, memory_share *&result)
{
	auto const &shares = m_machine.memory().shares();
	std::string_view relative = param;
	device_t &base = get_device_search_base(relative);
	auto const iter = shares.find(base.subtag(strmakelower(relative)));
	if (shares.end() != iter)
	{
		result = iter->second.get();
		return true;
	}
	else
	{
		printf("No matching memory share found for '%s'\n", param);
		return false;
	}
}


/// \brief Get search base for device or address space parameter
///
/// Handles prefix prefixes used to indicate that a device tag should be
/// interpreted relative to the selected CPU.  Removes the recognised
/// prefixes from the parameter value.
/// \param [in,out] param The parameter string.  Recognised prefixes
///   affecting the search base are removed, leaving a tag relative to
///   the base device.
/// \return A reference to the base device that the tag should be
///   interpreted relative to.
device_t &debugger_console::get_device_search_base(std::string_view &param) const
{
	if (!param.empty())
	{
		// handle ".:" or ".^" prefix for tag relative to current CPU if any
		if (('.' == param[0]) && ((param.size() == 1) || (':' == param[1]) || ('^' == param[1])))
		{
			param.remove_prefix(((param.size() > 1) && (':' == param[1])) ? 2 : 1);
			device_t *const current = m_visiblecpu;
			return current ? *current : m_machine.root_device();
		}

		// a sibling path makes most sense relative to current CPU
		if ('^' == param[0])
		{
			device_t *const current = m_visiblecpu;
			return current ? *current : m_machine.root_device();
		}
	}

	// default to root device
	return m_machine.root_device();
}


/// \brief Get CPU by index
///
/// Looks up a CPU by the number the debugger assigns it based on its
/// position in the device tree relative to other CPUs.
/// \param [in] cpunum Zero-based index of the CPU to find.
/// \return A pointer to the CPU if found, or \c nullptr if no CPU has
///   the specified index.
device_t *debugger_console::get_cpu_by_index(u64 cpunum) const
{
	unsigned index = 0;
	for (device_execute_interface &exec : execute_interface_enumerator(m_machine.root_device()))
	{
		// real CPUs should have pcbase
		device_state_interface const *state;
		if (exec.device().interface(state) && state->state_find_entry(STATE_GENPCBASE))
		{
			if (index++ == cpunum)
			{
				return &exec.device();
			}
		}
	}
	return nullptr;
}


//-------------------------------------------------
//  validate_expression_parameter - validates
//  an expression parameter
//-----------------------------------------------*/

bool debugger_console::validate_expression_parameter(std::string_view param, parsed_expression &result)
{
	try
	{
		// parse the expression; success if no error
		result.parse(param);
		return true;
	}
	catch (expression_error const &err)
	{
		// output an error
		printf("Error in expression: %s\n", param);
		printf("                     %*s^", err.offset(), "");
		printf("%s\n", err.code_string());
		return false;
	}
}


//-------------------------------------------------
//  validate_command_parameter - validates a
//  command parameter
//-------------------------------------------------

bool debugger_console::validate_command_parameter(std::string_view param)
{
	// validate the comment; success if no error
	CMDERR err = validate_command(param);
	if (err.error_class() == CMDERR::NONE)
		return true;

	// output an error
	printf("Error in command: %s\n", param);
	printf("                  %*s^", err.error_offset(), "");
	printf("%s\n", cmderr_to_string(err));
	return false;
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

void debugger_console::vprintf(util::format_argument_pack<char> const &args)
{
	print_core(util::string_format(std::locale::classic(), args));

	// force an update of any console views
	m_machine.debug_view().update_all(DVT_CONSOLE);
}

void debugger_console::vprintf(util::format_argument_pack<char> &&args)
{
	print_core(util::string_format(std::locale::classic(), std::move(args)));

	// force an update of any console views
	m_machine.debug_view().update_all(DVT_CONSOLE);
}


//-------------------------------------------------
//  vprintf_wrap - vprintfs the given arguments
//  using the format to the debug console
//-------------------------------------------------

void debugger_console::vprintf_wrap(int wrapcol, util::format_argument_pack<char> const &args)
{
	print_core_wrap(util::string_format(std::locale::classic(), args), wrapcol);

	// force an update of any console views
	m_machine.debug_view().update_all(DVT_CONSOLE);
}

void debugger_console::vprintf_wrap(int wrapcol, util::format_argument_pack<char> &&args)
{
	print_core_wrap(util::string_format(std::locale::classic(), std::move(args)), wrapcol);

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
