/***************************************************************************

    options.h

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

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "osdcore.h"
#include "corefile.h"
#include "tagmap.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// option types
const UINT32 OPTION_TYPE_MASK		= 0x0007;		// up to 8 different types
enum
{
	OPTION_INVALID,			// invalid
	OPTION_HEADER,			// a header item
	OPTION_COMMAND,			// a command
	OPTION_BOOLEAN,			// boolean option
	OPTION_INTEGER,			// integer option
	OPTION_FLOAT,			// floating-point option
	OPTION_STRING			// string option
};

// option priorities
const int OPTION_PRIORITY_DEFAULT	= 0;			// defaults are at 0 priority
const int OPTION_PRIORITY_LOW		= 50;			// low priority
const int OPTION_PRIORITY_NORMAL	= 100;			// normal priority
const int OPTION_PRIORITY_HIGH		= 150;			// high priority
const int OPTION_PRIORITY_MAXIMUM	= 255;			// maximum priority



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// static structure describing a single option with its description and default value
struct options_entry
{
	const char *		name;				// name on the command line
	const char *		defvalue;			// default value of this argument
	UINT32				flags;				// flags to describe the option
	const char *		description;		// description for -showusage
};


// structure holding information about a collection of options
class core_options
{
	static const int MAX_UNADORNED_OPTIONS = 16;

public:
	// information about a single entry in the options
	class entry
	{
		friend class core_options;

		// construction/destruction
		entry(const options_entry &entry);

	public:
		// getters
		entry *next() const { return m_next; }
		const char *name() const { return m_name[0] ? m_name[0].cstr() : NULL; }
		const char *description() const { return m_description; }
		const char *value() const { return m_data; }
		const char *default_value() const { return m_defdata; }
		const char *minimum() const { return m_minimum; }
		const char *maximum() const { return m_maximum; }
		UINT32 seqid() const { return m_seqid; }
		int type() const { return (m_flags & OPTION_TYPE_MASK); }
		UINT32 flags() const { return m_flags; }
		bool is_header() const { return type() == OPTION_HEADER; }
		bool is_command() const { return type() == OPTION_COMMAND; }
		bool has_range() const { return (m_minimum && m_maximum); }
		int priority() const { return m_priority; }

		// setters
		void set_value(const char *newvalue, int priority);
		void set_default_value(const char *defvalue);
		void revert(int priority);

	private:
		// internal state
		entry *					m_next;				// link to the next data
		UINT32					m_flags;			// flags from the entry
		UINT32					m_seqid;			// sequence ID; bumped on each change
		bool					m_error_reported;	// have we reported an error on this option yet?
		int						m_priority;			// priority of the data set
		const char *			m_description;		// description for this item
		astring					m_name[4];			// up to 4 names for the item
		astring					m_data;				// data for this item
		astring					m_defdata;			// default data for this item
		astring					m_minimum;			// minimum value
		astring					m_maximum;			// maximum value
	};

	// construction/destruction
	core_options();
	core_options(const options_entry *entrylist);
	core_options(const options_entry *entrylist1, const options_entry *entrylist2);
	core_options(const options_entry *entrylist1, const options_entry *entrylist2, const options_entry *entrylist3);
	core_options(const core_options &src);
	virtual ~core_options();

	// operators
	core_options &operator=(const core_options &rhs);
	bool operator==(const core_options &rhs);
	bool operator!=(const core_options &rhs);

	// getters
	entry *first() const { return m_entrylist; }
	const char *command() const { return m_command; }

	// configuration
	void add_entries(const options_entry *entrylist, bool override_existing = false);
	void set_default_value(const char *name, const char *defvalue);
	void remove_entry(entry &delentry);

	// parsing/input
	bool parse_command_line(int argc, char **argv, int priority, astring &error_string);
	bool parse_ini_file(core_file &inifile, int priority, int ignore_priority, astring &error_string);

	// reverting
	void revert(int priority = OPTION_PRIORITY_MAXIMUM);

	// output
	const char *output_ini(astring &buffer, const core_options *diff = NULL);
	const char *output_help(astring &buffer);

	// reading
	const char *value(const char *option) const;
	int priority(const char *option) const;
	bool bool_value(const char *name) const { return (atoi(value(name)) != 0); }
	int int_value(const char *name) const { return atoi(value(name)); }
	float float_value(const char *name) const { return atof(value(name)); }
	UINT32 seqid(const char *name) const;
	bool exists(const char *name) const;

	// setting
	void set_command(const char *command);
	bool set_value(const char *name, const char *value, int priority, astring &error_string);
	bool set_value(const char *name, int value, int priority, astring &error_string);
	bool set_value(const char *name, float value, int priority, astring &error_string);

	// misc
	static const char *unadorned(int x = 0) { return s_option_unadorned[MIN(x, MAX_UNADORNED_OPTIONS)]; }
	int options_count();
private:
	// internal helpers
	void reset();
	void append_entry(entry &newentry);
	void copyfrom(const core_options &src);
	bool validate_and_set_data(entry &curentry, const char *newdata, int priority, astring &error_string);

	// internal state
	entry *					m_entrylist;			// head of list of entries
	entry **				m_entrylist_tailptr;	// pointer to tail of entry list
	tagmap_t<entry *>		m_entrymap;				// map for fast lookup
	astring					m_command;				// command found
	static const char *const s_option_unadorned[];	// array of unadorned option "names"
};



#endif /* __OPTIONS_H__ */
