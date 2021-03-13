// license:BSD-3-Clause
// copyright-holders:Luca Elia, Nicola Salmoria, AJR
/**********************************************************************

    Dynax blitter, "revision 2" (TC17G032AP-0246 custom DIP64)

***********************************************************************

                            Blitter Data Format

    The blitter reads its commands from the gfx ROMs. They are
    instructions to draw an image pixel by pixel (in a compressed
    form) in a frame buffer.

    Fetch 1 Byte from the ROM:

    7654 ----   Pen to draw with
    ---- 3210   Command

    Other bytes may follow, depending on the command

    Commands:

    0       Stop.
    1-b     Draw 1-b pixels along X.
    c       Followed by 1 byte (N): draw N pixels along X.
    d       Followed by 2 bytes (X,N): skip X pixels, draw N pixels along X.
    e       ? unused
    f       Increment Y

**********************************************************************/

#include "emu.h"
#include "dynax_blitter_rev2.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(DYNAX_BLITTER_REV2, dynax_blitter_rev2_device, "tc17g032ap", "Dynax TC17G032AP Blitter")
DEFINE_DEVICE_TYPE(CDRACULA_BLITTER, cdracula_blitter_device, "cdracula_blitter", "Castle of Dracula TPC1020AFN Blitter")


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  dynax_blitter_rev2_device - constructor
//-------------------------------------------------

dynax_blitter_rev2_device::dynax_blitter_rev2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dynax_blitter_rev2_device(mconfig, DYNAX_BLITTER_REV2, tag, owner, clock)
{
}

dynax_blitter_rev2_device::dynax_blitter_rev2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rom_interface(mconfig, *this)
	, m_vram_out_cb(*this)
	, m_scrollx_cb(*this)
	, m_scrolly_cb(*this)
	, m_ready_cb(*this)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void dynax_blitter_rev2_device::device_resolve_objects()
{
	m_vram_out_cb.resolve_safe();
	m_scrollx_cb.resolve_safe();
	m_scrolly_cb.resolve_safe();
	m_ready_cb.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dynax_blitter_rev2_device::device_start()
{
	m_blit_pen = 0x7;
	m_blit_wrap_enable = 0;
	m_blit_x = 0;
	m_blit_y = 0;
	m_blit_flags = 0;
	m_blit_src = 0;

	save_item(NAME(m_blit_pen));
	save_item(NAME(m_blit_wrap_enable));
	save_item(NAME(m_blit_x));
	save_item(NAME(m_blit_y));
	save_item(NAME(m_blit_flags));
	save_item(NAME(m_blit_src));
}


//-------------------------------------------------
//  plot_pixel - send data for one pixel to VRAM
//-------------------------------------------------

void dynax_blitter_rev2_device::plot_pixel(int x, int y, int pen)
{
	if (y > 0xff && !BIT(m_blit_wrap_enable, 1))
		return;    // fixes mjdialq2 & mjangels title screens
	if (x > 0xff && !BIT(m_blit_wrap_enable, 0))
		return;

	x &= 0xff;  // confirmed by some mjdialq2 gfx and especially by mjfriday, which
				// uses the front layer to mask out the right side of the screen as
				// it draws stuff on the left, when it shows the girls scrolling
				// horizontally after you win.
	y &= 0xff;  // seems confirmed by mjdialq2 last picture of gal 6, but it breaks
				// mjdialq2 title screen so there's something we are missing. <fixed, see above>

	// Rotate: rotation = SWAPXY + FLIPY
	if (BIT(m_blit_flags, 3))
		std::swap(x, y);

	// Flip screen and destination layer selection are handled externally
	m_vram_out_cb(x | (y << 8), pen, 0xff);
}


//-------------------------------------------------
//  blitter_draw - perform a blit operation
//-------------------------------------------------

/*
    Flags:

    7654 ----   -
    ---- 3---   Rotation = SWAPXY + FLIPY
    ---- -2--   -
    ---- --1-   0 = Ignore the pens specified in ROM, draw everything with the pen supplied as parameter
    ---- ---0   Clear
*/
u32 dynax_blitter_rev2_device::blitter_draw(u32 src, int pen, int x, int y)
{
	pen = (pen >> 4) & 0xf;

	if (BIT(m_blit_flags, 0))
	{
		// Clear the buffer(s) starting from the given scanline and exit
		int addr = x | (y << 8);

		while (addr < 0x10000)
			m_vram_out_cb(addr++, pen, 0xff);

		return src;
	}

	int sx = x;

	src &= 0xfffff;

	for ( ;; )
	{
		u8 cmd = read_byte(src++);
		src &= 0xfffff;
		if (!BIT(m_blit_flags, 1))    // Ignore the pens specified in ROM, draw everything with the pen supplied as parameter
			pen = (cmd & 0xf0) >> 4;
		cmd = (cmd & 0x0f);

		switch (cmd)
		{
		case 0xf:   // Increment Y
			/* Rotate: rotation = SWAPXY + FLIPY */
			if (BIT(m_blit_flags, 3))
				y--;
			else
				y++;
			x = sx;
			break;

		case 0xe:   // unused ? was "change dest mask" in the "rev1" blitter
			LOG("Blitter unknown command %06X: %02X\n", src - 1, cmd);
			break;

		case 0xd:   // Skip X pixels
			x = sx + read_byte(src++);
			src &= 0xfffff;
			[[fallthrough]];
		case 0xc:   // Draw N pixels
			cmd = read_byte(src++);
			src &= 0xfffff;
			[[fallthrough]];
		case 0xb:
		case 0xa:
		case 0x9:
		case 0x8:
		case 0x7:
		case 0x6:
		case 0x5:
		case 0x4:
		case 0x3:
		case 0x2:
		case 0x1:   // Draw N pixels
			while (cmd--)
				plot_pixel(x++, y, pen);
			break;

		case 0x0:   // Stop
			return src;
		}
	}
}


//-------------------------------------------------
//  blitter_start - start and finish a blit
//-------------------------------------------------

void dynax_blitter_rev2_device::blitter_start()
{
	LOG("%s: XY=%X,%X SRC=%X BLIT=%X\n", machine().describe_context(), m_blit_x, m_blit_y, m_blit_src, m_blit_flags);

	// Blitter is busy
	m_ready_cb(0);

	u32 blit_newsrc = blitter_draw(m_blit_src, m_blit_pen, m_blit_x, m_blit_y);

	m_blit_src = (m_blit_src & ~0x0fffff) | (blit_newsrc & 0x0fffff);

	// Blitter is ready; generate an IRQ
	m_ready_cb(1);
}


//-------------------------------------------------
//  scroll_w - handle scroll/wrap register writes
//-------------------------------------------------

void dynax_blitter_rev2_device::scroll_w(u8 data)
{
	switch (m_blit_src & 0xc00000)
	{
	case 0x000000:
		m_scrollx_cb(data);
		LOG("%s: SX=%02X\n", machine().describe_context(), data);
		break;

	case 0x400000:
		m_scrolly_cb(data);
		LOG("%s: SY=%02X\n", machine().describe_context(), data);
		break;

	case 0x800000:
	case 0xc00000:
		m_blit_wrap_enable = data;
		LOG("%s: WE=%02X\n", machine().describe_context(), data);
		break;
	}
}


//-------------------------------------------------
//  pen_w - set the destination pen
//-------------------------------------------------

void dynax_blitter_rev2_device::pen_w(uint8_t data)
{
	m_blit_pen = data;
	LOG("%s: P=%02X\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  regs_w - handle blitter register writes
//-------------------------------------------------

void dynax_blitter_rev2_device::regs_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_blit_flags = data;
		blitter_start();
		break;

	case 1:
		m_blit_x = data;
		break;

	case 2:
		m_blit_y = data;
		break;

	case 3:
		m_blit_src = (m_blit_src & 0xffff00) | (data << 0);
		break;

	case 4:
		m_blit_src = (m_blit_src & 0xff00ff) | (data << 8);
		break;

	case 5:
		m_blit_src = (m_blit_src & 0x00ffff) | (data << 16);
		break;

	case 6:
		scroll_w(data);
		break;
	}
}


//**************************************************************************
//  DEVICE DEFINITION (BOOTLEG FPGA VARIANT)
//**************************************************************************

//-------------------------------------------------
//  cdracula_blitter_device - constructor
//-------------------------------------------------

cdracula_blitter_device::cdracula_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dynax_blitter_rev2_device(mconfig, CDRACULA_BLITTER, tag, owner, clock)
	, m_blit_dest_cb(*this)
{
}


//-------------------------------------------------
//  device_resolve_objects -
//-------------------------------------------------

void cdracula_blitter_device::device_resolve_objects()
{
	dynax_blitter_rev2_device::device_resolve_objects();
	m_blit_dest_cb.resolve_safe();
}


//-------------------------------------------------
//  flags_w - separate flags/start register for
//  bootleg blitter
//-------------------------------------------------

void cdracula_blitter_device::flags_w(uint8_t data)
{
	LOG("%s: FLG=%02X\n", machine().describe_context(), data);

	m_blit_flags  = (data & 0x08) ? 0x08 : 0x00;
	m_blit_flags |= (data & 0x10) ? 0x02 : 0x00;
	m_blit_flags |= (data & 0x80) ? 0x01 : 0x00;

	blitter_start();
}


//-------------------------------------------------
//  regs_w - handle blitter register writes
//  (slightly different for bootleg)
//-------------------------------------------------

void cdracula_blitter_device::regs_w(offs_t offset, uint8_t data)
{
	// first register does not trigger a blit, it sets the destination
	switch (offset)
	{
	case 0:
		LOG("%s: DD=%02X\n", machine().describe_context(), data);
		m_blit_dest_cb(data);
		break;

	case 1:
		m_blit_x = data;
		break;

	case 2:
		m_blit_y = data;
		break;

	case 3:
		m_blit_src = (m_blit_src & 0xffff00) | (data << 0);
		break;

	case 4:
		m_blit_src = (m_blit_src & 0xff00ff) | (data << 8);
		break;

	case 5:
		m_blit_src = (m_blit_src & 0x00ffff) | (data << 16);
		break;

	case 6:
		scroll_w(data);
		break;
	}
}
