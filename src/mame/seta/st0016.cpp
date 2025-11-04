// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, David Haywood
/* ST0016 - CPU (z80) + Sound + Video */

#include "emu.h"
#include "st0016.h"
#include "emupal.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(ST0016_CPU, st0016_cpu_device, "st0016_cpu", "ST0016")

void st0016_cpu_device::cpu_internal_map(address_map &map)
{
	// ROM, Fixed area
	map(0x0000, 0x7fff).lr8(NAME([this](offs_t offset) -> u8 { return m_extrom_cache.read_byte(m_fixedrom_offset + (offset & 0x7fff)); }));
	// ROM, Bankswitched area
	map(0x8000, 0xbfff).lr8(NAME([this](offs_t offset) -> u8 { return m_extrom_cache.read_byte((m_rom_bank << 14) | (offset & 0x3fff)); }));
	map(0xc000, 0xcfff).r(FUNC(st0016_cpu_device::sprite_ram_r<0>)).w(FUNC(st0016_cpu_device::sprite_ram_w<0>));
	map(0xd000, 0xdfff).r(FUNC(st0016_cpu_device::sprite_ram_r<1>)).w(FUNC(st0016_cpu_device::sprite_ram_w<1>));
	//map(0xe000, 0xe8ff).ram(); External area, commonly RAM
	map(0xe900, 0xe9ff).rw("stsnd", FUNC(st0016_device::snd_r), FUNC(st0016_device::snd_w)); // sound regs 8 x $20 bytes, see notes
	map(0xea00, 0xebff).r(FUNC(st0016_cpu_device::palette_ram_r)).w(FUNC(st0016_cpu_device::palette_ram_w));
	map(0xec00, 0xec1f).r(FUNC(st0016_cpu_device::charam_bank_r)).w(FUNC(st0016_cpu_device::charam_bank_w));
	//map(0xf000, 0xffff).ram(); External area, commonly RAM
}


void st0016_cpu_device::cpu_internal_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xbf).r(FUNC(st0016_cpu_device::vregs_r)).w(FUNC(st0016_cpu_device::vregs_w)); // video/crt regs ?
	map(0xe1, 0xe1).w(FUNC(st0016_cpu_device::rom_bank_w));
	map(0xe2, 0xe2).w(FUNC(st0016_cpu_device::sprite_bank_w));
	map(0xe3, 0xe4).w(FUNC(st0016_cpu_device::character_bank_w));
	map(0xe5, 0xe5).w(FUNC(st0016_cpu_device::palette_bank_w));
	// map(0xe7, 0xe7).w(FUNC(st0016_cpu_device::rom_bank_w)); ROM bank mirror? (only for seta/srmp5.cpp?)
	map(0xf0, 0xf0).r(FUNC(st0016_cpu_device::dma_r));
}


void st0016_cpu_device::charam_map(address_map &map)
{
	map(0x000000, 0x1fffff).rw(FUNC(st0016_cpu_device::character_ram_r), FUNC(st0016_cpu_device::character_ram_w));
}

// note: a lot of bits are left uninitialized by the games, the default values are uncertain

