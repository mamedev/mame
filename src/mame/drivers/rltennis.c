// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/****************************************************************************************
Reality Tennis - (c) 1993 TCH

 driver by Tomasz Slanina

    based on informations provided by Antonio 'Peluko' Carrillo

Game Credits:
Antonio 'Peluko' Carrillo: programmer and game designer
David Sandoval: hardware designer

PCB Layout
----------

|-----------------------------------------------------|
|  |-----|  KM424C257  tennis_6  tennis_12     -      |
|  |Bt478|  KM424C257                                 |
|  |-----|             tennis_5  tennis_11     -      |
|           |-------|                                 |
|           |Actel  | |-------|  tennis_10     -      |
|           | A1020B| |Actel  |                       |
|J          |-------| | A1020B|  tennis_9      -      |
|A  32MHz    JOAQUIN  |-------|                       |
|M                      JUANA    tennis_8      -      |
|M            28C264                                  |
|A          tennis_1  tennis_2   tennis_7  tennis_14  |
|                                 GAL22V10            |
|            MT5C256  MT5C256              tennis_13  |
|          |------------------|     74HC404           |
|          |      68000P8     |  tennis_3  tennis_4   |
|          |------------------|                       |
|                NE555              DAC        DAC    |
|-----------------------------------------------------|

Video hardware:
---------------
Blitter based. Two layers with tricky doublebuffering.
Two Actel FPGA chips (marked as JOAQUIN and JUANA).
Juana can read data from ROMs. JOAQUIN - write to VRAM.
Both can access 256x256 pixel pages.
Size and direction of data read/write, as well as active page is
selectable for each of the chips.

Sound hardware (verify):
------------------------
~15 kHz 8 bit sigend (music) and unsigned (sfx) sample player.
Two custom DACs are conencted directly to data lines of sound ROMs.
A0-A10 address lines are controlled by a counter, clocked by scaline
clock ( not verified, just guessed ). Top lines are controlled by cpu,
and select 2k sample to play. There's probably no way to stop the sample
player - when there's nothing to play - first, empty 2k of ROMs are selected.

 TODO:
- proper timing and interrupts (remove extra hacky blitter int generation @ vblank)
- fix various gfx glitches here and there, mostly related to wrong size of data
 (what's the correct size? based on src or dest rectangle ? is there some kind of zoom? or just rect clipping?)
- what the 70000a blitter reg is for ?


****************************************************************************************/
#include "emu.h"
#include "includes/rltennis.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/ramdac.h"

#define RLT_REFRESH_RATE   60
#define RLT_TIMER_FREQ     (RLT_REFRESH_RATE*256)
#define RLT_XTAL           XTAL_12MHz

READ16_MEMBER(rltennis_state::io_r)
{
	return (ioport("P1" )->read()&0x1fff) | (m_unk_counter<<13); /* top 3 bits controls smaple address update */
}

WRITE16_MEMBER(rltennis_state::snd1_w)
{
	COMBINE_DATA(&m_data760000);
}

WRITE16_MEMBER(rltennis_state::snd2_w)
{
	COMBINE_DATA(&m_data740000);
}

static ADDRESS_MAP_START( rltennis_main, AS_PROGRAM, 16, rltennis_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x700000, 0x70000f) AM_WRITE(blitter_w)
	AM_RANGE(0x720000, 0x720001) AM_DEVWRITE8("ramdac",ramdac_device,index_w,0x00ff)
	AM_RANGE(0x720002, 0x720003) AM_DEVREADWRITE8("ramdac",ramdac_device,pal_r,pal_w,0x00ff)
	AM_RANGE(0x720006, 0x720007) AM_DEVWRITE8("ramdac",ramdac_device,index_r_w,0x00ff)
	AM_RANGE(0x740000, 0x740001) AM_WRITE(snd1_w)
	AM_RANGE(0x760000, 0x760001) AM_WRITE(snd2_w)
	AM_RANGE(0x780000, 0x780001) AM_WRITENOP    /* sound control, unknown, usually = 0x0044 */
	AM_RANGE(0x7a0000, 0x7a0003) AM_READNOP     /* unknown, read only at boot time*/
	AM_RANGE(0x7e0000, 0x7e0001) AM_READ(io_r)
	AM_RANGE(0x7e0002, 0x7e0003) AM_READ_PORT("P2")
ADDRESS_MAP_END

static INPUT_PORTS_START( rltennis )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(rltennis_state::sample_player)
{
	if((m_dac_counter&0x7ff) == 0x7ff) /* reload top address bits */
	{
		m_sample_rom_offset_1=(( m_data740000 >> m_offset_shift ) & 0xff )<<11;
		m_sample_rom_offset_2=(( m_data760000 >> m_offset_shift ) & 0xff )<<11;
		m_offset_shift^=8; /* switch between MSB and LSB */
	}
	++m_dac_counter; /* update low address bits */

	m_dac_1->write_signed8(m_samples_1[m_sample_rom_offset_1 + ( m_dac_counter&0x7ff )]);
	m_dac_2->write_unsigned8(m_samples_2[m_sample_rom_offset_2 + ( m_dac_counter&0x7ff )]);
	m_timer->adjust(attotime::from_hz( RLT_TIMER_FREQ ));
}

INTERRUPT_GEN_MEMBER(rltennis_state::interrupt)
{
	++m_unk_counter; /* frame counter? verify */
	device.execute().set_input_line(4, HOLD_LINE);
	device.execute().set_input_line(1, HOLD_LINE); /* hack, to avoid dead loop */
}

void rltennis_state::machine_start()
{
	m_samples_1 = memregion("samples1")->base();
	m_samples_2 = memregion("samples2")->base();
	m_gfx =  memregion("gfx1")->base();
	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(rltennis_state::sample_player),this));

	save_item(NAME(m_data760000));
	save_item(NAME(m_data740000));
	save_item(NAME(m_dac_counter));
	save_item(NAME(m_sample_rom_offset_1));
	save_item(NAME(m_sample_rom_offset_2));
	save_item(NAME(m_offset_shift));
	save_item(NAME(m_unk_counter));
}

void rltennis_state::machine_reset()
{
	m_timer->adjust(attotime::from_hz(RLT_TIMER_FREQ));
}

static ADDRESS_MAP_START( ramdac_map, AS_0, 8, rltennis_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb888_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( rltennis, rltennis_state )

	MCFG_CPU_ADD("maincpu", M68000, RLT_XTAL/2) /* 68000P8  ??? */
	MCFG_CPU_PROGRAM_MAP(rltennis_main)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rltennis_state, interrupt)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( RLT_REFRESH_RATE )
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0,319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(rltennis_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")
	MCFG_RAMDAC_SPLIT_READ(1)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", .5)
	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", .5)

MACHINE_CONFIG_END

ROM_START( rltennis )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tennis_1.u12", 0x00001, 0x80000, CRC(2ded10d7) SHA1(cca1e858c9c759ef5c0aca6ee50d23d5d532534c) )
	ROM_LOAD16_BYTE( "tennis_2.u19", 0x00000, 0x80000, CRC(a0dbd2ed) SHA1(8db7dbb6a36fd0fb382a4938d7eba1f7662aa672) )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  )
	ROM_LOAD( "tennis_5.u33", 0x000000, 0x80000, CRC(067a2e4b) SHA1(ab5a227de2b0c51b17aeca68c8af1bf224904ac8) )
	ROM_LOAD( "tennis_6.u34", 0x080000, 0x80000, CRC(901df2c1) SHA1(7e57d7c7e281ddc02a3e34178d3e471bd8e1d572) )
	ROM_LOAD( "tennis_7.u35", 0x100000, 0x80000, CRC(8d70fb37) SHA1(250c4c3d32e5a7e17413ee41e1abccb0492b63fd) )
	ROM_LOAD( "tennis_8.u36", 0x180000, 0x80000, CRC(26d202ba) SHA1(0e841e35de328f23624a19780a734a18f5409d69) )
	ROM_LOAD( "tennis_9.u37", 0x200000, 0x80000, CRC(1d164ee0) SHA1(b9c80b3c0dadbff36a04141b8995a5282a8d10f7) )
	ROM_LOAD( "tennis_10.u38",0x280000, 0x80000, CRC(fd2c6647) SHA1(787f236d5b72ee24d39e783eb2453bea58f07290) )
	ROM_LOAD( "tennis_11.u39",0x300000, 0x80000, CRC(a59dc0c8) SHA1(48f258e74fbb64b7538c9777d7598774ca8396eb) )
	ROM_LOAD( "tennis_12.u40",0x380000, 0x80000, CRC(b9677887) SHA1(84b79864555d3d6e9c443913910a055e27d30d08) )
	ROM_LOAD( "tennis_13.u41",0x400000, 0x80000, CRC(3d4fbcac) SHA1(e01f479d7d516ff83cbbd82d83617146d7a242d3) )
	ROM_LOAD( "tennis_14.u42",0x480000, 0x80000, CRC(37fe0f5d) SHA1(7593f1ea07bc0a741c952e6850bed1bf0a824510) )

	ROM_REGION( 0x080000, "samples1", 0 )
	ROM_LOAD( "tennis_4.u59", 0x00000, 0x80000, CRC(f56462ea) SHA1(638777e12f2649a5b4366f034f0ba721fc4580a8) )

	ROM_REGION( 0x080000, "samples2", 0 )
	ROM_LOAD( "tennis_3.u52", 0x00000, 0x80000, CRC(517dcd0e) SHA1(b2703e185ee8cf7e115ea07151e7bee8be34948b) )
ROM_END

GAME( 1993, rltennis,    0, rltennis,    rltennis, driver_device,    0, ROT0,  "TCH", "Reality Tennis", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
