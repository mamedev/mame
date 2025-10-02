// license:BSD-3-Clause
// copyright-holders: Mike Balfour, Zsolt Vasvari

/***************************************************************************


Birdie King / Birdie King II / Birdie King III Memory Map
---------------------------------------------------------

0000-7fff ROM
8000-83ff Scratch RAM
8400-8fff (Scratch RAM again, address lines AB10, AB11 ignored)
9000-97ff Playfield RAM
a000-bfff Unused?

DIP Locations verified for:
    - bking2

***************************************************************************/

#include "emu.h"

#include "taito68705.h"

#include "cpu/m6805/m6805.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class bking_state : public driver_device
{
public:
	bking_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundnmi(*this, "soundnmi"),
		m_playfield_ram(*this, "playfield_ram"),
		m_collision_detection_prom(*this, "collision_detection"),
		m_track{ { *this, "TRACK0_%c", 'X' }, { *this, "TRACK1_%c", 'X' } }
	{
	}

	void bking(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<input_merger_device> m_soundnmi;

	required_shared_ptr<uint8_t> m_playfield_ram;
	required_region_ptr<uint8_t> m_collision_detection_prom;

	required_ioport_array<2> m_track[2];

	// video-related
	bitmap_ind16 m_colmap_bg = 0;
	bitmap_ind16 m_colmap_ball = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_pc3259_output[4] {};
	uint8_t m_pc3259_mask = 0;
	uint8_t m_xld[3] {};
	uint8_t m_yld[3] {};
	uint8_t m_ball1_pic = 0;
	uint8_t m_ball2_pic = 0;
	uint8_t m_crow_pic = 0;
	uint8_t m_crow_flip = 0;
	uint8_t m_palette_bank = 0;
	uint8_t m_controller = 0;
	uint8_t m_hit = 0;

	uint8_t sndnmi_disable_r();
	void sndnmi_enable_w(uint8_t data);
	void soundlatch_w(uint8_t data);
	template <uint8_t Which> void xld_w(uint8_t data);
	template <uint8_t Which> void yld_w(uint8_t data);
	void cont1_w(uint8_t data);
	void cont2_w(uint8_t data);
	void cont3_w(uint8_t data);
	void msk_w(uint8_t data);
	void hitclr_w(uint8_t data);
	void playfield_w(offs_t offset, uint8_t data);
	template <uint8_t Which> uint8_t input_port_r();
	uint8_t pos_r(offs_t offset);
	void port_b_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void audio_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
};

class bking3_state : public bking_state
{
public:
	bking3_state(const machine_config &mconfig, device_type type, const char *tag) :
		bking_state(mconfig, type, tag),
		m_bmcu(*this, "bmcu"),
		m_extra_rom(*this, "extra")
	{
	}

	void bking3(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<taito68705_mcu_device> m_bmcu;

	required_region_ptr<uint8_t> m_extra_rom;

	// misc
	uint8_t m_addr_h = 0;
	uint8_t m_addr_l = 0;

	void addr_l_w(uint8_t data);
	void addr_h_w(uint8_t data);
	uint8_t extrarom_r();
	uint8_t ext_check_r();
	uint8_t mcu_status_r();
	void unk_w(uint8_t data);

	void io_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 390 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- BLUE
        -- 820 ohm resistor  -- GREEN
        -- 390 ohm resistor  -- GREEN
        -- 220 ohm resistor  -- GREEN
        -- 820 ohm resistor  -- RED
        -- 390 ohm resistor  -- RED
  bit 0 -- 220 ohm resistor  -- RED

***************************************************************************/

void bking_state::palette_init(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 220, 390, 820 };
	static constexpr int resistances_b [2] = { 220, 390 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	for (int i = 0; i < palette.entries(); i++)
	{
		uint16_t pen;
		int bit0, bit1, bit2;

		// color PROM A7-A8 is the palette select
		if (i < 0x20) // characters - image bits go to A0-A2 of the color PROM
			pen = (((i - 0x00) << 4) & 0x180) | ((i - 0x00) & 0x07);
		else if (i < 0x30) // crow - image bits go to A5-A6.
			pen = (((i - 0x20) << 5) & 0x180) | (((i - 0x20) & 0x03) << 5);
		else if (i < 0x38) // ball #1 - image bit goes to A3
			pen = (((i - 0x30) << 6) & 0x180) | (((i - 0x30) & 0x01) << 3);
		else // ball #2 - image bit goes to A4
			pen = (((i - 0x38) << 6) & 0x180) | (((i - 0x38) & 0x01) << 4);

		// red component
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = (color_prom[pen] >> 6) & 0x01;
		bit1 = (color_prom[pen] >> 7) & 0x01;
		int const b = combine_weights(gweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


template <uint8_t Which>
void bking_state::xld_w(uint8_t data)
{
	m_xld[Which] = -data;
}

template <uint8_t Which>
void bking_state::yld_w(uint8_t data)
{
	m_yld[Which] = -data;
}


void bking_state::cont1_w(uint8_t data)
{
	// D0 = COIN LOCK
	// D1 = BALL 5 (Controller selection)
	// D2 = VINV (flip screen)
	// D3 = Not Connected
	// D4-D7 = CROW0-CROW3 (selects crow picture)

	machine().bookkeeping().coin_lockout_global_w(BIT(~data, 0));

	flip_screen_set(BIT(data, 2));

	m_controller = BIT(data, 1);

	m_crow_pic = (data >> 4) & 0x0f;
}

void bking_state::cont2_w(uint8_t data)
{
	// D0-D2 = BALL10 - BALL12 (Selects player 1 ball picture)
	// D3-D5 = BALL20 - BALL22 (Selects player 2 ball picture)
	// D6 = HIT1
	// D7 = HIT2

	m_ball1_pic = (data >> 0) & 0x07;
	m_ball2_pic = (data >> 3) & 0x07;

	m_hit = data >> 6;
}

void bking_state::cont3_w(uint8_t data)
{
	// D0 = CROW INV (inverts Crow picture and coordinates)
	// D1-D2 = COLOR 0 - COLOR 1 (switches 4 color palettes, global across all graphics)
	// D3 = SOUND STOP

	m_crow_flip = BIT(~data, 0);

	if (m_palette_bank != ((data >> 1) & 0x03))
	{
		m_palette_bank = (data >> 1) & 0x03;
		m_bg_tilemap->mark_all_dirty();
	}

	machine().sound().system_mute(BIT(data, 3));
}


void bking_state::msk_w(uint8_t data)
{
	m_pc3259_mask++;
}


void bking_state::hitclr_w(uint8_t data)
{
	m_pc3259_mask = 0;

	m_pc3259_output[0] = 0;
	m_pc3259_output[1] = 0;
	m_pc3259_output[2] = 0;
	m_pc3259_output[3] = 0;
}


void bking_state::playfield_w(offs_t offset, uint8_t data)
{
	m_playfield_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


template <uint8_t Which> // 0 is X, 1 is Y
uint8_t bking_state::input_port_r()
{
	return m_track[m_controller][Which]->read();
}

uint8_t bking_state::pos_r(offs_t offset)
{
	return m_pc3259_output[offset / 8] << 4;
}


TILE_GET_INFO_MEMBER(bking_state::get_tile_info)
{
	uint8_t const code0 = m_playfield_ram[2 * tile_index + 0];
	uint8_t const code1 = m_playfield_ram[2 * tile_index + 1];

	int flags = 0;

	if (code1 & 4) flags |= TILE_FLIPX;
	if (code1 & 8) flags |= TILE_FLIPY;

	tileinfo.set(0, code0 + 256 * code1, m_palette_bank, flags);
}


void bking_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bking_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_screen->register_screen_bitmap(m_colmap_bg);
	m_screen->register_screen_bitmap(m_colmap_ball);
}


uint32_t bking_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw the balls
	m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
		m_ball1_pic,
		m_palette_bank,
		0, 0,
		m_xld[0], m_yld[0], 0);

	m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
		m_ball2_pic,
		m_palette_bank,
		0, 0,
		m_xld[1], m_yld[1], 0);

	// draw the crow
	m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
		m_crow_pic,
		m_palette_bank,
		m_crow_flip, m_crow_flip,
		m_crow_flip ? m_xld[2] - 16 : 256 - m_xld[2], m_crow_flip ? m_yld[2] - 16 : 256 - m_yld[2], 0);

	return 0;
}


void bking_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		const rectangle rect(0, 7, 0, 15);

		int xld = 0;
		int yld = 0;

		uint32_t latch = 0;

		if (m_pc3259_mask == 6) // player 1
		{
			xld = m_xld[0];
			yld = m_yld[0];

			m_gfxdecode->gfx(2)->opaque(m_colmap_ball,rect, m_ball1_pic, 0, 0, 0, 0, 0);

			latch = 0x0c00;
		}
		else if (m_pc3259_mask == 3) // player 2
		{
			xld = m_xld[1];
			yld = m_yld[1];

			m_gfxdecode->gfx(3)->opaque(m_colmap_ball,rect, m_ball2_pic, 0, 0, 0, 0, 0);

			latch = 0x0400;
		}
		else
			return;

		m_bg_tilemap->set_scrollx(0, flip_screen() ? -xld : xld);
		m_bg_tilemap->set_scrolly(0, flip_screen() ? -yld : yld);

		m_bg_tilemap->draw(*m_screen, m_colmap_bg, rect, 0, 0);

		m_bg_tilemap->set_scrollx(0, 0);
		m_bg_tilemap->set_scrolly(0, 0);

		// check for collision
		uint8_t const *const colmask = &m_collision_detection_prom[8 * m_hit];

		for (int y = rect.min_y; y <= rect.max_y; y++)
		{
			uint16_t const *const p0 = &m_colmap_bg.pix(y);
			uint16_t const *const p1 = &m_colmap_ball.pix(y);

			for (int x = rect.min_x; x <= rect.max_x; x++)
			{
				if (colmask[p0[x] & 7] && p1[x] & 1)
				{
					int const col = (xld + x) / 8 + 1;
					int const row = (yld + y) / 8 + 0;

					latch |= (flip_screen() ? 31 - col : col) << 0;
					latch |= (flip_screen() ? 31 - row : row) << 5;

					m_pc3259_output[0] = (latch >> 0x0) & 0xf;
					m_pc3259_output[1] = (latch >> 0x4) & 0xf;
					m_pc3259_output[2] = (latch >> 0x8) & 0xf;
					m_pc3259_output[3] = (latch >> 0xc) & 0xf;

					return;
				}
			}
		}
	}
}


