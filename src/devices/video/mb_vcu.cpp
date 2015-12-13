// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Device for Mazer Blazer/Great Guns custom Video Controller Unit

Written by Angelo Salese, based off old implementation by Jarek Burczynski

TODO:
- understand what exactly modes 0x03 and 0x13 really reads in set_clr() and
  where it puts results (yeah, shared VCU RAM, but exactly where?). Almost
  surely Mazer Blazer tries to read the pixel data for collision detection and
  Great Guns read backs VRAM for VCU test (patched for now, btw).
- Understand look-up tables in i/o space.
- Understand how to handle layer clearance.
- Understand how planes are really handled.
- Understand how transparent pens are handled (is 0x0f always transparent or
  there's some clut gimmick? Great Guns title screen makes me think of the
  latter option)

***************************************************************************/

#include "emu.h"
#include "video/mb_vcu.h"
#include "video/resnet.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type MB_VCU = &device_creator<mb_vcu_device>;

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void mb_vcu_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<mb_vcu_device &>(device).m_palette.set_tag(tag);
}


static ADDRESS_MAP_START( mb_vcu_vram, AS_0, 8, mb_vcu_device )
	AM_RANGE(0x00000,0x7ffff) AM_RAM // enough for a 256x256x4 x 2 pages of framebuffer with 4 layers (TODO: doubled for simplicity)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mb_vcu_pal_ram, AS_1, 8, mb_vcu_device )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0200, 0x02ff) AM_RAM
	AM_RANGE(0x0400, 0x04ff) AM_RAM
	AM_RANGE(0x0600, 0x06ff) AM_READWRITE(mb_vcu_paletteram_r,mb_vcu_paletteram_w)
ADDRESS_MAP_END

READ8_MEMBER( mb_vcu_device::mb_vcu_paletteram_r )
{
	return m_palram[offset];
}

WRITE8_MEMBER( mb_vcu_device::mb_vcu_paletteram_w )
{
	int r,g,b, bit0, bit1, bit2;

	m_palram[offset] = data;

	/* red component */
	bit1 = (m_palram[offset] >> 7) & 0x01;
	bit0 = (m_palram[offset] >> 6) & 0x01;
	r = combine_2_weights(m_weights_r, bit0, bit1);

	/* green component */
	bit2 = (m_palram[offset] >> 5) & 0x01;
	bit1 = (m_palram[offset] >> 4) & 0x01;
	bit0 = (m_palram[offset] >> 3) & 0x01;
	g = combine_3_weights(m_weights_g, bit0, bit1, bit2);

	/* blue component */
	bit2 = (m_palram[offset] >> 2) & 0x01;
	bit1 = (m_palram[offset] >> 1) & 0x01;
	bit0 = (m_palram[offset] >> 0) & 0x01;
	b = combine_3_weights(m_weights_b, bit0, bit1, bit2);

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *mb_vcu_device::memory_space_config(address_spacenum spacenum) const
{
	switch (spacenum)
	{
		case AS_0: return &m_videoram_space_config;
		case AS_1: return &m_paletteram_space_config;
		default: return nullptr;
	}
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_byte - read a byte at the given address
//-------------------------------------------------

inline UINT8 mb_vcu_device::read_byte(offs_t address)
{
	return space(AS_0).read_byte(address);
}

//-------------------------------------------------
//  write_byte - write a byte at the given address
//-------------------------------------------------

inline void mb_vcu_device::write_byte(offs_t address, UINT8 data)
{
	space(AS_0).write_byte(address, data);
}

//-------------------------------------------------
//  read_byte - read a byte at the given i/o
//-------------------------------------------------

inline UINT8 mb_vcu_device::read_io(offs_t address)
{
	return space(AS_1).read_byte(address);
}

//-------------------------------------------------
//  write_byte - write a byte at the given i/o
//-------------------------------------------------

inline void mb_vcu_device::write_io(offs_t address, UINT8 data)
{
	space(AS_1).write_byte(address, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb_vcu_device - constructor
//-------------------------------------------------

mb_vcu_device::mb_vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MB_VCU, "Mazer Blazer custom VCU", tag, owner, clock, "mb_vcu", __FILE__),
		device_memory_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 19, 0, nullptr, *ADDRESS_MAP_NAME(mb_vcu_vram)),
		m_paletteram_space_config("palram", ENDIANNESS_LITTLE, 8, 16, 0, nullptr, *ADDRESS_MAP_NAME(mb_vcu_pal_ram)),
		m_cpu(*this),
		m_palette(*this)
{
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void mb_vcu_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb_vcu_device::device_start()
{
	// TODO: m_screen_tag
	m_ram = auto_alloc_array_clear(machine(), UINT8, 0x800);
	m_palram = auto_alloc_array_clear(machine(), UINT8, 0x100);

	{
		static const int resistances_r[2]  = { 4700, 2200 };
		static const int resistances_gb[3] = { 10000, 4700, 2200 };

		/* just to calculate coefficients for later use */
		compute_resistor_weights(0, 255,    -1.0,
				3,  resistances_gb, m_weights_g,    3600,   0,
				3,  resistances_gb, m_weights_b,    3600,   0,
				2,  resistances_r,  m_weights_r,    3600,   0);
	}

	save_item(NAME(m_status));
	save_pointer(NAME(m_ram), 0x800);
	save_pointer(NAME(m_palram), 0x100);
	save_item(NAME(m_param_offset_latch));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_color1));
	save_item(NAME(m_color2));
	save_item(NAME(m_mode));
	save_item(NAME(m_pix_xsize));
	save_item(NAME(m_pix_ysize));
	save_item(NAME(m_vregs));
	save_item(NAME(m_bk_color));
	save_item(NAME(m_vbank));
	save_item(NAME(m_weights_r));
	save_item(NAME(m_weights_g));
	save_item(NAME(m_weights_b));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb_vcu_device::device_reset()
{
	m_status = 1;

	for(int i=0;i<0x80000;i++)
	{
		write_byte(i,0x0f);
	}
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************
//  UINT8 *pcg = memregion("sub2")->base();

READ8_MEMBER( mb_vcu_device::read_ram )
{
	return m_ram[offset];
}

WRITE8_MEMBER( mb_vcu_device::write_ram )
{
	m_ram[offset] = data;
}

WRITE8_MEMBER( mb_vcu_device::write_vregs )
{
	m_vregs[offset] = data;
}

/* latches RAM offset to send to params */
READ8_MEMBER( mb_vcu_device::load_params )
{
	m_param_offset_latch = offset;

	m_xpos      = m_ram[m_param_offset_latch + 1] | (m_ram[m_param_offset_latch + 2]<<8);
	m_ypos      = m_ram[m_param_offset_latch + 3] | (m_ram[m_param_offset_latch + 4]<<8);
	m_color1    = m_ram[m_param_offset_latch + 5];
	m_color2    = m_ram[m_param_offset_latch + 6];
	m_mode      = m_ram[m_param_offset_latch + 7];
	m_pix_xsize = m_ram[m_param_offset_latch + 8] + 1;
	m_pix_ysize = m_ram[m_param_offset_latch + 9] + 1;

	if(0)
	{
		printf("[0] %02x ",m_ram[m_param_offset_latch]);
		printf("X: %04x ",m_xpos);
		printf("Y: %04x ",m_ypos);
		printf("C1:%02x ",m_color1);
		printf("C2:%02x ",m_color2);
		printf("M :%02x ",m_mode);
		printf("XS:%02x ",m_pix_xsize);
		printf("YS:%02x ",m_pix_ysize);
		printf("\n");
	}

	return 0; // open bus?
}

READ8_MEMBER( mb_vcu_device::load_gfx )
{
	int xi,yi;
	int dstx,dsty;
	UINT8 dot;
	int bits = 0;
	UINT8 pen = 0;
	UINT8 cur_layer;

//  cur_layer = (m_mode & 0x3);
	cur_layer = 0;

	switch(m_mode >> 2)
	{
		case 0x00: // 4bpp
			for(yi=0;yi<m_pix_ysize;yi++)
			{
				for(xi=0;xi<m_pix_xsize;xi++)
				{
					dstx = (m_xpos + xi);
					dsty = (m_ypos + yi);

					if(dstx >= 0 && dsty >= 0 && dstx < 256 && dsty < 256)
					{
						dot = m_cpu->space(AS_PROGRAM).read_byte(((offset + (bits >> 3)) & 0x1fff) + 0x4000) >> (4-(bits & 7));
						dot&= 0xf;

						//if(dot != 0xf || m_mode & 2)
							write_byte(dstx|dsty<<8|cur_layer<<16|m_vbank<<18, dot);
					}
					bits += 4;
				}
			}
			break;

		case 0x02: // 1bpp
			for(yi=0;yi<m_pix_ysize;yi++)
			{
				for(xi=0;xi<m_pix_xsize;xi++)
				{
					dstx = (m_xpos + xi);
					dsty = (m_ypos + yi);

					if(dstx >= 0 && dsty >= 0 && dstx < 256 && dsty < 256)
					{
						dot = m_cpu->space(AS_PROGRAM).read_byte(((offset + (bits >> 3)) & 0x1fff) + 0x4000) >> (7-(bits & 7));
						dot&= 1;

						pen = dot ? (m_color1 >> 4) : (m_color1 & 0xf);
						//if(pen != 0xf || m_mode & 2)
							write_byte(dstx|dsty<<8|cur_layer<<16|m_vbank<<18, pen);
					}
					bits++;
				}
			}
			break;
		case 0x03: //2bpp
			for (yi = 0; yi < m_pix_ysize; yi++)
			{
				for (xi = 0; xi < m_pix_xsize; xi++)
				{
					dstx = (m_xpos + xi);
					dsty = (m_ypos + yi);

					if(dstx >= 0 && dsty >= 0 && dstx < 256 && dsty < 256)
					{
						dot = m_cpu->space(AS_PROGRAM).read_byte(((offset + (bits >> 3)) & 0x1fff) + 0x4000) >> (6-(bits & 7));

						switch(dot & 3)
						{
							case 0:
								pen = m_color1 & 0xf;
								break;
							case 1:
								pen = m_color1 >> 4;
								break;
							case 2:
								pen = m_color2 & 0xf;
								break;
							case 3:
								pen = m_color2 >> 4;
								break;
						}

						//if(pen != 0xf)
							write_byte(dstx|dsty<<8|cur_layer<<16|m_vbank<<18, pen);
					}

					bits+=2;
				}
			}
			break;

		default:
			popmessage("Unsupported draw mode");
			break;
	}

	return 0; // open bus?
}

/*
---0 -111 (0x07) write to i/o?
---0 -011 (0x03) read to i/o?
---1 -011 (0x13) read to vram?
*/
READ8_MEMBER( mb_vcu_device::load_set_clr )
{
	int xi,yi;
	int dstx,dsty;
//  UINT8 dot;
	int bits = 0;
	if(m_mode == 0x13 || m_mode == 0x03)
	{
		printf("[0] %02x ",m_ram[m_param_offset_latch]);
		printf("X: %04x ",m_xpos);
		printf("Y: %04x ",m_ypos);
		printf("C1:%02x ",m_color1);
		printf("C2:%02x ",m_color2);
		printf("M :%02x ",m_mode);
		printf("XS:%02x ",m_pix_xsize);
		printf("YS:%02x ",m_pix_ysize);
		printf("VB:%02x ",m_vbank);
		printf("\n");
	}

	switch(m_mode)
	{
		case 0x13:
		case 0x03:
			for (yi = 0; yi < m_pix_ysize; yi++)
			{
				for (xi = 0; xi < m_pix_xsize; xi++)
				{
					dstx = (m_xpos + xi);
					dsty = (m_ypos + yi);

					if(dstx < 256 && dsty < 256)
					{
						#if 0
						dot = m_cpu->space(AS_PROGRAM).read_byte(((offset + (bits >> 3)) & 0x1fff) + 0x4000) >> (6-(bits & 7));
						dot&= 3;

						switch(dot)
						{
							case 0:
								write_byte(dstx|dsty<<8, m_color1 & 0xf);
								break;
							case 1:
								write_byte(dstx|dsty<<8, m_color1 >> 4);
								break;
							case 2:
								write_byte(dstx|dsty<<8, m_color2 & 0xf);
								break;
							case 3:
								write_byte(dstx|dsty<<8, m_color2 >> 4);
								break;
						}
						#endif

						//write_byte(dstx|dsty<<8, m_mode >> 4);
					}

					bits+=2;
				}
			}
			break;

		case 0x07:
			for(int i=0;i<m_pix_xsize;i++)
				write_io(i+(m_ypos<<8),m_ram[offset + i]);

			break;
	}

	return 0; // open bus?
}

WRITE8_MEMBER( mb_vcu_device::background_color_w )
{
	int bit0,bit1,bit2;
	int r,g,b;
	m_bk_color = data;

	/* red component */
	bit1 = (m_bk_color >> 7) & 0x01;
	bit0 = (m_bk_color >> 6) & 0x01;
	r = combine_2_weights(m_weights_r, bit0, bit1);

	/* green component */
	bit2 = (m_bk_color >> 5) & 0x01;
	bit1 = (m_bk_color >> 4) & 0x01;
	bit0 = (m_bk_color >> 3) & 0x01;
	g = combine_3_weights(m_weights_g, bit0, bit1, bit2);

	/* blue component */
	bit2 = (m_bk_color >> 2) & 0x01;
	bit1 = (m_bk_color >> 1) & 0x01;
	bit0 = (m_bk_color >> 0) & 0x01;
	b = combine_3_weights(m_weights_b, bit0, bit1, bit2);

	m_palette->set_pen_color(0x100, rgb_t(r, g, b));
}

READ8_MEMBER( mb_vcu_device::status_r )
{
	/*
	---- ---x busy or vblank flag
	*/
	return m_status;
}

WRITE8_MEMBER( mb_vcu_device::vbank_w )
{
	m_vbank = (data & 0x40) >> 6;
}

//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

UINT32 mb_vcu_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	UINT8 dot;

	bitmap.fill(0x100,cliprect);

	for(y=0;y<256;y++)
	{
		for(x=0;x<256;x++)
		{
			dot = read_byte((x >> 0)|(y<<8)|0<<16|(m_vbank ^ 1)<<18);
			//if(dot != 0xf)
			{
				dot|= m_vregs[1] << 4;

				bitmap.pix32(y,x) = m_palette->pen(dot);
			}
		}
	}

	#if 0
	for(y=0;y<256;y++)
	{
		for(x=0;x<256;x++)
		{
			dot = read_byte((x >> 0)|(y<<8)|3<<16);

			if(dot != 0xf)
			{
				dot|= m_vregs[1] << 4;

				bitmap.pix32(y,x) = machine().pens[dot];
			}
		}
	}

	for(y=0;y<256;y++)
	{
		for(x=0;x<256;x++)
		{
			dot = read_byte((x >> 0)|(y<<8)|0<<16);

			if(dot != 0xf)
			{
				dot|= m_vregs[1] << 4;

				bitmap.pix32(y,x) = machine().pens[dot];
			}
		}
	}

	for(y=0;y<256;y++)
	{
		for(x=0;x<256;x++)
		{
			dot = read_byte((x >> 0)|(y<<8)|1<<16);

			if(dot != 0xf)
			{
				dot|= m_vregs[1] << 4;

				bitmap.pix32(y,x) = machine().pens[dot];
			}
		}
	}
	#endif

	return 0;
}

void mb_vcu_device::screen_eof(void)
{
	//for(int i=0;i<0x10000;i++)
	{
		//write_byte(i|0x00000|m_vbank<<18,0x0f);
		//write_byte(i|0x10000|m_vbank<<18,0x0f);
		//write_byte(i|0x30000|m_vbank<<18,0x0f);
	}
}
