// license:BSD-3-Clause
// copyright-holders:Hau
/*

Galactic Storm
(c)1992 Taito

----------------------------------------------------------
MAIN PCB
CPU:MC68EC020RP25
TC0480SCP
TC0100SCN
TC0510NIO
TC0580PIV x2
TC0110PCR
TC0470LIN x2
TC0570SPC
TC0610
ADC0809CCN

OSC1:32MHz
OSC2:20MHz
----------------------------------------------------------
SOUND BOARD
CPU:MC68000P12F,MC68681P
ENSONIQ 5701,5510,5505

OSC1:16MHz
OSC2:30.47618MHz
----------------------------------------------------------
based on driver from drivers/gunbustr.c by Bryan McPhail & David Graves
Written by Hau
07/03/2008


tips
$300.b debugmode
$305.b invincibility
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"
#include "audio/taito_en.h"
#include "includes/galastrm.h"


/*********************************************************************/

INTERRUPT_GEN_MEMBER(galastrm_state::galastrm_interrupt)
{
	m_frame_counter ^= 1;
	device.execute().set_input_line(5, HOLD_LINE);
}

void galastrm_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_GALASTRM_INTERRUPT6:
		m_maincpu->set_input_line(6, HOLD_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in galastrm_state::device_timer");
	}
}


