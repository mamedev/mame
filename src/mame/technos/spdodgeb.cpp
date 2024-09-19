// license:BSD-3-Clause
// copyright-holders:Paul Hampson, Nicola Salmoria
/***************************************************************************

Super Dodge Ball / Nekketsu Koukou Dodgeball Bu

driver by Paul Hampson and Nicola Salmoria

TODO:
- sprite lag (the real game has quite a bit of lag too)

Notes:
- there's a 63701 on the board, used for protection. It is checked
  on startup and then just used to read the input ports. It doesn't return
  the ports verbatim, it adds further processing, setting flags when the
  player double-taps in one direction to run.(updated to edge-triggering)

- video timing is probably similar to Double Dragon and other Technos games
  of that era.  The rowscroll of the title bar is done with raster IRQs, I've
  removed the 'scroll value buffer' hack that the driver was using before and
  used partial updates instead. (DH, 29 Sept 07)

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6800/m6801.h"
#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class spdodgeb_state : public driver_device
{
public:
	spdodgeb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm%u", 1U),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_adpcm_rom(*this, "adpcm")
	{ }

	void spdodgeb(machine_config &config);

	int mcu_busy_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device_array<msm5205_device, 2> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_mainbank;
	required_region_ptr<uint8_t> m_adpcm_rom;

	uint32_t m_adpcm_pos[2]{};
	uint16_t m_adpcm_end[2]{};
	uint8_t m_adpcm_idle[2]{};
	int16_t m_adpcm_data[2]{};
	uint8_t m_mcu_status = 0;
	uint8_t m_inputs[5]{};

	uint8_t m_tile_palbank = 0;
	uint8_t m_sprite_palbank = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint16_t m_lastscroll = 0;

	template <uint8_t Which> void adpcm_int(int state);
	void adpcm_w(offs_t offset, uint8_t data);
	uint8_t mcu63701_r(offs_t offset);
	void mcu_data_w(offs_t offset, uint8_t data);
	void mcu_status_w(uint8_t data);
	void mcu_nmi_w(uint8_t data);

	void scrollx_lo_w(uint8_t data);
	void ctrl_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);

	TILEMAP_MAPPER_MEMBER(background_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;
};



void spdodgeb_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0;i < palette.entries();i++)
	{
		// red component
		int bit0 = BIT(color_prom[0], 0);
		int bit1 = BIT(color_prom[0], 1);
		int bit2 = BIT(color_prom[0], 2);
		int bit3 = BIT(color_prom[0], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = BIT(color_prom[0], 4);
		bit1 = BIT(color_prom[0], 5);
		bit2 = BIT(color_prom[0], 6);
		bit3 = BIT(color_prom[0], 7);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = BIT(color_prom[palette.entries()], 0);
		bit1 = BIT(color_prom[palette.entries()], 1);
		bit2 = BIT(color_prom[palette.entries()], 2);
		bit3 = BIT(color_prom[palette.entries()], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(spdodgeb_state::background_scan)
{
	// logical (col,row) -> memory offset
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

TILE_GET_INFO_MEMBER(spdodgeb_state::get_bg_tile_info)
{
	uint8_t code = m_videoram[tile_index];
	uint8_t attr = m_videoram[tile_index + 0x800];
	tileinfo.set(0,
			code + ((attr & 0x1f) << 8),
			((attr & 0xe0) >> 5) + 8 * m_tile_palbank,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void spdodgeb_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spdodgeb_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(spdodgeb_state::background_scan)), 8,8, 64,32);

	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base(), 0x4000);

	save_item(NAME(m_tile_palbank));
	save_item(NAME(m_sprite_palbank));
	save_item(NAME(m_lastscroll));
}


/***************************************************************************

  Memory handlers

***************************************************************************/


TIMER_DEVICE_CALLBACK_MEMBER(spdodgeb_state::interrupt)
{
	int scanline = param;

	if (scanline == 256)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		m_screen->update_partial(256);
	}
	else if ((scanline % 8) == 0)
	{
		m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		m_screen->update_partial(scanline + 16); // TODO: pretty off ...
	}
}

void spdodgeb_state::scrollx_lo_w(uint8_t data)
{
	m_lastscroll = (m_lastscroll & 0x100) | data;
}

void spdodgeb_state::ctrl_w(uint8_t data)
{
	// bit 0 = flip screen
	flip_screen_set(data & 0x01);

	// bit 1 = ROM bank switch
	m_mainbank->set_entry((~data & 0x02) >> 1);

	// bit 2 = scroll high bit
	m_lastscroll = (m_lastscroll & 0x0ff) | ((data & 0x04) << 6);

	// bit 3 = to mcu??

	// bits 4-7 = palette bank select
	if (m_tile_palbank != ((data & 0x30) >> 4))
	{
		m_tile_palbank = ((data & 0x30) >> 4);
		m_bg_tilemap->mark_all_dirty();
	}
	m_sprite_palbank = (data & 0xc0) >> 6;
}

