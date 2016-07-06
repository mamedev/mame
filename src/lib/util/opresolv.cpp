// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    opresolv.h

    Extensible ranged option resolution handling

****************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "pool.h"
#include "corestr.h"
#include "opresolv.h"


namespace util {

/***************************************************************************
	option_resolution
***************************************************************************/

// -------------------------------------------------
//	entry::int_value
// -------------------------------------------------

int option_resolution::entry::int_value() const
{
	return atoi(value.c_str());
}


// -------------------------------------------------
//	entry::set_int_value
// -------------------------------------------------

void option_resolution::entry::set_int_value(int i)
{
	value = string_format("%d", i);
}


// -------------------------------------------------
//	resolve_single_param
// -------------------------------------------------

option_resolution::error option_resolution::resolve_single_param(const char *specification, option_resolution::entry *param_value,
	struct range *range, size_t range_count)
{
	int FLAG_IN_RANGE           = 0x01;
	int FLAG_IN_DEFAULT         = 0x02;
	int FLAG_DEFAULT_SPECIFIED  = 0x04;
	int FLAG_HALF_RANGE         = 0x08;

	int last_value = 0;
	int value = 0;
	int flags = 0;
	const char *s = specification;

	while(*s && !isalpha(*s))
	{
		if (*s == '-')
		{
			/* range specifier */
			if (flags & (FLAG_IN_RANGE|FLAG_IN_DEFAULT))
			{
				return error::SYNTAX;
			}
			flags |= FLAG_IN_RANGE;
			s++;

			if (range)
			{
				range->max = -1;
				if ((flags & FLAG_HALF_RANGE) == 0)
				{
					range->min = -1;
					flags |= FLAG_HALF_RANGE;
				}
			}
		}
		else if (*s == '[')
		{
			/* begin default value */
			if (flags & (FLAG_IN_DEFAULT|FLAG_DEFAULT_SPECIFIED))
			{
				return error::SYNTAX;
			}
			flags |= FLAG_IN_DEFAULT;
			s++;
		}
		else if (*s == ']')
		{
			/* end default value */
			if ((flags & FLAG_IN_DEFAULT) == 0)
			{
				return error::SYNTAX;
			}
			flags &= ~FLAG_IN_DEFAULT;
			flags |= FLAG_DEFAULT_SPECIFIED;
			s++;

			if (param_value && param_value->int_value() == -1)
				param_value->set_int_value(value);
		}
		else if (*s == '/')
		{
			/* value separator */
			if (flags & (FLAG_IN_DEFAULT|FLAG_IN_RANGE))
			{
				return error::SYNTAX;
			}
			s++;

			/* if we are spitting out ranges, complete the range */
			if (range && (flags & FLAG_HALF_RANGE))
			{
				range++;
				flags &= ~FLAG_HALF_RANGE;
				if (--range_count == 0)
					range = nullptr;
			}
		}
		else if (*s == ';')
		{
			/* basic separator */
			s++;
		}
		else if (isdigit(*s))
		{
			/* numeric value */
			last_value = value;
			value = 0;
			do
			{
				value *= 10;
				value += *s - '0';
				s++;
			}
			while(isdigit(*s));

			if (range)
			{
				if ((flags & FLAG_HALF_RANGE) == 0)
				{
					range->min = value;
					flags |= FLAG_HALF_RANGE;
				}
				range->max = value;
			}

			// if we have a value; check to see if it is out of range
			if (param_value && (param_value->int_value() != -1) && (param_value->int_value() != value))
			{
				if ((last_value < param_value->int_value()) && (param_value->int_value() < value))
				{
					if ((flags & FLAG_IN_RANGE) == 0)
						return error::PARAMOUTOFRANGE;
				}
			}
			flags &= ~FLAG_IN_RANGE;
		}
		else
		{
			return error::SYNTAX;
		}
	}

	// we can't have zero length guidelines strings
	if (s == specification)
	{
		return error::SYNTAX;
	}

