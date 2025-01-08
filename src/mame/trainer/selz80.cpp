// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

SEL Z80 Trainer (LEHRSYSTEME)

2010-08-31 Skeleton driver.
2011-06-23 Working [Robbbert]

No diagram has been found. The following is guesswork.

Test sequence: Press -, enter an address, press = to show contents, press
               up/down-arrow to cycle through addresses.

Paste test: -=11H22H33H44H55H66H77H88H99HK-1000=
Now press UP to see that the data has been entered.

ToDo:
- Keys are a guess, need to be confirmed.
- Needs to be tested by a subject-matter expert.
- i8255 to be added (address is unknown)
- "Tape-Interface" to be added (has its own LED)
- "Binary" area to be added - has 8 slide switches and a LED for each
- Halt LED
- "User Display" to be added - has 6 digits and a 74C917 chip
- 3 large sockets labelled "E C B - BUS"

- Unknown I/O:
'maincpu' (00F2): unmapped i/o memory write to 000C = 00 & FF
'maincpu' (00F8): unmapped i/o memory write to 0010 = FF & FF

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/i8279.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "video/pwm.h"
#include "selz80.lh"


namespace {

class selz80_state : public driver_device
{
public:
	selz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "ram")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_clock(*this, "uart_clock")
		, m_display(*this, "display")
	{ }

	void selz80(machine_config &config);

protected:
	void scanlines_w(uint8_t data);
	void digit_w(uint8_t data);
	uint8_t kbd_r();

	void selz80_io(address_map &map) ATTR_COLD;

	u8 m_digit = 0U;
	u8 m_seg = 0U;
	void setup_baud();
	void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<u8> m_p_ram;
	required_ioport_array<4> m_io_keyboard;
	required_device<clock_device> m_clock;
	required_device<pwm_display_device> m_display;

private:
	void selz80_mem(address_map &map) ATTR_COLD;
	void machine_reset() override ATTR_COLD;

};

class dagz80_state : public selz80_state
{
public:
	using selz80_state::selz80_state;
	void dagz80(machine_config &config);

private:
	void dagz80_mem(address_map &map) ATTR_COLD;
	void machine_reset() override ATTR_COLD;
};

void dagz80_state::dagz80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram().share("ram");
	map(0xe000, 0xffff).ram();
}

void selz80_state::selz80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x27ff).ram(); // all 3 RAM sockets filled
	// map(0x3000, 0x37ff).rom();  // empty socket for ROM
	map(0xa000, 0xffff).rom();
}

void selz80_state::selz80_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0x18, 0x19).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

/* Input ports */
static INPUT_PORTS_START( selz80 )
/* 2 x 16-key pads
RS SS RN MM     C(IS)  D(FL)  E(FL') F
EX BP TW TR     8(IX)  9(IY)  A(PC)  B(SP)
RL IN -  +      4(AF') 5(BC') 6(DE') 7(HL')
RG EN SA SD     0(AF)  1(BC)  2(DE)  3(HL)
  */
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E FL'") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D FL") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C IS") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RN") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RS") PORT_CODE(KEYCODE_T) PORT_CHAR('T')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B SP") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A PC") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 IY") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 IX") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TR (tape read)") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TW (tape write)") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BP") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EX") PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 HL'") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 DE'") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 BC'") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 AF'") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("IN") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RL") PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 HL") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 DE") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 BC") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 AF") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SD") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EN") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RG") PORT_CODE(KEYCODE_L) PORT_CHAR('L')

	PORT_START("BJ") // baud jumper
	/* not connected to cpu, each pair of pins is connected directly to the output
	   of a 4020 counter dividing the ???? clock to feed the 8251. You use a jumper
	   (like the kind on the back of a IDE hard drive) to choose the speed. */
	PORT_DIPNAME( 0x0F, 0x00, "Baud Rate" )
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x01, "4800" )
	PORT_DIPSETTING(    0x02, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x04, "600" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x06, "150" )
	PORT_DIPSETTING(    0x07, "75" )
	PORT_DIPSETTING(    0x08, "38" ) // 37.5
	PORT_DIPSETTING(    0x09, "18" ) // 17.75
	PORT_DIPSETTING(    0x0A, "9" )  // 8.875
INPUT_PORTS_END


