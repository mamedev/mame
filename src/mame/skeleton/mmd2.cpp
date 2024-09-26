// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

MMD-2 driver by Miodrag Milanovic

2009-05-12 Initial version
2011-01-12 MMD2 working [Robbbert]


http://www.cs.unc.edu/~yakowenk/classiccmp/mmd2/
Memory map:

    * 4K RAM addresses $0000..$0FFF
    * ROM addresses $D800..$E7FF
    * 256 bytes of RAM, ($FC00..$FCFF?)

DIP switches:

    * WE 0 - Write enable for addresses $0000..$03FF
    * WE 1 - Write enable for addresses $0400..$07FF
    * WE 2 - Write enable for addresses $0800..$0BFF
    * WE 3 - Write enable for addresses $0C00..$0FFF
    * SPARE - ???
    * HEX OCT - choose display and entry to be in Hexadecimal or Octal
    * PUP RESET - ???
    * EXEC USER - update binary LED's with data entry? Or not?
      (in either setting, outputs to ports 0,1,2 still show)

Operation:

    * Enter bytes on the keypad of hex digits
    * Set MSByte of address by entering it on the keypad & pressing "HIGH".
    * ... LSByte ... "LOW"
    * Change contents of memory at the selected address by entering the new value & pressing "STORE".
    * Look at adjacent memory locations with "NEXT" and "PREV".
    * Execute the program at the selected address by pressing "GO".

AUX functions:

    * BRL HI # - OFF disables BRL LO
    * BRL LO #
    * STEP #
    * SRC HI # - source for COPY/DUMP - OFF disables "DUMP" function
    * DES HI # - destination for COPY/DUMP
    * LEN HI # - length for COPY/DUMP
    * CLR TST ON - test if PROM is empty
    * POP PRM ON - program a PROM
    * DUP TST ON - test if PROM duplicated okay
    * PROM 2708/2716
    * MEM MAP RAM/ROM
    * BAUD 110/150/300/600/1200

The memory map can be rearranged by the system by using IN5, IN6, IN7.
A pair of undumped proms control what goes where on each map.
Each set of ROMs also has its own pair of PROMs.


I/O ports:
IN0: user expansion
IN1: 0-TTYIN, 1-CASSIN, 3-SW8(binary/ports), 4-SW7(reset/pup), 5-SW6(hex/oct), 6-(pup signal)
IN3: 8279 status
IN4: 8279 key
IN5: set MAP1
IN6: set MAP2
IN7: set MAP3
IN8: read eprom (in the eprom programmer)
OUT0: PORT0 LEDs
OUT1: PORT1 LEDs
OUT2: PORT2 LEDs
OUT3: 8279 control
OUT4: 8279 7-segment LED data
OUT5: TTYOUT, CASSOUT
OUT9: turn pup signal off
OUTA: programming pulse on/off (eprom programmer)

Dips:
SW1-4 hardware control of RAM being writable
SW5 not used
SW6 Control if the 7-segment displays show Octal (low) or Hex (high).
SW7 Reset only causes the cpu to perform a warm start. PUP does a cold start (Power-UP).
SW8 Control if the PORT displays echo the 7-segment displays (high), or just act as normal output ports (low).


ToDo
- Hook up WE0-3
- tty rs232 interface
- Add interrupt module (INTE LED is always on atm)
- Need software
- Probably lots of other things

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8279.h"
#include "imagedev/cassette.h"
#include "speaker.h"
#include "mmd2.lh"


namespace {

class mmd2_state : public driver_device
{
public:
	mmd2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_banks(*this, "bank%u", 1U)
		, m_io_keyboard(*this, "X%u", 0U)
		, m_io_dsw(*this, "DSW")
		, m_digits(*this, "digit%u", 0U)
		, m_p(*this, "p%u_%u", 0U, 0U)
		, m_led_halt(*this, "led_halt")
		, m_led_hold(*this, "led_hold")
	{ }

	void mmd2(machine_config &config);

	void init_mmd2();

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void round_leds_w(offs_t, u8);
	void port05_w(u8 data);
	u8 port01_r();
	u8 port13_r();
	u8 bank_r(address_space &space, offs_t offset);
	u8 keyboard_r();
	void scanlines_w(u8 data);
	void digit_w(u8 data);
	void status_callback(u8 data);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void reset_banks();

