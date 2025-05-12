// license:BSD-3-Clause
// copyright-holders:

/*

Elektronische Steuereinheit
8085 based hardware
Lots of lamps and 4 7-segment LEDs

Main components:
Siemens SAB 8085AH-2-P (CPU)
Sharp LH5164D-10L or Sony CXK5816PN-12L (SRAM)
Siemens SAB 8256 A 2 P (MUART)
NEC D8279C-2 (keyboard & display interface)
AMI or Micrel S50240 (sound)

3 Different boards:
4040-000-101 (6 ROM slots, TC5514 RAM) Parent is excellent
4087-000-101 (3 ROM slots, RTC HD146818) Parent is doppelpot
4109-000-101 (2 ROM slots, RTC 62421A) Parent is kniffi
4382-000-101 (2 ROM slots, RTC 62421A) Parent is dicemstr

Dice Master reference: https://www.youtube.com/watch?v=NlB06dMxjME
Merkur Disc reference: https://www.youtube.com/watch?v=1NjJPkzg9Mk
Nova Kniffi reference: https://www.youtube.com/watch?v=YBq2Z1irXek
*/


#include "emu.h"

#include "cpu/i8085/i8085.h"
//#include "machine/i8256.h"
#include "machine/i8279.h"
#include "machine/mc146818.h"
#include "machine/msm6242.h"

#include "speaker.h"


namespace {

class stella8085_state : public driver_device
{
public:
	stella8085_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_kdc(*this, "kdc")
	{ }

	void dicemstr(machine_config &config);
	void doppelpot(machine_config &config);
	void excellent(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<i8279_device> m_kdc;

	void program_map(address_map &map) ATTR_COLD;
	void large_program_map(address_map &map) ATTR_COLD;
	void excellent_program_map(address_map &map) ATTR_COLD;
	void rtc62421_io_map(address_map &map) ATTR_COLD;
	void mc146818_io_map(address_map &map) ATTR_COLD;
};


void stella8085_state::program_map(address_map &map)
{
	map(0x0000, 0x8fff).rom(); // ICE6, ICD6, ICC5
	map(0xc000, 0xc7ff).ram(); // ICC6
}

void stella8085_state::large_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); // ICE6
	map(0x8000, 0x9fff).ram(); // ICC6
	map(0xa000, 0xffff).rom(); // ICD6
}

void stella8085_state::excellent_program_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
}

void stella8085_state::rtc62421_io_map(address_map &map)
{
	map(0x00, 0x0f).rw("rtc", FUNC(rtc62421_device::read), FUNC(rtc62421_device::write));
	map(0x50, 0x51).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	//map(0x60, 0x6f).rw("muart", FUNC(i8256_device::read8), FUNC(i8256_device::write8));
}

void stella8085_state::mc146818_io_map(address_map &map)
{
	// TODO: map RTC
	map(0x50, 0x51).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	//map(0x60, 0x6f).rw("muart", FUNC(i8256_device::read8), FUNC(i8256_device::write8));
}


