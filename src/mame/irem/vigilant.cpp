// license:BSD-3-Clause
// copyright-holders: Mike Balfour

/***************************************************************************

  Vigilante

If you have any questions about how this driver works, don't hesitate to
ask.  - Mike Balfour (mab22@po.cwru.edu)

- Video timings were measured using a logic analyzer by atrac17 on an original PCB.
  The results were reviewed by Jose Tejada (jotego)

TS 2004.12.26.:
- Buccaneers - different sound hw (YM2203x2)
    (to enter test mode press any button during memory test)

Buccaneers has a 5.6888 Mhz and a 18.432 Mhz OSC

system11 2015.05.08:
Irem board numbers for Vigilante sets:
Top board - M75-A-B (up to Rev A), M75-A-C (Rev B onwards)
Bottom board - M75-B-A (all versions regardless of mask ROM/EPROM)

****************************************************************************

Roberto Fresca 2022.04.23:
Added Bowmen, from Ten-Level.  A very rare Spanish game.

The game uses derivative hardware.

TODO:

- Add cocktail flipping.

***************************************************************************/

#include "emu.h"

#include "iremipt.h"
#include "m72_a.h"

#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/rstbuf.h"
#include "machine/timer.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class vigilant_state : public driver_device
{
public:
	vigilant_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_8(*this, "paletteram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_rombank(*this, "rombank")
	{ }

	void vigilant(machine_config &config);
	void kikcubic(machine_config &config);
	void buccanrs(machine_config &config);
	void bowmen(machine_config &config);

	void init_bowmen();

protected:
	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

	void vigilant_map(address_map &map) ATTR_COLD;
	void vigilant_io_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_soundcpu;
	required_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	required_memory_bank m_rombank;

	uint8_t m_horiz_scroll_low = 0;
	uint16_t m_horiz_scroll_high = 0;
	uint8_t m_rear_horiz_scroll_low = 0;
	uint16_t m_rear_horiz_scroll_high = 0;
	uint8_t m_rear_color = 0;
	uint8_t m_rear_disable = 0;
	uint8_t m_rear_refresh = 0;
	uint8_t m_rear_pages = 4;
	std::unique_ptr<bitmap_ind16> m_bg_bitmap;

	// common
	void bank_select_w(uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(sound_nmi);

	// vigilant and buccanrs
	void vigilant_out2_w(uint8_t data);
	void vigilant_horiz_scroll_w(offs_t offset, uint8_t data);
	void vigilant_rear_horiz_scroll_w(offs_t offset, uint8_t data);
	void vigilant_rear_color_w(uint8_t data);
	void bowmen_rear_horiz_scroll_w(offs_t offset, uint8_t data);
	void bowmen_rear_color_w(uint8_t data);

	// kikcubic
	void kikcubic_coin_w(uint8_t data);

	uint32_t screen_update_vigilant(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kikcubic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bowmen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_background();
	void update_background_bowmen();
	void draw_foreground(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque);
	void draw_foreground_bowmen(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background_bowmen(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void vigilant_postload();

	void buccanrs_sound_io_map(address_map &map) ATTR_COLD;
	void kikcubic_io_map(address_map &map) ATTR_COLD;
	void kikcubic_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void bowmen_io_map(address_map &map) ATTR_COLD;
};

class captainx_state : public vigilant_state
{
public:
	captainx_state(const machine_config &mconfig, device_type type, const char *tag) :
		vigilant_state(mconfig, type, tag),
		m_mcu(*this, "mcu"),
		m_maincpu_region(*this, "maincpu")
	{}

	void captainx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68705r_device> m_mcu;
	required_region_ptr<uint8_t> m_maincpu_region;

	uint8_t m_decryption_ram[0x800];

	uint8_t m_mcu_porta = 0;
	uint8_t m_mcu_portb = 0;
	uint8_t m_mcu_portc = 0;

	void mcu_porta_w(u8 data);
	void mcu_portb_w(u8 data);
	void mcu_portc_w(u8 data);

	void captainx_map(address_map &map) ATTR_COLD;

	uint8_t rom_r(offs_t offset);
};


static const rectangle bottomvisiblearea(16*8, 48*8-1, 6*8, 32*8-1);
static const rectangle bowmenvisiblearea(16*8, 48*8-1, 0*8, 32*8-1);


void vigilant_state::video_start()
{
	m_bg_bitmap = std::make_unique<bitmap_ind16>(512 * m_rear_pages, 256);

	save_item(NAME(m_horiz_scroll_low));
	save_item(NAME(m_horiz_scroll_high));
	save_item(NAME(m_rear_horiz_scroll_low));
	save_item(NAME(m_rear_horiz_scroll_high));
	save_item(NAME(m_rear_color));
	save_item(NAME(m_rear_disable));

	m_rear_refresh = 1;
	machine().save().register_postload(save_prepost_delegate(FUNC(vigilant_state::vigilant_postload), this));
}

void vigilant_state::vigilant_postload()
{
	m_rear_refresh = 1;
}

void vigilant_state::video_reset()
{
	m_horiz_scroll_low = 0;
	m_horiz_scroll_high = 0;
	m_rear_horiz_scroll_low = 0;
	m_rear_horiz_scroll_high = 0;
	m_rear_color = 0;
	m_rear_disable = 1;
}


/***************************************************************************
 update_background

 There are three background ROMs, each one contains a 512x256 picture.
 **************************************************************************/
void vigilant_state::update_background()
{
	int charcode = 0;

	// There are only three background ROMs (4 on Buccaneers!)
	for (int page = 0; page < 4; page++)
	{
		for (int row = 0; row < 256; row++)
		{
			for (int col = 0; col < 512; col += 32)
			{
				m_gfxdecode->gfx(2)->opaque(*m_bg_bitmap,
						m_bg_bitmap->cliprect(),
						charcode,
						row < 128 ? 0 : 1,
						0, 0,
						512 * page + col, row);
				charcode++;
			}
		}
	}
}

/***************************************************************************
 update_background

 There are three background ROMs, each one contains a 512x256 picture.
 **************************************************************************/
void vigilant_state::update_background_bowmen()
{
	int charcode = 0;

	// There are 16 backgrounds
	for (int page = 0; page < 16; page++)
	{
		for (int row = 0; row < 256; row++)
		{
			for (int col = 0; col < 512; col += 32)
			{
				m_gfxdecode->gfx(2)->opaque(*m_bg_bitmap,
						m_bg_bitmap->cliprect(),
						charcode,
						0,
						0, 0,
						512 * page + col, row);
				charcode++;
			}
		}
	}
}

/***************************************************************************
 paletteram_w

 There are two palette chips, each one is labelled "KNA91H014".  One is
 used for the sprites, one is used for the two background layers.

 The chip has three enables (!CS, !E0, !E1), R/W pins, A0-A7 as input,
 'L' and 'H' inputs, and D0-D4 as input.  'L' and 'H' are used to bank
 into Red, Green, and Blue memory.  There are only 5 bits of memory for
 each byte, and a total of 256*3 bytes memory per chip.

 There are additionally two sets of D0-D7 inputs per chip labelled 'A'
 and 'B'.  There is also an 'S' pin to select between the two input sets.
 These are used to index a color triplet of RGB.  The triplet is read
 from RAM, and output to R0-R4, G0-G4, and B0-B4.
 **************************************************************************/
void vigilant_state::paletteram_w(offs_t offset, uint8_t data)
{
	m_generic_paletteram_8[offset] = data;

	int const bank = offset & 0x400;
	offset &= 0xff;

	int const r = (m_generic_paletteram_8[bank + offset + 0x000] << 3) & 0xff;
	int const g = (m_generic_paletteram_8[bank + offset + 0x100] << 3) & 0xff;
	int const b = (m_generic_paletteram_8[bank + offset + 0x200] << 3) & 0xff;

	m_palette->set_pen_color((bank >> 2) + offset, rgb_t(r, g, b));
}



/***************************************************************************
 vigilant_horiz_scroll_w

 horiz_scroll_low  = HSPL, an 8-bit register
 horiz_scroll_high = HSPH, a 1-bit register
 **************************************************************************/
void vigilant_state::vigilant_horiz_scroll_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		m_horiz_scroll_low = data;
	else
		m_horiz_scroll_high = data * 256;
}

/***************************************************************************
 vigilant_rear_horiz_scroll_w

 rear_horiz_scroll_low  = RHSPL, an 8-bit register
 rear_horiz_scroll_high = RHSPH, an 8-bit register but only 3 bits are saved
***************************************************************************/
void vigilant_state::vigilant_rear_horiz_scroll_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		m_rear_horiz_scroll_low = data;
	else
		m_rear_horiz_scroll_high = (data & 0x07) * 256;
}

/***************************************************************************
 vigilant_rear_color_w

 This is an 8-bit register labelled RCOD.
 D6 is hooked to !ROME (rear_disable)
 D3 = RCC2 (rear color bit 2)
 D2 = RCC1 (rear color bit 1)
 D0 = RCC0 (rear color bit 0)

 I know it looks odd, but D1, D4, D5, and D7 are empty.

 What makes this extremely odd is that RCC is supposed to hook up to the
 palette.  However, the top four bits of the palette inputs are labelled:
 "RCC3", "RCC2", "V256E", "RCC0".  Methinks there's a typo.
 **************************************************************************/
void vigilant_state::vigilant_rear_color_w(uint8_t data)
{
	m_rear_disable = data & 0x40;
	m_rear_color = data & 0x0d;
}

/***************************************************************************
 bowmen_rear_horiz_scroll_w

 rear_horiz_scroll_low  = RHSPL, an 8-bit register
 rear_horiz_scroll_high = RHSPH, an 8-bit register
***************************************************************************/
void vigilant_state::bowmen_rear_horiz_scroll_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		m_rear_horiz_scroll_low = data;
	else
		m_rear_horiz_scroll_high = (data & 0x1f) * 256;
}

/***************************************************************************
 bowmen_rear_color_w

 This is an 8-bit register labelled RCOD.
 D6 is hooked to !ROME (rear_disable)
 D3 = RCC2 (rear color bit 2)
 D2 = RCC1 (rear color bit 1)
 D0 = RCC0 (rear color bit 0)

 I know it looks odd, but D1, D4, D5, and D7 are empty.

 What makes this extremely odd is that RCC is supposed to hook up to the
 palette.  However, the top four bits of the palette inputs are labelled:
 "RCC3", "RCC2", "V256E", "RCC0".  Methinks there's a typo.
 **************************************************************************/
void vigilant_state::bowmen_rear_color_w(uint8_t data)
{
	m_rear_disable = data & 0x40;
	m_rear_color = (data & 0x0f);
}


/***************************************************************************
 draw_foreground

 ???
 **************************************************************************/

void vigilant_state::draw_foreground(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque)
{
	int const scroll = -(m_horiz_scroll_low + m_horiz_scroll_high);


	for (int offs = 0; offs < 0x1000; offs += 2)
	{
		int const sy = 8 * ((offs / 2) / 64);
		int sx = 8 * ((offs / 2) % 64);
		int const attributes = m_videoram[offs + 1];
		int const color = attributes & 0x0f;
		int const tile_number = m_videoram[offs] | ((attributes & 0xf0) << 4);

		if (priority)    // foreground
		{
			if ((color & 0x0c) == 0x0c)  // mask sprites
			{
				if (sy >= 48)
				{
					sx = (sx + scroll) & 0x1ff;

					m_gfxdecode->gfx(0)->transmask(bitmap, bottomvisiblearea,
							tile_number,
							color,
							0, 0,
							sx, sy, 0x00ff);
				}
			}
		}
		else     // background
		{
			if (sy >= 48)
				sx = (sx + scroll) & 0x1ff;

			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					tile_number,
					color,
					0, 0,
					sx, sy,
					(opaque || color >= 4) ? -1 : 0);
		}
	}
}



void vigilant_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const scrollx = 0x17a + 16 * 8 - (m_rear_horiz_scroll_low + m_rear_horiz_scroll_high);


	if (m_rear_refresh)
	{
		update_background();
		m_rear_refresh = 0;
	}

	copyscrollbitmap(bitmap, *m_bg_bitmap, 1, &scrollx, 0, nullptr, bottomvisiblearea);
}


void vigilant_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 8)
	{
		int code = m_spriteram[offs + 4] | ((m_spriteram[offs + 5] & 0x0f) << 8);
		int const color = m_spriteram[offs + 0] & 0x0f;
		int const sx = (m_spriteram[offs + 6] | ((m_spriteram[offs + 7] & 0x01) << 8));
		int sy = 256 + 128 - (m_spriteram[offs + 2] | ((m_spriteram[offs + 3] & 0x01) << 8));
		int const flipx = m_spriteram[offs + 5] & 0x40;
		int const flipy = m_spriteram[offs + 5] & 0x80;
		int const h = 1 << ((m_spriteram[offs + 5] & 0x30) >> 4);
		sy -= 16 * h;

		code &= ~(h - 1);

		for (int y = 0; y < h; y++)
		{
			int c = code;

			if (flipy) c += h - 1 - y;
			else c += y;

			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					c,
					color,
					flipx, flipy,
					sx, sy + 16 * y, 0);
		}
	}
}

uint32_t vigilant_state::screen_update_kikcubic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x1000; offs += 2)
	{
		int const sy = 8 * ((offs / 2) / 64);
		int const sx = 8 * ((offs / 2) % 64);
		int const attributes = m_videoram[offs + 1];
		int const color = (attributes & 0xf0) >> 4;
		int const tile_number = m_videoram[offs] | ((attributes & 0x0f) << 8);

		m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
				tile_number,
				color,
				0, 0,
				sx, sy);
	}

	draw_sprites(bitmap, cliprect);
	return 0;
}

uint32_t vigilant_state::screen_update_vigilant(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// copy the background palette
	for (int i = 0; i < 16; i++)
	{
		int r = (m_generic_paletteram_8[0x400 + 16 * m_rear_color + i] << 3) & 0xff;
		int g = (m_generic_paletteram_8[0x500 + 16 * m_rear_color + i] << 3) & 0xff;
		int b = (m_generic_paletteram_8[0x600 + 16 * m_rear_color + i] << 3) & 0xff;

		m_palette->set_pen_color(512 + i, rgb_t(r, g, b));

		r = (m_generic_paletteram_8[0x400 + 16 * m_rear_color + 32 + i] << 3) & 0xff;
		g = (m_generic_paletteram_8[0x500 + 16 * m_rear_color + 32 + i] << 3) & 0xff;
		b = (m_generic_paletteram_8[0x600 + 16 * m_rear_color + 32 + i] << 3) & 0xff;

		m_palette->set_pen_color(512 + 16 + i, rgb_t(r, g, b));
	}

	if (m_rear_disable)  // opaque foreground
	{
		draw_foreground(bitmap, cliprect, 0, 1);
		draw_sprites(bitmap, bottomvisiblearea);
		draw_foreground(bitmap, cliprect, 1, 0);
	}
	else
	{
		draw_background(bitmap, cliprect);
		draw_foreground(bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, bottomvisiblearea);
		draw_foreground(bitmap, cliprect, 1, 0); // priority tiles
	}
	return 0;
}

void vigilant_state::draw_foreground_bowmen(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque)
{
	for (int offs = 0; offs < 0x1000; offs += 2)
	{
		int const sy = 8 * ((offs / 2) / 64);
		int const sx = 8 * ((offs / 2) % 64);
		int const attributes = m_videoram[offs + 1];
		int const color = attributes & 0x0f;
		int const tile_number = m_videoram[offs] | ((attributes & 0xf0) << 4);

		if (priority)    // foreground
		{
			if ((color & 0x0f) == 0x0f)  // mask sprites
			{
				m_gfxdecode->gfx(0)->transmask(bitmap, bowmenvisiblearea,
						tile_number,
						color,
						0, 0,
						sx, sy, 0x00ff);
			}
		}
		else     // background
		{
			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					tile_number,
					color,
					0, 0,
					sx, sy,
					(opaque) ? -1 : 0);
		}
	}
}

void vigilant_state::draw_background_bowmen(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const scrollx = 0x179 + 16 * 8 - (m_rear_horiz_scroll_low + m_rear_horiz_scroll_high);


	if (m_rear_refresh)
	{
		update_background_bowmen();
		m_rear_refresh = 0;
	}

	copyscrollbitmap(bitmap, *m_bg_bitmap, 1, &scrollx, 0, nullptr, bowmenvisiblearea);
}

uint32_t vigilant_state::screen_update_bowmen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// copy the background palette
	for (int i = 0; i < 16; i++)
	{
		int r = (m_generic_paletteram_8[0x400 + 16 * m_rear_color + i] << 3) & 0xff;
		int g = (m_generic_paletteram_8[0x500 + 16 * m_rear_color + i] << 3) & 0xff;
		int b = (m_generic_paletteram_8[0x600 + 16 * m_rear_color + i] << 3) & 0xff;

		m_palette->set_pen_color(512 + i, rgb_t(r, g, b));

		r = (m_generic_paletteram_8[0x400 + 16 * m_rear_color + 32 + i] << 3) & 0xff;
		g = (m_generic_paletteram_8[0x500 + 16 * m_rear_color + 32 + i] << 3) & 0xff;
		b = (m_generic_paletteram_8[0x600 + 16 * m_rear_color + 32 + i] << 3) & 0xff;

		m_palette->set_pen_color(512 + 16 + i, rgb_t(r, g, b));
	}

	if (m_rear_disable)  // opaque foreground
	{
		draw_foreground_bowmen(bitmap, cliprect, 0, 1);
	}
	else
	{
		draw_background_bowmen(bitmap, cliprect);
		draw_foreground_bowmen(bitmap, cliprect, 0, 0);
	}
	draw_sprites(bitmap, bowmenvisiblearea);
//  draw_foreground_bowmen(bitmap, cliprect, 1, 0); // priority tiles
	return 0;
}


void vigilant_state::machine_start()
{
	m_rombank->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);
}

void captainx_state::machine_start()
{
	vigilant_state::machine_start();

	save_item(NAME(m_decryption_ram));
	save_item(NAME(m_mcu_porta));
	save_item(NAME(m_mcu_portb));
	save_item(NAME(m_mcu_portc));
}

void captainx_state::machine_reset()
{
	vigilant_state::machine_reset();

	// the main CPU will be started later by the MCU
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void vigilant_state::bank_select_w(uint8_t data)
{
	m_rombank->set_entry(data & 0x07);
}

TIMER_DEVICE_CALLBACK_MEMBER(vigilant_state::sound_nmi)
{
	m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

/***************************************************************************
 vigilant_out2_w
 **************************************************************************/
void vigilant_state::vigilant_out2_w(uint8_t data)
{
	// D0 = FILP = Flip screen?
	// D1 = COA1 = Coin Counter A?
	// D2 = COB1 = Coin Counter B?

	// The hardware has both coin counters hooked up to a single meter.
	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x04);

//  data & 0x01 cocktail mode
}

void vigilant_state::kikcubic_coin_w(uint8_t data)
{
	// bits 0 is flip screen

	// bit 1 is used but unknown

	// bits 4/5 are coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x10);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
}


//**************************************************************************
//  CAPTAINX MCU EMULATION
//**************************************************************************

void captainx_state::mcu_porta_w(u8 data)
{
	m_mcu_porta = data;
}

void captainx_state::mcu_portb_w(u8 data)
{
	// 7-------  unused
	// -654----  address high bits
	// ----32--  unused
	// ------1-  on transition write to ram
	// -------0  main CPU control

	// write to decryption RAM
	if (BIT(m_mcu_portb, 1) == 1 && BIT(data, 1) == 0)
		m_decryption_ram[(data & 0x70) << 4 | m_mcu_porta] = m_mcu_portc;

	// the main CPU will start running once the table upload is finished
	m_maincpu->set_input_line(INPUT_LINE_HALT, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);

	m_mcu_portb = data;
}

void captainx_state::mcu_portc_w(u8 data)
{
	m_mcu_portc = data;
}


uint8_t captainx_state::rom_r(offs_t offset)
{
	// the range 0x0000 to 0x7fff is read indirectly using the decryption RAM
	// for captainx only the first 0x200 bytes (of 0x800 total) are used, so this works
	offs_t address = (BIT(offset, 0) ? 0x100 : 0) | m_maincpu_region[offset];
	return m_decryption_ram[address] ^ 0xff;
}


void vigilant_state::vigilant_map(address_map &map)
{
	map(0x8000, 0xbfff).bankr(m_rombank);        // Fallthrough
	map(0x0000, 0x7fff).rom();
	map(0xc020, 0xc0df).ram().share(m_spriteram);
	map(0xc800, 0xcfff).ram().w(FUNC(vigilant_state::paletteram_w)).share("paletteram");
	map(0xd000, 0xdfff).ram().share(m_videoram);
	map(0xe000, 0xefff).ram();
}

void captainx_state::captainx_map(address_map &map)
{
	vigilant_map(map);
	map(0x0000, 0x7fff).r(FUNC(captainx_state::rom_r));
}

void vigilant_state::vigilant_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w("soundlatch", FUNC(generic_latch_8_device::write));    // SD
	map(0x01, 0x01).portr("IN1").w(FUNC(vigilant_state::vigilant_out2_w));          // OUT2
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).portr("DSW1");
	map(0x04, 0x04).portr("DSW2").w(FUNC(vigilant_state::bank_select_w));  // PBANK
	map(0x80, 0x81).w(FUNC(vigilant_state::vigilant_horiz_scroll_w));      // HSPL, HSPH
	map(0x82, 0x83).w(FUNC(vigilant_state::vigilant_rear_horiz_scroll_w)); // RHSPL, RHSPH
	map(0x84, 0x84).w(FUNC(vigilant_state::vigilant_rear_color_w));        // RCOD
}

void vigilant_state::kikcubic_map(address_map &map)
{
	map(0x8000, 0xbfff).bankr(m_rombank);        // Fallthrough
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc0ff).ram().share(m_spriteram);
	map(0xc800, 0xcaff).ram().w(FUNC(vigilant_state::paletteram_w)).share("paletteram");
	map(0xd000, 0xdfff).ram().share(m_videoram);
	map(0xe000, 0xffff).ram();
}

void vigilant_state::kikcubic_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1").w(FUNC(vigilant_state::kikcubic_coin_w)); // also flip screen, and...?
	map(0x01, 0x01).portr("DSW2");
	map(0x02, 0x02).portr("IN0");
	map(0x03, 0x03).portr("IN1");
	map(0x04, 0x04).portr("IN2").w(FUNC(vigilant_state::bank_select_w));
	map(0x06, 0x06).w("soundlatch", FUNC(generic_latch_8_device::write));
