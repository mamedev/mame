// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Taito Captain Zodiac, pirate-theme punching bag game with red dot matrix display

****************************************************************************

Hardware summary (Japan ver.)

Main PCB:
- 2*Z80 TMPZ84C00AP-6, 12MHz XTAL
- Z80 CTC TMPZ84C30AP-6
- 27C512 EPROM, TMS27C010A EPROM, 2316000 Mask ROM
- 3*5563-100 (8KB RAM)
- YM2610B, 16MHz XTAL
- TE7750, TC0140SYT
- SED1351F LCD controller

Display PCB:
- Toshiba TD62C962LF
- 4 16*16 LED matrix boards plugged in

****************************************************************************

TODO:
- everything

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "taitosnd.h"
#include "machine/te7750.h"
#include "machine/z80ctc.h"
#include "sound/ymopn.h"
#include "speaker.h"


namespace {

class cpzodiac_state : public driver_device
{
public:
	cpzodiac_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank(*this, "databank")
	{ }

	void cpzodiac(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_memory_bank m_bank;

	void main_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void cpzodiac_state::machine_start()
{
	m_bank->configure_entries(0, 0x10, memregion("maincpu")->base(), 0x2000);
	m_bank->set_entry(0);
}


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

void cpzodiac_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("databank");
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram(); // video?
	map(0xe000, 0xe00f).rw("io", FUNC(te7750_device::read), FUNC(te7750_device::write));
	map(0xe020, 0xe020).w("syt", FUNC(tc0140syt_device::master_port_w));
	map(0xe021, 0xe021).rw("syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

void cpzodiac_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void cpzodiac_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610b_device::read), FUNC(ym2610b_device::write));
	map(0xe200, 0xe200).w("syt", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("syt", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xea00, 0xea00).nopr();
	map(0xf200, 0xf200).nopw();
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cpzodiac )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P10") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P11") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P12") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P13") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P14") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P15") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P16") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P17") PORT_CODE(KEYCODE_COMMA)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P20") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P21") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P22") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P23") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P24") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P25") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P26") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P27") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P30") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P31") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P32") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P33") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P34") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P35") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P36") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P37") PORT_CODE(KEYCODE_8)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P40") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P41") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P42") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

void cpzodiac_state::cpzodiac(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cpzodiac_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cpzodiac_state::main_io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	te7750_device &io(TE7750(config, "io"));
	io.ios_cb().set_constant(4);
	io.in_port1_cb().set_ioport("IN1");
	io.in_port2_cb().set_ioport("IN2");
	io.in_port3_cb().set_ioport("IN3");
	io.in_port4_cb().set_ioport("IN4");
	io.out_port8_cb().set_membank(m_bank).rshift(4);
	// Code initializes Port 3 and 4 latches to 0 by mistake?

	z80ctc_device &ctc(Z80CTC(config, "ctc", 12_MHz_XTAL/2));
	ctc.intr_callback().set_inputline(m_maincpu, 0);

	Z80(config, m_audiocpu, 12_MHz_XTAL/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cpzodiac_state::sound_map);

	/* video hardware */
	// TODO

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 16_MHz_XTAL/2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);

	tc0140syt_device &syt(TC0140SYT(config, "syt", 0));
	syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( cpzodiac ) // this set looks like a conversion from JP version
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "16.ic16", 0x00000, 0x20000, CRC(d73c21ea) SHA1(2b60a1cf1a9834a88d0a2911b314939ca98b0893) ) // M27C1001

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "14.ic14", 0x00000, 0x10000, CRC(eb1a77bb) SHA1(7a9ed992144d4aade6fefbcb78b6737924fcca01) ) // M27C512

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) // daughterboard with 4*27C040 at ic32
	ROM_LOAD( "17", 0x000000, 0x80000, CRC(0b457444) SHA1(022d9f030c9e9461a2ec954c9df00626e459d74a) )
	ROM_LOAD( "18", 0x080000, 0x80000, CRC(4edf3a9b) SHA1(95021ca153f842958176c35430ed58fc897c6d2e) )
	ROM_LOAD( "19", 0x100000, 0x80000, CRC(7c04ef12) SHA1(f5c5b2b1e28a65b0a33b332bcbf046aa462565c0) )
	ROM_LOAD( "20", 0x180000, 0x80000, CRC(c91ee395) SHA1(940b87d55de2ff3ad55cae216ab8959ad4c9a7b9) )

	ROM_REGION( 0x1000, "pals", 0 )
	ROM_LOAD( "d52-02.ic38", 0x0000, 0x0aee, CRC(6be9b935) SHA1(d36af591b03873aee3098b7c74b53ac6370ca064) ) // PAL16L8BCN
ROM_END

ROM_START( cpzodiacj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "d52_03-1.ic16", 0x00000, 0x20000, CRC(129b8f44) SHA1(2789cd6f1322176c1956668f024b8bc9d4b3a816) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d52_04.ic14", 0x00000, 0x10000, CRC(804b45d4) SHA1(db3296558077c7c4eea968417d3edf2509d3742b) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "d52-01.ic32", 0x00000, 0x200000, CRC(3bde2b85) SHA1(4cf3cf88f7b227ac6d31ede7cdeffe6adcac5529) )

	ROM_REGION( 0x1000, "pals", 0 )
	ROM_LOAD( "d52-02.ic38", 0x0000, 0x0aee, CRC(6be9b935) SHA1(d36af591b03873aee3098b7c74b53ac6370ca064) ) // PAL16L8BCN
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT        SCREEN  COMPANY              FULLNAME                  FLAGS
GAME( 1993, cpzodiac,  0,        cpzodiac, cpzodiac, cpzodiac_state, empty_init, ROT0,   "Taito Corporation", "Captain Zodiac (World)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1993, cpzodiacj, cpzodiac, cpzodiac, cpzodiac, cpzodiac_state, empty_init, ROT0,   "Taito Corporation", "Captain Zodiac (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
