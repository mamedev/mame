// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*************************************************************************

  This is a driver for a gambling board with a yet unknown name.
  The PCB is labeled with: WU- MARY-1A
  And there's a text string in the ROM that says: "Music by: SunKiss Chen"

  Driver by Felipe Sanches

  TODO:
  * Figure out where exactly all devices are mapped to (the devices are
    2 sound chips, the 2kb SRAM, the 8bit DIP switches,
    31 LEDs, 13 modules of double-digit 7-seg displays and 4 push-buttons).
  * we may also have user inputs from the coin slot and from the
    cabinet buttons, for making bets.
**************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "marywu.lh"


namespace {

class marywu_state : public driver_device
{
public:
	marywu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "DSW", "PUSHBUTTONS" })
	{ }

	void marywu(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);
	void ay1_port_a_w(uint8_t data);
	void ay1_port_b_w(uint8_t data);
	void ay2_port_a_w(uint8_t data);
	void ay2_port_b_w(uint8_t data);
	uint8_t keyboard_r();
	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

	uint8_t m_selected_7seg_module = 0;

	output_finder<32> m_digits;
	output_finder<30> m_leds;
	required_ioport_array<4> m_inputs;
};

static INPUT_PORTS_START( marywu )
	PORT_START("KEYS1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)

	PORT_START("KEYS2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:8")

	PORT_START("PUSHBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) // K0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) // K1
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) // K2
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) // K3
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void marywu_state::ay1_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);
}

void marywu_state::ay1_port_b_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(data, i);
}

void marywu_state::ay2_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 16] = BIT(data, i);
}

void marywu_state::ay2_port_b_w(uint8_t data)
{
	// we only have 30 LEDs. The last 2 bits in this port are unused.
	for (uint8_t i = 0; i < 6; i++)
		m_leds[i + 24] = BIT(data, i);
}

void marywu_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}

uint8_t marywu_state::keyboard_r()
{
	switch (m_selected_7seg_module & 0x07)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return m_inputs[m_selected_7seg_module & 0x07]->read();
	default:
		return 0x00;
	}
}

void marywu_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // HEF4511BP (7 seg display driver)

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void marywu_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void marywu_state::io_map(address_map &map)
{
	map(0x8000, 0x87ff).mirror(0x0800).ram().share("nvram"); /* HM6116: 2kbytes of Static RAM */
	map(0x9000, 0x9001).mirror(0x0ffc).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x9002, 0x9003).mirror(0x0ffc).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xb000, 0xb001).mirror(0x0ffe).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xf000, 0xf000).noprw(); /* TODO: Investigate this. There's something going on at this address range. */
}

void marywu_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();
}

void marywu_state::marywu(machine_config &config)
{
	// The 8279 (U23) and PSGs (U16 and U14) are clocked using the ALE output from the 8031.
	// This is normally 1/6th of the input clock frequency, but a pulse is skipped during external data memory accesses.
	// If the program accesses external data memory, sound generation and keyboard scanning will be affected.

	/* basic machine hardware */
	i80c31_device &maincpu(I80C31(config, "maincpu", XTAL(10'738'635))); //actual CPU is a Winbond w78c31b-24
	maincpu.set_addrmap(AS_PROGRAM, &marywu_state::program_map);
	maincpu.set_addrmap(AS_IO, &marywu_state::io_map);
	//TODO: figure out what each bit is mapped to in the 80c31 ports P1 and P3

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Keyboard & display interface */
	i8279_device &kbdc(I8279(config, "i8279", XTAL(10'738'635) / 6));
	kbdc.out_sl_callback().set(FUNC(marywu_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(marywu_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(marywu_state::display_7seg_data_w));

	/* Video */
	config.set_default_layout(layout_marywu);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(10'738'635) / 6));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay1.port_a_write_callback().set(FUNC(marywu_state::ay1_port_a_w));
	ay1.port_b_write_callback().set(FUNC(marywu_state::ay1_port_b_w));

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(10'738'635) / 6));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay2.port_a_write_callback().set(FUNC(marywu_state::ay2_port_a_w));
	ay2.port_b_write_callback().set(FUNC(marywu_state::ay2_port_b_w));
}

ROM_START( marywu )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "marywu_sunkiss_chen.rom", 0x0000, 0x8000, CRC(11f67c7d) SHA1(9c1fd1a5cc6e2b0d675f0217aa8ff21c30609a0c) )
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT   MACHINE   INPUT   STATE         INIT        ROT   COMPANY      FULLNAME                                                FLAGS
GAME( ????, marywu, 0,       marywu,   marywu, marywu_state, empty_init, ROT0, "<unknown>", "unknown Labeled 'WU- MARY-1A' Music by: SunKiss Chen", MACHINE_NOT_WORKING )
