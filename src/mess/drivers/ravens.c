/***************************************************************************

Ravensburger Selbstbaucomputer

This is a project described in "Ravensburger" magazine. You had to make
the entire thing (including the circuit boards) yourself.

http://petersieg.bplaced.com/?2650_Computer:2650_Selbstbaucomputer

        2013-04-23 Skeleton driver.

Hardware:
        0000-07FF ROM "MON1"
        0800-1FFF RAM (3x HM6116)
        24 pushbuttons and 6-digit LED display on front panel.
        Other buttons and switches on the main board.

The few photos show the CPU and a number of ordinary 74LSxxx chips.
There is a XTAL of unknown frequency.

There is a cassette interface..

ToDo:
- Everything

****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
//#include "sound/speaker.h"
#include "ravens.lh"


class ravens_state : public driver_device
{
public:
	ravens_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
//	, m_speaker(*this, "speaker")
	{ }

	DECLARE_READ8_MEMBER(port07_r);
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_WRITE8_MEMBER(port06_w);
	UINT8 m_last_key;
	bool m_speaker_state;
	required_device<cpu_device> m_maincpu;
//	required_device<speaker_sound_device> m_speaker;
};

WRITE8_MEMBER( ravens_state::port00_w )
{
	output_set_digit_value(offset, data);
}

WRITE8_MEMBER( ravens_state::port06_w )
{
//	m_speaker_state ^=1;
//	speaker_level_w(m_speaker, m_speaker_state);
}

READ8_MEMBER( ravens_state::port07_r )
{
	UINT8 keyin, i, data = 0x40;

	keyin = ioport("X0")->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if BIT(~keyin, i)
				data = i | 0xc0;

	keyin = ioport("X1")->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if BIT(~keyin, i)
				data = i | 0xc8;

	if (data == m_last_key)
		data &= 0x7f;
	else
		m_last_key = data;

	data |= ioport("X2")->read();

	return data;
}

static ADDRESS_MAP_START( ravens_mem, AS_PROGRAM, 8, ravens_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff) AM_ROM
	AM_RANGE( 0x0800, 0x1fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ravens_io, AS_IO, 8, ravens_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_WRITE(port00_w) // 4-led display
	AM_RANGE(0x06, 0x06) AM_WRITE(port06_w)  // speaker (NOT a keyclick)
	AM_RANGE(0x07, 0x07) AM_READ(port07_r) // pushbuttons
	//AM_RANGE(S2650_SENSE_PORT, S2650_FO_PORT) AM_READWRITE(ravens_cass_in,ravens_cass_out)
	AM_RANGE(0x102, 0x103) AM_NOP // stops error log filling up while using debug
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ravens )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ADR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
INPUT_PORTS_END


static MACHINE_CONFIG_START( ravens, ravens_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(ravens_mem)
	MCFG_CPU_IO_MAP(ravens_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_ravens)

	/* sound hardware */
//	MCFG_SPEAKER_STANDARD_MONO("mono")
//	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
//	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ravens )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v2.0", "V2.0")
	ROMX_LOAD( "mon_v2.0.bin", 0x0000, 0x0800, CRC(bcd47c58) SHA1(f261a3f128fbedbf59a8b5480758fff4d7f76de1), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v0.9", "V0.9")
	ROMX_LOAD( "mon_v0.9.bin", 0x0000, 0x0800, CRC(785eb1ad) SHA1(c316b8ac32ab6aa37746af37b9f81a23367fedd8), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT   COMPAT   MACHINE    INPUT     INIT     COMPANY    FULLNAME       FLAGS */
COMP( 1979, ravens,  0,       0,       ravens,  ravens, driver_device, 0,     "Joseph Glagla and Dieter Feiler", "Ravensburger Selbstbaucomputer", GAME_NOT_WORKING | GAME_NO_SOUND_HW )
