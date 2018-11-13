// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#include "emu.h"
#include "hlekbd.h"

#include "machine/keyboard.ipp"
#include "speaker.h"

DEFINE_DEVICE_TYPE_NS(SGI_HLE_KEYBOARD, bus::sgikbd, hle_device, "hlekbd", "SGI Indigo Keyboard (HLE)")

namespace bus { namespace sgikbd {

namespace {

INPUT_PORTS_START( hle_device )
	PORT_START("ROW0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

} // anonymous namespace

hle_device::hle_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HLE_KEYBOARD, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_sgi_keyboard_port_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7")
	, m_click_timer(nullptr)
	, m_beep_timer(nullptr)
	, m_beeper(*this, "beeper")
	, m_leds(*this, "led%u", 0U)
	, m_make_count(0)
	, m_keyclick(true)
	, m_auto_repeat(false)
	, m_beeper_state(0)
	, m_led_state(0)
{
}

hle_device::~hle_device()
{
}

ioport_constructor hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hle_device);
}

WRITE_LINE_MEMBER(hle_device::input_txd)
{
	device_buffered_serial_interface::rx_w(state);
}

void hle_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "bell").front_center();
	BEEP(config, m_beeper, ATTOSECONDS_TO_HZ(480 * ATTOSECONDS_PER_MICROSECOND));
	m_beeper->add_route(ALL_OUTPUTS, "bell", 1.0);
}

void hle_device::device_start()
{
	m_leds.resolve();
	m_click_timer = timer_alloc(TIMER_CLICK);
	m_beep_timer = timer_alloc(TIMER_BEEP);

	save_item(NAME(m_make_count));
	save_item(NAME(m_keyclick));
	save_item(NAME(m_auto_repeat));
	save_item(NAME(m_beeper_state));
	save_item(NAME(m_led_state));
}

void hle_device::device_reset()
{
	// initialise state
	clear_fifo();
	m_make_count = 0;
	m_keyclick = true;
	m_auto_repeat = false;
	m_beeper_state = 0;
	m_led_state = 0;

	// configure device_buffered_serial_interface
	set_data_frame(START_BIT_COUNT, DATA_BIT_COUNT, PARITY, STOP_BITS);
	set_rate(BAUD);
	receive_register_reset();
	transmit_register_reset();

	// set device_sgi_keyboard_port_interface lines
	output_rxd(1);

	// start with keyboard LEDs off
	m_leds[LED_NUM] = 0;
	m_leds[LED_CAPS] = 0;
	m_leds[LED_SCROLL] = 0;
	m_leds[LED_USER1] = 0;
	m_leds[LED_USER2] = 0;
	m_leds[LED_USER3] = 0;
	m_leds[LED_USER4] = 0;

	m_click_timer->adjust(attotime::never);
	m_beep_timer->adjust(attotime::never);

	// kick the base
	reset_key_state();
	start_processing(attotime::from_hz(600));
}

void hle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CLICK:
		m_beeper_state &= ~BEEPER_CLICK;
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		break;

	case TIMER_BEEP:
		m_beeper_state &= ~BEEPER_BELL;
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		break;

	default:
		break;
	}
}

void hle_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void hle_device::tra_complete()
{
	if (fifo_full())
		start_processing(attotime::from_hz(600));

	device_buffered_serial_interface::tra_complete();
}

void hle_device::key_make(uint8_t row, uint8_t column)
{
	// we should have stopped processing if we filled the FIFO
	assert(!fifo_full());

	// send the make code, click if desired
	//transmit_byte((row << 4) | column);
	if (m_keyclick)
	{
		m_beeper_state |= uint8_t(BEEPER_CLICK);
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		m_click_timer->reset(attotime::from_msec(5));
	}

	// count keys
	++m_make_count;
	assert(m_make_count);
}

void hle_device::key_break(uint8_t row, uint8_t column)
{
	// we should have stopped processing if we filled the FIFO
	assert(!fifo_full());
	assert(m_make_count);

	// send the break code, and the idle code if no other keys are down
	//transmit_byte(0x80 | (row << 4) | column);
	//if (!--m_make_count)
		//transmit_byte(0xf0);
}

void hle_device::transmit_byte(uint8_t byte)
{
	device_buffered_serial_interface::transmit_byte(byte);
	if (fifo_full())
		stop_processing();
}

void hle_device::received_byte(uint8_t byte)
{
	if (BIT(byte, CTRL_B))
	{
		if (BIT(byte, CTRL_B_CMPL_DS1_2))
		{
			logerror("Not Yet Implemented: Complement DS1/2\n");
		}

		if (BIT(byte, CTRL_B_SCRLK))
		{
			logerror("Toggle Scroll Lock LED\n");
			m_led_state ^= (1 << LED_SCROLL);
			m_leds[LED_SCROLL] = BIT(m_led_state, LED_SCROLL);
		}
		if (BIT(byte, CTRL_B_L1))
		{
			logerror("Toggle User LED 1\n");
			m_led_state ^= (1 << LED_USER1);
			m_leds[LED_USER1] = BIT(m_led_state, LED_USER1);
		}
		if (BIT(byte, CTRL_B_L2))
		{
			logerror("Toggle User LED 2\n");
			m_led_state ^= (1 << LED_USER2);
			m_leds[LED_USER2] = BIT(m_led_state, LED_USER2);
		}
		if (BIT(byte, CTRL_B_L3))
		{
			logerror("Toggle User LED 3\n");
			m_led_state ^= (1 << LED_USER3);
			m_leds[LED_USER3] = BIT(m_led_state, LED_USER3);
		}
		if (BIT(byte, CTRL_B_L4))
		{
			logerror("Toggle User LED 4\n");
			m_led_state ^= (1 << LED_USER4);
			m_leds[LED_USER3] = BIT(m_led_state, LED_USER4);
		}
	}
	else
	{
		if (BIT(byte, CTRL_A_SBEEP))
		{
			logerror("Short Beep\n");
			m_beeper_state |= uint8_t(BEEPER_BELL);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			m_click_timer->adjust(attotime::from_msec(200));
		}

		if (BIT(byte, CTRL_A_LBEEP))
		{
			logerror("Long Beep\n");
			m_beeper_state |= uint8_t(BEEPER_BELL);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			m_click_timer->adjust(attotime::from_msec(1000));
		}

		if (BIT(byte, CTRL_A_NOCLICK))
		{
			logerror("Keyclick Off\n");
			m_keyclick = false;
		}

		if (BIT(byte, CTRL_A_RCB))
		{
			logerror("Receive Config Byte\n");
			transmit_byte(0x6e);
			transmit_byte(0x00); // US layout
		}

		if (BIT(byte, CTRL_A_NUMLK))
		{
			logerror("Toggle Num Lock LED\n");
			m_led_state ^= (1 << LED_NUM);
			m_leds[LED_NUM] = BIT(m_led_state, LED_NUM);
		}

		if (BIT(byte, CTRL_A_CAPSLK))
		{
			logerror("Toggle Caps Lock LED\n");
			m_led_state ^= (1 << LED_CAPS);
			m_leds[LED_CAPS] = BIT(m_led_state, LED_CAPS);
		}

		if (BIT(byte, CTRL_A_AUTOREP))
		{
			logerror("Toggle Auto Repeat\n");
			m_auto_repeat = !m_auto_repeat;
		}
	}
}

} } // namespace bus::sgikbd
