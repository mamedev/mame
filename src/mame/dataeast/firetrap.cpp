// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Stephane Humbert
/***************************************************************************

Fire Trap memory map

driver by Nicola Salmoria

Z80:
0000-7fff ROM
8000-bfff Banked ROM (4 banks)
c000-cfff RAM
d000-d7ff bg #1 video/color RAM (alternating pages 0x100 long)
d000-dfff bg #2 video/color RAM (alternating pages 0x100 long)
e000-e3ff fg video RAM
e400-e7ff fg color RAM
e800-e97f sprites RAM

memory mapped ports:
read:
f010      IN0
f011      IN1
f012      IN2
f013      DSW0
f014      DSW1
f015      from pin 10 of 8751 controller
f016      from port #1 of 8751 controller

write:
f000      IRQ acknowledge
f001      sound command (also causes NMI on sound CPU)
f002      ROM bank selection
f003      flip screen
f004      NMI disable
f005      to port #2 of 8751 controller (signal on P3.2)
f008-f009 bg #1 x scroll
f00a-f00b bg #1 y scroll
f00c-f00d bg #2 x scroll
f00e-f00f bg #2 y scroll

interrupts:
VBlank triggers NMI.
the 8751 triggers IRQ

6502:
0000-07ff RAM
4000-7fff Banked ROM (2 banks)
8000-ffff ROM

read:
3400      command from the main CPU

write:
1000-1001 YM3526
2000      ADPCM data for the MSM5205 chip
2400      bit 0 = to sound chip MSM5205 (1 = play sample); bit 1 = IRQ enable
2800      ROM bank select



PCB Layouts (Japan version)
---------------------------

Top Board:

DE-0246-2
|--------------------------------------------|
| 8MHz 8751              FI-04.17C           |
|      (^=FI-13.16H)                         |
|                                           |-|
|                                           | |
|                                6116       | |
|                                           | |
|                                           | |
|                                           | |
|                                           |-|
|                                   Z80B     |
| 6116         6116                          |
|                                           |-|
|                                           | |
|                                           | |
|                                           | |
|                                    6164   | |
| FI-12.6J     FI-08.6E                     | |
| FI-11.4J     FI-07.4E            FI-03.4A |-|
| FI-10.3J     FI-06.3E            FI-02.3A  |
| FI-09.2J     FI-05.2E            FI-01.2A  |
|--------------------------------------------|
Notes:
      Z80    - clock 6.000MHz [12/2]
      i8751  - clock 8.000MHz


Bottom Board:

DE-0247-2
|--------------------------------------------------------|
|                    FI-17.17H         12MHz             |
|                    FI-16.15H             DECO_VSC30    |
|                                                       |-|
|                    FI-15.14H                          | |
| RCDM-I5                                  DECO_HMC20   | |
| RCDM-I1            FI-14.13H  |---------|             | |
| RCDM-I1  FI-19.12J            |DECO     |             | |
|J  DSW2                        |TC15G032AY             | |
|A         FI-18.10J            |DSPC10   |             |-|
|M                              |         |              |
|M                              |---------|              |
|A  DSW1   6116                                         |-|
| RCDM-I1             R6502                 2018   2018 | |
| RCDM-I1                    2018                       | |
|                                                       | |
| M5205   YM3526                                        | |
|                                           FI-3.4B     | |
|   VOL                                     FI-2.3B     |-|
| MC3403  YM3014      2018            2018               |
| MB3730                                         FI-1.1A |
|--------------------------------------------------------|
Notes:
      FI-1.1A - 82S129 Bipolar PROM
      FI-2.3B - 82S135 Bipolar PROM
      FI-3.4B - 82S129 Bipolar PROM

      6502   - clock 1.500MHz [12/8]
      YM3526 - clock 3.000MHz [12/4]
      M5205  - clock 375kHz [12/32]. Sample rate: Pin 1 High & Pin 2 Low = 0.375/48 = 7.8125kHz

      Measurements
      ------------
      XTAL1 - 11.9911MHz
      HSync - 15.6137kHz
      VSync - 57.4034Hz


Stephh's notes (based on the games Z80 code and some tests) :

1) 'firetrap' :

  - US version, licensed to Data East.
  - No warning screen.
  - Instructions in English
  - Initials : 3 letters.
  - Bugs in test mode :
      * when lives are set to "2", it displays "1".
      * bonus lives is still the same as the Japan version even if table at 0x0ca3 has changed.

2) 'firetrapj' :

  - Japan version.
  - Additional warning screen.
  - Instructions in Japanese
  - Initials : 5 letters.
  - Bugs in test mode :
      * when lives are set to "2", it displays "1".

3) 'firetrapbl' :

  - Bootleg based on 'firetrapj'.
  - Most MCU writes are patched while reads are handled differently.
  - Additional ROM with code at 0xf800 to simulate the MCU initialisation routine.
  - No warning screen.
  - Instructions in Japanese
  - Initials : 5 letters.
  - Bugs in test mode :
      * when lives are set to "2", it displays "1".

Many interrupt-related gates, flip-flops and connections are crossed out
on the schematics. Coins were once supposed to trigger IRQs on the main
CPU, and the YM3526 was intended to drive the 6502 IRQ line directly, with
the MSM5205-derived interrupt assigned to the NMI line instead.

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_adpcm_select(*this, "adpcm_select"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bgvideoram(*this, "bgvideoram%u", 1U),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_audiobank(*this, "audiobank")
	{ }

	void base(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;

	uint8_t m_nmi_enable = 0;

	void base_main_map(address_map &map) ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_select;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_mainbank;
	required_memory_bank m_audiobank;

	// video-related
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap[2]{};
	uint8_t m_scroll_x[2][2]{};
	uint8_t m_scroll_y[2][2]{};

	// misc
	uint8_t m_sound_irq_enable = 0U;
	uint8_t m_adpcm_toggle = 0;

	void nmi_disable_w(uint8_t data);
	void bankselect_w(uint8_t data);
	void irqack_w(uint8_t data);
	void sound_command_w(uint8_t data);
	void sound_flip_flop_w(uint8_t data);
	void sound_bankselect_w(uint8_t data);
	void adpcm_data_w(uint8_t data);
	void flip_screen_w(uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void bgvideoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void bg_scrollx_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void bg_scrolly_w(offs_t offset, uint8_t data);
	TILEMAP_MAPPER_MEMBER(get_fg_memory_offset);
	TILEMAP_MAPPER_MEMBER(get_bg_memory_offset);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void adpcm_int(int state);

	void sound_map(address_map &map) ATTR_COLD;
};

class original_state : public base_state
{
public:
	original_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_mcu(*this, "mcu"),
		m_coins(*this, "COINS")
	{ }

	void firetrap(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices
	required_device<i8751_device> m_mcu;

	required_ioport m_coins;

	uint8_t m_mcu_p3 = 0;
	uint8_t m_maincpu_to_mcu = 0;
	uint8_t m_mcu_to_maincpu = 0;

	uint8_t mcu_r();
	void mcu_w(uint8_t data);
	uint8_t mcu_p0_r();
	void mcu_p3_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void main_map(address_map &map) ATTR_COLD;
};

class bootleg_state : public base_state
{
public:
	bootleg_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_in2(*this, "IN2")
	{ }

	void firetrapbl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// misc
	uint8_t m_coin_command_pending = 0;

	required_ioport m_in2;

	uint8_t coin_r();
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void main_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Fire Trap has one 256x8 and one 256x4 palette PROMs.
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

void base_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0 = (color_prom[i] >> 0) & 0x01;
		int bit1 = (color_prom[i] >> 1) & 0x01;
		int bit2 = (color_prom[i] >> 2) & 0x01;
		int bit3 = (color_prom[i] >> 3) & 0x01;
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i + palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[i + palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[i + palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[i + palette.entries()] >> 3) & 0x01;
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(base_state::get_fg_memory_offset)
{
	return (row ^ 0x1f) + (col << 5);
}

TILEMAP_MAPPER_MEMBER(base_state::get_bg_memory_offset)
{
	return ((row & 0x0f) ^ 0x0f) | ((col & 0x0f) << 4) |
			// hole at bit 8
			((row & 0x10) << 5) | ((col & 0x10) << 6);
}

TILE_GET_INFO_MEMBER(base_state::get_fg_tile_info)
{
	int code = m_fgvideoram[tile_index];
	int color = m_fgvideoram[tile_index + 0x400];
	tileinfo.set(0,
			code | ((color & 0x01) << 8),
			color >> 4,
			0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(base_state::get_bg_tile_info)
{
	int code = m_bgvideoram[Which][tile_index];
	int color = m_bgvideoram[Which][tile_index + 0x100];
	tileinfo.set(Which + 1,
			code + ((color & 0x03) << 8),
			(color & 0x30) >> 4,
			TILE_FLIPXY((color & 0x0c) >> 2));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void base_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(base_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(base_state::get_fg_memory_offset)), 8, 8, 32, 32);
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(base_state::get_bg_tile_info<0>)), tilemap_mapper_delegate(*this, FUNC(base_state::get_bg_memory_offset)), 16, 16, 32, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(base_state::get_bg_tile_info<1>)), tilemap_mapper_delegate(*this, FUNC(base_state::get_bg_memory_offset)), 16, 16, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap[0]->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void base_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

template <uint8_t Which>
void base_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[Which][offset] = data;
	m_bg_tilemap[Which]->mark_tile_dirty(offset & 0x6ff);
}

template <uint8_t Which>
void base_state::bg_scrollx_w(offs_t offset, uint8_t data)
{
	m_scroll_x[Which][offset] = data;
	m_bg_tilemap[Which]->set_scrollx(0, m_scroll_x[Which][0] | (m_scroll_x[Which][1] << 8));
}

template <uint8_t Which>
void base_state::bg_scrolly_w(offs_t offset, uint8_t data)
{
	m_scroll_y[Which][offset] = data;
	m_bg_tilemap[Which]->set_scrolly(0, -(m_scroll_y[Which][0] | (m_scroll_y[Which][1] << 8)));
}


/***************************************************************************

  Display refresh

***************************************************************************/

