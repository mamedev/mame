// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	Jangou Custom Blitter Chip, codename "???" (name scratched afaik)

	device emulation by Angelo Salese, from original jangou.cpp implementation
	 by Angelo Salese, David Haywood and Phil Bennett

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
	if (y < 0 || y >= 512)
		return;
	if (x < 0 || x >= 512)
		return;

	if (x & 1)
		m_blit_buffer[(y * 256) + (x >> 1)] = (m_blit_buffer[(y * 256) + (x >> 1)] & 0x0f) | ((pix << 4) & 0xf0);
	else
		m_blit_buffer[(y * 256) + (x >> 1)] = (m_blit_buffer[(y * 256) + (x >> 1)] & 0xf0) | (pix & 0x0f);
}

WRITE8_MEMBER( jangou_blitter_device::blitter_process_w )
{
	int src, x, y, h, w, flipx;
	m_blit_data[offset] = data;

	if (offset == 5)
	{
		int count = 0;
		int xcount, ycount;

		printf("%02x %02x %02x %02x %02x %02x %02x\n", m_blit_data[0], m_blit_data[1], m_blit_data[2],
		           m_blit_data[3], m_blit_data[4], m_blit_data[5],m_blit_data[6]); 
		w = (m_blit_data[4] & 0xff) + 1;
		h = (m_blit_data[5] & 0xff) + 1;
		src = ((m_blit_data[1] << 8)|(m_blit_data[0] << 0));
		src |= (m_blit_data[6] & 3) << 16;
		x = (m_blit_data[2] & 0xff);
		y = (m_blit_data[3] & 0xff);

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
				UINT8 cur_pen_hi = m_pen_data[(dat & 0xf0) >> 4];
				UINT8 cur_pen_lo = m_pen_data[(dat & 0x0f) >> 0];

				dat = cur_pen_lo | (cur_pen_hi << 4);

				if ((dat & 0xff) != 0)
					plot_gfx_pixel(dat, drawx, drawy);

				if (!flipx)
					count--;
				else
					count++;
			}
		}
	}
}

WRITE8_MEMBER( jangou_blitter_device::blitter_alt_process_w)
{
	switch(offset)
	{
		case 0: blitter_process_w(space,0,data); break;
		case 1: blitter_process_w(space,1,data); break;
		case 2: blitter_process_w(space,6,data); break;
		case 3: blitter_process_w(space,2,data); break;
		case 4: blitter_process_w(space,3,data); break;
		case 5: blitter_process_w(space,4,data); break;
		case 6: blitter_process_w(space,5,data); break;

	}
}

WRITE8_MEMBER( jangou_blitter_device::blitter_vregs_w)
{
	//  printf("%02x %02x\n", offset, data);
	m_pen_data[offset] = data & 0xf;
}

