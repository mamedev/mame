// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna
/***************************************************************************

World Rally (c) 1993 Gaelco (Designed & Developed by Zigurat. Produced by Gaelco)

Driver by Manuel Abadia, Mike Coates, Nicola Salmoria and Miguel Angel Horna

Thanks to GAELCO SA for the DS5002FP code and information about the encryption

Main PCB components:
====================

CPUs related:
=============
* 1xDS5002FP @ D12 (Dallas security processor @ 12 MHz)
* 1xHM62256ALFP-8T (32KB NVSRAM) @ C11 (encrypted DS5002FP program code)
* 1xLithium cell
* 2xMS6264A-20NC (16KB SRAM) @ D14 & D15 (shared memory between M68000 & DS5002FP)
* 4x74LS157 (Quad 2 input multiplexer) @ F14, F15, F16 & F17 (used to select M68000 or DS5002FP address bus)
* 4x74LS245 (Octal bus transceiver) @ C14, C15, C16 & C17 (used to store shared RAM data)
* 2x74LS373 (Octal tristate latch) @ D16 & D17 (used by DS5002FP to access data from shared RAM)
* 1xMC68000P12 @ C20 (Motorola 68000 @ 12 MHz)
* 1xOSC24MHz @ B20
* 2xM27C4001 @ C22 & C23 (M68000 program ROMs)
* 1xPAL20L8 @ B23 (handles 1st level M68000 memory map)
    0 -> DTACK  M68000 data ack
    1 -> SELACT
    2 -> Input/sound (see below)
    3 -> ACTEXT
    4 -> SELMOV
    5 -> CSW
    6 -> CSR
    7 -> EXT

* 1x74LS138 (3 to 8 line decoder) @ B13 (handles 2nd level M68000 memory map)
    0 -> IN0    DIPSW #1 & #2
    1 -> IN1    Joystick 1P & 2P, COINSW, STARTSW
    2 -> IN2    Wheel input
    3 -> -
    4 -> IN4    TESTSW & SERVICESW
    5 -> OUT (see below)
    6 -> CSBAN  OKIM6295 bankswitch
    7 -> CSSON  OKIM6295 R/W

* 1x74LS259 (8 bit addressable latches) @A7 (handles 3rd level M68000 memory map)
    0 -> Coin lockout 1
    1 -> Coin lockout 2
    2 -> Coin counter 1
    3 -> Coin counter 2
    4 -> Sound muting
    5 -> flip screen
    6 -> ENA/D?
    7 -> CKA/D?

* 1x16AS @ B15
    0 -> OE
    1 -> XSRL   Shared RAM @ D14
    2 -> XSRH   Shared RAM @ D15
    3 -> SAD    Shared Access with DS5002FP
    4 -> SRE    Shared Access with M68000
    5 -> TRANS
    6 -> XLD
    7 -> XHI

Sound related:
==============
* 1xOKIM6295 @ C6
* 2xM27C4001 @ C1 & C3 (OKI ADPCM samples)
* 1xPAL16R4 @ E2 (handles OKI ROM banking)

Graphics related:
=================
* 1xOSC30MHz @ D5
* 2xTPC1020AFN-84C (FPGA) @ G8 & G13 (GFX processing)
* 2xMS6264A-20NC (8KB SRAM) @ I16 & I17 (Video RAM)
* 4xUM6116BK-25 (2KB SRAM) @ H1, H2, H4 & H5
* 2xUM6116BK-25 (2KB SRAM) @ H22 & H23

Palette related:
================
* 2xMS6264A-20NC (8KB SRAM) @ C8 & C9 (palette RAM (xxxxBBBBRRRRGGGG))
* 2x74HCT273 (octal D-Type flip-flop with clear) @ B8 & B9 (connected to RGB output)

Controls related: (added by Mirko Mattioli)
=================
When optical wheel is selected (via dipswitch), then gear shift (low/high) is enabled.
On the real PCB the optical wheel encoder is connected to 74LS169 ICs (@A16 and @A17)
via a flip-flop IC mounted in the steering wheel assembly. As a result, the output
of the flip-flop generates a signal that contains the information about the steering
direction; this signal is routed to pin #1 (U/D) at ICs A16 and A17 (high when turn
left and low when turn right). The second signal of the optical encoder goes directly
to pin #2 (CLK) at ICs A16 and A17 and it is a clock for the 74LS169 ICs; this clock
frequency is proportional to the movements of the steering wheel: fast movements
produces a high clock frequency, slow movements a low freq.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"
#include "includes/wrally.h"


static ADDRESS_MAP_START( wrally_map, AS_PROGRAM, 16, wrally_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                         /* ROM */
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(vram_w) AM_SHARE("videoram")   /* encrypted Video RAM */
	AM_RANGE(0x108000, 0x108007) AM_RAM AM_SHARE("vregs")                                   /* Video Registers */
	AM_RANGE(0x10800c, 0x10800d) AM_WRITENOP                                                /* CLR INT Video */
	AM_RANGE(0x200000, 0x203fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    /* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")                               /* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("WHEEL")
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(okim6295_bankswitch_w)                                /* OKI6295 bankswitch */
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)  /* OKI6295 status/data register */
	AM_RANGE(0x70000a, 0x70001b) AM_WRITE(wrally_coin_lockout_w)                                /* Coin lockouts */
	AM_RANGE(0x70002a, 0x70003b) AM_WRITE(wrally_coin_counter_w)                                /* Coin counters */
	AM_RANGE(0x70004a, 0x70004b) AM_WRITENOP                                                /* Sound muting */
	AM_RANGE(0x70005a, 0x70005b) AM_WRITE(flipscreen_w)                                  /* Flip screen */
	AM_RANGE(0x70006a, 0x70007b) AM_WRITENOP                                                /* ??? */
	AM_RANGE(0xfec000, 0xfeffff) AM_RAM AM_SHARE("shareram")                                        /* Work RAM (shared with DS5002FP) */
