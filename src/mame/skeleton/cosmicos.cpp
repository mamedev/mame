// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    COSMICOS

    http://retro.hansotten.nl/index.php?page=1802-cosmicos

    Press G to start, and to enable the debugger (if -debug used).
    The video options include 8-digit LEDs, 2-digit LEDs, and CRT,
    of which the default is the 8-digit LEDs. Unknown how to enable
    the others.

    HEX-monitor

    0 - start user program
    1 - inspect and/or change memory
    2 - write memory block to cassette
    3 - read memory block from cassette
    4 - move memory block
    5 - write memory block to EPROM
    C - start user program from address 0000

*/

/*

    TODO:

    - display interface INH
    - 2 segment display
    - single step
    - ascii monitor
    - PPI 8255
    - Floppy WD1793
    - COM8017 UART to printer

*/

#include "emu.h"

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "sound/cdp1864.h"
#include "sound/spkrdev.h"
#include "video/dm9368.h"
#include "screen.h"
#include "speaker.h"

#include "cosmicos.lh"

namespace {

#define CDP1802_TAG     "ic19"
#define CDP1864_TAG     "ic3"
#define DM9368_TAG      "ic10"
#define SCREEN_TAG      "screen"

enum
{
	LED_RUN = 0,
	LED_LOAD,
	LED_PAUSE,
	LED_RESET,
	LED_D7,
	LED_D6,
	LED_D5,
	LED_D4,
	LED_D3,
	LED_D2,
	LED_D1,
	LED_D0,
	LED_Q,
	LED_CASSETTE
};

class cosmicos_state : public driver_device
{
public:
	cosmicos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_digit(0),
		m_maincpu(*this, CDP1802_TAG),
		m_cti(*this, CDP1864_TAG),
		m_led(*this, DM9368_TAG),
		m_cassette(*this, "cassette"),
		m_speaker(*this, "speaker"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, CDP1802_TAG),
		m_key_row(*this, {"Y1", "Y2", "Y3", "Y4"}),
		m_io_data(*this, "DATA"),
		m_special(*this, "SPECIAL"),
		m_buttons(*this, "BUTTONS"),
		m_digits(*this, "digit%u", 0U),
		m_leds(*this, "led%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER( data );
	DECLARE_INPUT_CHANGED_MEMBER( enter );
	DECLARE_INPUT_CHANGED_MEMBER( single_step );
	DECLARE_INPUT_CHANGED_MEMBER( run );
	DECLARE_INPUT_CHANGED_MEMBER( load );
	DECLARE_INPUT_CHANGED_MEMBER( cosmicos_pause );
	DECLARE_INPUT_CHANGED_MEMBER( reset );
	DECLARE_INPUT_CHANGED_MEMBER( clear_data );
	DECLARE_INPUT_CHANGED_MEMBER( memory_protect );
	DECLARE_INPUT_CHANGED_MEMBER( memory_disable );

	void init_cosmicos();
	void cosmicos(machine_config &config);

private:
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t video_off_r();
	uint8_t video_on_r();
	void audio_latch_w(uint8_t data);
	uint8_t hex_keyboard_r();
	void hex_keylatch_w(uint8_t data);
	uint8_t reset_counter_r();
	void segment_w(uint8_t data);
	uint8_t data_r();
	void display_w(uint8_t data);
	uint8_t dma_r();
	void sc_w(uint8_t data);
	void set_cdp1802_mode(int mode);
	void clear_input_data();
	void dmaout_w(int state);
	void efx_w(int state);
	int wait_r();
	int clear_r();
	int ef1_r();
	int ef2_r();
	int ef3_r();
	int ef4_r();
	void q_w(int state);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(digit_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(int_tick);
	void cosmicos_io(address_map &map) ATTR_COLD;
	void cosmicos_mem(address_map &map) ATTR_COLD;

	/* CPU state */
	int m_wait = 0;
	int m_clear = 0;
	int m_sc1 = 0;

	/* memory state */
	uint8_t m_data = 0U;
	int m_boot = 0;
	int m_ram_protect = 0;
	int m_ram_disable = 0;

	/* keyboard state */
	uint8_t m_keylatch = 0U;

	/* display state */
	uint8_t m_segment = 0U;
	int m_digit = 0;
	int m_counter = 0;
	int m_q = 0;
	int m_dmaout = 0;
	int m_efx = 0;
	int m_video_on = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cosmac_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<dm9368_device> m_led;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_ioport_array<4> m_key_row;
	required_ioport m_io_data;
	required_ioport m_special;
	required_ioport m_buttons;
	output_finder<10> m_digits;
	output_finder<14> m_leds;
};

enum
{
	MODE_RUN,
	MODE_LOAD,
	MODE_PAUSE,
	MODE_RESET
};

/* Read/Write Handlers */

uint8_t cosmicos_state::read(offs_t offset)
{
	if (m_boot) offset |= 0xc000;

	uint8_t data = 0;

	if (offset < 0xc000)
	{
		// TODO
	}
	else if (offset < 0xd000)
	{
		data = m_rom->base()[offset & 0xfff];
	}
	else if (!m_ram_disable && (offset >= 0xff00))
	{
		data = m_ram->pointer()[offset & 0xff];
	}

	return data;
}

void cosmicos_state::write(offs_t offset, uint8_t data)
{
	if (m_boot) offset |= 0xc000;

	if (offset < 0xc000)
	{
		// TODO
	}
	else if (!m_ram_disable && !m_ram_protect && (offset >= 0xff00))
	{
		m_ram->pointer()[offset & 0xff] = data;
	}
}

uint8_t cosmicos_state::video_off_r()
{
	uint8_t data = 0;

	if (!m_q)
	{
		data = m_cti->dispoff_r();
	}

	return data;
}

uint8_t cosmicos_state::video_on_r()
{
	uint8_t data = 0;

	if (!m_q)
	{
		data = m_cti->dispon_r();
	}

	return data;
}

void cosmicos_state::audio_latch_w(uint8_t data)
{
	if (m_q)
	{
		m_cti->tone_latch_w(data);
	}
}

uint8_t cosmicos_state::hex_keyboard_r()
{
	uint8_t data = 0;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_keylatch, i))
		{
			uint8_t keydata = m_key_row[i]->read();

			if (BIT(keydata, 0)) data |= 0x01;
			if (BIT(keydata, 1)) data |= 0x02;
			if (BIT(keydata, 2)) data |= 0x04;
			if (BIT(keydata, 3)) data |= 0x06;
		}
	}

