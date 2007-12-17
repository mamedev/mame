/***************************************************************************

    inputseq.h

    Input sequence abstractions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __INPUTSEQ_H__
#define __INPUTSEQ_H__

#include "input.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* additional expanded input codes for sequences */
enum
{
	/* special codes */
	SEQCODE_END = INTERNAL_CODE(0),
	SEQCODE_DEFAULT = INTERNAL_CODE(1),
	SEQCODE_NOT = INTERNAL_CODE(2),
	SEQCODE_OR = INTERNAL_CODE(3)
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* a sequence of inputs */
typedef struct _input_seq input_seq;
struct _input_seq
{
	input_code code[16];
};



/***************************************************************************
    MACROS
***************************************************************************/

#define SEQ_DEF_7(a,b,c,d,e,f,g) {{ a, b, c, d, e, f, g, SEQCODE_END }}
#define SEQ_DEF_6(a,b,c,d,e,f)	{{ a, b, c, d, e, f, SEQCODE_END }}
#define SEQ_DEF_5(a,b,c,d,e)	{{ a, b, c, d, e, SEQCODE_END }}
#define SEQ_DEF_4(a,b,c,d)		{{ a, b, c, d, SEQCODE_END }}
#define SEQ_DEF_3(a,b,c)		{{ a, b, c, SEQCODE_END }}
#define SEQ_DEF_2(a,b)			{{ a, b, SEQCODE_END }}
#define SEQ_DEF_1(a)			{{ a, SEQCODE_END }}
#define SEQ_DEF_0				{{ SEQCODE_END }}



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- state queries ----- */

/* return TRUE if the given switch sequence has been pressed */
int input_seq_pressed(const input_seq *seq);

/* return the value of an axis sequence */
INT32 input_seq_axis_value(const input_seq *seq, input_item_class *itemclass_ptr);



/* ----- sequence polling ----- */

/* begin polling for a new sequence of the given itemclass */
void input_seq_poll_start(input_item_class itemclass, const input_seq *startseq);

/* continue polling for a sequence */
int input_seq_poll(input_seq *finalseq);



/* ----- strings and tokenization ----- */

/* generate the friendly name of an input sequence */
astring *input_seq_name(astring *string, const input_seq *seq);

/* convert an input sequence to tokens, returning the length */
astring *input_seq_to_tokens(astring *string, const input_seq *seq);

/* convert a set of tokens back to an input sequence */
int input_seq_from_tokens(const char *string, input_seq *seq);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    input_seq_get_1 - fetch the first item of an
    input sequence
-------------------------------------------------*/

INLINE input_code input_seq_get_1(const input_seq *seq)
{
	return seq->code[0];
}


/*-------------------------------------------------
    input_seq_set_n - set a sequence of n codes
    as an input sequence
-------------------------------------------------*/

INLINE void input_seq_set_5(input_seq *seq, input_code code0, input_code code1, input_code code2, input_code code3, input_code code4)
{
	int codenum;
	seq->code[0] = code0;
	seq->code[1] = code1;
	seq->code[2] = code2;
	seq->code[3] = code3;
	seq->code[4] = code4;
	for (codenum = 5; codenum < ARRAY_LENGTH(seq->code); codenum++)
		seq->code[codenum] = SEQCODE_END;
}

INLINE void input_seq_set_4(input_seq *seq, input_code code0, input_code code1, input_code code2, input_code code3)
{
	input_seq_set_5(seq, code0, code1, code2, code3, SEQCODE_END);
}

INLINE void input_seq_set_3(input_seq *seq, input_code code0, input_code code1, input_code code2)
{
	input_seq_set_5(seq, code0, code1, code2, SEQCODE_END, SEQCODE_END);
}

INLINE void input_seq_set_2(input_seq *seq, input_code code0, input_code code1)
{
	input_seq_set_5(seq, code0, code1, SEQCODE_END, SEQCODE_END, SEQCODE_END);
}

INLINE void input_seq_set_1(input_seq *seq, input_code code0)
{
	input_seq_set_5(seq, code0, SEQCODE_END, SEQCODE_END, SEQCODE_END, SEQCODE_END);
}

INLINE void input_seq_set_0(input_seq *seq)
{
	input_seq_set_5(seq, SEQCODE_END, SEQCODE_END, SEQCODE_END, SEQCODE_END, SEQCODE_END);
}


/*-------------------------------------------------
    input_seq_cmp - compare two input sequences
-------------------------------------------------*/

INLINE int input_seq_cmp(const input_seq *seqa, const input_seq *seqb)
{
	int codenum;
	for (codenum = 0; codenum < ARRAY_LENGTH(seqa->code); codenum++)
	{
		if (seqa->code[codenum] != seqb->code[codenum])
			return -1;
		if (seqa->code[codenum] == SEQCODE_END)
			break;
	}
	return 0;
}

#endif	/* __INPUTSEQ_H__ */
