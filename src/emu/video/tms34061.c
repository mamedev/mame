// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Aaron Giles
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


#define VERBOSE     (0)


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tms34061_device - constructor
//-------------------------------------------------

const device_type TMS34061 = &device_creator<tms34061_device>;

tms34061_device::tms34061_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS34061, "TMS34061 VSC", tag, owner, clock, "tms34061", __FILE__),
	device_video_interface(mconfig, *this),
	m_rowshift(0),
	m_vramsize(0),
	m_interrupt_cb(*this),
	m_xmask(0),
	m_yshift(0),
	m_vrammask(0),
	m_vram(NULL),
	m_latchram(NULL),
	m_latchdata(0),
	m_shiftreg(NULL),
	m_timer(NULL)
{
	memset(m_regs, 0, sizeof(m_regs));
	memset(&m_display, 0, sizeof(m_display));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms34061_device::device_start()
{
	/* resolve callbak */
	m_interrupt_cb.resolve();

	/* reset the data */
	m_vrammask = m_vramsize - 1;

	/* allocate memory for VRAM */
	m_vram = auto_alloc_array_clear(machine(), UINT8, m_vramsize + 256 * 2);

	/* allocate memory for latch RAM */
	m_latchram = auto_alloc_array_clear(machine(), UINT8, m_vramsize + 256 * 2);

	/* add some buffer space for VRAM and latch RAM */
	m_vram += 256;
	m_latchram += 256;

	/* point the shift register to the base of VRAM for now */
	m_shiftreg = m_vram;

	/* initialize registers to their default values from the manual */
	m_regs[TMS34061_HORENDSYNC]   = 0x0010;
	m_regs[TMS34061_HORENDBLNK]   = 0x0020;
	m_regs[TMS34061_HORSTARTBLNK] = 0x01f0;
	m_regs[TMS34061_HORTOTAL]     = 0x0200;
	m_regs[TMS34061_VERENDSYNC]   = 0x0004;
	m_regs[TMS34061_VERENDBLNK]   = 0x0010;
	m_regs[TMS34061_VERSTARTBLNK] = 0x00f0;
	m_regs[TMS34061_VERTOTAL]     = 0x0100;
	m_regs[TMS34061_DISPUPDATE]   = 0x0000;
	m_regs[TMS34061_DISPSTART]    = 0x0000;
	m_regs[TMS34061_VERINT]       = 0x0000;
	m_regs[TMS34061_CONTROL1]     = 0x7000;
	m_regs[TMS34061_CONTROL2]     = 0x0600;
	m_regs[TMS34061_STATUS]       = 0x0000;
	m_regs[TMS34061_XYOFFSET]     = 0x0010;
	m_regs[TMS34061_XYADDRESS]    = 0x0000;
	m_regs[TMS34061_DISPADDRESS]  = 0x0000;
	m_regs[TMS34061_VERCOUNTER]   = 0x0000;

	/* start vertical interrupt timer */
	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tms34061_device::interrupt), this));

	save_item(NAME(m_regs));
	save_item(NAME(m_xmask));
	save_item(NAME(m_yshift));
	save_pointer(NAME(m_vram), m_vramsize);
	save_pointer(NAME(m_latchram), m_vramsize);
	save_item(NAME(m_latchdata));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms34061_device::device_reset()
{
}

/*************************************
 *
 *  Global variables
 *
 *************************************/

static const char *const regnames[] =
{
	"HORENDSYNC",   "HORENDBLNK",   "HORSTARTBLNK",     "HORTOTAL",
	"VERENDSYNC",   "VERENDBLNK",   "VERSTARTBLNK",     "VERTOTAL",
	"DISPUPDATE",   "DISPSTART",    "VERINT",           "CONTROL1",
	"CONTROL2",     "STATUS",       "XYOFFSET",         "XYADDRESS",
	"DISPADDRESS",  "VERCOUNTER"
};


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void tms34061_device::update_interrupts()
{
	/* if we have a callback, process it */
	if (!m_interrupt_cb.isnull())
	{
		/* if the status bit is set, and ints are enabled, turn it on */
		if ((m_regs[TMS34061_STATUS] & 0x0001) && (m_regs[TMS34061_CONTROL1] & 0x0400))
			m_interrupt_cb(ASSERT_LINE);
		else
			m_interrupt_cb(CLEAR_LINE);
	}
}


