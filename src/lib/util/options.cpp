// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    options.c

    Core options code code

***************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "options.h"
#include "corestr.h"
#include <string>


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
//  CORE OPTIONS ENTRY
//**************************************************************************

//-------------------------------------------------
//  entry - constructor
//-------------------------------------------------

core_options::entry::entry(const char *name, const char *description, uint32_t flags, const char *defvalue)
	: m_next(nullptr),
		m_flags(flags),
		m_seqid(0),
		m_error_reported(false),
		m_priority(OPTION_PRIORITY_DEFAULT),
		m_description(description),
		m_changed(false)
{
	// copy in the name(s) as appropriate
	if (name != nullptr)
	{
		// first extract any range
		std::string namestr(name);
		int lparen = namestr.find_first_of('(',0);
		int dash = namestr.find_first_of('-',lparen + 1);
		int rparen = namestr.find_first_of(')',dash + 1);
		if (lparen != -1 && dash != -1 && rparen != -1)
		{
			strtrimspace(m_minimum.assign(namestr.substr(lparen + 1, dash - (lparen + 1))));
			strtrimspace(m_maximum.assign(namestr.substr(dash + 1, rparen - (dash + 1))));
			namestr.erase(lparen, rparen + 1 - lparen);
		}

		// then chop up any semicolon-separated names
		int semi;
		int nameindex = 0;
		while ((semi = namestr.find_first_of(';')) != -1 && nameindex < ARRAY_LENGTH(m_name))
		{
			m_name[nameindex++].assign(namestr.substr(0, semi));
			namestr.erase(0, semi + 1);
		}

		// finally add the last item
		if (nameindex < ARRAY_LENGTH(m_name))
			m_name[nameindex++] = namestr;
	}

	// set the default value
	if (defvalue != nullptr)
		m_defdata = defvalue;
	m_data = m_defdata;
}


//-------------------------------------------------
//  set_value - update our data value
//-------------------------------------------------

void core_options::entry::set_value(const char *newdata, int priority)
{
	// ignore if we don't have priority
	if (priority < m_priority)
		return;

	// set the data and priority, then bump the sequence
	m_data = newdata;
	m_priority = priority;
	m_seqid++;
}


//-------------------------------------------------
//  set_default_value - set the default value of
//  an option, and reset the current value to it
//-------------------------------------------------

void core_options::entry::set_default_value(const char *defvalue)
{
	m_data = defvalue;
	m_defdata = defvalue;
	m_priority = OPTION_PRIORITY_DEFAULT;
}


//-------------------------------------------------
//  set_description - set the description of
//  an option
//-------------------------------------------------

void core_options::entry::set_description(const char *description)
{
	m_description = description;
}


void core_options::entry::set_flag(uint32_t mask, uint32_t flag)
{
	m_flags = ( m_flags & mask ) | flag;
}


//-------------------------------------------------
//  revert - revert back to our default if we are
//  within the given priority range
//-------------------------------------------------

