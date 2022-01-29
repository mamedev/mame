// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    Pense Bem (TecToy 2017)
    driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>

---------------------------------------------------------------------

    In the 80s Tec Toy released Pense Bem in Brazil. It is most
    likely identical to VTech Smart Start since I think they run the
    exact same Z8 code. The only difference must be branding in the
    handheld case.

    In 2017 Tec Toy re-released Pense Bem in Brazil but this time
    using an Atmel ATMEGA168PB chip. It is not clear if the ATMEGA
    code contains a copy of the Z8 ROM and emulates the Z8, or if
    Tec Toy ported the original Z8 code retargetting it to the
    Atmel chip. Or even if they simply reimplemented the full
    functionality from scratch.

    Inspecting the ATMEGA disasm, it does not look like an emulation
    of the original Z8 code, but further research would be needed
    to be sure.

    As of October 2020, there's still no successfull ROM dump of the
    original Pense Bem's Z8 ROM code, so this driver only emulates
    the 2017 re-release.

---------------------------------------------------------------------

    The 2017 edition of TecToy's Pense Bem has a 2x4 programming
    pin-header at position CN4:

    CN4 - ATMEGA
      1 -  4 VCC
      2 - 15 MOSI
      3 - 31 TXD
      4 - 16 MISO
      5 - 30 RXD
      6 - 17 SCK
      7 -  5 GND
      8 - 29 RESET

    R34 is the pull-up resistor for the RESET signal

---------------------------------------------------------------------

    Changelog:

    2020 OCT 27 [Felipe Sanches]:
        * Fixed keyboard inputs, display & buzzer.
        * Implementation of AVR8 Timer 1 Output Compare Match A is
          sub-optimal resulting in bad sound quality when emulating
          the buzzer.

    2017 OCT 07 [Felipe Sanches]:
        * Initial driver skeleton

---------------------------------------------------------------------

== Notes about the hardware: ==
=== Select keypad rows: ===
keypad pin 1 - PORT_B5
keypad pin 2 - PORT_B3
keypad pin 12 - PORT_B2
keypad pin 13 - PORT_B4

=== Read keypad Columns: ===
keypad pin 3 - PORT_E0
keypad pin 4 - PORT_D2
keypad pin 5 - PORT_E1
keypad pin 6 - PORT_D7
keypad pin 7 - PORT_D6
keypad pin 8 - PORT_D5
keypad pin 9 - PORT_D4
keypad pin 10 - PORT_D3

=== Display digits: ===
digit_7 - PORT_E2
digit_6 - PORT_E3
digit_5 - PORT_C0
digit_4 - PORT_C1
digit_3 - PORT_C2
digit_2 - PORT_C3
digit_1 - PORT_C4
digit_0 - PORT_C5

=== Display segments: ===
In parentheses are the resistor reference numbers on
the PCB for each of these signals.

seg_7 (R11) - PORT_D7
seg_6 (R12) - PORT_D6
seg_5 (R17) - PORT_D5
seg_4 (R13) - PORT_D4
seg_3 (R14) - PORT_D3
seg_2 (R15) - PORT_D2
seg_1 (R18) - PORT_E1
seg_0 (R16) - PORT_E0

=== Piezo buzzer: ===
Port B, bit 1

--------------------------------------------------------------------- */

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "video/pwm.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"


class pensebem2017_state : public driver_device
{
public:
	pensebem2017_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_display(*this, "display"),
		m_keyb_rows(*this, "ROW%u", 0U)
	{
	}

	void pensebem2017(machine_config &config);

private:
	void prg_map(address_map &map);
	void data_map(address_map &map);

	uint8_t port_b_r();
	void port_b_w(uint8_t data);
	void port_c_w(uint8_t data);
	void port_d_w(uint8_t data);
	void port_e_w(uint8_t data);

	void update_display();

	uint8_t m_port_b;
	uint8_t m_port_c;
	uint8_t m_port_d;
	uint8_t m_port_e;

	required_device<avr8_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_keyb_rows;

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};

void pensebem2017_state::machine_start()
{
	save_item(NAME(m_port_b));
	save_item(NAME(m_port_c));
	save_item(NAME(m_port_d));
	save_item(NAME(m_port_e));
}

void pensebem2017_state::machine_reset()
{
	m_port_b = 0;
	m_port_c = 0;
	m_port_d = 0;
	m_port_e = 0;
}

