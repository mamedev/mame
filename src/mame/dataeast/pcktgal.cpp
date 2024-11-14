// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Pocket Gal                      (c) 1987 Data East Corporation
    Pocket Gal (Bootleg)            (c) 1989 Yada East Corporation(!!!)
    Super Pool III                  (c) 1989 Data East Corporation
    Pocket Gal 2                    (c) 1989 Data East Corporation
    Super Pool III (I-Vics Inc)     (c) 1990 Data East Corporation

    Pocket Gal (Bootleg) is often called 'Sexy Billiards'

    Emulation by Bryan McPhail, mish@tendril.co.uk

    TODO: determine how to best deal with pcktgalba peculiarities without
          duplicating too much code
***************************************************************************/

#include "emu.h"

#include "deco222.h"
#include "decbac06.h"

#include "cpu/m6502/m6502.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class pcktgal_state : public driver_device
{
public:
	pcktgal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_tilegen(*this, "tilegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank%u", 0U),
		m_soundbank(*this, "soundbank")
	{ }

	void init_original();

	void bootleg(machine_config &config);
	void pcktgal(machine_config &config);
	void pcktgal2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<deco_bac06_device> m_tilegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank_array<2> m_mainbank;
	required_memory_bank m_soundbank;

	uint16_t m_msm5205next = 0;
	uint8_t m_toggle = 0;

	void bank_w(uint8_t data);
	void sound_bank_w(uint8_t data);
	void sound_w(uint8_t data);
	void adpcm_data_w(uint8_t data);
	uint8_t sound_unk_r();
	void adpcm_int(int state);

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bool flip_screen);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};



void pcktgal_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0 = BIT(color_prom[i], 0);
		int bit1 = BIT(color_prom[i], 1);
		int bit2 = BIT(color_prom[i], 2);
		int bit3 = BIT(color_prom[i], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		bit2 = BIT(color_prom[i], 6);
		bit3 = BIT(color_prom[i], 7);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = BIT(color_prom[i + palette.entries()], 0);
		bit1 = BIT(color_prom[i + palette.entries()], 1);
		bit2 = BIT(color_prom[i + palette.entries()], 2);
		bit3 = BIT(color_prom[i + palette.entries()], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void pcktgal_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bool flip_screen)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		if (m_spriteram[offs] != 0xf8)
		{
			int sx = 240 - m_spriteram[offs + 2];
			int sy = 240 - m_spriteram[offs];

			int flipx = m_spriteram[offs + 1] & 0x04;
			int flipy = m_spriteram[offs + 1] & 0x02;
			if (flip_screen) {
				sx = 240 - sx;
				sy = 240 - sy;
				if (flipx) flipx = 0; else flipx = 1;
				if (flipy) flipy = 0; else flipy = 1;
			}

			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					m_spriteram[offs + 3] + ((m_spriteram[offs + 1] & 1) << 8),
					(m_spriteram[offs + 1] & 0x70) >> 4,
					flipx, flipy,
					sx, sy, 0);
		}
	}
}

uint32_t pcktgal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen->get_flip_state();
	m_tilegen->set_flip_screen(flip);
	m_tilegen->deco_bac06_pf_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(bitmap, cliprect, flip);
	return 0;
}

uint32_t pcktgal_state::screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen->get_flip_state();
	m_tilegen->set_flip_screen(flip);
	// the bootleg doesn't properly set the tilemap registers, because it's on non-original hardware, which probably doesn't have the flexible tilemaps.
	m_tilegen->deco_bac06_pf_draw_bootleg(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0, 2, 0);
	draw_sprites(bitmap, cliprect, flip);
	return 0;
}


/***************************************************************************/

void pcktgal_state::bank_w(uint8_t data)
{
	m_mainbank[0]->set_entry(data & 1);

	m_mainbank[1]->set_entry((data & 2) >> 1);
}

void pcktgal_state::sound_bank_w(uint8_t data)
{
	m_soundbank->set_entry((data >> 2) & 1);
	m_msm->reset_w((data & 2) >> 1);
}