static INPUT_PORTS_START( dicemstr )
	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, "Restart Interrupt 7")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Hold")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "Restart Interrupt 5")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "Reset")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("SERVICE1") // Row 6
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") // Col 7
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dauerlauf")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Spielzähler")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Münzspeicher")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hardware-Test")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Auszahlquote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Foul")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Gewinn") // Col 0

	PORT_START("SERVICE2") // Row 7
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hoch 1,-")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Runter 1,-")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hoch Serie")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Runter Serie")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hoch 0,10")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Runter 0,10")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Münzung")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Initialisieren")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("DM 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("DM 2.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("DM 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("DM 0.10")

	PORT_START("INPUTS")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void stella8085_state::dicemstr(machine_config &config)
{
	I8085A(config, m_maincpu, 10.240_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::large_program_map);
	m_maincpu->set_addrmap(AS_IO, &stella8085_state::rtc62421_io_map);

	//I8256(config, "muart", 10.240_MHz_XTAL / 2); // divider not verified

	I8279(config, m_kdc, 10.240_MHz_XTAL / 4); // divider not verified

	RTC62421(config, "rtc", 32.768_kHz_XTAL);

	SPEAKER(config, "mono").front_center();
}

void stella8085_state::doppelpot(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &stella8085_state::mc146818_io_map);

	//I8256(config, "muart", 6.144_MHz_XTAL);

	I8279(config, m_kdc, 6.144_MHz_XTAL / 2);

	MC146818(config, "rtc", 32.768_kHz_XTAL);

	SPEAKER(config, "mono").front_center();
}

void stella8085_state::excellent(machine_config &config)
{
	doppelpot(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::excellent_program_map);
}


ROM_START( dicemstr ) // curiously hand-written stickers say F3 but strings in ROM are F2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stella_dice_master_f3_i.ice6",  0x0000, 0x8000, CRC(9897fb87) SHA1(bfb18c1370d9bd12ec61622c0ebbad5c0138e1d8) )
	ROM_LOAD( "stella_dice_master_f3_ii.icd6", 0x8000, 0x8000, CRC(9484cf3b) SHA1(e1104882eaba860ab984c1a37e2f97d4bed08829) ) // 0x0000 - 0x1fff is 0xff filled
ROM_END

ROM_START( doppelpot )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "doppelpot.ice6", 0x0000, 0x4000, CRC(b01d3307) SHA1(8364506e8169432ddec275ef5b53660c01dc209e) )
	ROM_LOAD( "doppelpot.icd6", 0x4000, 0x4000, CRC(153708cb) SHA1(3d15b115ec39c1df42d4437226e83413f495c4d9) )
	ROM_LOAD( "doppelpot.icc5", 0x8000, 0x1000, CRC(135dac6b) SHA1(10873ee64579245eac7069bf84d61550684e67de) )
ROM_END

ROM_START( disc2000 )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "disc2000.ice6", 0x0000, 0x4000, CRC(53a66005) SHA1(a5bb63abe8eb631a0fb09496ef6e0ee6c713985c) )
	ROM_LOAD( "disc2000.icd6", 0x4000, 0x4000, CRC(787b6708) SHA1(be990f95b6d04cbe0b9832603204f2a81b0ace3f) )
ROM_END

ROM_START( disc2001 )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "disc2001.ice6", 0x0000, 0x4000, CRC(4d128fe1) SHA1(2b9b0a1296ff77b281173fb0fcf667ed3e3ece2b) )
	ROM_LOAD( "disc2001.icd6", 0x4000, 0x4000, CRC(72f6560a) SHA1(3fdc3aaafcc2c185a19a27ccd511d8522fbe0c2e) )
ROM_END

ROM_START( disc3000 )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "disc3000.ice6", 0x0000, 0x4000, CRC(6e024e72) SHA1(7198c0cd844d4bc080b2d8654d32d53a04ce8bb4) )
	ROM_LOAD( "disc3000.icd6", 0x4000, 0x4000, CRC(ad88715a) SHA1(660f4044e8f24ad59767ce025966475f9fd56885) )
ROM_END

ROM_START( elitedisc )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "elitedisc.ice6", 0x0000, 0x4000, CRC(7f7a2f30) SHA1(01e3ce5fce2c9d51d3f4b8aab7dd67ed4b26d8f4) )
	ROM_LOAD( "elitedisc.icd6", 0x4000, 0x4000, CRC(e56f2360) SHA1(691a6762578daca6ce4581418761dcc07c291fab) )
ROM_END