void spdodgeb_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

#define DRAW_SPRITE( order, sx, sy ) gfx->transpen(bitmap,\
					cliprect, \
					(which + order), color + 8 * m_sprite_palbank, flipx, flipy, sx, sy, 0);

void spdodgeb_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

/*  240-SY   Z|F|CLR|WCH WHICH    SX
    xxxxxxxx x|x|xxx|xxx xxxxxxxx xxxxxxxx
*/
	for (int i = 0; i < m_spriteram.bytes(); i += 4)
	{
		int attr = m_spriteram[i + 1];
		int which = m_spriteram[i + 2] + ((attr & 0x07) << 8);
		int sx = m_spriteram[i + 3];
		int sy = 240 - m_spriteram[i];
		int size = (attr & 0x80) >> 7;
		int color = (attr & 0x38) >> 3;
		int flipx = ~attr & 0x40;
		int flipy = 0;
		int dy = -16;
		int cy;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			dy = -dy;
		}

		if (sx < -8) sx += 256; else if (sx > 248) sx -= 256;

		switch (size)
		{
			case 0: // normal
			if (sy < -8) sy += 256; else if (sy > 248) sy -= 256;
			DRAW_SPRITE(0, sx, sy);
			break;

			case 1: // double y
			if (flip_screen()) { if (sy > 240) sy -= 256; } else { if (sy < 0) sy += 256; }
			cy = sy + dy;
			which &= ~1;
			DRAW_SPRITE(0, sx, cy);
			DRAW_SPRITE(1, sx, sy);
			break;
		}
	}
}

#undef DRAW_SPRITE


uint32_t spdodgeb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_lastscroll + 5);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

void spdodgeb_state::adpcm_w(offs_t offset, uint8_t data)
{
	int chip = offset & 1;

	switch (offset/2)
	{
		case 3:
			m_adpcm_idle[chip] = 1;
			m_msm[chip]->reset_w(1);
			break;

		case 2:
			m_adpcm_pos[chip] = (data & 0x7f) * 0x200;
			break;

		case 1:
			m_adpcm_end[chip] = (data & 0x7f) * 0x200;
			break;

		case 0:
			m_adpcm_idle[chip] = 0;
			m_msm[chip]->reset_w(0);
			break;
	}
}

template <uint8_t Which>
void spdodgeb_state::adpcm_int(int state)
{
	if (m_adpcm_pos[Which] >= m_adpcm_end[Which] || m_adpcm_pos[Which] >= 0x10000)
	{
		m_adpcm_idle[Which] = 1;
		m_msm[Which]->reset_w(1);
	}
	else if (m_adpcm_data[Which] != -1)
	{
		m_msm[Which]->data_w(m_adpcm_data[Which] & 0x0f);
		m_adpcm_data[Which] = -1;
	}
	else
	{
		uint8_t *rom = &m_adpcm_rom[0x10000 * Which];

		m_adpcm_data[Which] = rom[m_adpcm_pos[Which]++];
		m_msm[Which]->data_w(m_adpcm_data[Which] >> 4);
	}
}

uint8_t spdodgeb_state::mcu63701_r(offs_t offset)
{
//  logerror("CPU #0 PC %04x: read from port %02x of 63701 data address 3801\n",m_maincpu->pc(),offset);

	return m_inputs[offset];
}

void spdodgeb_state::mcu_data_w(offs_t offset, uint8_t data)
{
	m_inputs[offset] = data;
}

void spdodgeb_state::mcu_status_w(uint8_t data)
{
	m_mcu_status = data & 0xc0;
}

void spdodgeb_state::mcu_nmi_w(uint8_t data)
{
	m_mcu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}



void spdodgeb_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x10ff).writeonly().share(m_spriteram);
	map(0x2000, 0x2fff).ram().w(FUNC(spdodgeb_state::videoram_w)).share(m_videoram);
	map(0x3000, 0x3000).portr("IN0"); //.nopw();
	map(0x3001, 0x3001).portr("DSW"); //.nopw();
	map(0x3002, 0x3002).w(m_soundlatch, FUNC(generic_latch_8_device::write));