void base_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		// the meaning of bit 3 of [offs] is unknown

		int sy = m_spriteram[offs];
		int sx = m_spriteram[offs + 2];
		int code = m_spriteram[offs + 3] + 4 * (m_spriteram[offs + 1] & 0xc0);
		int color = ((m_spriteram[offs + 1] & 0x08) >> 2) | (m_spriteram[offs + 1] & 0x01);
		int flipx = m_spriteram[offs + 1] & 0x04;
		int flipy = m_spriteram[offs + 1] & 0x02;
		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (m_spriteram[offs + 1] & 0x10)    // double width
		{
			if (flip_screen()) sy -= 16;

			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
					code & ~1,
					color,
					flipx, flipy,
					sx, flipy ? sy : sy + 16, 0);
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
					code | 1,
					color,
					flipx, flipy,
					sx, flipy ? sy + 16 : sy, 0);

			// redraw with wraparound
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
					code & ~1,
					color,
					flipx, flipy,
					sx - 256, flipy ? sy : sy + 16, 0);
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
					code | 1,
					color,
					flipx, flipy,
					sx - 256, flipy ? sy + 16 : sy, 0);
		}
		else
		{
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					sx, sy, 0);

			// redraw with wraparound
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					sx - 256, sy, 0);
		}
	}
}