st0016_cpu_device::st0016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, ST0016_CPU, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, nullptr, "palette")
	, device_video_interface(mconfig, *this, false)
	, device_mixer_interface(mconfig, *this)
	, m_io_space_config("io", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(st0016_cpu_device::cpu_internal_io_map), this))
	, m_space_config("regs", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(st0016_cpu_device::cpu_internal_map), this))
	, m_charam_space_config("charam", ENDIANNESS_LITTLE, 8, 21, 0, address_map_constructor(FUNC(st0016_cpu_device::charam_map), this))
	, m_extrom_space_config("extrom", ENDIANNESS_LITTLE, 8, 22)
	, m_rom(*this, DEVICE_SELF)
	, m_spriteram(*this, "spriteram", 0x10000, ENDIANNESS_LITTLE)
	, m_charram(*this, "charam", 0x200000, ENDIANNESS_LITTLE)
	, m_paletteram(*this, "paletteram", 0x800, ENDIANNESS_LITTLE)
	, m_fixedrom_offset(0)
	, m_game_flag(-1)
	, m_spr_bank{0}
	, m_pal_bank(0)
	, m_char_bank(0)
	, m_rom_bank(0)
	, m_spr_dx(0)
	, m_spr_dy(0)
	, m_vregs{0}
	, m_ramgfx(0)
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector st0016_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config),
		std::make_pair(AS_IO,      &m_io_space_config),
		std::make_pair(AS_CHARAM,  &m_charam_space_config),
		std::make_pair(AS_EXTROM,  &m_extrom_space_config)
	};
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void st0016_cpu_device::device_start()
{
	z80_device::device_start();
	startup();
	space(AS_CHARAM).specific(m_charam_space);
	space(AS_EXTROM).cache(m_extrom_cache);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void st0016_cpu_device::device_reset()
{
	z80_device::device_reset();
	if (has_screen())
	{
		// TODO: move these into driver file?
		switch (m_game_flag & 0x3f)
		{
			case 0: // renju kizoku
				screen().set_visible_area(0, 40*8-1, 0, 30*8-1);
				m_spr_dx = 0;
				m_spr_dy = 0;
			break;

			case 1: // neratte chu!
				screen().set_visible_area(8,41*8-1,0,30*8-1);
				m_spr_dx = 0;
				m_spr_dy = 8;
			break;

			case 2: // Crown Poker
				screen().set_visible_area(8,42*8-1,0,29*8-1);
				m_spr_dx = 4;
				m_spr_dy = 4;
			break;

			case 3: // Dream Crown
				screen().set_visible_area(8,42*8-1,0,30*8-1);
				m_spr_dx = 4;
			break;

			case 4: // mayjinsen 1&2
				screen().set_visible_area(0,32*8-1,0,28*8-1);
			break;

			case 10:
				screen().set_visible_area(0,383,0,255);
			break;

			case 11:
				screen().set_visible_area(0,383,0,383);
			break;

		}
	}
}

// CPU interface
void st0016_cpu_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, "palette").set_entries(16 * 16 * 4 + 1);

	st0016_device &stsnd(ST0016(config, "stsnd", DERIVED_CLOCK(1,1)));
	stsnd.set_addrmap(0, &st0016_cpu_device::charam_map);
	stsnd.add_route(0, *this, 1.0, 0);
	stsnd.add_route(1, *this, 1.0, 1);
}


static const gfx_layout charlayout =
{
	8,8,
	0x10000,
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

void st0016_cpu_device::sprite_bank_w(u8 data)
{
/*
    76543210
        xxxx - spriteram  bank1
    xxxx     - spriteram  bank2
*/
	m_spr_bank[0] = data & SPR_BANK_MASK;
	m_spr_bank[1] = (data >> 4) & SPR_BANK_MASK;
}

void st0016_cpu_device::palette_bank_w(u8 data)
{
/*
    76543210
          xx - palram  bank
    xxxxxx   - unknown/unused
*/
	m_pal_bank = data & PAL_BANK_MASK;
}

void st0016_cpu_device::character_bank_w(offs_t offset, u8 data)
{
/*
    fedcba9876543210
    xxxxxxxxxxxxxxxx - character (bank )
*/

	if (offset & 1)
		m_char_bank = (m_char_bank & 0xff) | (data << 8);
	else
		m_char_bank = (m_char_bank & 0xff00) | data;

	m_char_bank &= CHAR_BANK_MASK;
}

void st0016_cpu_device::rom_bank_w(u8 data)
{
	m_rom_bank = data;
}

template <unsigned Which>
u8 st0016_cpu_device::sprite_ram_r(offs_t offset)
{
	return m_spriteram[SPR_BANK_SIZE * m_spr_bank[Which] + offset];
}

template <unsigned Which>
void st0016_cpu_device::sprite_ram_w(offs_t offset, u8 data)
{
	m_spriteram[SPR_BANK_SIZE * m_spr_bank[Which] + offset] = data;
}

u8 st0016_cpu_device::palette_ram_r(offs_t offset)
{
	return m_paletteram[PAL_BANK_SIZE * m_pal_bank + offset];
}

void st0016_cpu_device::palette_ram_w(offs_t offset, u8 data)
{
	offset += PAL_BANK_SIZE * m_pal_bank;
	const pen_t color = offset >> 1;
	m_paletteram[offset] = data;
	const u16 val = m_paletteram[offset & ~1] | (m_paletteram[offset | 1] << 8);
	if (!color)
		palette().set_pen_color(UNUSED_PEN, pal5bit(val >> 0), pal5bit(val >> 5), pal5bit(val >> 10)); // same as color 0 - bg ?
	palette().set_pen_color(color, pal5bit(val >> 0), pal5bit(val >> 5), pal5bit(val >> 10));
}

u8 st0016_cpu_device::charam_bank_r(offs_t offset)
{
	return m_charam_space.read_byte(CHAR_BANK_SIZE * m_char_bank + offset);
}

void st0016_cpu_device::charam_bank_w(offs_t offset, u8 data)
{
	m_charam_space.write_byte(CHAR_BANK_SIZE * m_char_bank + offset, data);
}

u8 st0016_cpu_device::character_ram_r(offs_t offset)
{
	return m_charram[offset];
}

void st0016_cpu_device::character_ram_w(offs_t offset, u8 data)
{
	m_charram[offset] = data;
	gfx(m_ramgfx)->mark_dirty(offset >> 5);
}

u8 st0016_cpu_device::vregs_r(offs_t offset)
{
	/*
	    $0, $1 = max scanline(including vblank)/timer? ($3e7)

	    $8-$3F = bg tilemaps  (8 bytes each) :
	               0 - ? = usually 0/20/ba*
	               1 - 0 = disabled , !zero = address of tilemap in spriteram /$1000  (for example: 3 -> tilemap at $3000 )
	               2 - ? = usually ff/1f/af*
	               3 - priority ? = 0 - under sprites , $ff - over sprites \
	               4 - ? = $7f/$ff
	               5 - ? = $29/$20 (29 when tilemap must be drawn over sprites . maybe this is real priority ?)
	               6 - ? = 0
	               7 - ? =$20/$10/$12*

	    $40-$5F = scroll registers , X.w, Y.w
	*/

	switch (offset)
	{
		case 0:
		case 1:
			return machine().rand();
	}

	return m_vregs[offset];
}

u8 st0016_cpu_device::dma_r()
{
	// bits 0 and 1 = 0 -> DMA transfer complete
	if (ismacs())
		return 0;
	else
		return 0;
}


void st0016_cpu_device::vregs_w(offs_t offset, u8 data)
{
	/*

	   I/O ports:

	    $60 \
	    $61 - H sync start?
	    $62 \
	    $63 - H visible start?
	    $64 \
	    $65 - H visible end >> 1?
	    $66 \
	    $67 - H total
	    $68 \
	    $69 - V sync start?
	    $6a \
	    $6b - V visible start?
	    $6c \
	    $6d - V visible end?
	    $6e \
	    $6f - V total

	    $74 x--- ---- global flip screen
	        -xx- ---- individual flip screen x/y
	        i.e. Mayjinsen sets 0x80, other ST0016 games 0x60.
	        TODO: Might also be paired with $70 & $75 (set up by Mayjinsen).

	    $a0 \
	    $a1 - source address >> 1
	    $a2 /

	    $a3 \
	    $a4 - destination address >> 1  (inside character ram)
	    $a5 /

	    $a6 \
	    &a7 - (length inbytes - 1 ) >> 1

	    $a8 - 76543210
	          ??faaaaa

	          a - most sign. bits of length
	          f - DMA start latch

	*/

	m_vregs[offset] = data;
	if (offset == 0xa8 && (data & 0x20))
	{
		u32 srcadr = (m_vregs[0xa0] | (m_vregs[0xa1] << 8) | (m_vregs[0xa2] << 16)) << 1;
		u32 dstadr = (m_vregs[0xa3] | (m_vregs[0xa4] << 8) | (m_vregs[0xa5] << 16)) << 1;
		u32 length = ((m_vregs[0xa6] | (m_vregs[0xa7] << 8) | ((m_vregs[0xa8] & 0x1f) << 16)) + 1) << 1;

		const u32 srclen = std::min<u32>(0x400000, m_rom->bytes());

		while (length > 0)
		{
			if (srcadr < srclen && (dstadr < MAX_CHAR_BANK*CHAR_BANK_SIZE))
			{
				m_charam_space.write_byte(dstadr, m_extrom_cache.read_byte(srcadr));
				srcadr++;
				dstadr++;
				length--;
			}
			else
			{
				// samples ? sound dma ?
				// speaglsht:  unknown DMA copy : src - 2B6740, dst - 4400, len - 1E400
				logerror("%s unknown DMA copy : src - %X, dst - %X, len - %X\n", machine().describe_context(), srcadr, dstadr, length);
				break;
			}
		}
	}
}

void st0016_cpu_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/*
	object ram :

	each entry is 8 bytes:

	76543210 (bit)
	0 llllllll
	1 ---gSSSl
	2 oooooooo
	3 fooooooo
	4 xxxxxxxx
	5 ----XXxx
	6 yyyyyyyy
	7 ----XYyy

	1 - always(?) set
	S - scroll index ? (ports $40-$60, X(word),Y(word) )
	l - sublist length (8 byte entries -1)
	o - sublist offset (*8 to get real offset)
	f - end of list  flag
	x,y - sprite coords

	(complete guess)
	g - use tile sizes specified in list (global / fixed ones if not set?) (gostop)
	X = global X size?
	Y = global Y size?

	sublist format (8 bytes/entry):

	76543210
	0 cccccccc
	1 cccccccc
	2 --kkkkkk
	3 QW------
	4 xxxxxxxx
	5 -B--XX-x
	6 yyyyyyyy
	7 ----YY-y

	c - character code
	k - palette
	QW - flips
	x,y - coords
	XX,YY - size (1<<size)
	B - merge pixel data with prevoius one (8bpp mode - neratte: seta logo and title screen)

	*/

	gfx_element *gfx = this->gfx(m_ramgfx);

	for (int i = 0; i < SPR_BANK_SIZE * MAX_SPR_BANK; i += 8)
	{
		int x = m_spriteram[i + 4] + ((m_spriteram[i + 5] & 3) << 8);
		int y = m_spriteram[i + 6] + ((m_spriteram[i + 7] & 3) << 8);

		const bool use_sizes = BIT(m_spriteram[i + 1], 4);
		const int globalw = (m_spriteram[i + 5] & 0x0c) >> 2;
		const int globalh = (m_spriteram[i + 7] & 0x0c) >> 2;

		int scrollx = (m_vregs[(((m_spriteram[i + 1] & 0x0f) >> 1) << 2) + 0x40] + 256 * m_vregs[(((m_spriteram[i + 1] & 0x0f) >> 1) << 2) + 1 + 0x40]) & 0x3ff;
		int scrolly = (m_vregs[(((m_spriteram[i + 1] & 0x0f) >> 1) << 2) + 2 + 0x40] + 256 * m_vregs[(((m_spriteram[i + 1] & 0x0f) >> 1) << 2) + 3 + 0x40]) & 0x3ff;

		if (!ismacs())
		{
			if (x & 0x200) x -= 0x400; //sign
			if (y & 0x200) y -= 0x400;

			if (scrollx & 0x200) scrollx -= 0x400; //sign
			if (scrolly & 0x200) scrolly -= 0x400;
		}

		if (ismacs1())
		{
			if (x & 0x200) x -= 0x400; //sign
			if (y & 0x200) y -= 0x2b0;//0x400;

			if (scrollx & 0x200) scrollx -= 0x400; //sign
			if (scrolly & 0x200) scrolly -= 0x400;
		}

		x += scrollx;
		y += scrolly;

		if (ismacs())
		{
			y += 0x20;
		}

		if (BIT(m_spriteram[i + 3], 7)) // end of list
			break;

		int offset = m_spriteram[i + 2] + 256 * (m_spriteram[i + 3]);
		offset <<= 3;

		const int length = m_spriteram[i + 0] + 1 + 256 * (m_spriteram[i + 1] & 1);

		//const int plx = (m_spriteram[i + 5] >> 2) & 0x3;
		//const int ply = (m_spriteram[i + 7] >> 2) & 0x3;

		if (offset < SPR_BANK_SIZE * MAX_SPR_BANK)
		{
			for (int j = 0; j < length; j++)
			{
				const int code = m_spriteram[offset] + 256 * m_spriteram[offset + 1];
				int sx = m_spriteram[offset + 4] + ((m_spriteram[offset + 5] & 1) << 8);
				int sy = m_spriteram[offset + 6] + ((m_spriteram[offset + 7] & 1) << 8);

				if (ismacs() && !ismacs1())
				{
					if (sy & 0x100) sy -= 0x200; //yuka & yujan
				}

				if (ismacs())
				{
					sy = 0xe0 - sy;
				}

				sx += x;
				sy += y;
				const int color = m_spriteram[offset + 2] & 0x3f;

				int lx, ly;
				if (use_sizes)
				{
					lx = (m_spriteram[offset + 5] >> 2) & 3;
					ly = (m_spriteram[offset + 7] >> 2) & 3;
				}
				else
				{
					lx = globalw;
					ly = globalh;

				}

				/*
				if (plx | ply) //parent
				{
				    lx = plx;
				    ly = ply;
				}
				*/

				const bool flipx = BIT(m_spriteram[offset + 3], 7);
				const bool flipy = BIT(m_spriteram[offset + 3], 6);

				if (ismacs())
					sy -= (1 << ly) * 8;

				{
					int i0 = 0;
					for (int x0 = (flipx ? ((1 << lx) - 1) : 0); x0 != (flipx ? -1 : (1 << lx)); x0 += (flipx ? -1 : 1))
					{
						const int xpos = sx + x0 * 8 + m_spr_dx;
						for (int y0 = (flipy ? ((1 << ly) - 1) : 0); y0 != (flipy ? -1 : (1 << ly)); y0 += (flipy ? -1 : 1))
						{
							// custom draw
							const int ypos = sy + y0 * 8 + m_spr_dy;
							const int tileno = (code + i0++) & CHAR_BANK_MASK;

							int gfxoffs = 0;
							const u8 *const srcgfx = gfx->get_data(tileno);

							for (int yloop = 0; yloop < 8; yloop++)
							{
								u16 drawypos;

								if (!flipy) { drawypos = ypos + yloop; }
								else { drawypos = (ypos + 8 - 1) - yloop; }
								u16 *const destline = &bitmap.pix(drawypos);

								for (int xloop = 0; xloop < 8; xloop++)
								{
									u16 drawxpos;
									const int pixdata = srcgfx[gfxoffs];

									if (!flipx) { drawxpos = xpos + xloop; }
									else { drawxpos = (xpos + 8 - 1) - xloop; }

									if (drawxpos > cliprect.max_x)
										drawxpos -= 512; // wrap around

									if (cliprect.contains(drawxpos, drawypos))
									{
										if (m_spriteram[offset + 5] & 0x40)
										{
											destline[drawxpos] = (destline[drawxpos] | pixdata << 4) & 0x3ff;
										}
										else
										{
											if (ismacs2())
											{
												if (pixdata)//|| destline[drawxpos]==UNUSED_PEN)
												{
													destline[drawxpos] = pixdata + (color * 16);
												}
											}
											else
											{
												if (pixdata || destline[drawxpos] == UNUSED_PEN)
												{
													destline[drawxpos] = pixdata + (color * 16);
												}
											}
										}
									}
									gfxoffs++;
								}
							}
						}
					}
				}
				offset += 8;
				if (offset >= SPR_BANK_SIZE * MAX_SPR_BANK)
					break;
			}
		}
	}
}


