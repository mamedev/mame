// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Nicola Salmoria
/******************************************************************

Volfied (c) 1989 Taito Corporation
==================================

    Original driver from RAINE

    68000 (8MHz) + Z80 (4MHz) + YM-2203 (4MHz) + C-Chip

    VIDEO RAM: 12 * MB-81461 (256k VRAM)

    Customs:
        TC0070RGB   - Colour output
        PC050CM     - Colour output
        TC0030CMD   - Protection (labeled C04 23)
        PC060HA     - Audio
        PC090OJ     - Sprites

    OSC: 32MHz, 26.686MHz & 20MHz

TC0030CMD is a custom Z80 with embedded 8K ram + 8k rom (20MHz OSC is next to chip, 20MHz/4 = 5MHz(?))

Stephh's notes (based on the game M68000 code and some tests) :

1) 'volfied*'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'volfied'  : region = 0x0003
      * 'volfiedj' : region = 0x0001
      * 'volfiedu' : region = 0x0002
  - These 3 games are 100% the same, only region differs !
  - Coinage relies on the region (code at 0x00666a) :
      * 0x0001 (Japan) uses TAITO_COINAGE_JAPAN_OLD
      * 0x0002 (US) uses slighlty different TAITO_COINAGE_US :
        in fact, as there is no possibility to continue a game,
        what are used to be "Continue Price" Dip Switches are unused
      * 0x0003 (World) uses TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0001
  - FBI logo only if region = 0x0002


********************************************************************/

/* Define clocks based on actual OSC on the PCB */

#define CPU_CLOCK           (XTAL_32MHz / 4)        /* 8 MHz clock for 68000 */
#define SOUND_CPU_CLOCK     (XTAL_32MHz / 8)        /* 4 MHz clock for Z80 sound CPU */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/taitoipt.h"
#include "audio/taitosnd.h"
#include "sound/2203intf.h"
#include "includes/volfied.h"


/***********************************************************
                MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, volfied_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM     /* program */
	AM_RANGE(0x080000, 0x0fffff) AM_ROM     /* tiles   */
	AM_RANGE(0x100000, 0x103fff) AM_RAM     /* main    */
	AM_RANGE(0x200000, 0x203fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)
	AM_RANGE(0x400000, 0x47ffff) AM_READWRITE(volfied_video_ram_r, volfied_video_ram_w)
	AM_RANGE(0x500000, 0x503fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x600000, 0x600001) AM_WRITE(volfied_video_mask_w)
	AM_RANGE(0x700000, 0x700001) AM_WRITE(volfied_sprite_ctrl_w)
	AM_RANGE(0xd00000, 0xd00001) AM_READWRITE(volfied_video_ctrl_r, volfied_video_ctrl_w)
	AM_RANGE(0xe00000, 0xe00001) AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0xe00002, 0xe00003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0xf00000, 0xf007ff) AM_READWRITE(volfied_cchip_ram_r, volfied_cchip_ram_w)
	AM_RANGE(0xf00802, 0xf00803) AM_READWRITE(volfied_cchip_ctrl_r, volfied_cchip_ctrl_w)
	AM_RANGE(0xf00c00, 0xf00c01) AM_WRITE(volfied_cchip_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_map, AS_PROGRAM, 8, volfied_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8800) AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0x8801, 0x8801) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x9800, 0x9800) AM_WRITENOP    /* ? */
ADDRESS_MAP_END


/***********************************************************
                INPUT PORTS
***********************************************************/

static INPUT_PORTS_START( volfied )
	/* Z80 CPU -> 0x10002c ($2c,A5) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	/* Z80 CPU -> 0x10002e ($2e,A5) */
	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:1,2") /* table at 0x003140 - 4 * 6 words - LSB first */
	PORT_DIPSETTING(    0x02, "20k 40k 120k 480k 2400k" )
	PORT_DIPSETTING(    0x03, "50k 150k 600k 3000k" )
	PORT_DIPSETTING(    0x01, "70k 280k 1400k" )
	PORT_DIPSETTING(    0x00, "100k 500k" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, "32768 Lives (Cheat)" )   PORT_DIPLOCATION("SW2:7")   /* code at 0x0015cc - Manual shows unused and OFF */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )

	PORT_START("F00007")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("F00009")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("F0000B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("F0000D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( volfiedu )
	PORT_INCLUDE(volfied)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_COIN_START_LOC(SW1)        /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )    /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )    /* see notes */
