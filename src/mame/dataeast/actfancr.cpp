// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Act Fancer (Japan)              FD (c) 1989 Data East Corporation
    Act Fancer (World)              FE (c) 1989 Data East Corporation
    Trio The Punch (World)          FG (c) 1989 Data East Corporation
    Trio The Punch (Japan)          FF (c) 1989 Data East Corporation

    Emulation by Bryan McPhail, mish@tendril.co.uk

    Update of the 03/04/2005
    Added Trio The Punch (World), this set will be the new parent.
    Fixed filenames in both Trio The Punch sets to match correct labels on the PCB.

    Update by Roberto Gandola, sophitia@email.it

*******************************************************************************/

#include "emu.h"

#include "decbac06.h"
#include "decmxc06.h"

#include "cpu/h6280/h6280.h"
#include "cpu/m6502/m6502.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class actfancr_state : public driver_device
{
public:
	actfancr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tilegen(*this, "tilegen%u", 1U),
		m_spritegen(*this, "spritegen"),
		m_spriteram(*this, "spriteram"),
		m_spriteram16(*this, "spriteram16", 0x800, ENDIANNESS_BIG) { }


	void actfancr(machine_config &config);

protected:
	// devices
	required_device<h6280_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<deco_bac06_device, 2> m_tilegen;
	required_device<deco_mxc06_device> m_spritegen;

	void buffer_spriteram_w(uint8_t data);

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_spriteram;
	memory_share_creator<uint16_t> m_spriteram16; // a 16-bit copy of spriteram for use with the MXC06 code

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
	void dec0_s_map(address_map &map) ATTR_COLD;
};


class triothep_state : public actfancr_state
{
public:
	triothep_state(const machine_config &mconfig, device_type type, const char *tag) :
		actfancr_state(mconfig, type, tag),
		m_p(*this, "P%u", 1U),
		m_dsw(*this, "DSW%u", 1U),
		m_system(*this, "SYSTEM") { }


	void triothep(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// misc
	required_ioport_array<2> m_p;
	required_ioport_array<2> m_dsw;
	required_ioport m_system;
	uint8_t m_control_select;

	void control_select_w(uint8_t data);
	uint8_t control_r();

	void prg_map(address_map &map) ATTR_COLD;
};


uint32_t actfancr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Draw playfield
	bool flip = m_tilegen[1]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	m_tilegen[0]->deco_bac06_pf_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16.target(), 0x800/2);
	m_tilegen[1]->deco_bac06_pf_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/******************************************************************************/

void triothep_state::control_select_w(uint8_t data)
{
	m_control_select = data;
}

uint8_t triothep_state::control_r()
{
	switch (m_control_select)
	{
		case 0: return m_p[0]->read();
		case 1: return m_p[1]->read();
		case 2: return m_dsw[0]->read();
		case 3: return m_dsw[1]->read();
		case 4: return m_system->read();    // VBL
	}

	return 0xff;
}

/******************************************************************************/

void actfancr_state::buffer_spriteram_w(uint8_t data)
{
	// copy to a 16-bit region for our sprite draw code too
	for (int i = 0; i < 0x800 / 2; i++)
	{
		m_spriteram16[i] = m_spriteram[i * 2] | (m_spriteram[(i * 2) + 1] << 8);
	}
}

void actfancr_state::prg_map(address_map &map)
{
	map(0x000000, 0x02ffff).rom();
	map(0x060000, 0x060007).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x060010, 0x06001f).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control1_8bit_swap_w));
	map(0x062000, 0x063fff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_8bit_swap_r), FUNC(deco_bac06_device::pf_data_8bit_swap_w));
	map(0x070000, 0x070007).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x070010, 0x07001f).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control1_8bit_swap_w));
	map(0x072000, 0x0727ff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_data_8bit_swap_r), FUNC(deco_bac06_device::pf_data_8bit_swap_w));
	map(0x100000, 0x1007ff).ram().share(m_spriteram);
	map(0x110000, 0x110001).w(FUNC(actfancr_state::buffer_spriteram_w));
	map(0x120000, 0x1205ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x130000, 0x130000).portr("P1");
	map(0x130001, 0x130001).portr("P2");
	map(0x130002, 0x130002).portr("DSW1");
	map(0x130003, 0x130003).portr("DSW2");
	map(0x140000, 0x140001).portr("SYSTEM"); // VBL
	map(0x150000, 0x150000).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x1f0000, 0x1f3fff).ram(); // Main RAM
}