void core_options::entry::revert(int priority_hi, int priority_lo)
{
	// if our priority is within the range, revert to the default
	if (m_priority <= priority_hi && m_priority >= priority_lo)
	{
		m_data = m_defdata;
		m_priority = OPTION_PRIORITY_DEFAULT;
	}
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

core_options::core_options(const options_entry *entrylist)
{
	add_entries(entrylist);
}

core_options::core_options(const options_entry *entrylist1, const options_entry *entrylist2)
{
	add_entries(entrylist1);
	add_entries(entrylist2);
}

core_options::core_options(const options_entry *entrylist1, const options_entry *entrylist2, const options_entry *entrylist3)
{
	add_entries(entrylist1);
	add_entries(entrylist2);
	add_entries(entrylist3);
}

core_options::core_options(const core_options &src)
{
	copyfrom(src);
}


//-------------------------------------------------
//  ~core_options - destructor
//-------------------------------------------------

core_options::~core_options()
{
}


//-------------------------------------------------
//  operator= - assignment operator
//-------------------------------------------------

core_options &core_options::operator=(const core_options &rhs)
{
	// ignore self-assignment
	if (this != &rhs)
		copyfrom(rhs);
	return *this;
}


//-------------------------------------------------
//  operator== - compare two sets of options
//-------------------------------------------------

bool core_options::operator==(const core_options &rhs)
{
	// iterate over options in the first list
	for (entry &curentry : m_entrylist)
		if (!curentry.is_header())
		{
			// if the values differ, return false
			if (strcmp(curentry.value(), rhs.value(curentry.name())) != 0)
				return false;
		}

	return true;
}


//-------------------------------------------------
//  operator!= - compare two sets of options
//-------------------------------------------------

bool core_options::operator!=(const core_options &rhs)
{
	return !operator==(rhs);
}


//-------------------------------------------------
//  add_entry - add an entry to the current
//  options set
//-------------------------------------------------

void core_options::add_entry(const char *name, const char *description, uint32_t flags, const char *defvalue, bool override_existing)
{
	// allocate a new entry
	auto  newentry = global_alloc(entry(name, description, flags, defvalue));
	if (newentry->name() != nullptr)
	{
		// see if we match an existing entry
		auto checkentry = m_entrymap.find(newentry->name());
		if (checkentry != m_entrymap.end())
		{
			entry *existing = checkentry->second;
			// if we're overriding existing entries, then remove the old one
			if (override_existing)
				m_entrylist.remove(*existing);

			// otherwise, just override the default and current values and throw out the new entry
			else
			{
				existing->set_default_value(newentry->value());
				global_free(newentry);
				return;
			}
		}

		// need to call value_changed() with initial value
		value_changed(newentry->name(), newentry->value());
	}

	// add us to the list and maps
	append_entry(*newentry);
}


//-------------------------------------------------
//  add_entries - add entries to the current
//  options sets
//-------------------------------------------------

void core_options::add_entries(const options_entry *entrylist, bool override_existing)
{
	// loop over entries until we hit a nullptr name
	for ( ; entrylist->name != nullptr || (entrylist->flags & OPTION_HEADER) != 0; entrylist++)
		add_entry(*entrylist, override_existing);
}


//-------------------------------------------------
//  set_default_value - change the default value
//  of an option
//-------------------------------------------------

void core_options::set_default_value(const char *name, const char *defvalue)
{
	// find the entry and bail if we can't
	auto curentry = m_entrymap.find(name);
	if (curentry == m_entrymap.end())
		return;

	// update the data and default data
	curentry->second->set_default_value(defvalue);
}


//-------------------------------------------------
//  set_description - change the description
//  of an option
//-------------------------------------------------

void core_options::set_description(const char *name, const char *description)
{
	// find the entry and bail if we can't
	auto curentry = m_entrymap.find(name);
	if (curentry == m_entrymap.end())
		return;

	// update the data and default data
	curentry->second->set_description(description);
}


//-------------------------------------------------
//  parse_command_line - parse a series of
//  command line arguments
//-------------------------------------------------

bool core_options::parse_command_line(std::vector<std::string> &args, int priority, std::string &error_string)
{
	// reset the errors and the command
	error_string.clear();
	m_command.clear();

	// iterate through arguments
	int unadorned_index = 0;
	size_t new_argc = 1;
	for (size_t arg = 1; arg < args.size(); arg++)
	{
		// determine the entry name to search for
		const char *curarg = args[arg].c_str();
		bool is_unadorned = (curarg[0] != '-');
		const char *optionname = is_unadorned ? core_options::unadorned(unadorned_index++) : &curarg[1];

		// find our entry; if not found, continue
		auto curentry = m_entrymap.find(optionname);
		if (curentry == m_entrymap.end())
		{
			// we need to relocate this option
			if (new_argc != arg)
				args[new_argc] = std::move(args[arg]);
			new_argc++;

			if (!is_unadorned)
			{
				arg++;
				if (arg < args.size())
				{
					if (new_argc != arg)
						args[new_argc] = std::move(args[arg]);
					new_argc++;
				}
			}
			continue;
		}

		// process commands first
		if (curentry->second->type() == OPTION_COMMAND)
		{
			// can only have one command
			if (!m_command.empty())
			{
				error_string.append(string_format("Error: multiple commands specified -%s and %s\n", m_command, curarg));
				return false;
			}
			m_command = curentry->second->name();
			continue;
		}

		// get the data for this argument, special casing booleans
		std::string newdata;
		if (curentry->second->type() == OPTION_BOOLEAN)
		{
			newdata = (strncmp(&curarg[1], "no", 2) == 0) ? "0" : "1";
		}
		else if (is_unadorned)
		{
			newdata = curarg;
		}
		else if (arg + 1 < args.size())
		{
			args[arg++].clear();
			newdata = std::move(args[arg]);
		}
		else
		{
			error_string.append(string_format("Error: option %s expected a parameter\n", curarg));
			return false;
		}
		args[arg].clear();

		// set the new data
		validate_and_set_data(*curentry->second, std::move(newdata), priority, error_string);
	}

	args.resize(new_argc);
	return true;
}


//-------------------------------------------------
//  parse_ini_file - parse a series of entries in
//  an INI file
//-------------------------------------------------

bool core_options::parse_ini_file(util::core_file &inifile, int priority, int ignore_priority, std::string &error_string)
{
	// loop over lines in the file
	char buffer[4096];
	while (inifile.gets(buffer, ARRAY_LENGTH(buffer)) != nullptr)
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
			error_string.append(string_format("Warning: invalid line in INI: %s", buffer));
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
		auto curentry = m_entrymap.find(optionname);
		if (curentry == m_entrymap.end())
		{
			if (priority >= ignore_priority)
				error_string.append(string_format("Warning: unknown option in INI: %s\n", optionname));
			continue;
		}

		// set the new data
		validate_and_set_data(*curentry->second, optiondata, priority, error_string);
	}
	return true;
}


