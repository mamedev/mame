// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Device for Mazer Blazer/Great Guns custom Video Controller Unit

Written by Angelo Salese, based off old implementation by Jarek Burczynski

TODO:
- priority, especially noticeable in Great Guns sprites and Mazer Blazer
  bonus stages;
- bit 0 of m_mode;
- first byte of parameter info;
- Glitchy UFO in Mazer Blazer when it's gonna zap one of the player lives, m_mode = 0xe and
  it's supposed to be set into layer 0 somehow but this breaks Mazer Blazer title screen sparkles;
- Understand look-up tables in i/o space.
- Understand how to handle layer clearance (mostly done).
- Understand how planes are really handled (mostly done).
- Understand how transparent pens are handled aka is 0x0f always transparent or
  there's some clut gimmick? Great Guns title screen makes me think of the
  latter option;
- Mazer Blazer collision detection parameters are a complete guesswork

***************************************************************************/

#include "emu.h"
#include "mb_vcu.h"

#include "video/resnet.h"

#define LOG_PARAMS (1 << 1)
#define LOG_DRAW   (1 << 2)
#define LOG_CLEAR  (1 << 3)

#define LOG_ALL (LOG_PARAMS | LOG_DRAW | LOG_CLEAR)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGPARAMS(...)  LOGMASKED(LOG_PARAMS, __VA_ARGS__)
#define LOGDRAW(...)    LOGMASKED(LOG_DRAW, __VA_ARGS__)
#define LOGCLEAR(...)   LOGMASKED(LOG_CLEAR, __VA_ARGS__)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB_VCU, mb_vcu_device, "mb_vcu", "Mazer Blazer custom VCU")


void mb_vcu_device::vram(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00000, 0x7ffff).ram(); // enough for a 256x256x4 x 2 pages of framebuffer with 4 layers (TODO: doubled for simplicity)
}


void mb_vcu_device::pal_ram(address_map &map)
{
	if (!has_configured_map(1))
	{
		map(0x0000, 0x00ff).ram();
		map(0x0200, 0x02ff).ram();
		map(0x0400, 0x04ff).ram();
		map(0x0600, 0x06ff).rw(FUNC(mb_vcu_device::paletteram_r), FUNC(mb_vcu_device::paletteram_w));
	}
}

uint8_t mb_vcu_device::paletteram_r(offs_t offset)
{
	return m_palram[offset];
}

void mb_vcu_device::paletteram_w(offs_t offset, uint8_t data)
{
	m_palram[offset] = data;
	set_pen_indirect(offset, m_palram[offset]);
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector mb_vcu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_videoram_space_config),
		std::make_pair(1, &m_paletteram_space_config)
	};
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_byte - read a byte at the given address
//-------------------------------------------------

inline uint8_t mb_vcu_device::read_byte(offs_t address)
{
	return space(0).read_byte(address);
}

//-------------------------------------------------
//  write_byte - write a byte at the given address
//-------------------------------------------------

inline void mb_vcu_device::write_byte(offs_t address, uint8_t data)
{
	space(0).write_byte(address, data);
}

//-------------------------------------------------
//  read_byte - read a byte at the given i/o
//-------------------------------------------------

inline uint8_t mb_vcu_device::read_io(offs_t address)
{
	return space(1).read_byte(address);
}

//-------------------------------------------------
//  write_byte - write a byte at the given i/o
//-------------------------------------------------

