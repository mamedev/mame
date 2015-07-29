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
#include "sound/speaker.h"
//TODO: #include "ds348.lh"

class daruma_state : public driver_device
{
public:
	daruma_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker") { }

	DECLARE_WRITE8_MEMBER(port_w);
	DECLARE_READ8_MEMBER(port_r);

	DECLARE_READ8_MEMBER(dev0_r);
	DECLARE_WRITE8_MEMBER(dev1_w);
	DECLARE_WRITE8_MEMBER(dev2_w);
	DECLARE_READ8_MEMBER(dev4_r);
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	char port0, port1, port2, port3;
};

WRITE8_MEMBER(daruma_state::port_w)
{
//  printf("port_w: write %02X to PORT (offset=%02X)\n", data, offset);
	switch(offset)
	{
		case MCS51_PORT_P0: port0=data;
		case MCS51_PORT_P1: port1=data;
		case MCS51_PORT_P2: port2=data;
		case MCS51_PORT_P3: port3=data;
	}
}

READ8_MEMBER(daruma_state::port_r)
{
	switch(offset)
	{
		case MCS51_PORT_P0: printf("port_r: read %02X from PORT0\n", port0); return port0;
		case MCS51_PORT_P1: printf("port_r: read %02X from PORT1\n", port1); return port1;
		case MCS51_PORT_P2: printf("port_r: read %02X from PORT2\n", port2); return port2;
		case MCS51_PORT_P3: printf("port_r: read %02X from PORT3\n", port3); return port3;
	}
	return 0;
}

READ8_MEMBER(daruma_state::dev0_r)
{
	return 0xFF;
}

READ8_MEMBER(daruma_state::dev4_r)
{
	return ioport("switches")->read();
}

WRITE8_MEMBER(daruma_state::dev1_w)
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

WRITE8_MEMBER(daruma_state::dev2_w)
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

static ADDRESS_MAP_START( mem_prg, AS_PROGRAM, 8, daruma_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mem_io, AS_IO, 8, daruma_state )
	AM_RANGE(0x0000, 0x0000) AM_READ(dev0_r)
	AM_RANGE(0x1000, 0x1000) AM_WRITE(dev1_w)
//    AM_RANGE(0x2000, 0x2000) AM_WRITE(dev2_w)
//    AM_RANGE(0x3000, 0x3000) AM_WRITE(dev3_w)
	AM_RANGE(0x4000, 0x4000) AM_READ(dev4_r)
	AM_RANGE(0x8000, 0xffff) AM_RAM /* 32K CMOS SRAM (HYUNDAY hy62256a) */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READWRITE(port_r, port_w)
ADDRESS_MAP_END

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

static MACHINE_CONFIG_START( daruma, daruma_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C32,11059200) //verified on pcb
	MCFG_CPU_PROGRAM_MAP(mem_prg)
	MCFG_CPU_IO_MAP(mem_io)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(0, "mono", 1.00)

/*  TODO:
    MCFG_DEFAULT_LAYOUT(layout_daruma)

    Motors: MTA011
    http://pdf.datasheetcatalog.com/datasheet/Shindengen/mXstzvq.pdf

    The motor controller suposedly is used to cut the paper strip out after finishing printing something.
*/
MACHINE_CONFIG_END

ROM_START( ds348 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "daruma_ds348_v1_1.rom",   0x0000, 0x10000, CRC(10bf9036) SHA1(d654a13bc582f5384e759ec6fe5309a642bd8e18) )
ROM_END

/*    YEAR  NAME   PARENT COMPAT MACHINE  INPUT   INIT              COMPANY           FULLNAME                                 FLAGS */
COMP( 1998, ds348, 0,     0,     daruma,  daruma, driver_device, 0, "Sigtron Daruma", "Print Plus DS348 - Dot matrix printer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
