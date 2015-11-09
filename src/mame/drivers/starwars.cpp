// license:???
// copyright-holders:Steve Baines, Frank Palazzolo
/***************************************************************************

    Atari Star Wars hardware

    driver by Steve Baines (sulaco@ntlworld.com) and Frank Palazzolo

    This file is Copyright Steve Baines.
    Modified by Frank Palazzolo for sound support

    Games supported:
        * Star Wars
        * The Empire Strikes Back
        * TomCat prototype on Star Wars hardware

    Known bugs:
        * the monitor "overdrive" effect is not simulated when you
            get hit by enemy fire

****************************************************************************

    Memory map (TBA)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/vector.h"
#include "video/avgdvg.h"
#include "sound/tms5220.h"
#include "sound/pokey.h"
#include "machine/x2212.h"
#include "includes/starwars.h"
#include "includes/slapstic.h"


#define MASTER_CLOCK (XTAL_12_096MHz)
#define CLOCK_3KHZ   ((double)MASTER_CLOCK / 4096)


WRITE8_MEMBER(starwars_state::quad_pokeyn_w)
{
	static const char *const devname[4] = { "pokey1", "pokey2", "pokey3", "pokey4" };
	int pokey_num = (offset >> 3) & ~0x04;
	int control = (offset & 0x20) >> 2;
	int pokey_reg = (offset % 8) | control;
	pokey_device *pokey = machine().device<pokey_device>(devname[pokey_num]);

	pokey->write(pokey_reg, data);
}

/*************************************
 *
 *  Machine init
 *
 *************************************/

void starwars_state::machine_reset()
{
	/* ESB-specific */
	if (m_is_esb)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		/* reset the slapstic */
		m_slapstic_device->slapstic_reset();
		m_slapstic_current_bank = m_slapstic_device->slapstic_bank();
		memcpy(m_slapstic_base, &m_slapstic_source[m_slapstic_current_bank * 0x2000], 0x2000);

		/* reset all the banks */
		starwars_out_w(space, 4, 0);
	}

	/* reset the matrix processor */
	starwars_mproc_reset();
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

WRITE8_MEMBER(starwars_state::irq_ack_w)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}



/*************************************
 *
 *  ESB Slapstic handler
 *
 *************************************/

void starwars_state::esb_slapstic_tweak(address_space &space, offs_t offset)
{
	int new_bank = m_slapstic_device->slapstic_tweak(space, offset);

	/* update for the new bank */
	if (new_bank != m_slapstic_current_bank)
	{
		m_slapstic_current_bank = new_bank;
		memcpy(m_slapstic_base, &m_slapstic_source[m_slapstic_current_bank * 0x2000], 0x2000);
	}
}


READ8_MEMBER(starwars_state::esb_slapstic_r)
{
	int result = m_slapstic_base[offset];
	esb_slapstic_tweak(space, offset);
	return result;
}


WRITE8_MEMBER(starwars_state::esb_slapstic_w)
{
	esb_slapstic_tweak(space, offset);
}



/*************************************
 *
 *  ESB Opcode base handler
 *
 *************************************/