INPUT_PORTS_END

static INPUT_PORTS_START( volfiedj )
	PORT_INCLUDE(volfied)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END


/**************************************************************
                GFX DECODING
**************************************************************/

static const gfx_layout tilelayout =
{
	16, 16,
	0x1800,
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 8*4, 9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( volfied )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 4096, 256 )
GFXDECODE_END


/**************************************************************
                YM2203 (SOUND)
**************************************************************/

/* handler called by the YM2203 emulator when the internal timers cause an IRQ */

WRITE_LINE_MEMBER(volfied_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

/***********************************************************
                MACHINE DRIVERS
***********************************************************/

void volfied_state::machine_start()
{
	volfied_cchip_init();
}

void volfied_state::machine_reset()
{
	volfied_cchip_reset();
}

static MACHINE_CONFIG_START( volfied, volfied_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, CPU_CLOCK)   /* 8MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", volfied_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CPU_CLOCK)   /* 4MHz sound CPU, required to run the game */
	MCFG_CPU_PROGRAM_MAP(z80_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(1200))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 8, 247)
	MCFG_SCREEN_UPDATE_DRIVER(volfied_state, screen_update_volfied)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", volfied)
	MCFG_PALETTE_ADD("palette", 8192)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 4000000)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(volfied_state, irqhandler))
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWB"))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.60)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( volfied )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code and tile data */
	ROM_LOAD16_BYTE( "c04-12-1.30", 0x00000, 0x10000, CRC(afb6a058) SHA1(fca488e86725a0a673332afeb0002f0e77ef2dbf) )
	ROM_LOAD16_BYTE( "c04-08-1.10", 0x00001, 0x10000, CRC(19f7e66b) SHA1(51b5d0d00ec398ed717154286bec24b05c3f81b8) )
	ROM_LOAD16_BYTE( "c04-11-1.29", 0x20000, 0x10000, CRC(1aaf6e9b) SHA1(4be643283dc78eb57e9fe4c5afebdc427e4354e8) )
	ROM_LOAD16_BYTE( "c04-25-1.9",  0x20001, 0x10000, CRC(b39e04f9) SHA1(7ae2cfbea30bc510e3ed6d2de8281bdfb0d75182) )
	ROM_LOAD16_BYTE( "c04-20.7",    0x80000, 0x20000, CRC(0aea651f) SHA1(a438a37ec9dc764c841561608924da158ddde66f) )
	ROM_LOAD16_BYTE( "c04-22.9",    0x80001, 0x20000, CRC(f405d465) SHA1(67f6a4baf640dc74d9534ffda790f76677e944e8) )
	ROM_LOAD16_BYTE( "c04-19.6",    0xc0000, 0x20000, CRC(231493ae) SHA1(2658e6556fd0e75ddd0f0b8628cfa5237c187a06) )
	ROM_LOAD16_BYTE( "c04-21.8",    0xc0001, 0x20000, CRC(8598d38e) SHA1(4ec1b819586b50e2f6aff2aaa5e3b06704b9bec2) )

	ROM_REGION( 0xc0000, "gfx1", 0 )    /* sprites 16x16 */
	ROM_LOAD16_BYTE( "c04-16.2",   0x00000, 0x20000, CRC(8c2476ef) SHA1(972ddc8e47a669f1aeca67d02b4a0bed867ddb7d) )
	ROM_LOAD16_BYTE( "c04-18.4",   0x00001, 0x20000, CRC(7665212c) SHA1(b816ac2a95ee273aaf90991f53766d7f0d5d9238) )
	ROM_LOAD16_BYTE( "c04-15.1",   0x40000, 0x20000, CRC(7c50b978) SHA1(aa9cad5f09f5d9dceaf4e06bcd347f1d5d02d292) )
	ROM_LOAD16_BYTE( "c04-17.3",   0x40001, 0x20000, CRC(c62fdeb8) SHA1(a9f6ca8335071169d772e65a9f5315a22a310b25) )
	ROM_LOAD16_BYTE( "c04-10.15",  0x80000, 0x10000, CRC(429b6b49) SHA1(dcb0c8bc9d67643d96b2ffdf5ccd747318704c37) )
	ROM_RELOAD     (               0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "c04-09.14",  0x80001, 0x10000, CRC(c78cf057) SHA1(097982e57b1d20fbdf21986c23684adefe6f1ce1) )
	ROM_RELOAD     (               0xa0001, 0x10000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "c04-06.71", 0x0000, 0x8000, CRC(b70106b2) SHA1(d71062f9d9b11492e13fc93982b95883f564f902) )

	ROM_REGION( 0x00400, "proms", 0 )   /* unused PROMs */
	ROM_LOAD( "c04-4-1.3", 0x00000, 0x00200, CRC(ab9fae65) SHA1(e2b29606aa63e42e041d3c47216551f62846bd99) ) /* PROM type is a MB7116H or compatible */
	ROM_LOAD( "c04-5.75",  0x00200, 0x00200, CRC(2763ec89) SHA1(1e8339e21ee35b526d8604a21cfed9a1ac6455e8) ) /* PROM type is a MB7124E or compatible */
