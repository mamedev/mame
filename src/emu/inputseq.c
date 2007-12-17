/***************************************************************************

    inputseq.c

    Input sequence abstractions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "inputseq.h"
#include "restrack.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* max time between key presses */
#define RECORD_TIME					(osd_ticks_per_second() * 2 / 3)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* information about the current sequence being recorded */
static input_seq		record_seq;
static osd_ticks_t		record_last;
static input_item_class	record_class;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* internal functions */
static int input_seq_is_valid(const input_seq *seq);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    input_seq_length - return the length of the
    sequence
-------------------------------------------------*/

INLINE int input_seq_length(const input_seq *seq)
{
	int seqnum;

	/* find the end token; error if none found */
	for (seqnum = 0; seqnum < ARRAY_LENGTH(seq->code); seqnum++)
		if (seq->code[seqnum] == SEQCODE_END)
			return seqnum;
	return ARRAY_LENGTH(seq->code);
}


/*-------------------------------------------------
    input_seq_append - append a code to the end
    of an input sequence
-------------------------------------------------*/

INLINE int input_seq_append(input_seq *seq, input_code code)
{
	int length = input_seq_length(seq);

	/* if not enough room, return FALSE */
	if (length >= ARRAY_LENGTH(seq->code) - 1)
		return FALSE;

	/* otherwise, append the code and add a new end */
	seq->code[length++] = code;
	seq->code[length] = SEQCODE_END;
	return TRUE;
}


/*-------------------------------------------------
    input_seq_get_last - return the last code
    in a sequence
-------------------------------------------------*/

INLINE input_code input_seq_get_last(const input_seq *seq)
{
	int length = input_seq_length(seq);
	return (length == 0) ? SEQCODE_END : seq->code[length - 1];
}


/*-------------------------------------------------
    input_seq_backspace - "backspace" over the
    last entry in a sequence
-------------------------------------------------*/

INLINE void input_seq_backspace(input_seq *seq)
{
	int length = input_seq_length(seq);

	/* if we have at least one entry, remove it */
	if (length > 0)
		seq->code[length - 1] = SEQCODE_END;
}



/***************************************************************************
    STATE QUERIES
***************************************************************************/

/*-------------------------------------------------
    input_seq_pressed - return true if the given
    sequence of switch inputs is "pressed"
-------------------------------------------------*/

int input_seq_pressed(const input_seq *seq)
{
	int result = FALSE;
	int invert = FALSE;
	int first = TRUE;
	int codenum;

	/* iterate over all of the codes */
	for (codenum = 0; codenum < ARRAY_LENGTH(seq->code); codenum++)
	{
		input_code code = seq->code[codenum];

		/* handle NOT */
		if (code == SEQCODE_NOT)
			invert = TRUE;

		/* handle OR and END */
		else if (code == SEQCODE_OR || code == SEQCODE_END)
		{
			/* if we have a positive result from the previous set, we're done */
			if (result || code == SEQCODE_END)
				break;

			/* otherwise, reset our state */
			result = FALSE;
			invert = FALSE;
			first = TRUE;
		}

		/* handle everything else as a series of ANDs */
		else
		{
			/* if this is the first in the sequence, result is set equal */
			if (first)
				result = input_code_pressed(code) ^ invert;

			/* further values are ANDed */
			else if (result)
				result &= input_code_pressed(code) ^ invert;

			/* no longer first, and clear the invert flag */
			first = invert = FALSE;
		}
	}

	/* return the result if we queried at least one switch */
	return result;
}


/*-------------------------------------------------
    input_seq_axis_value - return the value of an
    axis defined in an input sequence
-------------------------------------------------*/

