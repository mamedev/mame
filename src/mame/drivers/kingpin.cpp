// license:BSD-3-Clause
// copyright-holders:Andrew Gardner

/*
ACL Manufacturing 1983 hardware (a division of American Communication Laboratories)
Driver by Andrew Gardner

2 x Sharp Z80
2 x NEC 8255
2 x OKI MSM5126-25RS (2Kx8 RAM), 3.6V battery connected to main RAM
TI TMS9928ANL
GI AY-3-8912
DSW bank at S1
3.579545MHz XTAL for CPU/sound, 10.7386MHz XTAL for video

several multigame gamblers on this hardware:
- The Dealer (not dumped)
- Kingpin
- Maxi-Dealer

Notes:
- To enter setup mode, boot the game with dips 1, 4, 5, 7 set to on
- There are some writes around 0xe000 in the maxideal set that can't
  possibly go anywhere on the board I own. A bigger RAM chip would
  accommodate them though.
- There are 6 pots labeled vr2-vr7. Color adjustments?
- The edge-connectors are non-jamma on this board.

Todo:
- lamps

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "machine/nvram.h"


class kingpin_state : public driver_device
{
public:
	kingpin_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	DECLARE_WRITE8_MEMBER(sound_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(vdp_interrupt);
};


WRITE8_MEMBER(kingpin_state::sound_nmi_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( kingpin_program_map, AS_PROGRAM, 8, kingpin_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingpin_io_map, AS_IO, 8, kingpin_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x21, 0x21) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0x60, 0x60) AM_WRITE(sound_nmi_w)
	//AM_RANGE(0x30, 0x30) AM_WRITENOP // lamps?
	//AM_RANGE(0x40, 0x40) AM_WRITENOP // lamps?
	//AM_RANGE(0x50, 0x50) AM_WRITENOP // ?
	//AM_RANGE(0x70, 0x70) AM_WRITENOP // ?
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingpin_sound_map, AS_PROGRAM, 8, kingpin_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	//AM_RANGE(0x8400, 0x8400) AM_READNOP // ?
	//AM_RANGE(0x8401, 0x8401) AM_WRITENOP // ?
	AM_RANGE(0x8800, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END



static INPUT_PORTS_START( kingpin )
	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BET )

	PORT_START("IN1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "S1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "S1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "S1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "S1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "S1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "S1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "S1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "S1:8" )
INPUT_PORTS_END


WRITE_LINE_MEMBER(kingpin_state::vdp_interrupt)
{
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

static MACHINE_CONFIG_START( kingpin, kingpin_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(kingpin_program_map)
	MCFG_CPU_IO_MAP(kingpin_io_map)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	// PORT A read = watchdog?
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW1"))
	// PORT C read = unused?

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	// PORT C read = unknown

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(kingpin_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(kingpin_state, irq0_line_hold,  1000) // unknown freq

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9928a", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(kingpin_state, vdp_interrupt))

	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8912, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



ROM_START( kingpin )
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "1.u12", 0x0000, 0x2000, CRC(5ba9aca3) SHA1(480bfcf4d6223c00f50ff9ef9dc3b5a7a8a2982c) )
	ROM_LOAD( "2.u13", 0x2000, 0x2000, CRC(aedb5cc6) SHA1(7800d8d757180089d5ff4de0386bbb264b9f65e0) )
	ROM_LOAD( "3.u14", 0x4000, 0x2000, CRC(27849017) SHA1(60dd3d0448b5ee96df207c57644569dab630e3e6) )
	ROM_LOAD( "4.u15", 0x6000, 0x2000, CRC(1a483d5c) SHA1(b0775f70be7fff334fd7991d8852127739373b3b) )
	ROM_LOAD( "5.u16", 0x8000, 0x2000, CRC(70a52bcd) SHA1(9c72e501777d4d36933242276a5b0c4a01bc5543) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "7.u22", 0x0000, 0x2000, CRC(077f533d) SHA1(74d0115b2cef5c35294ecb29771689b40ad1c25a) )

	ROM_REGION( 0x0800, "nvram", 0 ) // default nvram
	ROM_LOAD( "nvram.u19", 0x0000, 0x0800, CRC(59de96fe) SHA1(0fcbd8305b66db4d3e9c8070d70ad673d30610a3) )

	ROM_REGION( 0x40, "user1", 0 )
	ROM_LOAD( "n82s123n.u29", 0x00, 0x20, CRC(ce8b1a6f) SHA1(9b8f564efa4efea867884970f4a5850d598bc7a7) )
	ROM_LOAD( "n82s123n.u43", 0x20, 0x20, CRC(55569a2a) SHA1(5b0482546161c9d14a7d2c719d40774539cb41ca) )
ROM_END

ROM_START( maxideal )
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "mdc0.u12", 0x0000, 0x2000, CRC(0a73dd98) SHA1(ef3e20ecae646c2eda7364f566f3841747f982a5) )
	ROM_LOAD( "mdc1.u13", 0x2000, 0x2000, CRC(18c2550c) SHA1(1466f7d9601c336b4c802821bd2ba0091c9ff143) )
	ROM_LOAD( "mdc2.u14", 0x4000, 0x2000, CRC(ae2dd544) SHA1(c1380be538e4e952fad30a1725b23eb7358889dd) )
	ROM_LOAD( "mdc3.u15", 0x6000, 0x2000, CRC(f9e178e5) SHA1(66a8dbe5dbe595c9a3e083fc8cb89aa66d5cdabc) )
	ROM_LOAD( "mdc4.u16", 0x8000, 0x2000, CRC(a6b364b8) SHA1(a5d782f2e89ec8770407b247306a69cdd90a1214) )
	ROM_LOAD( "mdc5.u17", 0xa000, 0x2000, CRC(1df82ad1) SHA1(03efe6fb5362a7488e325f1f7e35376e6b7455b2) )
	ROM_LOAD( "mdc6.u18", 0xc000, 0x2000, CRC(c59f8f92) SHA1(d95e38bec50f0e6522e4d75a50702e09aced3d1c) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "7.u22", 0x0000, 0x2000, CRC(077f533d) SHA1(74d0115b2cef5c35294ecb29771689b40ad1c25a) )

	ROM_REGION( 0x0800, "nvram", 0 ) // default nvram
	ROM_LOAD( "nvram.u19", 0x0000, 0x0800, CRC(30a08f13) SHA1(a87d21c333bb8bc2e714aa843319643a84928269) )

	ROM_REGION( 0x40, "user1", 0 )
	ROM_LOAD( "n82s123n.u29", 0x00, 0x20, CRC(ce8b1a6f) SHA1(9b8f564efa4efea867884970f4a5850d598bc7a7) )
	ROM_LOAD( "n82s123n.u43", 0x20, 0x20, CRC(55569a2a) SHA1(5b0482546161c9d14a7d2c719d40774539cb41ca) )
ROM_END


GAME( 1983, kingpin,  0, kingpin, kingpin, driver_device, 0, 0, "ACL Manufacturing", "Kingpin", 0)
GAME( 1983, maxideal, 0, kingpin, kingpin, driver_device, 0, 0, "ACL Manufacturing", "Maxi-Dealer", 0)
