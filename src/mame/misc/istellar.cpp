// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/*
Funai / Gakken Inter Stellar Laser Fantasy laserdisc hardware
Driver by Andrew Gardner with help from Daphne Source

Notes:
    Holding down the TEST switch while hitting reset will bring up the Self Test.
    Hit TEST switch again for color and monitor calibration.
    This is somewhat strange hardware : More z80's than necessary
                                        3 bpp sprites
                                        6-pin dip switches with odd handling
                                        inverted DIP logic?
                                        CPU2 maps RAM over where its ROM lives
    To skip LD boot: bp 4458,1,{pc=0x447a;g}
    Then fill $a074 with 1 to go in attract mode

Todo:
    How does one best make one DIP switch bit from address 0x02 tie to two bits from address 0x03?
    Get real ROM labels!  The current labels are unfortunately a bit odd.
    Make it work - this one should be close right now :/.
    istellar2 fails test for all the main CPU ROMs. Maybe because it's a prototype?
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/ldv1000.h"

#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"

// configurable logging
#define LOG_CPU2     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CPU2)

#include "logmacro.h"

#define LOGCPU2(...)     LOGMASKED(LOG_CPU2,     __VA_ARGS__)


namespace {

class istellar_state : public driver_device
{
public:
	istellar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_laserdisc(*this, "laserdisc") ,
		m_tile_ram(*this, "tile_ram"),
		m_tile_control_ram(*this, "tile_ctrl_ram"),
		m_sprite_ram(*this, "sprite_ram") { }

	void init_istellar();
	void istellar(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<pioneer_ldv1000_device> m_laserdisc;

	required_shared_ptr<uint8_t> m_tile_ram;
	required_shared_ptr<uint8_t> m_tile_control_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;

	tilemap_t *m_fg_tilemap = nullptr;
	TILE_GET_INFO_MEMBER(get_tile_info);

	void tile_w(offs_t offset, uint8_t data);
	void attr_w(offs_t offset, uint8_t data);
	void overlay_control_w(uint8_t data);

	u8 m_overlay_ctrl = 0;

	uint8_t z80_2_ldp_read();
	uint8_t z80_2_unknown_read();
	void z80_2_ldp_write(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void z80_0_io(address_map &map) ATTR_COLD;
	void z80_0_mem(address_map &map) ATTR_COLD;
	void z80_1_io(address_map &map) ATTR_COLD;
	void z80_1_mem(address_map &map) ATTR_COLD;
	void z80_2_io(address_map &map) ATTR_COLD;
	void z80_2_mem(address_map &map) ATTR_COLD;
};

void istellar_state::tile_w(offs_t offset, uint8_t data)
{
	m_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void istellar_state::attr_w(offs_t offset, uint8_t data)
{
	m_tile_control_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(istellar_state::get_tile_info)
{
	const u8 code = m_tile_ram[tile_index];
	const u8 color = m_tile_control_ram[tile_index] & 0xf;
	tileinfo.set(0, code, color, 0);
}

void istellar_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(istellar_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

uint32_t istellar_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// TODO: should really draw transparent when bit 7 disabled, also gradient to be verified.
	// (May actually be an opaque flag for tilemap + pal bank?)
	bitmap.fill(BIT(m_overlay_ctrl, 7) ? rgb_t(0x00, 0x00, 0xff) : rgb_t(0, 0, 0), cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// sprites, above tilemap according to PCB refs for both games
	// (Daphne is wrong and draws below, unless a bit is set for enemy sprites?)
	for (int i = 0; i < m_sprite_ram.bytes(); i += 4)
	{
		u8 const attr = m_sprite_ram[i + 2];
		// FIXME: any of the unused bits disables drawing
		if (attr == 0xff)
			continue;
		int y = 0xf0 - m_sprite_ram[i + 0];
		int x = m_sprite_ram[i + 3];
		u8 const code = m_sprite_ram[i + 1];
		u8 const color = attr & 0x1f;
		int flipx = attr & 0x40;

		m_gfxdecode->gfx(1)->transpen(
			bitmap, cliprect,
			code, color,
			flipx, 0,
			x, y,
			0
		);
	}

	return 0;
}


void istellar_state::machine_start()
{
}



// MEMORY HANDLERS


// Z80 1 R/W


// Z80 2 R/W
uint8_t istellar_state::z80_2_ldp_read()
{
	uint8_t readResult = m_laserdisc->data_r();
	LOGCPU2("CPU2 : reading LDP : %x\n", readResult);
	return readResult;
}

uint8_t istellar_state::z80_2_unknown_read()
{
	LOGCPU2("CPU2 : c000!\n");
	return 0x00;
}

void istellar_state::z80_2_ldp_write(uint8_t data)
{
	LOGCPU2("CPU2 : writing LDP : 0x%x\n", data);
	m_laserdisc->data_w(data);
}

void istellar_state::overlay_control_w(uint8_t data)
{
	m_overlay_ctrl = data;
	if (data & 0x7f)
		logerror("overlay_control_w: %02x\n", data);
}

// PROGRAM MAPS
void istellar_state::z80_0_mem(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xa7ff).ram();
	map(0xa800, 0xabff).ram().w(FUNC(istellar_state::tile_w)).share(m_tile_ram);
	map(0xac00, 0xafff).ram().w(FUNC(istellar_state::attr_w)).share(m_tile_control_ram);
	map(0xb000, 0xb1ff).ram().share(m_sprite_ram);
}

void istellar_state::z80_1_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x47ff).ram();
}

void istellar_state::z80_2_mem(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x1800, 0x1fff).ram();
	map(0xc000, 0xc000).r(FUNC(istellar_state::z80_2_unknown_read));     // Seems to be thrown away every time it's read - maybe interrupt related?
}


// IO MAPS
void istellar_state::z80_0_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x02, 0x02).portr("DSW1");
	map(0x03, 0x03).portr("DSW2");
	map(0x04, 0x04).w(FUNC(istellar_state::overlay_control_w));
	map(0x05, 0x05).r("latch1", FUNC(generic_latch_8_device::read)).w("latch2", FUNC(generic_latch_8_device::write));
}

