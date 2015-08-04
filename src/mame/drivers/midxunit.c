// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Midway X-unit system

    driver by Aaron Giles
    based on older drivers by Ernesto Corvi, Alex Pasadyn, Zsolt Vasvari

    Games supported:
        * Revolution X

    Known bugs:
        * none at this time

***************************************************************************

Revolution X
Midway, 1994

This is the only known game on Midway X-Unit hardware.

PCB Layout
----------

5770-13534-04
WILLIAMS ELECTRONIC GAMES INC.
|----------------------------------------------------------------------------------------------------------------|
|   P8       PIC16C57  BR2325                LED1  LED2                                                          |
|3.6864MHz   MAX691  RESET_SW   |---------|                      42S4260                                         |
|SCC2691   LH5268               |LATTICE  |                                                                      |
|ADC0848                        |PLSI 1032|     MACH110          42S4260             |--------|       40MHz      |
|P4                             |(A-17719)|    (A-17720)                             |TMS34020|                  |
|                               |---------|                      42S4260             |        |                  |
|               8MHz                                                                 |        |                  |
|P12                                           GAL16V8           42S4260             |--------|                  |
|   P7                                        (A-17721)                                                          |
|                        HM538123                                                                                |
|                                                                                                                |
|--| FUSE                HM538123                                                                                |
   | ULN2064                                                                                                     |
|--|                     HM538123                    25MHz       U51           U52           U53         U54     |
|                                                                                                                |
|                        HM538123                                U63           U64           U65         U66     |
|J                                             |-----------|                                                     |
|A                       HM538123              |5410-12862 |     U71           U72           U73         U74     |
|M                                             |-00        |                                                     |
|M             MB84256   HM538123              |WILLIAMS   |     U81           U82           U83         U84     |
|A             MB84256                         |ELECTRONICS|                                                     |
|                        HM538123              |-----------|     U91           U92           U93         U94     |
|                                                                                                                |
|                        HM538123                                U101          U102          U103        U104    |
|--|     DIPSW(U105)           DIPSW(U108)        GAL16V8                                                        |
   |                                             (A-17722)       U110          U111          U112        U113    |
|--|                                                                                                             |
|          P2       P6      P10     P3      P5                   U120          U121          U122        U123    |
|----------------------------------------------------------------------------------------------------------------|
Notes:                                                         |                                               |
      TMS34020 - clock 40MHz                                   |--------------- All ROMs 27C4001 --------------|
      PIC16C57 - clock 0.625MHz [25/40]
      VSync    - 54.7074Hz
      HSync    - 15.8104kHz
      HM538123 - Hitachi HM538123 128k x8 Multiport CMOS Video DRAM with 256 x8 Serial Access Memory
      42S4260  - NEC 42S4260 256k x16 DRAM
      MB84256  - 32k x8 SRAM
      LH5268   - 8k x8 SRAM for CMOS storage / settings
      SCC2691  - Philips SCC2691 Universal asynchronous receiver/transmitter (UART)
      MAX691   - Maxim MAX691 Microprocessor Supervisory Circuit (for battery backup power switching)
      ADC0848  - Analog Devices ADC0848 8-Bit Microprocessor Compatible A/D Converter with Multiplexer Option
      P2       - Player 3 controls connector
      P3       - Player 4 controls connector
      P4       - Gun connector
      P5       - 15 pin connector
      P6       - 7 pin connector
      P7       - Power connector for sound board
      P8       - Sound board data cable connector
      P10      - 10 pin connector
      P12      - 4 pin connector

There's a separate sound board also, but it wasn't available so is not documented here.

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/adsp2100/adsp2100.h"
#include "audio/dcs.h"
#include "machine/nvram.h"
#include "includes/midtunit.h"
#include "includes/midxunit.h"


