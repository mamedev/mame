/*********************************************************************

    generic.h

    Generic simple machine functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __MACHINE_GENERIC_H__
#define __MACHINE_GENERIC_H__

#include "mamecore.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* total # of coin counters */
#define COIN_COUNTERS			8

/* memory card actions */
#define MEMCARD_CREATE			0
#define MEMCARD_INSERT			1
#define MEMCARD_EJECT			2



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern UINT32 dispensed_tickets;
extern UINT32 coin_count[COIN_COUNTERS];
extern UINT32 coinlockedout[COIN_COUNTERS];
extern UINT32 servicecoinlockedout[COIN_COUNTERS];

extern size_t generic_nvram_size;
extern UINT8 *generic_nvram;
extern UINT16 *generic_nvram16;
extern UINT32 *generic_nvram32;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- initialization ----- */

/* set up all the common systems */
void generic_machine_init(running_machine *machine);



/* ----- coin counters ----- */

/* write to a particular coin counter (clocks on active high edge) */
void coin_counter_w(int num, int on);

/* enable/disable coin lockout for a particular coin */
void coin_lockout_w(int num, int on);

/* enable/disable coin lockout for a particular coin */
void service_coin_lockout_w(int num, int on);

/* enable/disable global coin lockout */
void coin_lockout_global_w(int on);



/* ----- NVRAM management ----- */

/* open an NVRAM file directly */
mame_file *nvram_fopen(running_machine *machine, UINT32 openflags);

/* load NVRAM from a file */
void nvram_load(void);

/* save NVRAM to a file */
void nvram_save(void);

/* generic NVRAM handler that defaults to a 0 fill */
NVRAM_HANDLER( generic_0fill );

/* generic NVRAM handler that defaults to a 1 fill */
NVRAM_HANDLER( generic_1fill );

/* generic NVRAM handler that defaults to a random fill */
NVRAM_HANDLER( generic_randfill );



/* ----- memory card management ----- */

/* create a new memory card with the given index */
int memcard_create(int index, int overwrite);

/* "insert" a memory card with the given index and load its data */
int memcard_insert(int index);

/* "eject" a memory card and save its data */
void memcard_eject(running_machine *machine);

/* returns the index of the current memory card, or -1 if none */
int memcard_present(void);



/* ----- miscellaneous bits & pieces ----- */

/* set the status of an LED */
void set_led_status(int num, int value);



/* ----- interrupt enable and vector helpers ----- */

void cpu_interrupt_enable(int cpu, int enabled);
READ8_HANDLER( interrupt_enable_r );
WRITE8_HANDLER( interrupt_enable_w );



/* ----- common interrupt callbacks ----- */

INTERRUPT_GEN( nmi_line_pulse );
INTERRUPT_GEN( nmi_line_assert );

INTERRUPT_GEN( irq0_line_hold );
INTERRUPT_GEN( irq0_line_pulse );
INTERRUPT_GEN( irq0_line_assert );

INTERRUPT_GEN( irq1_line_hold );
INTERRUPT_GEN( irq1_line_pulse );
INTERRUPT_GEN( irq1_line_assert );

INTERRUPT_GEN( irq2_line_hold );
INTERRUPT_GEN( irq2_line_pulse );
INTERRUPT_GEN( irq2_line_assert );

INTERRUPT_GEN( irq3_line_hold );
INTERRUPT_GEN( irq3_line_pulse );
INTERRUPT_GEN( irq3_line_assert );

INTERRUPT_GEN( irq4_line_hold );
INTERRUPT_GEN( irq4_line_pulse );
INTERRUPT_GEN( irq4_line_assert );

INTERRUPT_GEN( irq5_line_hold );
INTERRUPT_GEN( irq5_line_pulse );
INTERRUPT_GEN( irq5_line_assert );

INTERRUPT_GEN( irq6_line_hold );
INTERRUPT_GEN( irq6_line_pulse );
INTERRUPT_GEN( irq6_line_assert );

INTERRUPT_GEN( irq7_line_hold );
INTERRUPT_GEN( irq7_line_pulse );
INTERRUPT_GEN( irq7_line_assert );



/* ----- generic watchdog reset handlers ----- */

/* 8-bit watchdog read/write handlers */
WRITE8_HANDLER( watchdog_reset_w );
READ8_HANDLER( watchdog_reset_r );

/* 16-bit watchdog read/write handlers */
WRITE16_HANDLER( watchdog_reset16_w );
READ16_HANDLER( watchdog_reset16_r );

/* 32-bit watchdog read/write handlers */
WRITE32_HANDLER( watchdog_reset32_w );
READ32_HANDLER( watchdog_reset32_r );



/* ----- generic input port read handlers ----- */