	return data;
}

void cosmicos_state::hex_keylatch_w(uint8_t data)
{
	m_keylatch = data & 0x0f;
}

uint8_t cosmicos_state::reset_counter_r()
{
	m_counter = 0;

	return 0;
}

void cosmicos_state::segment_w(uint8_t data)
{
	m_counter++;

	if (m_counter == 10)
	{
		m_counter = 0;
	}

	if ((m_counter > 0) && (m_counter < 9))
	{
		m_digits[10 - m_counter] = data;
	}
}

uint8_t cosmicos_state::data_r()
{
	return m_data;
}

void cosmicos_state::display_w(uint8_t data)
{
	m_segment = data;
}

/* Memory Maps */

void cosmicos_state::cosmicos_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(cosmicos_state::read), FUNC(cosmicos_state::write));
}

void cosmicos_state::cosmicos_io(address_map &map)
{
//  map(0x00, 0x00)
	map(0x01, 0x01).r(FUNC(cosmicos_state::video_on_r));
	map(0x02, 0x02).rw(FUNC(cosmicos_state::video_off_r), FUNC(cosmicos_state::audio_latch_w));
//  map(0x03, 0x03)
//  map(0x04, 0x04)
	map(0x05, 0x05).rw(FUNC(cosmicos_state::hex_keyboard_r), FUNC(cosmicos_state::hex_keylatch_w));
	map(0x06, 0x06).rw(FUNC(cosmicos_state::reset_counter_r), FUNC(cosmicos_state::segment_w));
	map(0x07, 0x07).rw(FUNC(cosmicos_state::data_r), FUNC(cosmicos_state::display_w));
}

/* Input Ports */