ROM_END

ROM_START( volfiedo )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code and tile data */
	ROM_LOAD16_BYTE( "c04-12.30", 0x00000, 0x10000, CRC(e319c7ec) SHA1(e76fb872191fce0186ed0ac5066385a9913fdc4c) )
	ROM_LOAD16_BYTE( "c04-08.10", 0x00001, 0x10000, CRC(81c6f755) SHA1(43ad72bb05d847f58b3043c674fb9b1e317691df) )
	ROM_LOAD16_BYTE( "c04-11.29", 0x20000, 0x10000, CRC(f05696a6) SHA1(8514e5751e2f11840379e8cc6883a23cf1b3a4eb) )
	ROM_LOAD16_BYTE( "c04-25.9",  0x20001, 0x10000, CRC(a0e3c0a8) SHA1(716b43d2f38ef50f85f9044d403444695ca48456) )
	ROM_LOAD16_BYTE( "c04-20.7",  0x80000, 0x20000, CRC(0aea651f) SHA1(a438a37ec9dc764c841561608924da158ddde66f) )
	ROM_LOAD16_BYTE( "c04-22.9",  0x80001, 0x20000, CRC(f405d465) SHA1(67f6a4baf640dc74d9534ffda790f76677e944e8) )
	ROM_LOAD16_BYTE( "c04-19.6",  0xc0000, 0x20000, CRC(231493ae) SHA1(2658e6556fd0e75ddd0f0b8628cfa5237c187a06) )
	ROM_LOAD16_BYTE( "c04-21.8",  0xc0001, 0x20000, CRC(8598d38e) SHA1(4ec1b819586b50e2f6aff2aaa5e3b06704b9bec2) )

	ROM_REGION( 0xc0000, "gfx1", 0 )    /* sprites 16x16 */
	ROM_LOAD16_BYTE( "c04-16.2",  0x00000, 0x20000, CRC(8c2476ef) SHA1(972ddc8e47a669f1aeca67d02b4a0bed867ddb7d) )
	ROM_LOAD16_BYTE( "c04-18.4",  0x00001, 0x20000, CRC(7665212c) SHA1(b816ac2a95ee273aaf90991f53766d7f0d5d9238) )
	ROM_LOAD16_BYTE( "c04-15.1",  0x40000, 0x20000, CRC(7c50b978) SHA1(aa9cad5f09f5d9dceaf4e06bcd347f1d5d02d292) )
	ROM_LOAD16_BYTE( "c04-17.3",  0x40001, 0x20000, CRC(c62fdeb8) SHA1(a9f6ca8335071169d772e65a9f5315a22a310b25) )
	ROM_LOAD16_BYTE( "c04-10.15", 0x80000, 0x10000, CRC(429b6b49) SHA1(dcb0c8bc9d67643d96b2ffdf5ccd747318704c37) )
	ROM_RELOAD     (              0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "c04-09.14", 0x80001, 0x10000, CRC(c78cf057) SHA1(097982e57b1d20fbdf21986c23684adefe6f1ce1) )
	ROM_RELOAD     (              0xa0001, 0x10000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "c04-06.71", 0x0000, 0x8000, CRC(b70106b2) SHA1(d71062f9d9b11492e13fc93982b95883f564f902) )

	ROM_REGION( 0x00400, "proms", 0 )   /* unused PROMs */
	ROM_LOAD( "c04-4-1.3", 0x00000, 0x00200, CRC(ab9fae65) SHA1(e2b29606aa63e42e041d3c47216551f62846bd99) ) /* PROM type is a MB7116H or compatible */
	ROM_LOAD( "c04-5.75",  0x00200, 0x00200, CRC(2763ec89) SHA1(1e8339e21ee35b526d8604a21cfed9a1ac6455e8) ) /* PROM type is a MB7124E or compatible */