ADDRESS_MAP_END

READ8_MEMBER(wrally_state::dallas_share_r)
{
	UINT8 *shareram = (UINT8 *)m_shareram.target();

	return shareram[BYTE_XOR_BE(offset)];
}

WRITE8_MEMBER(wrally_state::dallas_share_w)
{
	UINT8 *shareram = (UINT8 *)m_shareram.target();

	shareram[BYTE_XOR_BE(offset)] = data;
}

static ADDRESS_MAP_START( dallas_rom, AS_PROGRAM, 8, wrally_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                                 /* Code in NVRAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dallas_ram, AS_IO, 8, wrally_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(dallas_share_r, dallas_share_w)   AM_MASK(0x3fff)     /* Shared RAM with the main CPU */
ADDRESS_MAP_END

static INPUT_PORTS_START( wrally )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )      PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Number of Joysticks" )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x0018, 0x0018, "Control Configuration" )    PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0018, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0010, "Pot Wheel" )
	PORT_DIPSETTING(      0x0000, "Optical Wheel" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) )         PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )          PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )          PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Credit configuration" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x4000, "Start 1C/Continue 1C" )
	PORT_DIPSETTING(      0x0000, "Start 2C/Continue 1C" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Gear Shift") PORT_TOGGLE PORT_REVERSE
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("WHEEL")
	PORT_BIT( 0xff00, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_RIGHT) PORT_CODE_INC(KEYCODE_LEFT) PORT_REVERSE
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* Go to test mode NOW */
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout wrally_tilelayout16 =
{
	16,16,                                  /* 16x16 tiles */
	RGN_FRAC(1,2),                          /* number of tiles */
	4,                                      /* 4 bpp */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+4, 16*16+5, 16*16+6, 16*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( wrally )
	GFXDECODE_ENTRY( "gfx1", 0x000000, wrally_tilelayout16, 0, 64*8 )
GFXDECODE_END


static MACHINE_CONFIG_START( wrally, wrally_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,XTAL_24MHz/2)        /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(wrally_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", wrally_state,  irq6_line_hold)

	MCFG_CPU_ADD("mcu", DS5002FP, XTAL_24MHz/2) /* verified on pcb */
	MCFG_DS5002FP_CONFIG( 0x88, 0x00, 0x80 )
	MCFG_CPU_PROGRAM_MAP(dallas_rom)
	MCFG_CPU_IO_MAP(dallas_ram)

	MCFG_QUANTUM_TIME(attotime::from_hz(38400))                 /* heavy sync */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(64*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(8, 24*16-8-1, 16, 16*16-8-1)
	MCFG_SCREEN_UPDATE_DRIVER(wrally_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wrally)
	MCFG_PALETTE_ADD("palette", 1024*8)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH)                 /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( wrally )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "worldr17.c23", 0x000000, 0x080000, CRC(050f5629) SHA1(74fc2cd5114f3bc4b2429f1d8d7eeb1658f9f179) ) /* Only difference compared to set 2 is how the Dallas DS5002FP */
	ROM_LOAD16_BYTE( "worldr16.c22", 0x000001, 0x080000, CRC(9e0d126c) SHA1(369360b7ec2c3497af3bf62b4eba24c3d9f94675) ) /* power failure shows on screen, IE: "Tension  baja " */

	ROM_REGION( 0x10000, "mcu", 0 ) /* DS5002FP code */
	ROM_LOAD( "wrdallas.bin", 0x00000, 0x8000, CRC(547d1768) SHA1(c58d1edd072d796be0663fb265f4739ec006b688) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "worldr21.i13", 0x000000, 0x080000, CRC(b7fddb12) SHA1(619a75daac8cbba7e85c97ca19733e2196d66d5c) )
	ROM_LOAD16_BYTE( "worldr20.i11", 0x000001, 0x080000, CRC(58b2809a) SHA1(8741ec544c54e2a2f5d17ac2f8400ee2ce382e83) )
	ROM_LOAD16_BYTE( "worldr19.i09", 0x100000, 0x080000, CRC(018b35bb) SHA1(ca789e23d18cc7d7e48b6858e6b61e03bf88b475) )
	ROM_LOAD16_BYTE( "worldr18.i07", 0x100001, 0x080000, CRC(b37c807e) SHA1(9e6155a2b5206c0d4dca669d24d9fe9830027651) )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "worldr14.c01",   0x000000, 0x080000, CRC(e931c2ee) SHA1(ea1cf8ad52713e5136a370e289567eea9e6403d6) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(     0x040000, 0x080000 )
	ROM_LOAD( "worldr15.c03",   0x0c0000, 0x080000, CRC(11f0fe2c) SHA1(96c2a04874fa036576b7cfc5559bb0e33582ffd2) )

	ROM_REGION( 0x0514, "plds", 0 ) /* PAL's and GAL's */
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

ROM_START( wrallya )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "c23.bin", 0x000000, 0x080000, CRC(8b7d93c3) SHA1(ce4163eebc5d4a0c1266d650523b1ffc702d1b87) ) /* Only difference compared to set 1 is how the Dallas DS5002FP */
	ROM_LOAD16_BYTE( "c22.bin", 0x000001, 0x080000, CRC(56da43b6) SHA1(02db8f969ed5e7f5e5356c45c0312faf5f000335) ) /* power failure shows on screen, IE: "Power  Failure" */

	ROM_REGION( 0x10000, "mcu", 0 ) /* DS5002FP code */
	ROM_LOAD( "wrdallas.bin", 0x00000, 0x8000, CRC(547d1768) SHA1(c58d1edd072d796be0663fb265f4739ec006b688) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "worldr21.i13", 0x000000, 0x080000, CRC(b7fddb12) SHA1(619a75daac8cbba7e85c97ca19733e2196d66d5c) )
	ROM_LOAD16_BYTE( "worldr20.i11", 0x000001, 0x080000, CRC(58b2809a) SHA1(8741ec544c54e2a2f5d17ac2f8400ee2ce382e83) )
	ROM_LOAD16_BYTE( "worldr19.i09", 0x100000, 0x080000, CRC(018b35bb) SHA1(ca789e23d18cc7d7e48b6858e6b61e03bf88b475) )
	ROM_LOAD16_BYTE( "worldr18.i07", 0x100001, 0x080000, CRC(b37c807e) SHA1(9e6155a2b5206c0d4dca669d24d9fe9830027651) )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "worldr14.c01",   0x000000, 0x080000, CRC(e931c2ee) SHA1(ea1cf8ad52713e5136a370e289567eea9e6403d6) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(     0x040000, 0x080000 )
	ROM_LOAD( "worldr15.c03",   0x0c0000, 0x080000, CRC(11f0fe2c) SHA1(96c2a04874fa036576b7cfc5559bb0e33582ffd2) )

	ROM_REGION( 0x0514, "plds", 0 ) /* PAL's and GAL's */
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

ROM_START( wrallyb ) /* Board Marked 930217, Atari License */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "rally.c23", 0x000000, 0x080000, CRC(366595ad) SHA1(e16341ed9eacf9b729c28184268150ea9b62f185) ) /* North & South America only... */
	ROM_LOAD16_BYTE( "rally.c22", 0x000001, 0x080000, CRC(0ad4ec6f) SHA1(991557cf25fe960b1c586e990e6019befe5a11d0) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* DS5002FP code */
	ROM_LOAD( "wrdallas.bin", 0x00000, 0x8000, CRC(547d1768) SHA1(c58d1edd072d796be0663fb265f4739ec006b688) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "rally h-12.h12", 0x000000, 0x100000, CRC(3353dc00) SHA1(db3b1686751dcaa231d66c08b5be81fcfe299ad9) ) /* Same data, different layout */
	ROM_LOAD( "rally h-8.h8",   0x100000, 0x100000, CRC(58dcd024) SHA1(384ff296d3c7c8e0c4469231d1940de3cea89fc2) )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "sound c-1.c1", 0x000000, 0x100000, CRC(2d69c9b8) SHA1(328cb3c928dc6921c0c3f0277f59bca6c747c504) ) /* Same data in a single rom */
	ROM_RELOAD(       0x040000, 0x100000 )

	ROM_REGION( 0x0514, "plds", 0 ) /* PAL's and GAL's */
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END


GAME( 1993, wrally,  0,      wrally, wrally, driver_device, 0, ROT0, "Gaelco", "World Rally (set 1)", MACHINE_SUPPORTS_SAVE ) /* Dallas DS5002FP power failure shows as: "Tension  baja " */
GAME( 1993, wrallya, wrally, wrally, wrally, driver_device, 0, ROT0, "Gaelco", "World Rally (set 2)", MACHINE_SUPPORTS_SAVE ) /* Dallas DS5002FP power failure shows as: "Power  Failure" */
GAME( 1993, wrallyb, wrally, wrally, wrally, driver_device, 0, ROT0, "Gaelco (Atari license)", "World Rally (US, 930217)", MACHINE_SUPPORTS_SAVE )