uint32_t base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void base_state::nmi_disable_w(uint8_t data)
{
	m_nmi_enable = ~data & 1;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void base_state::bankselect_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x03);
}

void base_state::irqack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

//**************************************************************************
//  PROTECTION MCU
//**************************************************************************

uint8_t original_state::mcu_r()
{
	return m_mcu_to_maincpu;
}

void original_state::mcu_w(uint8_t data)
{
	m_maincpu_to_mcu = data;
	m_mcu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
}

uint8_t original_state::mcu_p0_r()
{
	// 7654----  unused
	// ----3---  coin2
	// -----2--  coin1
	// ------1-  service coin
	// -------0  coin inserted

	int coin_inserted = ((m_coins->read() & 0x0e) == 0x0e) ? 1 : 0;

	return (m_coins->read() & 0x0e) | coin_inserted;
}

void original_state::mcu_p3_w(uint8_t data)
{
	// 765-----  unused
	// ---4----  coin flip-flop reset
	// ----3---  mcu int1
	// -----2--  mcu int0
	// ------1-  mcu int0 ack
	// -------0  maincpu int

	if (BIT(m_mcu_p3, 0) == 1 && BIT(data, 0) == 0)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

	if (BIT(m_mcu_p3, 1) == 1 && BIT(data, 1) == 0)
		m_mcu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);

//  if (BIT(m_mcu_p3, 4) == 1 && BIT(data, 4) == 0)

	m_mcu_p3 = data;
}

uint8_t bootleg_state::coin_r()
{
	/* Check for coin insertion
	   the following only works in the bootleg version, which doesn't have an
	   8751 - the real thing is much more complicated than that. */
	uint8_t coin = 0;
	uint8_t port = m_in2->read() & 0x70;

	if (m_maincpu->pc() == 0x1188)
		return ~m_coin_command_pending;

	if (port != 0x70)
	{
		if (!(port & 0x20)) // COIN1
			coin = 1;
		if (!(port & 0x40)) // COIN2
			coin = 2;
		if (!(port & 0x10)) // SERVICE1
			coin = 3;
		m_coin_command_pending = coin;
		return 0xff;
	}

	return 0;
}

