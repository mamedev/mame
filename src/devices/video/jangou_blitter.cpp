// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Jangou Custom Blitter Chip, codename "???" (name scratched afaik)

    device emulation by Angelo Salese, from original jangou.cpp implementation
     by Angelo Salese, David Haywood and Phil Bennett.

    TODO:
    - BLTFLIP mechanism;
    - clean-ups;

***************************************************************************/

#include "emu.h"
#include "jangou_blitter.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type JANGOU_BLITTER = &device_creator<jangou_blitter_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  jangou_blitter_device - constructor
//-------------------------------------------------

jangou_blitter_device::jangou_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, JANGOU_BLITTER, "Jangou Blitter Custom Chip", tag, owner, clock, "jangou_blitter", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jangou_blitter_device::device_start()
{
	memory_region *devregion =  machine().root_device().memregion("gfx");
	m_gfxrom = devregion->base();
	if (m_gfxrom == nullptr)
		fatalerror("JANGOU_BLITTER: \"gfx\" memory base not found");
	m_gfxrommask = devregion->bytes()-1;

	save_item(NAME(m_pen_data));
	save_item(NAME(m_blit_data));
	save_item(NAME(m_blit_buffer));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jangou_blitter_device::device_reset()
{
	memset(m_blit_data, 0, ARRAY_LENGTH(m_blit_data));
	memset(m_pen_data, 0, ARRAY_LENGTH(m_pen_data));
	m_bltflip = false;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// TODO: inline these
UINT8 jangou_blitter_device::gfx_nibble( UINT32 niboffset )
{
	if (niboffset & 1)
		return (m_gfxrom[(niboffset >> 1) & m_gfxrommask] & 0xf0) >> 4;
	else
		return (m_gfxrom[(niboffset >> 1) & m_gfxrommask] & 0x0f);
}

void jangou_blitter_device::plot_gfx_pixel( UINT8 pix, int x, int y )
{
	if (y < 0 || y >= 256)
		return;
	if (x < 0 || x >= 256)
		return;

	if (x & 1)
		m_blit_buffer[(y * 256) + (x >> 1)] = (m_blit_buffer[(y * 256) + (x >> 1)] & 0x0f) | ((pix << 4) & 0xf0);
	else
		m_blit_buffer[(y * 256) + (x >> 1)] = (m_blit_buffer[(y * 256) + (x >> 1)] & 0xf0) | (pix & 0x0f);
}

WRITE8_MEMBER( jangou_blitter_device::process_w )
{
	int src, x, y, h, w, flipx;
	m_blit_data[offset] = data;

	if (offset == 5)
	{
		int count = 0;
		int xcount, ycount;

		w = (m_blit_data[4] & 0xff) + 1;
		h = (m_blit_data[5] & 0xff) + 1;
		src = ((m_blit_data[1] << 8)|(m_blit_data[0] << 0));
		src |= (m_blit_data[6] & 3) << 16;
		x = (m_blit_data[2] & 0xff);
		y = (m_blit_data[3] & 0xff);

		#if 0
		if(m_bltflip == true)
		{
			printf("%02x %02x %02x %02x %02x %02x %02x\n", m_blit_data[0], m_blit_data[1], m_blit_data[2],m_blit_data[3], m_blit_data[4], m_blit_data[5],m_blit_data[6]);
			printf("=>");
			for(int i=0;i<0x10;i++)
				printf("%02x ",m_pen_data[i]);
			printf("\n");
		}
		#endif
		// lowest bit of src controls flipping / draw direction?
		flipx = (m_blit_data[0] & 1);

		if (!flipx)
			src += (w * h) - 1;
		else
			src -= (w * h) - 1;


		for (ycount = 0; ycount < h; ycount++)
		{
			for(xcount = 0; xcount < w; xcount++)
			{
				int drawx = (x + xcount) & 0xff;
				int drawy = (y + ycount) & 0xff;
				UINT8 dat = gfx_nibble(src + count);
				UINT8 cur_pen = m_pen_data[dat & 0x0f];

				//dat = cur_pen_lo | (cur_pen_hi << 4);

				if ((cur_pen & 0xff) != 0)
					plot_gfx_pixel(cur_pen, drawx, drawy);

				if (!flipx)
					count--;
				else
					count++;
			}
		}

		//UINT32 new_src = src + count;

		// update source and height after blitter operation
		// TODO: Jangou doesn't agree with this, later HW?
		#if 0
		m_blit_data[0] = new_src & 0xfe;
		m_blit_data[1] = new_src >> 8;
		m_blit_data[5] = 0;
		m_blit_data[6] = new_src >> 16;
		#endif
		m_bltflip = false;
	}
}

// Sexy Gal swaps around upper src address
WRITE8_MEMBER( jangou_blitter_device::alt_process_w )
{
	const UINT8 translate_addr[7] = { 0, 1, 6, 2, 3, 4, 5 };

	process_w(space,translate_addr[offset],data);
}

WRITE8_MEMBER( jangou_blitter_device::vregs_w )
{
	// bit 5 set by Jangou, left-over?
	m_pen_data[offset] = data & 0x0f;
}

WRITE8_MEMBER( jangou_blitter_device::bltflip_w )
{
	// TODO: unsure about how this works, Charles says it swaps the nibble but afaik it's used for CPU tiles in Night Gal Summer/Sexy Gal and they seems fine?
	//       Maybe flipx is actually bltflip for later HW?
	m_bltflip = true;
}

READ_LINE_MEMBER( jangou_blitter_device::status_r )
{
	return false;
}
