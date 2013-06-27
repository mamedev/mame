/****************************************************************************
 *                                                                          *
 *  Function prototypes and constants used by the TMS34061 emulator         *
 *                                                                          *
 *  Created by Zsolt Vasvari on 5/26/1998.                                  *
 *  Updated by Aaron Giles on 11/21/2000.                                   *
 *                                                                          *
 ****************************************************************************/


#ifndef __TMS34061_H__
#define __TMS34061_H__

/* register constants */
enum
{
	TMS34061_HORENDSYNC = 0,
	TMS34061_HORENDBLNK,
	TMS34061_HORSTARTBLNK,
	TMS34061_HORTOTAL,
	TMS34061_VERENDSYNC,
	TMS34061_VERENDBLNK,
	TMS34061_VERSTARTBLNK,
	TMS34061_VERTOTAL,
	TMS34061_DISPUPDATE,
	TMS34061_DISPSTART,
	TMS34061_VERINT,
	TMS34061_CONTROL1,
	TMS34061_CONTROL2,
	TMS34061_STATUS,
	TMS34061_XYOFFSET,
	TMS34061_XYADDRESS,
	TMS34061_DISPADDRESS,
	TMS34061_VERCOUNTER,
	TMS34061_REGCOUNT
};



/* interface structure */
struct tms34061_interface
{
	const char  *screen_tag;    /* the screen we are acting on */
	UINT8       rowshift;       /* VRAM address is (row << rowshift) | col */
	UINT32      vramsize;       /* size of video RAM */
	void        (*interrupt)(running_machine &machine, int state);  /* interrupt gen callback */
};


/* display state structure */
struct tms34061_display
{
	UINT8   blanked;        /* true if blanked */
	UINT8   *vram;          /* base of VRAM */
	UINT8   *latchram;      /* base of latch RAM */
	UINT16  *regs;          /* pointer to array of registers */
	offs_t  dispstart;      /* display start */
};


/* starts/stops the emulator */
void tms34061_start(running_machine &machine, const struct tms34061_interface *interface);

/* reads/writes to the 34061 */
UINT8 tms34061_r(address_space &space, int col, int row, int func);
void tms34061_w(address_space &space, int col, int row, int func, UINT8 data);

/* latch settings */
DECLARE_READ8_HANDLER( tms34061_latch_r );
DECLARE_WRITE8_HANDLER( tms34061_latch_w );

/* video update handling */
void tms34061_get_display_state(struct tms34061_display *state);

#endif