//  map(0x3003, 0x3003).nopw();
	map(0x3004, 0x3004).w(FUNC(spdodgeb_state::scrollx_lo_w));
	map(0x3005, 0x3005).w(FUNC(spdodgeb_state::mcu_nmi_w));
	map(0x3006, 0x3006).w(FUNC(spdodgeb_state::ctrl_w));  // scroll hi, flip screen, bank switch, palette select
	map(0x3800, 0x3800).w("mculatch", FUNC(generic_latch_8_device::write));
	map(0x3801, 0x3805).r(FUNC(spdodgeb_state::mcu63701_r));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void spdodgeb_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x2800, 0x2801).w("ymsnd", FUNC(ym3812_device::write));
	map(0x3800, 0x3807).w(FUNC(spdodgeb_state::adpcm_w));
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}

void spdodgeb_state::mcu_map(address_map &map)
{
	map(0x8080, 0x8080).r("mculatch", FUNC(generic_latch_8_device::read));
	map(0x8081, 0x8085).w(FUNC(spdodgeb_state::mcu_data_w));
}


int spdodgeb_state::mcu_busy_r()
{
	return BIT(m_mcu_status, 7);
}

static INPUT_PORTS_START( spdodgeb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(spdodgeb_state, mcu_busy_r) // mcu63701_busy flag
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ))
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ))
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ))

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 64+1, 64+0, 128+1, 128+0, 192+1, 192+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0,4 },
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
			32*8+3, 32*8+2, 32*8+1, 32*8+0, 48*8+3, 48*8+2, 48*8+1, 48*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	64*8
};

static GFXDECODE_START( gfx_spdodgeb )
	GFXDECODE_ENTRY( "text",    0, charlayout,   0x000, 32 )   // colors 0x000-0x1ff
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0x200, 32 )   // colors 0x200-0x3ff
GFXDECODE_END


void spdodgeb_state::machine_start()
{
	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_end));
	save_item(NAME(m_adpcm_idle));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_mcu_status));
	save_item(NAME(m_inputs));
}

void spdodgeb_state::machine_reset()
{
	m_adpcm_pos[0] = m_adpcm_pos[1] = 0;
	m_adpcm_end[0] = m_adpcm_end[1] = 0;
	m_adpcm_idle[0] = m_adpcm_data[1] = 0;
	m_adpcm_data[0] = m_adpcm_data[1] = -1;
	m_mcu_status = 0;
	memset(m_inputs, 0, sizeof(m_inputs));
}

