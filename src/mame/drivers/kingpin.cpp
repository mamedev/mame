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
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"
#include "speaker.h"


class kingpin_state : public driver_device
{
public:
	kingpin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
	{ }

	void kingpin(machine_config &config);
	void dealracl(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(sound_nmi_w);
	void kingpin_io_map(address_map &map);
	void kingpin_program_map(address_map &map);
	void kingpin_sound_map(address_map &map);
	void dealracl_program_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
};


WRITE8_MEMBER(kingpin_state::sound_nmi_w)
{
	m_soundlatch->write(space, 0, data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void kingpin_state::kingpin_program_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xf000, 0xf7ff).ram().share("nvram");
}

void kingpin_state::dealracl_program_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0xc000, 0xc7ff).ram().share("nvram");
}

void kingpin_state::kingpin_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x20).rw("tms9928a", FUNC(tms9928a_device::vram_r), FUNC(tms9928a_device::vram_w));
	map(0x21, 0x21).rw("tms9928a", FUNC(tms9928a_device::register_r), FUNC(tms9928a_device::register_w));
	map(0x60, 0x60).w(FUNC(kingpin_state::sound_nmi_w));
	//AM_RANGE(0x30, 0x30) AM_WRITENOP // lamps?
	//AM_RANGE(0x40, 0x40) AM_WRITENOP // lamps?
	//AM_RANGE(0x50, 0x50) AM_WRITENOP // ?
	//AM_RANGE(0x70, 0x70) AM_WRITENOP // ?
}

void kingpin_state::kingpin_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x8001).w("aysnd", FUNC(ay8910_device::address_data_w));
	//AM_RANGE(0x8400, 0x8400) AM_READNOP // ?
	//AM_RANGE(0x8401, 0x8401) AM_WRITENOP // ?
	map(0x8800, 0x8fff).ram();
	map(0x9000, 0x9000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}



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


void kingpin_state::kingpin(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &kingpin_state::kingpin_program_map);
	m_maincpu->set_addrmap(AS_IO, &kingpin_state::kingpin_io_map);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	// PORT A read = watchdog?
	ppi0.in_pb_callback().set_ioport("DSW1");
	// PORT C read = unused?

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("IN0");
	ppi1.in_pb_callback().set_ioport("IN1");
	// PORT C read = unknown

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &kingpin_state::kingpin_sound_map);
	m_audiocpu->set_periodic_int(FUNC(kingpin_state::irq0_line_hold), attotime::from_hz(1000)); // unknown freq

	/* video hardware */
	tms9928a_device &vdp(TMS9928A(config, "tms9928a", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline("maincpu", 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8912(config, "aysnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void kingpin_state::dealracl(machine_config &config)
{
	kingpin(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &kingpin_state::dealracl_program_map);
}



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

/* Guru Readme for "The Dealer, ACL, 1981"

ACL10001B
|-----------------------------------------------|
|                2114        DSW1   8255    DSW2|
|                      LM556                    |
| AY3-8912                               HMI6514|
|                2114   ROM    ROM    ROM       |
|                       ROM    ROM    ROM       |
| LM555                 ROM    ROM    ROM       |
|               Z80A    ROM           ROM       |
|                                     ROM       |
|                                     ROM       |
|                                               |
|                                               |
|                                        HMI6514|
|           TMS9918                             |
| 4116            10.738635MHz                  |
| 4116      8255                                |
| 4116                                          |
| 4116                                          |
| 4116                                          |
| 4116                                          |
| 4116                                          |
| 4116                              Z80A        |
|                                               |
|                                  LM339        |
|-----|             |------------|        |-----|
      |-------------|            |--------|

Note! This board was trashed and had many empty sockets so there's no
guarantee that the above layout is accurate to the PCB when it was working.
All ROMs have unknown locations. There were no identifying ROM stickers on them
and their locations were not recorded before removal from the PCB.
There are no bi-polar PROMs on the PCB so they are probably missing. */

// Does continual writes to i/o 70 and 80.

/* Bad dumps: 11a is a redump of 11, but they both have strange bytes. Both included if anyone wants to investigate.
   It's possible other roms could be bad too.
   At 12CB is a jump to E612, which has nothing there. Changed to 1612 which at least *seems* more sensible. */


ROM_START( dealracl ) // ROMs were unlabeled, so they might be ordered wrong.
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "1",   0x0000, 0x0800, CRC(6191abc7) SHA1(2decc88be89f081043c7a2604d7b17dc6b72f49a) )
	ROM_LOAD( "10",  0x0800, 0x0800, CRC(10b9bafd) SHA1(efda9245d9bba7cc7c97d411757b7a0a87e65e12) )
	ROM_LOAD( "11a", 0x1000, 0x0800, BAD_DUMP CRC(3f5d55b5) SHA1(5c2e7d11fb26aaf4759751e9c00003f070df648b) )
	ROM_FILL(0x12CD, 1 , 0x16)
	ROM_LOAD( "8",   0x1800, 0x0800, CRC(9f1621f8) SHA1(164e117479edfe478942054378e78125f40fe4f7) )
	ROM_LOAD( "7",   0x2000, 0x0800, CRC(8c491dd0) SHA1(9e77d50198e93d243d5a06893d3a29fc43f21b7d) )
	ROM_LOAD( "13",  0x2800, 0x0800, CRC(626fea42) SHA1(ed7727231b4bcb63928efb105ede9f42aee4c2df) )
	ROM_LOAD( "6",   0x3000, 0x0800, CRC(72dedb38) SHA1(d1e12b3d8b1c2170100802e6071df59fc72a211f) )
	ROM_LOAD( "2",   0x3800, 0x0800, CRC(f21652fb) SHA1(2f5d6bccc570425440d6ca4712ce0d8814bdada5) )
	ROM_LOAD( "12",  0x4000, 0x0800, CRC(4534cb68) SHA1(235aa0864762da86c30d6f6d64acb593873a8a12) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "5",   0x0000, 0x0800, CRC(0d77ffb4) SHA1(519a1c9efdbafa545640c9a124d81bbfa6fc0791) )
	ROM_LOAD( "9",   0x0800, 0x0800, CRC(3771b8ae) SHA1(3cc0c16219260c47390df43049665a9159c5c872) )
	ROM_LOAD( "4",   0x1000, 0x0800, CRC(cea5c377) SHA1(55cb9d45ae315a50dbab2b7082c942b6fb65017a) )
	ROM_LOAD( "3",   0x1800, 0x0800, CRC(bc1722d6) SHA1(1c00bc789b71669f591bec0fcaebe099a0ae00f1) )

	ROM_REGION( 0x0800, "nvram", ROMREGION_ERASE00 ) // default nvram

	ROM_REGION( 0x40, "user1", 0 ) // not dumped for this PCB
	ROM_LOAD( "n82s123n.u29", 0x00, 0x20, NO_DUMP ) //CRC(ce8b1a6f) SHA1(9b8f564efa4efea867884970f4a5850d598bc7a7) )
	ROM_LOAD( "n82s123n.u43", 0x20, 0x20, NO_DUMP ) //CRC(55569a2a) SHA1(5b0482546161c9d14a7d2c719d40774539cb41ca) )

	ROM_REGION( 0x800, "user2", 0 )
	ROM_LOAD( "11",  0x0000, 0x0800, BAD_DUMP CRC(91a12c65) SHA1(d6c24888937c01ebbc96e28cdd9bee83ad01a1cd) )
ROM_END



GAME( 1983, kingpin,  0, kingpin,  kingpin, kingpin_state, empty_init, 0, "ACL Manufacturing", "Kingpin",     0 )
GAME( 1983, maxideal, 0, kingpin,  kingpin, kingpin_state, empty_init, 0, "ACL Manufacturing", "Maxi-Dealer", 0 )
GAME( 1981, dealracl, 0, dealracl, kingpin, kingpin_state, empty_init, 0, "ACL Manufacturing", "The Dealer (ACL)",  MACHINE_NOT_WORKING )
