// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/*
Funai / Gakken Interstellar Laser Fantasy laserdisc hardware
Driver by Andrew Gardner with help from Daphne Source

Notes:
    Holding down the TEST switch while hitting reset will bring up the Self Test.
    Hit TEST switch again for color and monitor calibration.
    This is somewhat strange hardware : More z80's than necessary
                                        3 bpp sprites
                                        6-pin dip switches with odd handling
                                        inverted DIP logic?
                                        CPU2 maps RAM over where its ROM lives

Todo:
    How does one best make one DIP switch bit from address 0x02 tie to two bits from address 0x03?
    Get real ROM labels!  The current labels are unfortunately a bit odd.
    Add sprite drawing.
    Convert to tilemaps.
    Make it work - this one should be close right now :/.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/ldv1000.h"
#include "emupal.h"
#include "render.h"
#include "speaker.h"


class istellar_state : public driver_device
{
public:
	istellar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_laserdisc(*this, "laserdisc") ,
		m_tile_ram(*this, "tile_ram"),
		m_tile_control_ram(*this, "tile_ctrl_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	void init_istellar();
	void istellar(machine_config &config);
private:
	required_device<pioneer_ldv1000_device> m_laserdisc;
	required_shared_ptr<uint8_t> m_tile_ram;
	required_shared_ptr<uint8_t> m_tile_control_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;
	DECLARE_READ8_MEMBER(z80_2_ldp_read);
	DECLARE_READ8_MEMBER(z80_2_unknown_read);
	DECLARE_WRITE8_MEMBER(z80_2_ldp_write);
	virtual void machine_start() override;
	uint32_t screen_update_istellar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void z80_0_io(address_map &map);
	void z80_0_mem(address_map &map);
	void z80_1_io(address_map &map);
	void z80_1_mem(address_map &map);
	void z80_2_io(address_map &map);
	void z80_2_mem(address_map &map);
};


/* There is only 1 crystal on the stack of 3 boards - speed is unknown, the following is Daphne's guess */
#define GUESSED_CLOCK (3072000)


/* VIDEO GOODS */
uint32_t istellar_state::screen_update_istellar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x, y;

	/* clear */
	bitmap.fill(0, cliprect);

	/* Draw tiles */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			int tile = m_tile_ram[x+y*32];
			int attr = m_tile_control_ram[x+y*32];

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,tile,attr & 0x0f,0, 0, x*8, y*8, 0);
		}
	}


	/* Draw sprites */

	return 0;
}


void istellar_state::machine_start()
{
}



/* MEMORY HANDLERS */


/* Z80 1 R/W */


/* Z80 2 R/W */
READ8_MEMBER(istellar_state::z80_2_ldp_read)
{
	uint8_t readResult = m_laserdisc->status_r();
	logerror("CPU2 : reading LDP : %x\n", readResult);
	return readResult;
}

READ8_MEMBER(istellar_state::z80_2_unknown_read)
{
	logerror("CPU2 : c000!\n");
	return 0x00;
}

WRITE8_MEMBER(istellar_state::z80_2_ldp_write)
{
	logerror("CPU2 : writing LDP : 0x%x\n", data);
	m_laserdisc->data_w(data);
}



/* PROGRAM MAPS */
void istellar_state::z80_0_mem(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xa7ff).ram();
	map(0xa800, 0xabff).ram().share("tile_ram");
	map(0xac00, 0xafff).ram().share("tile_ctrl_ram");
	map(0xb000, 0xb3ff).ram().share("sprite_ram");
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
	map(0xc000, 0xc000).r(FUNC(istellar_state::z80_2_unknown_read));     /* Seems to be thrown away every time it's read - maybe interrupt related? */
}


/* IO MAPS */
void istellar_state::z80_0_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x02, 0x02).portr("DSW1");
	map(0x03, 0x03).portr("DSW2");
	/*AM_RANGE(0x04,0x04) AM_WRITE(volatile_palette_write)*/
	map(0x05, 0x05).r("latch1", FUNC(generic_latch_8_device::read)).w("latch2", FUNC(generic_latch_8_device::write));
}

void istellar_state::z80_1_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).noprw(); /*AM_READWRITE(z80_1_slatch_read,z80_1_slatch_write)*/
	map(0x01, 0x01).noprw(); /*AM_READWRITE(z80_1_nmienable,z80_1_soundwrite_front)*/
	map(0x02, 0x02).noprw(); /*AM_WRITE(z80_1_soundwrite_rear)*/
}

void istellar_state::z80_2_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(istellar_state::z80_2_ldp_read), FUNC(istellar_state::z80_2_ldp_write));
	map(0x01, 0x01).r("latch2", FUNC(generic_latch_8_device::read)).w("latch1", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).r("latch2", FUNC(generic_latch_8_device::acknowledge_r));
/*  AM_RANGE(0x03,0x03) AM_WRITE(z80_2_ldtrans_write)*/
}


/* PORTS */
static INPUT_PORTS_START( istellar )
	/* TEST MODE might display a 0 for a short and a 1 for an open circuit?  If so, everything below is inverted. */
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

	/* "In case of inter-stellar upright type the coin switch 2 is not used."  Quoth the manual. */
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!5")        /* Maybe SERVICE? */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	/* SERVICE might be hanging out back here */
INPUT_PORTS_END

static const gfx_layout istellar_gfx_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_istellar )
	GFXDECODE_ENTRY( "gfx1", 0, istellar_gfx_layout, 0x0, 0x20 )
GFXDECODE_END