TIMER_CALLBACK_MEMBER( tms34061_device::interrupt )
{
	/* set timer for next frame */
	m_timer->adjust(m_screen->frame_period());

	/* set the interrupt bit in the status reg */
	m_regs[TMS34061_STATUS] |= 1;

	/* update the interrupt state */
	update_interrupts();
}



/*************************************
 *
 *  Register writes
 *
 *************************************/

void tms34061_device::register_w(address_space &space, offs_t offset, UINT8 data)
{
	int scanline;
	int regnum = offset >> 2;

	/* certain registers affect the display directly */
	if ((regnum >= TMS34061_HORENDSYNC && regnum <= TMS34061_DISPSTART) ||
		(regnum == TMS34061_CONTROL2))
		m_screen->update_partial(m_screen->vpos());

	/* store the hi/lo half */
	if (regnum < ARRAY_LENGTH(m_regs))
	{
		if (offset & 0x02)
			m_regs[regnum] = (m_regs[regnum] & 0x00ff) | (data << 8);
		else
			m_regs[regnum] = (m_regs[regnum] & 0xff00) | data;
	}

	/* log it */
	if (VERBOSE) logerror("%s:tms34061 %s = %04x\n", space.machine().describe_context(), regnames[regnum], m_regs[regnum]);

	/* update the state of things */
	switch (regnum)
	{
		/* vertical interrupt: adjust the timer */
		case TMS34061_VERINT:
			scanline = m_regs[TMS34061_VERINT] - m_regs[TMS34061_VERENDBLNK];

			if (scanline < 0)
				scanline += m_regs[TMS34061_VERTOTAL];

			m_timer->adjust(m_screen->time_until_pos(scanline, m_regs[TMS34061_HORSTARTBLNK]));
			break;

		/* XY offset: set the X and Y masks */
		case TMS34061_XYOFFSET:
			switch (m_regs[TMS34061_XYOFFSET] & 0x00ff)
			{
				case 0x01:  m_yshift = 2;    break;
				case 0x02:  m_yshift = 3;    break;
				case 0x04:  m_yshift = 4;    break;
				case 0x08:  m_yshift = 5;    break;
				case 0x10:  m_yshift = 6;    break;
				case 0x20:  m_yshift = 7;    break;
				case 0x40:  m_yshift = 8;    break;
				case 0x80:  m_yshift = 9;    break;
				default:    logerror("Invalid value for XYOFFSET = %04x\n", m_regs[TMS34061_XYOFFSET]);  break;
			}
			m_xmask = (1 << m_yshift) - 1;
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

UINT8 tms34061_device::register_r(address_space &space, offs_t offset)
{
	int regnum = offset >> 2;
	UINT16 result;

	/* extract the correct portion of the register */
	if (regnum < ARRAY_LENGTH(m_regs))
		result = m_regs[regnum];
	else
		result = 0xffff;

	/* special cases: */
	switch (regnum)
	{
		/* status register: a read here clears it */
		case TMS34061_STATUS:
			m_regs[TMS34061_STATUS] = 0;
			update_interrupts();
			break;

		/* vertical count register: return the current scanline */
		case TMS34061_VERCOUNTER:
			result = (m_screen->vpos()+ m_regs[TMS34061_VERENDBLNK]) % m_regs[TMS34061_VERTOTAL];
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

void tms34061_device::adjust_xyaddress(int offset)
{
	/* note that carries are allowed if the Y coordinate isn't being modified */
	switch (offset & 0x1e)
	{
		case 0x00:  /* no change */
			break;

		case 0x02:  /* X + 1 */
			m_regs[TMS34061_XYADDRESS]++;
			break;

		case 0x04:  /* X - 1 */
			m_regs[TMS34061_XYADDRESS]--;
			break;

		case 0x06:  /* X = 0 */
			m_regs[TMS34061_XYADDRESS] &= ~m_xmask;
			break;

		case 0x08:  /* Y + 1 */
			m_regs[TMS34061_XYADDRESS] += 1 << m_yshift;
			break;

		case 0x0a:  /* X + 1, Y + 1 */
			m_regs[TMS34061_XYADDRESS] = (m_regs[TMS34061_XYADDRESS] & ~m_xmask) |
					((m_regs[TMS34061_XYADDRESS] + 1) & m_xmask);
			m_regs[TMS34061_XYADDRESS] += 1 << m_yshift;
			break;

		case 0x0c:  /* X - 1, Y + 1 */
			m_regs[TMS34061_XYADDRESS] = (m_regs[TMS34061_XYADDRESS] & ~m_xmask) |
					((m_regs[TMS34061_XYADDRESS] - 1) & m_xmask);
			m_regs[TMS34061_XYADDRESS] += 1 << m_yshift;
			break;

		case 0x0e:  /* X = 0, Y + 1 */
			m_regs[TMS34061_XYADDRESS] &= ~m_xmask;
			m_regs[TMS34061_XYADDRESS] += 1 << m_yshift;
			break;

		case 0x10:  /* Y - 1 */
			m_regs[TMS34061_XYADDRESS] -= 1 << m_yshift;
			break;

		case 0x12:  /* X + 1, Y - 1 */
			m_regs[TMS34061_XYADDRESS] = (m_regs[TMS34061_XYADDRESS] & ~m_xmask) |
					((m_regs[TMS34061_XYADDRESS] + 1) & m_xmask);
			m_regs[TMS34061_XYADDRESS] -= 1 << m_yshift;
			break;

		case 0x14:  /* X - 1, Y - 1 */
			m_regs[TMS34061_XYADDRESS] = (m_regs[TMS34061_XYADDRESS] & ~m_xmask) |
					((m_regs[TMS34061_XYADDRESS] - 1) & m_xmask);
			m_regs[TMS34061_XYADDRESS] -= 1 << m_yshift;
			break;

		case 0x16:  /* X = 0, Y - 1 */
			m_regs[TMS34061_XYADDRESS] &= ~m_xmask;
			m_regs[TMS34061_XYADDRESS] -= 1 << m_yshift;
			break;

		case 0x18:  /* Y = 0 */
			m_regs[TMS34061_XYADDRESS] &= m_xmask;
			break;

		case 0x1a:  /* X + 1, Y = 0 */
			m_regs[TMS34061_XYADDRESS]++;
			m_regs[TMS34061_XYADDRESS] &= m_xmask;
			break;

		case 0x1c:  /* X - 1, Y = 0 */
			m_regs[TMS34061_XYADDRESS]--;
			m_regs[TMS34061_XYADDRESS] &= m_xmask;
			break;

		case 0x1e:  /* X = 0, Y = 0 */
			m_regs[TMS34061_XYADDRESS] = 0;
			break;
	}
}


void tms34061_device::xypixel_w(address_space &space, int offset, UINT8 data)
{
	/* determine the offset, then adjust it */
	offs_t pixeloffs = m_regs[TMS34061_XYADDRESS];
	if (offset)
		adjust_xyaddress(offset);

	/* adjust for the upper bits */
	pixeloffs |= (m_regs[TMS34061_XYOFFSET] & 0x0f00) << 8;

	/* mask to the VRAM size */
	pixeloffs &= m_vrammask;
	if (VERBOSE) logerror("%s:tms34061 xy (%04x) = %02x/%02x\n", space.machine().describe_context(), pixeloffs, data, m_latchdata);

	/* set the pixel data */
	m_vram[pixeloffs] = data;
	m_latchram[pixeloffs] = m_latchdata;
}


UINT8 tms34061_device::xypixel_r(address_space &space, int offset)
{
	/* determine the offset, then adjust it */
	offs_t pixeloffs = m_regs[TMS34061_XYADDRESS];
	if (offset)
		adjust_xyaddress(offset);

	/* adjust for the upper bits */
	pixeloffs |= (m_regs[TMS34061_XYOFFSET] & 0x0f00) << 8;

	/* mask to the VRAM size */
	pixeloffs &= m_vrammask;

	/* return the result */
	return m_vram[pixeloffs];
}



/*************************************
 *
 *  Core writes
 *
 *************************************/

void tms34061_device::write(address_space &space, int col, int row, int func, UINT8 data)
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
			offs = ((row << m_rowshift) | col) & m_vrammask;
			if (m_regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (m_regs[TMS34061_CONTROL2] & 3) << 16;
			if (VERBOSE) logerror("%s:tms34061 direct (%04x) = %02x/%02x\n", space.machine().describe_context(), offs, data, m_latchdata);
			if (m_vram[offs] != data || m_latchram[offs] != m_latchdata)
			{
				m_vram[offs] = data;
				m_latchram[offs] = m_latchdata;
			}
			break;

		/* function 4 performs a shift reg transfer to VRAM */
		case 4:
			offs = col << m_rowshift;
			if (m_regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (m_regs[TMS34061_CONTROL2] & 3) << 16;
			offs &= m_vrammask;
			if (VERBOSE) logerror("%s:tms34061 shiftreg write (%04x)\n", space.machine().describe_context(), offs);

			memcpy(&m_vram[offs], m_shiftreg, (size_t)1 << m_rowshift);
			memset(&m_latchram[offs], m_latchdata, (size_t)1 << m_rowshift);
			break;

		/* function 5 performs a shift reg transfer from VRAM */
		case 5:
			offs = col << m_rowshift;
			if (m_regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (m_regs[TMS34061_CONTROL2] & 3) << 16;
			offs &= m_vrammask;
			if (VERBOSE) logerror("%s:tms34061 shiftreg read (%04x)\n", space.machine().describe_context(), offs);

			m_shiftreg = &m_vram[offs];
			break;

		/* log anything else */
		default:
			logerror("%s:Unsupported TMS34061 function %d\n", space.machine().describe_context(), func);
			break;
	}
}


UINT8 tms34061_device::read(address_space &space, int col, int row, int func)
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
			offs = ((row << m_rowshift) | col) & m_vrammask;
			result = m_vram[offs];
			break;

		/* function 4 performs a shift reg transfer to VRAM */
		case 4:
			offs = col << m_rowshift;
			if (m_regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (m_regs[TMS34061_CONTROL2] & 3) << 16;
			offs &= m_vrammask;

			memcpy(&m_vram[offs], m_shiftreg, (size_t)1 << m_rowshift);
			memset(&m_latchram[offs], m_latchdata, (size_t)1 << m_rowshift);
			break;

		/* function 5 performs a shift reg transfer from VRAM */
		case 5:
			offs = col << m_rowshift;
			if (m_regs[TMS34061_CONTROL2] & 0x0040)
				offs |= (m_regs[TMS34061_CONTROL2] & 3) << 16;
			offs &= m_vrammask;

			m_shiftreg = &m_vram[offs];
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

READ8_MEMBER( tms34061_device::latch_r )
{
	return m_latchdata;
}


WRITE8_MEMBER( tms34061_device::latch_w )
{
	if (VERBOSE) logerror("tms34061_latch = %02X\n", data);
	m_latchdata = data;
}


void tms34061_device::get_display_state()
{
	m_display.blanked = (~m_regs[TMS34061_CONTROL2] >> 13) & 1;
	m_display.vram = m_vram;
	m_display.latchram = m_latchram;
	m_display.regs = m_regs;

	/* compute the display start */
	m_display.dispstart = (m_regs[TMS34061_DISPSTART] << (m_rowshift - 2)) & m_vrammask;
}