	u8 m_digit = 0U;
	std::unique_ptr<u8[]> m_ram;
	required_device<i8080_cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_memory_bank_array<8> m_banks;
	required_ioport_array<4> m_io_keyboard;
	required_ioport m_io_dsw;
	output_finder<9> m_digits;
	output_finder<3, 8> m_p;
	output_finder<> m_led_halt;
	output_finder<> m_led_hold;
};


void mmd2_state::round_leds_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_p[offset][i] = BIT(~data, i);
}

void mmd2_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).bankr("bank1").bankw("bank2");
	map(0x0400, 0x0fff).bankr("bank3").bankw("bank4");
	map(0xd800, 0xe3ff).bankr("bank5").bankw("bank6");
	map(0xe400, 0xe7ff).bankr("bank7").bankw("bank8");
	map(0xfc00, 0xfcff).ram();
}

void mmd2_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x02).w(FUNC(mmd2_state::round_leds_w));
	map(0x01, 0x01).r(FUNC(mmd2_state::port01_r));
	map(0x03, 0x03).rw("i8279", FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));
	map(0x04, 0x04).rw("i8279", FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	map(0x05, 0x07).r(FUNC(mmd2_state::bank_r));
	map(0x05, 0x05).w(FUNC(mmd2_state::port05_w));
	//map(0x09, 0x09).w  PUP signal
	//map(0x0a, 0x0a).w  Eprom programmer
}