void spdodgeb_state::spdodgeb(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, XTAL(12'000'000) / 6);   // 2MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &spdodgeb_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(spdodgeb_state::interrupt), "screen", 0, 1); // 1 IRQ every 8 visible scanlines, plus NMI for vblank

	MC6809(config, m_audiocpu, XTAL(12'000'000) / 2); // HD68A09P (1.5MHz internally)
	m_audiocpu->set_addrmap(AS_PROGRAM, &spdodgeb_state::sound_map);

	hd63701y0_cpu_device &mcu(HD63701Y0(config, m_mcu, 4'000'000)); // unknown clock
	mcu.set_addrmap(AS_PROGRAM, &spdodgeb_state::mcu_map);
	mcu.in_p2_cb().set_ioport("P1");
	mcu.in_p6_cb().set_ioport("P2");
	mcu.in_p5_cb().set_ioport("IN1");
	mcu.out_p5_cb().set(FUNC(spdodgeb_state::mcu_status_w));

	GENERIC_LATCH_8(config, "mculatch");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(12'000'000) / 2, 384, 0, 256, 272, 0, 240);
	m_screen->set_screen_update(FUNC(spdodgeb_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_spdodgeb);
	PALETTE(config, m_palette, FUNC(spdodgeb_state::palette), 1024);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, M6809_IRQ_LINE);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(12'000'000) / 4));
	ymsnd.irq_handler().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
	ymsnd.add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	ymsnd.add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	MSM5205(config, m_msm[0], 384000);
	m_msm[0]->vck_legacy_callback().set(FUNC(spdodgeb_state::adpcm_int<0>));  // interrupt function
	m_msm[0]->set_prescaler_selector(msm5205_device::S48_4B);  // 8kHz?
	m_msm[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	m_msm[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.50);

	MSM5205(config, m_msm[1], 384000);
	m_msm[1]->vck_legacy_callback().set(FUNC(spdodgeb_state::adpcm_int<1>));  // interrupt function
	m_msm[1]->set_prescaler_selector(msm5205_device::S48_4B);  // 8kHz?
	m_msm[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	m_msm[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}



ROM_START( spdodgeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "22a-04.139",   0x00000, 0x10000, CRC(66071fda) SHA1(4a239295900e6234a2a693321ca821671747a58e) )  // First 0x8000: Two banks, second 0x8000: Static

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "22j5-0.33",    0x00000, 0x08000, CRC(c31e264e) SHA1(0828a2094122e3934b784ec9ad7c2b89d91a83bb) )

	ROM_REGION( 0x04000, "mcu", 0 ) // I/O
	ROM_LOAD( "22ja-0.162",   0x00000, 0x04000, CRC(7162a97b) SHA1(d6d4ee025e73a340428345f08711cd32f9169a8c) )

	ROM_REGION( 0x40000, "text", 0 )
	ROM_LOAD( "22a-4.121",    0x00000, 0x20000, CRC(acc26051) SHA1(445224238cce420990894824d95447e3f63a9ef0) )
	ROM_LOAD( "22a-3.107",    0x20000, 0x20000, CRC(10bb800d) SHA1(265a3d67669034d17713b505ef55cd1c90f8d205) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "22a-1.2",      0x00000, 0x20000, CRC(3bd1c3ec) SHA1(40f61552ea6f7a81915fe3e13f75dc1dc69da33e) )
	ROM_LOAD( "22a-2.35",     0x20000, 0x20000, CRC(409e1be1) SHA1(35a77fc8fe6fc212734e2f452dbde9b8cf696f61) )

	ROM_REGION( 0x20000, "adpcm", 0 )
	ROM_LOAD( "22j6-0.83",    0x00000, 0x10000, CRC(744a26e3) SHA1(519f22f1e5cc417cb8f9ced97e959d23c711283b) )
	ROM_LOAD( "22j7-0.82",    0x10000, 0x10000, CRC(2fa1de21) SHA1(e8c7af6057b64ecadd3473b82abd8e9f873082fd) )

	ROM_REGION( 0x0800, "proms", 0 )    // colors
	ROM_LOAD( "mb7132e.158",  0x0000, 0x0400, CRC(7e623722) SHA1(e1fe60533237bd0aba5c8de9775df620ed5227c0) )
	ROM_LOAD( "mb7122e.159",  0x0400, 0x0400, CRC(69706e8d) SHA1(778ee88ff566aa38c80e0e61bb3fe8458f0e9450) )
ROM_END

/*

Nekketsu Koukou Dodgeball Bu
(c)1987 Technos Japan

TA-0022-P1-04
M6100293A (PCB manufactured by Taito)

CPU: 6502 (Labeled TJC-706002)
Sound: 68A09, YM3812, M5205x2
OSC: 12.000MHz

ROMs:
22J4-0.139 - Main program
22J5-0.33  - Sound program
22JA-0.162 - HD63701Y0P

TJ22J4-0.121 - Text
TJ22J3-0.107 /

TJ22J1-0.2  - Graphics
TJ22J2-0.35 /

22J6-0.83 - ADPCM Samples
22J7-0.82 /

22J8-0.158 (7132)
22J9-0.159 (7122)

*/

ROM_START( nkdodge )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "22j4-0.139",   0x00000, 0x10000, CRC(aa674fd8) SHA1(4e8d3e07b54d23b221cb39cf10389bc7a56c4021) )  // First 0x8000: Two banks, second 0x8000: Static

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "22j5-0.33",    0x00000, 0x08000, CRC(c31e264e) SHA1(0828a2094122e3934b784ec9ad7c2b89d91a83bb) )

	ROM_REGION( 0x04000, "mcu", 0 ) // I/O
	ROM_LOAD( "22ja-0.162",   0x00000, 0x04000, CRC(7162a97b) SHA1(d6d4ee025e73a340428345f08711cd32f9169a8c) )

	ROM_REGION( 0x40000, "text", 0 )
	ROM_LOAD( "tj22j4-0.121",    0x00000, 0x20000, CRC(d2922b3f) SHA1(30ad37f8355c732b545017c2fc56879256b650be) )
	ROM_LOAD( "tj22j3-0.107",    0x20000, 0x20000, CRC(79cd1315) SHA1(2d7a877e59f704b10b5f609e60fa565c68f5fdb0) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "tj22j1-0.2",      0x00000, 0x20000, CRC(9ed27a8d) SHA1(d80d275bbe91f3e1bd0495a2d7a3be0280a7cda1) )
	ROM_LOAD( "tj22j2-0.35",     0x20000, 0x20000, CRC(768934f9) SHA1(922f3154dcfb29c2e5c1bebc53247136160f1229) )

	ROM_REGION( 0x20000, "adpcm", 0 )
	ROM_LOAD( "22j6-0.83",    0x00000, 0x10000, CRC(744a26e3) SHA1(519f22f1e5cc417cb8f9ced97e959d23c711283b) )
	ROM_LOAD( "22j7-0.82",    0x10000, 0x10000, CRC(2fa1de21) SHA1(e8c7af6057b64ecadd3473b82abd8e9f873082fd) )

	ROM_REGION( 0x0800, "proms", 0 )    // colors
	ROM_LOAD( "22j8-0.158",  0x0000, 0x0400, CRC(c368440f) SHA1(39762d102a42211f24db16bc721b01230df1c4d6) )
	ROM_LOAD( "22j9-0.159",  0x0400, 0x0400, CRC(6059f401) SHA1(280b1bda3a55f2d8c2fd4552c4dcec7100f0170f) )
ROM_END

// the bootleg just seems to have the gfx ROMs in a different format, program is identical

ROM_START( nkdodgeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "12.bin",       0x00000, 0x10000, CRC(aa674fd8) SHA1(4e8d3e07b54d23b221cb39cf10389bc7a56c4021) )  // First 0x8000: Two banks, second 0x8000: Static

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "22j5-0.33",    0x00000, 0x08000, CRC(c31e264e) SHA1(0828a2094122e3934b784ec9ad7c2b89d91a83bb) )

	ROM_REGION( 0x04000, "mcu", 0 ) // I/O
	ROM_LOAD( "hd63701y0p.n12", 0x0000, 0x4000, CRC(7162a97b) SHA1(d6d4ee025e73a340428345f08711cd32f9169a8c) BAD_DUMP ) // missing from dump, but probably also identical

	ROM_REGION( 0x40000, "text", 0 )
	ROM_LOAD( "10.bin",       0x00000, 0x10000, CRC(442326fd) SHA1(e0e9e1dfdca3edd6e2522f55c191b40b81b8eaff) )
	ROM_LOAD( "11.bin",       0x10000, 0x10000, CRC(2140b070) SHA1(7a9d89eb6130b1dd21236fefaeb09a29c7f0d208) )
	ROM_LOAD( "9.bin",        0x20000, 0x10000, CRC(18660ac1) SHA1(be6a47eea9649d7b9ff8b30a4de643522c9869e6) )
	ROM_LOAD( "8.bin",        0x30000, 0x10000, CRC(5caae3c9) SHA1(f81a1c4ce2117d41e81542d417ff3573ea0f5313) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "2.bin",        0x00000, 0x10000, CRC(1271583e) SHA1(98a597f2be1abdac6c4de811cfa8a53549bc6904) )
	ROM_LOAD( "1.bin",        0x10000, 0x10000, CRC(5ae6cccf) SHA1(6bd385d6559b54c681d05eed2e91bfc2aa3e6844) )
	ROM_LOAD( "4.bin",        0x20000, 0x10000, CRC(f5022822) SHA1(fa67b1f70da80365f14776b712df6f656e603fb0) )
	ROM_LOAD( "3.bin",        0x30000, 0x10000, CRC(05a71179) SHA1(7e5ed81b37ac458d7a40e89f83f1efb742e797a8) )

	ROM_REGION( 0x20000, "adpcm", 0 )
	ROM_LOAD( "22j6-0.83",    0x00000, 0x10000, CRC(744a26e3) SHA1(519f22f1e5cc417cb8f9ced97e959d23c711283b) )
	ROM_LOAD( "22j7-0.82",    0x10000, 0x10000, CRC(2fa1de21) SHA1(e8c7af6057b64ecadd3473b82abd8e9f873082fd) )

	ROM_REGION( 0x0800, "proms", 0 )    // colors
	ROM_LOAD( "27s191.bin",  0x0000, 0x0800, CRC(317e42ea) SHA1(59caacc02fb7fb11604bd177f790fd68830ca7c1) )
	ROM_LOAD( "82s137.bin",  0x0400, 0x0400, CRC(6059f401) SHA1(280b1bda3a55f2d8c2fd4552c4dcec7100f0170f) )
ROM_END

} // anonymous namespace


GAME( 1987, spdodgeb, 0,        spdodgeb, spdodgeb, spdodgeb_state, empty_init, ROT0, "Technos Japan", "Super Dodge Ball (US)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1987, nkdodge,  spdodgeb, spdodgeb, spdodgeb, spdodgeb_state, empty_init, ROT0, "Technos Japan", "Nekketsu Koukou Dodgeball Bu (Japan)",          MACHINE_SUPPORTS_SAVE )
GAME( 1987, nkdodgeb, spdodgeb, spdodgeb, spdodgeb, spdodgeb_state, empty_init, ROT0, "bootleg",       "Nekketsu Koukou Dodgeball Bu (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )
