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

enum resolution_entry_state
{
	RESOLUTION_ENTRY_STATE_UNSPECIFIED,
	RESOLUTION_ENTRY_STATE_SPECIFIED
};

struct option_resolution_entry
{
	const option_guide *guide_entry;
	enum resolution_entry_state state;
	union
	{
		int int_value;
		const char *str_value;
	} u;
};

struct option_resolution
{
	object_pool *pool;
	const char *specification;
	size_t option_count;
	struct option_resolution_entry *entries;
};

static optreserr_t resolve_single_param(const char *specification, int *param_value,
	struct OptionRange *range, size_t range_count)
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
				return OPTIONRESOLUTION_ERROR_SYNTAX;
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
				return OPTIONRESOLUTION_ERROR_SYNTAX;
			}
			flags |= FLAG_IN_DEFAULT;
			s++;
		}
		else if (*s == ']')
		{
			/* end default value */
			if ((flags & FLAG_IN_DEFAULT) == 0)
			{
				return OPTIONRESOLUTION_ERROR_SYNTAX;
			}
			flags &= ~FLAG_IN_DEFAULT;
			flags |= FLAG_DEFAULT_SPECIFIED;
			s++;

			if (param_value && *param_value == -1)
				*param_value = value;
		}
		else if (*s == '/')
		{
			/* value separator */
			if (flags & (FLAG_IN_DEFAULT|FLAG_IN_RANGE))
			{
				return OPTIONRESOLUTION_ERROR_SYNTAX;
			}
			s++;

			/* if we are spitting out ranges, complete the range */
			if (range && (flags & FLAG_HALF_RANGE))
			{
				range++;
				flags &= ~FLAG_HALF_RANGE;
				if (--range_count == 0)
					range = NULL;
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

			/* if we have a value; check to see if it is out of range */
			if (param_value && (*param_value != -1) && (*param_value != value))
			{
				if ((last_value < *param_value) && (*param_value < value))
				{
					if ((flags & FLAG_IN_RANGE) == 0)
						return OPTIONRESOLUTION_ERROR_PARAMOUTOFRANGE;
				}
			}
			flags &= ~FLAG_IN_RANGE;
		}
		else
		{
			return OPTIONRESOLUTION_ERROR_SYNTAX;
		}
	}

	/* we can't have zero length guidelines strings */
	if (s == specification)
	{
		return OPTIONRESOLUTION_ERROR_SYNTAX;
	}

	return OPTIONRESOLUTION_ERROR_SUCCESS;
}



static const char *lookup_in_specification(const char *specification, const option_guide *option)
{
	const char *s;
	s = strchr(specification, option->parameter);
	return s ? s + 1 : NULL;
}



option_resolution *option_resolution_create(const option_guide *guide, const char *specification)
{
	option_resolution *resolution = NULL;
	const option_guide *guide_entry;
	int option_count;
	int opt = -1;
	object_pool *pool;

	assert(guide);

	/* first count the number of options specified in the guide */
	option_count = option_resolution_countoptions(guide, specification);

	/* create a memory pool for this structure */
	pool = pool_alloc_lib(NULL);
	if (!pool)
		goto outofmemory;

	/* allocate the main structure */
	resolution = (option_resolution *)pool_malloc_lib(pool, sizeof(option_resolution));
	if (!resolution)
		goto outofmemory;
	memset(resolution, 0, sizeof(*resolution));
	resolution->pool = pool;

	/* set up the entries list */
	resolution->option_count = option_count;
	resolution->specification = specification;
	resolution->entries = (option_resolution_entry *)pool_malloc_lib(resolution->pool, sizeof(struct option_resolution_entry) * option_count);
	if (!resolution->entries)
		goto outofmemory;
	memset(resolution->entries, 0, sizeof(struct option_resolution_entry) * option_count);

	/* initialize each of the entries */
	opt = 0;
	guide_entry = guide;
	while(guide_entry->option_type != OPTIONTYPE_END)
	{
		switch(guide_entry->option_type) {
		case OPTIONTYPE_INT:
		case OPTIONTYPE_ENUM_BEGIN:
		case OPTIONTYPE_STRING:
			if (lookup_in_specification(specification, guide_entry))
				resolution->entries[opt++].guide_entry = guide_entry;
			break;
		case OPTIONTYPE_ENUM_VALUE:
			break;
		default:
			goto unexpected;
		}
		guide_entry++;
	}
	assert(opt == option_count);
	return resolution;

unexpected:
	assert(FALSE);
outofmemory:
	if (resolution)
		option_resolution_close(resolution);
	return NULL;
}



