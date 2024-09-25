// license: BSD-3-Clause
// copyright-holders: natarii, Dirk Best
/***************************************************************************

    Gerät 32620 (aka Sprach/Morsegenerator, Stimme and more)

    Digital speech generator. Used by the Ministerium für Staatssicherheit
    to send coded messages to agents using shortwave "Number Stations".

    More info: https://www.cryptomuseum.com/spy/owvl/32620/index.htm

    Hardware:
    - UB880 (Z80)
    - D2764D
    - D446C x2
    - UB8560D
    - UB855D x2
    - 9.832 MHz XTAL
    - UB855D x2 (on expansion board)
    - MBM2764-20 x12 (can be less, on speech module)

    TODO:
    - Tape input
    - DAC gain
    - Verify daisy chain
    - Remote

    Notes:
    - Emulated is "version 2", it's possible that there are many hardware
      variations (the available documention is for a slighly different
      version for example)
    - Serial protocol is undocumented (settings: 1200 Baud, 8-O-1) and
      might not even be fully implemented in this version
    - It uses a 10-bit DAC, but only 8 bits are used

***************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "sound/dac.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "sprachmg.lh"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sprachmg_state : public driver_device
{
public:
	sprachmg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "pio%u", 0U),
		m_sio(*this, "sio"),
		m_speech_module(*this, "speech"),
		m_dac(*this, "dac"),
		m_keys(*this, "keypad%u", 0U),
		m_special(*this, "special"),
		m_remote(*this, "remote"),
		m_chargen(*this, "chargen"),
		m_dmd(*this, "dot%u%u", 0U, 0U),
		m_led_speech(*this, "led_speech"),
		m_led_morse(*this, "led_morse"),
		m_led_standard(*this, "led_standard"),
		m_display_data(0x00),
		m_key_scan(0x1f),
		m_speech_select(0xff),
		m_speech_module_pcb1(nullptr),
		m_speech_module_pcb2(nullptr)
		{ }

	DECLARE_INPUT_CHANGED_MEMBER(keypad_res);

	void sprachmg(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device_array<z80pio_device, 4> m_pio;
	required_device<z80sio_device> m_sio;
	required_device<generic_slot_device> m_speech_module;
	required_device<dac_word_interface> m_dac;
	required_ioport_array<5> m_keys;
	required_ioport m_special;
	required_ioport m_remote;
	required_region_ptr<uint8_t> m_chargen;
	output_finder<8, 7> m_dmd;
	output_finder<> m_led_speech;
	output_finder<> m_led_morse;
	output_finder<> m_led_standard;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void display_data_w(uint8_t data);
	void display_column_w(uint8_t data);

	uint8_t key_r();

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(module_load);
	uint8_t speech_r(offs_t offset);

	void sys_w(uint8_t data);

	uint8_t m_display_data;
	uint8_t m_key_scan;
	uint8_t m_speech_select;
	uint8_t *m_speech_module_pcb1;
	uint8_t *m_speech_module_pcb2;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void sprachmg_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2fff).ram().share("nvram");
	map(0x4000, 0xffff).r(FUNC(sprachmg_state::speech_r));
}

void sprachmg_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x7c, 0x7f).rw(m_pio[2], FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // on expansion board
	map(0xbc, 0xbf).rw(m_pio[3], FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // on expansion board
	map(0xc0, 0xc3).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0xc8, 0xcb).rw(m_pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xd0, 0xd3).rw(m_pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( sprachmg )
	PORT_START("keypad0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E)     PORT_NAME("EX")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("<")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME(">")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("keypad1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE)    PORT_NAME("SPC")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH)    PORT_NAME("/")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON)    PORT_NAME(":")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C)        PORT_NAME("CLR")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("keypad2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)     PORT_NAME("?")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3 -")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_M)     PORT_NAME("MODE")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("keypad3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("=")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD)  PORT_NAME("2 .")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD)  PORT_NAME("5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD)  PORT_NAME("8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_O)      PORT_NAME("OUT")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("keypad4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 NO")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1 YES")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I)     PORT_NAME("INP")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("special")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("STA/STP") PORT_WRITE_LINE_DEVICE_MEMBER("pio0", z80pio_device, pb5_w)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("remote")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("STA/STP (Remote)") PORT_WRITE_LINE_DEVICE_MEMBER("pio0", z80pio_device, pb6_w)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("reset")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RES") PORT_CHANGED_MEMBER(DEVICE_SELF, sprachmg_state, keypad_res, 0)
INPUT_PORTS_END


//**************************************************************************
//  KEYPAD
//**************************************************************************

INPUT_CHANGED_MEMBER( sprachmg_state::keypad_res )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	if (newval && !oldval)
		m_sio->reset();
}

uint8_t sprachmg_state::key_r()
{
	// 7-------  not used
	// -6------  start/stop on remote
	// --5-----  start/stop
	// ---43210  keypad selected row

	uint8_t data = 0xff;

	for (unsigned i = 0; i < 5; i++)
		if (BIT(m_key_scan, i) == 0)
			data &= m_keys[i]->read();

	data &= m_special->read();
	data &= m_remote->read();

	return data;
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void sprachmg_state::display_data_w(uint8_t data)
{
	m_display_data = data;

	m_pio[1]->strobe_b(1);
	m_pio[1]->strobe_b(0);
}

void sprachmg_state::display_column_w(uint8_t data)
{
	for (unsigned i = 0; i < 7; i++)
		m_dmd[data & 0x07][i] = m_chargen[0x100 + (i << 8) + m_display_data];
}


//**************************************************************************
//  SPEECH MODULE
//**************************************************************************

DEVICE_IMAGE_LOAD_MEMBER( sprachmg_state::module_load )
{
	if (!image.loaded_through_softlist())
		return std::make_pair(image_error::UNSUPPORTED, "Speech modules can only be loaded using a software list");

	uint32_t const pcb1_size = image.get_software_region_length("pcb1");
	if (pcb1_size != 0xc000)
		return std::make_pair(image_error::BADSOFTWARE, "Invalid 'pcb1' data area length (must be 48K)");

	m_speech_module_pcb1 = image.get_software_region("pcb1");

	uint32_t const pcb2_size = image.get_software_region_length("pcb2");
	if (pcb2_size > 0 && pcb2_size != 0xc000)
		return std::make_pair(image_error::BADSOFTWARE, "Invalid 'pcb2' data area length (must be 48K)");

	m_speech_module_pcb2 = image.get_software_region("pcb2");

	return std::make_pair(std::error_condition(), std::string());
}

uint8_t sprachmg_state::speech_r(offs_t offset)
{
	if (BIT(m_speech_select, 1) == 0 && m_speech_module_pcb1)
		return m_speech_module_pcb1[offset];

	if (BIT(m_speech_select, 0) == 0 && m_speech_module_pcb2)
		return m_speech_module_pcb2[offset];

	return 0xff;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void sprachmg_state::sys_w(uint8_t data)
{
	// 7-------  speech card 1 (active low)
	// -6------  speech card 2 (active low)
	// --5-----  led standard
	// ---4----  led speech
	// ----3---  led morse
	// -----210  data to remote (ende, ausgabe, morse)

	m_led_morse = BIT(data, 3);
	m_led_speech = BIT(data, 4);
	m_led_standard = BIT(data, 5);

	m_speech_select = BIT(data, 6, 2);
}

void sprachmg_state::machine_start()
{
	// resolve outputs
	m_dmd.resolve();
	m_led_speech.resolve();
	m_led_morse.resolve();
	m_led_standard.resolve();

	// register for save states
	save_item(NAME(m_display_data));
	save_item(NAME(m_key_scan));
	save_item(NAME(m_speech_select));
}

void sprachmg_state::machine_reset()
{
	m_key_scan = 0x1f;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static const z80_daisy_config z80_daisy_chain[] =
{
	{ "pio0" },
	{ "pio1" },
	{ "sio" },
	{ "pio2" },
	{ "pio3" },
	{ nullptr }
};

void sprachmg_state::sprachmg(machine_config &config)
{
	Z80(config, m_maincpu, 9.8304_MHz_XTAL / 4);
	m_maincpu->set_daisy_config(z80_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &sprachmg_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sprachmg_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	Z80PIO(config, m_pio[0], 9.8304_MHz_XTAL / 4);
	m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[0]->out_pa_callback().set([this](uint8_t data) { m_key_scan = data & 0x1f; });
	m_pio[0]->in_pb_callback().set(FUNC(sprachmg_state::key_r));

	Z80PIO(config, m_pio[1], 9.8304_MHz_XTAL / 4);
	m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[1]->out_pa_callback().set(FUNC(sprachmg_state::display_data_w));
	m_pio[1]->out_pb_callback().set(FUNC(sprachmg_state::display_column_w));

	Z80PIO(config, m_pio[2], 0);
	m_pio[2]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[2]->out_pa_callback().set([this](uint8_t data) { m_dac->write(data); });
	m_pio[2]->out_pb_callback().set(FUNC(sprachmg_state::sys_w));

	Z80PIO(config, m_pio[3], 0);
	m_pio[3]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	// port a: tape
	// port b: tape, dac gain

	Z80SIO(config, m_sio, 9.8304_MHz_XTAL / 4);
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_sio->out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &serial(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	serial.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));

	clock_device &serclk(CLOCK(config, "serclk", 9.8304_MHz_XTAL / 4 / 128)); // cd4020b
	serclk.signal_handler().set(m_sio, FUNC(z80sio_device::rxca_w));
	serclk.signal_handler().append(m_sio, FUNC(z80sio_device::txca_w));

	SPEAKER(config, "speaker").front_center();

	AD7520(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0);

	GENERIC_CARTSLOT(config, m_speech_module, generic_plain_slot, "sprachmg");
	m_speech_module->set_device_load(FUNC(sprachmg_state::module_load));

	SOFTWARE_LIST(config, "module_list").set_original("sprachmg");

	config.set_default_layout(layout_sprachmg);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( sprachmg )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("bu01_8-87.bin", 0x0000, 0x2000, CRC(7d9a92a6) SHA1(c9ca4a0d118b2c30e2505de051671769ad08a1c5))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("zg-625_1-12-86.bin", 0x000, 0x800, CRC(9ffd1e15) SHA1(759660404dfe479d13a1bdd4beb19e6035a34e17))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                         FULLNAME                               FLAGS
COMP( 1985, sprachmg, 0,      0,      sprachmg, sprachmg, sprachmg_state, empty_init, "Institut für Kosmosforschung", "Gerät 32620 (Sprach/Morsegenerator)", MACHINE_SUPPORTS_SAVE )