void st0016_cpu_device::save_init()
{
	save_item(NAME(m_spr_bank));
	save_item(NAME(m_pal_bank));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_vregs));
}


void st0016_cpu_device::startup()
{
	u8 gfx_index = 0;

	// find first empty slot to decode gfx
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (gfx(gfx_index) == nullptr)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// create the char set (gfx will then be updated dynamically from RAM)
	set_gfx(gfx_index, std::make_unique<gfx_element>(&palette(), charlayout, m_charram.target(), 0, 0x40, 0));
	m_ramgfx = gfx_index;

	m_spr_dx = 0;
	m_spr_dy = 0;

	save_init();
}


void st0016_cpu_device::draw_bgmap(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	gfx_element *gfx = this->gfx(m_ramgfx);
	//for (j = 0x40 - 8; j >= 0; j -= 8)
	for (int j = 0; j < 0x40; j += 8)
	{
		if (m_vregs[j + 1] && ((priority && (m_vregs[j + 3] == 0xff)) || ((!priority) && (m_vregs[j + 3] != 0xff))))
		{
			int i = m_vregs[j + 1] * 0x1000;

			for (int x = 0; x < 32 * 2; x++)
			{
				for (int y = 0; y < 8 * 4; y++)
				{
					const int code = m_spriteram[i] + 256 * m_spriteram[i + 1];
					const int color = m_spriteram[i + 2] & 0x3f;

					const bool flipx = BIT(m_spriteram[i + 3], 7); // crownpkr test mode doesn't seem to agree with this
					const bool flipy = BIT(m_spriteram[i + 3], 6); // "

					if (priority)
					{
						gfx->transpen(bitmap, cliprect,
							code,
							color,
							flipx, flipy,
							x * 8 + m_spr_dx, y * 8 + m_spr_dy, 0);
					}
					else
					{
						int ypos = y * 8 + m_spr_dy;// + ((m_vregs[j + 2] == 0xaf) ? 0x50 : 0); //hack for mayjinsen title screen
						int xpos = x * 8 + m_spr_dx;
						int gfxoffs = 0;
						const u8 *const srcgfx = gfx->get_data(code);

						for (int yloop = 0; yloop < 8; yloop++)
						{
							u16 drawypos;

							if (!flipy) { drawypos = ypos + yloop; }
							else { drawypos = (ypos + 8 - 1) - yloop; }
							u16 *const destline = &bitmap.pix(drawypos);
							//u16 *destline = &bitmap.pix(drawypos ^ 0x07); // hack for dcrown test mode

							for (int xloop = 0; xloop < 8; xloop++)
							{
								u16 drawxpos;
								const int pixdata = srcgfx[gfxoffs];

								if (!flipx) { drawxpos = xpos + xloop; }
								else { drawxpos = (xpos + 8 - 1) - xloop; }

								if (drawxpos > cliprect.max_x)
									drawxpos -= 512; // wrap around

								if (cliprect.contains(drawxpos, drawypos))
								{
									if (m_vregs[j + 7] == 0x12)
										destline[drawxpos] = (destline[drawxpos] | (pixdata << 4)) & 0x3ff;
									else
									{
										if (ismacs2())
										{
											if (pixdata)// || destline[drawxpos]==UNUSED_PEN)
											{
												destline[drawxpos] = pixdata + (color * 16);
											}
										}
										else
										{
											if (pixdata || destline[drawxpos] == UNUSED_PEN)
											{
												destline[drawxpos] = pixdata + (color * 16);
											}
										}
									}
								}
								gfxoffs++;
							}
						}
					}
					i += 4;
				}
			}
		}
	}
}


void st0016_cpu_device::draw_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_bgmap(bitmap, cliprect, 0);
	draw_sprites(bitmap, cliprect);
	draw_bgmap(bitmap, cliprect, 1);
}

u32 st0016_cpu_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once(KEYCODE_Z))
	{
		int h, j;
		FILE *p = fopen("vram.bin", "wb");
		fwrite(&m_spriteram[0], 1, 0x1000 * MAX_SPR_BANK, p);
		fclose(p);

		p = fopen("vram.txt","wt");
		for (h = 0; h < 0xc0; h++)
			fprintf(p,"VREG %.4x - %.4x\n",h,m_vregs[h]);
		for (h = 0; h < 0x1000 * MAX_SPR_BANK; h += 8)
		{
			fprintf(p, "%.4x - %.4x - ", h, h >> 3);
			for (j = 0; j < 8; j++)
				fprintf(p, "%.2x ", m_spriteram[h + j]);
			fprintf(p, "\n");
		}
		fclose(p);
	}
#endif

	bitmap.fill(UNUSED_PEN, cliprect);
	draw_screen(screen, bitmap, cliprect);
	return 0;
}