WRITE32_MEMBER(galastrm_state::galastrm_palette_w)
{
	if (ACCESSING_BITS_16_31)
		m_tc0110pcr_addr = data >> 16;
	if ((ACCESSING_BITS_0_15) && (m_tc0110pcr_addr < 4096))
		m_palette->set_pen_color(m_tc0110pcr_addr, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

WRITE32_MEMBER(galastrm_state::galastrm_tc0610_0_w)
{
	if (ACCESSING_BITS_16_31)
		m_tc0610_0_addr = data >> 16;
	if ((ACCESSING_BITS_0_15) && (m_tc0610_0_addr < 8))
		m_tc0610_ctrl_reg[0][m_tc0610_0_addr] = data;
}

WRITE32_MEMBER(galastrm_state::galastrm_tc0610_1_w)
{
	if (ACCESSING_BITS_16_31)
		m_tc0610_1_addr = data >> 16;
	if ((ACCESSING_BITS_0_15) && (m_tc0610_1_addr < 8))
		m_tc0610_ctrl_reg[1][m_tc0610_1_addr] = data;
}


CUSTOM_INPUT_MEMBER(galastrm_state::frame_counter_r)
{
	return m_frame_counter;
}

CUSTOM_INPUT_MEMBER(galastrm_state::coin_word_r)
{
	return m_coin_word;
}

WRITE32_MEMBER(galastrm_state::galastrm_input_w)
{
#if 0
{
char t[64];
COMBINE_DATA(&m_mem[offset]);

sprintf(t,"%08x %08x",m_mem[0],m_mem[1]);
popmessage(t);
}
#endif

	switch (offset)
	{
		case 0x00:
		{
			if (ACCESSING_BITS_24_31)   /* $400000 is watchdog */
			{
				machine().watchdog_reset();
			}

			if (ACCESSING_BITS_0_7)
			{
				m_eeprom->clk_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
				m_eeprom->di_write((data & 0x40) >> 6);
				m_eeprom->cs_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
				return;
			}
			return;
		}

		case 0x01:
		{
			if (ACCESSING_BITS_24_31)
			{
				machine().bookkeeping().coin_lockout_w(0, ~data & 0x01000000);
				machine().bookkeeping().coin_lockout_w(1, ~data & 0x02000000);
				machine().bookkeeping().coin_counter_w(0, data & 0x04000000);
				machine().bookkeeping().coin_counter_w(1, data & 0x04000000);
				m_coin_word = (data >> 16) &0xffff;
			}
//logerror("CPU #0 PC %06x: write input %06x\n",device->safe_pc(),offset);
		}
	}
}

READ32_MEMBER(galastrm_state::galastrm_adstick_ctrl_r)
{
	if (offset == 0x00)
	{
		if (ACCESSING_BITS_24_31)
			return ioport("STICKX")->read() << 24;
		if (ACCESSING_BITS_16_23)
			return ioport("STICKY")->read() << 16;
	}
	return 0;
}

WRITE32_MEMBER(galastrm_state::galastrm_adstick_ctrl_w)
{
	timer_set(downcast<cpu_device *>(&space.device())->cycles_to_attotime(1000), TIMER_GALASTRM_INTERRUPT6);
}

/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( galastrm_map, AS_PROGRAM, 32, galastrm_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM AM_SHARE("ram")                             /* main CPUA ram */
	AM_RANGE(0x300000, 0x303fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x400000, 0x400003) AM_READ_PORT("IN0")
	AM_RANGE(0x400004, 0x400007) AM_READ_PORT("IN1")
	AM_RANGE(0x400000, 0x400007) AM_WRITE(galastrm_input_w)                                 /* eerom etc. */
	AM_RANGE(0x40fff0, 0x40fff3) AM_WRITENOP
	AM_RANGE(0x500000, 0x500007) AM_READWRITE(galastrm_adstick_ctrl_r, galastrm_adstick_ctrl_w)
	AM_RANGE(0x600000, 0x6007ff) AM_RAM AM_SHARE("snd_shared")                              /* Sound shared ram */
	AM_RANGE(0x800000, 0x80ffff) AM_DEVREADWRITE("tc0480scp", tc0480scp_device, long_r, long_w)        /* tilemaps */
	AM_RANGE(0x830000, 0x83002f) AM_DEVREADWRITE("tc0480scp", tc0480scp_device, ctrl_long_r, ctrl_long_w)
	AM_RANGE(0x900000, 0x900003) AM_WRITE(galastrm_palette_w)                               /* TC0110PCR */
	AM_RANGE(0xb00000, 0xb00003) AM_WRITE(galastrm_tc0610_0_w)                              /* TC0610 */
	AM_RANGE(0xc00000, 0xc00003) AM_WRITE(galastrm_tc0610_1_w)
	AM_RANGE(0xd00000, 0xd0ffff) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, long_r, long_w)        /* piv tilemaps */
	AM_RANGE(0xd20000, 0xd2000f) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, ctrl_long_r, ctrl_long_w)
ADDRESS_MAP_END

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( galastrm )
	PORT_START("IN0")
//  PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  /* Freeze input */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galastrm_state,frame_counter_r, NULL)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_SERVICE_NO_TOGGLE( 0x00000001, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galastrm_state,coin_word_r, NULL)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


/***********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 8, 16, 24 },
	{ 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*64, 1*64,  2*64,  3*64,  4*64,  5*64,  6*64,  7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	64*16   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	16,16,    /* 16*16 characters */
	RGN_FRAC(1,1),
	4,        /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 3*4, 2*4, 7*4, 6*4, 1*4, 0*4, 5*4, 4*4, 11*4, 10*4, 15*4, 14*4, 9*4, 8*4, 13*4, 12*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8     /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( galastrm )
	GFXDECODE_ENTRY( "gfx2", 0x0, tile16x16_layout, 0, 4096 )
	GFXDECODE_ENTRY( "gfx1", 0x0, charlayout,       0, 4096 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

/***************************************************************************/

static MACHINE_CONFIG_START( galastrm, galastrm_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, 16000000) /* 16 MHz */
	MCFG_CPU_PROGRAM_MAP(galastrm_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galastrm_state,  galastrm_interrupt) /* VBL */

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 50*8)
	MCFG_SCREEN_VISIBLE_AREA(0+96, 40*8-1+96, 3*8+60, 32*8-1+60)
	MCFG_SCREEN_UPDATE_DRIVER(galastrm_state, screen_update_galastrm)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galastrm)
	MCFG_PALETTE_ADD("palette", 4096)

	MCFG_DEVICE_ADD("tc0100scn", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(0)
	MCFG_TC0100SCN_TX_REGION(2)
	MCFG_TC0100SCN_OFFSETS(-48, -56)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0480scp", TC0480SCP, 0)
	MCFG_TC0480SCP_GFX_REGION(1)
	MCFG_TC0480SCP_TX_REGION(3)
	MCFG_TC0480SCP_OFFSETS(-40, -3)
	MCFG_TC0480SCP_GFXDECODE("gfxdecode")

	/* sound hardware */
	MCFG_DEVICE_ADD("taito_en", TAITO_EN, 0)
MACHINE_CONFIG_END

/***************************************************************************/

ROM_START( galastrm )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "c99_15.ic105", 0x00000, 0x40000,  CRC(7eae8efd) SHA1(6bbb3da697dfcd93337b53895678e2a4ff2de457) )
	ROM_LOAD32_BYTE( "c99_12.ic102", 0x00001, 0x40000,  CRC(e059d1ee) SHA1(560951f95f270f0559b5289dda7f4ba74538cfcb) )
	ROM_LOAD32_BYTE( "c99_13.ic103", 0x00002, 0x40000,  CRC(885fcb35) SHA1(be10e109c461c1f776e98efa1b2a4d588aa0c41c) )
	ROM_LOAD32_BYTE( "c99_14.ic104", 0x00003, 0x40000,  CRC(457ef6b1) SHA1(06c2613d46addacd380a0f2413cd795b17ac9474) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "c99_23.ic8",  0x100000, 0x20000,  CRC(5718ee92) SHA1(33cfa60c5bceb1525498f27b598067d2dc620431) )
	ROM_LOAD16_BYTE( "c99_22.ic7",  0x100001, 0x20000,  CRC(b90f7c42) SHA1(e2fa9ee10ad61ae1a672c3357c0072b79ec7fbcb) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "c99-06.ic2",  0x000000, 0x100000, CRC(812ed3ae) SHA1(775904dd42643d0e3a30890590d5f8eac1fe78db) )  /* SCR 16x16 tiles */
	ROM_LOAD16_BYTE( "c99-05.ic1",  0x000001, 0x100000, CRC(a91ffba4) SHA1(467af9646ddad5fbb520b6bc13517ed4deacf479) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "c99-02.ic50", 0x000000, 0x100000, CRC(81e9fc6f) SHA1(4495a7d130b755b5a48eaa814d884d6bb8243bcb) )  /* OBJ 16x16 tiles */
	ROM_LOAD32_BYTE( "c99-01.ic51", 0x000001, 0x100000, CRC(9dda1267) SHA1(c639ba064496dcadf5f1e55332a12bb442e9dc86) )
	ROM_LOAD32_BYTE( "c99-04.ic66", 0x000002, 0x100000, CRC(a681760f) SHA1(23d4fc7eb778c8a25c4bc7cee1d0c8cdd828a996) )
	ROM_LOAD32_BYTE( "c99-03.ic67", 0x000003, 0x100000, CRC(a2807a27) SHA1(977e395ea2ab2fb82807d3cf5fe5f1dbbde99da0) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "c99-11.ic90",  0x00000,  0x80000, CRC(26a6926c) SHA1(918860e2829131e9ecfe983b2ae3e49e1c9ecd72) )  /* STY, spritemap */

	ROM_REGION16_BE( 0x1000000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "c99-08.ic3",  0x000000, 0x100000, CRC(fedb4187) SHA1(83563e4af795a0dfeb261a62c31b6fed72f45a4d) )  /* Ensoniq samples */
	ROM_LOAD16_BYTE( "c99-09.ic4",  0x200000, 0x100000, CRC(ba70b86b) SHA1(ffbb9547d6b6e47a3ef23206b5f40c57f3ea7619) )
	ROM_LOAD16_BYTE( "c99-10.ic5",  0x400000, 0x100000, CRC(da016f1e) SHA1(581ef158c6f6576618dd75429b1d3aa92cd3581d) )
	ROM_LOAD16_BYTE( "c99-07.ic2",  0x680000, 0x040000, CRC(4cc3136f) SHA1(d9d7556bbe6af161fa0651b1fbd72e7dbf0a8e82) )
	ROM_CONTINUE( 0x600000, 0x040000 )
	ROM_CONTINUE( 0x780000, 0x040000 )
	ROM_CONTINUE( 0x700000, 0x040000 )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-galastrm.bin", 0x0000, 0x0080, CRC(94efa7a6) SHA1(5870b988cb364065e8bd779efbdadca8d3ffc17c) )
ROM_END


GAME( 1992, galastrm, 0, galastrm, galastrm, driver_device, 0, ROT0, "Taito Corporation", "Galactic Storm (Japan)", MACHINE_IMPERFECT_GRAPHICS )