uint8_t bking_state::sndnmi_disable_r()
{
	m_soundnmi->in_w<1>(0);
	return 0;
}

void bking_state::sndnmi_enable_w(uint8_t data)
{
	m_soundnmi->in_w<1>(1);
}

void bking_state::soundlatch_w(uint8_t data)
{
	m_soundlatch->write(bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7));
}

void bking3_state::addr_l_w(uint8_t data)
{
	m_addr_l = data;
}

void bking3_state::addr_h_w(uint8_t data)
{
	m_addr_h = data;
}

uint8_t bking3_state::extrarom_r()
{
	return m_extra_rom[m_addr_h * 256 + m_addr_l];
}

void bking3_state::unk_w(uint8_t data)
{
	// 0 = finished reading extra ROM
	// 1 = started reading extra ROM
}

uint8_t bking3_state::ext_check_r()
{
	return 0x31; //no "bad rom.", no "bad ext."
}

uint8_t bking3_state::mcu_status_r()
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}

void bking_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram();
	map(0x9000, 0x97ff).ram().w(FUNC(bking_state::playfield_w)).share(m_playfield_ram);
}

void bking_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w(FUNC(bking_state::xld_w<0>));
	map(0x01, 0x01).portr("IN1").w(FUNC(bking_state::yld_w<0>));
	map(0x02, 0x02).portr("DSWA").w(FUNC(bking_state::xld_w<1>));
	map(0x03, 0x03).portr("DSWB").w(FUNC(bking_state::yld_w<1>));
	map(0x04, 0x04).portr("DSWC").w(FUNC(bking_state::xld_w<2>));
	map(0x05, 0x05).rw(FUNC(bking_state::input_port_r<0>), FUNC(bking_state::yld_w<2>));
	map(0x06, 0x06).rw(FUNC(bking_state::input_port_r<1>), FUNC(bking_state::msk_w));
	map(0x07, 0x07).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x08, 0x08).w(FUNC(bking_state::cont1_w));
	map(0x09, 0x09).w(FUNC(bking_state::cont2_w));
	map(0x0a, 0x0a).w(FUNC(bking_state::cont3_w));
	map(0x0b, 0x0b).w(FUNC(bking_state::soundlatch_w));
