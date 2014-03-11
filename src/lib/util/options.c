// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    options.c

    Core options code code

***************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include "options.h"
#include "astring.h"



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

core_options::entry::entry(const char *name, const char *description, UINT32 flags, const char *defvalue)
	: m_next(NULL),
		m_flags(flags),
		m_seqid(0),
		m_error_reported(false),
		m_priority(OPTION_PRIORITY_DEFAULT),
		m_description(description)
{
	// copy in the name(s) as appropriate
	if (name != NULL)
	{
		// first extract any range
		astring namestr(name);
		int lparen = namestr.chr(0, '(');
		int dash = namestr.chr(lparen + 1, '-');
		int rparen = namestr.chr(dash + 1, ')');
		if (lparen != -1 && dash != -1 && rparen != -1)
		{
			m_minimum.cpysubstr(namestr, lparen + 1, dash - (lparen + 1)).trimspace();
			m_maximum.cpysubstr(namestr, dash + 1, rparen - (dash + 1)).trimspace();
			namestr.del(lparen, rparen + 1 - lparen);
		}

		// then chop up any semicolon-separated names
		int semi;
		int nameindex = 0;
		while ((semi = namestr.chr(0, ';')) != -1 && nameindex < ARRAY_LENGTH(m_name))
		{
			m_name[nameindex++].cpysubstr(namestr, 0, semi);
			namestr.del(0, semi + 1);
		}

		// finally add the last item
		if (nameindex < ARRAY_LENGTH(m_name))
			m_name[nameindex++] = namestr;
	}

	// set the default value
	if (defvalue != NULL)
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


void core_options::entry::set_flag(UINT32 mask, UINT32 flag)
{
	m_flags = ( m_flags & mask ) | flag;
}


//-------------------------------------------------
//  revert - revert back to our default if we are
//  at or below the given priority
//-------------------------------------------------

void core_options::entry::revert(int priority)
{
	// if our priority is low enough, revert to the default
	if (m_priority <= priority)
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
	for (entry *curentry = m_entrylist.first(); curentry != NULL; curentry = curentry->next())
		if (!curentry->is_header())
		{
			// if the values differ, return false
			if (strcmp(curentry->value(), rhs.value(curentry->name())) != 0)
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

void core_options::add_entry(const char *name, const char *description, UINT32 flags, const char *defvalue, bool override_existing)
{
	// allocate a new entry
	entry *newentry = global_alloc(entry(name, description, flags, defvalue));
	if (newentry->name() != NULL)
	{
		// see if we match an existing entry
		entry *existing = m_entrymap.find(newentry->name());
		if (existing != NULL)
		{
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
	// loop over entries until we hit a NULL name
	for ( ; entrylist->name != NULL || (entrylist->flags & OPTION_HEADER) != 0; entrylist++)
		add_entry(*entrylist, override_existing);
}


//-------------------------------------------------
//  set_default_value - change the default value
//  of an option
//-------------------------------------------------

void core_options::set_default_value(const char *name, const char *defvalue)
{
	// find the entry and bail if we can't
	entry *curentry = m_entrymap.find(name);
	if (curentry == NULL)
		return;

	// update the data and default data
	curentry->set_default_value(defvalue);
}


//-------------------------------------------------
//  parse_command_line - parse a series of
//  command line arguments
//-------------------------------------------------

bool core_options::parse_command_line(int argc, char **argv, int priority, astring &error_string)
{
	// reset the errors and the command
	error_string.reset();
	m_command.reset();

	// iterate through arguments
	int unadorned_index = 0;
	bool retval = true;
	for (int arg = 1; arg < argc; arg++)
	{
		// determine the entry name to search for
		const char *curarg = argv[arg];
		bool is_unadorned = (curarg[0] != '-');
		const char *optionname = is_unadorned ? core_options::unadorned(unadorned_index++) : &curarg[1];

		// find our entry; if not found, indicate invalid option
		entry *curentry = m_entrymap.find(optionname);
		if (curentry == NULL)
		{
			error_string.catprintf("Error: unknown option: %s\n", curarg);
			retval = false;
			if (!is_unadorned) arg++;
			continue;
		}

		// process commands first
		if (curentry->type() == OPTION_COMMAND)
		{
			// can only have one command
			if (m_command)
			{
				error_string.catprintf("Error: multiple commands specified -%s and %s\n", m_command.cstr(), curarg);
				return false;
			}
			m_command = curentry->name();
			continue;
		}

		// get the data for this argument, special casing booleans
		const char *newdata;
		if (curentry->type() == OPTION_BOOLEAN)
			newdata = (strncmp(&curarg[1], "no", 2) == 0) ? "0" : "1";
		else if (is_unadorned)
			newdata = curarg;
		else if (arg + 1 < argc)
			newdata = argv[++arg];
		else
		{
			error_string.catprintf("Error: option %s expected a parameter\n", curarg);
			return false;
		}

		// set the new data
		validate_and_set_data(*curentry, newdata, priority, error_string);
	}
	return retval;
}


//-------------------------------------------------
//  parse_ini_file - parse a series of entries in
//  an INI file
//-------------------------------------------------

bool core_options::parse_ini_file(core_file &inifile, int priority, int ignore_priority, astring &error_string)
{
	// loop over lines in the file
	char buffer[4096];
	while (core_fgets(buffer, ARRAY_LENGTH(buffer), &inifile) != NULL)
	{
		// find the extent of the name
		char *optionname;
		for (optionname = buffer; *optionname != 0; optionname++)
			if (!isspace((UINT8)*optionname))
				break;

		// skip comments
		if (*optionname == 0 || *optionname == '#')
			continue;

		// scan forward to find the first space
		char *temp;
		for (temp = optionname; *temp != 0; temp++)
			if (isspace((UINT8)*temp))
				break;

		// if we hit the end early, print a warning and continue
		if (*temp == 0)
		{
			error_string.catprintf("Warning: invalid line in INI: %s", buffer);
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
		entry *curentry = m_entrymap.find(optionname);
		if (curentry == NULL)
		{
			if (priority >= ignore_priority)
				error_string.catprintf("Warning: unknown option in INI: %s\n", optionname);
			continue;
		}

		// set the new data
		validate_and_set_data(*curentry, optiondata, priority, error_string);
	}
	return true;
}


//-------------------------------------------------
//  revert - revert options at or below a certain
//  priority back to their defaults
//-------------------------------------------------

void core_options::revert(int priority)
{
	// iterate over options and revert to defaults if below the given priority
	for (entry *curentry = m_entrylist.first(); curentry != NULL; curentry = curentry->next())
		curentry->revert(priority);
}


//-------------------------------------------------
//  output_ini - output the options in INI format,
//  only outputting entries that different from
//  the optional diff
//-------------------------------------------------

const char *core_options::output_ini(astring &buffer, const core_options *diff)
{
	// INI files are complete, so always start with a blank buffer
	buffer.reset();

	int num_valid_headers = 0;
	int unadorned_index = 0;
	const char *last_header = NULL;

	// loop over all items
	for (entry *curentry = m_entrylist.first(); curentry != NULL; curentry = curentry->next())
	{
		const char *name = curentry->name();
		const char *value = curentry->value();
		bool is_unadorned = false;

		// check if it's unadorned
		if (name && strlen(name) && !strcmp(name, core_options::unadorned(unadorned_index)))
		{
			unadorned_index++;
			is_unadorned = true;
		}

		// header: record description
		if (curentry->is_header())
			last_header = curentry->description();

		// otherwise, output entries for all non-command items
		else if (!curentry->is_command())
		{
			if ( !curentry->is_internal() )
			{
				// look up counterpart in diff, if diff is specified
				if (diff == NULL || strcmp(value, diff->value(name)) != 0)
				{
					// output header, if we have one
					if (last_header != NULL)
					{
						if (num_valid_headers++)
							buffer.catprintf("\n");
						buffer.catprintf("#\n# %s\n#\n", last_header);
						last_header = NULL;
					}

					// and finally output the data, skip if unadorned
					if (!is_unadorned)
					{
						if (strchr(value, ' ') != NULL)
							buffer.catprintf("%-25s \"%s\"\n", name, value);
						else
							buffer.catprintf("%-25s %s\n", name, value);
					}
				}
			}
		}
	}
	return buffer;
}


//-------------------------------------------------
//  output_help - output option help to a string
//-------------------------------------------------

const char *core_options::output_help(astring &buffer)
{
	// start empty
	buffer.reset();

	// loop over all items
	for (entry *curentry = m_entrylist.first(); curentry != NULL; curentry = curentry->next())
	{
		// header: just print
		if (curentry->is_header())
			buffer.catprintf("\n#\n# %s\n#\n", curentry->description());

		// otherwise, output entries for all non-deprecated items
		else if (curentry->description() != NULL)
			buffer.catprintf("-%-20s%s\n", curentry->name(), curentry->description());
	}
	return buffer;
}


//-------------------------------------------------
//  value - return the raw option value
//-------------------------------------------------

const char *core_options::value(const char *name) const
{
	entry *curentry = m_entrymap.find(name);
	return (curentry != NULL) ? curentry->value() : "";
}


//-------------------------------------------------
//  priority - return the priority of option
//-------------------------------------------------

int core_options::priority(const char *name) const
{
	entry *curentry = m_entrymap.find(name);
	return (curentry != NULL) ? curentry->priority() : 0;
}


//-------------------------------------------------
//  seqid - return the seqid for a given option
//-------------------------------------------------

UINT32 core_options::seqid(const char *name) const
{
	entry *curentry = m_entrymap.find(name);
	return (curentry != NULL) ? curentry->seqid() : 0;
}

//-------------------------------------------------
//  exists - return if option exists in list
//-------------------------------------------------

bool core_options::exists(const char *name) const
{
	entry *curentry = m_entrymap.find(name);
	return (curentry != NULL);
}

//-------------------------------------------------
//  set_value - set the raw option value
//-------------------------------------------------

bool core_options::set_value(const char *name, const char *value, int priority, astring &error_string)
{
	// find the entry first
	entry *curentry = m_entrymap.find(name);
	if (curentry == NULL)
	{
		error_string.catprintf("Attempted to set unknown option %s\n", name);
		return false;
	}

	// validate and set the item normally
	return validate_and_set_data(*curentry, value, priority, error_string);
}

bool core_options::set_value(const char *name, int value, int priority, astring &error_string)
{
	astring tempstr;
	tempstr.printf("%d", value);
	return set_value(name, tempstr.cstr(), priority, error_string);
}

bool core_options::set_value(const char *name, float value, int priority, astring &error_string)
{
	astring tempstr;
	tempstr.printf("%f", value);
	return set_value(name, tempstr.cstr(), priority, error_string);
}


void core_options::set_flag(const char *name, UINT32 mask, UINT32 flag)
{
	// find the entry first
	entry *curentry = m_entrymap.find(name);
	if ( curentry == NULL )
	{
		return;
	}
	curentry->set_flag(mask, flag);
}


//-------------------------------------------------
//  reset - reset the options state, removing
//  everything
//-------------------------------------------------

void core_options::reset()
{
	m_entrylist.reset();
	m_entrymap.reset();
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
		if (newentry.name(name) != NULL)
		{
			m_entrymap.add(newentry.name(name), &newentry);

			// for boolean options add a "no" variant as well
			if (newentry.type() == OPTION_BOOLEAN)
				m_entrymap.add(astring("no", newentry.name(name)), &newentry);
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
		if (delentry.m_name[name])
			m_entrymap.remove(delentry.m_name[name]);

	// remove the entry from the list
	m_entrylist.remove(delentry);
}


//-------------------------------------------------
//  copyfrom - copy options from another set
//-------------------------------------------------

void core_options::copyfrom(const core_options &src)
{
	// reset ourselves first
	reset();

	// iterate through the src options and make our own
	for (entry *curentry = src.m_entrylist.first(); curentry != NULL; curentry = curentry->next())
		append_entry(*global_alloc(entry(curentry->name(), curentry->description(), curentry->flags(), curentry->default_value())));
}


//-------------------------------------------------
//  validate_and_set_data - make sure the data is
//  of the appropriate type and within range,
//  then set it
//-------------------------------------------------

bool core_options::validate_and_set_data(core_options::entry &curentry, const char *newdata, int priority, astring &error_string)
{
	// trim any whitespace
	astring data(newdata);
	data.trimspace();

	// trim quotes
	if (data.chr(0, '"') == 0 && data.rchr(0, '"') == data.len() - 1)
	{
		data.del(0, 1);
		data.del(data.len() - 1, 1);
	}

	// validate the type of data and optionally the range
	float fval;
	int ival;
	switch (curentry.type())
	{
		// booleans must be 0 or 1
		case OPTION_BOOLEAN:
			if (sscanf(data, "%d", &ival) != 1 || ival < 0 || ival > 1)
			{
				error_string.catprintf("Illegal boolean value for %s: \"%s\"; reverting to %s\n", curentry.name(), data.cstr(), curentry.value());
				return false;
			}
			break;

		// integers must be integral
		case OPTION_INTEGER:
			if (sscanf(data, "%d", &ival) != 1)
			{
				error_string.catprintf("Illegal integer value for %s: \"%s\"; reverting to %s\n", curentry.name(), data.cstr(), curentry.value());
				return false;
			}
			if (curentry.has_range() && (ival < atoi(curentry.minimum()) || ival > atoi(curentry.maximum())))
			{
				error_string.catprintf("Out-of-range integer value for %s: \"%s\" (must be between %s and %s); reverting to %s\n", curentry.name(), data.cstr(), curentry.minimum(), curentry.maximum(), curentry.value());
				return false;
			}
			break;

		// floating-point values must be numeric
		case OPTION_FLOAT:
			if (sscanf(data, "%f", &fval) != 1)
			{
				error_string.catprintf("Illegal float value for %s: \"%s\"; reverting to %s\n", curentry.name(), data.cstr(), curentry.value());
				return false;
			}
			if (curentry.has_range() && (fval < atof(curentry.minimum()) || fval > atof(curentry.maximum())))
			{
				error_string.catprintf("Out-of-range float value for %s: \"%s\" (must be between %s and %s); reverting to %s\n", curentry.name(), data.cstr(), curentry.minimum(), curentry.maximum(), curentry.value());
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
			error_string.catprintf("Attempted to set invalid option %s\n", curentry.name());
			return false;
	}

	// set the data
	curentry.set_value(data, priority);
	return true;
}