/* Input ports */
static INPUT_PORTS_START( mmd2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x20, 0x00, "Sw6") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, "Hex")
	PORT_DIPSETTING(    0x00, "Octal")
	PORT_DIPNAME( 0x10, 0x10, "Sw7") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x10, "PUP")
	PORT_DIPSETTING(    0x00, "Reset")
	PORT_DIPNAME( 0x08, 0x08, "Sw8") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x08, "Exec")
	PORT_DIPSETTING(    0x00, "User")

	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CANCEL") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AUX") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REGS") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS)
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PROM") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DUMP") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOAD") PORT_CODE(KEYCODE_O)
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OPTION") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOW") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HIGH") PORT_CODE(KEYCODE_H)
	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STEP") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEXT") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORE") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PREV") PORT_CODE(KEYCODE_DOWN)
	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, mmd2_state, reset_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(mmd2_state::reset_button)
{
	if (newval)
		reset_banks();
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

/*
Keyboard
0  1  2  3      PREV  STORE  NEXT  STEP
4  5  6  7      HIGH  LOW  GO  OPTION
8  9  A  B      LOAD  DUMP  PROM  COPY
C  D  E  F      MEM  REGS  AUX  CANCEL

*/

u8 mmd2_state::bank_r(address_space &space, offs_t offset)
{
	for (auto &bank : m_banks)
		bank->set_entry(offset);
	return space.unmap();
}

u8 mmd2_state::port01_r()
{
	// need to add ttyin bit 0
	u8 data = 0x84;
	data |= m_io_dsw->read();
	data |= (m_cass->input() < 0.02) ? 0 : 2;
	return data;
}

void mmd2_state::port05_w(u8 data)
{
	// need to add ttyout bit 0
	m_cass->output(BIT(data, 1) ? -1.0 : +1.0);
}

void mmd2_state::scanlines_w(u8 data)
{
	m_digit = data;
}

void mmd2_state::digit_w(u8 data)
{
	if (m_digit < 9)
		m_digits[m_digit] = data;
}

u8 mmd2_state::keyboard_r()
{
	u8 data = 0xff;

	if ((m_digit & 7) < 4)
		data = m_io_keyboard[m_digit & 7]->read();

	return data;
}

void mmd2_state::status_callback(u8 data)
{
	// operate the HALT LED
	m_led_halt = (~data & i8080_cpu_device::STATUS_HLTA) ? 1 : 0;
	// operate the HOLD LED - this should connect to the HLDA pin,
	// but it isn't emulated, using WO instead (whatever that does).
	m_led_hold = (data & i8080_cpu_device::STATUS_WO) ? 1 : 0;
}

void mmd2_state::machine_start()
{
	m_digits.resolve();
	m_p.resolve();
	m_led_halt.resolve();
	m_led_hold.resolve();

	save_pointer(NAME(m_ram), 0x1400);
	save_item(NAME(m_digit));
}

void mmd2_state::machine_reset()
{
	reset_banks();
}

void mmd2_state::reset_banks()
{
	for (auto &bank : m_banks)
		bank->set_entry(0);
}

void mmd2_state::init_mmd2()
{
	// We preset all banks here, so that bankswitching will incur no speed penalty.
	// ROM 0000/0400 indicate ROMs, RAM /0400/0C00 indicate RAM, 1000 is a dummy write area for ROM banks.
	u8 *const ROM = memregion("maincpu")->base();
	m_ram = make_unique_clear<u8[]>(0x1400);
	u8 *RAM = m_ram.get();
	m_banks[0]->configure_entry(0, &ROM[0x0000]);
	m_banks[0]->configure_entry(1,  RAM);
	m_banks[0]->configure_entry(2, &ROM[0x0c00]);
	m_banks[1]->configure_entry(0,  RAM+0x1000);
	m_banks[1]->configure_entry(1,  RAM);
	m_banks[1]->configure_entry(2,  RAM+0x1000);
	m_banks[2]->configure_entry(0, &ROM[0x0400]);
	m_banks[2]->configure_entry(1,  RAM+0x0400);
	m_banks[2]->configure_entry(2,  RAM+0x0400);
	m_banks[3]->configure_entry(0,  RAM+0x1000);
	m_banks[3]->configure_entry(1,  RAM+0x0400);
	m_banks[3]->configure_entry(2,  RAM+0x0400);
	m_banks[4]->configure_entry(0,  RAM);
	m_banks[4]->configure_entry(1, &ROM[0x0000]);
	m_banks[4]->configure_entry(2, &ROM[0x0000]);
	m_banks[5]->configure_entry(0,  RAM);
	m_banks[5]->configure_entry(1,  RAM+0x1000);
	m_banks[5]->configure_entry(2,  RAM+0x1000);
	m_banks[6]->configure_entry(0,  RAM+0x0c00);
	m_banks[6]->configure_entry(1, &ROM[0x0c00]);
	m_banks[6]->configure_entry(2,  RAM);
	m_banks[7]->configure_entry(0,  RAM+0x0c00);
	m_banks[7]->configure_entry(1,  RAM+0x1000);
	m_banks[7]->configure_entry(2,  RAM);
}

void mmd2_state::mmd2(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 6750000 / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmd2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mmd2_state::io_map);
	m_maincpu->out_status_func().set(FUNC(mmd2_state::status_callback));
	m_maincpu->out_inte_func().set_output("led_inte");         // operate the INTE LED

	/* video hardware */
	config.set_default_layout(layout_mmd2);

	/* Devices */
	i8279_device &kbdc(I8279(config, "i8279", 400000));        // based on divider
	kbdc.out_sl_callback().set(FUNC(mmd2_state::scanlines_w)); // scan SL lines
	kbdc.out_disp_callback().set(FUNC(mmd2_state::digit_w));   // display A&B
	kbdc.in_rl_callback().set(FUNC(mmd2_state::keyboard_r));   // kbd RL lines
	kbdc.in_shift_callback().set_constant(1);                  // Shift key
	kbdc.in_ctrl_callback().set_constant(1);

	// Cassette
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	SPEAKER(config, "mono").front_center();
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( mmd2 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "mmd2330.bin", 0x0000, 0x0800, CRC(69a77199) SHA1(6c83093b2c32a558c969f4fe8474b234023cc348))
	ROM_LOAD( "mmd2340.bin", 0x0800, 0x0800, CRC(70681bd6) SHA1(c37e3cf34a75e8538471030bb49b8aed45d00ec3))
	ROM_LOAD( "mmd2350.bin", 0x1000, 0x0800, CRC(359f577c) SHA1(9405ca0c1977721e4540a4017907c06dab08d398))
	ROM_LOAD( "mmd2360.bin", 0x1800, 0x0800, CRC(967e69b8) SHA1(c21ec8bef955806a2c6e1b1c8e9068662fb88038))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                FULLNAME  FLAGS
COMP( 1976, mmd2,  mmd1,   0,      mmd2,    mmd2,  mmd2_state, init_mmd2,  "E&L Instruments Inc", "MMD-2 Mini-Micro Designer",  MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