//  map(0x0c, 0x0c).w(FUNC(bking_state::eport2_w));   this is not shown to be connected anywhere
	map(0x0d, 0x0d).w(FUNC(bking_state::hitclr_w));
	map(0x07, 0x1f).r(FUNC(bking_state::pos_r));
}

void bking3_state::io_map(address_map &map)
{
	bking_state::io_map(map);

	map(0x2f, 0x2f).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0x4f, 0x4f).rw(FUNC(bking3_state::mcu_status_r), FUNC(bking3_state::unk_w));
	map(0x60, 0x60).r(FUNC(bking3_state::extrarom_r));
	map(0x6f, 0x6f).rw(FUNC(bking3_state::ext_check_r), FUNC(bking3_state::addr_h_w));
	map(0x8f, 0x8f).w(FUNC(bking3_state::addr_l_w));
}

void bking_state::audio_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2fff).rom(); //only bking3
	map(0x4000, 0x43ff).ram();
	map(0x4400, 0x4401).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x4401, 0x4401).r("ay1", FUNC(ay8910_device::data_r));
	map(0x4402, 0x4403).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x4403, 0x4403).r("ay2", FUNC(ay8910_device::data_r));
	map(0x4800, 0x4800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x4802, 0x4802).rw(FUNC(bking_state::sndnmi_disable_r), FUNC(bking_state::sndnmi_enable_w));
	map(0xe000, 0xefff).rom();   // Space for diagnostic ROM
}


