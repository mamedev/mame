/****************************************************************************
 *                                                                          *
 *  Functions to emulate the TMS34061 video controller                      *
 *                                                                          *
 *  Created by Zsolt Vasvari on 5/26/1998.                                  *
 *  Updated by Aaron Giles on 11/21/2000.                                   *
 *                                                                          *
 *  This is far from complete. See the TMS34061 User's Guide available on   *
 *  www.spies.com/arcade                                                    *
 *                                                                          *
 ****************************************************************************/

#include "emu.h"
#include "tms34061.h"


#define VERBOSE		(0)


/*************************************
 *
 *  Internal structure
 *
 *************************************/

struct tms34061_data
{
	UINT16				regs[TMS34061_REGCOUNT];
	UINT16				xmask;
	UINT8				yshift;
	UINT32				vrammask;
	UINT8 *				vram;
	UINT8 *				latchram;
	UINT8				latchdata;
	UINT8 *				shiftreg;
	emu_timer *			timer;
	struct tms34061_interface intf;
	screen_device *screen;
};



/*************************************
 *
 *  Global variables
 *
 *************************************/

static struct tms34061_data tms34061;

static const char *const regnames[] =
{
	"HORENDSYNC",	"HORENDBLNK",	"HORSTARTBLNK",		"HORTOTAL",
	"VERENDSYNC",	"VERENDBLNK",	"VERSTARTBLNK",		"VERTOTAL",
	"DISPUPDATE",	"DISPSTART",	"VERINT",			"CONTROL1",
	"CONTROL2",		"STATUS",		"XYOFFSET",			"XYADDRESS",
	"DISPADDRESS",	"VERCOUNTER"
};



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static TIMER_CALLBACK( tms34061_interrupt );



/*************************************
 *
 *  Hardware startup
 *
 *************************************/