//  map(0x07, 0x07).nopw(); // ??
}

void vigilant_state::bowmen_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w("soundlatch", FUNC(generic_latch_8_device::write));  // SD seems BAD
	map(0x01, 0x01).portr("IN1").w(FUNC(vigilant_state::vigilant_out2_w));              // OUT2?
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).portr("DSW2");
	map(0x04, 0x04).portr("DSW1").w(FUNC(vigilant_state::bank_select_w));  // PBANK?
//  map(0x10, 0x11).w(FUNC(vigilant_state::vigilant_horiz_scroll_w));      // HSPL, HSPH
	map(0x12, 0x13).w(FUNC(vigilant_state::bowmen_rear_horiz_scroll_w));   // RHSPL, RHSPH
	map(0x14, 0x14).w(FUNC(vigilant_state::bowmen_rear_color_w));          // RCOD
/*
    02;  w
    10;  w
    11;  w
    12;  w
    13;  w
    14;  w
*/
}

void vigilant_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}

void vigilant_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80, 0x81).r("soundlatch", FUNC(generic_latch_8_device::read)).w(m_audio, FUNC(m72_audio_device::vigilant_sample_addr_w));   // STL / STH
	map(0x82, 0x82).w(m_audio, FUNC(m72_audio_device::sample_w));            // COUNT UP
	map(0x83, 0x83).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w)); // IRQ clear
	map(0x84, 0x84).r(m_audio, FUNC(m72_audio_device::sample_r)); // S ROM C
}

void vigilant_state::buccanrs_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x02, 0x03).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read));             // SDRE
	map(0x80, 0x81).w(m_audio, FUNC(m72_audio_device::vigilant_sample_addr_w));  // STL / STH
	map(0x82, 0x82).w(m_audio, FUNC(m72_audio_device::sample_w));                // COUNT UP
	map(0x83, 0x83).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));   // IRQ clear
	map(0x84, 0x84).r(m_audio, FUNC(m72_audio_device::sample_r));             // S ROM C
}


static INPUT_PORTS_START( vigilant )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, "Decrease of Energy" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	IREM_Z80_COINAGE_TYPE_4_LOC(SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	// In stop mode, press 2 to stop and 1 to restart
	PORT_DIPNAME( 0x20, 0x20, "Stop Mode (Cheat)")      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kikcubic )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(19)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	IREM_Z80_COINAGE_TYPE_2_LOC(SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Player Adding" )     PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( buccanrs )
	PORT_START("IN0")
	PORT_SERVICE( 0x2f, IP_ACTIVE_LOW ) // any of these bits while booting will enable service mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x50, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Invincibility (time still decrease)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( buccanra )
	PORT_INCLUDE( buccanrs )

	PORT_MODIFY("IN0") // this port is reversed on this set..
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0xf4, IP_ACTIVE_LOW ) // any of these bits while booting will enable service mode
INPUT_PORTS_END


static INPUT_PORTS_START( bowmen )
	PORT_START("IN0")
	PORT_SERVICE( 0x2f, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )  // checked in the test mode.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL  // checked in the test mode.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Screen Orientation" )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Normal Screen" )
	PORT_DIPSETTING(    0x00, "Inverted Screen" )
	PORT_DIPNAME( 0x02, 0x02, "Players" )               PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "Two Players" )
	PORT_DIPSETTING(    0x00, "One Player" )
	PORT_DIPNAME( 0x0c, 0x0c, "Time for Round" )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "Free Time per Round" )
	PORT_DIPSETTING(    0x04, "30 Seconds per Round" )
	PORT_DIPSETTING(    0x08, "50 Seconds per Round" )
	PORT_DIPSETTING(    0x0c, "60 Seconds per Round" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Type" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Easy Test" )
	PORT_DIPSETTING(    0x00, "Advanced Test" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
INPUT_PORTS_END


static const gfx_layout text_layout =
{
	8,8, // tile size
	RGN_FRAC(1,2), // number of tiles
	4, // bits per pixel
	{RGN_FRAC(1,2),RGN_FRAC(1,2)+4,0,4}, // plane offsets
	{ 0,1,2,3, 64+0,64+1,64+2,64+3 }, // x offsets
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 }, // y offsets
	128
};

static const gfx_layout sprite_layout =
{
	16,16,  // tile size
	RGN_FRAC(1,2),  // number of sprites ($1000)
	4,      // bits per pixel
	{RGN_FRAC(1,2),RGN_FRAC(1,2)+4,0,4}, // plane offsets
	{ // x offsets
		0x00*8+0,0x00*8+1,0x00*8+2,0x00*8+3,
		0x10*8+0,0x10*8+1,0x10*8+2,0x10*8+3,
		0x20*8+0,0x20*8+1,0x20*8+2,0x20*8+3,
		0x30*8+0,0x30*8+1,0x30*8+2,0x30*8+3
	},
	{ // y offsets
		0x00*8, 0x01*8, 0x02*8, 0x03*8,
		0x04*8, 0x05*8, 0x06*8, 0x07*8,
		0x08*8, 0x09*8, 0x0A*8, 0x0B*8,
		0x0C*8, 0x0D*8, 0x0E*8, 0x0F*8
	},
	0x40*8
};

static const gfx_layout sprite_layout_buccanrs =
{
	16,16,  // tile size
	RGN_FRAC(1,2),  // number of sprites ($1000)
	4,      // bits per pixel
	{RGN_FRAC(1,2),RGN_FRAC(1,2)+4,0,4}, // plane offsets
	{ // x offsets
		0x00*8+3,0x00*8+2,0x00*8+1,0x00*8+0,
		0x10*8+3,0x10*8+2,0x10*8+1,0x10*8+0,
		0x20*8+3,0x20*8+2,0x20*8+1,0x20*8+0,
		0x30*8+3,0x30*8+2,0x30*8+1,0x30*8+0
	},
	{ // y offsets
		0x00*8, 0x01*8, 0x02*8, 0x03*8,
		0x04*8, 0x05*8, 0x06*8, 0x07*8,
		0x08*8, 0x09*8, 0x0A*8, 0x0B*8,
		0x0C*8, 0x0D*8, 0x0E*8, 0x0F*8
	},
	0x40*8
};


static const gfx_layout back_layout =
{
	32,1, // tile size
	RGN_FRAC(1,1), // number of tiles
	4, // bits per pixel
	{0,2,4,6}, // plane offsets
	{ 0*8+1, 0*8,  1*8+1, 1*8, 2*8+1, 2*8, 3*8+1, 3*8, 4*8+1, 4*8, 5*8+1, 5*8,
	6*8+1, 6*8, 7*8+1, 7*8, 8*8+1, 8*8, 9*8+1, 9*8, 10*8+1, 10*8, 11*8+1, 11*8,
	12*8+1, 12*8, 13*8+1, 13*8, 14*8+1, 14*8, 15*8+1, 15*8 }, // x offsets
	{ 0 }, // y offsets
	16*8
};

static const gfx_layout buccaneer_back_layout =
{
	32,1, // tile size
	RGN_FRAC(1,1), // number of tiles
	4, // bits per pixel
	{6,4,2,0}, // plane offsets
	{ 0*8+1, 0*8,  1*8+1, 1*8, 2*8+1, 2*8, 3*8+1, 3*8, 4*8+1, 4*8, 5*8+1, 5*8,
	6*8+1, 6*8, 7*8+1, 7*8, 8*8+1, 8*8, 9*8+1, 9*8, 10*8+1, 10*8, 11*8+1, 11*8,
	12*8+1, 12*8, 13*8+1, 13*8, 14*8+1, 14*8, 15*8+1, 15*8 }, // x offsets
	{ 0 }, // y offsets
	16*8
};

static GFXDECODE_START( gfx_vigilant )
	GFXDECODE_ENTRY( "chars",   0, text_layout,   256, 16 )    // colors 256-511 */
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout,   0, 16 )    // colors   0-255 */
	GFXDECODE_ENTRY( "bgtiles", 0, back_layout,   512,  2 )    // actually the background uses colors
															   // 256-511, but giving it exclusive
															   // pens we can handle it more easily.
GFXDECODE_END

static GFXDECODE_START( gfx_buccanrs )
	GFXDECODE_ENTRY( "chars",   0, text_layout,            256, 16 )  // colors 256-511
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout_buccanrs,   0, 16 )  // colors   0-255
	GFXDECODE_ENTRY( "bgtiles", 0, buccaneer_back_layout,  512,  2 )  // actually the background uses colors
																	  // 256-511, but giving it exclusive
																	  // pens we can handle it more easily.
GFXDECODE_END

static GFXDECODE_START( gfx_bowmen )
	GFXDECODE_ENTRY( "chars",   0, text_layout,            256, 16 )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout_buccanrs,   0, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0, buccaneer_back_layout,  512,  2 )

GFXDECODE_END

static GFXDECODE_START( gfx_kikcubic )
	GFXDECODE_ENTRY( "chars",   0, text_layout,   0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout, 0, 16 )
GFXDECODE_END


void vigilant_state::vigilant(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vigilant_state::vigilant_map);
	m_maincpu->set_addrmap(AS_IO, &vigilant_state::vigilant_io_map);

	z80_device &soundcpu(Z80(config, m_soundcpu, 3.579545_MHz_XTAL));
	soundcpu.set_addrmap(AS_PROGRAM, &vigilant_state::sound_map);
	soundcpu.set_addrmap(AS_IO, &vigilant_state::sound_io_map);
	// IRQs are generated by main Z80 and YM2151
	soundcpu.set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	TIMER(config, "v1").configure_scanline(FUNC(vigilant_state::sound_nmi), "screen", 1, 2);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// Measured on PCB:
	screen.set_raw(18.432_MHz_XTAL / 3 /* pixel clock*/, 384 /* total H */, 128, 384, 256+28 /* total V */, 0, 256);
	screen.set_screen_update(FUNC(vigilant_state::screen_update_vigilant));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vigilant);
	PALETTE(config, m_palette).set_entries(512+32); // 512 real palette, 32 virtual palette

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq").int_callback().set_inputline(m_soundcpu, 0);

	IREM_M72_AUDIO(config, m_audio);
	m_audio->set_dac_tag("dac");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ymsnd.add_route(0, "speaker", 0.28, 0);
	ymsnd.add_route(1, "speaker", 0.28, 1);

	dac_8bit_r2r_device &dac(DAC_8BIT_R2R(config, "dac", 0)); // unknown DAC
	dac.add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	dac.add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
}