void base_state::sound_flip_flop_w(uint8_t data)
{
	m_msm->reset_w(!BIT(data, 0));
	m_sound_irq_enable = BIT(data, 1);
	if (!m_sound_irq_enable)
		m_audiocpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

void base_state::sound_bankselect_w(uint8_t data)
{
	m_audiobank->set_entry(data & 0x01);
}

void base_state::adpcm_int(int state)
{
	if (state)
	{
		m_adpcm_toggle ^= 1;
		m_adpcm_select->select_w(m_adpcm_toggle);

		if (m_sound_irq_enable && m_adpcm_toggle)
			m_audiocpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	}
}

void base_state::adpcm_data_w(uint8_t data)
{
	m_audiocpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	m_adpcm_select->ba_w(data);
}

void base_state::flip_screen_w(uint8_t data)
{
	flip_screen_set(data);
}

void base_state::base_main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(base_state::bgvideoram_w<0>)).share(m_bgvideoram[0]);
	map(0xd800, 0xdfff).ram().w(FUNC(base_state::bgvideoram_w<1>)).share(m_bgvideoram[1]);
	map(0xe000, 0xe7ff).ram().w(FUNC(base_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xe800, 0xe97f).ram().share(m_spriteram);
	map(0xf000, 0xf000).w(FUNC(base_state::irqack_w));
	map(0xf001, 0xf001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xf002, 0xf002).w(FUNC(base_state::bankselect_w));
	map(0xf003, 0xf003).w(FUNC(base_state::flip_screen_w));
	map(0xf004, 0xf004).w(FUNC(base_state::nmi_disable_w));
	map(0xf008, 0xf009).w(FUNC(base_state::bg_scrollx_w<0>));
	map(0xf00a, 0xf00b).w(FUNC(base_state::bg_scrolly_w<0>));
	map(0xf00c, 0xf00d).w(FUNC(base_state::bg_scrollx_w<1>));
	map(0xf00e, 0xf00f).w(FUNC(base_state::bg_scrolly_w<1>));
	map(0xf010, 0xf010).portr("IN0");
	map(0xf011, 0xf011).portr("IN1");
	map(0xf012, 0xf012).portr("IN2");
	map(0xf013, 0xf013).portr("DSW0");
	map(0xf014, 0xf014).portr("DSW1");
}

void original_state::main_map(address_map &map)
{
	base_main_map(map);

	map(0xf005, 0xf005).w(FUNC(original_state::mcu_w));
	map(0xf016, 0xf016).r(FUNC(original_state::mcu_r));
}

void bootleg_state::main_map(address_map &map)
{
	base_main_map(map);

	map(0xf016, 0xf016).r(FUNC(bootleg_state::coin_r));
	map(0xf800, 0xf8ff).rom(); // extra ROM in the bootleg with unprotection code
}

void base_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x1001).w("ymsnd", FUNC(ym3526_device::write));
	map(0x2000, 0x2000).w(FUNC(base_state::adpcm_data_w));
	map(0x2400, 0x2400).w(FUNC(base_state::sound_flip_flop_w));
	map(0x2800, 0x2800).w(FUNC(base_state::sound_bankselect_w));
	map(0x3400, 0x3400).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x4000, 0x7fff).bankr(m_audiobank);
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}

// verified from Z80 code
static INPUT_PORTS_START( firetrap )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )     PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )   PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )   PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )  PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )  PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )     PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )   PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )   PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )        // Manual shows 1C_5C
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )                         // "1" in the "test mode" but manual states "Infinite"
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5,6") // table at 0x0ca3 - 4*30 bytes
	PORT_DIPSETTING(    0x10, "30k and 70k" )
	PORT_DIPSETTING(    0x00, "50k and 100k" )
	PORT_DIPSETTING(    0x30, "30k only" )
	PORT_DIPSETTING(    0x20, "50k only" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("COINS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) // any coin input active
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2)
INPUT_PORTS_END

// verified from Z80 code
static INPUT_PORTS_START( firetrapj )
	PORT_INCLUDE( firetrap )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5,6") // table at 0x0ca3 - 4*30 bytes
	PORT_DIPSETTING(    0x30, "50k & Every 70k" )             // last bonus life at 960k
	PORT_DIPSETTING(    0x20, "60k & Every 80k" )             // last bonus life at 940k
	PORT_DIPSETTING(    0x10, "80k & Every 100k" )            // last bonus life at 980k
	PORT_DIPSETTING(    0x00, "50k only" )
INPUT_PORTS_END

// verified from Z80 code
static INPUT_PORTS_START( firetrapbl )
	PORT_INCLUDE( firetrapj )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_MODIFY("COINS")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, 4 },
	{ 3, 2, 1, 0, RGN_FRAC(1,2)+3, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 3, 2, 1, 0, RGN_FRAC(1,4)+3, RGN_FRAC(1,4)+2, RGN_FRAC(1,4)+1, RGN_FRAC(1,4)+0,
			16*8+3, 16*8+2, 16*8+1, 16*8+0, RGN_FRAC(1,4)+16*8+3, RGN_FRAC(1,4)+16*8+2, RGN_FRAC(1,4)+16*8+1, RGN_FRAC(1,4)+16*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*8
};

