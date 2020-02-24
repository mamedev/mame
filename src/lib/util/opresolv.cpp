// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    opresolv.cpp

    Extensible ranged option resolution handling

****************************************************************************/

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "pool.h"
#include "corestr.h"
#include "opresolv.h"


namespace util {

/***************************************************************************
    option_resolution
***************************************************************************/

// -------------------------------------------------
//  ctor
// -------------------------------------------------

option_resolution::option_resolution(const option_guide &guide)
{
	// reserve space for entries
	m_entries.reserve(guide.entries().size());

	// initialize each of the entries; can't use foreach because we need to scan the
	// ENUM_VALUE entries
	for (auto iter = guide.entries().begin(); iter != guide.entries().end(); iter++)
	{
		// create the entry
		m_entries.emplace_back(*iter);
		entry &entry(m_entries.back());

		// if this is an enumeration, identify the values
		if (iter->type() == option_guide::entry::option_type::ENUM_BEGIN)
		{
			// enum values begin after the ENUM_BEGIN
			auto enum_value_begin = iter + 1;
			auto enum_value_end = enum_value_begin;

			// and identify all entries of type ENUM_VALUE
			while (enum_value_end != guide.entries().end() && enum_value_end->type() == option_guide::entry::option_type::ENUM_VALUE)
			{
				iter++;
				enum_value_end++;
			}

			// set the range
			entry.set_enum_value_range(enum_value_begin, enum_value_end);
		}
	}
}


// -------------------------------------------------
//  dtor
// -------------------------------------------------

option_resolution::~option_resolution()
{
}


// -------------------------------------------------
//  lookup_in_specification
// -------------------------------------------------

const char *option_resolution::lookup_in_specification(const char *specification, const option_guide::entry &option)
{
	const char *s;
	s = strchr(specification, option.parameter());
	return s ? s + 1 : nullptr;
}


// -------------------------------------------------
//  set_specification - sets the option specification
//  and mutates values accordingly
// -------------------------------------------------

void option_resolution::set_specification(const std::string &specification)
{
	for (auto &entry : m_entries)
	{
		// find this entry's info in the specification
		auto entry_specification = lookup_in_specification(specification.c_str(), entry.m_guide_entry);

		// parse this entry's specification (e.g. - set up ranges and defaults)
		entry.parse_specification(entry_specification);

		// is this a range typed that needs to be defaulted for the first time?
		if (entry.is_pertinent() && entry.is_ranged() && entry.value().empty())
		{
			entry.set_value(entry.default_value());
		}
	}
}


// -------------------------------------------------
//  find
// -------------------------------------------------

option_resolution::entry *option_resolution::find(int parameter)
{
	auto iter = std::find_if(
		m_entries.begin(),
		m_entries.end(),
		[parameter](const entry &e) { return e.parameter() == parameter; });

	return iter != m_entries.end()
		? &*iter
		: nullptr;
}


// -------------------------------------------------
//  find
// -------------------------------------------------

option_resolution::entry *option_resolution::find(const std::string &identifier)
{
	auto iter = std::find_if(
		m_entries.begin(),
		m_entries.end(),
		[&](const entry &e) { return !strcmp(e.identifier(), identifier.c_str()); });

	return iter != m_entries.end()
		? &*iter
		: nullptr;
}


// -------------------------------------------------
//  lookup_int
// -------------------------------------------------

int option_resolution::lookup_int(int parameter)
{
	auto entry = find(parameter);
	assert(entry != nullptr);
	return entry->value_int();
}


// -------------------------------------------------
//  lookup_string
// -------------------------------------------------

const std::string &option_resolution::lookup_string(int parameter)
{
	auto entry = find(parameter);
	assert(entry != nullptr);
	return entry->value();
}


// -------------------------------------------------
//  error_string
// -------------------------------------------------

option_resolution::error option_resolution::get_default(const char *specification, int option_char, int *val)
{
	// NYI
	return error::INTERNAL;
}


// -------------------------------------------------
//  error_string
// -------------------------------------------------

const char *option_resolution::error_string(option_resolution::error err)
{
	switch (err)
	{
	case error::SUCCESS:                return "The operation completed successfully";
	case error::OUTOFMEMORY:            return "Out of memory";
	case error::PARAMOUTOFRANGE:        return "Parameter out of range";
	case error::PARAMNOTSPECIFIED:      return "Parameter not specified";
	case error::PARAMNOTFOUND:          return "Unknown parameter";
	case error::PARAMALREADYSPECIFIED:  return "Parameter specified multiple times";
	case error::BADPARAM:               return "Invalid parameter";
	case error::SYNTAX:                 return "Syntax error";
	case error::INTERNAL:               return "Internal error";
	}
	return nullptr;
}


// -------------------------------------------------
//  entry::ctor
// -------------------------------------------------

option_resolution::entry::entry(const option_guide::entry &guide_entry)
	: m_guide_entry(guide_entry), m_is_pertinent(false)
{
}


// -------------------------------------------------
//  entry::set_enum_value_range
// -------------------------------------------------

void option_resolution::entry::set_enum_value_range(const option_guide::entry *begin, const option_guide::entry *end)
{
	m_enum_value_begin = begin;
	m_enum_value_end = end;
}


// -------------------------------------------------
//  entry::parse_specification
// -------------------------------------------------

void option_resolution::entry::parse_specification(const char *specification)
{
	// clear some items out
	m_ranges.clear();
	m_default_value.clear();

	// is this even pertinent?
	m_is_pertinent = (specification != nullptr) && (specification[0] != '\0');
	if (m_is_pertinent)
	{
		int value = 0;
		bool in_range = false;
		bool in_default = false;
		bool default_specified = false;
		bool half_range = false;
		size_t pos = 0;

		m_ranges.emplace_back();

		while (specification[pos] && !isalpha(specification[pos]))
		{
			if (specification[pos] == '-')
			{
				// range specifier
				assert(!in_range && !in_default && "Syntax error in specification");

				in_range = true;
				pos++;

				m_ranges.back().max = -1;
				if (!half_range)
				{
					m_ranges.back().min = -1;
					half_range = true;
				}
			}
			else if (specification[pos] == '[')
			{
				// begin default value
				assert(!in_default && !default_specified && "Syntax error in specification");

				in_default = true;
				pos++;
			}
			else if (specification[pos] == ']')
			{
				// end default value
				assert(in_default && "Syntax error in specification");

				in_default = false;
				default_specified = true;
				pos++;

				m_default_value = numeric_value(value);
			}
			else if (specification[pos] == '/')
			{
				// value separator
				assert(!in_default && !in_range && "Syntax error in specification");
				pos++;

				// if we are spitting out ranges, complete the range
				if (half_range)
				{
					m_ranges.emplace_back();
					half_range = false;
				}
			}
			else if (specification[pos] == ';')
			{
				// basic separator
				pos++;
			}
			else if (isdigit(specification[pos]))
			{
				// numeric value */
				value = 0;
				do
				{
					value *= 10;
					value += specification[pos++] - '0';
				} while (isdigit(specification[pos]));

				if (!half_range)
				{
					m_ranges.back().min = value;
					half_range = true;
				}
				m_ranges.back().max = value;
				in_range = false;
			}
			else
			{
				// invalid character - abort because we cannot recover from this syntax error
				throw false;
			}
		}

		// we can't have zero length guidelines strings
		assert(pos > 0);

		// appease compiler for scenarios where assert() has been preprocessed out
		(void)(in_range && in_default && default_specified);
	}
}


// -------------------------------------------------
//  entry::numeric_value
// -------------------------------------------------

std::string option_resolution::entry::numeric_value(int value)
{
	return string_format("%d", value);
}


// -------------------------------------------------
//  entry::value
// -------------------------------------------------

const std::string &option_resolution::entry::value() const
{
	static std::string empty_string;
	return is_pertinent() ? m_value : empty_string;
}


// -------------------------------------------------
//  entry::value_int
// -------------------------------------------------

int option_resolution::entry::value_int() const
{
	return is_pertinent() ? atoi(m_value.c_str()) : -1;
}


// -------------------------------------------------
//  entry::set_value
// -------------------------------------------------

bool option_resolution::entry::set_value(const std::string &value)
{
	// reject the value if this isn't pertinent
	if (!is_pertinent())
		return false;

	// if this is ranged, check against the ranges
	if (is_ranged() && find_in_ranges(std::atoi(value.c_str())) == m_ranges.cend())
		return false;

	// looks good!  change the value
	m_value = value;
	return true;
}


// -------------------------------------------------
//  entry::can_bump_lower
// -------------------------------------------------

bool option_resolution::entry::can_bump_lower() const
{
	return !m_ranges.empty()
		&& value_int() > m_ranges.front().min;
}


// -------------------------------------------------
//  entry::can_bump_higher
// -------------------------------------------------

bool option_resolution::entry::can_bump_higher() const
{
	return !m_ranges.empty()
		&& value_int() < m_ranges.back().max;
}


// -------------------------------------------------
//  entry::bump_lower
// -------------------------------------------------

bool option_resolution::entry::bump_lower()
{
	auto old_value = value_int();
	auto current_range = find_in_ranges(old_value);
	assert(current_range != m_ranges.end());

	int new_value;
	if (old_value > current_range->min)
	{
		// decrement within current range
		new_value = old_value - 1;
	}
	else if (current_range != m_ranges.begin())
	{
		// go to the top of the previous range
		new_value = (current_range - 1)->max;
	}
	else
	{
		// at the minimum; don't bump
		new_value = old_value;
	}

	set_value(new_value);
	return new_value != old_value;
}


// -------------------------------------------------
//  entry::bump_higher
// -------------------------------------------------

bool option_resolution::entry::bump_higher()
{
	auto old_value = value_int();
	auto current_range = find_in_ranges(old_value);
	assert(current_range != m_ranges.end());

	int new_value;
	if (old_value < current_range->max)
	{
		// increment within current range
		new_value = old_value + 1;
	}
	else if (current_range != (m_ranges.end() - 1))
	{
		// go to the bottom of the next range
		new_value = (current_range + 1)->min;
	}
	else
	{
		// at the minimum; don't bump
		new_value = old_value;
	}

	set_value(new_value);
	return new_value != old_value;
}


// -------------------------------------------------
//  entry::find_in_ranges
// -------------------------------------------------

option_resolution::entry::rangelist::const_iterator option_resolution::entry::find_in_ranges(int value) const
{
	return std::find_if(
		m_ranges.begin(),
		m_ranges.end(),
		[&](const auto &r) { return r.min <= value && value <= r.max; });
}


} // namespace util
