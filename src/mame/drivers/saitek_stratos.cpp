// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

Scisys Kasparov Stratos Chess Computer

TODO:
- emulate LCD at lower level, probably an MCU with embedded LCDC
- add LCD 7*7 DMD, it's in m_lcd_data[0x30 to 0x3b] but scrambled
- corona: different addressmap, 64 leds
- tking different internal artwork
- interrupt timing is derived from the main XTAL, but result should be similar with 5MHz and 5.67MHz,
  there are a couple of "FREQ. SEL" nodes on the PCB, maybe related (not the ones in input ports)
- does nvram.u7 work? it's cleared during boot, but not used after
- clean up driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist.h"
#include "speaker.h"

#include <algorithm>

// internal artwork
#include "saitek_stratos.lh" // clickable

class stratos_state : public driver_device
{
public:
	stratos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram.u7"),
		m_rombank(*this, "rombank"),
		m_nvrambank(*this, "nvrambank"),
		m_lcd_busy(*this, "lcd_busy"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_extrom(*this, "extrom"),
		m_out_digit(*this, "digit%u", 0U),
		m_out_lcd(*this, "lcd%u.%u.%u", 0U, 0U, 0U),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void stratos(machine_config &config);
	void corona(machine_config &config);
	void tking2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(cpu_freq) { set_cpu_freq(); }
	DECLARE_INPUT_CHANGED_MEMBER(acl_button) { if (newval) power_off(); }
	DECLARE_INPUT_CHANGED_MEMBER(go_button);

private:
	DECLARE_WRITE8_MEMBER(p2000_w);
	DECLARE_READ8_MEMBER(p2200_r);
	DECLARE_WRITE8_MEMBER(p2200_w);
	DECLARE_WRITE8_MEMBER(p2400_w);
	DECLARE_READ8_MEMBER(control_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(lcd_r);
	DECLARE_WRITE8_MEMBER(lcd_w);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(extrom_load);
	DECLARE_READ8_MEMBER(extrom_r);

	void main_map(address_map &map);

	void set_cpu_freq();

	std::unique_ptr<uint8_t[]> m_nvram_data;
	u8 m_select;
	u8 m_control;
	u8 m_led_data;
	bool m_power;

	u8 m_lcd_address;
	u8 m_lcd_count;
	u8 m_lcd_data[0x40];

	void update_leds();
	void update_lcd();
	void power_off();
	virtual void machine_reset() override;
	virtual void machine_start() override;

	required_device<m65c02_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_memory_bank m_rombank;
	required_memory_bank m_nvrambank;
	required_device<timer_device> m_lcd_busy;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_device<generic_slot_device> m_extrom;
	output_finder<8+1> m_out_digit;
	output_finder<4, 16, 4> m_out_lcd;
	required_ioport_array<8+1> m_inputs;
};


void stratos_state::machine_start()
{
	// resolve handlers
	m_out_digit.resolve();
	m_out_lcd.resolve();

	m_nvram_data = std::make_unique<uint8_t[]>(0x2000);
	m_nvram->set_base(m_nvram_data.get(), 0x2000);

	m_rombank->configure_entries(0, 2, memregion("maincpu")->base(), 0x8000);
	m_nvrambank->configure_entries(0, 2, m_nvram_data.get(), 0x1000);

	m_control = 0x00;
	m_select = 0x00;
}

void stratos_state::machine_reset()
{
	m_power = true;
	set_cpu_freq();
	m_rombank->set_entry(0);
	m_nvrambank->set_entry(0);
}

void stratos_state::set_cpu_freq()
{
	m_maincpu->set_unscaled_clock((ioport("FAKE")->read() & 1) ? 5.67_MHz_XTAL : 5_MHz_XTAL);
}

void stratos_state::update_leds()
{
	m_display->matrix_partial(0, 2, 1 << (m_control >> 5 & 1), (~m_led_data & 0xff) | (~m_control << 6 & 0x100));
}

INPUT_CHANGED_MEMBER(stratos_state::go_button)
{
	if (newval && !m_power)
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		machine_reset();
	}
}

DEVICE_IMAGE_LOAD_MEMBER(stratos_state::extrom_load)
{
	u32 size = m_extrom->common_get_size("rom");

	// 32KB ROM only?
	if (size != 0x8000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid file size");
		return image_init_result::FAIL;
	}

	m_extrom->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_extrom->common_load_rom(m_extrom->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

READ8_MEMBER(stratos_state::extrom_r)
{
	u16 bank = BIT(m_control, 1) * 0x4000;
	return (m_extrom->exists()) ? m_extrom->read_rom(offset | bank) : 0xff;
}


WRITE8_MEMBER(stratos_state::p2000_w)
{
	// d0-d3: input/led mux
	// d4-d7: chessboard led data
	m_select = data;
	m_display->matrix_partial(2, 4, ~m_select >> 4 & 0xf, 1 << (m_select & 0xf));
}

READ8_MEMBER(stratos_state::p2200_r)
{
	// d0-d7: chessboard sensors
	return ~m_board->read_file(m_select & 0xf);
}

WRITE8_MEMBER(stratos_state::p2200_w)
{
	m_dac->write(1);
}

WRITE8_MEMBER(stratos_state::p2400_w)
{
	// d0-d7: button leds data
	m_led_data = data;
	update_leds();

	m_dac->write(0); // guessed
}

READ8_MEMBER(stratos_state::control_r)
{
	u8 data = 0;
	u8 sel = m_select & 0xf;

	if (sel == 8)
	{
		// lcd busy flag?
		if (m_lcd_busy->enabled())
			data |= 0x20;

		// battery low
		data |= m_inputs[8]->read();
	}

	// read button panel
	if (sel < 8)
		data |= m_inputs[sel]->read() << 5;

	return data;
}

void stratos_state::power_off()
{
	m_power = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// clear display
	m_display->matrix(0, 0);

	std::fill_n(m_lcd_data, ARRAY_LENGTH(m_lcd_data), 0);
	update_lcd();
}

WRITE8_MEMBER(stratos_state::control_w)
{
	u8 prev = m_control;
	m_control = data;

	// d0: main rom bank
	// d1: ext rom bank
	// d1: nvram bank?
	m_rombank->set_entry(data >> 0 & 1);
	m_nvrambank->set_entry((data >> 1) & 1);

	// d2: mode led state
	// d5: button leds select
	update_leds();

	// d6 falling edge: power-off request
	if (~data & prev & 0x40)
		power_off();
}

void stratos_state::update_lcd()
{
	// output individual segments
	for (int i = 0; i < 0x40; i++)
		for (int j = 0; j < 4; j++)
			m_out_lcd[i >> 4][i & 0xf][j] = BIT(m_lcd_data[i], j);

	m_out_digit[0] = 0; // where?

	// upper digits
	for (int i = 0; i < 4; i++)
		m_out_digit[i + 1] = (m_lcd_data[0x01 + i * 2] << 4 | m_lcd_data[0x01 + i * 2 + 1]) & 0x7f;

	// lower digits
	for (int i = 0; i < 4; i++)
		m_out_digit[i + 5] = (m_lcd_data[0x11 + i * 2] << 4 | m_lcd_data[0x11 + i * 2 + 1]) & 0x7f;
}

READ8_MEMBER(stratos_state::lcd_r)
{
	// unknown, maybe resets lcd controller
	return 0;
}

WRITE8_MEMBER(stratos_state::lcd_w)
{
	// d0-d3: lcd data
	// d4-d7: unused?
	data &= 0xf;

	if (m_lcd_count == 0)
		m_lcd_address = data;
	else
	{
		// write to lcd row
		if (m_lcd_address > 0 && m_lcd_address <= 4)
			m_lcd_data[(((m_lcd_address - 1) << 4) + (m_lcd_count - 1)) & 0x3f] = data;
	}

	// it expects a specific number of writes for each row
	const u8 maxcount[5] = { 1, 9, 9, 1, 12 };
	if (m_lcd_address > 4 || m_lcd_count == maxcount[m_lcd_address])
	{
		m_lcd_count = 0;
		update_lcd();
	}
	else
		m_lcd_count++;

	m_lcd_busy->adjust(attotime::from_usec(50)); // ?
}

void stratos_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram.u6");
	map(0x2000, 0x2000).w(FUNC(stratos_state::p2000_w));
	map(0x2200, 0x2200).rw(FUNC(stratos_state::p2200_r), FUNC(stratos_state::p2200_w));
	map(0x2400, 0x2400).w(FUNC(stratos_state::p2400_w));
	map(0x2600, 0x2600).rw(FUNC(stratos_state::control_r), FUNC(stratos_state::control_w));
	map(0x2800, 0x37ff).bankrw("nvrambank");
	map(0x3800, 0x3800).rw(FUNC(stratos_state::lcd_r), FUNC(stratos_state::lcd_w));
	map(0x4000, 0x7fff).r(FUNC(stratos_state::extrom_r));
	map(0x8000, 0xffff).bankr("rombank");
}

static INPUT_PORTS_START( stratos )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Sound")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Play")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Function")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Library")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Info")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM) // freq sel

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Analysis")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Normal")

	PORT_START("IN.8")
	PORT_CONFNAME( 0x80, 0x80, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x80, DEF_STR( Normal ) )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CHANGED_MEMBER(DEVICE_SELF, stratos_state, go_button, nullptr) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, stratos_state, acl_button, nullptr) PORT_NAME("ACL")

	PORT_START("FAKE")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, stratos_state, cpu_freq, nullptr) // factory set
	PORT_CONFSETTING(    0x00, "5MHz" )
	PORT_CONFSETTING(    0x01, "5.67MHz" )