INPUT_CHANGED_MEMBER( cosmicos_state::data )
{
	uint8_t data = m_io_data->read();
	int i;

	for (i = 0; i < 8; i++)
	{
		if (!BIT(data, i))
		{
			m_data |= (1 << i);
			m_leds[LED_D0 - i] = 1;
		}
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::enter )
{
	if (!newval && !m_wait && !m_clear)
	{
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, ASSERT_LINE);
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::single_step )
{
	// if in PAUSE mode, set RUN mode until TPB=active
}

void cosmicos_state::set_cdp1802_mode(int mode)
{
	m_leds[LED_RUN] = 0;
	m_leds[LED_LOAD] = 0;
	m_leds[LED_PAUSE] = 0;
	m_leds[LED_RESET] = 0;

	switch (mode)
	{
	case MODE_RUN:
		m_leds[LED_RUN] = 1;

		m_wait = 1;
		m_clear = 1;
		break;

	case MODE_LOAD:
		m_leds[LED_LOAD] = 1;

		m_wait = 0;
		m_clear = 0;
		break;

	case MODE_PAUSE:
		m_leds[LED_PAUSE] = 1;

		m_wait = 1;
		m_clear = 0;
		break;

	case MODE_RESET:
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, CLEAR_LINE);
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, CLEAR_LINE);

		m_wait = 1;
		m_clear = 0;
		m_boot = 1;

		m_leds[LED_RESET] = 1;
		break;
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::run )             { if (!newval) set_cdp1802_mode(MODE_RUN); }
INPUT_CHANGED_MEMBER( cosmicos_state::load )            { if (!newval) set_cdp1802_mode(MODE_LOAD); }
INPUT_CHANGED_MEMBER( cosmicos_state::cosmicos_pause )  { if (!newval) set_cdp1802_mode(MODE_PAUSE); }
INPUT_CHANGED_MEMBER( cosmicos_state::reset )           { if (!newval) set_cdp1802_mode(MODE_RESET); }

void cosmicos_state::clear_input_data()
{
	int i;

	m_data = 0;

	for (i = 0; i < 8; i++)
	{
		m_leds[LED_D0 - i] = 0;
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::clear_data )
{
	clear_input_data();
}

INPUT_CHANGED_MEMBER( cosmicos_state::memory_protect )
{
	m_ram_protect = newval;
}

INPUT_CHANGED_MEMBER( cosmicos_state::memory_disable )
{
	m_ram_disable = newval;
}

static INPUT_PORTS_START( cosmicos )
	PORT_START("DATA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)

	PORT_START("BUTTONS")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, enter, 0)
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Single Step") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, single_step, 0)
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_NAME("Run") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, run, 0)
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("Load") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, load, 0)
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME(DEF_STR( Pause )) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, cosmicos_pause, 0)
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, reset, 0)
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Clear Data") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, clear_data, 0)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("Memory Protect") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, memory_protect, 0) PORT_TOGGLE
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("Memory Disable") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, memory_disable, 0) PORT_TOGGLE

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RET") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEC") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("REQ") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SEQ") PORT_CODE(KEYCODE_F4)
INPUT_PORTS_END

/* Video */

TIMER_DEVICE_CALLBACK_MEMBER(cosmicos_state::digit_tick)
{
	m_digit ^= 1;

	m_digits[m_digit] = m_segment;
}

TIMER_DEVICE_CALLBACK_MEMBER(cosmicos_state::int_tick)
{
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, ASSERT_LINE);
}

void cosmicos_state::dmaout_w(int state)
{
	m_dmaout = state;
}

void cosmicos_state::efx_w(int state)
{
	m_efx = state;
}

/* CDP1802 Configuration */

int cosmicos_state::wait_r()
{
	return m_wait;
}

int cosmicos_state::clear_r()
{
	return m_clear;
}

int cosmicos_state::ef1_r()
{
	uint8_t special = m_special->read();

	return BIT(special, 0);
}

int cosmicos_state::ef2_r()
{
	uint8_t special = m_special->read();
	int casin = (m_cassette)->input() < 0.0;

	m_leds[LED_CASSETTE] = casin;

	return BIT(special, 1) | BIT(special, 3) | casin;
}

int cosmicos_state::ef3_r()
{
	uint8_t special = m_special->read();

	return BIT(special, 2) | BIT(special, 3);
}

int cosmicos_state::ef4_r()
{
	return BIT(m_buttons->read(), 0);
}

void cosmicos_state::q_w(int state)
{
	/* cassette */
	m_cassette->output(state ? +1.0 : -1.0);

	/* boot */
	if (state) m_boot = 0;

	/* CDP1864 audio enable */
	m_cti->aoe_w(state);

	m_q = state;
}

uint8_t cosmicos_state::dma_r()
{
	return m_data;
}

void cosmicos_state::sc_w(uint8_t data)
{
	int sc1 = BIT(data, 1);

	if (m_sc1 && !sc1)
	{
		clear_input_data();
	}

	if (sc1)
	{
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, CLEAR_LINE);
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, CLEAR_LINE);
	}

	m_sc1 = sc1;
}

/* Machine Initialization */

