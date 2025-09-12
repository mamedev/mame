// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/***************************************************************************

  Taito Cappricio crane game

****************************************************************************

Main PCB:
- Z84C0006PEC
- YAMAHA YM2203C
- OKI MSM6295
- 12 MHz oscillator
- TE7751
- 27C512 EPROM, 27C2001 EPROM

Sound PCB:


****************************************************************************

TODO:
- everything

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/te7750.h"
#include "sound/ymopn.h"
#include "speaker.h"


namespace {

class capriccio_state : public driver_device
{
public:
	capriccio_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank(*this, "databank")
	{ }

	void capriccio(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_memory_bank m_bank;

	void main_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void capriccio_state::machine_start()
{

}


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

void capriccio_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void capriccio_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
}

/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( capriccio )
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

void capriccio_state::capriccio(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &capriccio_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &capriccio_state::main_io_map);

	te7751_device &io(TE7751(config, "io"));
	io.ios_cb().set_constant(4);
	io.in_port1_cb().set_ioport("IN1");
	io.in_port2_cb().set_ioport("IN2");
	io.in_port3_cb().set_ioport("IN3");
	io.in_port4_cb().set_ioport("IN4");

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 16_MHz_XTAL/2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "speaker", 0.75, 0);
	ymsnd.add_route(0, "speaker", 0.75, 1);
	ymsnd.add_route(1, "speaker", 1.0, 0);
	ymsnd.add_route(2, "speaker", 1.0, 1);
}


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( capriccio ) // this set looks like a conversion from JP version
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "16.ic16", 0x00000, 0x20000, CRC(d73c21ea) SHA1(2b60a1cf1a9834a88d0a2911b314939ca98b0893) ) // M27C1001

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "14.ic14", 0x00000, 0x10000, CRC(eb1a77bb) SHA1(7a9ed992144d4aade6fefbcb78b6737924fcca01) ) // M27C512

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) // daughterboard with 4*27C040 at ic32
	ROM_LOAD( "17", 0x000000, 0x80000, CRC(0b457444) SHA1(022d9f030c9e9461a2ec954c9df00626e459d74a) )
	ROM_LOAD( "18", 0x080000, 0x80000, CRC(4edf3a9b) SHA1(95021ca153f842958176c35430ed58fc897c6d2e) )
	ROM_LOAD( "19", 0x100000, 0x80000, CRC(7c04ef12) SHA1(f5c5b2b1e28a65b0a33b332bcbf046aa462565c0) )
	ROM_LOAD( "20", 0x180000, 0x80000, CRC(c91ee395) SHA1(940b87d55de2ff3ad55cae216ab8959ad4c9a7b9) )

ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT        SCREEN  COMPANY              FULLNAME                  FLAGS
GAME( 199?, capriccio,  0,        capriccio, capriccio, capriccio_state, empty_init, ROT0,   "Taito Corporation", "Capriccio", MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