INPUT_PORTS_END

static INPUT_PORTS_START( tking2 )
	PORT_INCLUDE( stratos )

	PORT_MODIFY("IN.5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM)
INPUT_PORTS_END

void stratos_state::stratos(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 5_MHz_XTAL); // see set_cpu_freq
	m_maincpu->set_addrmap(AS_PROGRAM, &stratos_state::main_map);
	m_maincpu->set_periodic_int(FUNC(stratos_state::irq0_line_hold), attotime::from_hz(75));

	NVRAM(config, "nvram.u6", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram.u7", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(350));

	TIMER(config, "lcd_busy").configure_generic(timer_device::expired_delegate());

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2+4, 8+1);
	config.set_default_layout(layout_saitek_stratos);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* extension rom */
	GENERIC_CARTSLOT(config, m_extrom, generic_plain_slot, "saitek_egr", "bin");
	m_extrom->set_device_load(FUNC(stratos_state::extrom_load), this);

	SOFTWARE_LIST(config, "cart_list").set_original("saitek_egr");
}

void stratos_state::corona(machine_config &config)
{
	stratos(config);

	m_board->set_type(sensorboard_device::MAGNETS);
}

void stratos_state::tking2(machine_config &config)
{
	stratos(config);

	// seems much more responsive
	m_board->set_delay(attotime::from_msec(200));
}