uint8_t pensebem2017_state::port_b_r()
{
	uint8_t value = m_port_b & 0xc3;
	int bit;
	for (bit=0; bit<8; bit++)
	{
		if (bit < 2 && !BIT(m_port_e, bit)) break;
		if (bit >= 2 && !BIT(m_port_d, bit)) break;
	}
	if (BIT(m_keyb_rows[0]->read(), bit)) value |= (1 << 5);
	if (BIT(m_keyb_rows[1]->read(), bit)) value |= (1 << 3);
	if (BIT(m_keyb_rows[2]->read(), bit)) value |= (1 << 2);
	if (BIT(m_keyb_rows[3]->read(), bit)) value |= (1 << 4);
	return value;
}

void pensebem2017_state::port_b_w(uint8_t data) // buzzer + keyboard select rows
{
	m_port_b = data;
	m_dac->write(BIT(data, 1));
}

void pensebem2017_state::port_c_w(uint8_t data) // display
{
	m_port_c = data;
	update_display();
}

void pensebem2017_state::port_d_w(uint8_t data) // display
{
	m_port_d = data;
}

void pensebem2017_state::port_e_w(uint8_t data) // display
{
	m_port_e = data;
	update_display();
}

void pensebem2017_state::update_display()
{
	m_display->matrix(
		~bitswap<8>((m_port_c << 2 & 0xfc) | (m_port_e >> 2 & 0x03), 7,6,5,4,3,2,1,0),
		~((m_port_d & 0xfc) | (m_port_e & 0x03))
	);
}

void pensebem2017_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0); /* 16 kbytes of Flash ROM */
}

void pensebem2017_state::data_map(address_map &map)
{
	map(0x0000, 0x03ff).ram(); /* ATMEGA168PB Internal 1024 bytes of SRAM */
	map(0x0400, 0xffff).ram(); /* Some additional SRAM ? This is likely an exagerated amount ! */
}

static INPUT_PORTS_START( pensebem2017 )
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Desliga") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Livro") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D") PORT_CODE(KEYCODE_D)

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Adicao") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Subtracao") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Multiplicacao") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Aritmetica") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Divisao") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Adivinhe o Número") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Número do Meio") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Memória Tons") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Siga-me") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Operacao") PORT_CODE(KEYCODE_Y)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
INPUT_PORTS_END

void pensebem2017_state::pensebem2017(machine_config &config)
{
	/* CPU */
	ATMEGA168(config, m_maincpu, 16_MHz_XTAL); /* Actual chip is an Atmel ATMEGA168PB */
	m_maincpu->set_addrmap(AS_PROGRAM, &pensebem2017_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &pensebem2017_state::data_map);
	m_maincpu->set_eeprom_tag("eeprom");
	m_maincpu->set_low_fuses(0xf7);
	m_maincpu->set_high_fuses(0xdd);
	m_maincpu->set_extended_fuses(0xf9);
	m_maincpu->set_lock_bits(0x0f);
	m_maincpu->gpio_in<AVR8_IO_PORTB>().set(FUNC(pensebem2017_state::port_b_r));
	m_maincpu->gpio_out<AVR8_IO_PORTB>().set(FUNC(pensebem2017_state::port_b_w));
	m_maincpu->gpio_out<AVR8_IO_PORTC>().set(FUNC(pensebem2017_state::port_c_w));
	m_maincpu->gpio_out<AVR8_IO_PORTD>().set(FUNC(pensebem2017_state::port_d_w));
	m_maincpu->gpio_out<AVR8_IO_PORTE>().set(FUNC(pensebem2017_state::port_e_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(50);
	screen.set_size(1490, 1080);
	screen.set_visarea_full();
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
}

ROM_START( pbem2017 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_DEFAULT_BIOS("sept2017")

	/* September 2017 release */
	ROM_SYSTEM_BIOS(0, "sept2017", "SEPT/2017")
	ROMX_LOAD("pensebem-2017.bin", 0x0000, 0x35b6, CRC(d394279e) SHA1(5576599394231c1f83817dd55992e3b5838ab003), ROM_BIOS(0))

	/* on-die 4kbyte eeprom */
	ROM_REGION(0x1000, "eeprom", ROMREGION_ERASEFF)

	ROM_REGION(0x42e1a, "screen", 0)
	ROM_LOAD("pensebem.svg", 0, 0x42e1a, CRC(7146c0db) SHA1(966e95742acdda05028ee7b0c1352c88abb35041))
ROM_END

/*   YEAR  NAME    PARENT    COMPAT    MACHINE        INPUT         STATE                INIT         COMPANY    FULLNAME */
COMP(2017, pbem2017,    0,        0,   pensebem2017,  pensebem2017, pensebem2017_state,  empty_init,  "TecToy",  "Pense Bem (2017)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
