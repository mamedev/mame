// license:BSD-3-Clause
// copyright-holders:Mark McDougall, R. Belmont
/*************************************************************

    Namco ND-1 Driver - Mark McDougall
                        R. Belmont

        With contributions from:
            James Jenkins
            Walter Fath

    Currently Supported Games:
        Namco Classic Vol #1
        Namco Classic Vol #2

PCB Layout
----------

8655960101 (8655970101)
|----------------------------------------|
|    LA4705        NC1_MAIN0B.14D  68000 |
|   4558  LC78815  NC1_MAIN1B.13D        |
|J  POT1                                 |
|                      AT28C16           |
|A                                       |
|                                        |
|M                                       |
|                    NC1_CG0.10C    *    |
|M       M9524LT                         |
|               POT2                     |
|A                                       |
|                    YGV608-F            |
|                                        |
|                                        |
|        NC1_VOICE.7B    49.152MHz       |
|N                  25.326MHz            |
|A                                       |
|M       C352                            |
|C                 MACH210         C416  |
|O                                       |
|4                                       |
|8       H8/3002                 62256   |
|                  NC1_SUB.1C    62256   |
|----------------------------------------|
Notes:
      68000 clock  : 12.288MHz (49.152 / 4)
      H8/3002 clock: 16.384MHz (49.152 / 3)
      C352 clock:    24.576MHz (49.152 / 2)
      VSync        : 60Hz

      POT1    : Master volume
      POT2    : Brightness adjustment (video level)
      M9524LT : ? possibly some sort of RGB video output chip
      AT28C16 : 2K x8 EEPROM
      YGV608-F: Yamaha YVG608-F video controller
      C352    : Namco custom QFP100
      C416    : Namco custom QFP176
      H8/3002 : Hitachi H8/3002 HD6413002F16 QFP100 microcontroller (H8/3002 has no internal ROM capability)
      MACH210 : PLCC44 CPLD, Namco KEYCUS, stamped 'KC001'
      62256   : 32K x8 SOJ28 SRAM
      *       : Unpopulated position for SOP44 Mask ROM 'CG1'

      NC1_MAIN0B.14D: 512K x16 EPROM type 27C240
      NC1_MAIN1B.13D: 512K x16 EPROM type 27C240
      NC1_SUB.1C    : 512K x16 EPROM type 27C240
      NC1_CG0.10C   : 16MBit SOP44 Mask ROM
      NC1_VOICE.7B  : 16MBit SOP44 Mask ROM

 *************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h8/h83002.h"
#include "includes/namcond1.h"
#include "sound/c352.h"
#include "machine/at28c16.h"

/*************************************************************/

static ADDRESS_MAP_START( namcond1_map, AS_PROGRAM, 16, namcond1_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0x800000, 0x80000f) AM_DEVREADWRITE("ygv608", ygv608_device, read, write)
	AM_RANGE(0xa00000, 0xa00fff) AM_DEVREADWRITE8("at28c16", at28c16_device, read, write, 0xff00)
#ifdef MAME_DEBUG
	AM_RANGE(0xb00000, 0xb00001) AM_DEVREAD("ygv608", ygv608_device, debug_trigger_r)
#endif
	AM_RANGE(0xc3ff00, 0xc3ffff) AM_READWRITE(cuskey_r,cuskey_w)
ADDRESS_MAP_END

/*************************************************************/

static INPUT_PORTS_START( namcond1 )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Freeze" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Test ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/* text-layer characters */

static const UINT32 pts_4bits_layout_xoffset[64] =
{
	STEP8( 0*256, 4 ), STEP8( 1*256, 4 ), STEP8( 4*256, 4 ), STEP8( 5*256, 4 ),
	STEP8( 16*256, 4 ), STEP8( 17*256, 4 ), STEP8( 20*256, 4 ), STEP8( 21*256, 4 )
};

static const UINT32 pts_4bits_layout_yoffset[64] =
{
	STEP8( 0*256, 8*4 ), STEP8( 2*256, 8*4 ), STEP8( 8*256, 8*4 ), STEP8( 10*256, 8*4 ),
	STEP8( 32*256, 8*4 ), STEP8( 34*256, 8*4 ), STEP8( 40*256, 8*4 ), STEP8( 42*256, 8*4 )
};

static const gfx_layout pts_8x8_4bits_layout =
{
	8,8,          /* 8*8 pixels */
	RGN_FRAC(1,1),        /* 65536 patterns */
	4,            /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	8*8*4,
	pts_4bits_layout_xoffset,
	pts_4bits_layout_yoffset
};

