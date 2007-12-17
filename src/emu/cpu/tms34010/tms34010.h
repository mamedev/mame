/***************************************************************************

    TMS34010: Portable Texas Instruments TMS34010 emulator

    Copyright (C) Alex Pasadyn/Zsolt Vasvari 1998
    Parts based on code by Aaron Giles

***************************************************************************/

#ifndef _TMS34010_H
#define _TMS34010_H

#include "cpuintrf.h"
#include "driver.h"


/* the TMS34010 input clock is divided by 8; the 34020 by 4 */
#define TMS34010_CLOCK_DIVIDER		8
#define TMS34020_CLOCK_DIVIDER		4


/* register indexes for get_reg and set_reg */
enum
{
	TMS34010_PC = 1,
	TMS34010_SP,
	TMS34010_ST,
	TMS34010_A0,
	TMS34010_A1,
	TMS34010_A2,
	TMS34010_A3,
	TMS34010_A4,
	TMS34010_A5,
	TMS34010_A6,
	TMS34010_A7,
	TMS34010_A8,
	TMS34010_A9,
	TMS34010_A10,
	TMS34010_A11,
	TMS34010_A12,
	TMS34010_A13,
	TMS34010_A14,
	TMS34010_B0,
	TMS34010_B1,
	TMS34010_B2,
	TMS34010_B3,
	TMS34010_B4,
	TMS34010_B5,
	TMS34010_B6,
	TMS34010_B7,
	TMS34010_B8,
	TMS34010_B9,
	TMS34010_B10,
	TMS34010_B11,
	TMS34010_B12,
	TMS34010_B13,
	TMS34010_B14
};


/* Configuration structure */
typedef struct _tms34010_display_params tms34010_display_params;
struct _tms34010_display_params
{
	UINT16	vcount;								/* most recent VCOUNT */
	UINT16	veblnk, vsblnk;						/* start/end of VBLANK */
	UINT16	heblnk, hsblnk;						/* start/end of HBLANK */
	UINT16	rowaddr, coladdr;					/* row/column addresses */
	UINT8	yoffset;							/* y offset from addresses */
	UINT8	enabled;							/* video enabled */
};


typedef struct _tms34010_config tms34010_config;
struct _tms34010_config
{
	UINT8	halt_on_reset;						/* /HCS pin, which determines HALT state after reset */
	UINT8	scrnum;								/* the screen operated on */
	UINT32	pixclock;							/* the pixel clock (0 means don't adjust screen size) */
	int		pixperclock;						/* pixels per clock */
	void	(*scanline_callback)(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params);
	void	(*output_int)(int state);			/* output interrupt callback */
	void	(*to_shiftreg)(offs_t, UINT16 *);	/* shift register write */
	void	(*from_shiftreg)(offs_t, UINT16 *);	/* shift register read */
};


/* PUBLIC FUNCTIONS - 34010 */
VIDEO_UPDATE( tms340x0 );
void tms34010_get_display_params(int cpunum, tms34010_display_params *params);

void tms34010_get_info(UINT32 state, cpuinfo *info);

int 		tms34010_io_display_blanked(int cpu);
int 		tms34010_get_DPYSTRT(int cpu);


/* PUBLIC FUNCTIONS - 34020 */
void tms34020_get_info(UINT32 state, cpuinfo *info);

int 		tms34020_io_display_blanked(int cpu);
int 		tms34020_get_DPYSTRT(int cpu);



/* Host control interface */
#define TMS34010_HOST_ADDRESS_L		0
#define TMS34010_HOST_ADDRESS_H		1
#define TMS34010_HOST_DATA			2
#define TMS34010_HOST_CONTROL		3

void		tms34010_host_w(int cpunum, int reg, int data);
int			tms34010_host_r(int cpunum, int reg);


/* Reads & writes to the 34010 I/O registers; place at 0xc0000000 */
WRITE16_HANDLER( tms34010_io_register_w );
READ16_HANDLER( tms34010_io_register_r );

/* Reads & writes to the 34020 I/O registers; place at 0xc0000000 */
WRITE16_HANDLER( tms34020_io_register_w );
READ16_HANDLER( tms34020_io_register_r );


/* Use this macro in the memory definitions to specify bit-based addresses */
#define TOBYTE(bitaddr) ((offs_t)(bitaddr) >> 3)
#define TOWORD(bitaddr) ((offs_t)(bitaddr) >> 4)


#ifdef MAME_DEBUG
offs_t tms34010_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t tms34020_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif /* _TMS34010_H */
