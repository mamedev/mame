/***************************************************************************

        Dolphin

        08/04/2010 Skeleton driver.
        20/05/2012 Fixed keyboard, added notes & speaker [Robbbert]

    Minimal Setup:
        0000-00FF ROM "MO" (74S471)
        0100-01FF ROM "MONI" (74S471)
        0200-02FF RAM (2x 2112)
        18 pushbuttons for programming (0-F, ADR, NXT).
        4-digit LED display.

    Other options:
        0400-07FF Expansion RAM (8x 2112)
        0800-08FF Pulse for operation of an optional EPROM programmer
        0C00-0FFF ROM "MONA" (2708)
        LEDs connected to all Address and Data Lines
        LEDs connected to WAIT and FLAG lines.
        Speaker with a LED wired across it.
        PAUSE switch.
        RUN/STOP switch.
        STEP switch.
        CLOCK switch.

        Cassette player connected to SENSE and FLAG lines.

        Keyboard encoder: AY-5-2376 (57 keys)

        CRT interface: (512 characters on a separate bus)
        2114 video ram (one half holds the lower 4 data bits, other half the upper bits)
        74LS175 holds the upper bits for the 74LS472
        74LS472 Character Generator

        NOTE: a rom is missing, when the ADR button (- key) is pressed,
        it causes a freeze in nodebug mode, and a crash in debug mode.
        To see it, start in debug mode. g 6c. In the emulation, press the
        minus key. The debugger will stop and you can see an instruction
        referencing location 0100, which is in the missing rom.

        Keys:
        0-9,A-F hexadecimal numbers
        UP - (NXT) to enter data and advance to the next address
        MINUS - (ADR) to change the address to what is shown in the data side
        Special keys:
        Hold UP, hold 0, release UP, release 0 - execute program at the current address (i.e. 2xx)
        Hold UP, hold 1, release UP, release 1 - execute program at address 0C00 (rom MONA)
        Hold UP, hold 2, release UP, release 2 - play a tune with the keys
        Hold UP, hold 3, release UP, release 3 - decrement the address by 2
        Hold MINUS, hold any hex key, release MINUS, release other key - execute program
          at the current address-0x100 (i.e. 1xx).

        If you want to scan through other areas of memory (e.g. the roms), alter the
        data at address 2F9 (high byte) and 2FA (low byte).

        How to Use:
        The red digits are the address, and the orange digits are the data.
        The address range is 200-2FF (the 2 isn't displayed). To select an address,
        either press the UP key until you get there, or type the address and press
        minus. The orange digits show the current data at that address. To alter
        data, just type it in and press UP.

        TODO:
        - Find missing roms
        - Add optional hardware listed above

        Thanks to Amigan site for various documents.


****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "sound/speaker.h"
#include "dolphunk.lh"


class dolphunk_state : public driver_device
{
public:
	dolphunk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_speaker(*this, SPEAKER_TAG)
	{ }

	DECLARE_READ8_MEMBER(port07_r);
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_WRITE8_MEMBER(port06_w);
	UINT8 m_last_key;
	bool m_speaker_state;
	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_speaker;
};

WRITE8_MEMBER( dolphunk_state::port00_w )
{
	output_set_digit_value(offset, data);
}

WRITE8_MEMBER( dolphunk_state::port06_w )
{
	m_speaker_state ^=1;
	speaker_level_w(m_speaker, m_speaker_state);
}

READ8_MEMBER( dolphunk_state::port07_r )
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

static ADDRESS_MAP_START( dolphunk_mem, AS_PROGRAM, 8, dolphunk_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x01ff) AM_ROM
	AM_RANGE( 0x0200, 0x02ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dolphunk_io, AS_IO, 8, dolphunk_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_WRITE(port00_w) // 4-led display
	AM_RANGE(0x06, 0x06) AM_WRITE(port06_w)  // speaker (NOT a keyclick)
	AM_RANGE(0x07, 0x07) AM_READ(port07_r) // pushbuttons
	//AM_RANGE(S2650_SENSE_PORT, S2650_FO_PORT) AM_READWRITE(dolphunk_cass_in,dolphunk_cass_out)
	AM_RANGE(0x102, 0x103) AM_NOP // stops error log filling up while using debug
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dolphunk )
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


static MACHINE_CONFIG_START( dolphunk, dolphunk_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(dolphunk_mem)
	MCFG_CPU_IO_MAP(dolphunk_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_dolphunk)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dolphunk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dolphin_mo.rom", 0x0000, 0x0100, CRC(a8811f48) SHA1(233c629dc20fac286c8c1559e461bb0b742a675e) )
	ROM_LOAD( "dolphin_moni.rom", 0x0100, 0x0100, NO_DUMP )
	ROM_LOAD_OPTIONAL( "dolphin_mona.rom", 0x0c00, 0x0400, NO_DUMP )
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT     INIT     COMPANY    FULLNAME       FLAGS */
COMP( 1979, dolphunk,  0,       0,       dolphunk,  dolphunk, driver_device, 0,     "<unknown>", "Dolphin", 0 )