optreserr_t option_resolution_add_param(option_resolution *resolution, const char *param, const char *value)
{
	int i;
	int must_resolve;
	optreserr_t err;
	const char *option_specification;
	struct option_resolution_entry *entry = NULL;

	for (i = 0; i < resolution->option_count; i++)
	{
		if (!strcmp(param, resolution->entries[i].guide_entry->identifier))
		{
			entry = &resolution->entries[i];
			break;
		}
	}
	if (!entry)
		return OPTIONRESOLUTION_ERROR_PARAMNOTFOUND;

	if (entry->state != RESOLUTION_ENTRY_STATE_UNSPECIFIED)
		return OPTIONRESOLUTION_ERROR_PARAMALREADYSPECIFIED;

	switch(entry->guide_entry->option_type) {
	case OPTIONTYPE_INT:
		entry->u.int_value = atoi(value);
		entry->state = RESOLUTION_ENTRY_STATE_SPECIFIED;
		must_resolve = TRUE;
		break;

	case OPTIONTYPE_STRING:
		entry->u.str_value = pool_strdup_lib(resolution->pool, value);
		if (!entry->u.str_value)
		{
			err = OPTIONRESOLUTION_ERROR_OUTOFMEMORY;
			goto done;
		}
		entry->state = RESOLUTION_ENTRY_STATE_SPECIFIED;
		must_resolve = FALSE;
		break;

	case OPTIONTYPE_ENUM_BEGIN:
		for (i = 1; entry->guide_entry[i].option_type == OPTIONTYPE_ENUM_VALUE; i++)
		{
			if (!core_stricmp(value, entry->guide_entry[i].identifier))
			{
				entry->u.int_value = entry->guide_entry[i].parameter;
				entry->state = RESOLUTION_ENTRY_STATE_SPECIFIED;
				break;
			}
		}
		if (entry->state != RESOLUTION_ENTRY_STATE_SPECIFIED)
		{
			err = OPTIONRESOLUTION_ERROR_BADPARAM;
			goto done;
		}
		must_resolve = TRUE;
		break;

	default:
		err = OPTIONRESOLTUION_ERROR_INTERNAL;
		assert(0);
		goto done;
	}

	/* do a resolution step if necessary */
	if (must_resolve)
	{
		option_specification = lookup_in_specification(resolution->specification, entry->guide_entry);
		err = resolve_single_param(option_specification, &entry->u.int_value, NULL, 0);
		if (err)
			goto done;

		/* did we not get a real value? */
		if (entry->u.int_value < 0)
		{
			err = OPTIONRESOLUTION_ERROR_PARAMNOTSPECIFIED;
			goto done;
		}
	}

	err = OPTIONRESOLUTION_ERROR_SUCCESS;

done:
	return err;
}



void option_resolution_close(option_resolution *resolution)
{
	pool_free_lib(resolution->pool);
}



optreserr_t option_resolution_finish(option_resolution *resolution)
{
	int i;
	optreserr_t err;
	struct option_resolution_entry *entry;
	const char *option_specification;

	for (i = 0; i < resolution->option_count; i++)
	{
		entry = &resolution->entries[i];

		if (entry->state == RESOLUTION_ENTRY_STATE_UNSPECIFIED)
		{
			switch(entry->guide_entry->option_type) {
			case OPTIONTYPE_INT:
			case OPTIONTYPE_ENUM_BEGIN:
				option_specification = lookup_in_specification(resolution->specification, entry->guide_entry);
				assert(option_specification);
				entry->u.int_value = -1;
				err = resolve_single_param(option_specification, &entry->u.int_value, NULL, 0);
				if (err)
					return err;
				break;

			case OPTIONTYPE_STRING:
				entry->u.str_value = "";
				break;

			default:
				assert(FALSE);
				return OPTIONRESOLTUION_ERROR_INTERNAL;
			}
			entry->state = RESOLUTION_ENTRY_STATE_SPECIFIED;
		}
	}
	return OPTIONRESOLUTION_ERROR_SUCCESS;
}



static const struct option_resolution_entry *option_resolution_lookup_entry(option_resolution *resolution, int option_char)
{
	size_t i;
	const struct option_resolution_entry *entry;

	for (i = 0; i < resolution->option_count; i++)
	{
		entry = &resolution->entries[i];

		switch(entry->guide_entry->option_type) {
		case OPTIONTYPE_INT:
		case OPTIONTYPE_STRING:
		case OPTIONTYPE_ENUM_BEGIN:
			if (entry->guide_entry->parameter == option_char)
				return entry;
			break;

		default:
			assert(FALSE);
			return NULL;
		}
	}
	return NULL;
}