ROM_END

ROM_START( volfiedu )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code and tile data */
	ROM_LOAD16_BYTE( "c04-12-1.30", 0x00000, 0x10000, CRC(afb6a058) SHA1(fca488e86725a0a673332afeb0002f0e77ef2dbf) )
	ROM_LOAD16_BYTE( "c04-08-1.10", 0x00001, 0x10000, CRC(19f7e66b) SHA1(51b5d0d00ec398ed717154286bec24b05c3f81b8) )
	ROM_LOAD16_BYTE( "c04-11-1.29", 0x20000, 0x10000, CRC(1aaf6e9b) SHA1(4be643283dc78eb57e9fe4c5afebdc427e4354e8) )
	ROM_LOAD16_BYTE( "c04-24-1.9",  0x20001, 0x10000, CRC(c499346f) SHA1(f039b36050e6091929c44ab22e03af3d66d41eaf) )
	ROM_LOAD16_BYTE( "c04-20.7",    0x80000, 0x20000, CRC(0aea651f) SHA1(a438a37ec9dc764c841561608924da158ddde66f) )
	ROM_LOAD16_BYTE( "c04-22.9",    0x80001, 0x20000, CRC(f405d465) SHA1(67f6a4baf640dc74d9534ffda790f76677e944e8) )
	ROM_LOAD16_BYTE( "c04-19.6",    0xc0000, 0x20000, CRC(231493ae) SHA1(2658e6556fd0e75ddd0f0b8628cfa5237c187a06) )
	ROM_LOAD16_BYTE( "c04-21.8",    0xc0001, 0x20000, CRC(8598d38e) SHA1(4ec1b819586b50e2f6aff2aaa5e3b06704b9bec2) )

	ROM_REGION( 0xc0000, "gfx1", 0 )    /* sprites 16x16 */
	ROM_LOAD16_BYTE( "c04-16.2",   0x00000, 0x20000, CRC(8c2476ef) SHA1(972ddc8e47a669f1aeca67d02b4a0bed867ddb7d) )
	ROM_LOAD16_BYTE( "c04-18.4",   0x00001, 0x20000, CRC(7665212c) SHA1(b816ac2a95ee273aaf90991f53766d7f0d5d9238) )
	ROM_LOAD16_BYTE( "c04-15.1",   0x40000, 0x20000, CRC(7c50b978) SHA1(aa9cad5f09f5d9dceaf4e06bcd347f1d5d02d292) )
	ROM_LOAD16_BYTE( "c04-17.3",   0x40001, 0x20000, CRC(c62fdeb8) SHA1(a9f6ca8335071169d772e65a9f5315a22a310b25) )
	ROM_LOAD16_BYTE( "c04-10.15",  0x80000, 0x10000, CRC(429b6b49) SHA1(dcb0c8bc9d67643d96b2ffdf5ccd747318704c37) )
	ROM_RELOAD     (               0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "c04-09.14",  0x80001, 0x10000, CRC(c78cf057) SHA1(097982e57b1d20fbdf21986c23684adefe6f1ce1) )
	ROM_RELOAD     (               0xa0001, 0x10000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "c04-06.71", 0x0000, 0x8000, CRC(b70106b2) SHA1(d71062f9d9b11492e13fc93982b95883f564f902) )

	ROM_REGION( 0x00400, "proms", 0 )   /* unused PROMs */
	ROM_LOAD( "c04-4-1.3", 0x00000, 0x00200, CRC(ab9fae65) SHA1(e2b29606aa63e42e041d3c47216551f62846bd99) ) /* PROM type is a MB7116H or compatible */
	ROM_LOAD( "c04-5.75",  0x00200, 0x00200, CRC(2763ec89) SHA1(1e8339e21ee35b526d8604a21cfed9a1ac6455e8) ) /* PROM type is a MB7124E or compatible */
