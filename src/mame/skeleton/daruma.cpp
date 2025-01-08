// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

    Sigtron Daruma DS348 dot-matrix printer
    https://www.hardstand.com.br/daruma/ds348

    Driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>

    Model: Print Plus - DS348
    Manufacturer: Sigtron Daruma
    Firmware version 1.1
    Release Date: May 8th/1998
    PCB: SIGTRON DS348 REV.B

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/spkrdev.h"
#include "speaker.h"
//TODO: #include "ds348.lh"


namespace {

class daruma_state : public driver_device
{
public:
	daruma_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker") { }

	void daruma(machine_config &config);

private:
	uint8_t dev0_r();
	void dev1_w(uint8_t data);
	[[maybe_unused]] void dev2_w(uint8_t data);
	uint8_t dev4_r();
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	void mem_io(address_map &map) ATTR_COLD;
	void mem_prg(address_map &map) ATTR_COLD;
};

uint8_t daruma_state::dev0_r()
{
	return 0xFF;
}

uint8_t daruma_state::dev4_r()
{
	return ioport("switches")->read();
}

void daruma_state::dev1_w(uint8_t data)
{
	//while attempting to identify which bit is used for
	//controlling the buzzer, here's what I heard from each of
	//the signals on this address:

	//0x80 serial comm.? (noise)
	//0x20 LED? (3 clicks)
	//0x10 LED? (1 click)
	//0x08 serial comm.? click & noise
	//0x04 LED? (2 clicks)
	//0x02 motor control or printer heads? (I hear a series of rhythmic pulses)
	//0x01 LED? (2 clicks)
	m_speaker->level_w(data & 0x02);
}

void daruma_state::dev2_w(uint8_t data)
{
	//while attempting to identify which bit is used for
	//controlling the buzzer, here's what I heard from each of
	//the signals on this address:

	//0x80: LED? (3 clicks)
	//0x40: unused?
	//0x20: unused?
	//0x10: low freq brief beep followed by a click
	//0x08: low freq brief noise followed by a click
	//0x04: low freq brief beep followed by a click
	//0x02: low freq brief beep followed by a click
	//0x01: low freq brief noise
	//m_speaker->level_w(data & 0x01);
}

void daruma_state::mem_prg(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void daruma_state::mem_io(address_map &map)
{
	map(0x0000, 0x0000).r(FUNC(daruma_state::dev0_r));
	map(0x1000, 0x1000).w(FUNC(daruma_state::dev1_w));
//    map(0x2000, 0x2000).w(FUNC(daruma_state:dev2_w));
//    map(0x3000, 0x3000).w(FUNC(daruma_state:dev3_w));
	map(0x4000, 0x4000).r(FUNC(daruma_state::dev4_r));
	map(0x8000, 0xffff).ram(); /* 32K CMOS SRAM (HYUNDAY hy62256a) */
}

//TODO: These buttons and switches are all guesses. We'll need to further investigate this.
static INPUT_PORTS_START( daruma )
	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Paper A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Paper B") PORT_CODE(KEYCODE_B)

	PORT_START("switches")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Limit Switch") PORT_CODE(KEYCODE_S)

INPUT_PORTS_END

void daruma_state::daruma(machine_config &config)
{
	/* basic machine hardware */
	I80C32(config, m_maincpu, 11059200); //verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &daruma_state::mem_prg);
	m_maincpu->set_addrmap(AS_IO, &daruma_state::mem_io);
	// TODO: ports

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(0, "mono", 1.00);

/*  TODO:
    config.set_default_layout(layout_daruma);

    Motors: MTA011
    http://pdf.datasheetcatalog.com/datasheet/Shindengen/mXstzvq.pdf

    The motor controller supposedly is used to cut the paper strip out after finishing printing something.
*/
}

ROM_START( ds348 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "daruma_ds348_v1_1.rom",   0x0000, 0x10000, CRC(10bf9036) SHA1(d654a13bc582f5384e759ec6fe5309a642bd8e18) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY           FULLNAME                                 FLAGS
COMP( 1998, ds348, 0,      0,      daruma,  daruma, daruma_state, empty_init, "Sigtron Daruma", "Print Plus DS348 - Dot matrix printer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