static GFXDECODE_START( gfx_firetrap )
	GFXDECODE_ENTRY( "chars",   0, charlayout,   0x00, 16 )    // colors 0x00-0x3f
	GFXDECODE_ENTRY( "tiles1",  0, tilelayout,   0x80,  4 )    // colors 0x80-0xbf
	GFXDECODE_ENTRY( "tiles2",  0, tilelayout,   0xc0,  4 )    // colors 0xc0-0xff
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0x40,  4 )    // colors 0x40-0x7f
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(original_state::interrupt)
{
	if (param == 0 && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	if (param == 0)
		m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);

	if (param == 1)
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(bootleg_state::interrupt)
{
	if (param == 0 && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void base_state::machine_start()
{
	uint8_t *main = memregion("maincpu")->base();
	uint8_t *sound = memregion("audiocpu")->base();

	m_mainbank->configure_entries(0, 4, &main[0x10000], 0x4000);
	m_audiobank->configure_entries(0, 2, &sound[0x8000], 0x4000);

	save_item(NAME(m_sound_irq_enable));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_adpcm_toggle));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
}

void original_state::machine_start()
{
	base_state::machine_start();

	save_item(NAME(m_mcu_p3));
	save_item(NAME(m_maincpu_to_mcu));
	save_item(NAME(m_mcu_to_maincpu));
}

void bootleg_state::machine_start()
{
	base_state::machine_start();

	save_item(NAME(m_coin_command_pending));
}

void base_state::machine_reset()
{
	for (int i = 0; i < 2; i++)
	{
		m_scroll_x[0][i] = 0;
		m_scroll_y[0][i] = 0;
		m_scroll_x[1][i] = 0;
		m_scroll_y[1][i] = 0;
	}

	m_sound_irq_enable = 0;
	m_nmi_enable = 0;
	m_adpcm_toggle = 0;
}

void bootleg_state::machine_reset()
{
	base_state::machine_reset();

	m_coin_command_pending = 0;
}

void base_state::base(machine_config &config)
{
	constexpr XTAL FIRETRAP_XTAL = XTAL(12'000'000);

	// basic machine hardware
	Z80(config, m_maincpu, FIRETRAP_XTAL / 2);    // 6 MHz

	M6502(config, m_audiocpu, FIRETRAP_XTAL / 8); // 1.5 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &base_state::sound_map);
	// IRQs are caused by the ADPCM chip
	// NMIs are caused by the main CPU

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(57.4034); // PCB measurement
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
//  screen.set_size(32*8, 32*8);
//  screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	// DECO video CRTC, unverified
	screen.set_raw(FIRETRAP_XTAL / 2, 384, 0, 256, 272, 8, 248);
	screen.set_screen_update(FUNC(base_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_firetrap);
	PALETTE(config, m_palette, FUNC(base_state::palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym3526_device &ymsnd(YM3526(config, "ymsnd", FIRETRAP_XTAL / 4));    // 3 MHz
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	LS157(config, m_adpcm_select, 0);
	m_adpcm_select->out_callback().set("msm", FUNC(msm5205_device::data_w));

	MSM5205(config, m_msm, FIRETRAP_XTAL / 32);   // 375 kHz
	m_msm->vck_callback().set(FUNC(base_state::adpcm_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  // 7.8125kHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.30);
}

void original_state::firetrap(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &original_state::main_map);

	TIMER(config, "scantimer", 0).configure_scanline(FUNC(original_state::interrupt), "screen", 0, 1);

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(original_state::mcu_p0_r));
	m_mcu->port_out_cb<1>().set([this] (u8 data) { m_mcu_to_maincpu = data; });
	m_mcu->port_in_cb<2>().set([this] () { return m_maincpu_to_mcu; });
	m_mcu->port_out_cb<3>().set(FUNC(original_state::mcu_p3_w));

	// needs a tight sync with the MCU
	config.set_perfect_quantum(m_maincpu);
}

void bootleg_state::firetrapbl(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &bootleg_state::main_map);

	TIMER(config, "scantimer", 0).configure_scanline(FUNC(bootleg_state::interrupt), "screen", 0, 1);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( firetrap )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "di-02.4a",     0x00000, 0x8000, CRC(3d1e4bf7) SHA1(ee903b469619f49edb1727fb545c9a6085f50746) )
	ROM_LOAD( "di-01.3a",     0x10000, 0x8000, CRC(9bbae38b) SHA1(dc1d3ed5da71bfb104fd54fc70c56833f31d281f) )
	ROM_LOAD( "di-00-a.2a",   0x18000, 0x8000, CRC(f39e2cf4) SHA1(ce77a65089937906d8a41076a3f29ad6f45fda9d) ) // this ROM has data at 3FD7 where it seems to be blanked out in firetrapa, as well as other changes

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "di-17.10j",    0x00000, 0x8000, CRC(8605f6b9) SHA1(4fba88f34afd91d2cbc578b3b70f5399b8844390) )
	ROM_LOAD( "di-18.12j",    0x08000, 0x8000, CRC(49508c93) SHA1(3812b0b1a33a1506d2896d2b676ed6aabb29dac0) )

	ROM_REGION( 0x1000, "mcu", 0 )  // 8751 protection MCU
	ROM_LOAD( "di-12.16h",    0x00000, 0x1000, CRC(6340a4d7) SHA1(3c896015a2416e3d664fedd07e42bdd40078c700) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "di-03.17c",    0x00000, 0x2000, CRC(46721930) SHA1(a605fe993166e95c1602a35b548649ceae77bff2) )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD( "di-06.3e",     0x00000, 0x2000, CRC(441d9154) SHA1(340804e82d4aba8e9fcdd08cce0cfecefd2f77a9) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "di-04.2e",     0x04000, 0x2000, CRC(8e6e7eec) SHA1(9cff147702620987346449e2f83ef9b2efef7798) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "di-07.6e",     0x10000, 0x2000, CRC(ef0a7e23) SHA1(7c67ac27e6bde0f4943e8bed9898e730ae7ddd75) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "di-05.4e",     0x14000, 0x2000, CRC(ec080082) SHA1(3b034496bfa2aba9ed58ceba670d0364a9c2211d) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "di-09.3j",     0x00000, 0x2000, CRC(d11e28e8) SHA1(3e91764f74d551e0984bac92daeab4e094e8dc13) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "di-08.2j",     0x04000, 0x2000, CRC(c32a21d8) SHA1(01898abf24aa40b13939afed96c990f430eb3bf1) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "di-11.6j",     0x10000, 0x2000, CRC(6424d5c3) SHA1(9ad6cfe6effca795709f90839a338f2a9148128f) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "di-10.4j",     0x14000, 0x2000, CRC(9b89300a) SHA1(5575daa226188cb1ea7d7a23f4966252bfb748e0) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "di-16.17h",    0x00000, 0x8000, CRC(0de055d7) SHA1(ef763237c317545520c659f438b572b11c342d5a) )
	ROM_LOAD( "di-13.13h",    0x08000, 0x8000, CRC(869219da) SHA1(9ab2439d6d1c62fce24c4f78ac7887f34c86cd75) )
	ROM_LOAD( "di-14.14h",    0x10000, 0x8000, CRC(6b65812e) SHA1(209e07b2fced6b033c6d5398a998374588a35f46) )
	ROM_LOAD( "di-15.15h",    0x18000, 0x8000, CRC(3e27f77d) SHA1(9ceccb1f56a8d0e05f6dea45d102690a1370624e) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "firetrap.3b",  0x0000,  0x0100, CRC(8bb45337) SHA1(deaf6ea53eb3955230db1fdcb870079758a0c996) ) // palette red and green component
	ROM_LOAD( "firetrap.4b",  0x0100,  0x0100, CRC(d5abfc64) SHA1(6c808c1d6087804214dc29d35280f42382c40b18) ) // palette blue component
	ROM_LOAD( "firetrap.1a",  0x0200,  0x0100, CRC(d67f3514) SHA1(afadda6111fea663fe1373a81e184e51afc601df) ) // ?
ROM_END

ROM_START( firetrapa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "di-02.4a",     0x00000, 0x8000, CRC(3d1e4bf7) SHA1(ee903b469619f49edb1727fb545c9a6085f50746) )
	ROM_LOAD( "di-01.3a",     0x10000, 0x8000, CRC(9bbae38b) SHA1(dc1d3ed5da71bfb104fd54fc70c56833f31d281f) )
	ROM_LOAD( "di-00.2a",     0x18000, 0x8000, CRC(d0dad7de) SHA1(8783ebf6ddfef32f6036913d403f76c1545b813d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "di-17.10j",    0x00000, 0x8000, CRC(8605f6b9) SHA1(4fba88f34afd91d2cbc578b3b70f5399b8844390) )
	ROM_LOAD( "di-18.12j",    0x08000, 0x8000, CRC(49508c93) SHA1(3812b0b1a33a1506d2896d2b676ed6aabb29dac0) )

	ROM_REGION( 0x1000, "mcu", 0 )  // 8751 protection MCU
	ROM_LOAD( "di-12.16h",    0x00000, 0x1000, CRC(6340a4d7) SHA1(3c896015a2416e3d664fedd07e42bdd40078c700) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "di-03.17c",    0x00000, 0x2000, CRC(46721930) SHA1(a605fe993166e95c1602a35b548649ceae77bff2) )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD( "di-06.3e",     0x00000, 0x2000, CRC(441d9154) SHA1(340804e82d4aba8e9fcdd08cce0cfecefd2f77a9) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "di-04.2e",     0x04000, 0x2000, CRC(8e6e7eec) SHA1(9cff147702620987346449e2f83ef9b2efef7798) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "di-07.6e",     0x10000, 0x2000, CRC(ef0a7e23) SHA1(7c67ac27e6bde0f4943e8bed9898e730ae7ddd75) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "di-05.4e",     0x14000, 0x2000, CRC(ec080082) SHA1(3b034496bfa2aba9ed58ceba670d0364a9c2211d) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "di-09.3j",     0x00000, 0x2000, CRC(d11e28e8) SHA1(3e91764f74d551e0984bac92daeab4e094e8dc13) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "di-08.2j",     0x04000, 0x2000, CRC(c32a21d8) SHA1(01898abf24aa40b13939afed96c990f430eb3bf1) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "di-11.6j",     0x10000, 0x2000, CRC(6424d5c3) SHA1(9ad6cfe6effca795709f90839a338f2a9148128f) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "di-10.4j",     0x14000, 0x2000, CRC(9b89300a) SHA1(5575daa226188cb1ea7d7a23f4966252bfb748e0) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "di-16.17h",    0x00000, 0x8000, CRC(0de055d7) SHA1(ef763237c317545520c659f438b572b11c342d5a) )
	ROM_LOAD( "di-13.13h",    0x08000, 0x8000, CRC(869219da) SHA1(9ab2439d6d1c62fce24c4f78ac7887f34c86cd75) )
	ROM_LOAD( "di-14.14h",    0x10000, 0x8000, CRC(6b65812e) SHA1(209e07b2fced6b033c6d5398a998374588a35f46) )
	ROM_LOAD( "di-15.15h",    0x18000, 0x8000, CRC(3e27f77d) SHA1(9ceccb1f56a8d0e05f6dea45d102690a1370624e) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "firetrap.3b",  0x0000,  0x0100, CRC(8bb45337) SHA1(deaf6ea53eb3955230db1fdcb870079758a0c996) ) // palette red and green component
	ROM_LOAD( "firetrap.4b",  0x0100,  0x0100, CRC(d5abfc64) SHA1(6c808c1d6087804214dc29d35280f42382c40b18) ) // palette blue component
	ROM_LOAD( "firetrap.1a",  0x0200,  0x0100, CRC(d67f3514) SHA1(afadda6111fea663fe1373a81e184e51afc601df) ) // ?
ROM_END

ROM_START( firetrapj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fi-03.4a",     0x00000, 0x8000, CRC(20b2a4ff) SHA1(7c5de0789c5d3459063eb791f62d41c05ab474cc) )
	ROM_LOAD( "fi-02.3a",     0x10000, 0x8000, CRC(5c8a0562) SHA1(856766851faa4353445d944b7705e348fd1379e4) )
	ROM_LOAD( "fi-01.2a",     0x18000, 0x8000, CRC(f2412fe8) SHA1(28a9143e36c31fe34f40888dc848aed3d572d801) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fi-18.10j",    0x00000, 0x8000, CRC(8605f6b9) SHA1(4fba88f34afd91d2cbc578b3b70f5399b8844390) )
	ROM_LOAD( "fi-19.12j",    0x08000, 0x8000, CRC(49508c93) SHA1(3812b0b1a33a1506d2896d2b676ed6aabb29dac0) )

	ROM_REGION( 0x1000, "mcu", 0 )  // 8751 protection MCU
	ROM_LOAD( "fi-13.16h",    0x00000, 0x1000, CRC(e531a633) SHA1(f21349f4e1147643204ae9735c304129f49911e7) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "fi-04.17c",    0x00000, 0x2000, CRC(a584fc16) SHA1(6ac3692a14cb7c70799c23f8f6726fa5be1ac0d8) )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD( "fi-06.3e",     0x00000, 0x2000, CRC(441d9154) SHA1(340804e82d4aba8e9fcdd08cce0cfecefd2f77a9) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "fi-05.2e",     0x04000, 0x2000, CRC(8e6e7eec) SHA1(9cff147702620987346449e2f83ef9b2efef7798) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "fi-08.6e",     0x10000, 0x2000, CRC(ef0a7e23) SHA1(7c67ac27e6bde0f4943e8bed9898e730ae7ddd75) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "fi-07.4e",     0x14000, 0x2000, CRC(ec080082) SHA1(3b034496bfa2aba9ed58ceba670d0364a9c2211d) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "fi-10.3j",     0x00000, 0x2000, CRC(d11e28e8) SHA1(3e91764f74d551e0984bac92daeab4e094e8dc13) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "fi-09.2j",     0x04000, 0x2000, CRC(c32a21d8) SHA1(01898abf24aa40b13939afed96c990f430eb3bf1) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "fi-12.6j",     0x10000, 0x2000, CRC(6424d5c3) SHA1(9ad6cfe6effca795709f90839a338f2a9148128f) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "fi-11.4j",     0x14000, 0x2000, CRC(9b89300a) SHA1(5575daa226188cb1ea7d7a23f4966252bfb748e0) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "fi-17.17h",    0x00000, 0x8000, CRC(0de055d7) SHA1(ef763237c317545520c659f438b572b11c342d5a) )
	ROM_LOAD( "fi-14.13h",    0x08000, 0x8000, CRC(dbcdd3df) SHA1(99a7722e818a0f12ece1abd038531cdcb60f1181) )
	ROM_LOAD( "fi-15.14h",    0x10000, 0x8000, CRC(6b65812e) SHA1(209e07b2fced6b033c6d5398a998374588a35f46) )
	ROM_LOAD( "fi-16.15h",    0x18000, 0x8000, CRC(3e27f77d) SHA1(9ceccb1f56a8d0e05f6dea45d102690a1370624e) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "fi-2.3b",      0x0000,  0x0100, CRC(8bb45337) SHA1(deaf6ea53eb3955230db1fdcb870079758a0c996) ) // palette red and green component
	ROM_LOAD( "fi-3.4b",      0x0100,  0x0100, CRC(d5abfc64) SHA1(6c808c1d6087804214dc29d35280f42382c40b18) ) // palette blue component
	ROM_LOAD( "fi-1.1a",      0x0200,  0x0100, CRC(d67f3514) SHA1(afadda6111fea663fe1373a81e184e51afc601df) ) // ?
ROM_END

ROM_START( firetrapbl )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "ft0d.bin",     0x00000, 0x8000, CRC(793ef849) SHA1(5a2c587370733d43484ba0a38a357260cdde8357) )
	ROM_LOAD( "ft0a.bin",     0x08000, 0x8000, CRC(613313ee) SHA1(54e386b2b1faada3441e3e0bb7822a63eab36930) )   // unprotection code
	ROM_LOAD( "fi-02.3a",     0x10000, 0x8000, CRC(5c8a0562) SHA1(856766851faa4353445d944b7705e348fd1379e4) )
	ROM_LOAD( "fi-01.2a",     0x18000, 0x8000, CRC(f2412fe8) SHA1(28a9143e36c31fe34f40888dc848aed3d572d801) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fi-18.10j",    0x00000, 0x8000, CRC(8605f6b9) SHA1(4fba88f34afd91d2cbc578b3b70f5399b8844390) )
	ROM_LOAD( "fi-19.12j",    0x08000, 0x8000, CRC(49508c93) SHA1(3812b0b1a33a1506d2896d2b676ed6aabb29dac0) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "fi-04.17c",    0x00000, 0x2000, CRC(a584fc16) SHA1(6ac3692a14cb7c70799c23f8f6726fa5be1ac0d8) )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD( "fi-06.3e",     0x00000, 0x2000, CRC(441d9154) SHA1(340804e82d4aba8e9fcdd08cce0cfecefd2f77a9) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "fi-05.2e",     0x04000, 0x2000, CRC(8e6e7eec) SHA1(9cff147702620987346449e2f83ef9b2efef7798) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "fi-08.6e",     0x10000, 0x2000, CRC(ef0a7e23) SHA1(7c67ac27e6bde0f4943e8bed9898e730ae7ddd75) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "fi-07.4e",     0x14000, 0x2000, CRC(ec080082) SHA1(3b034496bfa2aba9ed58ceba670d0364a9c2211d) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "fi-10.3j",     0x00000, 0x2000, CRC(d11e28e8) SHA1(3e91764f74d551e0984bac92daeab4e094e8dc13) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "fi-09.2j",     0x04000, 0x2000, CRC(c32a21d8) SHA1(01898abf24aa40b13939afed96c990f430eb3bf1) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "fi-12.6j",     0x10000, 0x2000, CRC(6424d5c3) SHA1(9ad6cfe6effca795709f90839a338f2a9148128f) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "fi-11.4j",     0x14000, 0x2000, CRC(9b89300a) SHA1(5575daa226188cb1ea7d7a23f4966252bfb748e0) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "fi-17.17h",    0x00000, 0x8000, CRC(0de055d7) SHA1(ef763237c317545520c659f438b572b11c342d5a) )
	ROM_LOAD( "fi-14.13h",    0x08000, 0x8000, CRC(869219da) SHA1(9ab2439d6d1c62fce24c4f78ac7887f34c86cd75) ) // sldh
	ROM_LOAD( "fi-15.14h",    0x10000, 0x8000, CRC(6b65812e) SHA1(209e07b2fced6b033c6d5398a998374588a35f46) )
	ROM_LOAD( "fi-16.15h",    0x18000, 0x8000, CRC(3e27f77d) SHA1(9ceccb1f56a8d0e05f6dea45d102690a1370624e) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "fi-2.3b",      0x0000,  0x0100, CRC(8bb45337) SHA1(deaf6ea53eb3955230db1fdcb870079758a0c996) ) // palette red and green component
	ROM_LOAD( "fi-3.4b",      0x0100,  0x0100, CRC(d5abfc64) SHA1(6c808c1d6087804214dc29d35280f42382c40b18) ) // palette blue component
	ROM_LOAD( "fi-1.1a",      0x0200,  0x0100, CRC(d67f3514) SHA1(afadda6111fea663fe1373a81e184e51afc601df) ) // ?
ROM_END

} // anonymous namespace


GAME( 1986, firetrap,   0,        firetrap,   firetrap,   original_state, empty_init, ROT90, "Woodplace Inc. (Data East USA license)", "Fire Trap (US, rev A)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, firetrapa,  firetrap, firetrap,   firetrap,   original_state, empty_init, ROT90, "Woodplace Inc. (Data East USA license)", "Fire Trap (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, firetrapj,  firetrap, firetrap,   firetrapj,  original_state, empty_init, ROT90, "Woodplace Inc.",                         "Fire Trap (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, firetrapbl, firetrap, firetrapbl, firetrapbl, bootleg_state,  empty_init, ROT90, "bootleg",                                "Fire Trap (Japan bootleg)", MACHINE_SUPPORTS_SAVE )