/* 8-bit handlers */
READ8_HANDLER( input_port_0_r );
READ8_HANDLER( input_port_1_r );
READ8_HANDLER( input_port_2_r );
READ8_HANDLER( input_port_3_r );
READ8_HANDLER( input_port_4_r );
READ8_HANDLER( input_port_5_r );
READ8_HANDLER( input_port_6_r );
READ8_HANDLER( input_port_7_r );
READ8_HANDLER( input_port_8_r );
READ8_HANDLER( input_port_9_r );
READ8_HANDLER( input_port_10_r );
READ8_HANDLER( input_port_11_r );
READ8_HANDLER( input_port_12_r );
READ8_HANDLER( input_port_13_r );
READ8_HANDLER( input_port_14_r );
READ8_HANDLER( input_port_15_r );
READ8_HANDLER( input_port_16_r );
READ8_HANDLER( input_port_17_r );
READ8_HANDLER( input_port_18_r );
READ8_HANDLER( input_port_19_r );
READ8_HANDLER( input_port_20_r );
READ8_HANDLER( input_port_21_r );
READ8_HANDLER( input_port_22_r );
READ8_HANDLER( input_port_23_r );
READ8_HANDLER( input_port_24_r );
READ8_HANDLER( input_port_25_r );
READ8_HANDLER( input_port_26_r );
READ8_HANDLER( input_port_27_r );
READ8_HANDLER( input_port_28_r );
READ8_HANDLER( input_port_29_r );
READ8_HANDLER( input_port_30_r );
READ8_HANDLER( input_port_31_r );

/* 16-bit handlers */
READ16_HANDLER( input_port_0_word_r );
READ16_HANDLER( input_port_1_word_r );
READ16_HANDLER( input_port_2_word_r );
READ16_HANDLER( input_port_3_word_r );
READ16_HANDLER( input_port_4_word_r );
READ16_HANDLER( input_port_5_word_r );
READ16_HANDLER( input_port_6_word_r );
READ16_HANDLER( input_port_7_word_r );
READ16_HANDLER( input_port_8_word_r );
READ16_HANDLER( input_port_9_word_r );
READ16_HANDLER( input_port_10_word_r );
READ16_HANDLER( input_port_11_word_r );
READ16_HANDLER( input_port_12_word_r );
READ16_HANDLER( input_port_13_word_r );
READ16_HANDLER( input_port_14_word_r );
READ16_HANDLER( input_port_15_word_r );
READ16_HANDLER( input_port_16_word_r );
READ16_HANDLER( input_port_17_word_r );
READ16_HANDLER( input_port_18_word_r );
READ16_HANDLER( input_port_19_word_r );
READ16_HANDLER( input_port_20_word_r );
READ16_HANDLER( input_port_21_word_r );
READ16_HANDLER( input_port_22_word_r );
READ16_HANDLER( input_port_23_word_r );
READ16_HANDLER( input_port_24_word_r );
READ16_HANDLER( input_port_25_word_r );
READ16_HANDLER( input_port_26_word_r );
READ16_HANDLER( input_port_27_word_r );
READ16_HANDLER( input_port_28_word_r );
READ16_HANDLER( input_port_29_word_r );
READ16_HANDLER( input_port_30_word_r );
READ16_HANDLER( input_port_31_word_r );

/* 32-bit handlers */
READ32_HANDLER( input_port_0_dword_r );
READ32_HANDLER( input_port_1_dword_r );
READ32_HANDLER( input_port_2_dword_r );
READ32_HANDLER( input_port_3_dword_r );
READ32_HANDLER( input_port_4_dword_r );
READ32_HANDLER( input_port_5_dword_r );
READ32_HANDLER( input_port_6_dword_r );
READ32_HANDLER( input_port_7_dword_r );
READ32_HANDLER( input_port_8_dword_r );
READ32_HANDLER( input_port_9_dword_r );
READ32_HANDLER( input_port_10_dword_r );
READ32_HANDLER( input_port_11_dword_r );
READ32_HANDLER( input_port_12_dword_r );
READ32_HANDLER( input_port_13_dword_r );
READ32_HANDLER( input_port_14_dword_r );
READ32_HANDLER( input_port_15_dword_r );
READ32_HANDLER( input_port_16_dword_r );
READ32_HANDLER( input_port_17_dword_r );
READ32_HANDLER( input_port_18_dword_r );
READ32_HANDLER( input_port_19_dword_r );
READ32_HANDLER( input_port_20_dword_r );
READ32_HANDLER( input_port_21_dword_r );
READ32_HANDLER( input_port_22_dword_r );
READ32_HANDLER( input_port_23_dword_r );
READ32_HANDLER( input_port_24_dword_r );
READ32_HANDLER( input_port_25_dword_r );
READ32_HANDLER( input_port_26_dword_r );
READ32_HANDLER( input_port_27_dword_r );
READ32_HANDLER( input_port_28_dword_r );
READ32_HANDLER( input_port_29_dword_r );
READ32_HANDLER( input_port_30_dword_r );
READ32_HANDLER( input_port_31_dword_r );


#endif	/* __MACHINE_GENERIC_H__ */