void vigilant_state::buccanrs(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 5.0688_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vigilant_state::vigilant_map);
	m_maincpu->set_addrmap(AS_IO, &vigilant_state::vigilant_io_map);

	z80_device &soundcpu(Z80(config, m_soundcpu, 18.432_MHz_XTAL / 6));  // 3.072000 MHz
	soundcpu.set_addrmap(AS_PROGRAM, &vigilant_state::sound_map);
	soundcpu.set_addrmap(AS_IO, &vigilant_state::buccanrs_sound_io_map);
								// IRQs are generated by main Z80 and YM2151
	soundcpu.set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	TIMER(config, "v1").configure_scanline(FUNC(vigilant_state::sound_nmi), "screen", 1, 2);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 384, 128, 384, 284, 0, 256); // sync rates not verified
	screen.set_screen_update(FUNC(vigilant_state::screen_update_vigilant));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_buccanrs);
	PALETTE(config, m_palette).set_entries(512+32); // 512 real palette, 32 virtual palette

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq").int_callback().set_inputline(m_soundcpu, 0);

	IREM_M72_AUDIO(config, m_audio);
	m_audio->set_dac_tag("dac");

	ym2203_device &ym1(YM2203(config, "ym1", 18.432_MHz_XTAL / 6));
	ym1.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ym1.add_route(0, "speaker", 0.35, 0);
	ym1.add_route(0, "speaker", 0.35, 1);
	ym1.add_route(1, "speaker", 0.35, 0);
	ym1.add_route(1, "speaker", 0.35, 1);
	ym1.add_route(2, "speaker", 0.35, 0);
	ym1.add_route(2, "speaker", 0.35, 1);
	ym1.add_route(3, "speaker", 0.50, 0);
	ym1.add_route(3, "speaker", 0.50, 1);

	ym2203_device &ym2(YM2203(config, "ym2", 18.432_MHz_XTAL / 6));
	ym2.add_route(0, "speaker", 0.35, 0);
	ym2.add_route(0, "speaker", 0.35, 1);
	ym2.add_route(1, "speaker", 0.35, 0);
	ym2.add_route(1, "speaker", 0.35, 1);
	ym2.add_route(2, "speaker", 0.35, 0);
	ym2.add_route(2, "speaker", 0.35, 1);
	ym2.add_route(3, "speaker", 0.50, 0);
	ym2.add_route(3, "speaker", 0.50, 1);

	dac_8bit_r2r_device &dac(DAC_8BIT_R2R(config, "dac", 0)); // unknown DAC
	dac.add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	dac.add_route(ALL_OUTPUTS, "speaker", 0.35, 1);
}

void captainx_state::captainx(machine_config &config)
{
	buccanrs(config);

	m_maincpu->set_clock(5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &captainx_state::captainx_map);

	M68705R3(config, m_mcu, 5_MHz_XTAL / 2); // unknown clock
	m_mcu->porta_w().set(FUNC(captainx_state::mcu_porta_w));
	m_mcu->portb_w().set(FUNC(captainx_state::mcu_portb_w));
	m_mcu->portc_w().set(FUNC(captainx_state::mcu_portc_w));
}

void vigilant_state::kikcubic(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vigilant_state::kikcubic_map);
	m_maincpu->set_addrmap(AS_IO, &vigilant_state::kikcubic_io_map);

	z80_device &soundcpu(Z80(config, m_soundcpu, 3.579545_MHz_XTAL));
	soundcpu.set_addrmap(AS_PROGRAM, &vigilant_state::sound_map);
	soundcpu.set_addrmap(AS_IO, &vigilant_state::sound_io_map);
								// IRQs are generated by main Z80 and YM2151
	soundcpu.set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	TIMER(config, "v1").configure_scanline(FUNC(vigilant_state::sound_nmi), "screen", 1, 2);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24_MHz_XTAL / 3, 64*8, 8*8, (64-8)*8, 284, 0, 256); // sync rates not verified
	screen.set_screen_update(FUNC(vigilant_state::screen_update_kikcubic));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kikcubic);
	PALETTE(config, m_palette).set_entries(256);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq").int_callback().set_inputline(m_soundcpu, 0);

	IREM_M72_AUDIO(config, m_audio);
	m_audio->set_dac_tag("dac");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ymsnd.add_route(0, "speaker", 0.28, 0);
	ymsnd.add_route(1, "speaker", 0.28, 1);

	dac_8bit_r2r_device &dac(DAC_8BIT_R2R(config, "dac", 0)); // unknown DAC
	dac.add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	dac.add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
}


void vigilant_state::bowmen(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18_MHz_XTAL / 3);    // 5.99538 MHz verified
	m_maincpu->set_addrmap(AS_PROGRAM, &vigilant_state::vigilant_map);
	m_maincpu->set_addrmap(AS_IO, &vigilant_state::bowmen_io_map);

	z80_device &soundcpu(Z80(config, m_soundcpu, 18_MHz_XTAL / 3)); // 5.99528 MHz verified
	soundcpu.set_addrmap(AS_PROGRAM, &vigilant_state::sound_map);
	soundcpu.set_addrmap(AS_IO, &vigilant_state::buccanrs_sound_io_map);

	soundcpu.set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	TIMER(config, "v1").configure_scanline(FUNC(vigilant_state::sound_nmi), "screen", 1, 2); // 7.80650 kHz measured

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18_MHz_XTAL / 3, 384, 128, 384, 284, 0, 256);  // 54.9752 Hz verified
	screen.set_screen_update(FUNC(vigilant_state::screen_update_bowmen));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bowmen);
	PALETTE(config, m_palette).set_entries(512 + 32); // 512 real palette, 32 virtual palette

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq").int_callback().set_inputline(m_soundcpu, 0);

	IREM_M72_AUDIO(config, m_audio);
	m_audio->set_dac_tag("dac");

	ym2203_device &ym1(YM2203(config, "ym1", 18_MHz_XTAL / 6));
	ym1.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ym1.add_route(0, "speaker", 0.35, 0);
	ym1.add_route(0, "speaker", 0.35, 1);
	ym1.add_route(1, "speaker", 0.35, 0);
	ym1.add_route(1, "speaker", 0.35, 1);
	ym1.add_route(2, "speaker", 0.35, 0);
	ym1.add_route(2, "speaker", 0.35, 1);
	ym1.add_route(3, "speaker", 0.50, 0);
	ym1.add_route(3, "speaker", 0.50, 1);

	ym2203_device &ym2(YM2203(config, "ym2", 18_MHz_XTAL / 6));
	ym2.add_route(0, "speaker", 0.35, 0);
	ym2.add_route(0, "speaker", 0.35, 1);
	ym2.add_route(1, "speaker", 0.35, 0);
	ym2.add_route(1, "speaker", 0.35, 1);
	ym2.add_route(2, "speaker", 0.35, 0);
	ym2.add_route(2, "speaker", 0.35, 1);
	ym2.add_route(3, "speaker", 0.50, 0);
	ym2.add_route(3, "speaker", 0.50, 1);

	dac_8bit_r2r_device &dac(DAC_8BIT_R2R(config, "dac", 0)); // unknown DAC
	dac.add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	dac.add_route(ALL_OUTPUTS, "speaker", 0.35, 1);
}


/***************************************************************************

  Game ROMs

***************************************************************************/


ROM_START( vigilant ) // World Rev E
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "vg_a-8h-e.ic55",  0x00000, 0x08000, CRC(0d4e6866) SHA1(50ddeb34e72d3f6368b3da5cddf0f510693c8cce) )
	ROM_LOAD( "vg_a-8l-a.ic57",  0x10000, 0x10000, CRC(690d812f) SHA1(60d6513f8b27411018cdca1b25f94bc281476ae7) )
	// 0x20000-0x2ffff empty

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "vg_a-5j-.ic37",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "chars", 0 ) // 27C512 on E/G set
	ROM_LOAD( "vg_b-4f-.ic34",  0x00000, 0x10000, CRC(01579d20) SHA1(e58d8ca0ea0ac9d77225bf55faa499d1565924f9) )
	ROM_LOAD( "vg_b-4j-.ic35",  0x10000, 0x10000, CRC(4f5872f0) SHA1(6af21ba1c94097eecce30585983b4b07528c8635) )

	ROM_REGION( 0x80000, "sprites", 0 ) // 27C1000 on E/G set
	ROM_LOAD( "vg_b-6l-.ic62",  0x00000, 0x10000, CRC(fbe9552d) SHA1(3c7c218f13c0a94bb624745d81d63db9423777ea) )
	ROM_CONTINUE(0x20000,0x10000)
	ROM_LOAD( "vg_b-6k-.ic61",  0x10000, 0x10000, CRC(ae09d5c0) SHA1(9da5b824b148c1e1478e3f4b44ba4348376ed7d5) )
	ROM_CONTINUE(0x30000,0x10000)
	ROM_LOAD( "vg_b-6p-.ic64",  0x40000, 0x10000, CRC(afb77461) SHA1(18707768a4768b579c94092a260e286d3214b977) )
	ROM_CONTINUE(0x60000,0x10000)
	ROM_LOAD( "vg_b-6n-.ic63",  0x50000, 0x10000, CRC(5065cd35) SHA1(9a03c5af024fcae6b3371bb04be3e811ecc390d7) )
	ROM_CONTINUE(0x70000,0x10000)

	ROM_REGION( 0x40000, "bgtiles", 0 ) // 27C512 on E/G set
	ROM_LOAD( "vg_b-1d-.ic2",  0x00000, 0x10000, CRC(81b1ee5c) SHA1(2014165ec71f089fecb5a3e60b939cc0f565d7f1) )
	ROM_LOAD( "vg_b-1f-.ic3",  0x10000, 0x10000, CRC(d0d33673) SHA1(39761d97a71deaf7f17233d5bd5a55dbb1e6b30e) )
	ROM_LOAD( "vg_b-1h-.ic4",  0x20000, 0x10000, CRC(aae81695) SHA1(ca8e136eca3543b27f3a61b105d4a280711cd6ea) )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "vg_a-4d-.ic26",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 ) // All are pal16l8 - protected
	ROM_LOAD( "vg_b-8r.ic90", 0x0000, 0x0117, CRC(df368a7a) SHA1(597d85d1f90b7ee0188f2d849792ee02ff2ea48b) )
	ROM_LOAD( "vg_b-4m.ic38", 0x0200, 0x0117, CRC(dbca4204) SHA1(d8e190f2dc4d6285f22be331d01ed402520d2017) )
	ROM_LOAD( "vg_b-1b.ic1",  0x0400, 0x0117, CRC(922e5167) SHA1(08efdfdfeb35f3f73b6fd3d5c0c2a386dea5f617) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10_7a.ic52", 0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // tbp24s10, 82s129-equivalent - video timing
ROM_END

ROM_START( vigilantg ) // US Rev G
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "vg_a-8h-g.ic55",  0x00000, 0x08000, CRC(9444c04e) SHA1(463d2dae48df2d237bd19d5e16cab032df0d9052) )
	ROM_LOAD( "vg_a-8l-.ic57",   0x10000, 0x10000, CRC(7f95799b) SHA1(a371671c3c26976314aaac4e410bff0f13a8a085) )
	// 0x20000-0x2ffff empty

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "vg_a-5j-.ic37",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "chars", 0 ) // 27C512 on E/G set
	ROM_LOAD( "vg_b-4f-.ic34",  0x00000, 0x10000, CRC(01579d20) SHA1(e58d8ca0ea0ac9d77225bf55faa499d1565924f9) )
	ROM_LOAD( "vg_b-4j-.ic35",  0x10000, 0x10000, CRC(4f5872f0) SHA1(6af21ba1c94097eecce30585983b4b07528c8635) )

	ROM_REGION( 0x80000, "sprites", 0 ) // 27C1000 on E/G set
	ROM_LOAD( "vg_b-6l-.ic62",  0x00000, 0x10000, CRC(fbe9552d) SHA1(3c7c218f13c0a94bb624745d81d63db9423777ea) )
	ROM_CONTINUE(0x20000,0x10000)
	ROM_LOAD( "vg_b-6k-.ic61",  0x10000, 0x10000, CRC(ae09d5c0) SHA1(9da5b824b148c1e1478e3f4b44ba4348376ed7d5) )
	ROM_CONTINUE(0x30000,0x10000)
	ROM_LOAD( "vg_b-6p-.ic64",  0x40000, 0x10000, CRC(afb77461) SHA1(18707768a4768b579c94092a260e286d3214b977) )
	ROM_CONTINUE(0x60000,0x10000)
	ROM_LOAD( "vg_b-6n-.ic63",  0x50000, 0x10000, CRC(5065cd35) SHA1(9a03c5af024fcae6b3371bb04be3e811ecc390d7) )
	ROM_CONTINUE(0x70000,0x10000)

	ROM_REGION( 0x40000, "bgtiles", 0 ) // 27C512 on E/G set
	ROM_LOAD( "vg_b-1d-.ic2",  0x00000, 0x10000, CRC(81b1ee5c) SHA1(2014165ec71f089fecb5a3e60b939cc0f565d7f1) )
	ROM_LOAD( "vg_b-1f-.ic3",  0x10000, 0x10000, CRC(d0d33673) SHA1(39761d97a71deaf7f17233d5bd5a55dbb1e6b30e) )
	ROM_LOAD( "vg_b-1h-.ic4",  0x20000, 0x10000, CRC(aae81695) SHA1(ca8e136eca3543b27f3a61b105d4a280711cd6ea) )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "vg_a-4d-.ic26",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 ) // All are pal16l8 - protected
	ROM_LOAD( "vg_b-8r.ic90", 0x0000, 0x0117, CRC(df368a7a) SHA1(597d85d1f90b7ee0188f2d849792ee02ff2ea48b) )
	ROM_LOAD( "vg_b-4m.ic38", 0x0200, 0x0117, CRC(dbca4204) SHA1(d8e190f2dc4d6285f22be331d01ed402520d2017) )
	ROM_LOAD( "vg_b-1b.ic1",  0x0400, 0x0117, CRC(922e5167) SHA1(08efdfdfeb35f3f73b6fd3d5c0c2a386dea5f617) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10_7a.ic52", 0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // tbp24s10, 82s129-equivalent - video timing
ROM_END

ROM_START( vigilanto ) // US (earliest base version)
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "vg_a-8h-.ic55",  0x00000, 0x08000, CRC(8d15109e) SHA1(9ef57047a0b53cd0143a260193b33e3d5680ca71) )
	ROM_LOAD( "vg_a-8l-.ic57",  0x10000, 0x10000, CRC(7f95799b) SHA1(a371671c3c26976314aaac4e410bff0f13a8a085) )
	// 0x20000-0x2ffff empty

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "vg_a-5j-.ic37",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "chars", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set, top half empty
	ROM_LOAD( "613.ic34",  0x00000, 0x10000, CRC(ee7a6c2f) SHA1(e676654d5bdc53604d503fd1fe244a84372efaec) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "614.ic35",  0x10000, 0x10000, CRC(6422e8ba) SHA1(7bb4e5d5362d352c3fa70bf101d10b09f25a4c66) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x80000, "sprites", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "616.ic62",  0x00000, 0x10000, CRC(fbe9552d) SHA1(3c7c218f13c0a94bb624745d81d63db9423777ea) )
	ROM_CONTINUE(0x20000,0x10000)
	ROM_LOAD( "615.ic61",  0x10000, 0x10000, CRC(ae09d5c0) SHA1(9da5b824b148c1e1478e3f4b44ba4348376ed7d5) )
	ROM_CONTINUE(0x30000,0x10000)
	ROM_LOAD( "618.ic64",  0x40000, 0x10000, CRC(afb77461) SHA1(18707768a4768b579c94092a260e286d3214b977) )
	ROM_CONTINUE(0x60000,0x10000)
	ROM_LOAD( "617.ic63",  0x50000, 0x10000, CRC(5065cd35) SHA1(9a03c5af024fcae6b3371bb04be3e811ecc390d7) )
	ROM_CONTINUE(0x70000,0x10000)

	ROM_REGION( 0x40000, "bgtiles", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "619.ic2",  0x00000, 0x20000, CRC(9e2f8759) SHA1(2cc2f65b068c14e353e42f0b4adf921a97f0490a) )
	ROM_LOAD( "612.ic4",  0x20000, 0x10000, CRC(85057c81) SHA1(47663e17f08f47d847605c14e849266468ff39ba) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "vg_a-4d-.ic26",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 ) // All are pal16l8 - protected
	ROM_LOAD( "vg_b-8r.ic90", 0x0000, 0x0117, CRC(df368a7a) SHA1(597d85d1f90b7ee0188f2d849792ee02ff2ea48b) )
	ROM_LOAD( "vg_b-4m.ic38", 0x0200, 0x0117, CRC(dbca4204) SHA1(d8e190f2dc4d6285f22be331d01ed402520d2017) )
	ROM_LOAD( "vg_b-1b.ic1",  0x0400, 0x0117, CRC(922e5167) SHA1(08efdfdfeb35f3f73b6fd3d5c0c2a386dea5f617) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10_7a.ic52", 0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // tbp24s10, 82s129-equivalent - video timing
ROM_END

ROM_START( vigilanta ) // World Rev A
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "vg_a-8h-a.ic55",  0x00000, 0x08000, CRC(97df1454) SHA1(4c29e57529e20315459d36c1f1ad3d729546bef0) )
	ROM_LOAD( "vg_a-8l-a.ic57",  0x10000, 0x10000, CRC(690d812f) SHA1(60d6513f8b27411018cdca1b25f94bc281476ae7) )
	// 0x20000-0x2ffff empty

	ROM_REGION( 0x10000, "soundcpu", 0 ) // matches base set
	ROM_LOAD( "vg_a-5j-a.ic37",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "chars", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set, top half empty
	ROM_LOAD( "613.ic34",  0x00000, 0x10000, CRC(ee7a6c2f) SHA1(e676654d5bdc53604d503fd1fe244a84372efaec) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "614.ic35",  0x10000, 0x10000, CRC(6422e8ba) SHA1(7bb4e5d5362d352c3fa70bf101d10b09f25a4c66) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x80000, "sprites", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "616.ic62",  0x00000, 0x10000, CRC(fbe9552d) SHA1(3c7c218f13c0a94bb624745d81d63db9423777ea) )
	ROM_CONTINUE(0x20000,0x10000)
	ROM_LOAD( "615.ic61",  0x10000, 0x10000, CRC(ae09d5c0) SHA1(9da5b824b148c1e1478e3f4b44ba4348376ed7d5) )
	ROM_CONTINUE(0x30000,0x10000)
	ROM_LOAD( "618.ic64",  0x40000, 0x10000, CRC(afb77461) SHA1(18707768a4768b579c94092a260e286d3214b977) )
	ROM_CONTINUE(0x60000,0x10000)
	ROM_LOAD( "617.ic63",  0x50000, 0x10000, CRC(5065cd35) SHA1(9a03c5af024fcae6b3371bb04be3e811ecc390d7) )
	ROM_CONTINUE(0x70000,0x10000)

	ROM_REGION( 0x40000, "bgtiles", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "619.ic2",  0x00000, 0x20000, CRC(9e2f8759) SHA1(2cc2f65b068c14e353e42f0b4adf921a97f0490a) )
	ROM_LOAD( "612.ic4",  0x20000, 0x10000, CRC(85057c81) SHA1(47663e17f08f47d847605c14e849266468ff39ba) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x10000, "m72", 0 ) // samples, matches base set
	ROM_LOAD( "vg_a-4d-a.ic26",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 ) // All are pal16l8 - protected
	ROM_LOAD( "vg_b-8r.ic90", 0x0000, 0x0117, CRC(df368a7a) SHA1(597d85d1f90b7ee0188f2d849792ee02ff2ea48b) )
	ROM_LOAD( "vg_b-4m.ic38", 0x0200, 0x0117, CRC(dbca4204) SHA1(d8e190f2dc4d6285f22be331d01ed402520d2017) )
	ROM_LOAD( "vg_b-1b.ic1",  0x0400, 0x0117, CRC(922e5167) SHA1(08efdfdfeb35f3f73b6fd3d5c0c2a386dea5f617) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10_7a.ic52", 0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // tbp24s10, 82s129-equivalent - video timing
ROM_END

ROM_START( vigilantb ) // US Rev B
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "vg_a-8h-b.ic55", 0x00000, 0x08000, CRC(05350c2a) SHA1(5fe932bcae34b8f85ffb519879db4115a5ff5464) )
	ROM_LOAD( "vg_a-8l-.ic57",  0x10000, 0x10000, CRC(7f95799b) SHA1(a371671c3c26976314aaac4e410bff0f13a8a085) )
	// 0x20000-0x2ffff empty

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "vg_a-5j-.ic37",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "chars", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set, top half empty
	ROM_LOAD( "613.ic34",  0x00000, 0x10000, CRC(ee7a6c2f) SHA1(e676654d5bdc53604d503fd1fe244a84372efaec) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "614.ic35",  0x10000, 0x10000, CRC(6422e8ba) SHA1(7bb4e5d5362d352c3fa70bf101d10b09f25a4c66) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x80000, "sprites", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "616.ic62",  0x00000, 0x10000, CRC(fbe9552d) SHA1(3c7c218f13c0a94bb624745d81d63db9423777ea) )
	ROM_CONTINUE(0x20000,0x10000)
	ROM_LOAD( "615.ic61",  0x10000, 0x10000, CRC(ae09d5c0) SHA1(9da5b824b148c1e1478e3f4b44ba4348376ed7d5) )
	ROM_CONTINUE(0x30000,0x10000)
	ROM_LOAD( "618.ic64",  0x40000, 0x10000, CRC(afb77461) SHA1(18707768a4768b579c94092a260e286d3214b977) )
	ROM_CONTINUE(0x60000,0x10000)
	ROM_LOAD( "617.ic63",  0x50000, 0x10000, CRC(5065cd35) SHA1(9a03c5af024fcae6b3371bb04be3e811ecc390d7) )
	ROM_CONTINUE(0x70000,0x10000)

	ROM_REGION( 0x40000, "bgtiles", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "619.ic2",  0x00000, 0x20000, CRC(9e2f8759) SHA1(2cc2f65b068c14e353e42f0b4adf921a97f0490a) )
	ROM_LOAD( "612.ic4",  0x20000, 0x10000, CRC(85057c81) SHA1(47663e17f08f47d847605c14e849266468ff39ba) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "vg_a-4d-.ic26",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 ) // All are pal16l8 - protected
	ROM_LOAD( "vg_b-8r.ic90", 0x0000, 0x0117, CRC(df368a7a) SHA1(597d85d1f90b7ee0188f2d849792ee02ff2ea48b) )
	ROM_LOAD( "vg_b-4m.ic38", 0x0200, 0x0117, CRC(dbca4204) SHA1(d8e190f2dc4d6285f22be331d01ed402520d2017) )
	ROM_LOAD( "vg_b-1b.ic1",  0x0400, 0x0117, CRC(922e5167) SHA1(08efdfdfeb35f3f73b6fd3d5c0c2a386dea5f617) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10_7a.ic52", 0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // tbp24s10, 82s129-equivalent - video timing
ROM_END

ROM_START( vigilantc ) // World Rev C
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "vg_a-8h-c.ic55",  0x00000, 0x08000, CRC(d72682e8) SHA1(2401a6397164ff66d96f6023f021c615d70108a5) )
	ROM_LOAD( "vg_a-8l-a.ic57",  0x10000, 0x10000, CRC(690d812f) SHA1(60d6513f8b27411018cdca1b25f94bc281476ae7) )
	// 0x20000-0x2ffff empty

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "vg_a-5j-.ic37",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "chars", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set, top half empty
	ROM_LOAD( "613.ic34",  0x00000, 0x10000, CRC(ee7a6c2f) SHA1(e676654d5bdc53604d503fd1fe244a84372efaec) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "614.ic35",  0x10000, 0x10000, CRC(6422e8ba) SHA1(7bb4e5d5362d352c3fa70bf101d10b09f25a4c66) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x80000, "sprites", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "616.ic62",  0x00000, 0x10000, CRC(fbe9552d) SHA1(3c7c218f13c0a94bb624745d81d63db9423777ea) )
	ROM_CONTINUE(0x20000,0x10000)
	ROM_LOAD( "615.ic61",  0x10000, 0x10000, CRC(ae09d5c0) SHA1(9da5b824b148c1e1478e3f4b44ba4348376ed7d5) )
	ROM_CONTINUE(0x30000,0x10000)
	ROM_LOAD( "618.ic64",  0x40000, 0x10000, CRC(afb77461) SHA1(18707768a4768b579c94092a260e286d3214b977) )
	ROM_CONTINUE(0x60000,0x10000)
	ROM_LOAD( "617.ic63",  0x50000, 0x10000, CRC(5065cd35) SHA1(9a03c5af024fcae6b3371bb04be3e811ecc390d7) )
	ROM_CONTINUE(0x70000,0x10000)

	ROM_REGION( 0x40000, "bgtiles", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "619.ic2",  0x00000, 0x20000, CRC(9e2f8759) SHA1(2cc2f65b068c14e353e42f0b4adf921a97f0490a) )
	ROM_LOAD( "612.ic4",  0x20000, 0x10000, CRC(85057c81) SHA1(47663e17f08f47d847605c14e849266468ff39ba) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "vg_a-4d-.ic26",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 ) // All are pal16l8 - protected
	ROM_LOAD( "vg_b-8r.ic90", 0x0000, 0x0117, CRC(df368a7a) SHA1(597d85d1f90b7ee0188f2d849792ee02ff2ea48b) )
	ROM_LOAD( "vg_b-4m.ic38", 0x0200, 0x0117, CRC(dbca4204) SHA1(d8e190f2dc4d6285f22be331d01ed402520d2017) )
	ROM_LOAD( "vg_b-1b.ic1",  0x0400, 0x0117, CRC(922e5167) SHA1(08efdfdfeb35f3f73b6fd3d5c0c2a386dea5f617) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10_7a.ic52", 0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // tbp24s10, 82s129-equivalent - video timing
ROM_END

ROM_START( vigilantd ) // Japan Rev D
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "vg_a-8h-d.ic55",  0x00000, 0x08000, CRC(ba848713) SHA1(b357cbf404fb1874d555797ed9fb37f946cc4340) )
	ROM_LOAD( "vg_a-8l-d.ic57",  0x10000, 0x10000, CRC(3b12b1d8) SHA1(2f9207f8d8ec41ea1b8f5bf3c69a97d1d09f6c3f) )
	// 0x20000-0x2ffff empty

	ROM_REGION( 0x10000, "soundcpu", 0 ) // matches base set
	ROM_LOAD( "vg_a-5j-d.ic37",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "chars", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set, top half empty
	ROM_LOAD( "613.ic34",  0x00000, 0x10000, CRC(ee7a6c2f) SHA1(e676654d5bdc53604d503fd1fe244a84372efaec) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "614.ic35",  0x10000, 0x10000, CRC(6422e8ba) SHA1(7bb4e5d5362d352c3fa70bf101d10b09f25a4c66) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x80000, "sprites", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "616.ic62",  0x00000, 0x10000, CRC(fbe9552d) SHA1(3c7c218f13c0a94bb624745d81d63db9423777ea) )
	ROM_CONTINUE(0x20000,0x10000)
	ROM_LOAD( "615.ic61",  0x10000, 0x10000, CRC(ae09d5c0) SHA1(9da5b824b148c1e1478e3f4b44ba4348376ed7d5) )
	ROM_CONTINUE(0x30000,0x10000)
	ROM_LOAD( "618.ic64",  0x40000, 0x10000, CRC(afb77461) SHA1(18707768a4768b579c94092a260e286d3214b977) )
	ROM_CONTINUE(0x60000,0x10000)
	ROM_LOAD( "617.ic63",  0x50000, 0x10000, CRC(5065cd35) SHA1(9a03c5af024fcae6b3371bb04be3e811ecc390d7) )
	ROM_CONTINUE(0x70000,0x10000)

	ROM_REGION( 0x40000, "bgtiles", 0 ) // 23C1000 28 pin mask ROM on base/A/B/C/D set
	ROM_LOAD( "619.ic2",  0x00000, 0x20000, CRC(9e2f8759) SHA1(2cc2f65b068c14e353e42f0b4adf921a97f0490a) )
	ROM_LOAD( "612.ic4",  0x20000, 0x10000, CRC(85057c81) SHA1(47663e17f08f47d847605c14e849266468ff39ba) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x10000, "m72", 0 ) // samples, matches base set
	ROM_LOAD( "vg_a-4d-d.ic26",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 ) // All are pal16l8 - protected
	ROM_LOAD( "vg_b-8r.ic90", 0x0000, 0x0117, CRC(df368a7a) SHA1(597d85d1f90b7ee0188f2d849792ee02ff2ea48b) )
	ROM_LOAD( "vg_b-4m.ic38", 0x0200, 0x0117, CRC(dbca4204) SHA1(d8e190f2dc4d6285f22be331d01ed402520d2017) )
	ROM_LOAD( "vg_b-1b.ic1",  0x0400, 0x0117, CRC(922e5167) SHA1(08efdfdfeb35f3f73b6fd3d5c0c2a386dea5f617) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10_7a.ic52", 0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // tbp24s10, 82s129-equivalent - video timing
ROM_END

/* Bootleg
   "JB 306" silkscreen part# on bottom board, "VT-20" sticker on top board.
   The various Nanao custom chips are implemented with standard TTL, makes the bottom board considerably larger.
   The 3 additional PROMs are part of the KNA6074601 custom chip implementation.
   The differing PAL has some unused pins repurposed as part of the KNA6032701 custom chip implementation.
   The board-board connectors are 2x 50-pin vs the 2x 60-pin used on original game. */
ROM_START( vigilantbl )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "g07_c03.bin",  0x00000, 0x08000, CRC(9dcca081) SHA1(6d086b70e6bf1fbafa746ef5c82334645f199be9) )
	ROM_LOAD( "j07_c04.bin",  0x10000, 0x10000, CRC(e0159105) SHA1(da6d74ec075863c67c0ce21b07a54029d138f688) )
	// 0x20000-0x2ffff empty

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "g05_c02.bin",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "f05_c08.bin",  0x00000, 0x10000, CRC(01579d20) SHA1(e58d8ca0ea0ac9d77225bf55faa499d1565924f9) )
	ROM_LOAD( "h05_c09.bin",  0x10000, 0x10000, CRC(4f5872f0) SHA1(6af21ba1c94097eecce30585983b4b07528c8635) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "n07_c12.bin",  0x00000, 0x10000, CRC(10af8eb2) SHA1(664b178b248babc43a9af0fe140fe57bc7367762) )
	ROM_LOAD( "k07_c10.bin",  0x10000, 0x10000, CRC(9576f304) SHA1(0ec2a7d3d82208e2a9a4ef9ab2824e6fe26ebbe5) )
	ROM_LOAD( "o07_c13.bin",  0x20000, 0x10000, CRC(b1d9d4dc) SHA1(1aacf6b0ff8d102880d3dce3b55cd1488edb90cf) )
	ROM_LOAD( "l07_c11.bin",  0x30000, 0x10000, CRC(4598be4a) SHA1(6b68ec94bdee0e58133a8d3891054ef44a8ff0e5) )
	ROM_LOAD( "t07_c16.bin",  0x40000, 0x10000, CRC(f5425e42) SHA1(c401263b6a266d3e9cd23133f1d823fb4b095e3d) )
	ROM_LOAD( "p07_c14.bin",  0x50000, 0x10000, CRC(cb50a17c) SHA1(eb15704f715b6475ae7096f8d82f1b20f8277c71) )
	ROM_LOAD( "v07_c17.bin",  0x60000, 0x10000, CRC(959ba3c7) SHA1(dcd2a885ae7b61210cbd55a38ccbe91c73d071b0) )
	ROM_LOAD( "s07_c15.bin",  0x70000, 0x10000, CRC(7f2e91c5) SHA1(27dcc9b696834897c36c0b7a1c6202d93f41ad8d) )

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "d01_c05.bin",  0x00000, 0x10000, CRC(81b1ee5c) SHA1(2014165ec71f089fecb5a3e60b939cc0f565d7f1) )
	ROM_LOAD( "e01_c06.bin",  0x10000, 0x10000, CRC(d0d33673) SHA1(39761d97a71deaf7f17233d5bd5a55dbb1e6b30e) )
	ROM_LOAD( "f01_c07.bin",  0x20000, 0x10000, CRC(aae81695) SHA1(ca8e136eca3543b27f3a61b105d4a280711cd6ea) )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "d04_c01.bin",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 ) // 3x pal16l8
	ROM_LOAD( "p09_16l8.bin", 0x0000, 0x0117, CRC(df368a7a) SHA1(597d85d1f90b7ee0188f2d849792ee02ff2ea48b) )  // == official set ic90
	ROM_LOAD( "m05_16l8.bin", 0x0200, 0x0117, CRC(dbca4204) SHA1(d8e190f2dc4d6285f22be331d01ed402520d2017) )  // == official set ic38
	ROM_LOAD( "b01_16l8.bin", 0x0400, 0x0104, CRC(1beae498) SHA1(031c2f589eb715dc3909614bab8d89994f69be80) )

	ROM_REGION( 0x0400, "proms", 0 ) // 4x 82s129
	ROM_LOAD( "a07_129.bin",  0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // == official set ic52
	ROM_LOAD( "t10_129a.bin", 0x0100, 0x0100, CRC(1513df33) SHA1(7ab5066e3b5eb47fc4d5498b168929a9ade9bb7c) )
	ROM_LOAD( "u10_129b.bin", 0x0200, 0x0100, CRC(06661d00) SHA1(aa12a31751cad355ad545d92485432d6be12b45e) )
	ROM_LOAD( "v10_129c.bin", 0x0300, 0x0100, CRC(3f186bc8) SHA1(e5270fbc16c5844294cf20b42e57f4edaabbe629) )
ROM_END


ROM_START( kikcubic )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "mqj-p0",       0x00000, 0x08000, CRC(9cef394a) SHA1(be9cc78420b4c35f8f9523b529bd56315749762c) )
	ROM_LOAD( "mqj-b0",       0x10000, 0x10000, CRC(d9bcf4cd) SHA1(f1f1cb8609343dae8637f115e5c96fd88a00f5eb) )
	ROM_LOAD( "mqj-b1",       0x20000, 0x10000, CRC(54a0abe1) SHA1(0fb1d050c1e299394609214c903bcf4cf11329ff) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mqj-sp",       0x00000, 0x10000, CRC(bbcf3582) SHA1(4a5b9d4161b26e3ca400573fa78268893e42d5db) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "mqj-c0",       0x00000, 0x10000, CRC(975585c5) SHA1(eb8245e458a5d4880add5b4a305a4468fa8f6491) )
	ROM_LOAD( "mqj-c1",       0x10000, 0x10000, CRC(49d9936d) SHA1(c4169ddd481c19e8e24457e2fe011db1b34db6d3) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "mqj-00",       0x00000, 0x40000, CRC(7fb0c58f) SHA1(f70ff39e2d648606686c87cf1a7a3ffb46c2656a) )
	ROM_LOAD( "mqj-10",       0x40000, 0x40000, CRC(3a189205) SHA1(063d664d4cf709931b5e3a5b6eb7c75bcd57b518) )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "mqj-v0",       0x00000, 0x10000, CRC(54762956) SHA1(f08e983af28b16d27505d465ca64e7c7a93373a4) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "8d",           0x0000, 0x0100, CRC(7379bb12) SHA1(cf0c4e27911505f937004ea5eac1154956ec5d3b) )    // unknown (timing?)
	ROM_LOAD( "6h",           0x0100, 0x0020, CRC(face0cbb) SHA1(c56aea3b7aaabbd4ff1b4546fcad94f51b473cde) )    // unknown (bad read?)
	ROM_LOAD( "7s",           0x0120, 0x0020, CRC(face0cbb) SHA1(c56aea3b7aaabbd4ff1b4546fcad94f51b473cde) )    // unknown (bad read?)
ROM_END

ROM_START( kikcubicb )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x00000, 0x08000, CRC(d3a589ba) SHA1(be2fa4515ed3510fec2b182a3ffcf5ddb9d7256d) )
	ROM_LOAD( "4.bin",        0x10000, 0x10000, CRC(9ae1e1a6) SHA1(7f3099206300eaa275b003e829dff0b7b91d8cc8) )
	ROM_LOAD( "5.bin",        0x20000, 0x08000, CRC(a5a6bffd) SHA1(372452c8c9b2c65307434af19eddcb60e7cd0fa3) )
	ROM_RELOAD(               0x28000, 0x08000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mqj-sp",       0x00000, 0x10000, CRC(bbcf3582) SHA1(4a5b9d4161b26e3ca400573fa78268893e42d5db) ) // 2.bin

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "7.bin",        0x00000, 0x10000, CRC(1788d51a) SHA1(bf7182379a34c366f192cb7d2494b26f6e27d97f) )
	ROM_LOAD( "mqj-c1",       0x10000, 0x10000, CRC(49d9936d) SHA1(c4169ddd481c19e8e24457e2fe011db1b34db6d3) ) // 6.bin

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "11.bin",       0x00000, 0x10000, CRC(0f0cac92) SHA1(32cf4b274b61d69a6d9f0ad39aa903c7a99b981d) )
	ROM_RELOAD(               0x20000, 0x10000 )
	ROM_LOAD( "10.bin",       0x10000, 0x10000, CRC(7d3822a8) SHA1(20e07a6edd46abf46b0d101a0ccee72f087f63b2) )
	ROM_RELOAD(               0x30000, 0x10000 )
	ROM_LOAD( "9.bin",        0x40000, 0x10000, CRC(56fb4fa3) SHA1(ed82602bfe98e60208d50f29f064c11cec01b3a7) )
	ROM_RELOAD(               0x60000, 0x10000 )
	ROM_LOAD( "8.bin",        0x50000, 0x10000, CRC(947dbd4e) SHA1(278ad7126bacb752886800cf48c6fe704427149d) )
	ROM_RELOAD(               0x70000, 0x10000 )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "mqj-v0",       0x00000, 0x10000, CRC(54762956) SHA1(f08e983af28b16d27505d465ca64e7c7a93373a4) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "8d",           0x0000, 0x0100, CRC(7379bb12) SHA1(cf0c4e27911505f937004ea5eac1154956ec5d3b) )    // unknown (timing?)
	ROM_LOAD( "6h",           0x0100, 0x0020, CRC(face0cbb) SHA1(c56aea3b7aaabbd4ff1b4546fcad94f51b473cde) )    // unknown (bad read?)
	ROM_LOAD( "7s",           0x0120, 0x0020, CRC(face0cbb) SHA1(c56aea3b7aaabbd4ff1b4546fcad94f51b473cde) )    // unknown (bad read?)