ROM_END

ROM_START( volfieduo )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code and tile data */
	ROM_LOAD16_BYTE( "c04-12.30", 0x00000, 0x10000, CRC(e319c7ec) SHA1(e76fb872191fce0186ed0ac5066385a9913fdc4c) )
	ROM_LOAD16_BYTE( "c04-08.10", 0x00001, 0x10000, CRC(81c6f755) SHA1(43ad72bb05d847f58b3043c674fb9b1e317691df) )
	ROM_LOAD16_BYTE( "c04-11.29", 0x20000, 0x10000, CRC(f05696a6) SHA1(8514e5751e2f11840379e8cc6883a23cf1b3a4eb) )
	ROM_LOAD16_BYTE( "c04-24.9",  0x20001, 0x10000, CRC(d7e4f03e) SHA1(acb643f0c7f5d250ab8d1b3dbbe08cdf15136fc6) )
	ROM_LOAD16_BYTE( "c04-20.7",  0x80000, 0x20000, CRC(0aea651f) SHA1(a438a37ec9dc764c841561608924da158ddde66f) )
	ROM_LOAD16_BYTE( "c04-22.9",  0x80001, 0x20000, CRC(f405d465) SHA1(67f6a4baf640dc74d9534ffda790f76677e944e8) )
	ROM_LOAD16_BYTE( "c04-19.6",  0xc0000, 0x20000, CRC(231493ae) SHA1(2658e6556fd0e75ddd0f0b8628cfa5237c187a06) )
	ROM_LOAD16_BYTE( "c04-21.8",  0xc0001, 0x20000, CRC(8598d38e) SHA1(4ec1b819586b50e2f6aff2aaa5e3b06704b9bec2) )

	ROM_REGION( 0xc0000, "gfx1", 0 )    /* sprites 16x16 */
	ROM_LOAD16_BYTE( "c04-16.2",  0x00000, 0x20000, CRC(8c2476ef) SHA1(972ddc8e47a669f1aeca67d02b4a0bed867ddb7d) )
	ROM_LOAD16_BYTE( "c04-18.4",  0x00001, 0x20000, CRC(7665212c) SHA1(b816ac2a95ee273aaf90991f53766d7f0d5d9238) )
	ROM_LOAD16_BYTE( "c04-15.1",  0x40000, 0x20000, CRC(7c50b978) SHA1(aa9cad5f09f5d9dceaf4e06bcd347f1d5d02d292) )
	ROM_LOAD16_BYTE( "c04-17.3",  0x40001, 0x20000, CRC(c62fdeb8) SHA1(a9f6ca8335071169d772e65a9f5315a22a310b25) )
	ROM_LOAD16_BYTE( "c04-10.15", 0x80000, 0x10000, CRC(429b6b49) SHA1(dcb0c8bc9d67643d96b2ffdf5ccd747318704c37) )
	ROM_RELOAD     (              0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "c04-09.14", 0x80001, 0x10000, CRC(c78cf057) SHA1(097982e57b1d20fbdf21986c23684adefe6f1ce1) )
	ROM_RELOAD     (              0xa0001, 0x10000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "c04-06.71", 0x0000, 0x8000, CRC(b70106b2) SHA1(d71062f9d9b11492e13fc93982b95883f564f902) )

	ROM_REGION( 0x00400, "proms", 0 )   /* unused PROMs */
	ROM_LOAD( "c04-4-1.3", 0x00000, 0x00200, CRC(ab9fae65) SHA1(e2b29606aa63e42e041d3c47216551f62846bd99) ) /* PROM type is a MB7116H or compatible */
	ROM_LOAD( "c04-5.75",  0x00200, 0x00200, CRC(2763ec89) SHA1(1e8339e21ee35b526d8604a21cfed9a1ac6455e8) ) /* PROM type is a MB7124E or compatible */
ROM_END

ROM_START( volfiedj )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code and tile data */
	ROM_LOAD16_BYTE( "c04-12-1.30", 0x00000, 0x10000, CRC(afb6a058) SHA1(fca488e86725a0a673332afeb0002f0e77ef2dbf) )
	ROM_LOAD16_BYTE( "c04-08-1.10", 0x00001, 0x10000, CRC(19f7e66b) SHA1(51b5d0d00ec398ed717154286bec24b05c3f81b8) )
	ROM_LOAD16_BYTE( "c04-11-1.29", 0x20000, 0x10000, CRC(1aaf6e9b) SHA1(4be643283dc78eb57e9fe4c5afebdc427e4354e8) )
	ROM_LOAD16_BYTE( "c04-07-1.9",  0x20001, 0x10000, CRC(5d9065d5) SHA1(5682c92da14a736f76b5b6b3870571743cdde211) )
	ROM_LOAD16_BYTE( "c04-20.7",    0x80000, 0x20000, CRC(0aea651f) SHA1(a438a37ec9dc764c841561608924da158ddde66f) )
	ROM_LOAD16_BYTE( "c04-22.9",    0x80001, 0x20000, CRC(f405d465) SHA1(67f6a4baf640dc74d9534ffda790f76677e944e8) )
	ROM_LOAD16_BYTE( "c04-19.6",    0xc0000, 0x20000, CRC(231493ae) SHA1(2658e6556fd0e75ddd0f0b8628cfa5237c187a06) )
	ROM_LOAD16_BYTE( "c04-21.8",    0xc0001, 0x20000, CRC(8598d38e) SHA1(4ec1b819586b50e2f6aff2aaa5e3b06704b9bec2) )

	ROM_REGION( 0xc0000, "gfx1", 0 )    /* sprites 16x16 */
	ROM_LOAD16_BYTE( "c04-16.2",   0x00000, 0x20000, CRC(8c2476ef) SHA1(972ddc8e47a669f1aeca67d02b4a0bed867ddb7d) )
	ROM_LOAD16_BYTE( "c04-18.4",   0x00001, 0x20000, CRC(7665212c) SHA1(b816ac2a95ee273aaf90991f53766d7f0d5d9238) )
	ROM_LOAD16_BYTE( "c04-15.1",   0x40000, 0x20000, CRC(7c50b978) SHA1(aa9cad5f09f5d9dceaf4e06bcd347f1d5d02d292) )
	ROM_LOAD16_BYTE( "c04-17.3",   0x40001, 0x20000, CRC(c62fdeb8) SHA1(a9f6ca8335071169d772e65a9f5315a22a310b25) )
	ROM_LOAD16_BYTE( "c04-10.15",  0x80000, 0x10000, CRC(429b6b49) SHA1(dcb0c8bc9d67643d96b2ffdf5ccd747318704c37) )
	ROM_RELOAD     (               0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "c04-09.14",  0x80001, 0x10000, CRC(c78cf057) SHA1(097982e57b1d20fbdf21986c23684adefe6f1ce1) )
	ROM_RELOAD     (               0xa0001, 0x10000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "c04-06.71", 0x0000, 0x8000, CRC(b70106b2) SHA1(d71062f9d9b11492e13fc93982b95883f564f902) )

	ROM_REGION( 0x00400, "proms", 0 )   /* unused PROMs */
	ROM_LOAD( "c04-4-1.3", 0x00000, 0x00200, CRC(ab9fae65) SHA1(e2b29606aa63e42e041d3c47216551f62846bd99) ) /* PROM type is a MB7116H or compatible */
	ROM_LOAD( "c04-5.75",  0x00200, 0x00200, CRC(2763ec89) SHA1(1e8339e21ee35b526d8604a21cfed9a1ac6455e8) ) /* PROM type is a MB7124E or compatible */
ROM_END

