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


#define DEBUG_OUT_OF_MASK 0

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(JANGOU_BLITTER, jangou_blitter_device, "jangou_blitter", "Jangou Blitter Custom Chip")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  jangou_blitter_device - constructor
//-------------------------------------------------

jangou_blitter_device::jangou_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, JANGOU_BLITTER, tag, owner, clock)
{
}


void jangou_blitter_device::blit_v1_regs(address_map &map)
{
	map(0x00, 0x00).w(FUNC(jangou_blitter_device::src_lo_address_w));
	map(0x01, 0x01).w(FUNC(jangou_blitter_device::src_md_address_w));
	map(0x02, 0x02).w(FUNC(jangou_blitter_device::x_w));
	map(0x03, 0x03).w(FUNC(jangou_blitter_device::y_w));
	map(0x04, 0x04).w(FUNC(jangou_blitter_device::width_w));
	map(0x05, 0x05).w(FUNC(jangou_blitter_device::height_and_trigger_w));
	map(0x06, 0x06).w(FUNC(jangou_blitter_device::src_hi_address_w));
}

// Sexy Gal and variants (v2) swaps around upper src address
void jangou_blitter_device::blit_v2_regs(address_map &map)
{
	map(0x00, 0x00).w(FUNC(jangou_blitter_device::src_lo_address_w));
	map(0x01, 0x01).w(FUNC(jangou_blitter_device::src_md_address_w));
	map(0x02, 0x02).w(FUNC(jangou_blitter_device::src_hi_address_w));
	map(0x03, 0x03).w(FUNC(jangou_blitter_device::x_w));
	map(0x04, 0x04).w(FUNC(jangou_blitter_device::y_w));
	map(0x05, 0x05).w(FUNC(jangou_blitter_device::width_w));
	map(0x06, 0x06).w(FUNC(jangou_blitter_device::height_and_trigger_w));
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
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_src_addr));
	save_item(NAME(m_blit_buffer));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jangou_blitter_device::device_reset()
{
	memset(m_pen_data, 0, ARRAY_LENGTH(m_pen_data));
	m_bltflip = false;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// TODO: inline these
uint8_t jangou_blitter_device::gfx_nibble( uint32_t niboffset )
{
	if (niboffset & 1)
		return (m_gfxrom[(niboffset >> 1) & m_gfxrommask] & 0xf0) >> 4;
	else
		return (m_gfxrom[(niboffset >> 1) & m_gfxrommask] & 0x0f);
}

void jangou_blitter_device::plot_gfx_pixel( uint8_t pix, int x, int y )
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

void jangou_blitter_device::trigger_write(void)
{
	int src, x, y, h, w, flipx;
	int count = 0;
	int xcount, ycount;
#if DEBUG_OUT_OF_MASK
	bool debug_flag;
#endif

	w = (m_width & 0xff) + 1;
	h = (m_height & 0xff) + 1;

	src = m_src_addr & m_gfxrommask;
	if(m_src_addr & ~m_gfxrommask)
	{
		logerror("%s: Warning blit src address = %08x above ROM mask %08x\n",this->tag(),m_src_addr,m_gfxrommask);
#if DEBUG_OUT_OF_MASK
		debug_flag = true;
#endif
	}
#if DEBUG_OUT_OF_MASK
	else
		debug_flag = false;
#endif

	x = (m_x & 0xff);
	y = (m_y & 0xff);

	#if 0
	// bail out if parameters are blantantly invalid (an indication that the host is using protection tho)
	if((x + w) > 256 || (y + h) > 256)
	{
		printf("%d %d %d %d %08x\n",x,y,w,h,src);
//      return;
	}
	#endif

	// lowest bit of src controls flipping / draw direction?
	flipx = (m_src_addr & 1);

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
			uint8_t dat = gfx_nibble(src + count);
			uint8_t cur_pen = m_pen_data[dat & 0x0f];

#if DEBUG_OUT_OF_MASK
			if(debug_flag == true)
				cur_pen = machine().rand() & 0xf;
#endif
			//dat = cur_pen_lo | (cur_pen_hi << 4);

			if ((cur_pen & 0xff) != 0)
				plot_gfx_pixel(cur_pen, drawx, drawy);

			if (!flipx)
				count--;
			else
				count++;
		}
	}

	//uint32_t new_src = src + count;

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

// data accessors

WRITE8_MEMBER( jangou_blitter_device::x_w ) { m_x = data; }
WRITE8_MEMBER( jangou_blitter_device::y_w ) { m_y = data; }
WRITE8_MEMBER( jangou_blitter_device::src_lo_address_w )
{
	m_src_addr &= ~0xff;
	m_src_addr |= data & 0xff;
}

WRITE8_MEMBER( jangou_blitter_device::src_md_address_w )
{
	m_src_addr &= ~0xff00;
	m_src_addr |= data << 8;
}

WRITE8_MEMBER( jangou_blitter_device::src_hi_address_w )
{
	m_src_addr &= ~0xff0000;
	m_src_addr |= data << 16;
}

WRITE8_MEMBER( jangou_blitter_device::width_w )
{
	m_width = data;
}

WRITE8_MEMBER( jangou_blitter_device::height_and_trigger_w )
{
	m_height = data;
	trigger_write();
}

