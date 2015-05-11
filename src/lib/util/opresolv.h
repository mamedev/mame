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


/***************************************************************************

    Type definitions

***************************************************************************/

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


enum optreserr_t
{
	OPTIONRESOLUTION_ERROR_SUCCESS,
	OPTIONRESOLUTION_ERROR_OUTOFMEMORY,
	OPTIONRESOLUTION_ERROR_PARAMOUTOFRANGE,
	OPTIONRESOLUTION_ERROR_PARAMNOTSPECIFIED,
	OPTIONRESOLUTION_ERROR_PARAMNOTFOUND,
	OPTIONRESOLUTION_ERROR_PARAMALREADYSPECIFIED,
	OPTIONRESOLUTION_ERROR_BADPARAM,
	OPTIONRESOLUTION_ERROR_SYNTAX,
	OPTIONRESOLTUION_ERROR_INTERNAL
};



struct OptionResolutionError
{
	const option_guide *option;
	optreserr_t error;
};



struct option_resolution;

struct OptionRange
{
	int min, max;
};



/***************************************************************************

    Prototypes

***************************************************************************/

/* processing options with option_resolution objects */
option_resolution *option_resolution_create(const option_guide *guide, const char *specification);
optreserr_t option_resolution_add_param(option_resolution *resolution, const char *param, const char *value);
optreserr_t option_resolution_finish(option_resolution *resolution);
void option_resolution_close(option_resolution *resolution);
int option_resolution_lookup_int(option_resolution *resolution, int option_char);
const char *option_resolution_lookup_string(option_resolution *resolution, int option_char);

/* option resolution accessors */
const char *option_resolution_specification(option_resolution *resolution);
const option_guide *option_resolution_find_option(option_resolution *resolution, int option_char);
const option_guide *option_resolution_index_option(option_resolution *resolution, int indx);

/* processing option guides */
int option_resolution_countoptions(const option_guide *guide, const char *specification);

/* processing option specifications */
optreserr_t option_resolution_listranges(const char *specification, int option_char,
	struct OptionRange *range, size_t range_count);
optreserr_t option_resolution_getdefault(const char *specification, int option_char, int *val);
optreserr_t option_resolution_isvalidvalue(const char *specification, int option_char, int val);
int option_resolution_contains(const char *specification, int option_char);

/* misc */
const char *option_resolution_error_string(optreserr_t err);

#endif /* __OPRESOLV_H__ */
