// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************



NEC TK80 / MIKROLAB KR580IK80
*****************************
TK80 driver by Robbbert
Mikrolab driver by Micko
Merged by Robbbert.

TK80 (Training Kit 80) considered to be Japan's first home computer.
It consisted of 25 keys and 8 LED digits, and was programmed in hex.
The Mikrolab is a Russian clone which appears to be almost completely identical.

TK85 seems to be the same as TK80, except it has a 8085 and a larger ROM.
No schematics etc are available. Thanks to 'Nama' who dumped the rom.
It has 25 keys, so a few aren't defined yet.

ND-80Z : http://www.alles.or.jp/~thisida/nd80z3syokai.html (newer version)
Like the TK85, it has a 2KB rom. Thanks again to 'Nama' who dumped it.

When booted, the system begins at 0000 which is ROM. You need to change the
address to 8000 before entering a program. Here is a test to paste in:
8000-11^22^33^44^55^66^77^88^99^8000-
Press the right-arrow to confirm data has been entered.

Operation:
4 digits at left is the address; 2 digits at right is the data.
As you increment addresses, the middle 2 digits show the previous byte.
You can enter 4 digits, and pressing 'ADRS SET' will transfer this info
to the left, thus setting the address to this value. Press 'WRITE INCR' to
store new data and increment the address. Press 'READ INCR' and 'READ DECR'
to scan through data without updating it. Other keys unknown/not implemented.

ToDo:
- Add storage



ICS8080
- Keys labels are correct, but which key is which is not known
- Character B is corrupt
- Operation is different to the other systems

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "tk80.lh"


namespace {

class tk80_state : public driver_device
{
public:
	tk80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi8255")
		, m_digit(*this, "digit%u", 0U)
	{ }

	void ics8080(machine_config &config);
	void tk80(machine_config &config);
	void mikrolab(machine_config &config);
	void nd80z(machine_config &config);
	void tk85(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	uint8_t key_matrix_r();
	uint8_t nd80z_key_r();
	uint8_t serial_r();
	void serial_w(uint8_t data);
	void mikrolab_serial_w(uint8_t data);
	uint8_t display_r(offs_t offset);
	void display_w(offs_t offset, uint8_t data);

	void ics8080_mem(address_map &map) ATTR_COLD;
	void mikrolab_io(address_map &map) ATTR_COLD;
	void nd80z_io(address_map &map) ATTR_COLD;
	void tk80_io(address_map &map) ATTR_COLD;
	void tk80_mem(address_map &map) ATTR_COLD;
	void tk85_mem(address_map &map) ATTR_COLD;

	uint8_t m_term_data = 0U;
	uint8_t m_keyb_press = 0U;
	uint8_t m_keyb_press_flag = 0U;
	uint8_t m_shift_press_flag = 0U;
	uint8_t m_ppi_portc = 0U;

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	output_finder<8> m_digit;
};


void tk80_state::machine_start()
{
	m_digit.resolve();

	save_item(NAME(m_term_data));
	save_item(NAME(m_keyb_press));
	save_item(NAME(m_keyb_press_flag));
	save_item(NAME(m_shift_press_flag));
	save_item(NAME(m_ppi_portc));
}

uint8_t tk80_state::display_r(offs_t offset)
{
	return m_digit[offset & 0x7];
}

void tk80_state::display_w(offs_t offset, uint8_t data)
{
	m_digit[offset & 0x7] = data;
}

void tk80_state::tk80_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x83ff); // A10-14 not connected
	map(0x0000, 0x02ff).rom();
	map(0x0300, 0x03ff).ram(); // EEPROM
	map(0x8000, 0x83f7).ram(); // RAM
	map(0x83f8, 0x83ff).ram().rw(FUNC(tk80_state::display_r), FUNC(tk80_state::display_w));
}

void tk80_state::tk85_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x87ff); // A10-14 not connected
	map(0x0000, 0x07ff).rom();
	map(0x8000, 0x83f7).ram();
	map(0x83f8, 0x83ff).ram().rw(FUNC(tk80_state::display_r), FUNC(tk80_state::display_w));
}

void tk80_state::ics8080_mem(address_map &map)
{
	map.unmap_value_high();
	//map.global_mask(0x87ff); // A10-14 not connected
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x83f7).ram();
	map(0x83f8, 0x83ff).ram().rw(FUNC(tk80_state::display_r), FUNC(tk80_state::display_w));
	map(0x8400, 0x8fff).ram();
}

void tk80_state::tk80_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x03);
	map(0x00, 0x03).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void tk80_state::mikrolab_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x03);
	map(0x00, 0x03).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void tk80_state::nd80z_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x03);
	map(0x00, 0x03).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

/* Input ports */
static INPUT_PORTS_START( tk80 )
	PORT_START("X0") /* KEY ROW 0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1") /* KEY ROW 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2") /* KEY ROW 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RET") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ADRS SET") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("READ DECR") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("READ INCR") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WRITE INCR") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORE DATA") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOAD DATA") PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

INPUT_PORTS_START( mikrolab )
	PORT_INCLUDE( tk80 )
	PORT_MODIFY("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RESUME") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ADDRESS") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAVE") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("END") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

INPUT_PORTS_START( ics8080 )
	PORT_INCLUDE( tk80 )
	PORT_MODIFY("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ADDR") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BRK") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLR") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STEP") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

static INPUT_PORTS_START( tk85 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 / AF") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 / BC") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 / DE") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 / HL") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 / SP") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 / BR.P") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 / BR.D") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A / SAVE") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B / LOAD") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C / TM") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D / MOV") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E / OUT") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F / IN") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONT") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ADRS SET") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("READ DEC") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("READ INC") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WR / ENT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R)
INPUT_PORTS_END

uint8_t tk80_state::key_matrix_r()
{
	// PA0-7 keyscan in

	uint8_t data = 0xff;

	if (BIT(m_ppi_portc, 4))
		data &= ioport("X0")->read();
	if (BIT(m_ppi_portc, 5))
		data &= ioport("X1")->read();
	if (BIT(m_ppi_portc, 6))
		data &= ioport("X2")->read();

	return data;
}

uint8_t tk80_state::nd80z_key_r()
{
	// PA0-7 keyscan in

	uint8_t data = 0xff, row = m_ppi_portc & 7;
	if (row == 6)
		data &= ioport("X0")->read();
	else if (row == 5)
		data &= ioport("X1")->read();
	else if (row == 3)
		data &= ioport("X2")->read();

	return data;
}

uint8_t tk80_state::serial_r()
{
	// PB0 - serial in
	//printf("B R\n");

	return 0;
}

void tk80_state::serial_w(uint8_t data)
{
	// PC0 - serial out
	// PC4-6 keyscan out
	// PC7 - display on/off
	m_ppi_portc = data ^ 0x70;
}

void tk80_state::mikrolab_serial_w(uint8_t data)
{
	// PC0 - serial out
	// PC4-6 keyscan out
	// PC7 - display on/off
	m_ppi_portc = data;
}

void tk80_state::tk80(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 18.432_MHz_XTAL / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &tk80_state::tk80_mem);
	m_maincpu->set_addrmap(AS_IO, &tk80_state::tk80_io);

	/* video hardware */
	config.set_default_layout(layout_tk80);

	/* Devices */
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(tk80_state::key_matrix_r));
	m_ppi->in_pb_callback().set(FUNC(tk80_state::serial_r));
	m_ppi->out_pc_callback().set(FUNC(tk80_state::serial_w));
}

void tk80_state::mikrolab(machine_config &config)
{
	tk80(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &tk80_state::tk85_mem);
	m_maincpu->set_addrmap(AS_IO, &tk80_state::mikrolab_io);

	/* Devices */
	m_ppi->out_pc_callback().set(FUNC(tk80_state::mikrolab_serial_w));
}

void tk80_state::nd80z(machine_config &config)
{
	Z80(config, m_maincpu, 1e6); // Sharp LH0080A, can't see writing on xtal
	m_maincpu->set_addrmap(AS_PROGRAM, &tk80_state::tk85_mem);
	m_maincpu->set_addrmap(AS_IO, &tk80_state::nd80z_io);

	/* video hardware */
	config.set_default_layout(layout_tk80);

	/* Devices */
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(tk80_state::nd80z_key_r));
	m_ppi->in_pb_callback().set(FUNC(tk80_state::serial_r));
	m_ppi->out_pc_callback().set(FUNC(tk80_state::mikrolab_serial_w));
}