#define PIXEL_CLOCK     (8000000)



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, midxunit_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_READWRITE(midtunit_vram_data_r, midtunit_vram_data_w)
	AM_RANGE(0x00800000, 0x00bfffff) AM_READWRITE(midtunit_vram_color_r, midtunit_vram_color_w)
	AM_RANGE(0x20000000, 0x20ffffff) AM_RAM
	AM_RANGE(0x40800000, 0x4fffffff) AM_WRITE(midxunit_unknown_w)
	AM_RANGE(0x60400000, 0x6040001f) AM_READWRITE(midxunit_status_r, midxunit_security_clock_w)
	AM_RANGE(0x60c00000, 0x60c0007f) AM_READ(midxunit_io_r)
	AM_RANGE(0x60c00080, 0x60c000df) AM_WRITE(midxunit_io_w)
	AM_RANGE(0x60c000e0, 0x60c000ff) AM_READWRITE(midxunit_security_r, midxunit_security_w)
	AM_RANGE(0x80800000, 0x8080001f) AM_READWRITE(midxunit_analog_r, midxunit_analog_select_w)
	AM_RANGE(0x80c00000, 0x80c000ff) AM_READWRITE(midxunit_uart_r, midxunit_uart_w)
	AM_RANGE(0xa0440000, 0xa047ffff) AM_READWRITE(midxunit_cmos_r, midxunit_cmos_w) AM_SHARE("nvram")
	AM_RANGE(0xa0800000, 0xa08fffff) AM_READWRITE(midxunit_paletteram_r, midxunit_paletteram_w) AM_SHARE("palette")
	AM_RANGE(0xc0000000, 0xc00003ff) AM_DEVREADWRITE("maincpu", tms34020_device, io_register_r, io_register_w)
	AM_RANGE(0xc0c00000, 0xc0c000ff) AM_MIRROR(0x00400000) AM_READWRITE(midtunit_dma_r, midtunit_dma_w)
	AM_RANGE(0xf8000000, 0xfeffffff) AM_READ(midwunit_gfxrom_r)
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( revx )
	PORT_START("IN0")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_SERVICE_NO_TOGGLE(0x0010, IP_ACTIVE_LOW)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SPECIAL ) /* coin door */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 ) /* bill validator */

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Flip_Screen ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0001, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0000, "Dipswitch Coinage" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0002, DEF_STR( On ))
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x001c, "1" )
	PORT_DIPSETTING(      0x0018, "2" )
	PORT_DIPSETTING(      0x0014, "3" )
	PORT_DIPSETTING(      0x000c, "ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x00e0, 0x0060, "Credits" )
	PORT_DIPSETTING(      0x0020, "3 Start/1 Continue" )
	PORT_DIPSETTING(      0x00e0, "2 Start/2 Continue" )
	PORT_DIPSETTING(      0x00a0, "2 Start/1 Continue" )
	PORT_DIPSETTING(      0x0000, "1 Start/4 Continue" )
	PORT_DIPSETTING(      0x0040, "1 Start/3 Continue" )
	PORT_DIPSETTING(      0x0060, "1 Start/1 Continue" )
	PORT_DIPNAME( 0x0300, 0x0300, "Country" )
	PORT_DIPSETTING(      0x0300, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( German ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( Unused ))
	PORT_DIPNAME( 0x0400, 0x0400, "Bill Validator" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0800, 0x0000, "Two Counters" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Players ) )
	PORT_DIPSETTING(      0x1000, "3 Players" )
	PORT_DIPSETTING(      0x0000, "2 Players" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Cabinet ))
	PORT_DIPSETTING(      0x2000, "Rev X" )
	PORT_DIPSETTING(      0x0000, "Terminator 2" )
	PORT_DIPNAME( 0x4000, 0x4000, "Video Freeze" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START("AN0")
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN1")
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN2")
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN3")
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN4")
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN5")
	PORT_BIT( 0x00ff, 0x0080, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( midxunit, midxunit_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS34020, 40000000)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TMS340X0_HALT_ON_RESET(FALSE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(PIXEL_CLOCK) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(1) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_IND16_CB(midxunit_state, scanline_update)       /* scanline updater (indexed16) */
	MCFG_TMS340X0_TO_SHIFTREG_CB(midtunit_state, to_shiftreg)           /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(midtunit_state, from_shiftreg)          /* read from shiftreg function */

	MCFG_MACHINE_RESET_OVERRIDE(midxunit_state,midxunit)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_PALETTE_ADD("palette", 32768)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 506, 101, 501, 289, 20, 274)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", tms34010_device, tms340x0_ind16)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_VIDEO_START_OVERRIDE(midxunit_state,midxunit)

	MCFG_DEVICE_ADD("serial_pic", MIDWAY_SERIAL_PIC, 0)
	/* serial prefixes 419, 420 */
	MCFG_MIDWAY_SERIAL_PIC_UPPER(419);

	/* sound hardware */
	MCFG_DEVICE_ADD("dcs", DCS_AUDIO_2K_UART, 0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( revx )
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "revx_snd.2", 0x000000, 0x80000, CRC(4ed9e803) SHA1(ba50f1beb9f2a2cf5110897209b5e9a2951ff165) )
	ROM_LOAD16_BYTE( "revx_snd.3", 0x200000, 0x80000, CRC(af8f253b) SHA1(25a0000cab177378070f7a6e3c7378fe87fad63e) )
	ROM_LOAD16_BYTE( "revx_snd.4", 0x400000, 0x80000, CRC(3ccce59c) SHA1(e81a31d64c64e7b1d25f178c53da3d68453c203c) )
	ROM_LOAD16_BYTE( "revx_snd.5", 0x600000, 0x80000, CRC(a0438006) SHA1(560d216d21cb8073dbee0fd20ebe589932a9144e) )
	ROM_LOAD16_BYTE( "revx_snd.6", 0x800000, 0x80000, CRC(b7b34f60) SHA1(3b9682c6a00fa3bdb47e69d8e8ceccc244ee55b5) )
	ROM_LOAD16_BYTE( "revx_snd.7", 0xa00000, 0x80000, CRC(6795fd88) SHA1(7c3790730a8b99b63112c851318b1c7e4989e5e0) )
	ROM_LOAD16_BYTE( "revx_snd.8", 0xc00000, 0x80000, CRC(793a7eb5) SHA1(4b1f81b68f95cedf1b356ef362d1eb37acc74b16) )
	ROM_LOAD16_BYTE( "revx_snd.9", 0xe00000, 0x80000, CRC(14ddbea1) SHA1(8dba9dc5529ea77c4312ea61f825bf9062ffc6c3) )

	ROM_REGION16_LE( 0x200000, "maincpu", 0 )   /* 34020 code */
	ROM_LOAD32_BYTE( "revx.51",  0x00000, 0x80000, CRC(9960ac7c) SHA1(441322f061d627ca7573f612f370a85794681d0f) )
	ROM_LOAD32_BYTE( "revx.52",  0x00001, 0x80000, CRC(fbf55510) SHA1(8a5b0004ed09391fe37f0f501b979903d6ae4868) )
	ROM_LOAD32_BYTE( "revx.53",  0x00002, 0x80000, CRC(a045b265) SHA1(b294d3a56e41f5ec4ab9bbcc0088833b1cab1879) )
	ROM_LOAD32_BYTE( "revx.54",  0x00003, 0x80000, CRC(24471269) SHA1(262345bd147402100785459af422dafd1c562787) )

	ROM_REGION( 0x2000, "pic", 0 )
	ROM_LOAD( "revx_16c57.bin", 0x0000000, 0x2000, CRC(eb8a8649) SHA1(a1e1d0b7a5e9802e8f889eb7e719259656dc8133) )

	ROM_REGION( 0x1000000, "gfxrom", 0 )
	ROM_LOAD32_BYTE( "revx.120", 0x0000000, 0x80000, CRC(523af1f0) SHA1(a67c0fd757e860fc1c1236945952a295b4d5df5a) )
	ROM_LOAD32_BYTE( "revx.121", 0x0000001, 0x80000, CRC(78201d93) SHA1(fb0b8f887eec433f7624f387d7fb6f633ea30d7c) )
	ROM_LOAD32_BYTE( "revx.122", 0x0000002, 0x80000, CRC(2cf36144) SHA1(22ed0eefa2c7c836811fac5f717c3f38254eabc2) )
	ROM_LOAD32_BYTE( "revx.123", 0x0000003, 0x80000, CRC(6912e1fb) SHA1(416f0de711d80e9182ede524c568c5095b1bec61) )

	ROM_LOAD32_BYTE( "revx.110", 0x0200000, 0x80000, CRC(e3f7f0af) SHA1(5877d9f488b0f4362a9482007c3ff7f4589a036f) )
	ROM_LOAD32_BYTE( "revx.111", 0x0200001, 0x80000, CRC(49fe1a69) SHA1(9ae54b461f0524c034fbcb6fcd3fd5ccb5d7265a) )
	ROM_LOAD32_BYTE( "revx.112", 0x0200002, 0x80000, CRC(7e3ba175) SHA1(dd2fe90988b544f67dbe6151282fd80d49631388) )
	ROM_LOAD32_BYTE( "revx.113", 0x0200003, 0x80000, CRC(c0817583) SHA1(2f866e5888e212b245984344950d0e1fb8957a73) )

	ROM_LOAD32_BYTE( "revx.101", 0x0400000, 0x80000, CRC(5a08272a) SHA1(17da3c9d71114f5fdbf50281a942be3da3b6f564) )
	ROM_LOAD32_BYTE( "revx.102", 0x0400001, 0x80000, CRC(11d567d2) SHA1(7ebe6fd39a0335e1fdda150d2dc86c3eaab17b2e) )
	ROM_LOAD32_BYTE( "revx.103", 0x0400002, 0x80000, CRC(d338e63b) SHA1(0a038217542667b3a01ecbcad824ee18c084f293) )
	ROM_LOAD32_BYTE( "revx.104", 0x0400003, 0x80000, CRC(f7b701ee) SHA1(0fc5886e5857326bee7272d5d482a878cbcea83c) )

	ROM_LOAD32_BYTE( "revx.91",  0x0600000, 0x80000, CRC(52a63713) SHA1(dcc0ff3596bd5d273a8d4fd33b0b9b9d588d8354) )
	ROM_LOAD32_BYTE( "revx.92",  0x0600001, 0x80000, CRC(fae3621b) SHA1(715d41ea789c0c724baa5bd90f6f0f06b9cb1c64) )
	ROM_LOAD32_BYTE( "revx.93",  0x0600002, 0x80000, CRC(7065cf95) SHA1(6c5888da099e51c4b1c592721c5027c899cf52e3) )
	ROM_LOAD32_BYTE( "revx.94",  0x0600003, 0x80000, CRC(600d5b98) SHA1(6aef98c91f87390c0759fe71a272a3ccadd71066) )

	ROM_LOAD32_BYTE( "revx.81",  0x0800000, 0x80000, CRC(729eacb1) SHA1(d130162ae22b99c84abfbe014c4e23e20afb757f) )
	ROM_LOAD32_BYTE( "revx.82",  0x0800001, 0x80000, CRC(19acb904) SHA1(516059b516bc5b1669c9eb085e0cdcdee520dff0) )
	ROM_LOAD32_BYTE( "revx.83",  0x0800002, 0x80000, CRC(0e223456) SHA1(1eedbd667f4a214533d1c22ca5312ecf2d4a3ab4) )
	ROM_LOAD32_BYTE( "revx.84",  0x0800003, 0x80000, CRC(d3de0192) SHA1(2d22c5bac07a7411f326691167c7c70eba4b371f) )

	ROM_LOAD32_BYTE( "revx.71",  0x0a00000, 0x80000, CRC(2b29fddb) SHA1(57b71e5c18b56bf58216e690fdefa6d30d88d34a) )
	ROM_LOAD32_BYTE( "revx.72",  0x0a00001, 0x80000, CRC(2680281b) SHA1(d1ae0701d20166a00d8733d9d12246c140a5fb96) )
	ROM_LOAD32_BYTE( "revx.73",  0x0a00002, 0x80000, CRC(420bde4d) SHA1(0f010cdeddb59631a5420dddfc142c50c2a1e65a) )
	ROM_LOAD32_BYTE( "revx.74",  0x0a00003, 0x80000, CRC(26627410) SHA1(a612121554549afff5c8e8c54774ca7b0220eda8) )

	ROM_LOAD32_BYTE( "revx.63",  0x0c00000, 0x80000, CRC(3066e3f3) SHA1(25548923db111bd6c6cff44bfb63cb9eb2ef0b53) )
	ROM_LOAD32_BYTE( "revx.64",  0x0c00001, 0x80000, CRC(c33f5309) SHA1(6bb333f563ea66c4c862ffd5fb91fb5e1b919fe8) )
	ROM_LOAD32_BYTE( "revx.65",  0x0c00002, 0x80000, CRC(6eee3e71) SHA1(0ef22732e0e2bb5207559decd43f90d1e338ad7b) )
	ROM_LOAD32_BYTE( "revx.66",  0x0c00003, 0x80000, CRC(b43d6fff) SHA1(87584e7aeea9d52a43023d40c359591ff6342e84) )

	ROM_LOAD32_BYTE( "revx.51",  0x0e00000, 0x80000, CRC(9960ac7c) SHA1(441322f061d627ca7573f612f370a85794681d0f) )
	ROM_LOAD32_BYTE( "revx.52",  0x0e00001, 0x80000, CRC(fbf55510) SHA1(8a5b0004ed09391fe37f0f501b979903d6ae4868) )
	ROM_LOAD32_BYTE( "revx.53",  0x0e00002, 0x80000, CRC(a045b265) SHA1(b294d3a56e41f5ec4ab9bbcc0088833b1cab1879) )
	ROM_LOAD32_BYTE( "revx.54",  0x0e00003, 0x80000, CRC(24471269) SHA1(262345bd147402100785459af422dafd1c562787) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "a-17722.u1",   0x000, 0x117, CRC(054de7a3) SHA1(bb7abaec50ed704c03b44d5d54296898f7c80d38) )
	ROM_LOAD( "a-17721.u955", 0x200, 0x117, CRC(033fe902) SHA1(6efb4e519ed3c9d49fff046a679762b506b3a75b) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, revx,   0,         midxunit, revx, midxunit_state, revx, ROT0, "Midway",   "Revolution X (Rev. 1.0 6/16/94)", MACHINE_SUPPORTS_SAVE )