static const gfx_layout pts_16x16_4bits_layout =
{
	16,16,        /* 16*16 pixels */
	RGN_FRAC(1,1),        /* 16384 patterns */
	4,            /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	16*16*4,
	pts_4bits_layout_xoffset,
	pts_4bits_layout_yoffset
};

static const gfx_layout pts_32x32_4bits_layout =
{
	32,32,        /* 32*32 pixels */
	RGN_FRAC(1,1),         /* 4096 patterns */
	4,            /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	32*32*4,
	pts_4bits_layout_xoffset,
	pts_4bits_layout_yoffset
};

static const gfx_layout pts_64x64_4bits_layout =
{
	64,64,        /* 32*32 pixels */
	RGN_FRAC(1,1),         /* 1024 patterns */
	4,            /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	64*64*4,
	pts_4bits_layout_xoffset,
	pts_4bits_layout_yoffset
};


static const gfx_layout pts_8x8_8bits_layout =
{
	8,8,          /* 8*8 pixels */
	RGN_FRAC(1,1),        /* 32768 patterns */
	8,            /* 8 bits per pixel */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ STEP8( 0*512, 8 ) },
	{ STEP8( 0*512, 8*8 ) },
	8*8*8
};

static const gfx_layout pts_16x16_8bits_layout =
{
	16,16,        /* 16*16 pixels */
	RGN_FRAC(1,1),         /* 8192 patterns */
	8,            /* 8 bits per pixel */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ STEP8( 0*512, 8 ), STEP8( 1*512, 8 ) },
	{ STEP8( 0*512, 8*8 ), STEP8( 2*512, 8*8 ) },
	16*16*8
};

static GFXDECODE_START( namcond1 )
	GFXDECODE_ENTRY( "gfx1", 0x00000000, pts_8x8_4bits_layout,    0,  16 )
	GFXDECODE_ENTRY( "gfx1", 0x00000000, pts_16x16_4bits_layout,  0,  16 )
	GFXDECODE_ENTRY( "gfx1", 0x00000000, pts_32x32_4bits_layout,  0,  16 )
	GFXDECODE_ENTRY( "gfx1", 0x00000000, pts_64x64_4bits_layout,  0,  16 )
	GFXDECODE_ENTRY( "gfx1", 0x00000000, pts_8x8_8bits_layout,    0, 256 )
	GFXDECODE_ENTRY( "gfx1", 0x00000000, pts_16x16_8bits_layout,  0, 256 )
GFXDECODE_END

READ16_MEMBER(namcond1_state::mcu_p7_read)
{
	return 0xff;
}

READ16_MEMBER(namcond1_state::mcu_pa_read)
{
	return 0xff;
}

WRITE16_MEMBER(namcond1_state::mcu_pa_write)
{
	m_p8 = data;
}