INT32 input_seq_axis_value(const input_seq *seq, input_item_class *itemclass_ptr)
{
	input_item_class itemclasszero = ITEM_CLASS_ABSOLUTE;
	input_item_class itemclass = ITEM_CLASS_INVALID;
	int result = 0;
	int invert = FALSE;
	int enable = TRUE;
	int codenum;

	/* iterate over all of the codes */
	for (codenum = 0; codenum < ARRAY_LENGTH(seq->code); codenum++)
	{
		input_code code = seq->code[codenum];

		/* handle NOT */
		if (code == SEQCODE_NOT)
			invert = TRUE;

		/* handle OR and END */
		else if (code == SEQCODE_OR || code == SEQCODE_END)
		{
			/* if we have a positive result from the previous set, we're done */
			if (itemclass != ITEM_CLASS_INVALID || code == SEQCODE_END)
				break;

			/* otherwise, reset our state */
			result = 0;
			invert = FALSE;
			enable = TRUE;
		}

		/* handle everything else only if we're still enabled */
		else if (enable)
		{
			/* switch codes serve as enables */
			if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_SWITCH)
			{
				/* AND against previous digital codes */
				if (enable)
					enable &= input_code_pressed(code) ^ invert;
			}

			/* non-switch codes are analog values */
			else
			{
				INT32 value = input_code_value(code);

				/* if we got a 0 value, don't do anything except remember the first type */
				if (value == 0)
				{
					if (itemclasszero == ITEM_CLASS_INVALID)
						itemclasszero = INPUT_CODE_ITEMCLASS(code);
				}

				/* non-zero absolute values stick */
				else if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_ABSOLUTE)
				{
					itemclass = ITEM_CLASS_ABSOLUTE;
					result = value;
				}

				/* non-zero relative values accumulate */
				else if (INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_RELATIVE)
				{
					itemclass = ITEM_CLASS_RELATIVE;
					result += value;
				}
			}

			/* clear the invert flag */
			invert = FALSE;
		}
	}

	/* if the caller wants to know the type, provide it */
	if (itemclass_ptr != NULL)
		*itemclass_ptr = (result == 0) ? itemclasszero : itemclass;
	return result;
}



/***************************************************************************
    SEQUENCE POLLING
***************************************************************************/

/*-------------------------------------------------
    input_seq_poll_start - begin polling for a
    new sequence of the given itemclass
-------------------------------------------------*/

void input_seq_poll_start(input_item_class itemclass, const input_seq *startseq)
{
	input_code dummycode;

	assert(itemclass == ITEM_CLASS_SWITCH || itemclass == ITEM_CLASS_ABSOLUTE || itemclass == ITEM_CLASS_RELATIVE);

	/* reset the recording count and the clock */
	record_last = 0;
	record_class = itemclass;

	/* grab the starting sequence to append to */
	if (startseq != NULL)
		record_seq = *startseq;
	else
		input_seq_set_0(&record_seq);

	/* append an OR if this is not a NULL sequence */
	if (input_seq_length(&record_seq) > 0)
		input_seq_append(&record_seq, SEQCODE_OR);

	/* flush out any goobers */
	dummycode = (record_class == ITEM_CLASS_SWITCH) ? input_code_poll_switches(TRUE) : input_code_poll_axes(TRUE);
	while (dummycode != INPUT_CODE_INVALID)
		dummycode = (record_class == ITEM_CLASS_SWITCH) ? input_code_poll_switches(FALSE) : input_code_poll_axes(FALSE);
}


/*-------------------------------------------------
    input_seq_poll - continue polling
-------------------------------------------------*/

