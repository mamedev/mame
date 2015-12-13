// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    output.h

    General purpose output routines.
***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __OUTPUT_H__
#define __OUTPUT_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*output_notifier_func)(const char *outname, INT32 value, void *param);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core initialization */
void output_init(running_machine &machine);

/* set the value for a given output */
void output_set_value(const char *outname, INT32 value);

/* set an indexed value for an output (concatenates basename + index) */
void output_set_indexed_value(const char *basename, int index, int value);

/* return the current value for a given output */
INT32 output_get_value(const char *outname);

/* return the current value for a given indexed output */
INT32 output_get_indexed_value(const char *outname, int index);

/* set a notifier on a particular output, or globally if NULL */
void output_set_notifier(const char *outname, output_notifier_func callback, void *param);

/* set a notifier on a particular output, or globally if NULL */
void output_notify_all(output_notifier_func callback, void *param);

/* map a name to a unique ID */
UINT32 output_name_to_id(const char *outname);

/* map a unique ID back to a name */
const char *output_id_to_name(UINT32 id);



/***************************************************************************
    INLINES
***************************************************************************/

static inline void output_set_led_value(int index, int value)
{
	output_set_indexed_value("led", index, value ? 1 : 0);
}

static inline void output_set_lamp_value(int index, int value)
{
	output_set_indexed_value("lamp", index, value);
}

static inline void output_set_digit_value(int index, int value)
{
	output_set_indexed_value("digit", index, value);
}


static inline INT32 output_get_led_value(int index)
{
	return output_get_indexed_value("led", index);
}

static inline INT32 output_get_lamp_value(int index)
{
	return output_get_indexed_value("lamp", index);
}

static inline INT32 output_get_digit_value(int index)
{
	return output_get_indexed_value("digit", index);
}


#endif  /* __OUTPUT_H__ */