	return error::SUCCESS;
}


// -------------------------------------------------
//	lookup_in_specification
// -------------------------------------------------

const char *option_resolution::lookup_in_specification(const char *specification, const option_guide *option)
{
	const char *s;
	s = strchr(specification, option->parameter);
	return s ? s + 1 : nullptr;
}


// -------------------------------------------------
//	ctor
// -------------------------------------------------

option_resolution::option_resolution(const option_guide *guide, const char *specification)
{
	const option_guide *guide_entry;
	int option_count;
	int opt = -1;

	assert(guide);

	// first count the number of options specified in the guide 
	option_count = count_options(guide, specification);

	// set up the entries list
	m_specification = specification;
	m_entries.resize(option_count);

	// initialize each of the entries 
	opt = 0;
	guide_entry = guide;
	while(guide_entry->option_type != OPTIONTYPE_END)
	{
		switch(guide_entry->option_type)
		{
		case OPTIONTYPE_INT:
		case OPTIONTYPE_ENUM_BEGIN:
		case OPTIONTYPE_STRING:
			if (lookup_in_specification(specification, guide_entry))
				m_entries[opt++].guide_entry = guide_entry;
			break;

		case OPTIONTYPE_ENUM_VALUE:
			break;

		default:
			assert(false && "Invalid option type");
			break;
		}
		guide_entry++;
	}
	assert(opt == option_count);
}


// -------------------------------------------------
//	dtor
// -------------------------------------------------

option_resolution::~option_resolution()
{
}


// -------------------------------------------------
//	add_param
// -------------------------------------------------

option_resolution::error option_resolution::add_param(const char *param, const std::string &value)
{
	int i;
	bool must_resolve;
	error err;
	const char *option_specification;
	entry *entry = nullptr;

	for(auto &this_entry : m_entries)
	{
		if (!strcmp(param, this_entry.guide_entry->identifier))
		{
			entry = &this_entry;
			break;
		}
	}
	if (entry == nullptr)
		return error::PARAMNOTFOUND;

	if (entry->state != entry_state::UNSPECIFIED)
		return error::PARAMALREADYSPECIFIED;

	switch(entry->guide_entry->option_type) {
	case OPTIONTYPE_INT:
		entry->set_int_value(atoi(value.c_str()));
		entry->state = entry_state::SPECIFIED;
		must_resolve = true;
		break;

	case OPTIONTYPE_STRING:
		entry->value = value;
		entry->state = entry_state::SPECIFIED;
		must_resolve = false;
		break;

	case OPTIONTYPE_ENUM_BEGIN:
		for (i = 1; entry->guide_entry[i].option_type == OPTIONTYPE_ENUM_VALUE; i++)
		{
			if (!core_stricmp(value.c_str(), entry->guide_entry[i].identifier))
			{
				entry->set_int_value(entry->guide_entry[i].parameter);
				entry->state = entry_state::SPECIFIED;
				break;
			}
		}
		if (entry->state != entry_state::SPECIFIED)
			return error::BADPARAM;

		must_resolve = true;
		break;

	default:
		assert(0);
		return error::INTERNAL;
	}

	// do a resolution step if necessary
	if (must_resolve)
	{
		option_specification = lookup_in_specification(m_specification, entry->guide_entry);
		err = resolve_single_param(option_specification, entry, nullptr, 0);
		if (err != error::SUCCESS)
			return err;

		// did we not get a real value?
		if (entry->int_value() < 0)
			return error::PARAMNOTSPECIFIED;
	}

	return error::SUCCESS;
}


// -------------------------------------------------
//	finish
// -------------------------------------------------

option_resolution::error option_resolution::finish()
{
	const char *option_specification;
	error err;

	for (auto &entry : m_entries)
	{
		if (entry.state == entry_state::UNSPECIFIED)
		{
			switch(entry.guide_entry->option_type) {
			case OPTIONTYPE_INT:
			case OPTIONTYPE_ENUM_BEGIN:
				option_specification = lookup_in_specification(m_specification, entry.guide_entry);
				assert(option_specification);
				entry.set_int_value(-1);
				err = resolve_single_param(option_specification, &entry, nullptr, 0);
				if (err != error::SUCCESS)
					return err;
				break;

			case OPTIONTYPE_STRING:
				entry.value = "";
				break;

			default:
				assert(FALSE);
				return error::INTERNAL;
			}
			entry.state = entry_state::SPECIFIED;
		}
	}
	return error::SUCCESS;
}


// -------------------------------------------------
//	lookup_entry
// -------------------------------------------------

const option_resolution::entry *option_resolution::lookup_entry(int option_char) const
{
	for (auto &entry : m_entries)
	{
		switch(entry.guide_entry->option_type) {
		case OPTIONTYPE_INT:
		case OPTIONTYPE_STRING:
		case OPTIONTYPE_ENUM_BEGIN:
			if (entry.guide_entry->parameter == option_char)
				return &entry;
			break;

		default:
			assert(FALSE);
			return nullptr;
		}
	}
	return nullptr;
}


// -------------------------------------------------
//	lookup_int
// -------------------------------------------------

int option_resolution::lookup_int(int option_char) const
{
	auto entry = lookup_entry(option_char);
	return entry ? entry->int_value() : -1;
}


// -------------------------------------------------
//	lookup_string
// -------------------------------------------------

const char *option_resolution::lookup_string(int option_char) const
{
	auto entry = lookup_entry(option_char);
	return entry ? entry->value.c_str() : nullptr;
}


// -------------------------------------------------
//	find_option
// -------------------------------------------------

const option_guide *option_resolution::find_option(int option_char) const
{
	auto entry = lookup_entry(option_char);
	return entry ? entry->guide_entry : nullptr;
}


// -------------------------------------------------
//	index_option
// -------------------------------------------------

const option_guide *option_resolution::index_option(int indx) const
{
	if ((indx < 0) || (indx >= m_entries.size()))
		return nullptr;
	return m_entries[indx].guide_entry;
}


// -------------------------------------------------
//	count_options
// -------------------------------------------------

int option_resolution::count_options(const option_guide *guide, const char *specification)
{
	int option_count = 0;

	while(guide->option_type != OPTIONTYPE_END)
	{
		switch(guide->option_type) {
		case OPTIONTYPE_INT:
		case OPTIONTYPE_STRING:
		case OPTIONTYPE_ENUM_BEGIN:
			if (lookup_in_specification(specification, guide))
				option_count++;
			break;
		case OPTIONTYPE_ENUM_VALUE:
			break;
		default:
			assert(FALSE);
			return 0;
		}
		guide++;
	}
	return option_count;
}


// -------------------------------------------------
//	list_ranges
// -------------------------------------------------

option_resolution::error option_resolution::list_ranges(const char *specification, int option_char, range *range, size_t range_count)
{
	assert(range_count > 0);

	// clear out range
	memset(range, -1, sizeof(*range) * range_count);
	range_count--;

	specification = strchr(specification, option_char);
	if (!specification)
	{
		return error::SYNTAX;
	}

	return resolve_single_param(specification + 1, nullptr, range, range_count);
}


// -------------------------------------------------
//	get_default
// -------------------------------------------------

option_resolution::error option_resolution::get_default(const char *specification, int option_char, int *val)
{
	assert(val);

	// clear out default
	*val = -1;

	specification = strchr(specification, option_char);
	if (!specification)
	{
		return error::SYNTAX;
	}

	entry ent;
	auto err = resolve_single_param(specification + 1, &ent, nullptr, 0);
	*val = ent.int_value();
	return err;
}


// -------------------------------------------------
//	list_ranges
// -------------------------------------------------

option_resolution::error option_resolution::is_valid_value(const char *specification, int option_char, int val)
{
	option_resolution::error err;
	range ranges[256];
	int i;

	err = list_ranges(specification, option_char, ranges, ARRAY_LENGTH(ranges));
	if (err != error::SUCCESS)
		return err;

	for (i = 0; (ranges[i].min >= 0) && (ranges[i].max >= 0); i++)
	{
		if ((ranges[i].min <= val) && (ranges[i].max >= val))
			return error::SUCCESS;
	}
	return error::PARAMOUTOFRANGE;
}


// -------------------------------------------------
//	contains
// -------------------------------------------------

bool option_resolution::contains(const char *specification, int option_char)
{
	return strchr(specification, option_char) != nullptr;
}


// -------------------------------------------------
//	error_string
// -------------------------------------------------

const char *option_resolution::error_string(option_resolution::error err)
{
	switch (err)
	{
	case error::SUCCESS:				return "The operation completed successfully";
	case error::OUTOFMEMORY:			return "Out of memory";
	case error::PARAMOUTOFRANGE:		return "Parameter out of range";
	case error::PARAMNOTSPECIFIED:		return "Parameter not specified";
	case error::PARAMNOTFOUND:			return "Unknown parameter";
	case error::PARAMALREADYSPECIFIED:	return "Parameter specified multiple times";
	case error::BADPARAM:				return "Invalid parameter";
	case error::SYNTAX:					return "Syntax error";
	case error::INTERNAL:				return "Internal error";
	}
	return nullptr;
}

} // namespace util