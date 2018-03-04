// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
/***************************************************************************

Glass (c) 1993 Gaelco (Developed by OMK. Produced by Gaelco)

Driver by Manuel Abadia <emumanu+mame@gmail.com>

Todo:
 - video priorities are incorrect, like most earlier Gaelco titles in MAME

***************************************************************************/

#include "emu.h"
#include "includes/glass.h"

#include "machine/gaelco_ds5002fp.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"


WRITE8_MEMBER(glass_state::shareram_w)
{
	// why isn't there an AM_SOMETHING macro for this?
	reinterpret_cast<u8 *>(m_shareram.target())[BYTE_XOR_BE(offset)] = data;
}

READ8_MEMBER(glass_state::shareram_r)
{
	// why isn't there an AM_SOMETHING macro for this?
	return reinterpret_cast<u8 const *>(m_shareram.target())[BYTE_XOR_BE(offset)];
}


WRITE16_MEMBER(glass_state::clr_int_w)
{
	m_cause_interrupt = 1;
}

INTERRUPT_GEN_MEMBER(glass_state::interrupt)
{
	if (m_cause_interrupt)
	{
		device.execute().set_input_line(6, HOLD_LINE);
		m_cause_interrupt = 0;
	}
}


static const gfx_layout glass_tilelayout16 =
{
	16,16,                                  /* 16x16 tiles */
	0x100000/32,                            /* number of tiles */
	4,                                      /* 4 bpp */
	{ 3*0x100000*8, 2*0x100000*8, 1*0x100000*8, 0*0x100000*8 },
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7
	},
	{
		0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8
	},
	32*8
};

static GFXDECODE_START( glass )
	GFXDECODE_ENTRY( "gfx1", 0x000000, glass_tilelayout16, 0, 64 )
GFXDECODE_END


WRITE16_MEMBER(glass_state::OKIM6295_bankswitch_w)
{
	if (ACCESSING_BITS_0_7)
		membank("okibank")->set_entry(data & 0x0f);
}

WRITE16_MEMBER(glass_state::coin_w)
{
	m_outlatch->write_bit(offset >> 3, BIT(data, 0));
}

WRITE_LINE_MEMBER(glass_state::coin1_lockout_w)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
}

WRITE_LINE_MEMBER(glass_state::coin2_lockout_w)
{
	machine().bookkeeping().coin_lockout_w(1, !state);
}

WRITE_LINE_MEMBER(glass_state::coin1_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(glass_state::coin2_counter_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


ADDRESS_MAP_START(glass_state::mcu_hostmem_map)
	AM_RANGE(0x0000, 0xffff) AM_MASK(0x3fff) AM_READWRITE(shareram_r, shareram_w) // shared RAM with the main CPU
ADDRESS_MAP_END


ADDRESS_MAP_START(glass_state::glass_map)
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                                   // ROM
	AM_RANGE(0x100000, 0x101fff) AM_RAM_WRITE(vram_w) AM_SHARE("videoram")                                // Video RAM
	AM_RANGE(0x102000, 0x102fff) AM_RAM                                                                   // Extra Video RAM
	AM_RANGE(0x108000, 0x108007) AM_WRITEONLY AM_SHARE("vregs")                                           // Video Registers
	AM_RANGE(0x108008, 0x108009) AM_WRITE(clr_int_w)                                                      // CLR INT Video
	AM_RANGE(0x200000, 0x2007ff) AM_RAM_DEVWRITE("palette", palette_device, write16) AM_SHARE("palette")    // Palette
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")                                             // Sprite RAM
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW2")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSW1")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("P1")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("P2")
	AM_RANGE(0x700008, 0x700009) AM_WRITE(blitter_w)                                                      // serial blitter
	AM_RANGE(0x70000a, 0x70000b) AM_SELECT(0x000070) AM_WRITE(coin_w)                                     // Coin Counters/Lockout
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(OKIM6295_bankswitch_w)                                          // OKI6295 bankswitch
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)            // OKI6295 status register
	AM_RANGE(0xfec000, 0xfeffff) AM_RAM AM_SHARE("shareram")                                              // Work RAM (partially shared with DS5002FP)