void tk80_state::tk85(machine_config &config)
{
	I8085A(config, m_maincpu, 4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tk80_state::tk85_mem);
	m_maincpu->set_addrmap(AS_IO, &tk80_state::tk80_io);
	// TODO: SID and SOD = serial cassette interface

	/* video hardware */
	config.set_default_layout(layout_tk80);

	/* Devices */
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(tk80_state::key_matrix_r));
	m_ppi->out_pc_callback().set(FUNC(tk80_state::serial_w));
}

void tk80_state::ics8080(machine_config &config)
{
	tk80(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &tk80_state::ics8080_mem);
}


/* ROM definition */
ROM_START( tk80 )
	ROM_REGION( 0x0300, "maincpu", 0 )
	ROM_LOAD( "tk80-1.bin", 0x0000, 0x0100, CRC(897295e4) SHA1(50fb42b07252fc48044830e2f228e218fc59481c))
	ROM_LOAD( "tk80-2.bin", 0x0100, 0x0100, CRC(d54480c3) SHA1(354962aca1710ac75b40c8c23a6c303938f9d596))
	ROM_LOAD( "tk80-3.bin", 0x0200, 0x0100, CRC(8d4b02ef) SHA1(2b5a1ee8f97db23ffec48b96f12986461024c995))
ROM_END

ROM_START( ics8080 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "amtsmnonv27.rom1", 0x0000, 0x0800, CRC(82d40dc5) SHA1(9215457101c3b9b8706dbebe902196494993a282) )
	ROM_LOAD( "thmcplv11.rom2",   0x0800, 0x0800, CRC(51784f70) SHA1(c05c75d566c4ff8f681eba29cd48e72b95be89e0) )
	ROM_LOAD( "tunev08.rom3",     0x1000, 0x0800, CRC(aae2344b) SHA1(b02b22cadb43c7ac26c43d443688b9b19d465973) )
	ROM_LOAD( "mtrspdv12.rom4",   0x1800, 0x0800, CRC(920dda33) SHA1(631ee5e6314d9788e7be0ae00a97b55693eeb855) )
	ROM_FILL(0x2be,1,0x7c)    // Fix display of B
ROM_END

ROM_START( mikrolab )
	ROM_REGION( 0x0800, "maincpu", 0 )
	/* these dumps are taken from PDF so need check with real device */
	ROM_LOAD( "rom-1.bin", 0x0000, 0x0200, BAD_DUMP CRC(eed5f23b) SHA1(c82f7a16ce44c4fcbcb333245555feae1fcdf058))
	ROM_LOAD( "rom-2.bin", 0x0200, 0x0200, BAD_DUMP CRC(726a224f) SHA1(7ed8d2c6dd4fb7836475e207e1972e33a6a91d2f))
ROM_END

ROM_START( nectk85 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "tk85.bin",  0x0000, 0x0800, CRC(8a0b6d7e) SHA1(6acc8c04990692b08929043ccf638761b7301def))
ROM_END

ROM_START( nd80z )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "ndf.bin",  0x0000, 0x0800, CRC(fe829f1d) SHA1(6fff31884b8d984076d4450ca3a3e48efadeb648))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS       INIT        COMPANY      FULLNAME              FLAGS
COMP( 1976, tk80,     0,      0,      tk80,     tk80,     tk80_state, empty_init, "NEC",       "TK-80",              MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, nectk85,  tk80,   0,      tk85,     tk85,     tk80_state, empty_init, "NEC",       "TK-85",              MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 19??, nd80z,    tk80,   0,      nd80z,    tk80,     tk80_state, empty_init, "Chunichi",  "ND-80Z",             MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 19??, mikrolab, tk80,   0,      mikrolab, mikrolab, tk80_state, empty_init, "<unknown>", "Mikrolab KR580IK80", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 19??, ics8080,  tk80,   0,      ics8080,  ics8080,  tk80_state, empty_init, "<unknown>", "ICS8080",            MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