void selz80_state::setup_baud()
{
	// setup baud rate for uart
	u8 baudsw = ioport("BJ")->read() & 15;
	if (baudsw)
	{
		u32 speed = (9600*16) >> baudsw;
		m_clock->set_unscaled_clock(speed);
	}
}

void selz80_state::machine_start()
{
	save_item(NAME(m_digit));
	save_item(NAME(m_seg));
}

void selz80_state::machine_reset()
{
	setup_baud();
}

void dagz80_state::machine_reset()
{
	setup_baud();
	uint8_t* rom = memregion("user1")->base();
	uint16_t size = memregion("user1")->bytes();
	memcpy(m_p_ram, rom, size);
	m_maincpu->reset();
}

void selz80_state::scanlines_w(uint8_t data)
{
	m_digit = data;
	m_display->matrix(1 << m_digit, m_seg);
}

void selz80_state::digit_w(uint8_t data)
{
	m_seg = bitswap<8>(data, 3, 2, 1, 0, 7, 6, 5, 4);
	m_display->matrix(1 << m_digit, m_seg);
}

uint8_t selz80_state::kbd_r()
{
	uint8_t data = 0xff;

	if ((m_digit & 7) < 4)
		data = m_io_keyboard[m_digit & 3]->read();

	return data;
}

void selz80_state::selz80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4.9152_MHz_XTAL / 2); // NEC uPD780C-1 cpu
	m_maincpu->set_addrmap(AS_PROGRAM, &selz80_state::selz80_mem);
	m_maincpu->set_addrmap(AS_IO, &selz80_state::selz80_io);

	/* video hardware */
	config.set_default_layout(layout_selz80);
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);

	/* Devices */
	CLOCK(config, m_clock, 153'600);
	m_clock->signal_handler().set("uart", FUNC(i8251_device::write_txc));
	m_clock->signal_handler().append("uart", FUNC(i8251_device::write_rxc));

	i8251_device &uart(I8251(config, "uart", 0));
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart", FUNC(i8251_device::write_cts));

	i8279_device &kbdc(I8279(config, "i8279", 4.9152_MHz_XTAL / 2)); // based on divider
	kbdc.out_sl_callback().set(FUNC(selz80_state::scanlines_w));    // scan SL lines
	kbdc.out_disp_callback().set(FUNC(selz80_state::digit_w));      // display A&B
	kbdc.in_rl_callback().set(FUNC(selz80_state::kbd_r));           // kbd RL lines
	kbdc.in_shift_callback().set_constant(1);                       // Shift key
	kbdc.in_ctrl_callback().set_constant(1);
}

void dagz80_state::dagz80(machine_config &config)
{
	selz80(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &dagz80_state::dagz80_mem);
}


/* ROM definition */
ROM_START( selz80 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v3.3", "V3.3")
	ROMX_LOAD( "z80-trainer.rom", 0x0000, 0x1000, CRC(eed1755f) SHA1(72e6ebfccb0e50034660bc36db1a741932311ce1), ROM_BIOS(0)) // (c)TEL86/V3.3
	ROM_SYSTEM_BIOS(1, "v3.2", "V3.2")
	ROMX_LOAD( "moniz80_3.2_04.12.1985.bin", 0x0000, 0x1000, CRC(3a3cf574) SHA1(ba6cd2276ce66f3a4545baf4d396f6c06d51dc38), ROM_BIOS(1)) // (c)SEL85/V3.2
	ROM_LOAD( "term80-a000.bin", 0xa000, 0x2000, CRC(0a58c0a7) SHA1(d1b4b3b2ad0d084175b1ff6966653d8b20025252))
	ROM_LOAD( "term80-e000.bin", 0xe000, 0x2000, CRC(158e08e6) SHA1(f1add43bcf8744a01238fb893ee284872d434db5))
ROM_END

ROM_START( dagz80 )
	ROM_REGION( 0x2000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "moni_1.5_15.08.1988.bin", 0x0000, 0x2000, CRC(318aee6e) SHA1(c698fdee401b88e673791aabcba6a9628938a075) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME           FLAGS
COMP( 1985, selz80, 0,      0,      selz80,  selz80, selz80_state, empty_init, "SEL",   "SEL Z80 Trainer", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1988, dagz80, selz80, 0,      dagz80,  selz80, dagz80_state, empty_init, "DAG",   "DAG Z80 Trainer", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