void tms34061_start(running_machine &machine, const struct tms34061_interface *interface)
{
	/* reset the data */
	memset(&tms34061, 0, sizeof(tms34061));
	tms34061.intf = *interface;
	tms34061.screen = downcast<screen_device *>(machine.device(tms34061.intf.screen_tag));
	tms34061.vrammask = tms34061.intf.vramsize - 1;

	/* allocate memory for VRAM */
	tms34061.vram = auto_alloc_array_clear(machine, UINT8, tms34061.intf.vramsize + 256 * 2);
	/* not really a save state, just there for debugging purposes */
	state_save_register_global_pointer(machine, tms34061.vram, tms34061.intf.vramsize);

	/* allocate memory for latch RAM */
	tms34061.latchram = auto_alloc_array_clear(machine, UINT8, tms34061.intf.vramsize + 256 * 2);

	/* add some buffer space for VRAM and latch RAM */
	tms34061.vram += 256;
	tms34061.latchram += 256;

	/* point the shift register to the base of VRAM for now */
	tms34061.shiftreg = tms34061.vram;

	/* initialize registers to their default values from the manual */
	tms34061.regs[TMS34061_HORENDSYNC]   = 0x0010;
	tms34061.regs[TMS34061_HORENDBLNK]   = 0x0020;
	tms34061.regs[TMS34061_HORSTARTBLNK] = 0x01f0;
	tms34061.regs[TMS34061_HORTOTAL]     = 0x0200;
	tms34061.regs[TMS34061_VERENDSYNC]   = 0x0004;
	tms34061.regs[TMS34061_VERENDBLNK]   = 0x0010;
	tms34061.regs[TMS34061_VERSTARTBLNK] = 0x00f0;
	tms34061.regs[TMS34061_VERTOTAL]     = 0x0100;
	tms34061.regs[TMS34061_DISPUPDATE]   = 0x0000;
	tms34061.regs[TMS34061_DISPSTART]    = 0x0000;
	tms34061.regs[TMS34061_VERINT]       = 0x0000;
	tms34061.regs[TMS34061_CONTROL1]     = 0x7000;
	tms34061.regs[TMS34061_CONTROL2]     = 0x0600;
	tms34061.regs[TMS34061_STATUS]       = 0x0000;
	tms34061.regs[TMS34061_XYOFFSET]     = 0x0010;
	tms34061.regs[TMS34061_XYADDRESS]    = 0x0000;
	tms34061.regs[TMS34061_DISPADDRESS]  = 0x0000;
	tms34061.regs[TMS34061_VERCOUNTER]   = 0x0000;

	/* start vertical interrupt timer */
	tms34061.timer = machine.scheduler().timer_alloc(FUNC(tms34061_interrupt));
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

INLINE void update_interrupts(void)
{
	/* if we have a callback, process it */
	if (tms34061.intf.interrupt)
	{
		/* if the status bit is set, and ints are enabled, turn it on */
		if ((tms34061.regs[TMS34061_STATUS] & 0x0001) && (tms34061.regs[TMS34061_CONTROL1] & 0x0400))
			(*tms34061.intf.interrupt)(tms34061.screen->machine(), ASSERT_LINE);
		else
			(*tms34061.intf.interrupt)(tms34061.screen->machine(), CLEAR_LINE);
	}
}


static TIMER_CALLBACK( tms34061_interrupt )
{
	/* set timer for next frame */
	tms34061.timer->adjust(tms34061.screen->frame_period());

	/* set the interrupt bit in the status reg */
	tms34061.regs[TMS34061_STATUS] |= 1;

	/* update the interrupt state */
	update_interrupts();
}



/*************************************
 *
 *  Register writes
 *
 *************************************/

static void register_w(address_space &space, offs_t offset, UINT8 data)
{
	int scanline;
	int regnum = offset >> 2;

	/* certain registers affect the display directly */
	if ((regnum >= TMS34061_HORENDSYNC && regnum <= TMS34061_DISPSTART) ||
		(regnum == TMS34061_CONTROL2))
		tms34061.screen->update_partial(tms34061.screen->vpos());

	/* store the hi/lo half */
	if (regnum < ARRAY_LENGTH(tms34061.regs))
	{
		if (offset & 0x02)
			tms34061.regs[regnum] = (tms34061.regs[regnum] & 0x00ff) | (data << 8);
		else
			tms34061.regs[regnum] = (tms34061.regs[regnum] & 0xff00) | data;
	}

	/* log it */
	if (VERBOSE) logerror("%s:tms34061 %s = %04x\n", space.machine().describe_context(), regnames[regnum], tms34061.regs[regnum]);

	/* update the state of things */
	switch (regnum)
	{
		/* vertical interrupt: adjust the timer */
		case TMS34061_VERINT:
			scanline = tms34061.regs[TMS34061_VERINT] - tms34061.regs[TMS34061_VERENDBLNK];

			if (scanline < 0)
				scanline += tms34061.regs[TMS34061_VERTOTAL];

			tms34061.timer->adjust(tms34061.screen->time_until_pos(scanline, tms34061.regs[TMS34061_HORSTARTBLNK]));
			break;

		/* XY offset: set the X and Y masks */
		case TMS34061_XYOFFSET:
			switch (tms34061.regs[TMS34061_XYOFFSET] & 0x00ff)
			{
				case 0x01:	tms34061.yshift = 2;	break;
				case 0x02:	tms34061.yshift = 3;	break;
				case 0x04:	tms34061.yshift = 4;	break;
				case 0x08:	tms34061.yshift = 5;	break;
				case 0x10:	tms34061.yshift = 6;	break;
				case 0x20:	tms34061.yshift = 7;	break;
				case 0x40:	tms34061.yshift = 8;	break;
				case 0x80:	tms34061.yshift = 9;	break;
				default:	logerror("Invalid value for XYOFFSET = %04x\n", tms34061.regs[TMS34061_XYOFFSET]);	break;
			}
			tms34061.xmask = (1 << tms34061.yshift) - 1;
			break;

		/* CONTROL1: they could have turned interrupts on */
		case TMS34061_CONTROL1:
			update_interrupts();
			break;

		/* other supported registers */
		case TMS34061_XYADDRESS:
			break;
	}
}



/*************************************
 *
 *  Register reads
 *
 *************************************/

static UINT8 register_r(address_space &space, offs_t offset)
{
	int regnum = offset >> 2;
	UINT16 result;

	/* extract the correct portion of the register */
	if (regnum < ARRAY_LENGTH(tms34061.regs))
		result = tms34061.regs[regnum];
	else
		result = 0xffff;

	/* special cases: */
	switch (regnum)
	{
		/* status register: a read here clears it */
		case TMS34061_STATUS:
			tms34061.regs[TMS34061_STATUS] = 0;
			update_interrupts();
			break;

		/* vertical count register: return the current scanline */
		case TMS34061_VERCOUNTER:
			result = (tms34061.screen->vpos()+ tms34061.regs[TMS34061_VERENDBLNK]) % tms34061.regs[TMS34061_VERTOTAL];
			break;
	}

	/* log it */
	if (VERBOSE) logerror("%s:tms34061 %s read = %04X\n", space.machine().describe_context(), regnames[regnum], result);
	return (offset & 0x02) ? (result >> 8) : result;
}



/*************************************
 *
 *  XY addressing
 *
 *************************************/

INLINE void adjust_xyaddress(int offset)
{
	/* note that carries are allowed if the Y coordinate isn't being modified */
	switch (offset & 0x1e)
	{
		case 0x00:	/* no change */
			break;

		case 0x02:	/* X + 1 */
			tms34061.regs[TMS34061_XYADDRESS]++;
			break;

		case 0x04:	/* X - 1 */
			tms34061.regs[TMS34061_XYADDRESS]--;
			break;

		case 0x06:	/* X = 0 */
			tms34061.regs[TMS34061_XYADDRESS] &= ~tms34061.xmask;
			break;

		case 0x08:	/* Y + 1 */
			tms34061.regs[TMS34061_XYADDRESS] += 1 << tms34061.yshift;
			break;

		case 0x0a:	/* X + 1, Y + 1 */
			tms34061.regs[TMS34061_XYADDRESS] = (tms34061.regs[TMS34061_XYADDRESS] & ~tms34061.xmask) |
					((tms34061.regs[TMS34061_XYADDRESS] + 1) & tms34061.xmask);
			tms34061.regs[TMS34061_XYADDRESS] += 1 << tms34061.yshift;
			break;

		case 0x0c:	/* X - 1, Y + 1 */
			tms34061.regs[TMS34061_XYADDRESS] = (tms34061.regs[TMS34061_XYADDRESS] & ~tms34061.xmask) |
					((tms34061.regs[TMS34061_XYADDRESS] - 1) & tms34061.xmask);
			tms34061.regs[TMS34061_XYADDRESS] += 1 << tms34061.yshift;
			break;

		case 0x0e:	/* X = 0, Y + 1 */
			tms34061.regs[TMS34061_XYADDRESS] &= ~tms34061.xmask;
			tms34061.regs[TMS34061_XYADDRESS] += 1 << tms34061.yshift;
			break;

		case 0x10:	/* Y - 1 */
			tms34061.regs[TMS34061_XYADDRESS] -= 1 << tms34061.yshift;
			break;

		case 0x12:	/* X + 1, Y - 1 */
			tms34061.regs[TMS34061_XYADDRESS] = (tms34061.regs[TMS34061_XYADDRESS] & ~tms34061.xmask) |
					((tms34061.regs[TMS34061_XYADDRESS] + 1) & tms34061.xmask);
			tms34061.regs[TMS34061_XYADDRESS] -= 1 << tms34061.yshift;
			break;

		case 0x14:	/* X - 1, Y - 1 */
			tms34061.regs[TMS34061_XYADDRESS] = (tms34061.regs[TMS34061_XYADDRESS] & ~tms34061.xmask) |
					((tms34061.regs[TMS34061_XYADDRESS] - 1) & tms34061.xmask);
			tms34061.regs[TMS34061_XYADDRESS] -= 1 << tms34061.yshift;
			break;

		case 0x16:	/* X = 0, Y - 1 */
			tms34061.regs[TMS34061_XYADDRESS] &= ~tms34061.xmask;
			tms34061.regs[TMS34061_XYADDRESS] -= 1 << tms34061.yshift;
			break;

		case 0x18:	/* Y = 0 */
			tms34061.regs[TMS34061_XYADDRESS] &= tms34061.xmask;
			break;

		case 0x1a:	/* X + 1, Y = 0 */
			tms34061.regs[TMS34061_XYADDRESS]++;
			tms34061.regs[TMS34061_XYADDRESS] &= tms34061.xmask;
			break;

		case 0x1c:	/* X - 1, Y = 0 */
			tms34061.regs[TMS34061_XYADDRESS]--;
			tms34061.regs[TMS34061_XYADDRESS] &= tms34061.xmask;
			break;

		case 0x1e:	/* X = 0, Y = 0 */
			tms34061.regs[TMS34061_XYADDRESS] = 0;
			break;
	}
}


static void xypixel_w(address_space &space, int offset, UINT8 data)
{
	/* determine the offset, then adjust it */
	offs_t pixeloffs = tms34061.regs[TMS34061_XYADDRESS];
	if (offset)
		adjust_xyaddress(offset);

	/* adjust for the upper bits */
	pixeloffs |= (tms34061.regs[TMS34061_XYOFFSET] & 0x0f00) << 8;

	/* mask to the VRAM size */
	pixeloffs &= tms34061.vrammask;
	if (VERBOSE) logerror("%s:tms34061 xy (%04x) = %02x/%02x\n", space.machine().describe_context(), pixeloffs, data, tms34061.latchdata);

	/* set the pixel data */
	tms34061.vram[pixeloffs] = data;
	tms34061.latchram[pixeloffs] = tms34061.latchdata;
}


static UINT8 xypixel_r(address_space &space, int offset)
{
	/* determine the offset, then adjust it */
	offs_t pixeloffs = tms34061.regs[TMS34061_XYADDRESS];
	if (offset)
		adjust_xyaddress(offset);

	/* adjust for the upper bits */
	pixeloffs |= (tms34061.regs[TMS34061_XYOFFSET] & 0x0f00) << 8;

	/* mask to the VRAM size */
	pixeloffs &= tms34061.vrammask;

	/* return the result */
	return tms34061.vram[pixeloffs];
}



/*************************************
 *
 *  Core writes
 *
 *************************************/

void tms34061_w(address_space &space, int col, int row, int func, UINT8 data)
{
	offs_t offs;

	/* the function code determines what to do */
	switch (func)
	{
		/* both 0 and 2 map to register access */
		case 0:
		case 2:
			register_w(space, col, data);
			break;

		/* function 1 maps to XY access; col is the address adjustment */
		case 1:
			xypixel_w(space, col, data);
			break;

		/* function 3 maps to direct access */
		case 3:
			offs = ((row << tms34061.intf.rowshift) | col) & tms34061.vrammask;
			if (tms34061.regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (tms34061.regs[TMS34061_CONTROL2] & 3) << 16;
			if (VERBOSE) logerror("%s:tms34061 direct (%04x) = %02x/%02x\n", space.machine().describe_context(), offs, data, tms34061.latchdata);
			if (tms34061.vram[offs] != data || tms34061.latchram[offs] != tms34061.latchdata)
			{
				tms34061.vram[offs] = data;
				tms34061.latchram[offs] = tms34061.latchdata;
			}
			break;

		/* function 4 performs a shift reg transfer to VRAM */
		case 4:
			offs = col << tms34061.intf.rowshift;
			if (tms34061.regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (tms34061.regs[TMS34061_CONTROL2] & 3) << 16;
			offs &= tms34061.vrammask;
			if (VERBOSE) logerror("%s:tms34061 shiftreg write (%04x)\n", space.machine().describe_context(), offs);

			memcpy(&tms34061.vram[offs], tms34061.shiftreg, (size_t)1 << tms34061.intf.rowshift);
			memset(&tms34061.latchram[offs], tms34061.latchdata, (size_t)1 << tms34061.intf.rowshift);
			break;

		/* function 5 performs a shift reg transfer from VRAM */
		case 5:
			offs = col << tms34061.intf.rowshift;
			if (tms34061.regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (tms34061.regs[TMS34061_CONTROL2] & 3) << 16;
			offs &= tms34061.vrammask;
			if (VERBOSE) logerror("%s:tms34061 shiftreg read (%04x)\n", space.machine().describe_context(), offs);

			tms34061.shiftreg = &tms34061.vram[offs];
			break;

		/* log anything else */
		default:
			logerror("%s:Unsupported TMS34061 function %d\n", space.machine().describe_context(), func);
			break;
	}
}


UINT8 tms34061_r(address_space &space, int col, int row, int func)
{
	int result = 0;
	offs_t offs;

	/* the function code determines what to do */
	switch (func)
	{
		/* both 0 and 2 map to register access */
		case 0:
		case 2:
			result = register_r(space, col);
			break;

		/* function 1 maps to XY access; col is the address adjustment */
		case 1:
			result = xypixel_r(space, col);
			break;

		/* funtion 3 maps to direct access */
		case 3:
			offs = ((row << tms34061.intf.rowshift) | col) & tms34061.vrammask;
			result = tms34061.vram[offs];
			break;

		/* function 4 performs a shift reg transfer to VRAM */
		case 4:
			offs = col << tms34061.intf.rowshift;
			if (tms34061.regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (tms34061.regs[TMS34061_CONTROL2] & 3) << 16;
			offs &= tms34061.vrammask;

			memcpy(&tms34061.vram[offs], tms34061.shiftreg, (size_t)1 << tms34061.intf.rowshift);
			memset(&tms34061.latchram[offs], tms34061.latchdata, (size_t)1 << tms34061.intf.rowshift);
			break;

		/* function 5 performs a shift reg transfer from VRAM */
		case 5:
			offs = col << tms34061.intf.rowshift;
			if (tms34061.regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (tms34061.regs[TMS34061_CONTROL2] & 3) << 16;
			offs &= tms34061.vrammask;

			tms34061.shiftreg = &tms34061.vram[offs];
			break;

		/* log anything else */
		default:
			logerror("%s:Unsupported TMS34061 function %d\n", space.machine().describe_context(),
					func);
			break;
	}

	return result;
}



/*************************************
 *
 *  Misc functions
 *
 *************************************/

READ8_HANDLER( tms34061_latch_r )
{
	return tms34061.latchdata;
}


WRITE8_HANDLER( tms34061_latch_w )
{
	if (VERBOSE) logerror("tms34061_latch = %02X\n", data);
	tms34061.latchdata = data;
}


void tms34061_get_display_state(struct tms34061_display *state)
{
	state->blanked = (~tms34061.regs[TMS34061_CONTROL2] >> 13) & 1;
	state->vram = tms34061.vram;
	state->latchram = tms34061.latchram;
	state->regs = tms34061.regs;

	/* compute the display start */
	state->dispstart = (tms34061.regs[TMS34061_DISPSTART] << (tms34061.intf.rowshift - 2)) & tms34061.vrammask;
}
