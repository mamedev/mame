// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    CES Blitter, with two layers and double buffering (Xilinx FPGA)

    Offset:     Bits:                     Value:

        00

        02      fedc ba-- ---- ----
                ---- --9- ---- ----       Layer 1 Buffer To Display
                ---- ---8 ---- ----       Layer 0 Buffer To Display
                ---- ---- 7654 3210

        04                                Width
        06                                X

        08                                Height - 1
        0A                                Y

        0C                                Source Address (low)
        0E                                Source Address (mid)

        10      fedc ba-- ---- ----
                ---- --9- ---- ----       Pen Replacement Mode
                ---- ---8 ---- ----
                ---- ---- 7--- ----       Layer To Draw To
                ---- ---- -6-- ----       Draw To Not Displayed Buffer
                ---- ---- --5- ----       Solid Fill
                ---- ---- ---4 ----       Enable VBlank IRQ           (e.g. level 3) (0 clears the IRQ line too)
                ---- ---- ---- 3---       Enable Blitter Finished IRQ (e.g. level 2) ""
                ---- ---- ---- -2--       Enable Scanline IRQ         (e.g. level 1) ""
                ---- ---- ---- --1-       Flip Y
                ---- ---- ---- ---0       Flip X

    Addr_Hi Register:

                fedc ba98 ---- ----       Solid Fill Pen
                ---- ---- 7654 3210       Source Address (high)

    Color Register:

                fedc ba98 ---- ----       Source Pen (Pen Replacement Mode)
                ---- ---- 7654 3210       Palette (0-f) / Destination Pen (Pen Replacement Mode)

    A write to the source address (mid) triggers the blit.
    A the end of the blit, an IRQ is issued.

***************************************************************************/

#include "emu.h"
#include "cesblit.h"

#include "screen.h"


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(CESBLIT, cesblit_device, "cesblit", "CES Blitter FPGA")

/***************************************************************************
    LIVE DEVICE
***************************************************************************/

//-------------------------------------------------
//  cesblit_device - constructor
//-------------------------------------------------

cesblit_device::cesblit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CESBLIT, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_space_config("blitter_space", ENDIANNESS_BIG, 16, 23),
	m_blit_irq_cb(*this)
{
}

device_memory_interface::space_config_vector cesblit_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config)
	};
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cesblit_device::device_start()
{
	// resolve callbacks
	m_blit_irq_cb.resolve();

	// default to rom reading from a region
	m_space = &space(AS_PROGRAM);
	memory_region *region = memregion(tag());
	if (region)
		m_space->install_rom(0, region->bytes() - 1, region->base());

	// bitmaps
	for (int layer = 0; layer < 2; ++layer)
	{
		for (int buffer = 0; buffer < 2; ++buffer)
		{
			m_bitmap[layer][buffer].allocate(512, 512);
			m_bitmap[layer][buffer].fill(0xff);
		}
	}
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void cesblit_device::device_stop()
{
	for (int layer = 0; layer < 2; ++layer)
	{
		for (int buffer = 0; buffer < 2; ++buffer)
		{
			m_bitmap[layer][buffer].reset();
		}
	}
}

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

READ16_MEMBER(cesblit_device::status_r)
{
	return 0x0000;  // bit 7 = blitter busy
}

WRITE16_MEMBER(cesblit_device::regs_w)
{
	uint16_t olddata = m_regs[offset];
	uint16_t newdata = COMBINE_DATA( &m_regs[offset] );

	switch (offset)
	{
//      case 0x00/2:    // bit 15: FPGA programming serial in (lsb first)

		case 0x10/2:
			if (!m_blit_irq_cb.isnull() && !BIT(olddata, 3) && BIT(newdata, 3))
				m_blit_irq_cb(CLEAR_LINE);
			break;

		case 0x0e/2:
			do_blit();
			if (!m_blit_irq_cb.isnull() && BIT(m_regs[0x10/2], 3))
				m_blit_irq_cb(ASSERT_LINE);
			break;
	}
}

WRITE16_MEMBER(cesblit_device::color_w)
{
	COMBINE_DATA( &m_color );
}

WRITE16_MEMBER(cesblit_device::addr_hi_w)
{
	COMBINE_DATA( &m_addr_hi );
}

void cesblit_device::do_blit()
{
	int buffer  =   (m_regs[0x02/2] >> 8) & 3;   // 1 bit per layer, selects the currently displayed buffer
	int sw      =    m_regs[0x04/2];
	int sx      =    m_regs[0x06/2];
	int sh      =    m_regs[0x08/2] + 1;
	int sy      =    m_regs[0x0a/2];
	int addr    =   (*m_compute_addr)(m_regs[0x0c/2], m_regs[0x0e/2], m_addr_hi);
	int mode    =    m_regs[0x10/2];

	int layer   =   (mode >> 7) & 1;    // layer to draw to
	buffer      =   ((mode >> 6) & 1) ^ ((buffer >> layer) & 1);    // bit 6 selects whether to use the opposite buffer to that displayed

	addr <<= 1;

#ifdef MAME_DEBUG
#if 0
	logerror("%s: blit w %03x, h %02x, x %03x, y %02x, src %06x, fill/addr_hi %04x, repl/color %04x, mode %02x\n", machine().describe_context(),
			sw,sh,sx,sy, addr, m_addr_hi, m_color, mode
	);
#endif
#endif

	sx &= 0x1ff;
	sw &= 0x1ff;    // can be 0: draw nothing (see e.g. fade-in effect in galgame3/diamond derby)

	sy &= 0x1ff;
	sh &= 0x1ff;

	int flipx = mode & 1;
	int flipy = mode & 2;

	int x0,x1,y0,y1,dx,dy;

	if (flipx)  { x0 = sw-1;    x1 = -1;    dx = -1;    sx -= sw-1; }
	else        { x0 = 0;       x1 = sw;    dx = +1;    }

	if (flipy)  { y0 = sh-1;    y1 = -1;    dy = -1;    sy -= sh-1; }
	else        { y0 = 0;       y1 = sh;    dy = +1;    }

	int color = (m_color & 0x0f) << 8;

	// Draw

	bitmap_ind16 &bitmap = m_bitmap[layer][buffer];
	int x,y;
	uint16_t pen;

	switch (mode & 0x20)
	{
		case 0x00:    // blit from ROM

			if ( mode & 0x200 )
			{
				// copy from ROM, replacing occurrences of src pen with dst pen

				uint8_t dst_pen = (m_color >> 8) & 0xff;
				uint8_t src_pen = (m_color >> 0) & 0xff;

				for (y = y0; y != y1; y += dy)
				{
					for (x = x0; x != x1; x += dx)
					{
						pen = m_space->read_byte(addr);

						if (pen == src_pen)
							pen = dst_pen;

						if (pen != 0xff)
							bitmap.pix16((sy + y) & 0x1ff, (sx + x) & 0x1ff) = pen + color;

						++addr;
					}
				}
			}
			else
			{
				// copy from ROM as is

				for (y = y0; y != y1; y += dy)
				{
					for (x = x0; x != x1; x += dx)
					{
						pen = m_space->read_byte(addr);

						if (pen != 0xff)
							bitmap.pix16((sy + y) & 0x1ff, (sx + x) & 0x1ff) = pen + color;

						++addr;
					}
				}
			}
			break;

		case 0x20:    // solid fill
			pen = ((m_addr_hi >> 8) & 0xff) + color;

			if ((pen & 0xff) == 0xff)
				pen = 0xff;

			for (y = y0; y != y1; y += dy)
			{
				for (x = x0; x != x1; x += dx)
				{
						bitmap.pix16((sy + y) & 0x1ff, (sx + x) & 0x1ff) = pen;
				}
			}
			break;
	}
}

uint32_t cesblit_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 2;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	if (layers_ctrl & 1)    copybitmap_trans(bitmap, m_bitmap[0][(m_regs[0x02/2]>>8)&1], 0,0,0,0, cliprect, 0xff);
	if (layers_ctrl & 2)    copybitmap_trans(bitmap, m_bitmap[1][(m_regs[0x02/2]>>9)&1], 0,0,0,0, cliprect, 0xff);

	return 0;
}