DIRECT_UPDATE_MEMBER(starwars_state::esb_setdirect)
{
	/* if we are in the slapstic region, process it */
	if ((address & 0xe000) == 0x8000)
	{
		offs_t pc = direct.space().device().safe_pc();

		/* filter out duplicates; we get these because the handler gets called for
		   multiple reasons:
		    1. Because we have read/write handlers backing the current address
		    2. Because the CPU core executed a jump to a new address
		*/
		if (pc != m_slapstic_last_pc || address != m_slapstic_last_address)
		{
			m_slapstic_last_pc = pc;
			m_slapstic_last_address = address;
			esb_slapstic_tweak(direct.space(), address & 0x1fff);
		}
		return ~0;
	}
	return address;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, starwars_state )
	AM_RANGE(0x0000, 0x2fff) AM_RAM AM_SHARE("vectorram") AM_REGION("maincpu", 0)
	AM_RANGE(0x3000, 0x3fff) AM_ROM                             /* vector_rom */
	AM_RANGE(0x4300, 0x431f) AM_READ_PORT("IN0")
	AM_RANGE(0x4320, 0x433f) AM_READ_PORT("IN1")
	AM_RANGE(0x4340, 0x435f) AM_READ_PORT("DSW0")
	AM_RANGE(0x4360, 0x437f) AM_READ_PORT("DSW1")
	AM_RANGE(0x4380, 0x439f) AM_READ(starwars_adc_r)            /* a-d control result */
	AM_RANGE(0x4400, 0x4400) AM_READWRITE(starwars_main_read_r, starwars_main_wr_w)
	AM_RANGE(0x4401, 0x4401) AM_READ(starwars_main_ready_flag_r)
	AM_RANGE(0x4500, 0x45ff) AM_DEVREADWRITE("x2212", x2212_device, read, write)
	AM_RANGE(0x4600, 0x461f) AM_DEVWRITE("avg", avg_starwars_device, go_w)
	AM_RANGE(0x4620, 0x463f) AM_DEVWRITE("avg", avg_starwars_device, reset_w)
	AM_RANGE(0x4640, 0x465f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x4660, 0x467f) AM_WRITE(irq_ack_w)
	AM_RANGE(0x4680, 0x469f) AM_READNOP AM_WRITE(starwars_out_w)
	AM_RANGE(0x46a0, 0x46bf) AM_WRITE(starwars_nstore_w)
	AM_RANGE(0x46c0, 0x46c2) AM_WRITE(starwars_adc_select_w)
	AM_RANGE(0x46e0, 0x46e0) AM_WRITE(starwars_soundrst_w)
	AM_RANGE(0x4700, 0x4707) AM_WRITE(starwars_math_w)
	AM_RANGE(0x4700, 0x4700) AM_READ(starwars_div_reh_r)
	AM_RANGE(0x4701, 0x4701) AM_READ(starwars_div_rel_r)
	AM_RANGE(0x4703, 0x4703) AM_READ(starwars_prng_r)           /* pseudo random number generator */
	AM_RANGE(0x4800, 0x4fff) AM_RAM                             /* CPU and Math RAM */
	AM_RANGE(0x5000, 0x5fff) AM_RAM AM_SHARE("mathram") /* CPU and Math RAM */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")                        /* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM                             /* rest of main_rom */
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, starwars_state )
	AM_RANGE(0x0000, 0x07ff) AM_WRITE(starwars_sout_w)
	AM_RANGE(0x0800, 0x0fff) AM_READ(starwars_sin_r)        /* SIN Read */
	AM_RANGE(0x1000, 0x107f) AM_RAM                         /* 6532 ram */
	AM_RANGE(0x1080, 0x109f) AM_DEVREADWRITE("riot", riot6532_device, read, write)
	AM_RANGE(0x1800, 0x183f) AM_WRITE(quad_pokeyn_w)
	AM_RANGE(0x2000, 0x27ff) AM_RAM                         /* program RAM */
	AM_RANGE(0x4000, 0x7fff) AM_ROM                         /* sound roms */
	AM_RANGE(0xb000, 0xffff) AM_ROM                         /* more sound roms */
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *  Dips Manual Verified and Defaults
 *  set for starwars and esb - 06/2009
 *
 *************************************/