static INPUT_PORTS_START( bking )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	// continue inputs are labelled in schematics.
	// They are not connected though to any button

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // Continue 1
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // Continue 2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // Not Connected

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, "Holes Awarded" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x00, "Par Play: 0 Holes/Birdie: 1 Hole/Eagle: 2 Holes/Double Eagle: 4 Holes" )
	PORT_DIPSETTING(    0x01, "Par Play: 1 Hole/Birdie: 2 Holes/Eagle: 3 Holes/Double Eagle: 4 Holes" )
	PORT_DIPNAME( 0x02, 0x02, "Holes Awarded For Hole-in-One" ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x00, "3 Holes" )
	PORT_DIPSETTING(    0x02, "9 Holes" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR(Free_Play) ) PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x18, 0x18, "Holes Per Play" ) PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0x20, 0x20, "Self Test" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR(Flip_Screen) ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR(Cabinet) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR(Upright) )
	PORT_DIPSETTING(    0x80, DEF_STR(Cocktail) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWB:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWB:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "Appearance of Crow" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x02, "Crow Flight Pattern" ) PORT_DIPLOCATION("SWC:2,3") // "The hole from which a crow appears and flys with drawing a circle or a 8-shape in the air."
	PORT_DIPSETTING(    0x00, "1" ) // Circle 1,7,11,15,18 / 8-shape 3,5,9,13,17
	PORT_DIPSETTING(    0x02, "2" ) // Circle 1,10,16,18 / 8-shape 4,7,13,17
	PORT_DIPSETTING(    0x04, "3" ) // Circle 4,7,13,18 / 8-shape 1,10,16,17
	PORT_DIPSETTING(    0x06, "4" ) // Circle 3,5,9,13,18 / 8-shape 1,7,11,15,17
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SWC:4" ) // Listed as "Unused"
	PORT_DIPNAME( 0x10, 0x10, "Coin Display" ) PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x10, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "Year Display" ) PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x20, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "Check" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x00, "Check" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x80, 0x00, "Coin System" ) PORT_DIPLOCATION("SWC:8") // Default is "1 Way" according to manual
	PORT_DIPSETTING(    0x00, "1 Way" )
	PORT_DIPSETTING(    0x80, "2 Way" )

	PORT_START("TRACK0_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) // Sensitivity, clip, min, max */

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE // Sensitivity, clip, min, max

	PORT_START("TRACK1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_COCKTAIL // Sensitivity, clip, min, max

	PORT_START("TRACK1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL // Sensitivity, clip, min, max
INPUT_PORTS_END

static INPUT_PORTS_START( bking2 )
	PORT_INCLUDE( bking )

	PORT_MODIFY("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWA:6" ) // Listed as "Unused"
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	1024,   // 1024 characters
	3,      // 3 bits per pixel
	{ 0*1024*8*8, 1*1024*8*8, 2*1024*8*8 }, // the bitplanes are separated
	{ 7, 6, 5, 4, 3, 2, 1, 0 }, // reverse layout
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     // every char takes 8 consecutive bytes
};

static const gfx_layout crowlayout =
{
	16,32,  // 16*32 characters
	16,     // 16 characters
	2,      // 2 bits per pixel
	{ 0, 4 },
	{ 3*32*8+3, 3*32*8+2, 3*32*8+1, 3*32*8+0,
		2*32*8+3, 2*32*8+2, 2*32*8+1, 2*32*8+0,
		32*8+3,   32*8+2,   32*8+1,   32*8+0,
				3,        2,        1,        0 }, // reverse layout
	{ 31*8, 30*8, 29*8, 28*8, 27*8, 26*8, 25*8, 24*8,
		23*8, 22*8, 21*8, 20*8, 19*8, 18*8, 17*8, 16*8,
		15*8, 14*8, 13*8, 12*8, 11*8, 10*8,  9*8,  8*8,
		7*8,  6*8,  5*8,  4*8,  3*8,  2*8,  1*8,  0*8 },
	128*8    // every sprite takes 128 consecutive bytes
};

static const gfx_layout balllayout =
{
	8,16,  // 8*16 sprites
	8,     // 8 sprites
	1,  // 1 bit per pixel
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },   // pretty straightforward layout
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8    // every sprite takes 16 consecutive bytes
};

static GFXDECODE_START( gfx_bking )
	GFXDECODE_ENTRY( "playfield", 0, charlayout, 0,           4  )
	GFXDECODE_ENTRY( "crow",      0, crowlayout, 4*8,         4  )
	GFXDECODE_ENTRY( "ball1",     0, balllayout, 4*8+4*4,     4  )
	GFXDECODE_ENTRY( "ball2",     0, balllayout, 4*8+4*4+4*2, 4  )
GFXDECODE_END


void bking_state::port_b_w(uint8_t data)
{
	// don't know what this is... could be a filter
	if (data != 0x00)
		logerror("port_b = %02x\n", data);
}

void bking_state::machine_start()
{
	save_item(NAME(m_pc3259_output));
	save_item(NAME(m_pc3259_mask));
	save_item(NAME(m_xld));
	save_item(NAME(m_yld));;
	save_item(NAME(m_ball1_pic));
	save_item(NAME(m_ball2_pic));
	save_item(NAME(m_crow_pic));
	save_item(NAME(m_crow_flip));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_controller));
	save_item(NAME(m_hit));
}

void bking3_state::machine_start()
{
	bking_state::machine_start();

	save_item(NAME(m_addr_h));
	save_item(NAME(m_addr_l));
}

void bking_state::machine_reset()
{
	m_pc3259_output[0] = 0;
	m_pc3259_output[1] = 0;
	m_pc3259_output[2] = 0;
	m_pc3259_output[3] = 0;
	m_pc3259_mask = 0;
	m_xld[0] = 0;
	m_xld[1] = 0;
	m_xld[2] = 0;
	m_yld[0] = 0;
	m_yld[1] = 0;
	m_yld[2] = 0;
	m_ball1_pic = 0;
	m_ball2_pic = 0;
	m_crow_pic = 0;
	m_crow_flip = 0;
	m_palette_bank = 0;
	m_controller = 0;
	m_hit = 0;

	m_soundnmi->in_w<1>(0);
}

void bking3_state::machine_reset()
{
	bking_state::machine_reset();

	m_addr_h = 0;
	m_addr_l = 0;
}

