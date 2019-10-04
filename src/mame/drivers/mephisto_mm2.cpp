// license:LGPL-2.1+
// copyright-holders:Dirk Verwiebe, Cowering
/******************************************************************************
 Mephisto 4 + 5 Chess Computer
 2007 Dirk V.

CPU 65C02 P4 (also seen: R65C02P3, G65SC02P-4)
Clock 4.9152 MHz
NMI CLK 600 Hz
IRQ Line is set to VSS
8 KByte  SRAM Sony CXK5864-15l

1-CD74HC4060E   14 Bit Counter
1-CD74HC166E
1-CD74HC251E
1-SN74HC138N TI
1-SN74HC139N TI
1-74HC14AP Toshiba
1-74HC02AP Toshiba
1-74HC00AP Toshiba
1-CD74HC259E

LCD module:
PCB label HGS 10 122 01
1-CD4011
4-CD4015

$0000-$1fff   S-RAM
$2000 LCD 4 Byte Shift Register writeonly right to left
every 2nd char xor'd by $FF

2c00-2c07 Keyboard (8to1 Multiplexer) 74HCT251
2*8 Matrix
Adr. 0x3407
==0 !=0
2c00 CL E5
2c01 POS F6
2c02 MEMO G7
2c03 INFO A1
2c04 LEV H8
2c05 ENT B2
2c06 >0 C3
2c07 <9 D4

$3400-$3407 LED 1-6, Buzzer, Keyboard select

$2400 // Chess Board
$2800 // Chess Board
$3000 // Chess Board

$4000-7FFF Opening Modul HG550
$8000-$FFF ROM

Mephisto 4 Turbo Kit 18mhz - (mm4tk)
    This is a replacement rom combining the turbo kit initial rom with the original MM IV.
    The Turbo Kit powers up to it's tiny rom, copies itself to ram, banks in normal rom,
    copies that to faster SRAM, then patches the checksum and the LED blink delays.
    If someone else wants to code up the power up banking, feel free

    There is an undumped MM V Turbo Kit, which will be the exact same except for location of
    the patches. The mm5tk just needs the normal mm5 ROM swapped out for that one to
    blinks the LEDs a little slower.

    -- Cowering (2011)

***********************************************************************************************/


#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/74259.h"
#include "machine/mmboard.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "speaker.h"

// internal artwork
#include "mephisto_mm2.lh"
#include "mephisto_bup.lh"


class mephisto_state : public driver_device
{
public:
	mephisto_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_outlatch(*this, "outlatch")
		, m_dac(*this, "dac")
		, m_key1(*this, "KEY1_%u", 0U)
		, m_key2(*this, "KEY2_%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	void rebel5(machine_config &config);
	void mm4(machine_config &config);
	void mm4tk(machine_config &config);
	void mm5(machine_config &config);
	void mm2(machine_config &config);
	void bup(machine_config &config);

private:
	required_device<m65c02_device> m_maincpu;
	required_device<hc259_device> m_outlatch;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8> m_key1;
	required_ioport_array<8> m_key2;
	output_finder<4> m_digits;

	DECLARE_WRITE8_MEMBER(write_lcd);
	DECLARE_WRITE8_MEMBER(mephisto_nmi_w);
	DECLARE_READ8_MEMBER(read_keys);
	DECLARE_WRITE_LINE_MEMBER(write_led7);
	uint8_t m_lcd_shift_counter;
	uint8_t m_led7;
	uint8_t m_allowNMI;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(update_nmi);
	TIMER_DEVICE_CALLBACK_MEMBER(update_nmi_r5);

	void bup_mem(address_map &map);
	void mm2_mem(address_map &map);
	void mm4_mem(address_map &map);
	void rebel5_mem(address_map &map);
};

void mephisto_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_lcd_shift_counter));
	save_item(NAME(m_led7));
	save_item(NAME(m_allowNMI));
}

void mephisto_state::machine_reset()
{
	m_lcd_shift_counter = 3;
	m_allowNMI = 1;
	m_led7 = 0xff;
}



/******************************************************************************
    I/O
******************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(mephisto_state::update_nmi)
{
	if (m_allowNMI)
	{
		m_allowNMI = 0;
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(mephisto_state::update_nmi_r5)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE8_MEMBER(mephisto_state::write_lcd)
{
	if (m_led7 == 0)
		m_digits[m_lcd_shift_counter] = data; // 0x109 MM IV // 0x040 MM V

	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;

	m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER(mephisto_state::mephisto_nmi_w)
{
	m_allowNMI = 1;
}

READ8_MEMBER(mephisto_state::read_keys)
{
	uint8_t data = 0;

	if (!m_outlatch->q7_r())
	{
		data = m_key1[offset]->read();
	}
	else
	{
		data = m_key2[offset]->read();
	}

	return data | 0x7f;
}

WRITE_LINE_MEMBER(mephisto_state::write_led7)
{
	m_led7 = state ? 0x00 : 0xff;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void mephisto_state::bup_mem(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1007).w("outlatch", FUNC(hc259_device::write_d7));
	map(0x1800, 0x1807).r(FUNC(mephisto_state::read_keys));
	map(0x2000, 0x2000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x2800, 0x2800).w(FUNC(mephisto_state::write_lcd));
	map(0x3000, 0x3000).w("board", FUNC(mephisto_board_device::led_w));
	map(0x3800, 0x3800).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x8000, 0xffff).rom();
}

void mephisto_state::mm2_mem(address_map &map)
{
	bup_mem(map);
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom)); // opening library
}

void mephisto_state::rebel5_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).w("outlatch", FUNC(hc259_device::write_d7));
	map(0x3000, 0x4000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x3000, 0x3007).r(FUNC(mephisto_state::read_keys));
	map(0x5000, 0x5000).w(FUNC(mephisto_state::write_lcd));
	map(0x6000, 0x6000).w("board", FUNC(mephisto_board_device::led_w));
	map(0x7000, 0x7000).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x8000, 0xffff).rom();
}

void mephisto_state::mm4_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).w(FUNC(mephisto_state::write_lcd));
	map(0x2400, 0x2407).w("board", FUNC(mephisto_board_device::led_w));
	map(0x2800, 0x2800).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x2c00, 0x2c07).r(FUNC(mephisto_state::read_keys));
	map(0x3000, 0x3000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x3400, 0x3407).w("outlatch", FUNC(hc259_device::write_d7));
	map(0x3800, 0x3800).w(FUNC(mephisto_state::mephisto_nmi_w));
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( mephisto )
	PORT_START("KEY1_0") //Port $2c00
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_START("KEY1_1") //Port $2c01
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_O)
	PORT_START("KEY1_2") //Port $2c02
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)
	PORT_START("KEY1_3") //Port $2c03
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_START("KEY1_4") //Port $2c04
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_START("KEY1_5") //Port $2c05
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_START("KEY1_6") //Port $2c06
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)
	PORT_START("KEY1_7") //Port $2c07
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)

	PORT_START("KEY2_0") //Port $2c08
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E / 5 / Queen") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_START("KEY2_1") //Port $2c09
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F / 6 / King") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_START("KEY2_2") //Port $2c0a
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("G / 7") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_START("KEY2_3") //Port $2c0b
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A / 1 / Pawn") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_START("KEY2_4") //Port $2c0c
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_START("KEY2_5") //Port $2c0d
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B / 2 / Knight") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_START("KEY2_6") //Port $2c0e
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C / 3 / Bishop") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_START("KEY2_7") //Port $2c0f
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D / 4 / Rook") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 1") PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mephisto_state, reset_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 2") PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mephisto_state, reset_button, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( bup )
	PORT_START("KEY1_0")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1 / Pawn") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_START("KEY1_1")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2 / Knight") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_START("KEY1_2")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3 / Bishop") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_START("KEY1_3")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4 / Rook") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_START("KEY1_4")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5 / Queen") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_START("KEY1_5")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6 / King") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_START("KEY1_6")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("White") PORT_CODE(KEYCODE_W)
	PORT_START("KEY1_7")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Black") PORT_CODE(KEYCODE_B)

	PORT_START("KEY2_0")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_START("KEY2_1")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_START("KEY2_2")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("BEST") PORT_CODE(KEYCODE_S)
	PORT_START("KEY2_3")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_START("KEY2_4")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MON") PORT_CODE(KEYCODE_N)
	PORT_START("KEY2_5")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_O)
	PORT_START("KEY2_6")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_START("KEY2_7")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 1") PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mephisto_state, reset_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 2") PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mephisto_state, reset_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(mephisto_state::reset_button)
{
	// RES buttons in serial tied to CPU RESET
	if (ioport("RESET")->read() == 3)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		machine_reset();
	}
}



/******************************************************************************
    Machine Drivers
******************************************************************************/

void mephisto_state::rebel5(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 9.8304_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_state::rebel5_mem);

	TIMER(config, "nmi_timer").configure_periodic(FUNC(mephisto_state::update_nmi_r5), attotime::from_hz(600));

	HC259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set_output("led100");
	m_outlatch->q_out_cb<1>().set_output("led101");
	m_outlatch->q_out_cb<2>().set_output("led102");
	m_outlatch->q_out_cb<3>().set_output("led103");
	m_outlatch->q_out_cb<4>().set_output("led104");
	m_outlatch->q_out_cb<5>().set_output("led105");
	m_outlatch->q_out_cb<6>().set(m_dac, FUNC(dac_bit_interface::write));
	m_outlatch->q_out_cb<7>().set(FUNC(mephisto_state::write_led7));

	MEPHISTO_SENSORS_BOARD(config, "board");
	config.set_default_layout(layout_mephisto_mm2);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void mephisto_state::mm4(machine_config &config)
{
	rebel5(config);

	/* basic machine hardware */
	m_maincpu->set_clock(4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_state::mm4_mem);

	TIMER(config.replace(), "nmi_timer").configure_periodic(FUNC(mephisto_state::update_nmi), attotime::from_hz(600));

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "mephisto_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("mephisto_mm4");
}

void mephisto_state::mm4tk(machine_config &config)
{
	mm4(config);
	m_maincpu->set_clock(18000000);
}

void mephisto_state::mm5(machine_config &config)
{
	mm4(config);
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("mephisto_mm5");
}

void mephisto_state::bup(machine_config &config)
{
	rebel5(config);

	/* basic machine hardware */
	m_maincpu->set_clock(7.3728_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_state::bup_mem);

	config.device_remove("nmi_timer");
	const attotime irq_period = attotime::from_hz(7.3728_MHz_XTAL / 2 / 0x2000); // 450Hz from 4020 Q13
	m_maincpu->set_periodic_int(FUNC(mephisto_state::irq0_line_assert), irq_period);

	m_outlatch->q_out_cb<7>().set(FUNC(mephisto_state::write_led7)).invert();

	config.set_default_layout(layout_mephisto_bup);
}

void mephisto_state::mm2(machine_config &config)
{
	bup(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_state::mm2_mem);

	config.set_default_layout(layout_mephisto_mm2);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "mephisto_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("mephisto_mm2");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START(rebel5)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS( 0, "v1", "V1" )
	ROMX_LOAD("rebel5_v1.rom", 0x8000, 0x8000, CRC(8d02e1ef) SHA1(9972c75936613bd68cfd3fe62bd222e90e8b1083), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v2", "V2" )
	ROMX_LOAD("rebel5_v2.rom", 0x8000, 0x8000, CRC(17232752) SHA1(3cd6893c0071f3dc02785bf99f1950eed81eba39), ROM_BIOS(1))
ROM_END

ROM_START(mm2)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_DEFAULT_BIOS("v42") // no official versioning, chronological order is assumed here from hex compare
	ROM_SYSTEM_BIOS( 0, "v1", "V1 (2 EPROMs)" )
	ROMX_LOAD("mm2_v1_1.bin", 0x8000, 0x4000, CRC(b91dab77) SHA1(67762304afe51fb8f1eb91259567b2451bf9bbfd), ROM_BIOS(0))
	ROMX_LOAD("mm2_v1_2.bin", 0xc000, 0x4000, CRC(01143cc1) SHA1(f78474b410dbecb209aa23ef81e9f894e8b54942), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v2", "V2 (2 EPROMs)" )
	ROMX_LOAD("mm2_v2_1.bin", 0x8000, 0x4000, CRC(e2daac82) SHA1(c9fa59ca92362f8ee770733073bfa2ab8c7904ad), ROM_BIOS(1))
	ROMX_LOAD("mm2_v2_2.bin", 0xc000, 0x4000, CRC(5e296939) SHA1(badd2a377259cf738cd076d8fb245c3dc284c24d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v3", "V3 (1 EPROM)" )
	ROMX_LOAD("300",          0x8000, 0x8000, CRC(60c777d4) SHA1(a77d678be60094073275558b4e8f0d34b43dd9ae), ROM_BIOS(2)) // D27C256D-20
	ROM_SYSTEM_BIOS( 3, "v41", "V4 (1 EPROM)" ) // v41 and v42 are binary identical
	ROMX_LOAD("400",          0x8000, 0x8000, CRC(e8c1f431) SHA1(c32dfa66eefbf3e539438d2fe6e6916f78a128be), ROM_BIOS(3)) // HN27C256G-20
	ROM_SYSTEM_BIOS( 4, "v42", "V4 (2 EPROMs)" )
	ROMX_LOAD("mm2_v4_1.bin", 0x8000, 0x4000, CRC(e9adcb8f) SHA1(498f48f96678f7bf429fd43e4f392ec6dd649fc6), ROM_BIOS(4))
	ROMX_LOAD("mm2_v4_2.bin", 0xc000, 0x4000, CRC(d40cbfc2) SHA1(4e9b19b1a0ad97868b31d7a55143a1778110cc96), ROM_BIOS(4))
ROM_END

ROM_START(bup)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS( 0, "v1", "V1" )
	ROMX_LOAD("bup_v1_1.bin", 0x8000, 0x4000, CRC(e1e9625a) SHA1(8a757e28b7afca2a092f8ff419087e06b07b743e), ROM_BIOS(0))
	ROMX_LOAD("bup_v1_2.bin", 0xc000, 0x4000, CRC(708338ea) SHA1(d617c4aa2161865a22b4b0646ba793f8a1fda863), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v2", "V2" )
	ROMX_LOAD("bup_v2_1.bin", 0x8000, 0x4000, CRC(e1e9625a) SHA1(8a757e28b7afca2a092f8ff419087e06b07b743e), ROM_BIOS(1))
	ROMX_LOAD("bup_v2_2.bin", 0xc000, 0x4000, CRC(6db30b80) SHA1(df4b379c4e916dff6b4110ec9c3591a9620c3424), ROM_BIOS(1))
ROM_END

ROM_START(mm4)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("mephisto4.rom", 0x8000, 0x8000, CRC(f68a4124) SHA1(d1d03a9aacc291d5cb720d2ee2a209eeba13a36c))
ROM_END

ROM_START(mm4tk)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("mm4tk.rom", 0x8000, 0x8000, CRC(51cb36a4) SHA1(9e184b4e85bb721e794b88d8657ae8d2ff5a24af))
ROM_END

ROM_START(mm5)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_DEFAULT_BIOS("v51")
	ROM_SYSTEM_BIOS( 0, "v50", "V5.0" )
	ROMX_LOAD("mm50.rom", 0x8000, 0x8000, CRC(fcfa7e6e) SHA1(afeac3a8c957ba58cefaa27b11df974f6f2066da), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v51", "V5.1" )
	ROMX_LOAD("mephisto5.rom", 0x8000, 0x8000, CRC(89c3d9d2) SHA1(77cd6f8eeb03c713249db140d2541e3264328048), ROM_BIOS(1))
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1984, mm2,    0,      0,      mm2,      mephisto, mephisto_state, empty_init, "Hegener + Glaser", "Mephisto MM II", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1985, bup,    0,      0,      bup,      bup,      mephisto_state, empty_init, "Hegener + Glaser", "Mephisto Blitz- und Problemloesungs-Modul", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, rebel5, 0,      0,      rebel5,   mephisto, mephisto_state, empty_init, "Hegener + Glaser", "Mephisto Rebell 5,0", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, mm4,    0,      0,      mm4,      mephisto, mephisto_state, empty_init, "Hegener + Glaser", "Mephisto MM IV", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, mm4tk,  mm4,    0,      mm4tk,    mephisto, mephisto_state, empty_init, "hack",             "Mephisto MM IV (TurboKit)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, mm5,    0,      0,      mm5,      mephisto, mephisto_state, empty_init, "Hegener + Glaser", "Mephisto MM V",  MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