ROM_START( stratos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w1y01f_728m_u3.u3", 0x0000, 0x8000, CRC(b58a7256) SHA1(75b3a3a65f4ca8d52aa5b17a06319bff59d9014f) )
	ROM_LOAD("bw1_819n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632) )
ROM_END

ROM_START( stratosa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w1y01f_728l_u3.u3", 0x0000, 0x8000, CRC(19a22058) SHA1(a5ca54d870c70b7ce9c7be2951800bf49cc57527) )
	ROM_LOAD("bw1_819n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632) )
ROM_END

ROM_START( tking ) // PCB rev. 10
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("y01f_713d_u3.u3", 0x0000, 0x8000, CRC(b8c6d853) SHA1(98923f44bbbd2ea17c269850971d3df229e6057e) )
	ROM_LOAD("y01f_712a_u4.u4", 0x8000, 0x8000, CRC(7d3f8f7b) SHA1(8be5d8d988ff0577ccfec0a773bfd94599f2534f) )
ROM_END

ROM_START( tkinga ) // PCB rev. 3, also PCB rev. 5
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w1y01f_728l_u3.u3", 0x0000, 0x8000, CRC(19a22058) SHA1(a5ca54d870c70b7ce9c7be2951800bf49cc57527) )
	ROM_LOAD("y01f-b_819o_u4.u4", 0x8000, 0x8000, CRC(336040d4) SHA1(aca662b8cc4d6bafd61ca158c768ba8896117169) )
ROM_END

ROM_START( tkingb ) // PCB rev. 7
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w1y01f_728p_u3.u3", 0x0000, 0x8000, CRC(ad77f83e) SHA1(598fdb1e40267d9d43a3d8f287723070b9afa349) )
	ROM_LOAD("y01f-b_819o_u4.u4", 0x8000, 0x8000, CRC(336040d4) SHA1(aca662b8cc4d6bafd61ca158c768ba8896117169) )
ROM_END

ROM_START( corona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w2_708g_u2.u2", 0x0000, 0x8000, CRC(52568bb4) SHA1(83fe91787e17bbefc2b3ec651ddb11c88990060d) )
	ROM_LOAD("bw2_708a_u3.u3", 0x8000, 0x8000, CRC(32848f73) SHA1(a447543e3eb4757f9afed26fde77b66985eb96a7) )
ROM_END

ROM_START( coronaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w2_a14c_u2.u2", 0x0000, 0x8000, CRC(be82e199) SHA1(cfcc573774b6907ed137dca01fa7f3fce493a89f) )
	ROM_LOAD("bw2_a14_u3.u3", 0x8000, 0x8000, CRC(abe87285) SHA1(b15f7ddeac78d252cf413ba4085523e44c6d15df) )
ROM_END


/*    YEAR  NAME      PARENT  CMP MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1986, stratos,  0,       0, stratos, stratos, stratos_state, empty_init, "SciSys", "Kasparov Stratos (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, stratosa, stratos, 0, stratos, stratos, stratos_state, empty_init, "SciSys", "Kasparov Stratos (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK )

CONS( 1990, tking,    0,       0, tking2,  tking2,  stratos_state, empty_init, "Saitek", "Kasparov Turbo King (set 1, ver. D)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK ) // aka Turbo King II
CONS( 1988, tkinga,   tking,   0, stratos, stratos, stratos_state, empty_init, "Saitek", "Kasparov Turbo King (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK ) // oldest?
CONS( 1988, tkingb,   tking,   0, stratos, stratos, stratos_state, empty_init, "Saitek", "Kasparov Turbo King (set 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK )

CONS( 1988, corona,   0,       0, corona,  stratos, stratos_state, empty_init, "Saitek", "Kasparov Corona (ver. D+)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK ) // aka Corona II
CONS( 1988, coronaa,  corona,  0, corona,  stratos, stratos_state, empty_init, "Saitek", "Kasparov Corona (ver. D)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK )
