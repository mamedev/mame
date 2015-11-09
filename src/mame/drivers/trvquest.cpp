// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

Trivia Quest
Sunn/Techstar 1984

CPU: 6809
Three SY6522 - for flashing lighted buttons?

Sound: Two AY-3-8910

Forty eight! MK4027 4K Rams

Six 6116 Rams with battery backup

Two Crystals:
11.6688 Mhz
6 Mhz

Two 8 position DIPS

rom3 through rom7 - Main pcb PRG.

roma through romi - Sub pcb Questions.

The main pcb had empty sockets for
rom0, rom1 and rom2.
This pcb has been tested and works
as is.

 driver by Pierpaolo Prazzoli

Notes:
- Hardware is similar to the one in gameplan.c

*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "sound/ay8910.h"
#include "includes/gameplan.h"
#include "machine/nvram.h"

READ8_MEMBER(gameplan_state::trvquest_question_r)
{
	return memregion("questions")->base()[*m_trvquest_question * 0x2000 + offset];
}

WRITE_LINE_MEMBER(gameplan_state::trvquest_coin_w)
{
	coin_counter_w(machine(), 0, ~state & 1);
}

WRITE_LINE_MEMBER(gameplan_state::trvquest_misc_w)
{
	// data & 1 -> led on/off ?
}

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 8, gameplan_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram") // cmos ram
	AM_RANGE(0x2000, 0x27ff) AM_RAM // main ram
	AM_RANGE(0x3800, 0x380f) AM_DEVREADWRITE("via6522_1", via6522_device, read, write)
	AM_RANGE(0x3810, 0x381f) AM_DEVREADWRITE("via6522_2", via6522_device, read, write)
	AM_RANGE(0x3820, 0x382f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)
	AM_RANGE(0x3830, 0x3831) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x3840, 0x3841) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x3850, 0x3850) AM_READNOP //watchdog_reset_r ?
	AM_RANGE(0x8000, 0x9fff) AM_READ(trvquest_question_r)
	AM_RANGE(0xa000, 0xa000) AM_WRITEONLY AM_SHARE("trvquest_q")
	AM_RANGE(0xa000, 0xa000) AM_READNOP // bogus read from the game code when reads question roms
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( trvquest )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("UNK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


MACHINE_START_MEMBER(gameplan_state,trvquest)
{
	/* register for save states */
	save_item(NAME(m_video_x));
	save_item(NAME(m_video_y));
	save_item(NAME(m_video_command));
	save_item(NAME(m_video_data));

	/* this is needed for trivia quest */
	m_via_0->write_pb5(1);
}

MACHINE_RESET_MEMBER(gameplan_state,trvquest)
{
	m_video_x = 0;
	m_video_y = 0;
	m_video_command = 0;
	m_video_data = 0;
}

INTERRUPT_GEN_MEMBER(gameplan_state::trvquest_interrupt)
{
	m_via_2->write_ca1(1);
	m_via_2->write_ca1(0);
}

static MACHINE_CONFIG_START( trvquest, gameplan_state )

	MCFG_CPU_ADD("maincpu", M6809,XTAL_6MHz/4)
	MCFG_CPU_PROGRAM_MAP(cpu_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gameplan_state, trvquest_interrupt)

	MCFG_NVRAM_ADD_1FILL("nvram")
	MCFG_MACHINE_START_OVERRIDE(gameplan_state,trvquest)
	MCFG_MACHINE_RESET_OVERRIDE(gameplan_state,trvquest)

	/* video hardware */
	MCFG_FRAGMENT_ADD(trvquest_video)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_6MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_6MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 0)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(gameplan_state, video_data_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(gameplan_state, gameplan_video_command_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(gameplan_state, video_command_trigger_w))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, 0)
	MCFG_VIA6522_READPA_HANDLER(IOPORT("IN0"))
	MCFG_VIA6522_READPB_HANDLER(IOPORT("IN1"))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(gameplan_state, trvquest_coin_w))

	MCFG_DEVICE_ADD("via6522_2", VIA6522, 0)
	MCFG_VIA6522_READPA_HANDLER(IOPORT("UNK"))
	MCFG_VIA6522_READPB_HANDLER(IOPORT("DSW"))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(gameplan_state, trvquest_misc_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(gameplan_state, via_irq))
MACHINE_CONFIG_END

ROM_START( trvquest )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom3", 0xb000, 0x1000, CRC(2ff7f370) SHA1(66f40426ed02ee44235e17a49d9054ede42b83b9) )
	ROM_LOAD( "rom4", 0xc000, 0x1000, CRC(b1adebcb) SHA1(661cabc92b1defce5c2edb8e873a80d5032084d0) )
	ROM_LOAD( "rom5", 0xd000, 0x1000, CRC(2fc10a15) SHA1(8ecce32a5a167056c8fb48554a8907ae6299921e) )
	ROM_LOAD( "rom6", 0xe000, 0x1000, CRC(fabf4846) SHA1(862cac32de78f2ff4afef398b864d5533d302a4f) )
	ROM_LOAD( "rom7", 0xf000, 0x1000, CRC(a9f56551) SHA1(fb6fc3b17a6e66571a5ba837befbfac1ac26cc39) )

	ROM_REGION( 0x18000, "questions", ROMREGION_ERASEFF ) /* Question roms */
	/* 0x00000 - 0x07fff empty */
	ROM_LOAD( "romi", 0x06000, 0x2000, CRC(c8368f69) SHA1(c1dfb701482c5ae922df0a93665a519995a2f4f1) )
	ROM_LOAD( "romh", 0x08000, 0x2000, CRC(f3aa8a08) SHA1(2bf8f878cc1df84806a6fb8e7be2656c422d61b9) )
	ROM_LOAD( "romg", 0x0a000, 0x2000, CRC(f85f8e48) SHA1(38c9142181a8ee5c0bc80cf2a06d4137fcb2a8b9) )
	ROM_LOAD( "romf", 0x0c000, 0x2000, CRC(2bffdcab) SHA1(96bd9aede5a76f9ddcf29e8df2c632075d21b8f6) )
	ROM_LOAD( "rome", 0x0e000, 0x2000, CRC(3ff66402) SHA1(da13fe6b99d7517ad2ecd0e42d0c306d4e49563a) )
	ROM_LOAD( "romd", 0x10000, 0x2000, CRC(4e21653f) SHA1(719a8dda9b81963a6b6d7d3e4966ecde676b9ecf) )
	ROM_LOAD( "romc", 0x12000, 0x2000, CRC(081a5322) SHA1(09e7ea5f1ee1dc35ec00bcea1550c6fe0bbdf60d) )
	ROM_LOAD( "romb", 0x14000, 0x2000, CRC(819ab451) SHA1(78c181eae63d55d1d0643bb7be07ca3cdbe14285) )
	ROM_LOAD( "roma", 0x16000, 0x2000, CRC(b4bcaf33) SHA1(c6b08fb8d55b2834d0c6c5baff9f544c795e4c15) )
ROM_END

GAME( 1984, trvquest, 0, trvquest, trvquest, driver_device, 0, ROT90, "Sunn / Techstar", "Trivia Quest", MACHINE_SUPPORTS_SAVE )