ROM_START( volfiedjo )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code and tile data */
	ROM_LOAD16_BYTE( "c04-12.30", 0x00000, 0x10000, CRC(e319c7ec) SHA1(e76fb872191fce0186ed0ac5066385a9913fdc4c) )
	ROM_LOAD16_BYTE( "c04-08.10", 0x00001, 0x10000, CRC(81c6f755) SHA1(43ad72bb05d847f58b3043c674fb9b1e317691df) )
	ROM_LOAD16_BYTE( "c04-11.29", 0x20000, 0x10000, CRC(f05696a6) SHA1(8514e5751e2f11840379e8cc6883a23cf1b3a4eb) )
	ROM_LOAD16_BYTE( "c04-07.9",  0x20001, 0x10000, CRC(4eeda184) SHA1(8d6bb20bd75ad17199d8ebb319849842f9106e90) )
	ROM_LOAD16_BYTE( "c04-20.7",  0x80000, 0x20000, CRC(0aea651f) SHA1(a438a37ec9dc764c841561608924da158ddde66f) )
	ROM_LOAD16_BYTE( "c04-22.9",  0x80001, 0x20000, CRC(f405d465) SHA1(67f6a4baf640dc74d9534ffda790f76677e944e8) )
	ROM_LOAD16_BYTE( "c04-19.6",  0xc0000, 0x20000, CRC(231493ae) SHA1(2658e6556fd0e75ddd0f0b8628cfa5237c187a06) )
	ROM_LOAD16_BYTE( "c04-21.8",  0xc0001, 0x20000, CRC(8598d38e) SHA1(4ec1b819586b50e2f6aff2aaa5e3b06704b9bec2) )

	ROM_REGION( 0xc0000, "gfx1", 0 )    /* sprites 16x16 */
	ROM_LOAD16_BYTE( "c04-16.2",  0x00000, 0x20000, CRC(8c2476ef) SHA1(972ddc8e47a669f1aeca67d02b4a0bed867ddb7d) )
	ROM_LOAD16_BYTE( "c04-18.4",  0x00001, 0x20000, CRC(7665212c) SHA1(b816ac2a95ee273aaf90991f53766d7f0d5d9238) )
	ROM_LOAD16_BYTE( "c04-15.1",  0x40000, 0x20000, CRC(7c50b978) SHA1(aa9cad5f09f5d9dceaf4e06bcd347f1d5d02d292) )
	ROM_LOAD16_BYTE( "c04-17.3",  0x40001, 0x20000, CRC(c62fdeb8) SHA1(a9f6ca8335071169d772e65a9f5315a22a310b25) )
	ROM_LOAD16_BYTE( "c04-10.15", 0x80000, 0x10000, CRC(429b6b49) SHA1(dcb0c8bc9d67643d96b2ffdf5ccd747318704c37) )
	ROM_RELOAD     (              0xa0000, 0x10000 )
	ROM_LOAD16_BYTE( "c04-09.14", 0x80001, 0x10000, CRC(c78cf057) SHA1(097982e57b1d20fbdf21986c23684adefe6f1ce1) )
	ROM_RELOAD     (              0xa0001, 0x10000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "c04-06.71", 0x0000, 0x8000, CRC(b70106b2) SHA1(d71062f9d9b11492e13fc93982b95883f564f902) )

	ROM_REGION( 0x00400, "proms", 0 )   /* unused PROMs */
	ROM_LOAD( "c04-4-1.3", 0x00000, 0x00200, CRC(ab9fae65) SHA1(e2b29606aa63e42e041d3c47216551f62846bd99) ) /* PROM type is a MB7116H or compatible */
	ROM_LOAD( "c04-5.75",  0x00200, 0x00200, CRC(2763ec89) SHA1(1e8339e21ee35b526d8604a21cfed9a1ac6455e8) ) /* PROM type is a MB7124E or compatible */
ROM_END


GAME( 1989, volfied,   0,       volfied, volfied, driver_device,  0, ROT270, "Taito Corporation Japan",   "Volfied (World, revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, volfiedu,  volfied, volfied, volfiedu, driver_device, 0, ROT270, "Taito America Corporation", "Volfied (US, revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, volfiedj,  volfied, volfied, volfiedj, driver_device, 0, ROT270, "Taito Corporation",         "Volfied (Japan, revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, volfiedo,  volfied, volfied, volfiedj, driver_device, 0, ROT270, "Taito Corporation Japan",   "Volfied (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, volfieduo, volfied, volfied, volfiedj, driver_device, 0, ROT270, "Taito America Corporation", "Volfied (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, volfiedjo, volfied, volfied, volfiedj, driver_device, 0, ROT270, "Taito Corporation",         "Volfied (Japan)", MACHINE_SUPPORTS_SAVE )