void istellar_state::z80_1_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).noprw(); //.rw(FUNC(istellar_state::z80_1_slatch_read), FUNC(istellar_state::z80_1_slatch_write));
	map(0x01, 0x01).noprw(); //.rw(FUNC(istellar_state::z80_1_nmienable), FUNC(istellar_state::z80_1_soundwrite_front));
	map(0x02, 0x02).noprw(); //.w(FUNC(istellar_state::z80_1_soundwrite_rear));
}

void istellar_state::z80_2_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(istellar_state::z80_2_ldp_read), FUNC(istellar_state::z80_2_ldp_write));
	map(0x01, 0x01).r("latch2", FUNC(generic_latch_8_device::read)).w("latch1", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).r("latch2", FUNC(generic_latch_8_device::acknowledge_r));
//  map(0x03, 0x03).w(FUNC(istellar_state::z80_2_ldtrans_write));
}

INPUT_CHANGED_MEMBER(istellar_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

// PORTS
static INPUT_PORTS_START( istellar )
	// TEST MODE might display a 0 for a short and a 1 for an open circuit?  If so, everything below is inverted.
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!2,!3,!4")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPNAME( 0x40, 0x00, "Barrier UFO" ) PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!2")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* NOTE - bit 0x80 in the above read is combined with bits 0x03 in the below read to form the Coin_B
	          settings.  I'm unaware of what mechanism MAME will use to make this work right? */

	// "In case of inter-stellar upright type the coin switch 2 is not used."  Quoth the manual.
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!5")        // Maybe SERVICE?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, istellar_state, coin_inserted, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, istellar_state, coin_inserted, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	// SERVICE might be hanging out back here
INPUT_PORTS_END

static const gfx_layout layout_8x8 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	8*8
};

static const gfx_layout layout_16x16 =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0, 1), STEP8(8*8, 1) },
	{ STEP8(0, 8), STEP8(8*8*2, 8) },
	16*16
};

static GFXDECODE_START( gfx_istellar )
	GFXDECODE_ENTRY( "tiles", 0, layout_8x8, 0x0, 0x20 )
	GFXDECODE_ENTRY( "tiles", 0, layout_16x16, 0x0, 0x20 )
GFXDECODE_END

void istellar_state::vblank_irq(int state)
{
	if (state)
	{
		// Interrupt presumably comes from VBlank
		m_maincpu->set_input_line(0, HOLD_LINE);

		// Interrupt presumably comes from the LDP's status strobe
		m_subcpu->set_input_line(0, ASSERT_LINE);
	}
}


// DRIVER
void istellar_state::istellar(machine_config &config)
{
	// There is only 1 crystal on the stack of 3 boards - speed is unknown, the following is Daphne's guess
	constexpr int GUESSED_CLOCK = 3'072'000;

	Z80(config, m_maincpu, GUESSED_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &istellar_state::z80_0_mem);
	m_maincpu->set_addrmap(AS_IO, &istellar_state::z80_0_io);

	z80_device &audiocpu(Z80(config, "audiocpu", GUESSED_CLOCK));
	audiocpu.set_addrmap(AS_PROGRAM, &istellar_state::z80_1_mem);
	audiocpu.set_addrmap(AS_IO, &istellar_state::z80_1_io);

	// ldp comm cpu
	Z80(config, m_subcpu, GUESSED_CLOCK);
	m_subcpu->set_addrmap(AS_PROGRAM, &istellar_state::z80_2_mem);
	m_subcpu->set_addrmap(AS_IO, &istellar_state::z80_2_io);

	GENERIC_LATCH_8(config, "latch1");

	generic_latch_8_device &latch2(GENERIC_LATCH_8(config, "latch2"));
	latch2.data_pending_callback().set_inputline(m_subcpu, INPUT_LINE_NMI);
	latch2.set_separate_acknowledge(true);

	PIONEER_LDV1000(config, m_laserdisc, 0);
	m_laserdisc->set_overlay(256, 256, FUNC(istellar_state::screen_update));
	m_laserdisc->add_route(0, "lspeaker", 1.0);
	m_laserdisc->add_route(1, "rspeaker", 1.0);

	// video hardware
	m_laserdisc->add_ntsc_screen(config, "screen");
	subdevice<screen_device>("screen")->screen_vblank().set(FUNC(istellar_state::vblank_irq));

	// Daphne says "TODO: get the real interstellar resistor values"
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_istellar);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}


// There is a photo of the PCB with blurry IC locations and labels.  Comments reflect what I can (barely) see.
ROM_START( istellar )
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "rom2.top", 0x0000, 0x2000, CRC(5d643381) SHA1(75ca52c28a52f534eda00c18b0db97e9923ff670) )    // At IC location C63 (top board) - label ?
	ROM_LOAD( "rom3.top", 0x2000, 0x2000, CRC(ce5a2b09) SHA1(2de6a6e993c3411577ac0c834db8aaf16fb007ed) )    // At IC location C64 (top board) - label ?
	ROM_LOAD( "rom4.top", 0x4000, 0x2000, CRC(7c2cb1f1) SHA1(ffd92510c03c2d35a59d233883c2b9f57394a51c) )    // At IC location C65 (top board) - label ?
	ROM_LOAD( "rom5.top", 0x6000, 0x2000, CRC(354377f6) SHA1(bcf95b7ee1b47854e10baf24b0d8af3d56738b99) )    // At IC location C66 (top board) - label ?
	ROM_LOAD( "rom6.top", 0x8000, 0x2000, CRC(0319bf40) SHA1(f324626e457c3eb7d6b74bc6afbfcc3aab2b3c72) )    // At IC location C67 (top board) - label ?

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "rom1.top", 0x0000, 0x2000, CRC(4f34fb1d) SHA1(56ca19344c84c5989d0be797e2759f84760310be) )    // At IC location C62 (top board) - label ?

	// LDP Communications CPU
	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "rom11.bot", 0x0000, 0x2000, CRC(165cbc57) SHA1(39463888f22ec3125f0686066d923a9aae79a8f7) )   // At IC location C12 (bottom board) - label IS11

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "rom9.bot", 0x0000, 0x2000, CRC(9d79acb6) SHA1(72af972695face0016afce8a26c629d963e86d48) )    // At IC location C47? (bottom board) - label ?
	ROM_LOAD( "rom8.bot", 0x2000, 0x2000, CRC(e9c9e490) SHA1(79aa35552b984018bc723adece5c40a0833a313c) )    // At IC location C48? (bottom board) - label ?
	ROM_LOAD( "rom7.bot", 0x4000, 0x2000, CRC(1447ce3a) SHA1(8545cec108df6adab303802b1407c89b2dceba21) )    // At IC location C49? (bottom board) - label ?

	// Color PROMs
	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "red6b.bot",   0x000, 0x100, CRC(5c52f844) SHA1(a8a3d91f3247ad13c805d8d8288b07f3cdaf1189) )   // At IC location C63? (bottom board) - label ?
	ROM_LOAD( "green6c.bot", 0x100, 0x100, CRC(7d8c845c) SHA1(04ae2ca0cc6679e21346ce34e9e01aa5bf4e2067) )   // At IC location C62? (bottom board) - label ?
	ROM_LOAD( "blue6d.bot",  0x200, 0x100, CRC(5ebb81f9) SHA1(285d60f2894c524ca80fc68ad7c2dfd9093a67ea) )   // At IC location C61? (bottom board) - label ?

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "istellar", 0, NO_DUMP )
ROM_END


/*
Interstellar 2 - Zangus

ROM dump taken from untested prototype board.
Accuracy of these ROMs is unknown.

3 boards, Approx 11 x 12.5 Inches

All EPROMs are 2764
Color PROMs are all MB7052

Top board has 2 RCA phono jacks.  The board is marked 1613208.
Board set is exactly the same (visually) as Interstellar LF, except
for 3 new ROM sockets for ROMs 11, 12, & 13.
*/

ROM_START( istellar2 )
	ROM_REGION( 0xa000, "maincpu", 0 ) // on top board
	ROM_LOAD( "rom2.g1", 0x0000, 0x2000, CRC(4dbe81aa) SHA1(cea50ed4d3bff51158d8cbefccb5be3e47103fd0) )
	ROM_LOAD( "rom3.f1", 0x2000, 0x2000, CRC(9ce08198) SHA1(2f815e1fbace0a864dfcd839e96346dd25d55bc9) )
	ROM_LOAD( "rom4.e1", 0x4000, 0x2000, CRC(a294bf7f) SHA1(cf34f7d1b4c2b9a272f777f89d78773903a3701e) ) // 1xxxxxxxxxxxx = 0xFF. Strange, but 2 different dumpers got the same result
	ROM_LOAD( "rom5.d1", 0x6000, 0x2000, CRC(0803a4ba) SHA1(e3e45d28fc95ec91133379f0c3e4937115d711be) )
	ROM_LOAD( "rom6.b1", 0x8000, 0x2000, CRC(16abbf32) SHA1(11662fa32d3aed274a2b87beabf4d5f5d7f24dd6) )

	ROM_REGION( 0x2000, "audiocpu", 0 ) // on top board
	ROM_LOAD( "rom1.j1", 0x0000, 0x2000, CRC(0c25a2ba) SHA1(b4546c35ce6e04750bc055835188c9c32b1e02f3) )

	// LDP Communications CPU
	ROM_REGION( 0x2000, "sub", 0 ) // on bottom board
	ROM_LOAD( "rom10.f1", 0x0000, 0x2000, CRC(ff97c588) SHA1(016769b6f38f40438800bd02c45f1bb5ba385c16) )

	ROM_REGION( 0x6000, "tiles", 0 ) // on bottom board
	ROM_LOAD( "rom9.g5", 0x0000, 0x2000, CRC(9d91f15e) SHA1(4d9b98e4c764367562c64aee356e826537a2666e) )
	ROM_LOAD( "rom8.h5", 0x2000, 0x2000, CRC(addec265) SHA1(041f20ffaedc337cdf4677e00c557d2d77a33147) )
	ROM_LOAD( "rom7.j5", 0x4000, 0x2000, CRC(bef6e10f) SHA1(18d15e858afffd8b5c4747e2b6357f18b845a69b) )

	ROM_REGION( 0x6000, "unsorted_roms", 0 ) // given they're on the bottom board, probably GFX related too
	ROM_LOAD( "rom11.o1", 0x0000, 0x2000, CRC(2396bc70) SHA1(7827f11b6a1d9a70e26e72d28b6eb513d3acacbc) )
	ROM_LOAD( "rom12.o2", 0x2000, 0x2000, CRC(9f2b0fc0) SHA1(d75cc1c2b0cd6083e4a7b949e27277a13b92405f) )
	ROM_LOAD( "rom13.m1", 0x4000, 0x2000, CRC(2db6188b) SHA1(72a2e421859bd039649c0af097836ef4468fa635) )

	// Color PROMs
	ROM_REGION( 0x300, "proms", 0 ) // on bottom board
	ROM_LOAD( "red.b6",   0x000, 0x100, CRC(560fbcc1) SHA1(9165167478372c796ba33a4d6ed525d941117ad3) )
	ROM_LOAD( "green.c6", 0x100, 0x100, CRC(9c1326c9) SHA1(4d707335bb3be28c9cc14f36b63516e2145c73bf) )
	ROM_LOAD( "blue.d6",  0x200, 0x100, CRC(fd8b98d8) SHA1(a64fd145262a9f1d10945551358c79a5d2733e05) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "istellar2", 0, NO_DUMP )
ROM_END


void istellar_state::init_istellar()
{
	//m_z80_2_nmi_enable = 0;

	#if 0
	{
		uint8_t *ROM = memregion("maincpu")->base();

		ROM[0x4465] = 0x00;
		ROM[0x4466] = 0x00;
		ROM[0x4478] = 0x00;
		ROM[0x4479] = 0x00;
		ROM[0x43b4] = 0x00;
		ROM[0x43b5] = 0x00;
		ROM[0x4409] = 0x20;
		ROM[0x46de] = 0x00;
		ROM[0x46df] = 0x00;
	}
	#endif
}

} // anonymous namespace


//    YEAR  NAME       PARENT   MACHINE    INPUT     STATE            INIT           MONITOR  COMPANY          FULLNAME                                       FLAGS)
GAME( 1983, istellar,  0,       istellar,  istellar, istellar_state,  init_istellar, ROT0,    "Funai/Gakken",  "Inter Stellar (Laser Fantasy)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 1984, istellar2, 0,       istellar,  istellar, istellar_state,  init_istellar, ROT0,    "Funai/Gakken",  "Inter Stellar Zangus (Laser Fantasy vol. 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