int option_resolution_lookup_int(option_resolution *resolution, int option_char)
{
	const struct option_resolution_entry *entry;
	entry = option_resolution_lookup_entry(resolution, option_char);
	return entry ? entry->u.int_value : -1;
}



const char *option_resolution_lookup_string(option_resolution *resolution, int option_char)
{
	const struct option_resolution_entry *entry;
	entry = option_resolution_lookup_entry(resolution, option_char);
	return entry ? entry->u.str_value : NULL;
}



const char *option_resolution_specification(option_resolution *resolution)
{
	return resolution->specification;
}



const option_guide *option_resolution_find_option(option_resolution *resolution, int option_char)
{
	const struct option_resolution_entry *entry;
	entry = option_resolution_lookup_entry(resolution, option_char);
	return entry ? entry->guide_entry : NULL;
}



const option_guide *option_resolution_index_option(option_resolution *resolution, int indx)
{
	if ((indx < 0) || (indx >= resolution->option_count))
		return NULL;
	return resolution->entries[indx].guide_entry;
}



int option_resolution_countoptions(const option_guide *guide, const char *specification)
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



optreserr_t option_resolution_listranges(const char *specification, int option_char,
	struct OptionRange *range, size_t range_count)
{
	assert(range_count > 0);

	/* clear out range */
	memset(range, -1, sizeof(*range) * range_count);
	range_count--;

	specification = strchr(specification, option_char);
	if (!specification)
	{
		return OPTIONRESOLUTION_ERROR_SYNTAX;
	}

	return resolve_single_param(specification + 1, NULL, range, range_count);
}



optreserr_t option_resolution_getdefault(const char *specification, int option_char, int *val)
{
	assert(val);

	/* clear out default */
	*val = -1;

	specification = strchr(specification, option_char);
	if (!specification)
	{
		return OPTIONRESOLUTION_ERROR_SYNTAX;
	}

	return resolve_single_param(specification + 1, val, NULL, 0);
}



optreserr_t option_resolution_isvalidvalue(const char *specification, int option_char, int val)
{
	optreserr_t err;
	struct OptionRange ranges[256];
	int i;

	err = option_resolution_listranges(specification, option_char, ranges, ARRAY_LENGTH(ranges));
	if (err)
		return err;

	for (i = 0; (ranges[i].min >= 0) && (ranges[i].max >= 0); i++)
	{
		if ((ranges[i].min <= val) && (ranges[i].max >= val))
			return OPTIONRESOLUTION_ERROR_SUCCESS;
	}
	return OPTIONRESOLUTION_ERROR_PARAMOUTOFRANGE;
}

/**
 * @fn  int option_resolution_contains(const char *specification, int option_char)
 *
 * @brief   Option resolution contains.
 *
 * @param   specification   The specification.
 * @param   option_char     The option character.
 *
 * @return  An int.
 */

int option_resolution_contains(const char *specification, int option_char)
{
	return strchr(specification, option_char) != NULL;
}

/**
 * @fn  const char *option_resolution_error_string(optreserr_t err)
 *
 * @brief   Option resolution error string.
 *
 * @param   err The error.
 *
 * @return  null if it fails, else a char*.
 */

const char *option_resolution_error_string(optreserr_t err)
{
	static const char *const errors[] =
	{
		"The operation completed successfully",     /* OPTIONRESOLUTION_ERROR_SUCCESS */
		"Out of memory",                            /* OPTIONRESOLUTION_ERROR_OUTOFMEMORY */
		"Parameter out of range",                   /* OPTIONRESOLUTION_ERROR_PARAMOUTOFRANGE */
		"Parameter not specified",                  /* OPTIONRESOLUTION_ERROR_PARAMNOTSPECIFIED */
		"Unknown parameter",                        /* OPTIONRESOLUTION_ERROR_PARAMNOTFOUND */
		"Parameter specified multiple times",       /* OPTIONRESOLUTION_ERROR_PARAMALREADYSPECIFIED */
		"Invalid parameter",                        /* OPTIONRESOLUTION_ERROR_BADPARAM */
		"Syntax error",                             /* OPTIONRESOLUTION_ERROR_SYNTAX */
		"Internal error"                            /* OPTIONRESOLTUION_ERROR_INTERNAL */
	};

	if ((err < 0) || (err >= ARRAY_LENGTH(errors)))
		return NULL;
	return errors[err];
}