inline void mb_vcu_device::write_io(offs_t address, uint8_t data)
{
	space(1).write_byte(address, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb_vcu_device - constructor
//-------------------------------------------------

mb_vcu_device::mb_vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MB_VCU, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 19, 0, address_map_constructor(FUNC(mb_vcu_device::vram), this))
	, m_paletteram_space_config("palram", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(mb_vcu_device::pal_ram), this))
	, m_host_space(*this, finder_base::DUMMY_TAG, 0, 8)
	, m_status(1)
	, m_ram(nullptr)
	, m_palram(nullptr)
	, m_param_offset_latch(0)
	, m_xpos(0)
	, m_ypos(0)
	, m_color(0)
	, m_mode(0)
	, m_pix_xsize(0)
	, m_pix_ysize(0)
	, m_vregs{0}
	, m_bk_color(0)
	, m_vbank(0)
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
	m_host_space->cache(m_memory);
	m_ram = make_unique_clear<uint8_t[]>(0x800);
	m_palram = make_unique_clear<uint8_t[]>(0x100);

	// setup palette
	double weights_r[2]{};
	double weights_g[3]{};
	double weights_b[3]{};

	static const int resistances_r[2]  = { 4700, 2200 };
	static const int resistances_gb[3] = { 10000, 4700, 2200 };

	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_gb, weights_g,    3600,   0,
			3,  resistances_gb, weights_b,    3600,   0,
			2,  resistances_r,  weights_r,    3600,   0);

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2;

		/* red component */
		bit1 = BIT(i, 7);
		bit0 = BIT(i, 6);
		const int r = combine_weights(weights_r, bit0, bit1);

		/* green component */
		bit2 = BIT(i, 5);
		bit1 = BIT(i, 4);
		bit0 = BIT(i, 3);
		const int g = combine_weights(weights_g, bit0, bit1, bit2);

		/* blue component */
		bit2 = BIT(i, 2);
		bit1 = BIT(i, 1);
		bit0 = BIT(i, 0);
		const int b = combine_weights(weights_b, bit0, bit1, bit2);

		set_indirect_color(i, rgb_t(r, g, b));
	}

	save_item(NAME(m_status));
	save_pointer(NAME(m_ram), 0x800);
	save_pointer(NAME(m_palram), 0x100);
	save_item(NAME(m_param_offset_latch));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_color));
	save_item(NAME(m_mode));
	save_item(NAME(m_pix_xsize));
	save_item(NAME(m_pix_ysize));
	save_item(NAME(m_vregs));
	save_item(NAME(m_bk_color));
	save_item(NAME(m_vbank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb_vcu_device::device_reset()
{
	m_status = 1;

	for (int i = 0; i < 0x80000; i++)
	{
		write_byte(i, 0x0f);
	}
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

static inline uint32_t get_vram_addr(uint8_t x, uint8_t y, uint8_t layer, uint8_t bank)
{
	return x | (y << 8) | (layer << 16) | (bank << 18);
}

uint8_t mb_vcu_device::read_ram(offs_t offset)
{
	return m_ram[offset];
}

void mb_vcu_device::write_ram(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;
}

void mb_vcu_device::write_vregs(offs_t offset, uint8_t data)
{
	m_vregs[offset] = data;
}

/* latches RAM offset to send to params */
uint8_t mb_vcu_device::load_params(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_param_offset_latch = offset;

		m_xpos      = m_ram[m_param_offset_latch + 1] | (m_ram[m_param_offset_latch + 2] << 8);
		m_ypos      = m_ram[m_param_offset_latch + 3] | (m_ram[m_param_offset_latch + 4] << 8);
		m_color     = m_ram[m_param_offset_latch + 5] | (m_ram[m_param_offset_latch + 6] << 8);
		m_mode      = m_ram[m_param_offset_latch + 7];
		m_pix_xsize = m_ram[m_param_offset_latch + 8] + 1;
		m_pix_ysize = m_ram[m_param_offset_latch + 9] + 1;

		LOGPARAMS("[0] %02x ", m_ram[m_param_offset_latch]);
		LOGPARAMS("X: %04x ", m_xpos);
		LOGPARAMS("Y: %04x ", m_ypos);
		LOGPARAMS("C :%04x ", m_color);
		LOGPARAMS("M :%02x ", m_mode);
		LOGPARAMS("XS:%02x ", m_pix_xsize);
		LOGPARAMS("YS:%02x ", m_pix_ysize);
		LOGPARAMS("\n");
	}
	return 0; // open bus?
}

uint8_t mb_vcu_device::load_gfx(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		int bits = 0;

		LOGDRAW("%02x %02x\n", m_mode >> 2, m_mode & 3);

		// const uint8_t cur_layer = (m_mode & 0x3);
		const uint8_t cur_layer = BIT(m_mode, 1);
		const uint8_t opaque_pen = (cur_layer == 1);

		switch (m_mode >> 2)
		{
			case 0x00: // 4bpp
				for (int yi = 0; yi < m_pix_ysize; yi++)
				{
					for (int xi = 0; xi < m_pix_xsize; xi++)
					{
						const int dstx = (m_xpos + xi);
						const int dsty = (m_ypos + yi);

						if (dstx >= 0 && dsty >= 0 && dstx < 256 && dsty < 256)
						{
							const uint8_t dot = (m_memory.read_byte(((offset + (bits >> 3)) & 0x1fff) + 0x4000) >> (4 - (bits & 7))) & 0xf;

							if (dot != 0xf || opaque_pen)
								write_byte(get_vram_addr(dstx, dsty, cur_layer, m_vbank), dot);
						}
						bits += 4;
					}
				}
				break;

			case 0x02: // 1bpp
				for (int yi = 0; yi < m_pix_ysize; yi++)
				{
					for (int xi = 0; xi < m_pix_xsize; xi++)
					{
						const int dstx = (m_xpos + xi);
						const int dsty = (m_ypos + yi);

						if (dstx >= 0 && dsty >= 0 && dstx < 256 && dsty < 256)
						{
							const uint8_t dot = (m_memory.read_byte(((offset + (bits >> 3)) & 0x1fff) + 0x4000) >> (7 - (bits & 7))) & 1;
							const uint8_t pen = (m_color >> (dot << 2)) & 0xf;

							if (pen != 0xf || opaque_pen)
								write_byte(get_vram_addr(dstx, dsty, cur_layer, m_vbank), pen);
						}
						bits++;
					}
				}
				break;
			case 0x03: //2bpp
				for (int yi = 0; yi < m_pix_ysize; yi++)
				{
					for (int xi = 0; xi < m_pix_xsize; xi++)
					{
						const int dstx = (m_xpos + xi);
						const int dsty = (m_ypos + yi);

						if (dstx >= 0 && dsty >= 0 && dstx < 256 && dsty < 256)
						{
							const uint8_t dot = (m_memory.read_byte(((offset + (bits >> 3)) & 0x1fff) + 0x4000) >> (6 - (bits & 7))) & 3;
							const uint8_t pen = (m_color >> (dot << 2)) & 0xf;

							if (pen != 0xf || opaque_pen)
								write_byte(get_vram_addr(dstx, dsty, cur_layer, m_vbank), pen);
						}

						bits += 2;
					}
				}
				break;

			default:
				popmessage("Unsupported draw mode");
				break;
		}
	}
	return 0; // open bus?
}


/*
Read-Modify-Write operations

---0 -111 (0x07) write to i/o
---0 -011 (0x03) clear VRAM
---1 -011 (0x13) collision detection
*/
uint8_t mb_vcu_device::load_set_clr(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		//uint8_t dot;

		switch (m_mode)
		{
			case 0x13:
			{
				//int16_t srcx = m_ram[m_param_offset_latch + 1];
				//int16_t srcy = m_ram[m_param_offset_latch + 3];
				//uint16_t src_xsize = m_ram[m_param_offset_latch + 18] + 1;
				//uint16_t src_ysize = m_ram[m_param_offset_latch + 19] + 1;
				int collision_flag = 0;

				for (int yi = 0; yi < m_pix_ysize; yi++)
				{
					for (int xi = 0; xi < m_pix_xsize; xi++)
					{
						const int dstx = (m_xpos + xi);
						const int dsty = (m_ypos + yi);

						if (dstx >= 0 && dsty >= 0 && dstx < 256 && dsty < 256)
						{
							const uint8_t res = read_byte(get_vram_addr(dstx, dsty, 0, m_vbank));
							//uint8_t res2 = read_byte(srcx|srcy<<8|0<<16|(m_vbank)<<18);

							//LOG_CLEAR("%02x %02x\n",res,res2);

							// TODO: how it calculates the pen? Might use the commented out stuff and/or the offset somehow
							if (res == 5)
							{
								collision_flag++;
	//                          test++;
							}
						}

						//srcx++;
					}
					//srcy++;
				}

				// threshold for collision, necessary to avoid bogus collision hits
				// the typical test scenario is to shoot near the top left hatch for stage 1 then keep shooting,
				// at some point the top right hatch will bogusly detect a collision without this.
				// It's also unlikely that game tests 1x1 targets anyway, as the faster moving targets are quite too easy to hit that way.
				// TODO: likely it works differently (checks area?)
				if (collision_flag > 5)
					m_ram[m_param_offset_latch] |= 8;
				else
					m_ram[m_param_offset_latch] &= ~8;
				break;
			}

			case 0x03:
			{
				for (int yi = 0; yi < m_pix_ysize; yi++)
				{
					for (int xi = 0; xi < m_pix_xsize; xi++)
					{
						const int dstx = (m_xpos + xi);
						const int dsty = (m_ypos + yi);

						if (dstx >= 0 && dsty >= 0 && dstx < 256 && dsty < 256)
							write_byte(get_vram_addr(dstx, dsty, 0, m_vbank), 0xf);
					}
				}
				break;
			}

			case 0x07:
				for (int i = 0; i < m_pix_xsize; i++)
					write_io(i + (m_ypos << 8), m_ram[offset + i]);

				break;
		}
	}
	return 0; // open bus?
}