void cosmicos_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	/* initialize LED display */
	m_led->rbi_w(1);

	/* register for state saving */
	save_item(NAME(m_wait));
	save_item(NAME(m_clear));
	save_item(NAME(m_sc1));
	save_item(NAME(m_data));
	save_item(NAME(m_boot));
	save_item(NAME(m_ram_protect));
	save_item(NAME(m_ram_disable));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_segment));
	save_item(NAME(m_digit));
	save_item(NAME(m_counter));
	save_item(NAME(m_q));
	save_item(NAME(m_dmaout));
	save_item(NAME(m_efx));
	save_item(NAME(m_video_on));
}

void cosmicos_state::machine_reset()
{
	set_cdp1802_mode(MODE_RESET);
}

/* Quickload */

QUICKLOAD_LOAD_MEMBER(cosmicos_state::quickload_cb)
{
	// FIXME: comment says "RAM" but data is loaded to ROM region - which should it be?
	int const size = image.length();
	if (size > m_rom->bytes())
	{
		return std::make_pair(
				image_error::INVALIDLENGTH,
				util::string_format("%u-byte image file is too large for %u-byte ROM", size, m_rom->bytes()));
	}

	uint8_t *const ptr = m_rom->base();

	// load image to RAM
	image.fread(ptr, size);

	return std::make_pair(std::error_condition(), std::string());
}

/* Machine Driver */

void cosmicos_state::cosmicos(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 1.75_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cosmicos_state::cosmicos_mem);
	m_maincpu->set_addrmap(AS_IO, &cosmicos_state::cosmicos_io);
	m_maincpu->wait_cb().set(FUNC(cosmicos_state::wait_r));
	m_maincpu->clear_cb().set(FUNC(cosmicos_state::clear_r));
	m_maincpu->ef1_cb().set(FUNC(cosmicos_state::ef1_r));
	m_maincpu->ef2_cb().set(FUNC(cosmicos_state::ef2_r));
	m_maincpu->ef3_cb().set(FUNC(cosmicos_state::ef3_r));
	m_maincpu->ef4_cb().set(FUNC(cosmicos_state::ef4_r));
	m_maincpu->q_cb().set(FUNC(cosmicos_state::q_w));
	m_maincpu->dma_rd_cb().set(FUNC(cosmicos_state::dma_r));
	m_maincpu->sc_cb().set(FUNC(cosmicos_state::sc_w));

	/* video hardware */
	config.set_default_layout(layout_cosmicos);
	DM9368(config, m_led, 0);
	TIMER(config, "digit").configure_periodic(FUNC(cosmicos_state::digit_tick), attotime::from_hz(100));
	TIMER(config, "interrupt").configure_periodic(FUNC(cosmicos_state::int_tick), attotime::from_hz(1000));

	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	CDP1864(config, m_cti, 1.75_MHz_XTAL).set_screen(SCREEN_TAG);
	m_cti->inlace_cb().set_constant(0);
	m_cti->int_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT);
	m_cti->dma_out_cb().set(FUNC(cosmicos_state::dmaout_w));
	m_cti->efx_cb().set(FUNC(cosmicos_state::efx_w));
	m_cti->rdata_cb().set_constant(1);
	m_cti->gdata_cb().set_constant(1);
	m_cti->bdata_cb().set_constant(1);
	m_cti->set_chrominance(RES_K(2), 0, 0, 0); // R2
	m_cti->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(cosmicos_state::quickload_cb));
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256").set_extra_options("4K,48K");
}

/* ROMs */

ROM_START( cosmicos )
	ROM_REGION( 0x1000, CDP1802_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "hex", "Hex Monitor" )
	ROMX_LOAD( "hex.ic6",   0x000, 0x400, BAD_DUMP CRC(d25124bf) SHA1(121215ba3a979e1962327ebe73cbadf784c568d9), ROM_BIOS(0) ) // typed in from manual
	ROMX_LOAD( "hex.ic7",   0x400, 0x400, BAD_DUMP CRC(364ac81b) SHA1(83936ee6a7ed44632eb290889b98fb9a500f15d4), ROM_BIOS(0) ) // typed in from manual
	ROM_SYSTEM_BIOS( 1, "ascii", "ASCII Monitor" )
	ROMX_LOAD( "ascii.ic6", 0x000, 0x400, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "ascii.ic7", 0x400, 0x400, NO_DUMP, ROM_BIOS(1) )
ROM_END

} // anonymous namespace

/* System Drivers */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY           FULLNAME    FLAGS
COMP( 1979, cosmicos, 0,      0,      cosmicos, cosmicos, cosmicos_state, empty_init, "Radio Bulletin", "Cosmicos", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
