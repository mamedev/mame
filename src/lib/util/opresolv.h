// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    opresolv.h

    Extensible ranged option resolution handling

    An extensible mechanism for handling options is a major need in MESS and
    Imgtool.  Unfortunately, since we are using straight C for everything, it
    can be hard and awkward to create non-arcane mechanisms for representing
    these options.

    In this system, we have the following concepts:
    1.  An "option specification"; a string that represents what options are
        available, their defaults, and their allowed ranges.  Here is an
        example:

        Examples:
            "H[1]-2;T[35]/40/80;S[18]"
                Allow 1-2 heads; 35, 40 or 80 tracks, and 18 sectors,
                defaulting to 1 heads and 35 tracks.

            "N'Simon''s desk'"
                Simon's desk (strings are not subject to range checking)

    2.  An "option guide"; a struct that provides information about what the
        various members of the option specification mean (i.e. - H=heads)

    3.  An "option resolution"; an object that represents a set of interpreted
        options.  At this stage, the option bid has been processed and it is
        guaranteed that all options reside in their expected ranges.

    An option_resolution object is created based on an option guide and an
    option specification.  It is then possible to specify individual parameters
    to the option_resolution object.  Argument checks occur at this time.  When
    one is all done, you can then query the object for any given value.

****************************************************************************/

#ifndef __OPRESOLV_H__
#define __OPRESOLV_H__

#include <stdlib.h>
#include <vector>
#include <string>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum option_type
{
	OPTIONTYPE_END,
	OPTIONTYPE_INT,
	OPTIONTYPE_STRING,
	OPTIONTYPE_ENUM_BEGIN,
	OPTIONTYPE_ENUM_VALUE
};

struct option_guide
{
	enum option_type option_type;
	int parameter;
	const char *identifier;
	const char *display_name;
};

#define OPTION_GUIDE_START(option_guide_)                                   \
	const option_guide option_guide_[] =                                \
	{
#define OPTION_GUIDE_END                                                    \
		{ OPTIONTYPE_END }                                                  \
	};
#define OPTION_GUIDE_EXTERN(option_guide_)                                  \
	extern const option_guide option_guide_[]
#define OPTION_INT(option_char, identifier, display_name)                   \
		{ OPTIONTYPE_INT, (option_char), (identifier), (display_name) },
#define OPTION_STRING(option_char, identifier, display_name)                \
	{ OPTIONTYPE_STRING, (option_char), (identifier), (display_name) },
#define OPTION_ENUM_START(option_char, identifier, display_name)            \
	{ OPTIONTYPE_ENUM_BEGIN, (option_char), (identifier), (display_name) },
#define OPTION_ENUM(value, identifier, display_name)                        \
	{ OPTIONTYPE_ENUM_VALUE, (value), (identifier), (display_name) },
#define OPTION_ENUM_END

namespace util {
class option_resolution
{
public:
	enum class error
	{
		SUCCESS,
		OUTOFMEMORY,
		PARAMOUTOFRANGE,
		PARAMNOTSPECIFIED,
		PARAMNOTFOUND,
		PARAMALREADYSPECIFIED,
		BADPARAM,
		SYNTAX,
		INTERNAL
	};

	struct range
	{
		int min, max;
	};

	option_resolution(const option_guide *guide, const char *specification);
	~option_resolution();

	// processing options with option_resolution objects
	error add_param(const char *param, const std::string &value);
	error finish();
	int lookup_int(int option_char) const;
	const char *lookup_string(int option_char) const;

	// accessors
	const char *specification() const { return m_specification; }
	const option_guide *find_option(int option_char) const;
	const option_guide *index_option(int indx) const;

	// processing option guides
	static size_t count_options(const option_guide *guide, const char *specification);

	// processing option specifications
	static error list_ranges(const char *specification, int option_char,
		range *range, size_t range_count);
	static error get_default(const char *specification, int option_char, int *val);
	static error is_valid_value(const char *specification, int option_char, int val);
	static bool contains(const char *specification, int option_char);

	// misc
	static const char *error_string(error err);

private:
	enum class entry_state
	{
		UNSPECIFIED,
		SPECIFIED
	};

	struct entry
	{
		const option_guide *guide_entry;
		entry_state state;
		std::string value;

		int int_value() const;
		void set_int_value(int i);
	};

	const char *m_specification;
	std::vector<entry> m_entries;

	static error resolve_single_param(const char *specification, entry *param_value,
		struct range *range, size_t range_count);
	static const char *lookup_in_specification(const char *specification, const option_guide *option);
	const entry *lookup_entry(int option_char) const;
};


} // namespace util

#endif /* __OPRESOLV_H__ */