void bking_state::bking(machine_config &config)
{
	// basic machine hardware
	z80_device &maincpu(Z80(config, "maincpu", 12_MHz_XTAL / 4)); // 3 MHz
	maincpu.set_addrmap(AS_PROGRAM, &bking_state::program_map);
	maincpu.set_addrmap(AS_IO, &bking_state::io_map);
	maincpu.set_vblank_int("screen", FUNC(bking_state::irq0_line_hold));

	Z80(config, m_audiocpu, 6_MHz_XTAL / 2);  // 3 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &bking_state::audio_map);
	// interrupts (from Jungle King hardware, might be wrong):
	// - no interrupts synced with vblank
	// - NMI triggered by the main CPU
	// - periodic IRQ, with frequency 6000000/(4*16*16*10*16) = 36.621 Hz,
	m_audiocpu->set_periodic_int(FUNC(bking_state::irq0_line_hold), attotime::from_hz((double)6'000'000 / (4*16*16*10*16)));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(bking_state::screen_update));
	m_screen->screen_vblank().set(FUNC(bking_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bking);
	PALETTE(config, m_palette, FUNC(bking_state::palette_init), 4*8 + 4*4 + 4*2 + 4*2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set(m_soundnmi, FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi).output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	AY8910(config, "ay1", 6_MHz_XTAL /4).add_route(ALL_OUTPUTS, "speaker", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", 6_MHz_XTAL / 4));
	ay2.port_a_write_callback().set("dac", FUNC(dac_byte_interface::data_w));
	ay2.port_b_write_callback().set(FUNC(bking_state::port_b_w));
	ay2.add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}

void bking3_state::bking3(machine_config &config)
{
	bking(config);

	// basic machine hardware
	subdevice<z80_device>("maincpu")->set_addrmap(AS_IO, &bking3_state::io_map);

	TAITO68705_MCU(config, m_bmcu, 3_MHz_XTAL);      // xtal is 3MHz, divided by 4 internally

	config.set_maximum_quantum(attotime::from_hz(6000));
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bking )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dm_11.f13", 0x0000, 0x1000, CRC(d84fe4f7) SHA1(3ad1641d05e4faca2be28052ccae8f81bc2255bb) )
	ROM_LOAD( "dm_12.f11", 0x1000, 0x1000, CRC(e065bbe6) SHA1(8d6d3334977c1eea1bf238817d59c25acd9d99f0) )
	ROM_LOAD( "dm_13.f10", 0x2000, 0x1000, CRC(aac7cddd) SHA1(12a8887bd8d3334e0d740a7f54374b0e48021140) )
	ROM_LOAD( "dm_14.f8",  0x3000, 0x1000, CRC(1179d074) SHA1(23df9a7e3e1bf42d6ea3a2d85629d27bd68e9af4) )
	ROM_LOAD( "dm_15.f7",  0x4000, 0x1000, CRC(fda31475) SHA1(784ffa089b7bd4ab4cbd454f4c1c26553a11fc48) )
	ROM_LOAD( "dm_16.f5",  0x5000, 0x1000, CRC(b6c3c3ed) SHA1(6c7f67d5eba35e32b556b531e848ef375123de78) )

	ROM_REGION( 0xf000, "audiocpu", 0 )
	ROM_LOAD( "dm_17.f4",  0x0000, 0x1000, CRC(54840bc3) SHA1(225daf7ff8a4095b0e69ce6ccce6d8eab26ec1c8) )
	ROM_LOAD( "dm_18.d4",  0x1000, 0x1000, CRC(2abadd42) SHA1(d921d333ec9b9140a7d3ce7aaddab35f45fae018) )
	ROM_LOAD( "diagrom",   0xe000, 0x1000, NO_DUMP )

	ROM_REGION( 0x6000, "playfield", 0 )
	ROM_LOAD( "dm_10.a5",  0x0000, 0x1000, CRC(fe96dd67) SHA1(11014602f926cf6edbf06e7b2acef92036b2f30a) )
	ROM_LOAD( "dm_09.a7",  0x1000, 0x1000, CRC(80c675d7) SHA1(e590a71a15ea485abf099eceaa16d5a1dbe0c3dc) )
	ROM_LOAD( "dm_08.a8",  0x2000, 0x1000, CRC(d9bd6b60) SHA1(3c790b6a69472e0a37f45baa00ce5c7d09e7b588) )
	ROM_LOAD( "dm_07.a10", 0x3000, 0x1000, CRC(65f7a0e4) SHA1(034dbf2fe384cb69963936e9f3029aa54e032e4a) )
	ROM_LOAD( "dm_06.a11", 0x4000, 0x1000, CRC(00fdbafc) SHA1(b2a8d9c96415fecee52f1c4691a5f10c96f484b1) )
	ROM_LOAD( "dm_05.a13", 0x5000, 0x1000, CRC(3e4fe925) SHA1(9ed73601c8b34ea8889717cbb3ee4a00ab7ab458) )

	ROM_REGION( 0x0800, "crow", 0 )
	ROM_LOAD( "dm_01.e10", 0x0000, 0x0800, CRC(e5663f0b) SHA1(b0fed8c4cdff7b12bb220e51d5b7188933934a34) )

	ROM_REGION( 0x0800, "ball1", 0 )
	ROM_LOAD( "dm_02.e7",  0x0000, 0x0800, CRC(fc9cec31) SHA1(5ab1c9b3b15334c6ec06826005ecb66b34d8879a) ) // only the first 128 bytes used

	ROM_REGION( 0x0800, "ball2", 0 )
	ROM_LOAD( "dm_02.e9",  0x0000, 0x0800, CRC(fc9cec31) SHA1(5ab1c9b3b15334c6ec06826005ecb66b34d8879a) ) // only the first 128 bytes used

	ROM_REGION( 0x0020, "collision_detection", 0 )
	ROM_LOAD( "dm04.c2",   0x0000, 0x0020, CRC(4cb5bd32) SHA1(8851bae033ba67516d5ff6888e5daef10c2116ee) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "dm_03.d1",  0x0000, 0x0200, CRC(61b7a9ff) SHA1(4302de0c0dad2b871ad4719ad934beaee05a0c40) ) // palette
ROM_END

ROM_START( bking2 ) // Top board: DMO70003A, middle board: DMO00001A, bottom board: DMO70002A
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ad6_01.13f",       0x0000, 0x1000, CRC(078ada3f) SHA1(5e82a6d27c65fe29d664dbfc2ede547c0f4869f0) )
	ROM_LOAD( "ad6_02.11f",       0x1000, 0x1000, CRC(c37d110a) SHA1(7aec6c949d1cf136c3037140bd86597feaf29108) )
	ROM_LOAD( "ad6_03.10f",       0x2000, 0x1000, CRC(2ba5c681) SHA1(d0df24f5e52e6162b40308d8aa38b0348a100f37) )
	ROM_LOAD( "ad6_04.8f",        0x3000, 0x1000, CRC(8fad54e8) SHA1(55edc185914686d42efd848a402f78884d42292b) )
	ROM_LOAD( "ad6_05.7f",        0x4000, 0x1000, CRC(b4de6b58) SHA1(f62bdc3128b226454b1f00a4cbe382e1219a11b0) )
	ROM_LOAD( "ad6_06.5f",        0x5000, 0x1000, CRC(9ac43b87) SHA1(dd562fee01c81317978d1bd8a0178e3d9be6145a) )
	ROM_LOAD( "ad6_07.4f",        0x6000, 0x1000, CRC(b3ed40b7) SHA1(d481094c0381234314f797928e3cdb22f36f4e32) )
	ROM_LOAD( "ad6_08.2f",        0x7000, 0x1000, CRC(8fddb2e8) SHA1(6ee5f09d154440851f370a97b35450e3726e14e7) )

	ROM_REGION( 0xf000, "audiocpu", 0 )
	ROM_LOAD( "ad6_15.4f",           0x0000, 0x1000, CRC(f045d0fe) SHA1(3b34081fa6cd0423236d09b6f23e8cf8cfd627c5) )
	ROM_LOAD( "ad6_16.4d",           0x1000, 0x1000, CRC(92d50410) SHA1(e6f4c27031744bbc832a1eb121a7dba4da5286c4) )
	ROM_LOAD( "diagrom",   0xe000, 0x1000, NO_DUMP )

	ROM_REGION( 0x6000, "playfield", 0 )
	ROM_LOAD( "ad6_14.5a",        0x0000, 0x1000, CRC(52636a94) SHA1(185c4455bd9bb23d14aa2f6f7baa74959da08fc2) )
	ROM_LOAD( "ad6_13.7a",        0x1000, 0x1000, CRC(6b9e0564) SHA1(6cdd3820caa3825e98b61fe260960cc05c04d032) )
	ROM_LOAD( "ad6_12.8a",        0x2000, 0x1000, CRC(c6d685d9) SHA1(2dd2fda365e6bdf9aa26de90650f4a2588ea0515) )
	ROM_LOAD( "ad6_11.10a",       0x3000, 0x1000, CRC(2b949987) SHA1(a94666c4f2fdc25399f7976ed2c25fd454387be6) )
	ROM_LOAD( "ad6_10.11a",       0x4000, 0x1000, CRC(eb96f948) SHA1(295ba5a620a8a85a121d3e823804adceeeef64d9) )
	ROM_LOAD( "ad6_09.13a",       0x5000, 0x1000, CRC(595e3dd4) SHA1(9dd3388ce704dd5473af034716cd8d48df3dc495) )

	ROM_REGION( 0x0800, "crow", 0 )
	ROM_LOAD( "dm_01.e10",           0x0000, 0x0800, CRC(e5663f0b) SHA1(b0fed8c4cdff7b12bb220e51d5b7188933934a34) )

	ROM_REGION( 0x0800, "ball1", 0 )
	ROM_LOAD( "dm_02.e7",           0x0000, 0x0800, CRC(fc9cec31) SHA1(5ab1c9b3b15334c6ec06826005ecb66b34d8879a) ) // only the first 128 bytes used

	ROM_REGION( 0x0800, "ball2", 0 )
	ROM_LOAD( "dm_02.e9",           0x0000, 0x0800, CRC(fc9cec31) SHA1(5ab1c9b3b15334c6ec06826005ecb66b34d8879a) ) // only the first 128 bytes used

	ROM_REGION( 0x0020, "collision_detection", 0 )
	ROM_LOAD( "dm_04.2c",    0x0000, 0x0020, CRC(4cb5bd32) SHA1(8851bae033ba67516d5ff6888e5daef10c2116ee) ) // mb7051(?)

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "dm_03.2d",    0x0000, 0x0200, CRC(61b7a9ff) SHA1(4302de0c0dad2b871ad4719ad934beaee05a0c40) ) // palette, 82s141

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.1",  0x0000, 0x0104, CRC(e75d19f5) SHA1(d51cbb247760312b8884bbd0478a321eee05034f) )
	ROM_LOAD( "pal16l8.2",  0x0200, 0x0104, CRC(0302b683) SHA1(91bfba22c883adb15309d9ec0c42b5b744887c77) )
	ROM_LOAD( "pal16l8.3",  0x0400, 0x0104, CRC(a609d0cf) SHA1(7a18040720646c2dff4c1dc6f272c6a69e538c47) )
ROM_END

/*
Birdie King 3
Taito, 1984

A golf game using a trackball. Uses same harness/pinout as
Elevator Action, Victorious Nine, Jolly Jogger (etc)

PCB Layouts
(Note! There are no PALs on ANY of the PCBs)

Top PCB
-------
DMO70003A
K1000173B (sticker)
 |------------------------|
 |                        |
|-|                       |
| |  Z80        2114      |
| |             2114      |
| |N                      |
| |  A24_18.4F            |
| |      A24_19.4D        |
|-|           A24_20.4B   |
 |                        |
 |  AY3-8910              |
 |                        |
 |      AY3-8910          |
 |                        |
 |                        |
 |                   6MHz |
 |                        |
 |                        |
 |                        |
 | LM3900  LM3900  LM3900 |
 |------------------------|
Notes:
      Z80      - Clock 3.000MHz [6/2]
      AY3-8910 - Clock 1.500MHz [6/4]
      2114     - 1kx4 SRAM (DIP18)
      N        - Flat cable connector, joins to main board
      LM3900   - National LM3900 Quad, dual-input amplifier IC (DIP14)
      plus many resistors/capacitors below the AY3-8910's


Sub PCB (below Top PCB)
-----------------------
SUB PCB J910 0010 A
        K910 0018 A
|------------------------|
|           *            |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|A24_21.IC25             |
|                        |
|                   S2   |
|                        |
|         3MHz         S1|
|T    A24_22.IC17        |
|------------------------|
Notes:
      A24_22.IC17 - Motorola 68705P5 Microcontroller, clock 3.000MHz
      A24_21.IC25 - 2732 EPROM
      *           - DIP24 socket with flat cable below PCB, joins to main board
      T           - 4-pin power connector (5 volts) coming from main board
      S1          - Flat cable connector, joins to main PCB to connector S
      S2          - Flat cable connector, joins to bottom PCB to connector S



Main PCB
--------
J1100001A
K1100001A
M4300001D (sticker)
K1100032A (sticker)
|--------------------------------------------------------------|
|      M3712    VOL             A24_03.2D             M53354   |
|H                                        DM-04.2C            |-|
|                     N                                  *    | |
|           MC14584                                          R| |
|                                                             | |
|                                                             | |
|                                                             | |
|                                                             |-|
|                                                              |
|G          LM3900                                             |
|    MC14093                                                   |
|                   A24_01.7E                                 |-|
|                                                             | |
|                   A24_01.9E                                 | |
|    MC14584                                                 S| |
|                   A24_02.10E                                | |
|    MC14584                    MC1455                        | |
|                                                             |-|
|    DSWC   DSWB   DSWA                                        |
|--------------------------------------------------------------|
Notes:
      M53354    - ?, maybe 74LS154? (DIP24)
      MC1455    - Motorola MC1455 Monolithic Timing Circuit (NE555 compatible)
      A24_01/02 - 2716 EPROMs
      A24_03    - Signetics 82S141 PROM (DIP24)
      DM-04     - Signetics 82S123 PROM (DIP8)
      *         - DIP24 socket with flat cable, joins to SUB PCB DIP24 socket
      R/S       - Flat cable connector, R joins to main board, S joins to SUB PCB
      G         - 22-way Edge Connector
      N         - Flat cable connector, joins to TOP PCB
      H         - 12-pin power connector
      VSync     - 60Hz
      HSync     - 15.67kHz


Bottom PCB
----------
DMO70002A
DMN00002A
K1000172B (sticker)
|--------------------------------------------------------------|
|   A24_17.13A         2114    Z80             A24_04.13F      |
|                                                             |-|
|   A24_16.11A         2114                    A24_05.11F     | |
|                                                            S| |
|   A24_15.10A                                 A24_06.10F     | |
|                                                             | |
|   A24_14.8A                                  A24_07.8F      | |
|                                                             |-|
|T  A24_13.7A                                  A24_08.7F       |
|                                                              |
|   A24_12.5A                                  A24_09.5F       |
|                                                             |-|
|                                              A24_10.4F      | |
|            2114                                             | |
|                                              A24_11.2F     R| |
|            2114                                             | |
|                                                             | |
|            2114                                             |-|
|                                                    12MHz     |
|--------------------------------------------------------------|
Notes:
      R/S   - Flat cable connector, R joins to main board, S joins to SUB PCB
      T     - 18-Way Edge Connector (for +5V/GND only)
      A24_* - 2732 EPROMs
      Z80   - Clock 3.000MHz [12/4]
      2114  - 1kx4 SRAM (DIP18)
*/

ROM_START( bking3 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a24-04.13f",   0x0000, 0x1000, CRC(a0c319a6) SHA1(6b79667288113fde43975fcfd05e93d8e45bf92d) )
	ROM_LOAD( "a24-05.11f",   0x1000, 0x1000, CRC(fedc9b4a) SHA1(3ac22c3ca09df9983f3c8c05e807ecf5999c9fc5) )
	ROM_LOAD( "a24-06.10f",   0x2000, 0x1000, CRC(6a116ebf) SHA1(e58b1f75eb75027749a900b27107930e9072ca5a) )
	ROM_LOAD( "a24-07.8f",    0x3000, 0x1000, CRC(75a74d2d) SHA1(d433e8fcf3819b845936e7e107fef414f72bfc16) )
	ROM_LOAD( "a24-08.7f",    0x4000, 0x1000, CRC(9fe07cf9) SHA1(23fdae48e519a171bf4adeeadf2fdfedfd56f4ea) )
	ROM_LOAD( "a24-09.5f",    0x5000, 0x1000, CRC(51545ced) SHA1(4addad527c6fd675506bf584ec8670a23767787c) )
	ROM_LOAD( "a24-10.4f",    0x6000, 0x1000, CRC(a86b3e62) SHA1(f97a13e31e622b5ac55c23458c65a49c2998196a) )
	ROM_LOAD( "a24-11.2f",    0x7000, 0x1000, CRC(b39db430) SHA1(4f48a34f3aaa1e998a4a5656bc3f399d9e6633c4) )

	ROM_REGION( 0xf000, "audiocpu", 0 )
	ROM_LOAD( "a24-18.4f",    0x0000, 0x1000, CRC(fa3bfa98) SHA1(733924e154e301a9d692d80b485afc4ab0e200c1) )
	ROM_LOAD( "a24-19.4d",    0x1000, 0x1000, CRC(817f9c2a) SHA1(7365ecf2700e1fd13016408f5493f8d51aab5bbd) )
	ROM_LOAD( "a24-20.4b",    0x2000, 0x1000, CRC(0e9e16d6) SHA1(43c69602a8d9c34c527ce54472db84168acc4ef4) )
	ROM_LOAD( "diagrom",      0xe000, 0x1000, NO_DUMP )

	ROM_REGION( 0x800, "bmcu:mcu", 0 )
	ROM_LOAD( "a24_22.ic17",  0x000, 0x800, CRC(27c497d5) SHA1(c6c72bbf0537da53148fa0a56d412ab46129d29c) )  //M68705P5S uC 3MHz xtal

	ROM_REGION( 0x6000, "playfield", 0 )
	ROM_LOAD( "a24-12.5a",    0x0000, 0x1000, CRC(c5fe4817) SHA1(fbf82d9d85e18b76c7e939932df074a545e73f42) )
	ROM_LOAD( "a24-13.7a",    0x1000, 0x1000, CRC(728bac57) SHA1(3daa246f95b31c971e5418f55b821616d0bce25d) )
	ROM_LOAD( "a24-14.8a",    0x2000, 0x1000, CRC(63cd0009) SHA1(10fcfeec70b23e2206c4f4bf686dc6a48ecba1ce) )
	ROM_LOAD( "a24-15.10a",   0x3000, 0x1000, CRC(590275d0) SHA1(563bebb344c606ca3a2124fc7a8804935a011e90) )
	ROM_LOAD( "a24-16.11a",   0x4000, 0x1000, CRC(728d069e) SHA1(b4adb14281e4874bab7cec7f38ade70b5b7c6b8f) )
	ROM_LOAD( "a24-17.13a",   0x5000, 0x1000, CRC(4c04c4f2) SHA1(8e9eee6d89e91910b398d42ac86597ef91baad96) )

	ROM_REGION( 0x0800, "crow", 0 )
	ROM_LOAD( "a24-02.10e",   0x0000, 0x0800, CRC(8560da46) SHA1(56f249f0b56336daac1a3624ef9b71354bb8ca40) )

	ROM_REGION( 0x0800, "ball1", 0 )
	ROM_LOAD( "a24-01.7e",    0x0000, 0x0800, CRC(369c01e1) SHA1(196e12d0bcaf74cefe4cad3fccb69d104aab061e) ) // only the first 128 bytes used

	ROM_REGION( 0x0800, "ball2", 0 )
	ROM_LOAD( "a24-01.9e",    0x0000, 0x0800, CRC(369c01e1) SHA1(196e12d0bcaf74cefe4cad3fccb69d104aab061e) ) // only the first 128 bytes used

	ROM_REGION( 0x0020, "collision_detection", 0 )
	ROM_LOAD( "82s123.2c",    0x0000, 0x0020, CRC(4cb5bd32) SHA1(8851bae033ba67516d5ff6888e5daef10c2116ee) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a24_03.2d",    0x0000, 0x0200, CRC(599a6cbe) SHA1(eed8592aaba7b2b6d06f26a2b8720a288f9ad90f) ) // palette

	ROM_REGION( 0x1000, "extra", 0 )
	ROM_LOAD( "a24-21.25",    0x0000, 0x1000, CRC(3106fcac) SHA1(08454adfb58e5df84140d86ed52fa4ef684df9f1) ) // extra ROM on the same SUB PCB where the MCU is
ROM_END

} // anonymous namespace


GAME( 1982, bking,  0, bking,  bking,  bking_state,  empty_init, ROT270, "Taito Corporation", "Birdie King",   MACHINE_SUPPORTS_SAVE )
GAME( 1983, bking2, 0, bking,  bking2, bking_state,  empty_init, ROT90,  "Taito Corporation", "Birdie King 2", MACHINE_SUPPORTS_SAVE )
GAME( 1984, bking3, 0, bking3, bking2, bking3_state, empty_init, ROT90,  "Taito Corporation", "Birdie King 3", MACHINE_SUPPORTS_SAVE )
