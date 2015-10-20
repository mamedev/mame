// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Avalanche hardware

    driver by Mike Balfour

    Games supported:
        * Avalanche
        * Catch (prototype)

    TODO:
        * Catch discrete sound

****************************************************************************

    Memory Map:
                    0000-1FFF               RAM
                    2000-2FFF       R       INPUTS
                    3000-3FFF       W       WATCHDOG
                    4000-4FFF       W       OUTPUTS
                    5000-5FFF       W       NOISE SOUND LEVEL (not in Catch)
                    6000-7FFF       R       PROGRAM ROM
                    8000-DFFF               UNUSED
                    E000-FFFF               PROGRAM ROM (remapped)

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/discrete.h"
#include "includes/avalnche.h"

#include "avalnche.lh"

#define MASTER_CLOCK XTAL_12_096MHz


/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 avalnche_state::screen_update_avalnche(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	for (offs = 0; offs < m_videoram.bytes(); offs++)
	{
		int i;

		UINT8 x = offs << 3;
		int y = offs >> 5;
		UINT8 data = m_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			if (m_avalance_video_inverted)
				pen = (data & 0x80) ? rgb_t::white : rgb_t::black;
			else
				pen = (data & 0x80) ? rgb_t::black : rgb_t::white;

			bitmap.pix32(y, x) = pen;

			data = data << 1;
			x = x + 1;
		}
	}

	return 0;
}


/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/

WRITE8_MEMBER(avalnche_state::avalance_video_invert_w)
{
	m_avalance_video_inverted = data & 0x01;
}

WRITE8_MEMBER(avalnche_state::catch_coin_counter_w)
{
	coin_counter_w(machine(), 0, data & 1);
	coin_counter_w(machine(), 1, data & 2);
}

WRITE8_MEMBER(avalnche_state::avalance_credit_1_lamp_w)
{
	set_led_status(machine(), 0, data & 1);
}

WRITE8_MEMBER(avalnche_state::avalance_credit_2_lamp_w)
{
	set_led_status(machine(), 1, data & 1);
}

WRITE8_MEMBER(avalnche_state::avalance_start_lamp_w)
{
	set_led_status(machine(), 2, data & 1);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, avalnche_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0ffc) AM_READ_PORT("IN0")
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x0ffc) AM_READ_PORT("IN1")
	AM_RANGE(0x2002, 0x2002) AM_MIRROR(0x0ffc) AM_READ_PORT("PADDLE")
	AM_RANGE(0x2003, 0x2003) AM_MIRROR(0x0ffc) AM_READNOP
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0fff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0ff8) AM_WRITE(avalance_credit_1_lamp_w)
	AM_RANGE(0x4001, 0x4001) AM_MIRROR(0x0ff8) AM_WRITE(avalnche_attract_enable_w)
	AM_RANGE(0x4002, 0x4002) AM_MIRROR(0x0ff8) AM_WRITE(avalance_video_invert_w)
	AM_RANGE(0x4003, 0x4003) AM_MIRROR(0x0ff8) AM_WRITE(avalance_credit_2_lamp_w)
	AM_RANGE(0x4004, 0x4006) AM_MIRROR(0x0ff8) AM_WRITE(avalnche_audio_w)
	AM_RANGE(0x4007, 0x4007) AM_MIRROR(0x0ff8) AM_WRITE(avalance_start_lamp_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_WRITE(avalnche_noise_amplitude_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( catch_map, AS_PROGRAM, 8, avalnche_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0ffc) AM_READ_PORT("IN0")
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x0ffc) AM_READ_PORT("IN1")
	AM_RANGE(0x2002, 0x2002) AM_MIRROR(0x0ffc) AM_READ_PORT("PADDLE")
	AM_RANGE(0x2003, 0x2003) AM_MIRROR(0x0ffc) AM_READNOP
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0fff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0ff8) AM_WRITE(avalance_credit_1_lamp_w)
//  AM_RANGE(0x4001, 0x4001) AM_MIRROR(0x0ff8) AM_WRITE(avalnche_attract_enable_w) /* It is attract_enable just like avalnche, but not hooked up yet. */
	AM_RANGE(0x4002, 0x4002) AM_MIRROR(0x0ff8) AM_WRITE(avalance_video_invert_w)
	AM_RANGE(0x4003, 0x4003) AM_MIRROR(0x0ff8) AM_WRITE(avalance_credit_2_lamp_w)
	AM_RANGE(0x4004, 0x4006) AM_MIRROR(0x0ff8) AM_WRITE(catch_audio_w)
	AM_RANGE(0x4007, 0x4007) AM_MIRROR(0x0ff8) AM_WRITE(avalance_start_lamp_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_WRITE(catch_coin_counter_w)
	AM_RANGE(0x7000, 0x7fff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( avalnche )
	PORT_START("IN0")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW:6" )         /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW:5" )         /* Listed as "Unused" */
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW:4,3")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x30, DEF_STR( German ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Spanish ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x04, 0x04, "Allow Extended Play" )   PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Lives/Extended Play" )   PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, "3/450 Points" )
	PORT_DIPSETTING(    0x08, "5/750 Points" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* SLAM */
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )       /* Serve */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")           /* VBLANK */

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x40, 0xb7) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
INPUT_PORTS_END

static INPUT_PORTS_START( cascade )
	PORT_INCLUDE( avalnche )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x30, DEF_STR( German ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Italian ) )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x08, 0x00, "Extended Play" )         PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, "1500 Points" )
	PORT_DIPSETTING(    0x08, "2500 Points" )
INPUT_PORTS_END

static INPUT_PORTS_START( catch )
	PORT_INCLUDE( avalnche )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x30, DEF_STR( German ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x10, DEF_STR( English ) )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x0c, 0x00, "Extended Play" )         PORT_DIPLOCATION("SW:8,7")
	PORT_DIPSETTING(    0x00, "600 Points" )
	PORT_DIPSETTING(    0x04, "850 Points" )
	PORT_DIPSETTING(    0x08, "1100 Points" )
	PORT_DIPSETTING(    0x0c, "1350 Points" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(75) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void avalnche_state::machine_start()
{
	save_item(NAME(m_avalance_video_inverted));
}

void avalnche_state::machine_reset()
{
	m_avalance_video_inverted = 0;
}

static MACHINE_CONFIG_START( avalnche, avalnche_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502,MASTER_CLOCK/16)     /* clock input is the "2H" signal divided by two */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(avalnche_state, nmi_line_pulse, 8*60)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(avalnche_state, screen_update_avalnche)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(avalnche)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( catch, avalnche )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(catch_map)

	/* sound hardware... */
	MCFG_DEVICE_REMOVE("discrete")
	MCFG_DEVICE_REMOVE("mono")

MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( avalnche )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH(  "30612.d2",     0x6800, 0x0800, CRC(3f975171) SHA1(afe680865da97824f1ebade4c7a2ba5d7ee2cbab) )
	ROM_LOAD_NIB_LOW (  "30615.d3",     0x6800, 0x0800, CRC(3e1a86b4) SHA1(3ff4cffea5b7a32231c0996473158f24c3bbe107) )
	ROM_LOAD_NIB_HIGH(  "30613.e2",     0x7000, 0x0800, CRC(47a224d3) SHA1(9feb7444a2e5a3d90a4fe78ae5d23c3a5039bfaa) )
	ROM_LOAD_NIB_LOW (  "30616.e3",     0x7000, 0x0800, CRC(f620f0f8) SHA1(7802b399b3469fc840796c3145b5f63781090956) )
	ROM_LOAD_NIB_HIGH(  "30611.c2",     0x7800, 0x0800, CRC(0ad07f85) SHA1(5a1a873b14e63dbb69ee3686ba53f7ca831fe9d0) )
	ROM_LOAD_NIB_LOW (  "30614.c3",     0x7800, 0x0800, CRC(a12d5d64) SHA1(1647d7416bf9266d07f066d3797bda943e004d24) )
ROM_END

ROM_START( cascade )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD(           "10005.1a",     0x6800, 0x0400, CRC(54774594) SHA1(03e013b563675fb8a30bd69836466a353db9bfc7) )
	ROM_LOAD(           "10005.1b",     0x6c00, 0x0400, CRC(fd9575ad) SHA1(ed0a1343d3c0456d458561256d5ee966b6c213f4) )
	ROM_LOAD(           "10005.2a",     0x7000, 0x0400, CRC(12857c75) SHA1(e42fdee70bd19d6f60e88f106a49dbbd68c591cd) )
	ROM_LOAD(           "10005.2b",     0x7400, 0x0400, CRC(26361698) SHA1(587cc6f0dc068a74aac41c2cb3336d70d2dd91e5) )
	ROM_LOAD(           "10005.3a",     0x7800, 0x0400, CRC(d1f422ff) SHA1(65ecbf0a22ba340d6a1768ed029030bac9c19e0f) )
	ROM_LOAD(           "10005.3b",     0x7c00, 0x0400, CRC(bb243d96) SHA1(3a387a8c50cd9b0db37d12b94dc9e260892dbf21) )
ROM_END

ROM_START( catchp )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH(  "catch.e2",     0x7000, 0x0800, CRC(ce1911ef) SHA1(1237b3e2c69342d2cccc8a3a9303f1b39b66e927) ) // n82s185
	ROM_LOAD_NIB_LOW (  "catch.e3",     0x7000, 0x0800, CRC(32bf254a) SHA1(22920d43beb2f21c0862c3675a4ee3e230d889b2) ) // "
	ROM_LOAD_NIB_HIGH(  "catch.d2",     0x7800, 0x0800, CRC(404a9551) SHA1(1e966ebb6e08e86e858146fe6c070e96b80171f8) ) // "
	ROM_LOAD_NIB_LOW (  "catch.d3",     0x7800, 0x0800, CRC(fde4dd73) SHA1(bc6fbfed8099865bef35b4c3061c7f5541bcec23) ) // "

	ROM_REGION( 0x0200, "user1", 0 ) // ?
	ROM_LOAD(           "catch.k3",     0x0000, 0x0200, CRC(a0ffc6fc) SHA1(b5c1a170acbf490786ed6ecc582712c3d2a97f37) ) // n82s129
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1978, avalnche, 0,        avalnche, avalnche, driver_device, 0, ROT0, "Atari", "Avalanche", MACHINE_SUPPORTS_SAVE, layout_avalnche )
GAMEL( 1978, cascade,  avalnche, avalnche, cascade, driver_device,  0, ROT0, "bootleg? (Sidam)", "Cascade", MACHINE_SUPPORTS_SAVE, layout_avalnche )
GAME ( 1977, catchp,   0,        catch,    catch, driver_device,    0, ROT0, "Atari", "Catch (prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND ) // pre-production board, evolved into Avalanche