void pcktgal_state::sound_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void pcktgal_state::adpcm_int(int state)
{
	m_msm->data_w(m_msm5205next >> 4);
	m_msm5205next <<= 4;

	m_toggle = 1 - m_toggle;
	if (m_toggle)
		m_audiocpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
}

void pcktgal_state::adpcm_data_w(uint8_t data)
{
	m_msm5205next = data;
}

uint8_t pcktgal_state::sound_unk_r()
{
	// POST only? Unknown purpose
//  m_msm->reset_w(0);
	return 0;
}

/***************************************************************************/

void pcktgal_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0fff).rw(m_tilegen, FUNC(deco_bac06_device::pf_data_8bit_r), FUNC(deco_bac06_device::pf_data_8bit_w));
	map(0x1000, 0x11ff).ram().share(m_spriteram);
	map(0x1800, 0x1800).portr("P1");
	map(0x1800, 0x1807).w(m_tilegen, FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x1810, 0x181f).rw(m_tilegen, FUNC(deco_bac06_device::pf_control1_8bit_r), FUNC(deco_bac06_device::pf_control1_8bit_w));
	map(0x1a00, 0x1a00).portr("P2").w(FUNC(pcktgal_state::sound_w));
	map(0x1c00, 0x1c00).portr("DSW").w(FUNC(pcktgal_state::bank_w));
	map(0x4000, 0x5fff).bankr(m_mainbank[0]);
	map(0x6000, 0x7fff).bankr(m_mainbank[1]);
	map(0x8000, 0xffff).rom();
}


/***************************************************************************/

void pcktgal_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0801).w("ym1", FUNC(ym2203_device::write));
	map(0x1000, 0x1001).w("ym2", FUNC(ym3812_device::write));
	map(0x1800, 0x1800).w(FUNC(pcktgal_state::adpcm_data_w)); // ADPCM data for the MSM5205 chip
	map(0x2000, 0x2000).w(FUNC(pcktgal_state::sound_bank_w));
	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x3400, 0x3400).r(FUNC(pcktgal_state::sound_unk_r));
	map(0x4000, 0x7fff).bankr(m_soundbank);
	map(0x8000, 0xffff).rom();
}


/***************************************************************************/

static INPUT_PORTS_START( pcktgal )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Allow 2 Players Game" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Time" )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "120" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	4096,
	4,
	{ 0x10000*8, 0, 0x18000*8, 0x8000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8  // every char takes 8 consecutive bytes
};

static const gfx_layout bootleg_charlayout =
{
	8,8,    // 8*8 characters
	4096,
	4,
	{ 0x18000*8, 0x8000*8, 0x10000*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8  // every char takes 8 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	1024,   // 1024 sprites
	2,    // 2 bits per pixel
	{ 0x8000*8, 0 },
	{ 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every char takes 8 consecutive bytes
};

static const gfx_layout bootleg_spritelayout =
{
	16,16,  // 16*16 sprites
	1024,   // 1024 sprites
	2,    // 2 bits per pixel
	{ 0x8000*8, 0 },
	{ 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0, 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every char takes 8 consecutive bytes
};

static GFXDECODE_START( gfx_pcktgal )
	GFXDECODE_ENTRY( "chars",   0x00000, charlayout,   256, 16 )
	GFXDECODE_ENTRY( "sprites", 0x00000, spritelayout,   0,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_bootleg )
	GFXDECODE_ENTRY( "chars",   0x00000, bootleg_charlayout,   256, 16 )
	GFXDECODE_ENTRY( "sprites", 0x00000, bootleg_spritelayout,   0,  8 )
GFXDECODE_END


/***************************************************************************/


void pcktgal_state::machine_start()
{
	m_mainbank[0]->configure_entry(0, memregion("maincpu")->base());
	m_mainbank[0]->configure_entry(1, memregion("maincpu")->base() + 0x4000);
	m_mainbank[1]->configure_entry(0, memregion("maincpu")->base() + 0x2000);
	m_mainbank[1]->configure_entry(1, memregion("maincpu")->base() + 0x6000);
	m_soundbank->configure_entries(0, 2, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_msm5205next));
	save_item(NAME(m_toggle));
}

void pcktgal_state::pcktgal(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pcktgal_state::main_map);

	DECO_222(config, m_audiocpu, 1500000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &pcktgal_state::sound_map);
	// IRQs are caused by the ADPCM chip
	// NMIs are caused by the main CPU

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(pcktgal_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_pcktgal);
	PALETTE(config, m_palette, FUNC(pcktgal_state::palette), 512);

	DECO_BAC06(config, m_tilegen, 0);
	m_tilegen->set_gfx_region_wide(0, 0, 0);
	m_tilegen->set_gfxdecode_tag(m_gfxdecode);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2203(config, "ym1", 1500000).add_route(ALL_OUTPUTS, "mono", 0.60);
	YM3812(config, "ym2", 3000000).add_route(ALL_OUTPUTS, "mono", 1.00);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(pcktgal_state::adpcm_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  // 8kHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.70);
}

void pcktgal_state::bootleg(machine_config &config)
{
	pcktgal(config);
	m_gfxdecode->set_info(gfx_bootleg);
	subdevice<screen_device>("screen")->set_screen_update(FUNC(pcktgal_state::screen_update_bootleg));
}

void pcktgal_state::pcktgal2(machine_config &config)
{
	pcktgal(config);
	M6502(config.replace(), m_audiocpu, 1500000); // doesn't use the encrypted 222
	m_audiocpu->set_addrmap(AS_PROGRAM, &pcktgal_state::sound_map);
}

/***************************************************************************/

ROM_START( pcktgal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eb04.j7", 0x00000, 0x10000, CRC(8215d60d) SHA1(ac26dfce7e215be21f2a17f864c5e966b8b8322e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eb03.f2", 0x00000, 0x10000, CRC(cb029b02) SHA1(fbb3da08ed05ae73fbeeb13e0e2ff735aaf83db8) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "eb01.d11", 0x00000, 0x10000, CRC(63542c3d) SHA1(4f42af99a6d9d4766afe0bebe10d6a97811a0082) )
	ROM_LOAD( "eb02.d12", 0x10000, 0x10000, CRC(a9dcd339) SHA1(245824ab86cdfe4b842ce1be0af60f2ff4c6ae07) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "eb00.a1", 0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14", 0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) // 82s147.084
	ROM_LOAD( "eb06.k15", 0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) // 82s131.101
ROM_END

ROM_START( pcktgalb )  // bootleg - "Yada East Corporation"
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sexybill.001", 0x00000, 0x10000, CRC(4acb3e84) SHA1(c83d03969587c6be80fb8fc84afe250907674a44) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eb03.f2", 0x00000, 0x10000, CRC(cb029b02) SHA1(fbb3da08ed05ae73fbeeb13e0e2ff735aaf83db8) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "sexybill.005", 0x00000, 0x10000, CRC(3128dc7b) SHA1(d011181e544b8284ecdf54578da5469804e06c63) )
	ROM_LOAD( "sexybill.006", 0x10000, 0x10000, CRC(0fc91eeb) SHA1(9d9a54c8dd41c10d07aabb6a2d8dbaf35c6e4533) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "sexybill.003", 0x00000, 0x08000, CRC(58182daa) SHA1(55ce4b0ea2cb1c559c12815c9e453624e0d95515) )
	ROM_LOAD( "sexybill.004", 0x08000, 0x08000, CRC(33a67af6) SHA1(6d9c04658ed75b970821a5c8b1f60c3c08fdda0a) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14", 0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) // 82s147.084
	ROM_LOAD( "eb06.k15", 0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) // 82s131.101

	ROM_REGION( 0x0400, "plds", 0 ) // same as official sets?
	ROM_LOAD( "pal16l8", 0x0000, 0x0104, CRC(b8d4b318) SHA1(6dd68892501c9b61714aaa7a3cfe14cc8ad1a877) )
	ROM_LOAD( "pal16r6", 0x0200, 0x0104, CRC(43aad537) SHA1(892104f4315d7a739718ce32b910694ea9b13fae) ) // also seen peel18CV8 used on other boards
ROM_END

ROM_START( pcktgalba )  // strange bootleg with 2 connected PCBs, one for the Pocket Gal bootleg and one for an unknown card game. Pocket Gal used as cover for a stealth gambling game?
	// Pocket Gal PCB: standard chips emulated in this driver, with no Data East customs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sex_v1.3", 0x00000, 0x10000, CRC(e278da6b) SHA1(71306fc8f8129cb2f924c67fe8dfdf82f02652e8) ) // minor differences to pcktgalb, probably to allow for game switching

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2_sex", 0x00000, 0x10000, CRC(cb029b02) SHA1(fbb3da08ed05ae73fbeeb13e0e2ff735aaf83db8) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "5_sex", 0x00000, 0x10000, CRC(3128dc7b) SHA1(d011181e544b8284ecdf54578da5469804e06c63) )
	ROM_LOAD( "6_sex", 0x10000, 0x10000, CRC(0fc91eeb) SHA1(9d9a54c8dd41c10d07aabb6a2d8dbaf35c6e4533) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "3_sex", 0x00000, 0x08000, CRC(58182daa) SHA1(55ce4b0ea2cb1c559c12815c9e453624e0d95515) )
	ROM_LOAD( "4_sex", 0x08000, 0x08000, CRC(33a67af6) SHA1(6d9c04658ed75b970821a5c8b1f60c3c08fdda0a) )

	ROM_REGION( 0x0400, "proms", 0 ) // not dumped for this PCB
	ROM_LOAD( "prom1", 0x0000, 0x0200, BAD_DUMP CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) )
	ROM_LOAD( "prom2", 0x0200, 0x0200, BAD_DUMP CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) )

	// unknown card game PCB: Z84C00AB6 (Z80), 2 scratched off chips (possibly I8255?), AY38912A/P, 4 8-dip banks
	ROM_REGION( 0x4000, "z80", 0 )
	ROM_LOAD( "7_sex.u45", 0x0000, 0x4000, CRC(65b0b6d0) SHA1(29dc3da40ff990df943b3b0e7474a0ba3fbb6468) ) // seems to contain 2 versions of the program. The second one is almost identical to unkitpkr in midcoin/wallc.cpp

	ROM_REGION( 0x6000, "card_chars", 0 )
	ROM_LOAD( "8_sex.u35",  0x0000, 0x2000, CRC(36e450e5) SHA1(848000d656cd00d32898c22677940f11789e50d4) ) // 2nd half is identical to unkitpkr in midcoin/wallc.cpp
	ROM_LOAD( "9_sex.u36",  0x2000, 0x2000, CRC(ffcc1198) SHA1(d90ae88e2755f614fdea11cd6935366f4d588144) ) // "
	ROM_LOAD( "10_sex.u37", 0x4000, 0x2000, CRC(73cf56a0) SHA1(fd2fdd997bca7b96ddd3898f0b6279c1dd60ec92) ) // "
ROM_END

ROM_START( pcktgal2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eb04-2.j7", 0x00000, 0x10000, CRC(0c7f2905) SHA1(882dbc1888a0149486c1fac5568dc3d297c2dadd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eb03-2.f2", 0x00000, 0x10000, CRC(9408ffb4) SHA1(ddcb67da4acf3d986d54ad10404f213528a8bb62) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "eb01-2.rom", 0x00000, 0x10000, CRC(e52b1f97) SHA1(4814fe3b2eb08ac173e09ffadc6e5daa9affa1a0) )
	ROM_LOAD( "eb02-2.rom", 0x10000, 0x10000, CRC(f30d965d) SHA1(a787457b33ad39e78fcf8da0715fab7a63869bf9) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "eb00.a1", 0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14", 0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) // 82s147.084
	ROM_LOAD( "eb06.k15", 0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) // 82s131.101
ROM_END

ROM_START( pcktgal2j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eb04-2.j7", 0x00000, 0x10000, CRC(0c7f2905) SHA1(882dbc1888a0149486c1fac5568dc3d297c2dadd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eb03-2.f2", 0x00000, 0x10000, CRC(9408ffb4) SHA1(ddcb67da4acf3d986d54ad10404f213528a8bb62) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "eb01-2.d11", 0x00000, 0x10000, CRC(8f42ab1a) SHA1(315fb26bbe004c08629a0a3a6e9d129768119e6b) )
	ROM_LOAD( "eb02-2.d12", 0x10000, 0x10000, CRC(f394cb35) SHA1(f351b8b6fd8a6637ef9031f7a410a334da8ea5ae) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "eb00.a1", 0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14", 0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) // 82s147.084
	ROM_LOAD( "eb06.k15", 0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) // 82s131.101
ROM_END

ROM_START( spool3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eb04-2.j7", 0x00000, 0x10000, CRC(0c7f2905) SHA1(882dbc1888a0149486c1fac5568dc3d297c2dadd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eb03-2.f2", 0x00000, 0x10000, CRC(9408ffb4) SHA1(ddcb67da4acf3d986d54ad10404f213528a8bb62) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "deco2.bin", 0x00000, 0x10000, CRC(0a23f0cf) SHA1(8554215001ffc9e6f141e57cc11b400a853f89f2) )
	ROM_LOAD( "deco3.bin", 0x10000, 0x10000, CRC(55ea7c45) SHA1(a8a6ff0c8a5aaee3afbfc3e71a171fb1d2360b45) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "eb00.a1", 0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14", 0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) // 82s147.084
	ROM_LOAD( "eb06.k15", 0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) // 82s131.101
ROM_END

ROM_START( spool3i )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "de1.bin", 0x00000, 0x10000, CRC(a59980fe) SHA1(64b55af4d0b314d14184784e9f817b56be0f24f2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eb03-2.f2", 0x00000, 0x10000, CRC(9408ffb4) SHA1(ddcb67da4acf3d986d54ad10404f213528a8bb62) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "deco2.bin", 0x00000, 0x10000, CRC(0a23f0cf) SHA1(8554215001ffc9e6f141e57cc11b400a853f89f2) )
	ROM_LOAD( "deco3.bin", 0x10000, 0x10000, CRC(55ea7c45) SHA1(a8a6ff0c8a5aaee3afbfc3e71a171fb1d2360b45) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "eb00.a1", 0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14", 0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) // 82s147.084
	ROM_LOAD( "eb06.k15", 0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) // 82s131.101
ROM_END

/***************************************************************************/



void pcktgal_state::init_original()
{
	uint8_t *rom = memregion("chars")->base();
	const int len = memregion("chars")->bytes();

	// Tile graphics ROMs have some swapped lines, original version only
	for (int i = 0x00000; i < len; i += 32)
	{
		int temp[16];
		for (int j = 0; j < 16; j++)
		{
			temp[j] = rom[i + j + 16];
			rom[i + j + 16] = rom[i + j];
			rom[i + j] = temp[j];
		}
	}
}

} // anonymous namespace


/***************************************************************************/

GAME( 1987, pcktgal,  0,       pcktgal, pcktgal, pcktgal_state, init_original,  ROT0, "Data East Corporation", "Pocket Gal (Japan)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1987, pcktgalb, pcktgal, bootleg, pcktgal, pcktgal_state, empty_init,     ROT0, "bootleg", "Pocket Gal (Yada East bootleg)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1987, pcktgalba,pcktgal, bootleg, pcktgal, pcktgal_state, empty_init,     ROT0, "bootleg", "Pocket Gal / unknown card game",                         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // only the Pocket Gal game is emulated
GAME( 1989, pcktgal2, pcktgal, pcktgal2,pcktgal, pcktgal_state, init_original,  ROT0, "Data East Corporation", "Pocket Gal 2 (English)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1989, pcktgal2j,pcktgal, pcktgal2,pcktgal, pcktgal_state, init_original,  ROT0, "Data East Corporation", "Pocket Gal 2 (Japanese)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1989, spool3,   pcktgal, pcktgal2,pcktgal, pcktgal_state, init_original,  ROT0, "Data East Corporation", "Super Pool III (English)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1990, spool3i,  pcktgal, pcktgal2,pcktgal, pcktgal_state, init_original,  ROT0, "Data East Corporation (I-Vics license)", "Super Pool III (I-Vics)", MACHINE_SUPPORTS_SAVE )