void triothep_state::prg_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x040007).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x040010, 0x04001f).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control1_8bit_swap_w));
	map(0x044000, 0x045fff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_data_8bit_swap_r), FUNC(deco_bac06_device::pf_data_8bit_swap_w));
	map(0x046400, 0x0467ff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_rowscroll_8bit_swap_r), FUNC(deco_bac06_device::pf_rowscroll_8bit_swap_w));
	map(0x060000, 0x060007).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x060010, 0x06001f).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control1_8bit_swap_w));
	map(0x064000, 0x0647ff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_8bit_swap_r), FUNC(deco_bac06_device::pf_data_8bit_swap_w));
	map(0x066400, 0x0667ff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_rowscroll_8bit_swap_r), FUNC(deco_bac06_device::pf_rowscroll_8bit_swap_w));
	map(0x100000, 0x100000).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x110000, 0x110001).w(FUNC(triothep_state::buffer_spriteram_w));
	map(0x120000, 0x1207ff).ram().share("spriteram");
	map(0x130000, 0x1305ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x140000, 0x140001).nopr(); // Value doesn't matter
	map(0x1f0000, 0x1f3fff).ram(); // Main RAM
}

/******************************************************************************/

void actfancr_state::dec0_s_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0801).w("ym1", FUNC(ym2203_device::write));
	map(0x1000, 0x1001).w("ym2", FUNC(ym3812_device::write));
	map(0x3000, 0x3000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x3800, 0x3800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x4000, 0xffff).rom();
}

/******************************************************************************/

static INPUT_PORTS_START( actfancr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )        // Listed as "Unused"
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "100 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        // Listed as "Unused"
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "800000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        // Listed as "Unused"
INPUT_PORTS_END

static INPUT_PORTS_START( triothep )
	PORT_INCLUDE( actfancr )

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x03, "12" )
	PORT_DIPSETTING(    0x02, "14" )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty (Time)" )     PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "Easy (130)" )
	PORT_DIPSETTING(    0x0c, "Normal (100)" )
	PORT_DIPSETTING(    0x04, "Hard (70)" )
	PORT_DIPSETTING(    0x00, "Hardest (60)" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Lives" )           PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        // Listed as "Unused"
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout layout_8x8x4 =
{
	8,8,    // 8*8 chars
	RGN_FRAC(1,4),
	4,      // 4 bits per pixel
	{ RGN_FRAC(1,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8 // every char takes 8 consecutive bytes
};

static const gfx_layout layout_16x16x4 =
{
	16,16,  // 16*16 sprites
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },  // plane offset
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	32*8    // every sprite takes 32 consecutive bytes
};

static GFXDECODE_START( gfx_actfan )
	GFXDECODE_ENTRY( "chars",   0, layout_8x8x4,     0, 16 )
	GFXDECODE_ENTRY( "tiles",   0, layout_16x16x4, 256, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_actfan_spr )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4, 512, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_triothep )
	GFXDECODE_ENTRY( "chars",   0, layout_8x8x4,     0, 16 )
	GFXDECODE_ENTRY( "tiles",   0, layout_16x16x4, 512, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_triothep_spr )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4, 256, 16 )
GFXDECODE_END

/******************************************************************************/

void triothep_state::machine_start()
{
	save_item(NAME(m_control_select));
}

void triothep_state::machine_reset()
{
	m_control_select = 0;
}

/******************************************************************************/

void actfancr_state::actfancr(machine_config &config)
{
	// basic machine hardware
	H6280(config, m_maincpu, 21477200/3); // Should be accurate
	m_maincpu->set_addrmap(AS_PROGRAM, &actfancr_state::prg_map);
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	M6502(config, m_audiocpu, 1500000); // Should be accurate
	m_audiocpu->set_addrmap(AS_PROGRAM, &actfancr_state::dec0_s_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(actfancr_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE); // VBL
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_actfan);
	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 768);

	DECO_BAC06(config, m_tilegen[0], 0);
	m_tilegen[0]->set_gfx_region_wide(1, 1, 2);
	m_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO_BAC06(config, m_tilegen[1], 0);
	m_tilegen[1]->set_gfx_region_wide(0, 0, 0);
	m_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_MXC06(config, m_spritegen, 0, "palette", gfx_actfan_spr);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 1500000));
	ym1.add_route(0, "mono", 0.90);
	ym1.add_route(1, "mono", 0.90);
	ym1.add_route(2, "mono", 0.90);
	ym1.add_route(3, "mono", 0.50);

	ym3812_device &ym2(YM3812(config, "ym2", 3000000));
	ym2.irq_handler().set_inputline("audiocpu", M6502_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.90);

	okim6295_device &oki(OKIM6295(config, "oki", 1024188, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 0.85);
}

void triothep_state::triothep(machine_config &config)
{
	actfancr(config);

	// basic machine hardware
	m_maincpu->set_clock(XTAL(21'477'272) / 3); /* XIN = 21.4772Mhz, verified on PCB */
	m_maincpu->set_addrmap(AS_PROGRAM, &triothep_state::prg_map);
	m_maincpu->port_in_cb().set(FUNC(triothep_state::control_r));
	m_maincpu->port_out_cb().set(FUNC(triothep_state::control_select_w));

	m_audiocpu->set_clock(XTAL(12'000'000) / 8); // verified on PCB

	// video hardware
	m_gfxdecode->set_info(gfx_triothep);
	m_spritegen->set_info(gfx_triothep_spr);

	m_tilegen[0]->set_gfx_region_wide(1, 1, 0);

	// sound hardware
	subdevice<ym2203_device>("ym1")->set_clock(XTAL(12'000'000) / 8); // verified on PCB

	subdevice<ym3812_device>("ym2")->set_clock(XTAL(12'000'000) / 4); // verified on PCB

	subdevice<okim6295_device>("oki")->set_clock(XTAL(1'056'000)); // verified on PCB
}

/******************************************************************************/

ROM_START( actfancr )
	ROM_REGION( 0x30000, "maincpu", 0 ) // Need to allow full RAM allocation for now
	ROM_LOAD( "fe08-3.bin", 0x00000, 0x10000, CRC(35f1999d) SHA1(03b61b6544a21350dbb7a31591db163a00cf7a64) )
	ROM_LOAD( "fe09-3.bin", 0x10000, 0x10000, CRC(d21416ca) SHA1(2c863042fe7cf5e4d8cf8a1138089b539406bec3) )
	ROM_LOAD( "fe10-3.bin", 0x20000, 0x10000, CRC(85535fcc) SHA1(c764032463a40c9110ab35c38642de070d528a60) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502 Sound CPU
	ROM_LOAD( "17-1", 0x08000, 0x8000, CRC(289ad106) SHA1(cf1b32ac41d3d92860fab04d82a08efe57b6ecf3) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "15", 0x00000, 0x10000, CRC(a1baf21e) SHA1(b85cf9180efae6c95cc0310064b52a78e591826a) )
	ROM_LOAD( "16", 0x10000, 0x10000, CRC(22e64730) SHA1(f1376c6e2c9d021eca7ccee3daab00593ba724b6) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "02", 0x00000, 0x10000, CRC(b1db0efc) SHA1(a7bd7748ea37f473499ba5bf8ab4995b9240ff48) )
	ROM_LOAD( "03", 0x10000, 0x08000, CRC(f313e04f) SHA1(fe69758910d38f742971c1027fc8f498c88262b1) )
	ROM_LOAD( "06", 0x18000, 0x10000, CRC(8cb6dd87) SHA1(fab4fe76d2426c906a9070cbf7ce81200ba27ff6) )
	ROM_LOAD( "07", 0x28000, 0x08000, CRC(dd345def) SHA1(44fbf9da636a4e18c421fdc0a1eadc3c7ba66068) )
	ROM_LOAD( "00", 0x30000, 0x10000, CRC(d50a9550) SHA1(b366826e0df11ab6b97e2cb0e813432e95f9513d) )
	ROM_LOAD( "01", 0x40000, 0x08000, CRC(34935e93) SHA1(8cd02a72659f6cb0536b54c1c8b34dae818fbfdc) )
	ROM_LOAD( "04", 0x48000, 0x10000, CRC(bcf41795) SHA1(1d18afc974ac43fe6194e2840bbb2e93cd2b6cff) )
	ROM_LOAD( "05", 0x58000, 0x08000, CRC(d38b94aa) SHA1(773d01427744fda9104f673d2b4183a0f7471a39) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "14", 0x00000, 0x10000, CRC(d6457420) SHA1(d03d2e944e768b297ec0c3389320c42bc0259d00) )
	ROM_LOAD( "12", 0x10000, 0x10000, CRC(08787b7a) SHA1(23b10b75c4cbff8effadf4c6ed15d90b87648ce9) )
	ROM_LOAD( "13", 0x20000, 0x10000, CRC(c30c37dc) SHA1(0f7a325738eafa85239497e2b97aa51a6f2ffc4d) )
	ROM_LOAD( "11", 0x30000, 0x10000, CRC(1f006d9f) SHA1(74bc2d4d022ad7c65be781f974919262cacb4b64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "18",   0x00000, 0x10000, CRC(5c55b242) SHA1(62ba60b2f02483875da12aefe849f7e2fd137ef1) )
ROM_END

ROM_START( actfancr2 )
	ROM_REGION( 0x30000, "maincpu", 0 ) // Need to allow full RAM allocation for now
	ROM_LOAD( "fe08-2.bin", 0x00000, 0x10000, CRC(0d36fbfa) SHA1(cef5cfd053beac5ca2ac52421024c316bdbfba42) )
	ROM_LOAD( "fe09-2.bin", 0x10000, 0x10000, CRC(27ce2bb1) SHA1(52a423dfc2bba7b3330d1a10f4149ae6eeb9198c) )
	ROM_LOAD( "10",         0x20000, 0x10000, CRC(cabad137) SHA1(41ca833649671a29e9395968cde2be8137a9ff0a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502 Sound CPU
	ROM_LOAD( "17-1", 0x08000, 0x8000, CRC(289ad106) SHA1(cf1b32ac41d3d92860fab04d82a08efe57b6ecf3) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "15", 0x00000, 0x10000, CRC(a1baf21e) SHA1(b85cf9180efae6c95cc0310064b52a78e591826a) )
	ROM_LOAD( "16", 0x10000, 0x10000, CRC(22e64730) SHA1(f1376c6e2c9d021eca7ccee3daab00593ba724b6) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "02", 0x00000, 0x10000, CRC(b1db0efc) SHA1(a7bd7748ea37f473499ba5bf8ab4995b9240ff48) )
	ROM_LOAD( "03", 0x10000, 0x08000, CRC(f313e04f) SHA1(fe69758910d38f742971c1027fc8f498c88262b1) )
	ROM_LOAD( "06", 0x18000, 0x10000, CRC(8cb6dd87) SHA1(fab4fe76d2426c906a9070cbf7ce81200ba27ff6) )
	ROM_LOAD( "07", 0x28000, 0x08000, CRC(dd345def) SHA1(44fbf9da636a4e18c421fdc0a1eadc3c7ba66068) )
	ROM_LOAD( "00", 0x30000, 0x10000, CRC(d50a9550) SHA1(b366826e0df11ab6b97e2cb0e813432e95f9513d) )
	ROM_LOAD( "01", 0x40000, 0x08000, CRC(34935e93) SHA1(8cd02a72659f6cb0536b54c1c8b34dae818fbfdc) )
	ROM_LOAD( "04", 0x48000, 0x10000, CRC(bcf41795) SHA1(1d18afc974ac43fe6194e2840bbb2e93cd2b6cff) )
	ROM_LOAD( "05", 0x58000, 0x08000, CRC(d38b94aa) SHA1(773d01427744fda9104f673d2b4183a0f7471a39) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "14", 0x00000, 0x10000, CRC(d6457420) SHA1(d03d2e944e768b297ec0c3389320c42bc0259d00) )
	ROM_LOAD( "12", 0x10000, 0x10000, CRC(08787b7a) SHA1(23b10b75c4cbff8effadf4c6ed15d90b87648ce9) )
	ROM_LOAD( "13", 0x20000, 0x10000, CRC(c30c37dc) SHA1(0f7a325738eafa85239497e2b97aa51a6f2ffc4d) )
	ROM_LOAD( "11", 0x30000, 0x10000, CRC(1f006d9f) SHA1(74bc2d4d022ad7c65be781f974919262cacb4b64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "18",   0x00000, 0x10000, CRC(5c55b242) SHA1(62ba60b2f02483875da12aefe849f7e2fd137ef1) )
ROM_END

ROM_START( actfancr1 )
	ROM_REGION( 0x30000, "maincpu", 0 ) // Need to allow full RAM allocation for now
	ROM_LOAD( "08-1", 0x00000, 0x10000, CRC(3bf214a4) SHA1(f7513672b2292d3acb4332b392695888bf6560a5) )
	ROM_LOAD( "09-1", 0x10000, 0x10000, CRC(13ae78d5) SHA1(eba77d3dbfe273e18c7fa9c0ca305ac2468f9381) )
	ROM_LOAD( "10",   0x20000, 0x10000, CRC(cabad137) SHA1(41ca833649671a29e9395968cde2be8137a9ff0a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502 Sound CPU
	ROM_LOAD( "17-1", 0x08000, 0x8000, CRC(289ad106) SHA1(cf1b32ac41d3d92860fab04d82a08efe57b6ecf3) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "15", 0x00000, 0x10000, CRC(a1baf21e) SHA1(b85cf9180efae6c95cc0310064b52a78e591826a) )
	ROM_LOAD( "16", 0x10000, 0x10000, CRC(22e64730) SHA1(f1376c6e2c9d021eca7ccee3daab00593ba724b6) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "02", 0x00000, 0x10000, CRC(b1db0efc) SHA1(a7bd7748ea37f473499ba5bf8ab4995b9240ff48) )
	ROM_LOAD( "03", 0x10000, 0x08000, CRC(f313e04f) SHA1(fe69758910d38f742971c1027fc8f498c88262b1) )
	ROM_LOAD( "06", 0x18000, 0x10000, CRC(8cb6dd87) SHA1(fab4fe76d2426c906a9070cbf7ce81200ba27ff6) )
	ROM_LOAD( "07", 0x28000, 0x08000, CRC(dd345def) SHA1(44fbf9da636a4e18c421fdc0a1eadc3c7ba66068) )
	ROM_LOAD( "00", 0x30000, 0x10000, CRC(d50a9550) SHA1(b366826e0df11ab6b97e2cb0e813432e95f9513d) )
	ROM_LOAD( "01", 0x40000, 0x08000, CRC(34935e93) SHA1(8cd02a72659f6cb0536b54c1c8b34dae818fbfdc) )
	ROM_LOAD( "04", 0x48000, 0x10000, CRC(bcf41795) SHA1(1d18afc974ac43fe6194e2840bbb2e93cd2b6cff) )
	ROM_LOAD( "05", 0x58000, 0x08000, CRC(d38b94aa) SHA1(773d01427744fda9104f673d2b4183a0f7471a39) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "14", 0x00000, 0x10000, CRC(d6457420) SHA1(d03d2e944e768b297ec0c3389320c42bc0259d00) )
	ROM_LOAD( "12", 0x10000, 0x10000, CRC(08787b7a) SHA1(23b10b75c4cbff8effadf4c6ed15d90b87648ce9) )
	ROM_LOAD( "13", 0x20000, 0x10000, CRC(c30c37dc) SHA1(0f7a325738eafa85239497e2b97aa51a6f2ffc4d) )
	ROM_LOAD( "11", 0x30000, 0x10000, CRC(1f006d9f) SHA1(74bc2d4d022ad7c65be781f974919262cacb4b64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "18",   0x00000, 0x10000, CRC(5c55b242) SHA1(62ba60b2f02483875da12aefe849f7e2fd137ef1) )
ROM_END

ROM_START( actfancrj )
	ROM_REGION( 0x30000, "maincpu", 0 ) // Need to allow full RAM allocation for now
	ROM_LOAD( "fd08-1.bin", 0x00000, 0x10000, CRC(69004b60) SHA1(7c6b876ca04377d2aa2d3c3f19d8e6cc7345363d) )
	ROM_LOAD( "fd09-1.bin", 0x10000, 0x10000, CRC(a455ae3e) SHA1(960798271c8370c1c4ffce2a453f59d7a301c9f9) )
	ROM_LOAD( "10",         0x20000, 0x10000, CRC(cabad137) SHA1(41ca833649671a29e9395968cde2be8137a9ff0a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502 Sound CPU
	ROM_LOAD( "17-1", 0x08000, 0x8000, CRC(289ad106) SHA1(cf1b32ac41d3d92860fab04d82a08efe57b6ecf3) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "15", 0x00000, 0x10000, CRC(a1baf21e) SHA1(b85cf9180efae6c95cc0310064b52a78e591826a) )
	ROM_LOAD( "16", 0x10000, 0x10000, CRC(22e64730) SHA1(f1376c6e2c9d021eca7ccee3daab00593ba724b6) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "02", 0x00000, 0x10000, CRC(b1db0efc) SHA1(a7bd7748ea37f473499ba5bf8ab4995b9240ff48) )
	ROM_LOAD( "03", 0x10000, 0x08000, CRC(f313e04f) SHA1(fe69758910d38f742971c1027fc8f498c88262b1) )
	ROM_LOAD( "06", 0x18000, 0x10000, CRC(8cb6dd87) SHA1(fab4fe76d2426c906a9070cbf7ce81200ba27ff6) )
	ROM_LOAD( "07", 0x28000, 0x08000, CRC(dd345def) SHA1(44fbf9da636a4e18c421fdc0a1eadc3c7ba66068) )
	ROM_LOAD( "00", 0x30000, 0x10000, CRC(d50a9550) SHA1(b366826e0df11ab6b97e2cb0e813432e95f9513d) )
	ROM_LOAD( "01", 0x40000, 0x08000, CRC(34935e93) SHA1(8cd02a72659f6cb0536b54c1c8b34dae818fbfdc) )
	ROM_LOAD( "04", 0x48000, 0x10000, CRC(bcf41795) SHA1(1d18afc974ac43fe6194e2840bbb2e93cd2b6cff) )
	ROM_LOAD( "05", 0x58000, 0x08000, CRC(d38b94aa) SHA1(773d01427744fda9104f673d2b4183a0f7471a39) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "14", 0x00000, 0x10000, CRC(d6457420) SHA1(d03d2e944e768b297ec0c3389320c42bc0259d00) )
	ROM_LOAD( "12", 0x10000, 0x10000, CRC(08787b7a) SHA1(23b10b75c4cbff8effadf4c6ed15d90b87648ce9) )
	ROM_LOAD( "13", 0x20000, 0x10000, CRC(c30c37dc) SHA1(0f7a325738eafa85239497e2b97aa51a6f2ffc4d) )
	ROM_LOAD( "11", 0x30000, 0x10000, CRC(1f006d9f) SHA1(74bc2d4d022ad7c65be781f974919262cacb4b64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "18",   0x00000, 0x10000, CRC(5c55b242) SHA1(62ba60b2f02483875da12aefe849f7e2fd137ef1) )
ROM_END

ROM_START( triothep )
	ROM_REGION( 0x40000, "maincpu", 0 ) // Need to allow full RAM allocation for now
	ROM_LOAD( "fg-16.bin", 0x00000, 0x20000, CRC(7238355a) SHA1(4ac6c3fd808e7c94025972fdb45956bd707ec89f) )
	ROM_LOAD( "fg-15.bin", 0x20000, 0x10000, CRC(1c0551ab) SHA1(1f90f80db44d92af4b233bc16cb1023db2797e8a) )
	ROM_LOAD( "fg-14.bin", 0x30000, 0x10000, CRC(4ba7de4a) SHA1(bf552fa33746f3d27f9b193424a38fef58fe0765) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502 Sound CPU
	ROM_LOAD( "fg-18.bin", 0x00000, 0x10000, CRC(9de9ee63) SHA1(c91b824b9a791cb90365d45c8e1b69e67f7d065f) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "fg-12.bin", 0x00000, 0x10000, CRC(15fb49f2) SHA1(a81ff1dbc813ab9b37edb832e01aab9a9a3ed5a1) )
	ROM_LOAD( "fg-13.bin", 0x10000, 0x10000, CRC(e20c9623) SHA1(b5a58599a016378f34217396212f81ede9272598) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "fg-11.bin", 0x00000, 0x10000, CRC(1143ebd7) SHA1(0ef2cf40f852bf0842beeb9727508e28437ab54b) )
	ROM_LOAD( "fg-10.bin", 0x10000, 0x08000, CRC(4b6b477a) SHA1(77486e0ff957cbfdae16d2b5977e95b7a7ced948) )
	ROM_LOAD( "fg-09.bin", 0x18000, 0x10000, CRC(6bf6c803) SHA1(c16fd4b7e1e86db48c6e78a4b5dcd42e8269b465) )
	ROM_LOAD( "fg-08.bin", 0x28000, 0x08000, CRC(1391e445) SHA1(bd53a969567bb5a46a35bd02e84bbb58c446a0a2) )
	ROM_LOAD( "fg-03.bin", 0x30000, 0x10000, CRC(3d3ca9ad) SHA1(de6532063500a4ddccdecfca1024f03a1fbb78f7) )
	ROM_LOAD( "fg-02.bin", 0x40000, 0x08000, CRC(6b9d24ce) SHA1(9d6d52e742fc37d83682291f918f3348395f0cd8) )
	ROM_LOAD( "fg-01.bin", 0x48000, 0x10000, CRC(4987f7ac) SHA1(e8e81b15f6b6c8597d34eef3cabb89b90d3ae7f5) )
	ROM_LOAD( "fg-00.bin", 0x58000, 0x08000, CRC(41232442) SHA1(1c10a4f5607e41d6239cb478ed7355963ad6b2d0) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "fg-04.bin", 0x00000, 0x10000, CRC(7cea3c87) SHA1(b58156140a75f88ee6ec97ca7cdc02619ec51726) )
	ROM_LOAD( "fg-06.bin", 0x10000, 0x10000, CRC(5e7f3e8f) SHA1(c92ec281b3985b442957f7d9237eb38a6d621cd4) )
	ROM_LOAD( "fg-05.bin", 0x20000, 0x10000, CRC(8bb13f05) SHA1(f524cb0a38d0025c93124fc329d913e000155e9b) )
	ROM_LOAD( "fg-07.bin", 0x30000, 0x10000, CRC(0d7affc3) SHA1(59f9fbf13216aaf67c7d1ad3a11a1738c4afd9e5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM sounds */
	ROM_LOAD( "fg-17.bin", 0x00000, 0x10000, CRC(f0ab0d05) SHA1(29d3ab513a8d46a1cb70f5333fa56bb787a58288) )
ROM_END

// All ROMSs are FF even the ones matching the parent FG ROMs
ROM_START( triothepj )
	ROM_REGION( 0x40000, "maincpu", 0 ) // Need to allow full RAM allocation for now
	ROM_LOAD( "ff-16.bin", 0x00000, 0x20000, CRC(84d7e1b6) SHA1(28381d2e1f6d22a959383eb2e8d73f2e03f4d39f) )
	ROM_LOAD( "ff-15.bin", 0x20000, 0x10000, CRC(6eada47c) SHA1(98fc4e93c47bc42ea7c20e8ac994b117cd7cb5a5) )
	ROM_LOAD( "ff-14.bin", 0x30000, 0x10000, CRC(4ba7de4a) SHA1(bf552fa33746f3d27f9b193424a38fef58fe0765) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502 Sound CPU
	ROM_LOAD( "ff-18.bin", 0x00000, 0x10000, CRC(9de9ee63) SHA1(c91b824b9a791cb90365d45c8e1b69e67f7d065f) )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "ff-12.bin", 0x00000, 0x10000, CRC(15fb49f2) SHA1(a81ff1dbc813ab9b37edb832e01aab9a9a3ed5a1) )
	ROM_LOAD( "ff-13.bin", 0x10000, 0x10000, CRC(e20c9623) SHA1(b5a58599a016378f34217396212f81ede9272598) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "ff-11.bin", 0x00000, 0x10000, CRC(19e885c7) SHA1(694f0aa4c1c976320d985ee50bb59c1894b853ed) )
	ROM_LOAD( "ff-10.bin", 0x10000, 0x08000, CRC(4b6b477a) SHA1(77486e0ff957cbfdae16d2b5977e95b7a7ced948) )
	ROM_LOAD( "ff-09.bin", 0x18000, 0x10000, CRC(79c6bc0e) SHA1(d4bf195f6114103d2eb68f3aaf65d4044947f600) )
	ROM_LOAD( "ff-08.bin", 0x28000, 0x08000, CRC(1391e445) SHA1(bd53a969567bb5a46a35bd02e84bbb58c446a0a2) )
	ROM_LOAD( "ff-03.bin", 0x30000, 0x10000, CRC(b36ad42d) SHA1(9d72cbb0904e82271e4835d668b133f17dec8255) )
	ROM_LOAD( "ff-02.bin", 0x40000, 0x08000, CRC(6b9d24ce) SHA1(9d6d52e742fc37d83682291f918f3348395f0cd8) )
	ROM_LOAD( "ff-01.bin", 0x48000, 0x10000, CRC(68d80a66) SHA1(526ed8c920915877f5ee0519c9c8eee7e5580c54) )
	ROM_LOAD( "ff-00.bin", 0x58000, 0x08000, CRC(41232442) SHA1(1c10a4f5607e41d6239cb478ed7355963ad6b2d0) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "ff-04.bin", 0x00000, 0x10000, CRC(7cea3c87) SHA1(b58156140a75f88ee6ec97ca7cdc02619ec51726) )
	ROM_LOAD( "ff-06.bin", 0x10000, 0x10000, CRC(5e7f3e8f) SHA1(c92ec281b3985b442957f7d9237eb38a6d621cd4) )
	ROM_LOAD( "ff-05.bin", 0x20000, 0x10000, CRC(8bb13f05) SHA1(f524cb0a38d0025c93124fc329d913e000155e9b) )
	ROM_LOAD( "ff-07.bin", 0x30000, 0x10000, CRC(0d7affc3) SHA1(59f9fbf13216aaf67c7d1ad3a11a1738c4afd9e5) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "ff-17.bin", 0x00000, 0x10000, CRC(f0ab0d05) SHA1(29d3ab513a8d46a1cb70f5333fa56bb787a58288) )
ROM_END

} // Anonymous namespace


/******************************************************************************/

GAME( 1989, actfancr,  0,        actfancr, actfancr, actfancr_state, empty_init, ROT0, "Data East Corporation", "Act-Fancer Cybernetick Hyper Weapon (World revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, actfancr2, actfancr, actfancr, actfancr, actfancr_state, empty_init, ROT0, "Data East Corporation", "Act-Fancer Cybernetick Hyper Weapon (World revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, actfancr1, actfancr, actfancr, actfancr, actfancr_state, empty_init, ROT0, "Data East Corporation", "Act-Fancer Cybernetick Hyper Weapon (World revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, actfancrj, actfancr, actfancr, actfancr, actfancr_state, empty_init, ROT0, "Data East Corporation", "Act-Fancer Cybernetick Hyper Weapon (Japan revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, triothep,  0,        triothep, triothep, triothep_state, empty_init, ROT0, "Data East Corporation", "Trio The Punch - Never Forget Me... (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, triothepj, triothep, triothep, triothep, triothep_state, empty_init, ROT0, "Data East Corporation", "Trio The Punch - Never Forget Me... (Japan)", MACHINE_SUPPORTS_SAVE )