int input_seq_poll(input_seq *finalseq)
{
	input_code lastcode = input_seq_get_last(&record_seq);
	input_code newcode;

	/* switch case: see if we have a new code to process */
	if (record_class == ITEM_CLASS_SWITCH)
	{
		newcode = input_code_poll_switches(FALSE);
		if (newcode != INPUT_CODE_INVALID)
		{
			/* if code is duplicate, toggle the NOT state on the code */
			if (input_seq_length(&record_seq) > 0 && newcode == lastcode)
			{
				/* back up over the code */
				input_seq_backspace(&record_seq);

				/* if there was a NOT preceding it, just delete it, otherwise append one */
				if (input_seq_get_last(&record_seq) == SEQCODE_NOT)
					input_seq_backspace(&record_seq);
				else
					input_seq_append(&record_seq, SEQCODE_NOT);
			}
		}
	}

	/* absolute/relative case: see if we have an analog change of sufficient amount */
	else
	{
		newcode = input_code_poll_axes(FALSE);

		/* if the last code doesn't match absolute/relative of this code, ignore the new one */
		if ((INPUT_CODE_ITEMCLASS(lastcode) == ITEM_CLASS_ABSOLUTE && INPUT_CODE_ITEMCLASS(newcode) != ITEM_CLASS_ABSOLUTE) ||
			(INPUT_CODE_ITEMCLASS(lastcode) == ITEM_CLASS_RELATIVE && INPUT_CODE_ITEMCLASS(newcode) != ITEM_CLASS_RELATIVE))
			newcode = INPUT_CODE_INVALID;

		if (newcode != INPUT_CODE_INVALID)
		{
			/* if code is duplicate and an absolute control, toggle to half axis */
			if (input_seq_length(&record_seq) > 0 && INPUT_CODE_ITEMCLASS(newcode) == ITEM_CLASS_ABSOLUTE)
				if (newcode == INPUT_CODE_SET_MODIFIER(lastcode, ITEM_MODIFIER_NONE))
				{
					/* increment the modifier, wrapping back to none */
					input_item_modifier oldmod = INPUT_CODE_MODIFIER(lastcode);
					input_item_modifier newmod = (oldmod < ITEM_MODIFIER_NEG) ? oldmod + 1 : ITEM_MODIFIER_NONE;
					newcode = INPUT_CODE_SET_MODIFIER(newcode, newmod);

					/* back up over the previous code so we can re-append */
					input_seq_backspace(&record_seq);
				}
		}
	}

	/* if we got a new code to append it, append it and reset the timer */
	if (newcode != INPUT_CODE_INVALID)
	{
		input_seq_append(&record_seq, newcode);
		record_last = osd_ticks();
	}

	/* if we're recorded at least one item and the RECORD_TIME has passed, we're done */
	if (record_last != 0 && osd_ticks() > record_last + RECORD_TIME)
	{
		/* if the final result is invalid, reset to nothing */
		if (!input_seq_is_valid(&record_seq))
			input_seq_set_0(&record_seq);

		/* return TRUE to indicate that we are finished */
		*finalseq = record_seq;
		return TRUE;
	}

	/* return FALSE to indicate we are still polling */
	return FALSE;
}



/***************************************************************************
    STRINGS AND TOKENIZATION
***************************************************************************/

/*-------------------------------------------------
    input_seq_name - generate the friendly name
    of a sequence
-------------------------------------------------*/

astring *input_seq_name(astring *string, const input_seq *seq)
{
	astring *codestr = astring_alloc();
	int codenum, copycodenum;
	input_seq seqcopy;

	/* walk the sequence first, removing any pieces that are invalid */
	for (codenum = copycodenum = 0; codenum < ARRAY_LENGTH(seq->code) && seq->code[codenum] != SEQCODE_END; codenum++)
	{
		input_code code = seq->code[codenum];

		/* if this is a code item which is not valid, don't copy it and remove any preceding ORs/NOTs */
		if (!INPUT_CODE_IS_INTERNAL(code) && astring_len(input_code_name(codestr, code)) == 0)
		{
			while (copycodenum > 0 && INPUT_CODE_IS_INTERNAL(seqcopy.code[copycodenum - 1]))
				copycodenum--;
		}
		else if (copycodenum > 0 || !INPUT_CODE_IS_INTERNAL(code))
			seqcopy.code[copycodenum++] = code;
	}
	seqcopy.code[copycodenum] = SEQCODE_END;

	/* special case: empty */
	if (copycodenum == 0)
	{
		astring_free(codestr);
		return astring_cpyc(string, (seq->code[0] == SEQCODE_END) ? "None" : "n/a");
	}

	/* start with an empty buffer */
	astring_reset(string);

	/* loop until we hit the end */
	for (codenum = 0; codenum < ARRAY_LENGTH(seqcopy.code) && seqcopy.code[codenum] != SEQCODE_END; codenum++)
	{
		input_code code = seqcopy.code[codenum];

		/* append a space if not the first code */
		if (codenum != 0)
			astring_catc(string, " ");

		/* handle OR/NOT codes here */
		if (code == SEQCODE_OR)
			astring_catc(string, "or");
		else if (code == SEQCODE_NOT)
			astring_catc(string, "not");

		/* otherwise, assume it is an input code and ask the input system to generate it */
		else
			astring_cat(string, input_code_name(codestr, code));
	}

	astring_free(codestr);
	return string;
}


/*-------------------------------------------------
    input_seq_to_tokens - generate the tokenized
    form of a sequence
-------------------------------------------------*/