static INPUT_PORTS_START( starwars )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	/* Bit 6 is VG_HALT */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("avg", avg_starwars_device, done_r, NULL)
	/* Bit 7 is MATH_RUN - see machine/starwars.c */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, starwars_state,matrix_flag_r, NULL)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x02, "Starting Shields" )  PORT_DIPLOCATION("10D:1,2")
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x02, "8" )
	PORT_DIPSETTING(    0x03, "9" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("10D:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, "Moderate" )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x10, "Bonus Shields" )  PORT_DIPLOCATION("10D:5,6")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("10D:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )  PORT_DIPLOCATION("10D:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )  PORT_DIPLOCATION("10EF:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
		/* Manual shows Coin_B (Right) as Bit 4,5 - actually Bit 3,4 */
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )  PORT_DIPLOCATION("10EF:3,4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
		/* Manual shows Coin_A (Left) as Bit 3 - actually Bit 5 */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coin_A ) )  PORT_DIPLOCATION("10EF:5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coin Adder" )  PORT_DIPLOCATION("10EF:6,7,8")
	PORT_DIPSETTING(    0x20, "2 gives 1" )
	PORT_DIPSETTING(    0x60, "4 gives 2" )
	PORT_DIPSETTING(    0xa0, "3 gives 1" )
	PORT_DIPSETTING(    0x40, "4 gives 1" )
	PORT_DIPSETTING(    0x80, "5 gives 1" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	/* 0xc0 and 0xe0 None */


	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END


static INPUT_PORTS_START( esb )
	PORT_INCLUDE( starwars )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, "Starting Shields" )  PORT_DIPLOCATION("10D:1,2")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Jedi-Letter Mode" )  PORT_DIPLOCATION("10D:5,6")
	PORT_DIPSETTING(    0x00, "Level Only" )
	PORT_DIPSETTING(    0x10, "Level" )
	PORT_DIPSETTING(    0x20, "Increment Only" )
	PORT_DIPSETTING(    0x30, "Increment" )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( starwars, starwars_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK / 8)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(starwars_state, irq0_line_assert, CLOCK_3KHZ / 12)
	MCFG_WATCHDOG_TIME_INIT(attotime::from_hz(CLOCK_3KHZ / 128))

	MCFG_SLAPSTIC_ADD("slapstic")

	MCFG_CPU_ADD("audiocpu", M6809, MASTER_CLOCK / 8)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_DEVICE_ADD("riot", RIOT6532, MASTER_CLOCK / 8)
	MCFG_RIOT6532_IN_PA_CB(READ8(starwars_state, r6532_porta_r))
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(starwars_state, r6532_porta_w))
	MCFG_RIOT6532_IN_PB_CB(DEVREAD8("tms", tms5220_device, status_r))
	MCFG_RIOT6532_OUT_PB_CB(DEVWRITE8("tms", tms5220_device, data_w))
	MCFG_RIOT6532_IRQ_CB(WRITELINE(starwars_state, snd_interrupt))

	MCFG_X2212_ADD_AUTOSAVE("x2212") /* nvram */

	/* video hardware */
	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD("screen", VECTOR)
	MCFG_SCREEN_REFRESH_RATE(CLOCK_3KHZ / 12 / 6)
	MCFG_SCREEN_SIZE(400, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 250, 0, 280)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)

	MCFG_DEVICE_ADD("avg", AVG_STARWARS, 0)
	MCFG_AVGDVG_VECTOR("vector")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("pokey1", POKEY, MASTER_CLOCK / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_SOUND_ADD("pokey2", POKEY, MASTER_CLOCK / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_SOUND_ADD("pokey3", POKEY, MASTER_CLOCK / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_SOUND_ADD("pokey4", POKEY, MASTER_CLOCK / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_SOUND_ADD("tms", TMS5220, MASTER_CLOCK/2/9)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/


ROM_START( starwars )
	ROM_REGION( 0x12000, "maincpu", 0 )     /* 2 64k ROM spaces */
	ROM_LOAD( "136021.105",   0x3000, 0x1000, CRC(538e7d2f) SHA1(032c933fd94a6b0b294beee29159a24494ae969b) ) /* 3000-3fff is 4k vector rom */
	ROM_LOAD( "136021.214.1f",   0x6000, 0x2000, CRC(04f1876e) SHA1(c1d3637cb31ece0890c25f6122d6bcd27e6ffe0c) )   /* ROM 0 bank pages 0 and 1 */
	ROM_CONTINUE(               0x10000, 0x2000 )
	ROM_LOAD( "136021.102.1hj",  0x8000, 0x2000, CRC(f725e344) SHA1(f8943b67f2ea032ab9538084756ba86f892be5ca) ) /*  8k ROM 1 bank */
	ROM_LOAD( "136021.203.1jk",  0xa000, 0x2000, CRC(f6da0a00) SHA1(dd53b643be856787bbc4da63e5eb132f98f623c3) ) /*  8k ROM 2 bank */
	ROM_LOAD( "136021.104.1kl",  0xc000, 0x2000, CRC(7e406703) SHA1(981b505d6e06d7149f8bcb3e81e4d0c790f2fc86) ) /*  8k ROM 3 bank */
	ROM_LOAD( "136021.206.1m",   0xe000, 0x2000, CRC(c7e51237) SHA1(4960f4446271316e3f730eeb2531dbc702947395) ) /*  8k ROM 4 bank */

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136021.107",   0x4000, 0x2000, CRC(dbf3aea2) SHA1(c38661b2b846fe93487eef09ca3cda19c44f08a0) ) /* Sound ROM 0 */
	ROM_RELOAD(               0xc000, 0x2000 )
	ROM_LOAD( "136021.208",   0x6000, 0x2000, CRC(e38070a8) SHA1(c858ae1702efdd48615453ab46e488848891d139) ) /* Sound ROM 0 */
	ROM_RELOAD(               0xe000, 0x2000 )

	ROM_REGION( 0x100, "user1", 0)
	ROM_LOAD( "136021-105.1l",   0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "136021.110",   0x0000, 0x0400, CRC(810e040e) SHA1(d247cbb0afb4538d5161f8ce9eab337cdb3f2da4) ) /* PROM 0 */
	ROM_LOAD( "136021.111",   0x0400, 0x0400, CRC(ae69881c) SHA1(f3420c6e15602956fd94982a5d8d4ddd015ed977) ) /* PROM 1 */
	ROM_LOAD( "136021.112",   0x0800, 0x0400, CRC(ecf22628) SHA1(4dcf5153221feca329b8e8d199bd4fc00b151d9c) ) /* PROM 2 */
	ROM_LOAD( "136021.113",   0x0c00, 0x0400, CRC(83febfde) SHA1(e13541b09d1724204fdb171528e9a1c83c799c1c) ) /* PROM 3 */
ROM_END

ROM_START( starwars1 )
	ROM_REGION( 0x12000, "maincpu", 0 )     /* 2 64k ROM spaces */
	ROM_LOAD( "136021.105",   0x3000, 0x1000, CRC(538e7d2f) SHA1(032c933fd94a6b0b294beee29159a24494ae969b) ) /* 3000-3fff is 4k vector rom */
	ROM_LOAD( "136021.114.1f",   0x6000, 0x2000, CRC(e75ff867) SHA1(3a40de920c31ffa3c3e67f3edf653b79fcc5ddd7) )   /* ROM 0 bank pages 0 and 1 */
	ROM_CONTINUE(               0x10000, 0x2000 )
	ROM_LOAD( "136021.102.1hj",  0x8000, 0x2000, CRC(f725e344) SHA1(f8943b67f2ea032ab9538084756ba86f892be5ca) ) /*  8k ROM 1 bank */
	ROM_LOAD( "136021.203.1jk",  0xa000, 0x2000, CRC(f6da0a00) SHA1(dd53b643be856787bbc4da63e5eb132f98f623c3) ) /*  8k ROM 2 bank */
	ROM_LOAD( "136021.104.1kl",  0xc000, 0x2000, CRC(7e406703) SHA1(981b505d6e06d7149f8bcb3e81e4d0c790f2fc86) ) /*  8k ROM 3 bank */
	ROM_LOAD( "136021.206.1m",   0xe000, 0x2000, CRC(c7e51237) SHA1(4960f4446271316e3f730eeb2531dbc702947395) ) /*  8k ROM 4 bank */

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136021.107",   0x4000, 0x2000, CRC(dbf3aea2) SHA1(c38661b2b846fe93487eef09ca3cda19c44f08a0) ) /* Sound ROM 0 */
	ROM_RELOAD(               0xc000, 0x2000 )
	ROM_LOAD( "136021.208",   0x6000, 0x2000, CRC(e38070a8) SHA1(c858ae1702efdd48615453ab46e488848891d139) ) /* Sound ROM 0 */
	ROM_RELOAD(               0xe000, 0x2000 )

	ROM_REGION( 0x100, "user1", 0)
	ROM_LOAD( "136021-105.1l",   0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0)
	ROM_LOAD( "136021.110",   0x0000, 0x0400, CRC(810e040e) SHA1(d247cbb0afb4538d5161f8ce9eab337cdb3f2da4) ) /* PROM 0 */
	ROM_LOAD( "136021.111",   0x0400, 0x0400, CRC(ae69881c) SHA1(f3420c6e15602956fd94982a5d8d4ddd015ed977) ) /* PROM 1 */
	ROM_LOAD( "136021.112",   0x0800, 0x0400, CRC(ecf22628) SHA1(4dcf5153221feca329b8e8d199bd4fc00b151d9c) ) /* PROM 2 */
	ROM_LOAD( "136021.113",   0x0c00, 0x0400, CRC(83febfde) SHA1(e13541b09d1724204fdb171528e9a1c83c799c1c) ) /* PROM 3 */
ROM_END

ROM_START( starwarso )
	ROM_REGION( 0x12000, "maincpu", 0 )     /* 2 64k ROM spaces */
	ROM_LOAD( "136021.105",   0x3000, 0x1000, CRC(538e7d2f) SHA1(032c933fd94a6b0b294beee29159a24494ae969b) ) /* 3000-3fff is 4k vector rom */
	ROM_LOAD( "136021-114.1f",   0x6000, 0x2000, CRC(e75ff867) SHA1(3a40de920c31ffa3c3e67f3edf653b79fcc5ddd7) )   /* ROM 0 bank pages 0 and 1 */
	ROM_CONTINUE(            0x10000, 0x2000 )
	ROM_LOAD( "136021-102.1hj",  0x8000, 0x2000, CRC(f725e344) SHA1(f8943b67f2ea032ab9538084756ba86f892be5ca) ) /*  8k ROM 1 bank */
	ROM_LOAD( "136021-103.1jk",  0xa000, 0x2000, CRC(3fde9ccb) SHA1(8d88fc7a28ac8f189f8aba08598732ac8c5491aa) ) /*  8k ROM 2 bank */
	ROM_LOAD( "136021-104.1kl",  0xc000, 0x2000, CRC(7e406703) SHA1(981b505d6e06d7149f8bcb3e81e4d0c790f2fc86) ) /*  8k ROM 3 bank */
	ROM_LOAD( "136021-206.1m",   0xe000, 0x2000, CRC(c7e51237) SHA1(4960f4446271316e3f730eeb2531dbc702947395) ) /*  8k ROM 4 bank */

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136021.107",   0x4000, 0x2000, CRC(dbf3aea2) SHA1(c38661b2b846fe93487eef09ca3cda19c44f08a0) ) /* Sound ROM 0 */
	ROM_RELOAD(               0xc000, 0x2000 )
	ROM_LOAD( "136021.208",   0x6000, 0x2000, CRC(e38070a8) SHA1(c858ae1702efdd48615453ab46e488848891d139) ) /* Sound ROM 0 */
	ROM_RELOAD(               0xe000, 0x2000 )

	ROM_REGION( 0x100, "user1", 0)
	ROM_LOAD( "136021-105.1l",   0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "136021.110",   0x0000, 0x0400, CRC(810e040e) SHA1(d247cbb0afb4538d5161f8ce9eab337cdb3f2da4) ) /* PROM 0 */
	ROM_LOAD( "136021.111",   0x0400, 0x0400, CRC(ae69881c) SHA1(f3420c6e15602956fd94982a5d8d4ddd015ed977) ) /* PROM 1 */
	ROM_LOAD( "136021.112",   0x0800, 0x0400, CRC(ecf22628) SHA1(4dcf5153221feca329b8e8d199bd4fc00b151d9c) ) /* PROM 2 */
	ROM_LOAD( "136021.113",   0x0c00, 0x0400, CRC(83febfde) SHA1(e13541b09d1724204fdb171528e9a1c83c799c1c) ) /* PROM 3 */
ROM_END



ROM_START( tomcatsw )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "tcavg3.1l",    0x3000, 0x1000, CRC(27188aa9) SHA1(5d9a978a7ac1913b57586e81045a1b955db27b48) )
	ROM_LOAD( "tc6.1f",       0x6000, 0x2000, CRC(56e284ff) SHA1(a5fda9db0f6b8f7d28a4a607976fe978e62158cf) )
	ROM_LOAD( "tc8.1hj",      0x8000, 0x2000, CRC(7b7575e3) SHA1(bdb838603ffb12195966d0ce454900253bc0f43f) )
	ROM_LOAD( "tca.1jk",      0xa000, 0x2000, CRC(a1020331) SHA1(128745a2ec771ac818a8fbba59a08f0cf5f28e8f) )
	ROM_LOAD( "tce.1m",       0xe000, 0x2000, CRC(4a3de8a3) SHA1(e48fc17201326358317f6b428e583ecaa3ecb881) )

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136021.107",   0x4000, 0x2000, NO_DUMP ) /* Sound ROM 0 */
	ROM_RELOAD(               0xc000, 0x2000 )
	ROM_LOAD( "136021.208",   0x6000, 0x2000, NO_DUMP ) /* Sound ROM 0 */
	ROM_RELOAD(               0xe000, 0x2000 )

	ROM_REGION( 0x100, "user1", 0)
	ROM_LOAD( "136021-105.1l",   0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "136021.110",   0x0000, 0x0400, CRC(810e040e) SHA1(d247cbb0afb4538d5161f8ce9eab337cdb3f2da4) ) /* PROM 0 */
	ROM_LOAD( "136021.111",   0x0400, 0x0400, CRC(ae69881c) SHA1(f3420c6e15602956fd94982a5d8d4ddd015ed977) ) /* PROM 1 */
	ROM_LOAD( "136021.112",   0x0800, 0x0400, CRC(ecf22628) SHA1(4dcf5153221feca329b8e8d199bd4fc00b151d9c) ) /* PROM 2 */
	ROM_LOAD( "136021.113",   0x0c00, 0x0400, CRC(83febfde) SHA1(e13541b09d1724204fdb171528e9a1c83c799c1c) ) /* PROM 3 */
ROM_END


ROM_START( esb )
	ROM_REGION( 0x22000, "maincpu", 0 )     /* 64k for code and a buttload for the banked ROMs */
	ROM_LOAD( "136031.111",   0x03000, 0x1000, CRC(b1f9bd12) SHA1(76f15395c9fdcd80dd241307a377031a1f44e150) )    /* 3000-3fff is 4k vector rom */
	ROM_LOAD( "136031.101",   0x06000, 0x2000, CRC(ef1e3ae5) SHA1(d228ff076faa7f9605badeee3b827adb62593e0a) )
	ROM_CONTINUE(             0x10000, 0x2000 )
	/* $8000 - $9fff : slapstic page */
	ROM_LOAD( "136031.102",   0x0a000, 0x2000, CRC(62ce5c12) SHA1(976256acf4499dc396542a117910009a8808f448) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_LOAD( "136031.203",   0x0c000, 0x2000, CRC(27b0889b) SHA1(a13074e83f0f57d65096d7f49ae78f33ab00c479) )
	ROM_CONTINUE(             0x1e000, 0x2000 )
	ROM_LOAD( "136031.104",   0x0e000, 0x2000, CRC(fd5c725e) SHA1(541cfd004b1736b6cec13836dfa813f00eedeed0) )
	ROM_CONTINUE(             0x20000, 0x2000 )

	ROM_LOAD( "136031.105",   0x14000, 0x4000, CRC(ea9e4dce) SHA1(9363fd5b1fce62c2306b448a7766eaf7ec97cdf5) ) /* slapstic 0, 1 */
	ROM_LOAD( "136031.106",   0x18000, 0x4000, CRC(76d07f59) SHA1(44dd018b406f95e1512ce92923c2c87f1458844f) ) /* slapstic 2, 3 */

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136031.113",   0x4000, 0x2000, CRC(24ae3815) SHA1(b1a93af76de79b902317eebbc50b400b1f8c1e3c) ) /* Sound ROM 0 */
	ROM_CONTINUE(             0xc000, 0x2000 )
	ROM_LOAD( "136031.112",   0x6000, 0x2000, CRC(ca72d341) SHA1(52de5b82bb85d7c9caad2047e540d0748aa93ba5) ) /* Sound ROM 1 */
	ROM_CONTINUE(             0xe000, 0x2000 )

	ROM_REGION( 0x100, "user1", 0)
	ROM_LOAD( "136021-105.1l",   0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "136031.110",   0x0000, 0x0400, CRC(b8d0f69d) SHA1(c196f1a592bd1ac482a81e23efa224d9dfaefc0a) ) /* PROM 0 */
	ROM_LOAD( "136031.109",   0x0400, 0x0400, CRC(6a2a4d98) SHA1(cefca71f025f92a193c5a7d8b5ab8be10db2fd44) ) /* PROM 1 */
	ROM_LOAD( "136031.108",   0x0800, 0x0400, CRC(6a76138f) SHA1(9ef7af898a3e29d03f35045901023615a6a55205) ) /* PROM 2 */
	ROM_LOAD( "136031.107",   0x0c00, 0x0400, CRC(afbf6e01) SHA1(0a6438e6c106d98e5d67a019751e1584324f5e5c) ) /* PROM 3 */
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(starwars_state,starwars)
{
	/* prepare the mathbox */
	m_is_esb = 0;
	starwars_mproc_init();

	/* initialize banking */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x6000, 0x10000 - 0x6000);
	membank("bank1")->set_entry(0);
}


DRIVER_INIT_MEMBER(starwars_state,esb)
{
	UINT8 *rom = memregion("maincpu")->base();

	/* init the slapstic */
	m_slapstic_device->slapstic_init(machine(), 101);
	m_slapstic_source = &rom[0x14000];
	m_slapstic_base = &rom[0x08000];

	/* install an opcode base handler */
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.set_direct_update_handler(direct_update_delegate(FUNC(starwars_state::esb_setdirect), this));

	/* install read/write handlers for it */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x8000, 0x9fff, read8_delegate(FUNC(starwars_state::esb_slapstic_r),this), write8_delegate(FUNC(starwars_state::esb_slapstic_w),this));

	/* install additional banking */
	m_maincpu->space(AS_PROGRAM).install_read_bank(0xa000, 0xffff, "bank2");

	/* prepare the matrix processor */
	m_is_esb = 1;
	starwars_mproc_init();

	/* initialize banking */
	membank("bank1")->configure_entries(0, 2, rom + 0x6000, 0x10000 - 0x6000);
	membank("bank1")->set_entry(0);
	membank("bank2")->configure_entries(0, 2, rom + 0xa000, 0x1c000 - 0xa000);
	membank("bank2")->set_entry(0);

	/* additional globals for state saving */
	save_item(NAME(m_slapstic_current_bank));
	save_item(NAME(m_slapstic_last_pc));
	save_item(NAME(m_slapstic_last_address));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, starwars, 0,        starwars, starwars, starwars_state, starwars, ROT0, "Atari", "Star Wars (set 1)", 0 ) // newest
GAME( 1983, starwars1,starwars, starwars, starwars, starwars_state, starwars, ROT0, "Atari", "Star Wars (set 2)", 0 )
GAME( 1983, starwarso,starwars, starwars, starwars, starwars_state, starwars, ROT0, "Atari", "Star Wars (set 3)", 0 ) // oldest
// is there an even older starwars set with 136021-106.1m ?

GAME( 1983, tomcatsw, tomcat,   starwars, starwars, starwars_state, starwars, ROT0, "Atari", "TomCat (Star Wars hardware, prototype)", MACHINE_NO_SOUND )

GAME( 1985, esb,      0,        starwars, esb, starwars_state,      esb,      ROT0, "Atari Games", "The Empire Strikes Back", 0 )
