// license:GPL2+
// copyright-holders:Felipe Sanches
/******************************************************************

 Alesis QS-series keyboards

 Currently this skeleton covers only the Alesis QS-7
 unit, but other keyboards in this series have similar
 hardware caracteristics.

 Author: Felipe Correa da Silva Sanches <juca@members.fsf.org>

*******************************************************************/

#include "emu.h"
#include "cpu/h8500/h8510.h"
//#include "sound/alesis_qs.h"


namespace {

class qs_state : public driver_device
{
public:
	qs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void qs7(machine_config &config);

private:
	void qs7_prog_map(address_map &map) ATTR_COLD;

	required_device<h8510_device> m_maincpu;
};

/* Input ports */
static INPUT_PORTS_START( qs7 )
//        PORT_START("COL1")
//        PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_)
//        PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_)
//        PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_)
//        PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_)
//        PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_)
//        PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_)
//        PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_)
//        PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_)
INPUT_PORTS_END

void qs_state::qs7_prog_map(address_map &map)
{
	map(0x00000, 0x3ffff).mirror(0x40000).rom().region("program", 0);
}

void qs_state::qs7(machine_config &config)
{
	/* basic machine hardware */
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &qs_state::qs7_prog_map);

	//ALESIS_KEYSCAN(config, "keyscan", 20_MHz_XTAL / 2 / 8);

	/* Alesis Sound Generator ASIC */
	//ALESIS_SG(config, "sndgen");

	/* Alesis Sound Effects Processor ASIC */
	//ALESIS_FXCHIP(config, "sfx", 7.056_MHz_XTAL);

	/* video hardware */
	//TODO: add LCD display controller here

	/* sound hardware */
	//SPEAKER(config, "left").front_left();
	//SPEAKER(config, "right").front_right();
	//alesis_qs_series_device &sound(ALESIS_QS_SERIES(config, "sound", SND_CLOCK));
	//sound.add_route(0, "left", 1.0);
	//sound.add_route(1, "right", 1.0);

		/* Interfaces */
		//PCMCIA
		//MIDI
		//RS232
}

// XTALs: 20 MHz (H8/510), 7.056 MHz (FXCHIP), 14.7456 MHz
ROM_START( alesqs7 )
	ROM_REGION16_BE( 0x80000, "program", 0 )
	ROM_LOAD16_WORD_SWAP( "2-31-0069_q7_v1.02_alesis_sp_09_12_96_cs_dbcc.u18", 0x00000, 0x80000, CRC(6e5404cb) SHA1(f00598b66ab7a83b16105cbb73e09c66ce3493a7) )

//  ROM_REGION( 0x200000, "sound", 0 ) /* Samples ROMs (2Mbyte each) */
//  ROM_LOAD( "?.u?", 0x00000, 0x200000, NO_DUMP )
//  ROM_LOAD( "?.u?", 0x00000, 0x200000, NO_DUMP )
//  ROM_LOAD( "?.u?", 0x00000, 0x200000, NO_DUMP )
//  ROM_LOAD( "?.u?", 0x00000, 0x200000, NO_DUMP )
//  ROM_LOAD( "?.u?", 0x00000, 0x200000, NO_DUMP )
//  ROM_LOAD( "?.u?", 0x00000, 0x200000, NO_DUMP )
//  ROM_LOAD( "?.u?", 0x00000, 0x200000, NO_DUMP )
//  ROM_LOAD( "?.u?", 0x00000, 0x200000, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS     INIT        COMPANY   FULLNAME                       FLAGS
COMP( 1996, alesqs7, 0,      0,      qs7,     qs7,   qs_state, empty_init, "Alesis", "Alesis QS7 musical keyboard", MACHINE_IS_SKELETON )