ROM_START( excellent )
	ROM_REGION( 0x5000, "maincpu", 0 )
	ROM_LOAD( "excellent.ice5", 0x0800, 0x0800, CRC(b4c573b5) SHA1(5b01b68b8abd48bd293bc9aa507c3285a6e7550f) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.ice6", 0x1800, 0x0800, CRC(f1d53581) SHA1(7aef66149f3427b287d3e9d86cc198dc1ed40d7c) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icd5", 0x2800, 0x0800, CRC(912a5f59) SHA1(3df3ca7eaef8de8e13e93f6a1e6975f8da7ed7a1) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icd6", 0x3800, 0x0800, CRC(5a2b95b4) SHA1(b0d17b327664e8680b163c872109769c4ae42039) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icc5", 0x4800, 0x0800, CRC(ae424805) SHA1(14e12ceebd9fbf6eba96c168e8e7b797b34f7ca5) BAD_DUMP ) // underdumped
ROM_END

ROM_START( extrablatt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "extrablatt.ice6", 0x0000, 0x8000, CRC(6885cf89) SHA1(30acd5511fb73cb22ae4230fedcf40f385c0d261) )
	ROM_LOAD( "extrablatt.icd6", 0x8000, 0x8000, CRC(5c0cb9bd) SHA1(673d5f8dec7ccce1c4f39dce6be1e9d1ed699047) )
ROM_END

ROM_START( glucksstern )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "glucksstern.ice6", 0x00000, 0x20000, CRC(8e969bae) SHA1(bf66d491932b77dab4c6b15ec7fbf470223636ac) )
	ROM_LOAD( "glucksstern.icd6", 0x20000, 0x20000, CRC(f31b860a) SHA1(7b016bb7d0699cfe7165c0abb2c1bbcb944cdc86) )
ROM_END

ROM_START( juwel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "juwel.ice6", 0x0000, 0x8000, CRC(6fd9fd6a) SHA1(2ff982750d87be1bc7757bde706d9e329ac29785) )
	ROM_LOAD( "juwel.icd6", 0x8000, 0x8000, CRC(a9ec9e36) SHA1(f7a2b5866988116e0bbeb8a120cae9083d651c5b) )
ROM_END

ROM_START( karoas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "karoas.ice6", 0x0000, 0x8000, CRC(71c4c39d) SHA1(b188896838a788d5bfc7b18f1bb423a06fe5fcc6) )
	ROM_LOAD( "karoas.icd6", 0x8000, 0x8000, CRC(e1b131bd) SHA1(dc2fbfaf86fa5b161d17a563eae2bc8fc4d19395) )
ROM_END

ROM_START( kniffi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kniffi.ice6", 0x0000, 0x8000, CRC(57df5d69) SHA1(78bc9cabf0b4bec5f8c2578d55011f0adc034798) )
	ROM_LOAD( "kniffi.icd6", 0x8000, 0x8000, CRC(1c129cec) SHA1(bad22f18b94c16dba36995ff8daf4d48f4d082a2) )
ROM_END

} // anonymous namespace


// 'STELLA DICE MASTER F2' and 'COPYRIGHT BY ADP LUEBBECKE GERMANY 1993' in ROM
GAME( 1993, dicemstr,    0,         dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "Stella", "Dice Master",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1986, doppelpot,   0,         doppelpot, dicemstr, stella8085_state, empty_init, ROT0, "Nova",   "Doppelpot",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1987, disc2000,    doppelpot, doppelpot, dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Disc 2000",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1987, disc2001,    doppelpot, doppelpot, dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Disc 2001",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1989, disc3000,    doppelpot, doppelpot, dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Disc 3000",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1986, elitedisc,   doppelpot, doppelpot, dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Elite Disc",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1982, excellent,   0,         excellent, dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Excellent",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1987, kniffi,      0,         dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "Nova",   "Kniffi",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1988, extrablatt,  kniffi,    dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Extrablatt",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1998, glucksstern, kniffi,    dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "ADP",    u8"Glücks-Stern", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1988, juwel,       kniffi,    dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Juwel",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1992, karoas,      kniffi,    dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Karo As",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