ADDRESS_MAP_END


ADDRESS_MAP_START(glass_state::oki_map)
	AM_RANGE(0x00000, 0x2ffff) AM_ROM
	AM_RANGE(0x30000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END


static INPUT_PORTS_START( glass )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, "Credit configuration" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "Start 1C" )
	PORT_DIPSETTING(    0x00, "Start 2C" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Version ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Light" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:2" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:1" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



void glass_state::machine_start()
{
	membank("okibank")->configure_entries(0, 16, memregion("oki")->base(), 0x10000);

	save_item(NAME(m_cause_interrupt));
	save_item(NAME(m_current_bit));
	save_item(NAME(m_current_command));
	save_item(NAME(m_blitter_serial_buffer));
}

void glass_state::machine_reset()
{
	m_cause_interrupt = 1;
	m_current_bit = 0;
	m_current_command = 0;

	for (int i = 0; i < 5; i++)
		m_blitter_serial_buffer[i] = 0;
}

MACHINE_CONFIG_START(glass_state::glass)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL(24'000'000)/2)      /* 12 MHz verified on PCB */
	MCFG_CPU_PROGRAM_MAP(glass_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", glass_state,  interrupt)

	MCFG_DEVICE_ADD("outlatch", LS259, 0)
	MCFG_ADDRESSABLE_LATCH_Q0_OUT_CB(WRITELINE(glass_state, coin1_lockout_w))
	MCFG_ADDRESSABLE_LATCH_Q1_OUT_CB(WRITELINE(glass_state, coin2_lockout_w))
	MCFG_ADDRESSABLE_LATCH_Q2_OUT_CB(WRITELINE(glass_state, coin1_counter_w))
	MCFG_ADDRESSABLE_LATCH_Q3_OUT_CB(WRITELINE(glass_state, coin2_counter_w))
	MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(NOOP) // Sound Muting (if bit 0 == 1, sound output stream = 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 368-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(glass_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", glass)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL(1'000'000), PIN7_HIGH) /* 1MHz Resonator & pin 7 high verified on PCB */
	MCFG_DEVICE_ADDRESS_MAP(0, oki_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(glass_state::glass_ds5002fp)
	glass(config);
	MCFG_DEVICE_ADD("gaelco_ds5002fp", GAELCO_DS5002FP, XTAL(24'000'000) / 2) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(0, mcu_hostmem_map)
MACHINE_CONFIG_END

ROM_START( glass ) /* Version 1.1 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "1.c23", 0x000000, 0x040000, CRC(aeebd4ed) SHA1(04759dc146dff0fc74b78d70e79dfaebe68328f9) )
	ROM_LOAD16_BYTE( "2.c22", 0x000001, 0x040000, CRC(165e2e01) SHA1(180a2e2b5151f2321d85ac23eff7fbc9f52023a5) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )   /* Graphics */
	/* 0x000000-0x3fffff filled in later in the DRIVER_INIT */

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Graphics */
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* 16 bitmaps (320x200, indexed colors) */
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
ROM_END

ROM_START( glass10 ) /* Version 1.0 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "c23.bin", 0x000000, 0x040000, CRC(688cdf33) SHA1(b59dcc3fc15f72037692b745927b110e97d8282e) )
	ROM_LOAD16_BYTE( "c22.bin", 0x000001, 0x040000, CRC(ab17c992) SHA1(1509b5b4bbfb4e022e0ab6fbbc0ffc070adfa531) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )   /* Graphics */
	/* 0x000000-0x3fffff filled in later in the DRIVER_INIT */

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Graphics */
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* 16 bitmaps (320x200, indexed colors) */
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
ROM_END

ROM_START( glass10a ) /* Title screen shows "GLASS" and under that "Break Edition" on a real PCB */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "spl-c23.bin", 0x000000, 0x040000, CRC(c1393bea) SHA1(a5f877ba38305a7b49fa3c96b9344cbf71e8c9ef) )
	ROM_LOAD16_BYTE( "spl-c22.bin", 0x000001, 0x040000, CRC(0d6fa33e) SHA1(37e9258ef7e108d034c80abc8e5e5ab6dacf0a61) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )   /* Graphics */
	/* 0x000000-0x3fffff filled in later in the DRIVER_INIT */

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Graphics */
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* 16 bitmaps (320x200, indexed colors) */
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
ROM_END

ROM_START( glasskr )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "glassk.c23", 0x000000, 0x080000, CRC(6ee19376) SHA1(8a8fdeebe094bd3e29c35cf59584e3cab708732d) )
	ROM_LOAD16_BYTE( "glassk.c22", 0x000001, 0x080000, CRC(bd546568) SHA1(bcd5e7591f4e68c9470999b8a0ef1ee4392c907c) )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_ERASE00 )   /* Graphics */
	/* 0x000000-0x3fffff filled in later in the DRIVER_INIT */

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Graphics */
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* 16 bitmaps (320x200, indexed colors) */
	ROM_LOAD( "glassk.h9", 0x000000, 0x100000, CRC(d499be4c) SHA1(204f754813be687e8dc00bfe7b5dbc4857ac8738) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
ROM_END

/***************************************************************************

    Split even/odd bytes from ROMs in 16 bit mode to different memory areas

***************************************************************************/

void glass_state::ROM16_split_gfx( const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2 )
{
	int i;

	/* get a pointer to the source data */
	uint8_t *src = (uint8_t *)memregion(src_reg)->base();

	/* get a pointer to the destination data */
	uint8_t *dst = (uint8_t *)memregion(dst_reg)->base();

	/* fill destination areas with the proper data */
	for (i = 0; i < length / 2; i++)
	{
		dst[dest1 + i] = src[start + i * 2 + 0];
		dst[dest2 + i] = src[start + i * 2 + 1];
	}
}


DRIVER_INIT_MEMBER(glass_state, glass)
{
	/*
	For "gfx2" we have this memory map:
	    0x000000-0x1fffff ROM H13
	    0x200000-0x3fffff ROM H11

	and we are going to construct this one for "gfx1":
	    0x000000-0x0fffff ROM H13 even bytes
	    0x100000-0x1fffff ROM H13 odd bytes
	    0x200000-0x2fffff ROM H11 even bytes
	    0x300000-0x3fffff ROM H11 odd bytes
	*/

	/* split ROM H13 */
	ROM16_split_gfx("gfx2", "gfx1", 0x0000000, 0x0200000, 0x0000000, 0x0100000);

	/* split ROM H11 */
	ROM16_split_gfx("gfx2", "gfx1", 0x0200000, 0x0200000, 0x0200000, 0x0300000);

}


/*
 ALL versions of Glass contain the 'Break Edition' string (it just seems to be part of the title?)
 The 2 version 1.0 releases are very similar code, it was thought that one was a break edition and the other wasn't, but this is not the case.

 Version 1.1 releases also show Version 1994 on the title screen.  These versions do not have skulls in the playfield (at least not on early stages)
 The protected version 1.1 also doesn't show any kind of attract gameplay, looks like it was patched out? (should be verified on an untouched original 1.1 using it's original SRAM tho)
 The unprotected version appears to be a Korean set, is censored, and has different girl pictures.
*/

GAME( 1994, glass,    0,     glass_ds5002fp, glass, glass_state, glass, ROT0, "OMK / Gaelco",                  "Glass (Ver 1.1, Break Edition, Checksum 49D5E66B, Version 1994)",                           MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, glasskr,  glass, glass,          glass, glass_state, glass, ROT0, "OMK / Gaelco (Promat license)", "Glass (Ver 1.1, Break Edition, Checksum D419AB69, Version 1994) (censored, unprotected)",   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // promat stickers on program roms
GAME( 1993, glass10,  glass, glass_ds5002fp, glass, glass_state, glass, ROT0, "OMK / Gaelco",                  "Glass (Ver 1.0, Break Edition, Checksum C5513F3C)",                                 MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, glass10a, glass, glass_ds5002fp, glass, glass_state, glass, ROT0, "OMK / Gaelco",                  "Glass (Ver 1.0, Break Edition, Checksum D3864FDB)",                                 MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