astring *input_seq_to_tokens(astring *string, const input_seq *seq)
{
	astring *codestr = astring_alloc();
	int codenum;

	/* start with an empty buffer */
	astring_reset(string);

	/* loop until we hit the end */
	for (codenum = 0; codenum < ARRAY_LENGTH(seq->code) && seq->code[codenum] != SEQCODE_END; codenum++)
	{
		input_code code = seq->code[codenum];

		/* append a space if not the first code */
		if (codenum != 0)
			astring_catc(string, " ");

		/* handle OR/NOT codes here */
		if (code == SEQCODE_OR)
			astring_catc(string, "OR");
		else if (code == SEQCODE_NOT)
			astring_catc(string, "NOT");
		else if (code == SEQCODE_DEFAULT)
			astring_catc(string, "DEFAULT");

		/* otherwise, assume it is an input code and ask the input system to generate it */
		else
			astring_cat(string, input_code_to_token(codestr, code));
	}

	astring_free(codestr);
	return string;
}


/*-------------------------------------------------
    input_seq_from_tokens - generate the tokenized
    form of a sequence
-------------------------------------------------*/

int input_seq_from_tokens(const char *string, input_seq *seq)
{
	char *strcopy = malloc_or_die(strlen(string) + 1);
	char *str = strcopy;
	int result = FALSE;

	/* start with a blank sequence */
	input_seq_set_0(seq);

	/* loop until we're done */
	strcpy(strcopy, string);
	while (1)
	{
		input_code code;
		char origspace;
		char *strtemp;

		/* trim any leading spaces */
		while (*str != 0 && isspace(*str))
			str++;

		/* bail if we're done */
		if (*str == 0)
		{
			result = TRUE;
			break;
		}

		/* find the end of the token and make it upper-case along the way */
		for (strtemp = str; *strtemp != 0 && !isspace(*strtemp); strtemp++)
			*strtemp = toupper(*strtemp);
		origspace = *strtemp;
		*strtemp = 0;

		/* look for common stuff */
		if (strcmp(str, "OR") == 0)
			code = SEQCODE_OR;
		else if (strcmp(str, "NOT") == 0)
			code = SEQCODE_NOT;
		else if (strcmp(str, "DEFAULT") == 0)
			code = SEQCODE_DEFAULT;
		else
			code = input_code_from_token(str);

		/* translate and add to the sequence */
		input_seq_append(seq, code);

		/* advance */
		if (origspace == 0)
		{
			result = TRUE;
			break;
		}
		str = strtemp + 1;
	}

	free(strcopy);
	return result;
}



/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    input_seq_is_valid - return TRUE if a given
    sequence is valid
-------------------------------------------------*/

static int input_seq_is_valid(const input_seq *seq)
{
	input_item_class lastclass = ITEM_CLASS_INVALID;
	input_code lastcode = INPUT_CODE_INVALID;
	int positive_code_count = 0;
	int seqnum;

	/* scan the sequence for valid codes */
	for (seqnum = 0; seqnum < ARRAY_LENGTH(seq->code); seqnum++)
	{
		input_code code = seq->code[seqnum];

		/* invalid codes are invalid */
		if (code == INPUT_CODE_INVALID)
			return FALSE;

		/* if we hit an OR or the end, validate the previous chunk */
		if (code == SEQCODE_OR || code == SEQCODE_END)
		{
			/* must be at least one positive code */
			if (positive_code_count == 0)
				return FALSE;

			/* last code must not have been an internal code */
			if (INPUT_CODE_IS_INTERNAL(lastcode))
				return FALSE;

			/* if this is the end, we're ok */
			if (code == SEQCODE_END)
				return TRUE;

			/* reset the state for the next chunk */
			positive_code_count = 0;
			lastclass = ITEM_CLASS_INVALID;
		}

		/* if we hit a NOT, make sure we don't have a double */
		else if (code == SEQCODE_NOT)
		{
			if (lastcode == SEQCODE_NOT)
				return FALSE;
		}

		/* anything else */
		else
		{
			input_item_class itemclass = INPUT_CODE_ITEMCLASS(code);

			/* count positive codes */
			if (lastcode != SEQCODE_NOT)
				positive_code_count++;

			/* non-switch items can't have a NOT */
			if (itemclass != ITEM_CLASS_SWITCH && lastcode == SEQCODE_NOT)
				return FALSE;

			/* absolute/relative items must all be the same class */
			if ((lastclass == ITEM_CLASS_ABSOLUTE && itemclass != ITEM_CLASS_ABSOLUTE) ||
				(lastclass == ITEM_CLASS_RELATIVE && itemclass != ITEM_CLASS_RELATIVE))
				return FALSE;
		}

		/* remember the last code */
		lastcode = code;
	}

	/* if we got here, we were missing an END token; fail */
	return FALSE;
}
