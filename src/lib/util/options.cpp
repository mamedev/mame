// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    options.cpp

    Core options code code

***************************************************************************/

#include "options.h"

#include "corefile.h"
#include "corestr.h"
#include "osdcore.h"

#include <locale>
#include <string>

#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <sstream>


const int core_options::MAX_UNADORNED_OPTIONS;

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const char *const core_options::s_option_unadorned[MAX_UNADORNED_OPTIONS] =
{
	"<UNADORNED0>",
	"<UNADORNED1>",
	"<UNADORNED2>",
	"<UNADORNED3>",
	"<UNADORNED4>",
	"<UNADORNED5>",
	"<UNADORNED6>",
	"<UNADORNED7>",
	"<UNADORNED8>",
	"<UNADORNED9>",
	"<UNADORNED10>",
	"<UNADORNED11>",
	"<UNADORNED12>",
	"<UNADORNED13>",
	"<UNADORNED14>",
	"<UNADORNED15>"
};


//**************************************************************************
//  UTILITY
//**************************************************************************

namespace
{
	std::string_view trim_spaces_and_quotes(std::string_view data)
	{
		// trim any whitespace
		data = strtrimspace(data);

		// trim quotes
		if (data.length() >= 2 && data.front() == '"' && data.back() == '"')
		{
			data.remove_prefix(1);
			data.remove_suffix(1);
		}

		return data;
	}
};

//**************************************************************************
//  OPTIONS EXCEPTION CLASS
//**************************************************************************

//-------------------------------------------------
//  options_exception - constructor
//-------------------------------------------------

options_exception::options_exception(std::string &&message)
	: m_message(std::move(message))
{
}


//-------------------------------------------------
//  options_warning_exception - constructor
//-------------------------------------------------

options_warning_exception::options_warning_exception(std::string &&message)
	: options_exception(std::move(message))
{
}


//-------------------------------------------------
//  options_error_exception - constructor
//-------------------------------------------------

options_error_exception::options_error_exception(std::string &&message)
	: options_exception(std::move(message))
{
}


//**************************************************************************
//  CORE OPTIONS ENTRY BASE CLASS
//**************************************************************************

//-------------------------------------------------
//  entry - constructor
//-------------------------------------------------

core_options::entry::entry(std::vector<std::string> &&names, option_type type, const char *description)
	: m_names(std::move(names))
	, m_priority(OPTION_PRIORITY_DEFAULT)
	, m_type(type)
	, m_description(description)
{
	assert(m_names.empty() == (m_type == option_type::HEADER));
}

core_options::entry::entry(std::string &&name, option_type type, const char *description)
	: entry(std::vector<std::string>({ std::move(name) }), type, description)
{
}


//-------------------------------------------------
//  entry - destructor
//-------------------------------------------------

core_options::entry::~entry()
{
}


//-------------------------------------------------
//  entry::value
//-------------------------------------------------

const char *core_options::entry::value() const noexcept
{
	// returning 'nullptr' from here signifies a value entry that is essentially "write only"
	// and cannot be meaningfully persisted (e.g. - a command or the software name)
	return nullptr;
}


//-------------------------------------------------
//  entry::value_unsubstituted
//-------------------------------------------------

const char *core_options::entry::value_unsubstituted() const noexcept
{
	return value();
}


//-------------------------------------------------
//  entry::int_value
//-------------------------------------------------

int core_options::entry::int_value() const
{
	char const *const data = value();
	if (!data)
		return 0;
	std::istringstream str(data);
	str.imbue(std::locale::classic());
	int ival;
	if (str >> ival)
		return ival;
	else
		return 0;
}


//-------------------------------------------------
//  entry::float_value
//-------------------------------------------------

float core_options::entry::float_value() const
{
	char const *const data = value();
	if (!data)
		return 0.0F;
	std::istringstream str(data);
	str.imbue(std::locale::classic());
	float fval;
	if (str >> fval)
		return fval;
	else
		return 0.0F;
}


//-------------------------------------------------
//  entry::int_default_value
//-------------------------------------------------

int core_options::entry::int_default_value() const
{
	std::istringstream str(default_value());
	str.imbue(std::locale::classic());
	int ival;
	if (str >> ival)
		return ival;
	else
		return 0;
}


//-------------------------------------------------
//  entry::float_default_value
//-------------------------------------------------

float core_options::entry::float_default_value() const
{
	std::istringstream str(default_value());
	str.imbue(std::locale::classic());
	float fval;
	if (str >> fval)
		return fval;
	else
		return 0.0F;
}


//-------------------------------------------------
//  entry::copy_from
//-------------------------------------------------

void core_options::entry::copy_from(const entry &that, bool always_override)
{
	// it is invalid to set the value on a header
	assert(type() != option_type::HEADER);

	// only set the value if we have priority
	if (always_override || that.priority() >= priority())
	{
		if (internal_copy_value(that))
		{
			m_priority = that.priority();

			// invoke the value changed handler, if appropriate
			if (m_value_changed_handler)
				m_value_changed_handler(value());
		}
	}
}


//-------------------------------------------------
//  entry::set_value
//-------------------------------------------------

void core_options::entry::set_value(std::string &&newvalue, int priority_value, bool always_override, bool perform_substitutions)
{
	// it is invalid to set the value on a header
	assert(type() != option_type::HEADER);

	validate(newvalue);

	// only set the value if we have priority
	if (always_override || priority_value >= priority())
	{
		internal_set_value(std::move(newvalue), perform_substitutions);
		m_priority = priority_value;

		// invoke the value changed handler, if appropriate
		if (m_value_changed_handler)
			m_value_changed_handler(value());
	}
}

void core_options::entry::set_value(int value, int priority)
{
	set_value(util::string_format(std::locale::classic(), "%d", value), priority, false, false);
}

void core_options::entry::set_value(float value, int priority)
{
	set_value(util::string_format(std::locale::classic(), "%f", value), priority, false, false);
}


//-------------------------------------------------
//  entry::set_default_value
//-------------------------------------------------

void core_options::entry::set_default_value(std::string &&newvalue)
{
	// set_default_value() is not necessarily supported for all entry types
	throw false;
}


//-------------------------------------------------
//  entry::internal_copy_value
//-------------------------------------------------