ROM_END


ROM_START( buccanrs )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "bc-011_k-163.u58",  0x00000, 0x10000, CRC(bf1d7e6f) SHA1(55dcf993515b57c3eb1fab98097a2171df3e38ed) ) // both halves are identical (correct for ROM type on this board tho)
	ROM_LOAD( "bc-012_k-163.u25",  0x10000, 0x10000, CRC(87303ba8) SHA1(49a25393e853b9adf7df00a6f9c38a526a02ea4e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bc-001_k-0161.u128",  0x00000, 0x10000, CRC(eb65f8c3) SHA1(82566becb630ce92303905dc0c5bef9e80e9caad) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "bc-003_k-0161.u212",  0x00000, 0x10000, CRC(95e3c517) SHA1(9954830ebc3a6414a3236f4e41981db082e5ea19) )
	ROM_LOAD( "bc-004_k-0161.u189",  0x10000, 0x10000, CRC(fe2377ab) SHA1(8578c5466d98f140fdfc41e91cd841e725786e32) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "bc-005_k-0161.u113",  0x00000, 0x10000, CRC(16dc435f) SHA1(0c13e9786b356770c84f94684697e43d0ea9e7cc) )
	ROM_CONTINUE(        0x20000, 0x10000 )
	ROM_LOAD( "bc-006_k-161.u80",   0x10000, 0x10000, CRC(4fe3bf97) SHA1(7910ace1eed80bfafa1f9f057ed67e23aa446a22) )
	ROM_LOAD( "bc-008_k-161.u52",   0x40000, 0x10000, CRC(078aef7f) SHA1(72e60d39d8af8bd31e9ae019b12620797eb0af7f) )
	ROM_CONTINUE(        0x60000, 0x10000 )
	ROM_LOAD( "bc-007_k-161.u70",   0x50000, 0x10000, CRC(f650fa90) SHA1(c87081b4d6b09f865d08c5120da3d0fb3196a2c3) )

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "bc-009_k-163.u49",  0x20000, 0x20000, CRC(0c6188fb) SHA1(d49034384c6d0e94db2890223b32a2a49e79a639) )
	ROM_LOAD( "bc-010_k-163.u27",  0x00000, 0x20000, CRC(2d383ff8) SHA1(3062baac27feba69c6ed94935c5ced72d89ed4fb) )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "bc-002_k-0161.u74",  0x00000, 0x10000, CRC(36ee1dac) SHA1(6dfd2a885c0b1c9347abc4b204ade66551c4b404) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "prom1.u54",  0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // == ic52 video timing prom from vigilante
	ROM_LOAD( "prom4.u79",  0x0100, 0x0100, CRC(e6506ef4) SHA1(079841da7640b14d94aaaeb572bf018932b58293) )
	ROM_LOAD( "prom3.u88",  0x0200, 0x0100, CRC(b43d094f) SHA1(2bed4892d8a91d7faac5a07bf858d9294eb30606) )
	ROM_LOAD( "prom2.u99",  0x0300, 0x0100, CRC(e0aa8869) SHA1(ac8bdfeba69420ba56ec561bf3d0f1229d02cea2) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.u103", 0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8.u156", 0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8.u42",  0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( buccanrsa )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "bc-011",            0x00000, 0x08000, CRC(6b657ef1) SHA1(a3356654d4b04177af23b39e924cc5ad64930bb6) )
	ROM_LOAD( "bc-012_k-163.u25",  0x10000, 0x10000, CRC(87303ba8) SHA1(49a25393e853b9adf7df00a6f9c38a526a02ea4e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bc-001_k-0161.u128",  0x00000, 0x10000, CRC(eb65f8c3) SHA1(82566becb630ce92303905dc0c5bef9e80e9caad) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "bc-003_k-0161.u212",  0x00000, 0x10000, CRC(95e3c517) SHA1(9954830ebc3a6414a3236f4e41981db082e5ea19) )
	ROM_LOAD( "bc-004_k-0161.u189",  0x10000, 0x10000, CRC(fe2377ab) SHA1(8578c5466d98f140fdfc41e91cd841e725786e32) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "bc-005_k-0161.u113",  0x00000, 0x10000, CRC(16dc435f) SHA1(0c13e9786b356770c84f94684697e43d0ea9e7cc) )
	ROM_CONTINUE(        0x20000, 0x10000 )
	ROM_LOAD( "bc-006_k-161.u80",   0x10000, 0x10000, CRC(4fe3bf97) SHA1(7910ace1eed80bfafa1f9f057ed67e23aa446a22) )
	ROM_LOAD( "bc-008_k-161.u52",   0x40000, 0x10000, CRC(078aef7f) SHA1(72e60d39d8af8bd31e9ae019b12620797eb0af7f) )
	ROM_CONTINUE(        0x60000, 0x10000 )
	ROM_LOAD( "bc-007_k-161.u70",   0x50000, 0x10000, CRC(f650fa90) SHA1(c87081b4d6b09f865d08c5120da3d0fb3196a2c3) )

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "bc-009_k-163.u49",  0x20000, 0x20000, CRC(0c6188fb) SHA1(d49034384c6d0e94db2890223b32a2a49e79a639) )
	ROM_LOAD( "bc-010_k-163.u27",  0x00000, 0x20000, CRC(2d383ff8) SHA1(3062baac27feba69c6ed94935c5ced72d89ed4fb) )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "bc-002_k-0161.u74",  0x00000, 0x10000, CRC(36ee1dac) SHA1(6dfd2a885c0b1c9347abc4b204ade66551c4b404) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "prom1.u54",  0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // == ic52 video timing prom from vigilante
	ROM_LOAD( "prom4.u79",  0x0100, 0x0100, CRC(e6506ef4) SHA1(079841da7640b14d94aaaeb572bf018932b58293) )
	ROM_LOAD( "prom3.u88",  0x0200, 0x0100, CRC(b43d094f) SHA1(2bed4892d8a91d7faac5a07bf858d9294eb30606) )
	ROM_LOAD( "prom2.u99",  0x0300, 0x0100, CRC(e0aa8869) SHA1(ac8bdfeba69420ba56ec561bf3d0f1229d02cea2) )
ROM_END


ROM_START( buccanrsb )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "rr_du.u58",         0x00000, 0x08000, CRC(dcad3a8b) SHA1(e961927bdff28db18b829ce3f64051ff1604d1e6) )
	ROM_LOAD( "bc-012_k-163.u25",  0x10000, 0x10000, CRC(87303ba8) SHA1(49a25393e853b9adf7df00a6f9c38a526a02ea4e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bc-001_k-0161.u128",  0x00000, 0x10000, CRC(eb65f8c3) SHA1(82566becb630ce92303905dc0c5bef9e80e9caad) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "bc-003_k-0161.u212",  0x00000, 0x10000, CRC(95e3c517) SHA1(9954830ebc3a6414a3236f4e41981db082e5ea19) )
	ROM_LOAD( "bc-004_k-0161.u189",  0x10000, 0x10000, CRC(fe2377ab) SHA1(8578c5466d98f140fdfc41e91cd841e725786e32) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "bc-005_k-0161.u113",  0x00000, 0x10000, CRC(16dc435f) SHA1(0c13e9786b356770c84f94684697e43d0ea9e7cc) )
	ROM_CONTINUE(        0x20000, 0x10000 )
	ROM_LOAD( "bc-006_k-161.u80",   0x10000, 0x10000, CRC(4fe3bf97) SHA1(7910ace1eed80bfafa1f9f057ed67e23aa446a22) )
	ROM_LOAD( "bc-008_k-161.u52",   0x40000, 0x10000, CRC(078aef7f) SHA1(72e60d39d8af8bd31e9ae019b12620797eb0af7f) )
	ROM_CONTINUE(        0x60000, 0x10000 )
	ROM_LOAD( "bc-007_k-161.u70",   0x50000, 0x10000, CRC(f650fa90) SHA1(c87081b4d6b09f865d08c5120da3d0fb3196a2c3) )

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "bc-009_k-163.u49",  0x20000, 0x20000, CRC(0c6188fb) SHA1(d49034384c6d0e94db2890223b32a2a49e79a639) )
	ROM_LOAD( "bc-010_k-163.u27",  0x00000, 0x20000, CRC(2d383ff8) SHA1(3062baac27feba69c6ed94935c5ced72d89ed4fb) )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "bc-002_k-0161.u74",  0x00000, 0x10000, CRC(36ee1dac) SHA1(6dfd2a885c0b1c9347abc4b204ade66551c4b404) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "prom1.u54",  0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) ) // == ic52 video timing prom from vigilante
	ROM_LOAD( "prom4.u79",  0x0100, 0x0100, CRC(e6506ef4) SHA1(079841da7640b14d94aaaeb572bf018932b58293) )
	ROM_LOAD( "prom3.u88",  0x0200, 0x0100, CRC(b43d094f) SHA1(2bed4892d8a91d7faac5a07bf858d9294eb30606) )
	ROM_LOAD( "prom2.u99",  0x0300, 0x0100, CRC(e0aa8869) SHA1(ac8bdfeba69420ba56ec561bf3d0f1229d02cea2) )
ROM_END


/*
PCB silkscreened DABONG-3

2 x Z80B
1 Motorola MC68705R3P + 11.bin on daughterboard riser
2 x Yamaha YM2203C
2 x Yamaha Y3014B DAC
2 x 8 position DipSwitch
18.432MHz OSC
5.0000MHz OSC

It appears the MC68705R3P only does the decryption.
*/

ROM_START( captainx )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "11.bin",  0x00000, 0x10000, CRC(1a80f2ba) SHA1(028f80731d2922ffd005e5ba1e957bc9de22cec7) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "12.bin",  0x10000, 0x10000, CRC(87303ba8) SHA1(49a25393e853b9adf7df00a6f9c38a526a02ea4e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "1.bin",  0x00000, 0x10000, CRC(eb65f8c3) SHA1(82566becb630ce92303905dc0c5bef9e80e9caad) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "68705r3",  0x00000, 0x1000, CRC(33b737f9) SHA1(a7e8ce1d82c642d70300320947c94acc6035afc8) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "7.bin",  0x00000, 0x10000, CRC(95e3c517) SHA1(9954830ebc3a6414a3236f4e41981db082e5ea19) )
	ROM_LOAD( "8.bin",  0x10000, 0x10000, CRC(fe2377ab) SHA1(8578c5466d98f140fdfc41e91cd841e725786e32) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "3.bin",   0x00000, 0x10000, CRC(ebe12ed8) SHA1(f66c1839972d039e2bc7f09c5632506afbb5514f) )
	ROM_CONTINUE(        0x20000, 0x10000 )
	ROM_LOAD( "4.bin",   0x10000, 0x10000, CRC(4fe3bf97) SHA1(7910ace1eed80bfafa1f9f057ed67e23aa446a22) )
	ROM_LOAD( "6.bin",   0x40000, 0x10000, CRC(078aef7f) SHA1(72e60d39d8af8bd31e9ae019b12620797eb0af7f) )
	ROM_CONTINUE(        0x60000, 0x10000 )
	ROM_LOAD( "5.bin",   0x50000, 0x10000, CRC(f650fa90) SHA1(c87081b4d6b09f865d08c5120da3d0fb3196a2c3) )

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "9.bin",   0x20000, 0x20000, CRC(0c6188fb) SHA1(d49034384c6d0e94db2890223b32a2a49e79a639) )
	ROM_LOAD( "10.bin",  0x00000, 0x20000, CRC(2d383ff8) SHA1(3062baac27feba69c6ed94935c5ced72d89ed4fb) )

	ROM_REGION( 0x10000, "m72", 0 ) // samples
	ROM_LOAD( "2.bin",  0x00000, 0x10000, CRC(36ee1dac) SHA1(6dfd2a885c0b1c9347abc4b204ade66551c4b404) )

	ROM_REGION( 0x400, "proms", 0 ) /* NOT dumped from this PCB, assumed to be the same */
	ROM_LOAD( "prom1.u54",  0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) )
	ROM_LOAD( "prom4.u79",  0x0100, 0x0100, CRC(e6506ef4) SHA1(079841da7640b14d94aaaeb572bf018932b58293) )
	ROM_LOAD( "prom3.u88",  0x0200, 0x0100, CRC(b43d094f) SHA1(2bed4892d8a91d7faac5a07bf858d9294eb30606) )
	ROM_LOAD( "prom2.u99",  0x0300, 0x0100, CRC(e0aa8869) SHA1(ac8bdfeba69420ba56ec561bf3d0f1229d02cea2) )
ROM_END


ROM_START( bowmen )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "4_27256.bin",   0x00000, 0x08000, CRC(e8dcabb6) SHA1(d6baf9af0ebdced8b73a36486ffd9b48b0d446ca) )
	ROM_LOAD( "3_27c512.bin",  0x10000, 0x10000, CRC(9c8af1eb) SHA1(e36efb2b6d1df4b313bf81c083df64ccf6764bbc) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "2_27c512.bin",  0x00000, 0x10000, CRC(477a5756) SHA1(675e066adec5fa34c491d71520827d568a5f8d7a) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "8_27c512.bin",  0x00000, 0x10000, CRC(782b0b7f) SHA1(35eef9bd32ca2063c730026a71300a7cf2e55778) )
	ROM_LOAD( "7_27c512.bin",  0x10000, 0x10000, CRC(449720ea) SHA1(aa52ea17293ab86c87eeb04c0c4172001ec56379) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "10_27c512.bin", 0x00000, 0x10000, CRC(8cf4e040) SHA1(641a5d13a63ca0dc56fef42ea1ada3b5b28ca864) )
	ROM_LOAD( "9_27c512.bin",  0x10000, 0x10000, CRC(3c06fdf7) SHA1(cfce7eda1e6353b850777c38cabc304e1b123ea0) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD( "5_27c040.bin",  0x00000, 0x80000, CRC(5bac2567) SHA1(bd773761ce31192efc69d031fee13a95ab13b2fd) )
	ROM_LOAD( "6_27c4001.bin", 0x80000, 0x80000, CRC(35b3b7bc) SHA1(2b7dc0606deb52e5ac725cd63cd56e1e2ef040f8) )

	ROM_REGION( 0x10000, "m72", 0 )  // samples
	ROM_LOAD( "1_27c512.bin",  0x00000, 0x10000, CRC(456c478b) SHA1(c8c8bf682dcc2b3a28ffbadcfaaba524141f792b) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "prom_4_82s129.bin",  0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) )  // video timing PROM
	ROM_LOAD( "prom_3_82s129.bin",  0x0100, 0x0100, CRC(e6506ef4) SHA1(079841da7640b14d94aaaeb572bf018932b58293) )  // unknown
	ROM_LOAD( "prom_2_82s129.bin",  0x0200, 0x0100, CRC(b43d094f) SHA1(2bed4892d8a91d7faac5a07bf858d9294eb30606) )  // unknown
	ROM_LOAD( "prom_1_82s129.bin",  0x0300, 0x0100, CRC(e0aa8869) SHA1(ac8bdfeba69420ba56ec561bf3d0f1229d02cea2) )  // unknown

	ROM_REGION( 0x2000, "plds", 0 )
	ROM_LOAD( "1_16v8.bin",   0x0000, 0x0117, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "2_22v10.bin",  0x0200, 0x0200, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "3_20v8.bin",   0x0400, 0x0157, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "4_16v8.bin",   0x0600, 0x0117, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "5_22v10.bin",  0x0800, 0x0200, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "6_16v8.bin",   0x0a00, 0x0117, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "7_22v10.bin",  0x0c00, 0x0200, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "8_16v8.bin",   0x0e00, 0x0117, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "9_16v8.bin",   0x1000, 0x0117, NO_DUMP )  // PLD is read protected
ROM_END


void vigilant_state::init_bowmen()
{
	uint8_t *rom = memregion("maincpu")->base();
	int const size = memregion("maincpu")->bytes();

	m_rear_pages = 16;

	for (int i = 0x0000; i < size; i++)
	{
		rom[i]= bitswap(rom[i], 2, 5, 0, 7, 6, 1, 4, 3);  // 0->5, 1->2, 2->7, 3->0, 4->1, 5->6, 6->3, 7->4
	}
}

} // anonymous namespace


GAME( 1988, vigilant,   0,          vigilant, vigilant, vigilant_state, empty_init,  ROT0, "Irem",                     "Vigilante (World, Rev E)",   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, vigilantg,  vigilant,   vigilant, vigilant, vigilant_state, empty_init,  ROT0, "Irem (Data East license)", "Vigilante (US, Rev G)",      MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, vigilanto,  vigilant,   vigilant, vigilant, vigilant_state, empty_init,  ROT0, "Irem (Data East license)", "Vigilante (US)",             MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, vigilanta,  vigilant,   vigilant, vigilant, vigilant_state, empty_init,  ROT0, "Irem",                     "Vigilante (World, Rev A)",   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, vigilantb,  vigilant,   vigilant, vigilant, vigilant_state, empty_init,  ROT0, "Irem (Data East license)", "Vigilante (US, Rev B)",      MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, vigilantc,  vigilant,   vigilant, vigilant, vigilant_state, empty_init,  ROT0, "Irem",                     "Vigilante (World, Rev C)",   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, vigilantd,  vigilant,   vigilant, vigilant, vigilant_state, empty_init,  ROT0, "Irem",                     "Vigilante (Japan, Rev D)",   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, vigilantbl, vigilant,   vigilant, vigilant, vigilant_state, empty_init,  ROT0, "bootleg",                  "Vigilante (bootleg)",        MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1988, kikcubic,   0,          kikcubic, kikcubic, vigilant_state, empty_init,  ROT0, "Irem",                     "Meikyu Jima (Japan)",        MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // English title is Kickle Cubicle
GAME( 1988, kikcubicb,  kikcubic,   kikcubic, kikcubic, vigilant_state, empty_init,  ROT0, "bootleg",                  "Kickle Cubele",              MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1989, buccanrs,   0,          buccanrs, buccanrs, vigilant_state, empty_init,  ROT0, "Duintronic",               "Buccaneers (set 1)",         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, buccanrsa,  buccanrs,   buccanrs, buccanra, vigilant_state, empty_init,  ROT0, "Duintronic",               "Buccaneers (set 2)",         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, buccanrsb,  buccanrs,   buccanrs, buccanrs, vigilant_state, empty_init,  ROT0, "Duintronic",               "Buccaneers (set 3, harder)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, captainx,   buccanrs,   captainx, buccanrs, captainx_state, empty_init,  ROT0, "bootleg (Electronics)",    "Captain X",                  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1994, bowmen,     0,          bowmen,   bowmen,   vigilant_state, init_bowmen, ROT0, "Ten-Level",                "Bowmen",                     MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