void mb_vcu_device::background_color_w(uint8_t data)
{
	m_bk_color = data;
	set_pen_indirect(0x100, m_bk_color);
}

uint8_t mb_vcu_device::status_r()
{
	/*
	---- ---x busy or vblank flag
	*/
	return m_status;
}

void mb_vcu_device::vbank_w(uint8_t data)
{
	m_vbank = BIT(data, 6);
}

void mb_vcu_device::vbank_clear_w(uint8_t data)
{
	vbank_w(data & 0x40);

	// setting vbank clears VRAM in the setted bank, applies to Great Guns only since it never ever access the RMW stuff
	for (int i = 0; i < 0x10000; i++)
	{
		write_byte(i | 0x00000 | (m_vbank << 18), 0x0f);
		write_byte(i | 0x10000 | (m_vbank << 18), 0x0f);
	}
}

//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

uint32_t mb_vcu_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(pen(0x100), cliprect);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const uint8_t dot[2] = {read_byte(get_vram_addr(x, y, 0, m_vbank ^ 1)), read_byte(get_vram_addr(x, y, 1, m_vbank ^ 1))};
			uint8_t dot_sel = uint8_t(~0);

			if (dot[0] != 0xf)
				dot_sel = 0;
			else/* if (dot[1] != 0xf) */
				dot_sel = 1;

			if (dot_sel <= 1)
				dst[x] = pen(dot[dot_sel] | (m_vregs[1] << 4));
		}
	}

	return 0;
}

void mb_vcu_device::screen_eof()
{
	#if 0
	for (int i = 0; i < 0x10000; i++)
	{
		write_byte(i | 0x00000 | m_vbank << 18, 0x0f);
		//write_byte(i | 0x10000 | m_vbank << 18, 0x0f);
		//write_byte(i | 0x30000 | m_vbank << 18, 0x0f);
	}
	#endif
}