/* H8/3002 MCU stuff */
static ADDRESS_MAP_START( nd1h8rwmap, AS_PROGRAM, 16, namcond1_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_SHARE("shared_ram")
	AM_RANGE(0xa00000, 0xa07fff) AM_DEVREADWRITE("c352", c352_device, read, write)
	AM_RANGE(0xc00000, 0xc00001) AM_READ_PORT("DSW")
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("P1_P2")
	AM_RANGE(0xc00010, 0xc00011) AM_NOP
	AM_RANGE(0xc00030, 0xc00031) AM_NOP
	AM_RANGE(0xc00040, 0xc00041) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( nd1h8iomap, AS_IO, 16, namcond1_state )
	AM_RANGE(h8_device::PORT_7, h8_device::PORT_7) AM_READ(mcu_p7_read )
	AM_RANGE(h8_device::PORT_A, h8_device::PORT_A) AM_READWRITE(mcu_pa_read, mcu_pa_write )
	AM_RANGE(h8_device::ADC_0,  h8_device::ADC_3)  AM_NOP // MCU reads these, but the games have no analog controls
ADDRESS_MAP_END

INTERRUPT_GEN_MEMBER(namcond1_state::mcu_interrupt)
{
	if( m_h8_irq5_enabled )
	{
		generic_pulse_irq_line(device.execute(), 5, 1);
	}
}

/******************************************
  ND-1 Master clock = 49.152MHz
  - 680000  = 12288000 (CLK/4)
  - H8/3002 = 16384000 (CLK/3)
  - The level 1 interrupt to the 68k has been measured at 60Hz.
*******************************************/

static MACHINE_CONFIG_START( namcond1, namcond1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_49_152MHz/4)
	MCFG_CPU_PROGRAM_MAP(namcond1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", namcond1_state,  irq1_line_hold)

	// I've disabled this for now, I don't think it's correct, it breaks ncv2 'game options' in test
	// mode (and could also be responsible for the random resets?)
	// also, if you log the timing of it and the scanlines on which the interrupt fires, it doesn't
	// seem correct for the intended purpose?
	//MCFG_DEVICE_PERIODIC_INT_DEVICE("ygv608", ygv608_device, timed_interrupt, 1000)


	MCFG_CPU_ADD("mcu", H83002, XTAL_49_152MHz/3 )
	MCFG_CPU_PROGRAM_MAP( nd1h8rwmap)
	MCFG_CPU_IO_MAP( nd1h8iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", namcond1_state,  mcu_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.0)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(288, 224)   // maximum display resolution (512x512 in theory)
	MCFG_SCREEN_VISIBLE_AREA(0, 287, 0, 223)   // default visible area
	MCFG_SCREEN_UPDATE_DEVICE("ygv608", ygv608_device, update_screen)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", namcond1)
	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_C352_ADD("c352", XTAL_49_152MHz/2, C352_DIVIDER_288)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "lspeaker", 1.00)

	MCFG_AT28C16_ADD( "at28c16", NULL )

	MCFG_YGV608_ADD("ygv608")
	MCFG_YGV608_GFXDECODE("gfxdecode")
	MCFG_YGV608_PALETTE("palette")
MACHINE_CONFIG_END

ROM_START( ncv1 )
	ROM_REGION( 0x100000,"maincpu", 0 )     /* 16MB for Main CPU */
	ROM_LOAD16_WORD( "nc2main0.14d", 0x00000, 0x80000, CRC(4ffc530b) SHA1(23d622d0261a3584236a77b2cefa522a0f46490e) )
	ROM_LOAD16_WORD( "nc2main1.13d", 0x80000, 0x80000, CRC(26499a4e) SHA1(4af0c365713b4a51da684a3423b07cbb70d9599b) )

	ROM_REGION( 0x80000,"mcu", 0 )      /* sub CPU */
	ROM_LOAD( "nc1sub.1c",          0x00000, 0x80000, CRC(48ea0de2) SHA1(33e57c8d084a960ccbda462d18e355de44ec7ad9) )

	ROM_REGION( 0x200000,"gfx1", 0 )    /* 2MB character generator */
	ROM_LOAD( "nc1cg0.10c",         0x000000, 0x200000, CRC(355e7f29) SHA1(47d92c4e28c3610a620d3c9b3be558199477f6d8) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "nc1voice.7b",     0x000000, 0x200000, CRC(91c85bd6) SHA1(c2af8b1518b2b601f2b14c3f327e7e3eae9e29fc) )
ROM_END

ROM_START( ncv1j )
	ROM_REGION( 0x100000,"maincpu", 0 )     /* 16MB for Main CPU */
	ROM_LOAD16_WORD( "nc1main0.14d",  0x00000, 0x80000, CRC(48ce0b2b) SHA1(07dfca8ba935ee0151211f9eb4d453f2da1d4bd7) )
	ROM_LOAD16_WORD( "nc1main1.13d",  0x80000, 0x80000, CRC(49f99235) SHA1(97afde7f7dddd8538de78a74325d0038cb1217f7) )

	ROM_REGION( 0x80000,"mcu", 0 )      /* sub CPU */
	ROM_LOAD( "nc1sub.1c",          0x00000, 0x80000, CRC(48ea0de2) SHA1(33e57c8d084a960ccbda462d18e355de44ec7ad9) )

	ROM_REGION( 0x200000,"gfx1", 0 )    /* 2MB character generator */
	ROM_LOAD( "nc1cg0.10c",         0x000000, 0x200000, CRC(355e7f29) SHA1(47d92c4e28c3610a620d3c9b3be558199477f6d8) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "nc1voice.7b",     0x000000, 0x200000, CRC(91c85bd6) SHA1(c2af8b1518b2b601f2b14c3f327e7e3eae9e29fc) )
ROM_END

ROM_START( ncv1j2 )
	ROM_REGION( 0x100000,"maincpu", 0 )     /* 16MB for Main CPU */
	ROM_LOAD16_WORD( "nc1main0b.14d", 0x00000, 0x80000, CRC(7207469d) SHA1(73faf1973a57c1bc2163e9ee3fe2febd3b8763a4) )
	ROM_LOAD16_WORD( "nc1main1b.13d", 0x80000, 0x80000, CRC(52401b17) SHA1(60c9f20831d0101c02dafbc0bd15422f71f3ad81) )

	ROM_REGION( 0x80000,"mcu", 0 )      /* sub CPU */
	ROM_LOAD( "nc1sub.1c",          0x00000, 0x80000, CRC(48ea0de2) SHA1(33e57c8d084a960ccbda462d18e355de44ec7ad9) )

	ROM_REGION( 0x200000,"gfx1", 0 )    /* 2MB character generator */
	ROM_LOAD( "nc1cg0.10c",         0x000000, 0x200000, CRC(355e7f29) SHA1(47d92c4e28c3610a620d3c9b3be558199477f6d8) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "nc1voice.7b",     0x000000, 0x200000, CRC(91c85bd6) SHA1(c2af8b1518b2b601f2b14c3f327e7e3eae9e29fc) )
ROM_END

ROM_START( ncv2 )
	ROM_REGION( 0x100000,"maincpu", 0 )     /* 16MB for Main CPU */
	ROM_LOAD16_WORD( "ncs2main0.14e", 0x00000, 0x80000, CRC(fb8a4123) SHA1(47acdfe9b5441d0e3649aaa9780e676f760c4e42) )
	ROM_LOAD16_WORD( "ncs2main1.13e", 0x80000, 0x80000, CRC(7a5ef23b) SHA1(0408742424a6abad512b5baff63409fe44353e10) )

	ROM_REGION( 0x80000,"mcu", 0 )      /* sub CPU */
	ROM_LOAD( "ncs1sub.1d",          0x00000, 0x80000, CRC(365cadbf) SHA1(7263220e1630239e3e88b828c00389d02628bd7d) )

	ROM_REGION( 0x400000,"gfx1", 0 )    /* 4MB character generator */
	ROM_LOAD( "ncs1cg0.10e",         0x000000, 0x200000, CRC(fdd24dbe) SHA1(4dceaae3d853075f58a7408be879afc91d80292e) )
	ROM_LOAD( "ncs1cg1.10e",         0x200000, 0x200000, CRC(007b19de) SHA1(d3c093543511ec1dd2f8be6db45f33820123cabc) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "ncs1voic.7c",     0x000000, 0x200000, CRC(ed05fd88) SHA1(ad88632c89a9946708fc6b4c9247e1bae9b2944b) )
ROM_END

ROM_START( ncv2j )
	ROM_REGION( 0x100000,"maincpu", 0 )     /* 16MB for Main CPU */
	ROM_LOAD16_WORD( "ncs1main0.14e", 0x00000, 0x80000, CRC(99991192) SHA1(e0b0e15ae23560b77119b3d3e4b2d2bb9d8b36c9) )
	ROM_LOAD16_WORD( "ncs1main1.13e", 0x80000, 0x80000, CRC(af4ba4f6) SHA1(ff5adfdd462cfd3f17fbe2401dfc88ff8c71b6f8) )

	ROM_REGION( 0x80000,"mcu", 0 )      /* sub CPU */
	ROM_LOAD("ncs1sub.1d",          0x00000, 0x80000, CRC(365cadbf) SHA1(7263220e1630239e3e88b828c00389d02628bd7d) )

	ROM_REGION( 0x400000,"gfx1", 0 )    /* 4MB character generator */
	ROM_LOAD( "ncs1cg0.10e",         0x000000, 0x200000, CRC(fdd24dbe) SHA1(4dceaae3d853075f58a7408be879afc91d80292e) )
	ROM_LOAD( "ncs1cg1.10e",         0x200000, 0x200000, CRC(007b19de) SHA1(d3c093543511ec1dd2f8be6db45f33820123cabc) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "ncs1voic.7c",     0x000000, 0x200000, CRC(ed05fd88) SHA1(ad88632c89a9946708fc6b4c9247e1bae9b2944b) )
ROM_END

GAME( 1995, ncv1,      0, namcond1, namcond1, driver_device, 0, ROT90, "Namco", "Namco Classic Collection Vol.1", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1995, ncv1j,  ncv1, namcond1, namcond1, driver_device, 0, ROT90, "Namco", "Namco Classic Collection Vol.1 (Japan, v1.00)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1995, ncv1j2, ncv1, namcond1, namcond1, driver_device, 0, ROT90, "Namco", "Namco Classic Collection Vol.1 (Japan, v1.03)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1996, ncv2,      0, namcond1, namcond1, driver_device, 0, ROT90, "Namco", "Namco Classic Collection Vol.2", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1996, ncv2j,  ncv2, namcond1, namcond1, driver_device, 0, ROT90, "Namco", "Namco Classic Collection Vol.2 (Japan)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