bool core_options::entry::internal_copy_value(const entry &that)
{
	char const *const newvalue = that.value();
	if (newvalue)
	{
		std::string stringval = newvalue;
		validate(stringval);

		internal_set_value(std::move(stringval), false);

		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------
//  entry::validate
//-------------------------------------------------

void core_options::entry::validate(const std::string &data)
{
	std::istringstream str(data);
	str.imbue(std::locale::classic());

	switch (type())
	{
	case option_type::BOOLEAN:
		{
			// booleans must be 0 or 1
			int ival;
			if (!(str >> ival) || (0 > ival) || (1 < ival))
				throw options_warning_exception("Illegal boolean value for %s: \"%s\"; reverting to %s\n", name(), data, value());
		}
		break;

	case option_type::INTEGER:
		{
			// integers must be integral
			int ival;
			if (!(str >> ival))
				throw options_warning_exception("Illegal integer value for %s: \"%s\"; reverting to %s\n", name(), data, value());

			// range checking
			char const *const strmin(minimum());
			char const *const strmax(maximum());
			int imin(0), imax(0);
			if (strmin && *strmin)
			{
				str.str(strmin);
				str.seekg(0);
				str >> imin;
			}
			if (strmax && *strmax)
			{
				str.str(strmax);
				str.seekg(0);
				str >> imax;
			}
			if ((strmin && *strmin && (ival < imin)) || (strmax && *strmax && (ival > imax)))
			{
				if (!strmax || !*strmax)
					throw options_warning_exception("Out-of-range integer value for %s: \"%s\" (must be no less than %d); reverting to %s\n", name(), data, imin, value());
				else if (!strmin || !*strmin)
					throw options_warning_exception("Out-of-range integer value for %s: \"%s\" (must be no greater than %d); reverting to %s\n", name(), data, imax, value());
				else
					throw options_warning_exception("Out-of-range integer value for %s: \"%s\" (must be between %d and %d, inclusive); reverting to %s\n", name(), data, imin, imax, value());
			}
		}
		break;

	case option_type::FLOAT:
		{
			float fval;
			if (!(str >> fval))
				throw options_warning_exception("Illegal float value for %s: \"%s\"; reverting to %s\n", name(), data, value());

			// range checking
			char const *const strmin(minimum());
			char const *const strmax(maximum());
			float fmin(0), fmax(0);
			if (strmin && *strmin)
			{
				str.str(strmin);
				str.seekg(0);
				str >> fmin;
			}
			if (strmax && *strmax)
			{
				str.str(strmax);
				str.seekg(0);
				str >> fmax;
			}
			if ((strmin && *strmin && (fval < fmin)) || (strmax && *strmax && (fval > fmax)))
			{
				if (!strmax || !*strmax)
					throw options_warning_exception("Out-of-range float value for %s: \"%s\" (must be no less than %f); reverting to %s\n", name(), data, fmin, value());
				else if (!strmin || !*strmin)
					throw options_warning_exception("Out-of-range float value for %s: \"%s\" (must be no greater than %f); reverting to %s\n", name(), data, fmax, value());
				else
					throw options_warning_exception("Out-of-range float value for %s: \"%s\" (must be between %f and %f, inclusive); reverting to %s\n", name(), data, fmin, fmax, value());
			}
		}
		break;

	case option_type::STRING:
	case option_type::PATH:
	case option_type::MULTIPATH:
		// strings can be anything
		break;

	case option_type::INVALID:
	case option_type::HEADER:
	default:
		// anything else is invalid
		throw options_error_exception("Attempted to set invalid option %s\n", name());
	}
}


//-------------------------------------------------
//  entry::minimum
//-------------------------------------------------

const char *core_options::entry::minimum() const noexcept
{
	return nullptr;
}


//-------------------------------------------------
//  entry::maximum
//-------------------------------------------------

const char *core_options::entry::maximum() const noexcept
{
	return nullptr;
}


//-------------------------------------------------
//  entry::has_range
//-------------------------------------------------

bool core_options::entry::has_range() const noexcept
{
	return minimum() && maximum();
}


//-------------------------------------------------
//  entry::default_value
//-------------------------------------------------

const std::string &core_options::entry::default_value() const noexcept
{
	// I don't really want this generally available, but MewUI seems to need it.  Please do not use.
	abort();
}


//**************************************************************************
//  CORE OPTIONS SIMPLE ENTRYCLASS
//**************************************************************************

//-------------------------------------------------
//  simple_entry - constructor
//-------------------------------------------------

core_options::simple_entry::simple_entry(std::vector<std::string> &&names, const char *description, core_options::option_type type, std::string &&defdata, std::string &&minimum, std::string &&maximum)
	: entry(std::move(names), type, description)
	, m_defdata_unsubst(std::move(defdata))
	, m_minimum(std::move(minimum))
	, m_maximum(std::move(maximum))
{
	m_defdata = type_specific_substitutions(m_defdata_unsubst);
	m_data_unsubst = m_defdata_unsubst;
	m_data = m_defdata;
}


//-------------------------------------------------
//  simple_entry - destructor
//-------------------------------------------------

core_options::simple_entry::~simple_entry()
{
}


//-------------------------------------------------
//  simple_entry::value
//-------------------------------------------------

const char *core_options::simple_entry::value() const noexcept
{
	switch (type())
	{
	case option_type::BOOLEAN:
	case option_type::INTEGER:
	case option_type::FLOAT:
	case option_type::STRING:
	case option_type::PATH:
	case option_type::MULTIPATH:
		return m_data.c_str();

	default:
		// this is an option type for which returning a value is
		// a meaningless operation (e.g. - core_options::option_type::COMMAND)
		return nullptr;
	}
}


//-------------------------------------------------
//  simple_entry::value_unsubstituted
//-------------------------------------------------

const char *core_options::simple_entry::value_unsubstituted() const noexcept
{
	switch (type())
	{
	case core_options::option_type::BOOLEAN:
	case core_options::option_type::INTEGER:
	case core_options::option_type::FLOAT:
	case core_options::option_type::STRING:
	case core_options::option_type::PATH:
	case core_options::option_type::MULTIPATH:
		return m_data_unsubst.c_str();

	default:
		// this is an option type for which returning a value is
		// a meaningless operation (e.g. - core_options::option_type::COMMAND)
		return nullptr;
	}
}


//-------------------------------------------------
//  simple_entry::default_value
//-------------------------------------------------

const std::string &core_options::simple_entry::default_value() const noexcept
{
	// only MewUI seems to need this; please don't use
	return m_defdata;
}


//-------------------------------------------------
//  simple_entry::internal_set_value
//-------------------------------------------------

void core_options::simple_entry::internal_set_value(std::string &&newvalue, bool perform_substitutions)
{
	m_data = perform_substitutions
		? type_specific_substitutions(newvalue)
		: newvalue;
	m_data_unsubst = std::move(newvalue);
}


//-------------------------------------------------
//  simple_entry::internal_copy_value
//-------------------------------------------------

bool core_options::simple_entry::internal_copy_value(const entry &that)
{
	simple_entry const *const simple = dynamic_cast<simple_entry const *>(&that);
	if (!simple)
	{
		return entry::internal_copy_value(that);
	}
	else
	{
		switch (simple->type())
		{
		case option_type::BOOLEAN:
		case option_type::INTEGER:
		case option_type::FLOAT:
		case option_type::STRING:
		case option_type::PATH:
		case option_type::MULTIPATH:
			validate(simple->m_data);

			m_data = simple->m_data;
			m_data_unsubst = simple->m_data_unsubst;

			return true;

		default:
			return false;
		}
	}
}


//-------------------------------------------------
//  set_default_value
//-------------------------------------------------

void core_options::simple_entry::set_default_value(std::string &&newvalue)
{
	m_data = m_defdata = type_specific_substitutions(newvalue);
	m_data_unsubst = m_defdata_unsubst = std::move(newvalue);
}


//-------------------------------------------------
//  type_specific_substitutions
//-------------------------------------------------

std::string core_options::simple_entry::type_specific_substitutions(std::string_view s) const noexcept
{
	switch (type())
	{
	case option_type::PATH:
		return osd_subst_env(s);

	case option_type::MULTIPATH:
		{
			std::ostringstream result;
			while (!s.empty())
			{
				std::string_view::size_type split = s.find(';');
				if (std::string_view::npos == split)
					split = s.length();
				result << osd_subst_env(s.substr(0, split));
				if (s.length() > split)
				{
					result << s[split];
					s.remove_prefix(split + 1);
				}
				else
				{
					s.remove_prefix(split);
				}
			}
			return std::move(result).str();
		}

	default:
		return std::string(s);
	}
}


//-------------------------------------------------
//  minimum
//-------------------------------------------------

const char *core_options::simple_entry::minimum() const noexcept
{
	return m_minimum.c_str();
}


//-------------------------------------------------
//  maximum
//-------------------------------------------------

const char *core_options::simple_entry::maximum() const noexcept
{
	return m_maximum.c_str();
}


//**************************************************************************
//  CORE OPTIONS
//**************************************************************************

//-------------------------------------------------
//  core_options - constructor
//-------------------------------------------------

core_options::core_options()
{
}


//-------------------------------------------------
//  ~core_options - destructor
//-------------------------------------------------

core_options::~core_options()
{
}


//-------------------------------------------------
//  add_entry - adds an entry
//-------------------------------------------------

void core_options::add_entry(entry::shared_ptr &&entry, const char *after_header)
{
	// update the entry map
	for (const std::string &name : entry->names())
	{
		// append the entry
		add_to_entry_map(name, entry);
	}

	// and add the entry to the vector
	m_entries.emplace_back(std::move(entry));
}


//-------------------------------------------------
//  add_to_entry_map - adds an entry to the entry
//  map
//-------------------------------------------------

void core_options::add_to_entry_map(const std::string &name, entry::shared_ptr &entry)
{
	// it is illegal to call this method for something that already exists
	assert(m_entrymap.find(name) == m_entrymap.end());

	// append the entry
	m_entrymap.emplace(std::make_pair(std::string_view(name), entry::weak_ptr(entry)));
}


//-------------------------------------------------
//  add_entry - adds an entry based on an
//  options_entry
//-------------------------------------------------

void core_options::add_entry(const options_entry &opt, bool override_existing)
{
	std::vector<std::string> names;
	std::string minimum, maximum;

	// copy in the name(s) as appropriate
	if (opt.name)
	{
		// first extract any range
		std::string namestr(opt.name);
		std::string::size_type lparen = namestr.find_first_of('(', 0);
		if (lparen != std::string::npos)
		{
			std::string::size_type dash = namestr.find_first_of('-', lparen + 2);
			if (dash != std::string::npos)
			{
				std::string::size_type rparen = namestr.find_first_of(')', dash + 1);
				if (rparen != std::string::npos)
				{
					minimum.assign(strtrimspace(std::string_view(&namestr[lparen + 1], dash - (lparen + 1))));
					maximum.assign(strtrimspace(std::string_view(&namestr[dash + 1], rparen - (dash + 1))));
					namestr.erase(lparen, rparen + 1 - lparen);
				}
			}
		}

		// then chop up any semicolon-separated names
		std::string::size_type semi;
		while ((semi = namestr.find_first_of(';')) != std::string::npos)
		{
			names.push_back(namestr.substr(0, semi));

			// for booleans, add the "-noXYZ" option as well
			if (opt.type == option_type::BOOLEAN)
				names.push_back(std::string("no") + names.back());

			namestr.erase(0, semi + 1);
		}

		// finally add the last item
		names.push_back(std::move(namestr));
		if (opt.type == option_type::BOOLEAN)
			names.push_back(std::string("no") + names.back());
	}

	// we might be called with an existing entry
	entry::shared_ptr existing_entry;
	do
	{
		for (const std::string &name : names)
		{
			existing_entry = get_entry(name);
			if (existing_entry)
				break;
		}

		if (existing_entry)
		{
			if (override_existing)
				remove_entry(*existing_entry);
			else
				return;
		}
	} while (existing_entry);

	// set the default value
	std::string defdata = opt.defvalue ? opt.defvalue : "";

	// create and add the entry
	add_entry(
			std::move(names),
			opt.description,
			opt.type,
			std::move(defdata),
			std::move(minimum),
			std::move(maximum));
}


//-------------------------------------------------
//  add_entry
//-------------------------------------------------

void core_options::add_entry(std::vector<std::string> &&names, const char *description, option_type type, std::string &&default_value, std::string &&minimum, std::string &&maximum)
{
	// create the entry
	entry::shared_ptr new_entry = std::make_shared<simple_entry>(
			std::move(names),
			description,
			type,
			std::move(default_value),
			std::move(minimum),
			std::move(maximum));

	// and add it
	add_entry(std::move(new_entry));
}


//-------------------------------------------------
//  add_header
//-------------------------------------------------

void core_options::add_header(const char *description)
{
	add_entry(std::vector<std::string>(), description, option_type::HEADER);
}


//-------------------------------------------------
//  add_entries - add entries to the current
//  options sets
//-------------------------------------------------

void core_options::add_entries(const options_entry *entrylist, bool override_existing)
{
	// loop over entries until we hit a nullptr name
	for (int i = 0; entrylist[i].name || entrylist[i].type == option_type::HEADER; i++)
		add_entry(entrylist[i], override_existing);
}


//-------------------------------------------------
//  set_default_value - change the default value
//  of an option
//-------------------------------------------------

void core_options::set_default_value(std::string_view name, std::string &&defvalue)
{
	// update the data and default data
	auto entry = get_entry(name);
	assert(entry != nullptr);
	entry->set_default_value(std::move(defvalue));
}


//-------------------------------------------------
//  set_description - change the description
//  of an option
//-------------------------------------------------

void core_options::set_description(std::string_view name, const char *description)
{
	// update the data and default data
	auto entry = get_entry(name);
	assert(entry != nullptr);
	entry->set_description(description);
}


//-------------------------------------------------
//  parse_command_line - parse a series of
//  command line arguments
//-------------------------------------------------

void core_options::parse_command_line(const std::vector<std::string> &args, int priority, bool ignore_unknown_options)
{
	std::ostringstream error_stream;
	condition_type condition = condition_type::NONE;

	// reset the errors and the command
	m_command.clear();

	// we want to identify commands first
	for (size_t arg = 1; arg < args.size(); arg++)
	{
		if (!args[arg].empty() && args[arg][0] == '-')
		{
			auto curentry = get_entry(&args[arg][1]);
			if (curentry && curentry->type() == core_options::option_type::COMMAND)
			{
				// can only have one command
				if (!m_command.empty())
					throw options_error_exception("Error: multiple commands specified -%s and %s\n", m_command, args[arg]);

				m_command = curentry->name();
			}
		}
	}

	// iterate through arguments
	int unadorned_index = 0;
	for (size_t arg = 1; arg < args.size(); arg++)
	{
		// determine the entry name to search for
		const char *curarg = args[arg].c_str();
		bool is_unadorned = (curarg[0] != '-');
		const char *optionname = is_unadorned ? core_options::unadorned(unadorned_index++) : &curarg[1];

		// special case - collect unadorned arguments after commands into a special place
		if (is_unadorned && !m_command.empty())
		{
			m_command_arguments.push_back(args[arg]);
			command_argument_processed();
			continue;
		}

		// find our entry; if not found, continue
		auto curentry = get_entry(optionname);
		if (!curentry)
		{
			if (!ignore_unknown_options)
				throw options_error_exception("Error: unknown option: -%s\n", optionname);
			continue;
		}

		// at this point, we've already processed commands
		if (curentry->type() == core_options::option_type::COMMAND)
			continue;

		// get the data for this argument, special casing booleans
		std::string_view newdata;
		if (curentry->type() == option_type::BOOLEAN)
		{
			newdata = (strncmp(&curarg[1], "no", 2) == 0) ? "0" : "1";
		}
		else if (is_unadorned)
		{
			newdata = curarg;
		}
		else if (arg + 1 < args.size())
		{
			newdata = args[++arg];
		}
		else
		{
			throw options_error_exception("Error: option %s expected a parameter\n", curarg);
		}

		// set the new data
		do_set_value(*curentry, newdata, priority, error_stream, condition, false);
	}

	// did we have any errors that may need to be aggregated?
	throw_options_exception_if_appropriate(condition, error_stream);
}


//-------------------------------------------------
//  parse_ini_file - parse a series of entries in
//  an INI file
//-------------------------------------------------

void core_options::parse_ini_file(util::core_file &inifile, int priority, bool ignore_unknown_options, bool always_override)
{
	std::ostringstream error_stream;
	condition_type condition = condition_type::NONE;

	// loop over lines in the file
	char buffer[4096];
	while (inifile.gets(buffer, std::size(buffer)) != nullptr)
	{
		// find the extent of the name
		char *optionname;
		for (optionname = buffer; *optionname != 0; optionname++)
			if (!isspace((uint8_t)*optionname))
				break;

		// skip comments
		if (*optionname == 0 || *optionname == '#')
			continue;

		// scan forward to find the first space
		char *temp;
		for (temp = optionname; *temp != 0; temp++)
			if (isspace((uint8_t)*temp))
				break;

		// if we hit the end early, print a warning and continue
		if (*temp == 0)
		{
			condition = std::max(condition, condition_type::WARN);
			util::stream_format(error_stream, "Warning: invalid line in INI: %s", buffer);
			continue;
		}

		// NULL-terminate
		*temp++ = 0;
		char *optiondata = temp;

		// scan the data, stopping when we hit a comment
		bool inquotes = false;
		for (temp = optiondata; *temp != 0; temp++)
		{
			if (*temp == '"')
				inquotes = !inquotes;
			if (*temp == '#' && !inquotes)
				break;
		}
		*temp = 0;

		// find our entry
		entry::shared_ptr curentry = get_entry(optionname);
		if (!curentry)
		{
			if (!ignore_unknown_options)
			{
				condition = std::max(condition, condition_type::WARN);
				util::stream_format(error_stream, "Warning: unknown option in INI: %s\n", optionname);
			}
			continue;
		}

		// set the new data
		do_set_value(*curentry, trim_spaces_and_quotes(optiondata), priority, error_stream, condition, true);
	}

	// did we have any errors that may need to be aggregated?
	throw_options_exception_if_appropriate(condition, error_stream);
}


//-------------------------------------------------
//  throw_options_exception_if_appropriate
//-------------------------------------------------

void core_options::throw_options_exception_if_appropriate(core_options::condition_type condition, std::ostringstream &error_stream)
{
	switch(condition)
	{
	case condition_type::NONE:
		// do nothing
		break;

	case condition_type::WARN:
		throw options_warning_exception(error_stream.str());

	case condition_type::ERR:
		throw options_error_exception(error_stream.str());

	default:
		// should not get here
		throw false;
	}
}


//-------------------------------------------------
//  copy_from
//-------------------------------------------------

void core_options::copy_from(const core_options &that)
{
	for (auto &dest_entry : m_entries)
	{
		if (dest_entry->names().size() > 0)
		{
			// identify the source entry
			const entry::shared_const_ptr source_entry = that.get_entry(dest_entry->name());
			if (source_entry)
				dest_entry->copy_from(*source_entry, false);
		}
	}
}


//-------------------------------------------------
//  output_ini - output the options in INI format,
//  only outputting entries that different from
//  the optional diff
//-------------------------------------------------

std::string core_options::output_ini(const core_options *diff) const
{
	// INI files are complete, so always start with a blank buffer
	std::ostringstream buffer;
	buffer.imbue(std::locale::classic());

	int num_valid_headers = 0;
	int unadorned_index = 0;
	const char *last_header = nullptr;
	std::string overridden_value;

	// loop over all items
	for (auto &curentry : m_entries)
	{
		if (curentry->type() == option_type::HEADER)
		{
			// header: record description
			last_header = curentry->description();
		}
		else
		{
			const std::string &name(curentry->name());
			const char *value(curentry->value_unsubstituted());

			// check if it's unadorned
			bool is_unadorned = false;
			if (name == core_options::unadorned(unadorned_index))
			{
				unadorned_index++;
				is_unadorned = true;
			}

			// output entries for all non-command items (items with value)
			if (value)
			{
				// look up counterpart in diff, if diff is specified
				if (!diff || strcmp(value, diff->get_entry(name.c_str())->value_unsubstituted()))
				{
					// output header, if we have one
					if (last_header)
					{
						if (num_valid_headers++)
							buffer << '\n';
						util::stream_format(buffer, "#\n# %s\n#\n", last_header);
						last_header = nullptr;
					}

					// and finally output the data, skip if unadorned
					if (!is_unadorned)
					{
						if (strchr(value, ' '))
							util::stream_format(buffer, "%-25s \"%s\"\n", name, value);
						else
							util::stream_format(buffer, "%-25s %s\n", name, value);
					}
				}
			}
		}
	}
	return buffer.str();
}


//-------------------------------------------------
//  output_help - output option help to a string
//-------------------------------------------------

std::string core_options::output_help() const
{
	// start empty
	std::ostringstream buffer;

	// loop over all items
	for (auto &curentry : m_entries)
	{
		if (curentry->type() == option_type::HEADER)
		{
			// header: just print
			util::stream_format(buffer, "\n#\n# %s\n#\n", curentry->description());
		}
		else if (curentry->description() != nullptr)
		{
			// otherwise, output entries for all non-deprecated items
			util::stream_format(buffer, "-%-19s %s\n", curentry->name(), curentry->description());
		}
	}
	return buffer.str();
}


//-------------------------------------------------
//  value - return the raw option value
//-------------------------------------------------

const char *core_options::value(std::string_view option) const noexcept
{
	auto const entry = get_entry(option);
	return entry ? entry->value() : nullptr;
}


//-------------------------------------------------
//  description - return description of option
//-------------------------------------------------

const char *core_options::description(std::string_view option) const noexcept
{
	auto const entry = get_entry(option);
	return entry ? entry->description() : nullptr;
}


//-------------------------------------------------
//  value - return the option value as an integer
//-------------------------------------------------

int core_options::int_value(std::string_view option) const
{
	auto const entry = get_entry(option);
	return entry ? entry->int_value() : 0;
}


//-------------------------------------------------
//  value - return the option value as a float
//-------------------------------------------------

float core_options::float_value(std::string_view option) const
{
	auto const entry = get_entry(option);
	return entry ? entry->float_value() : 0.0F;
}


//**************************************************************************
//  LEGACY
//**************************************************************************

//-------------------------------------------------
//  set_value - set the raw option value
//-------------------------------------------------

void core_options::set_value(std::string_view name, std::string_view value, int priority)
{
	set_value(name, std::string(value), priority);
}

void core_options::set_value(std::string_view name, const char *value, int priority)
{
	set_value(name, std::string(value), priority);
}

void core_options::set_value(std::string_view name, std::string &&value, int priority)
{
	auto entry = get_entry(name);
	assert(entry);
	entry->set_value(std::move(value), priority, false, false);
}

void core_options::set_value(std::string_view name, int value, int priority)
{
	auto entry = get_entry(name);
	assert(entry);
	entry->set_value(value, priority);
}

void core_options::set_value(std::string_view name, float value, int priority)
{
	auto entry = get_entry(name);
	assert(entry);
	entry->set_value(value, priority);
}


//-------------------------------------------------
//  remove_entry - remove an entry from our list
//  and map
//-------------------------------------------------

void core_options::remove_entry(core_options::entry &delentry)
{
	// find this in m_entries
	auto iter = std::find_if(
			m_entries.begin(),
			m_entries.end(),
			[&delentry](const auto &x) { return &*x == &delentry; });
	assert(iter != m_entries.end());

	// erase each of the items out of the entry map
	for (const std::string &name : delentry.names())
		m_entrymap.erase(name);

	// finally erase it
	m_entries.erase(iter);
}


//-------------------------------------------------
//  do_set_value
//-------------------------------------------------

void core_options::do_set_value(entry &curentry, std::string_view data, int priority, std::ostream &error_stream, condition_type &condition, bool perform_substitutions)
{
	// this is called when parsing a command line or an INI - we want to catch the option_exception and write
	// any exception messages to the error stream
	try
	{
		curentry.set_value(std::string(data), priority, false, perform_substitutions);
	}
	catch (options_warning_exception &ex)
	{
		// we want to aggregate option exceptions
		error_stream << ex.message();
		condition = std::max(condition, condition_type::WARN);
	}
	catch (options_error_exception &ex)
	{
		// we want to aggregate option exceptions
		error_stream << ex.message();
		condition = std::max(condition, condition_type::ERR);
	}
}


//-------------------------------------------------
//  get_entry
//-------------------------------------------------

core_options::entry::shared_const_ptr core_options::get_entry(std::string_view name) const noexcept
{
	auto curentry = m_entrymap.find(name);
	return (curentry != m_entrymap.end()) ? curentry->second.lock() : nullptr;
}

core_options::entry::shared_ptr core_options::get_entry(std::string_view name) noexcept
{
	auto curentry = m_entrymap.find(name);
	return (curentry != m_entrymap.end()) ? curentry->second.lock() : nullptr;
}


//-------------------------------------------------
//  set_value_changed_handler
//-------------------------------------------------

void core_options::set_value_changed_handler(std::string_view name, std::function<void(const char *)> &&handler)
{
	auto entry = get_entry(name);
	assert(entry != nullptr);
	entry->set_value_changed_handler(std::move(handler));
}


//-------------------------------------------------
//  header_exists
//-------------------------------------------------

bool core_options::header_exists(const char *description) const noexcept
{
	auto iter = std::find_if(
			m_entries.begin(),
			m_entries.end(),
			[description](const auto &entry)
			{
				return entry->type() == option_type::HEADER
						&& entry->description()
						&& !strcmp(entry->description(), description);
			});

	return iter != m_entries.end();
}

//-------------------------------------------------
//  revert - revert options at or below a certain
//  priority back to their defaults
//-------------------------------------------------

void core_options::revert(int priority_hi, int priority_lo)
{
	for (entry::shared_ptr &curentry : m_entries)
		if (curentry->type() != option_type::HEADER && curentry->type() != option_type::COMMAND)
			curentry->revert(priority_hi, priority_lo);
}

//-------------------------------------------------
//  revert - revert back to our default if we are
//  within the given priority range
//-------------------------------------------------

void core_options::simple_entry::revert(int priority_hi, int priority_lo)
{
	// if our priority is within the range, revert to the default
	if (priority() <= priority_hi && priority() >= priority_lo)
	{
		set_value(std::string(m_defdata_unsubst), priority(), true, true);
		set_priority(OPTION_PRIORITY_DEFAULT);
	}
}