WRITE_LINE_MEMBER(istellar_state::vblank_irq)
{
	if (state)
	{
		/* Interrupt presumably comes from VBlank */
		m_maincpu->set_input_line(0, HOLD_LINE);

		/* Interrupt presumably comes from the LDP's status strobe */
		m_subcpu->set_input_line(0, ASSERT_LINE);
	}
}


/* DRIVER */
void istellar_state::istellar(machine_config &config)
{
	/* main cpu */
	Z80(config, m_maincpu, GUESSED_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &istellar_state::z80_0_mem);
	m_maincpu->set_addrmap(AS_IO, &istellar_state::z80_0_io);

	/* sound cpu */
	z80_device &audiocpu(Z80(config, "audiocpu", GUESSED_CLOCK));
	audiocpu.set_addrmap(AS_PROGRAM, &istellar_state::z80_1_mem);
	audiocpu.set_addrmap(AS_IO, &istellar_state::z80_1_io);

	/* ldp comm cpu */
	Z80(config, m_subcpu, GUESSED_CLOCK);
	m_subcpu->set_addrmap(AS_PROGRAM, &istellar_state::z80_2_mem);
	m_subcpu->set_addrmap(AS_IO, &istellar_state::z80_2_io);

	GENERIC_LATCH_8(config, "latch1");

	generic_latch_8_device &latch2(GENERIC_LATCH_8(config, "latch2"));
	latch2.data_pending_callback().set_inputline(m_subcpu, INPUT_LINE_NMI);
	latch2.set_separate_acknowledge(true);

	PIONEER_LDV1000(config, m_laserdisc, 0);
	m_laserdisc->set_overlay(256, 256, FUNC(istellar_state::screen_update_istellar));
	m_laserdisc->set_overlay_palette(m_palette);
	m_laserdisc->add_route(0, "lspeaker", 1.0);
	m_laserdisc->add_route(1, "rspeaker", 1.0);

	/* video hardware */
	m_laserdisc->add_ntsc_screen(config, "screen");
	subdevice<screen_device>("screen")->screen_vblank().set(FUNC(istellar_state::vblank_irq));

	// Daphne says "TODO: get the real interstellar resistor values"
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_istellar);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}


/* There is a photo of the PCB with blurry IC locations and labels.  Comments reflect what I can (barely) see. */
ROM_START( istellar )
	/* Main program CPU */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "rom2.top", 0x0000, 0x2000, CRC(5d643381) SHA1(75ca52c28a52f534eda00c18b0db97e9923ff670) )    /* At IC location C63 (top board) - label ? */
	ROM_LOAD( "rom3.top", 0x2000, 0x2000, CRC(ce5a2b09) SHA1(2de6a6e993c3411577ac0c834db8aaf16fb007ed) )    /* At IC location C64 (top board) - label ? */
	ROM_LOAD( "rom4.top", 0x4000, 0x2000, CRC(7c2cb1f1) SHA1(ffd92510c03c2d35a59d233883c2b9f57394a51c) )    /* At IC location C65 (top board) - label ? */
	ROM_LOAD( "rom5.top", 0x6000, 0x2000, CRC(354377f6) SHA1(bcf95b7ee1b47854e10baf24b0d8af3d56738b99) )    /* At IC location C66 (top board) - label ? */
	ROM_LOAD( "rom6.top", 0x8000, 0x2000, CRC(0319bf40) SHA1(f324626e457c3eb7d6b74bc6afbfcc3aab2b3c72) )    /* At IC location C67 (top board) - label ? */

	/* Sound CPU */
	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "rom1.top", 0x0000, 0x2000, CRC(4f34fb1d) SHA1(56ca19344c84c5989d0be797e2759f84760310be) )    /* At IC location C62 (top board) - label ? */

	/* LDP Communications CPU */
	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "rom11.bot", 0x0000, 0x2000, CRC(165cbc57) SHA1(39463888f22ec3125f0686066d923a9aae79a8f7) )   /* At IC location C12 (bottom board) - label IS11 */

	/* Tiles */
	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "rom9.bot", 0x0000, 0x2000, CRC(9d79acb6) SHA1(72af972695face0016afce8a26c629d963e86d48) )    /* At IC location C47? (bottom board) - label ? */
	ROM_LOAD( "rom8.bot", 0x2000, 0x2000, CRC(e9c9e490) SHA1(79aa35552b984018bc723adece5c40a0833a313c) )    /* At IC location C48? (bottom board) - label ? */
	ROM_LOAD( "rom7.bot", 0x4000, 0x2000, CRC(1447ce3a) SHA1(8545cec108df6adab303802b1407c89b2dceba21) )    /* At IC location C49? (bottom board) - label ? */

	/* Color PROMs */
	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "red6b.bot",   0x000, 0x100, CRC(5c52f844) SHA1(a8a3d91f3247ad13c805d8d8288b07f3cdaf1189) )   /* At IC location C63? (bottom board) - label ? */
	ROM_LOAD( "green6c.bot", 0x100, 0x100, CRC(7d8c845c) SHA1(04ae2ca0cc6679e21346ce34e9e01aa5bf4e2067) )   /* At IC location C62? (bottom board) - label ? */
	ROM_LOAD( "blue6d.bot",  0x200, 0x100, CRC(5ebb81f9) SHA1(285d60f2894c524ca80fc68ad7c2dfd9093a67ea) )   /* At IC location C61? (bottom board) - label ? */

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "istellar", 0, NO_DUMP )
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

//    YEAR  NAME      PARENT   MACHINE    INPUT     STATE            INIT           MONITOR  COMPANY          FULLNAME                       FLAGS)
GAME( 1983, istellar, 0,       istellar,  istellar, istellar_state,  init_istellar, ROT0,    "Funai/Gakken",  "Interstellar Laser Fantasy",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