//-------------------------------------------------
//  pluck_from_command_line - finds a specific
//  value from within a command line
//-------------------------------------------------

bool core_options::pluck_from_command_line(std::vector<std::string> &args, const std::string &optionname, std::string &result)
{
	// find this entry within the options (it is illegal to call this with a non-existant option
	// so we assert if not present)
	auto curentry = m_entrymap.find(optionname);
	assert(curentry != m_entrymap.end());

	// build a vector with potential targets
	std::vector<std::string> targets;
	const char *potential_target;
	int index = 0;
	while ((potential_target = curentry->second->name(index++)) != nullptr)
	{
		// not supporting unadorned options for now
		targets.push_back(std::string("-") + potential_target);
	}

	// find each of the targets in the argv array
	for (int i = 1; i < args.size() - 1; i++)
	{
		auto const iter = std::find_if(
			targets.begin(),
			targets.end(),
			[&args, i](const std::string &targ) { return targ == args[i]; });
		if (iter != targets.end())
		{
			// get the result
			result = std::move(args[i + 1]);

			// remove this arguments from the list
			auto const pos = std::next(args.begin(), i);
			args.erase(pos, std::next(pos, 2));
			return true;
		}
	}

	result.clear();
	return false;
}


//-------------------------------------------------
//  revert - revert options at or below a certain
//  priority back to their defaults
//-------------------------------------------------

void core_options::revert(int priority_hi, int priority_lo)
{
	// iterate over options and revert to defaults if below the given priority
	for (entry &curentry : m_entrylist)
		curentry.revert(priority_hi, priority_lo);
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

	int num_valid_headers = 0;
	int unadorned_index = 0;
	const char *last_header = nullptr;

	// loop over all items
	for (entry &curentry : m_entrylist)
	{
		const char *name = curentry.name();
		const char *value = curentry.value();
		bool is_unadorned = false;

		// check if it's unadorned
		if (name && strlen(name) && !strcmp(name, core_options::unadorned(unadorned_index)))
		{
			unadorned_index++;
			is_unadorned = true;
		}

		// header: record description
		if (curentry.is_header())
			last_header = curentry.description();

		// otherwise, output entries for all non-command items
		else if (!curentry.is_command())
		{
			if (!curentry.is_internal())
			{
				// look up counterpart in diff, if diff is specified
				if (diff == nullptr || strcmp(value, diff->value(name)) != 0)
				{
					// output header, if we have one
					if (last_header != nullptr)
					{
						if (num_valid_headers++)
							buffer << '\n';
						util::stream_format(buffer, "#\n# %s\n#\n", last_header);
						last_header = nullptr;
					}

					// and finally output the data, skip if unadorned
					if (!is_unadorned)
					{
						if (strchr(value, ' ') != nullptr)
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
	for (entry &curentry : m_entrylist)
	{
		// header: just print
		if (curentry.is_header())
			util::stream_format(buffer, "\n#\n# %s\n#\n", curentry.description());

		// otherwise, output entries for all non-deprecated items
		else if (curentry.description() != nullptr)
			util::stream_format(buffer, "-%-20s%s\n", curentry.name(), curentry.description());
	}
	return buffer.str();
}


//-------------------------------------------------
//  value - return the raw option value
//-------------------------------------------------

const char *core_options::value(const char *name) const
{
	auto curentry = m_entrymap.find(name);
	return (curentry != m_entrymap.end()) ? curentry->second->value() : "";
}


//-------------------------------------------------
//  description - return description of option
//-------------------------------------------------

const char *core_options::description(const char *name) const
{
	auto curentry = m_entrymap.find(name);
	return (curentry != m_entrymap.end()) ? curentry->second->description() : "";
}


//-------------------------------------------------
//  priority - return the priority of option
//-------------------------------------------------

int core_options::priority(const char *name) const
{
	auto curentry = m_entrymap.find(name);
	return (curentry != m_entrymap.end()) ? curentry->second->priority() : 0;
}


//-------------------------------------------------
//  seqid - return the seqid for a given option
//-------------------------------------------------

uint32_t core_options::seqid(const char *name) const
{
	auto curentry = m_entrymap.find(name);
	return (curentry != m_entrymap.end()) ? curentry->second->seqid() : 0;
}

//-------------------------------------------------
//  exists - return if option exists in list
//-------------------------------------------------

bool core_options::exists(const char *name) const
{
	return (m_entrymap.find(name) != m_entrymap.end());
}

//-------------------------------------------------
//  is_changed - return if option have been marked
//  changed
//-------------------------------------------------

bool core_options::is_changed(const char *name) const
{
	auto curentry = m_entrymap.find(name);
	return (curentry != m_entrymap.end()) ? curentry->second->is_changed() : false;
}
//-------------------------------------------------
//  set_value - set the raw option value
//-------------------------------------------------

bool core_options::set_value(const char *name, const char *value, int priority, std::string &error_string)
{
	// find the entry first
	auto curentry = m_entrymap.find(name);
	if (curentry == m_entrymap.end())
	{
		error_string.append(string_format("Attempted to set unknown option %s\n", name));
		return false;
	}

	// validate and set the item normally
	return validate_and_set_data(*curentry->second, value, priority, error_string);
}

bool core_options::set_value(const char *name, int value, int priority, std::string &error_string)
{
	return set_value(name, string_format("%d", value).c_str(), priority, error_string);
}

bool core_options::set_value(const char *name, float value, int priority, std::string &error_string)
{
	return set_value(name, string_format("%f", value).c_str(), priority, error_string);
}


void core_options::set_flag(const char *name, uint32_t mask, uint32_t flag)
{
	// find the entry first
	auto curentry = m_entrymap.find(name);
	if ( curentry == m_entrymap.end())
	{
		return;
	}
	curentry->second->set_flag(mask, flag);
}

void core_options::mark_changed(const char* name)
{
	// find the entry first
	auto curentry = m_entrymap.find(name);
	if (curentry == m_entrymap.end())
	{
		return;
	}
	curentry->second->mark_changed();
}

//-------------------------------------------------
//  reset - reset the options state, removing
//  everything
//-------------------------------------------------

void core_options::reset()
{
	m_entrylist.reset();
	m_entrymap.clear();
}


//-------------------------------------------------
//  append_entry - append an entry to our list
//  and index it in the map
//-------------------------------------------------

void core_options::append_entry(core_options::entry &newentry)
{
	m_entrylist.append(newentry);

	// if we have names, add them to the map
	for (int name = 0; name < ARRAY_LENGTH(newentry.m_name); name++)
		if (newentry.name(name) != nullptr)
		{
			m_entrymap.insert(std::make_pair(newentry.name(name), &newentry));
			// for boolean options add a "no" variant as well
			if (newentry.type() == OPTION_BOOLEAN)
				m_entrymap.insert(std::make_pair(std::string("no").append(newentry.name(name)), &newentry));
		}
}


//-------------------------------------------------
//  remove_entry - remove an entry from our list
//  and map
//-------------------------------------------------

void core_options::remove_entry(core_options::entry &delentry)
{
	// remove all names from the map
	for (int name = 0; name < ARRAY_LENGTH(delentry.m_name); name++)
		if (!delentry.m_name[name].empty())
		{
			auto entry = m_entrymap.find(delentry.m_name[name]);
			if (entry!= m_entrymap.end()) m_entrymap.erase(entry);
		}

	// remove the entry from the list
	m_entrylist.remove(delentry);
}

/**
 * @fn  void core_options::copyfrom(const core_options &src)
 *
 * @brief   -------------------------------------------------
 *            copyfrom - copy options from another set
 *          -------------------------------------------------.
 *
 * @param   src Source for the.
 */

void core_options::copyfrom(const core_options &src)
{
	// reset ourselves first
	reset();

	// iterate through the src options and make our own
	for (entry &curentry : src.m_entrylist)
		append_entry(*global_alloc(entry(curentry.name(), curentry.description(), curentry.flags(), curentry.default_value())));
}

/**
 * @fn  bool core_options::validate_and_set_data(core_options::entry &curentry, const char *newdata, int priority, std::string &error_string)
 *
 * @brief   -------------------------------------------------
 *            validate_and_set_data - make sure the data is of the appropriate type and within
 *            range, then set it
 *          -------------------------------------------------.
 *
 * @param [in,out]  curentry        The curentry.
 * @param   newdata                 The newdata.
 * @param   priority                The priority.
 * @param [in,out]  error_string    The error string.
 *
 * @return  true if it succeeds, false if it fails.
 */

bool core_options::validate_and_set_data(core_options::entry &curentry, std::string &&data, int priority, std::string &error_string)
{
	// trim any whitespace
	strtrimspace(data);

	// trim quotes
	if (data.find_first_of('"') == 0 && data.find_last_of('"') == data.length() - 1)
	{
		data.erase(0, 1);
		data.erase(data.length() - 1, 1);
	}

	// validate the type of data and optionally the range
	float fval;
	int ival;
	switch (curentry.type())
	{
	// booleans must be 0 or 1
	case OPTION_BOOLEAN:
		if (sscanf(data.c_str(), "%d", &ival) != 1 || ival < 0 || ival > 1)
		{
			error_string.append(string_format("Illegal boolean value for %s: \"%s\"; reverting to %s\n", curentry.name(), data.c_str(), curentry.value()));
			return false;
		}
		break;

	// integers must be integral
	case OPTION_INTEGER:
		if (sscanf(data.c_str(), "%d", &ival) != 1)
		{
			error_string.append(string_format("Illegal integer value for %s: \"%s\"; reverting to %s\n", curentry.name(), data.c_str(), curentry.value()));
			return false;
		}
		if (curentry.has_range() && (ival < atoi(curentry.minimum()) || ival > atoi(curentry.maximum())))
		{
			error_string.append(string_format("Out-of-range integer value for %s: \"%s\" (must be between %s and %s); reverting to %s\n", curentry.name(), data.c_str(), curentry.minimum(), curentry.maximum(), curentry.value()));
			return false;
		}
		break;

	// floating-point values must be numeric
	case OPTION_FLOAT:
		if (sscanf(data.c_str(), "%f", &fval) != 1)
		{
			error_string.append(string_format("Illegal float value for %s: \"%s\"; reverting to %s\n", curentry.name(), data.c_str(), curentry.value()));
			return false;
		}
		if (curentry.has_range() && ((double) fval < atof(curentry.minimum()) || (double) fval > atof(curentry.maximum())))
		{
			error_string.append(string_format("Out-of-range float value for %s: \"%s\" (must be between %s and %s); reverting to %s\n", curentry.name(), data.c_str(), curentry.minimum(), curentry.maximum(), curentry.value()));
			return false;
		}
		break;

	// strings can be anything
	case OPTION_STRING:
		break;

	// anything else is invalid
	case OPTION_INVALID:
	case OPTION_HEADER:
	default:
		error_string.append(string_format("Attempted to set invalid option %s\n", curentry.name()));
		return false;
	}

	// set the data
	curentry.set_value(data.c_str(), priority);
	value_changed(curentry.name(), data);
	return true;
}

core_options::entry *core_options::get_entry(const char *name) const
{
	auto curentry = m_entrymap.find(name);
	return (curentry != m_entrymap.end()) ? curentry->second : nullptr;
}
