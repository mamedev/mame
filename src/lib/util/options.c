/***************************************************************************

    options.c

    Core options code code

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

core_options::entry::entry(const options_entry &entrylist)
	: m_next(NULL),
	  m_flags(entrylist.flags),
	  m_seqid(0),
	  m_error_reported(false),
	  m_priority(OPTION_PRIORITY_DEFAULT),
	  m_description(entrylist.description)
{
	// copy in the name(s) as appropriate
	if (entrylist.name != NULL)
	{
		// first extract any range
		astring namestr(entrylist.name);
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
	if (entrylist.defvalue != NULL)
		m_defdata = entrylist.defvalue;
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
	: m_entrylist(NULL),
	  m_entrylist_tailptr(&m_entrylist)
{
}

core_options::core_options(const options_entry *entrylist)
	: m_entrylist(NULL),
	  m_entrylist_tailptr(&m_entrylist)
{
	add_entries(entrylist);
}

core_options::core_options(const options_entry *entrylist1, const options_entry *entrylist2)
	: m_entrylist(NULL),
	  m_entrylist_tailptr(&m_entrylist)
{
	add_entries(entrylist1);
	add_entries(entrylist2);
}

core_options::core_options(const options_entry *entrylist1, const options_entry *entrylist2, const options_entry *entrylist3)
	: m_entrylist(NULL),
	  m_entrylist_tailptr(&m_entrylist)
{
	add_entries(entrylist1);
	add_entries(entrylist2);
	add_entries(entrylist3);
}

core_options::core_options(const core_options &src)
	: m_entrylist(NULL),
	  m_entrylist_tailptr(&m_entrylist)
{
	copyfrom(src);
}


//-------------------------------------------------
//  ~core_options - destructor
//-------------------------------------------------

core_options::~core_options()
{
	// delete all entries from the list
	while (m_entrylist != NULL)
	{
		core_options::entry *e = m_entrylist;
		remove_entry(*m_entrylist);
		delete e;
	}
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
	for (entry *curentry = m_entrylist; curentry != NULL; curentry = curentry->next())
		if (!curentry->is_header())
		{
			// if the values differ, return false
			if (strcmp(curentry->m_data, rhs.value(curentry->name())) != 0)
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
//  add_entries - add entries to the current
//  options sets
//-------------------------------------------------

void core_options::add_entries(const options_entry *entrylist, bool override_existing)
{
	// loop over entries until we hit a NULL name
	for ( ; entrylist->name != NULL || (entrylist->flags & OPTION_HEADER) != 0; entrylist++)
	{
		// allocate a new entry
		entry *newentry = new entry(*entrylist);
		if (newentry->name() != NULL)
		{
			// see if we match an existing entry
			entry *existing = m_entrymap.find(newentry->name());
			if (existing != NULL)
			{
				// if we're overriding existing entries, then remove the old one
				if (override_existing)
				{
					core_options::entry *e = m_entrylist;
					remove_entry(*existing);
					delete e;
				}

				// otherwise, just override the default and current values and throw out the new entry
				else
				{
					existing->set_default_value(newentry->value());
					delete newentry;
					continue;
				}
			}
		}

		// add us to the list and maps
		append_entry(*newentry);
	}
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
	bool retVal = true;
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
			retVal = false;
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
	return retVal;
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
	for (entry *curentry = m_entrylist; curentry != NULL; curentry = curentry->next())
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
	for (entry *curentry = m_entrylist; curentry != NULL; curentry = curentry->next())
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
	for (entry *curentry = m_entrylist; curentry != NULL; curentry = curentry->next())
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


//-------------------------------------------------
//  reset - reset the options state, removing
//  everything
//-------------------------------------------------

void core_options::reset()
{
	// remove all entries from the list
	while (m_entrylist != NULL)
	{
		core_options::entry *e = m_entrylist;
		remove_entry(*m_entrylist);
		delete e;
	}

	// reset the map
	m_entrymap.reset();
}


//-------------------------------------------------
//  append_entry - append an entry to our list
//  and index it in the map
//-------------------------------------------------

void core_options::append_entry(core_options::entry &newentry)
{
	// append to the list
	*m_entrylist_tailptr = &newentry;
	m_entrylist_tailptr = &newentry.m_next;

	// if we have names, add them to the map
	astring tempstr;
	for (int name = 0; name < ARRAY_LENGTH(newentry.m_name); name++)
		if (newentry.m_name[name])
		{
			m_entrymap.add(newentry.m_name[name], &newentry);

			// for boolean options add a "no" variant as well
			if (newentry.type() == OPTION_BOOLEAN)
				m_entrymap.add(tempstr.cpy("no").cat(newentry.m_name[name]), &newentry);
		}
}


//-------------------------------------------------
//  remove_entry - remove an entry from our list
//  and map
//-------------------------------------------------

void core_options::remove_entry(core_options::entry &delentry)
{
	// remove us from the list
	entry *preventry = NULL;
	for (entry *curentry = m_entrylist; curentry != NULL; curentry = curentry->next())
		if (curentry == &delentry)
		{
			// update link from previous to us
			if (preventry != NULL)
				preventry->m_next = delentry.m_next;
			else
				m_entrylist = delentry.m_next;

			// if we're the last item, update the next pointer
			if (delentry.m_next == NULL)
			{
				if (preventry != NULL)
					m_entrylist_tailptr = &preventry->m_next;
				else
					m_entrylist_tailptr = &m_entrylist;
			}

			// remove all entries from the map
			for (int name = 0; name < ARRAY_LENGTH(delentry.m_name); name++)
				if (delentry.m_name[name])
					m_entrymap.remove(delentry.m_name[name]);
			break;
		}
}


//-------------------------------------------------
//  copyfrom - copy options from another set
//-------------------------------------------------

void core_options::copyfrom(const core_options &src)
{
	// reset ourselves first
	reset();

	// iterate through the src options and make our own
	for (entry *curentry = src.m_entrylist; curentry != NULL; curentry = curentry->next())
		append_entry(*new entry(*curentry));
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
