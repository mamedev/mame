// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
/*
    Ensoniq panel/display device
*/
#include "emu.h"
#include "ioport.h"
#include "esqpanel.h"

#include "vfx.lh"
#include "vfxsd.lh"
#include "sd1.lh"

#include "main.h"

#define VERBOSE 0
#include "logmacro.h"

#include <list>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ESQPANEL1X22,     esqpanel1x22_device,     "esqpanel122",     "Ensoniq front panel with 1x22 VFD")
DEFINE_DEVICE_TYPE(ESQPANEL2X40,     esqpanel2x40_device,     "esqpanel240",     "Ensoniq front panel with 2x40 VFD")
DEFINE_DEVICE_TYPE(ESQPANEL2X40_VFX, esqpanel2x40_vfx_device, "esqpanel240_vfx", "Ensoniq front panel with 2x40 VFD for VFX family")
DEFINE_DEVICE_TYPE(ESQPANEL2X16_SQ1, esqpanel2x16_sq1_device, "esqpanel216_sq1", "Ensoniq front panel with 2x16 LCD")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  esqpanel_device - constructor
//-------------------------------------------------

esqpanel_device::esqpanel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_light_states(),
	m_write_tx(*this),
	m_write_analog(*this),
	m_read_analog(*this, 0)
{
	std::fill(std::begin(m_xmitring), std::end(m_xmitring), 0);
	m_light_states.reserve(0x80);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void esqpanel_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void esqpanel_device::device_reset()
{
	device_t::device_reset();

	// panel comms is at 62500 baud (double the MIDI rate), 8N2
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rcv_rate(62500);
	set_tra_rate(62500);

	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;
	m_expect_calibration_second_byte = false;
	m_expect_light_second_byte = false;
}

//-------------------------------------------------
//  device_reset_after_children - device-specific reset after children
//-------------------------------------------------

void esqpanel_device::device_reset_after_children()
{
	device_t::device_reset_after_children();

	attotime sample_time(0, ATTOSECONDS_PER_MILLISECOND);
	attotime initial_delay(0, ATTOSECONDS_PER_MILLISECOND);

	m_blink_timer->adjust(initial_delay, 0, sample_time);
	m_blink_timer->enable(true);
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void esqpanel_device::device_stop()
{
	device_t::device_stop();
}

void esqpanel_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

//  if (data >= 0xe0) LOG("Got %02x from motherboard (second %s)\n", data, m_expect_calibration_second_byte ? "yes" : "no");

	send_to_display(data);

	if (m_expect_calibration_second_byte)
	{
//      LOG("second byte is %02x\n", data);
		if (data == 0xfd)   // calibration request
		{
//          LOG("let's send reply!\n");
			xmit_char(0xff);   // this is the correct response for "calibration OK"
		}
		m_expect_calibration_second_byte = false;
	}
	else if (m_expect_light_second_byte)
	{
		// Lights on the Buttons, on the VFX-SD:
		// Number   Button
		// 0        1-6
		// 1        8
		// 2        6
		// 3        4
		// 4        2
		// 5        Compare
		// 6        1
		// 7        Presets
		// 8        7-12
		// 9        9
		// a        7
		// b        5
		// c        3
		// d        Sounds
		// e        0
		// f        Cart
		int light_number = data & 0x3f;

		// Light states:
		// 0 = Off
		// 2 = On
		// 3 = Blinking
		m_light_states[light_number] = (data & 0xc0) >> 6;
		m_expect_light_second_byte = false;
	}
	else if (data == 0xfb)   // request calibration
	{
		m_expect_calibration_second_byte = true;
	}
	else if (data == 0xff)  // button light state command
	{
		m_expect_light_second_byte = true;
	}
	else
	{
		// EPS wants a throwaway reply byte for each byte sent to the KPC
		// VFX-SD and SD-1 definitely don't :)
		if (m_eps_mode)
		{
			if (data == 0xe7)
			{
				xmit_char(0x00);   // actual value of response is never checked
			}
			else if (data == 0x71)
			{
				xmit_char(0x00);   // actual value of response is never checked
			}
			else
			{
				xmit_char(data);   // actual value of response is never checked
			}
		}
	}
}

void esqpanel_device::tra_complete()    // Tx completed sending byte
{
//  LOG("panel Tx complete\n");
	// is there more waiting to send?
	if (m_xmit_read != m_xmit_write)
	{
		transmit_register_setup(m_xmitring[m_xmit_read++]);
		if (m_xmit_read >= XMIT_RING_SIZE)
		{
			m_xmit_read = 0;
		}
	}
	else
	{
		m_tx_busy = false;
	}
}

void esqpanel_device::tra_callback()    // Tx send bit
{
	m_write_tx(transmit_register_get_data_bit());
}

void esqpanel_device::xmit_char(uint8_t data)
{
//  LOG("Panel: xmit %02x\n", data);

	// if tx is busy it'll pick this up automatically when it completes
	if (!m_tx_busy)
	{
		m_tx_busy = true;
		transmit_register_setup(data);
	}
	else
	{
		// tx is busy, it'll pick this up next time
		m_xmitring[m_xmit_write++] = data;
		if (m_xmit_write >= XMIT_RING_SIZE)
		{
			m_xmit_write = 0;
		}
	}
}

void esqpanel_device::set_analog_value(offs_t offset, uint16_t value)
{
	m_write_analog(offset, value);
}

uint16_t esqpanel_device::get_analog_value(offs_t offset)
{
	return m_read_analog(offset, 0);
}

void esqpanel_device::set_button(uint8_t button, bool pressed)
{
	LOG("set_button(%d, %d)\r\n", button, pressed);
	bool current = m_pressed_buttons.find(button) != m_pressed_buttons.end();
	if (pressed == current)
	{
		LOG("- button %d already %d, skipping\r\n", button, pressed);
		return;
	}

	uint8_t sendme = (pressed ? 0x80 : 0) | (button & 0xff);
	// LOG("button %d %s : sending char to mainboard: %02x\n", button, pressed ? "down" : "up", sendme);
	xmit_char(sendme);
	xmit_char(0x00);
	if (pressed) 
	{
		m_pressed_buttons.insert(button);
	}
	else
	{
		m_pressed_buttons.erase(button);
	}
}

/* panel with 1x22 VFD display used in the EPS-16 and EPS-16 Plus */

void esqpanel1x22_device::device_add_mconfig(machine_config &config)
{
	ESQ1X22(config, m_vfd, 60);
}


esqpanel1x22_device::esqpanel1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL1X22, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = true;
}

/* panel with 2x40 VFD display used in the ESQ-1, SQ-80 */

void esqpanel2x40_device::device_add_mconfig(machine_config &config)
{
	ESQ2X40(config, m_vfd, 60);
}

esqpanel2x40_device::esqpanel2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X40, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}

/* panel with 2x40 VFD display used in the VFX, VFX-SD, SD-1 series */

esqpanel2x40_vfx_device::esqpanel2x40_vfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X40_VFX, tag, owner, clock),
	m_vfd(*this, "vfd"),
	m_lights(*this, "lights"),
	m_variant(*this, "variant"), // an output on the panel
	m_buttons_0(*this, "buttons_0"),
	m_buttons_32(*this, "buttons_32"),
	m_analog_data_entry(*this, "analog_data_entry"),
	m_analog_volume(*this, "analog_volume")
{
	m_eps_mode = false;
}

void esqpanel2x40_vfx_device::device_add_mconfig(machine_config &config)
{
	ESQ2X40_VFX(config, m_vfd, 60);

	const std::string &name = owner()->shortname();
	if (name == "vfx")
		config.set_default_layout(layout_vfx);
	else if (name == "vfxsd")
		config.set_default_layout(layout_vfxsd);
	else // "sd1" or "sd132"
		config.set_default_layout(layout_sd1);
}

void esqpanel2x40_vfx_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

//  if (data >= 0xe0) LOG("Got %02x from motherboard (second %s)\n", data, m_expect_calibration_second_byte ? "yes" : "no");

	if (m_expect_calibration_second_byte)
	{
//      LOG("second byte is %02x\n", data);
		if (data == 0xfd)   // calibration request
		{
//          LOG("let's send reply!\n");
			xmit_char(0xff);   // this is the correct response for "calibration OK"
		}
		m_expect_calibration_second_byte = false;
	}
	else if (m_expect_light_second_byte)
	{
		// Lights on the Buttons, on the VFX-SD:
		// Number   Button
		// 0        1-6
		// 1        8
		// 2        6
		// 3        4
		// 4        2
		// 5        Compare
		// 6        1
		// 7        Presets
		// 8        7-12
		// 9        9
		// a        7
		// b        5
		// c        3
		// d        Sounds
		// e        0
		// f        Cart
		int light_number = data & 0x3f;

		// Light states:
		// 0 = Off
		// 2 = On
		// 3 = Blinking
		auto light_state = (data & 0xc0) >> 6;
		m_light_states[light_number] = light_state;

		// update the internal panel
		update_lights();

		m_expect_light_second_byte = false;
	}
	else if (data == 0xfb)   // request calibration
	{
		m_expect_calibration_second_byte = true;
	}
	else if (data == 0xff)  // button light state command
	{
		m_expect_light_second_byte = true;
	}
	else
	{
		if ((data >= 0x80) && (data < 0xd0))
		{
			m_cursy = ((data & 0x7f) >= 40) ? 1 : 0;
			m_cursx = (data & 0x7f) % 40;
			m_curattr = m_vfd->get_attr(m_cursy, m_cursx);
			LOG("%02x: -> (%d, %d)\n", data, m_cursy, m_cursx);
		}
		else if (data >= 0xd0)
		{
			switch (data)
			{
				case 0xd0:  // blink start
					m_curattr |= AT_BLINK;
					break;

				case 0xd1:  // cancel all attribs on VFX+
					m_curattr = 0;
					break;

				case 0xd2:  // blinking underline on VFX
					m_curattr |= AT_BLINK | AT_UNDERLINE;
					break;

				case 0xd3:  // start underline
					m_curattr |= AT_UNDERLINE;
					break;

				case 0xd4:  // move curser one step right
					LOG("d4: %d", m_cursx);
					if (m_cursx < 39)
						m_cursx += 1;
					LOG(" -> %d\n", m_cursx);
					break;

					case 0xd5:  // move curser one step left
					LOG("d5: %d", m_cursx);
					if (m_cursx > 0)
						m_cursx -= 1;
					LOG(" -> %d\n", m_cursx);
					break;

				case 0xd6:  // clear screen
					LOG("d6: clear screen\n");
					m_vfd->clear();
					m_cursx = m_cursy = 0;
					m_curattr = 0;
					break;

				case 0xf5:  // save cursor position
					LOG("f5: save pos (%d, %d)\n", m_cursy, m_cursx);
					m_savedx = m_cursx;
					m_savedy = m_cursy;
					break;

				case 0xf6:  // restore cursor position
					LOG("f6: restore pos (%d, %d)\n", m_cursy, m_cursx);
					m_cursx = m_savedx;
					m_cursy = m_savedy;
					LOG("f6: restore pos (%d, %d)\n", m_cursy, m_cursx);
					m_curattr = m_vfd->get_attr(m_cursy, m_cursx);
					break;

				case 0xfd: // also clear screen?
					LOG("fd: clear screen\n");
					m_vfd->clear();
					m_cursx = m_cursy = 0;
					m_curattr = 0;
					break;

				default:
					char c = m_vfd->get_char(m_cursy, m_cursx);
					LOG("Unknown control code %02x (@ %d, %d, %02x '%c')\n", data, m_cursy, m_cursy, c, c);
					break;
			}
		}
		else
		{
			if ((data >= 0x20) && (data <= 0x5f))
			{
				LOG("%c\n", data);
				m_vfd->set_char(m_cursy, m_cursx, data, m_curattr);

				m_cursx++;

				if (m_cursx >= 39)
				{
					m_cursx = 39;
				}
			}
			else if (data == 0x7f)
			{
				// DEL character -> move one step right? used on ENV TIMES page, RELEASE value
				if (m_cursx < 39)
					m_cursx++;
				LOG("7f: DEL, skipping -> (%d, %d)\n", m_cursy, m_cursx);
			}
			else
			{
				char c = m_vfd->get_char(m_cursy, m_cursx);
				LOG("Unknown character code %02x (@ %d, %d, %02x '%c')\n", data, m_cursy, m_cursy, c, c);
			}
		}
	}
}

void esqpanel2x40_vfx_device::update_lights() {
	// set the lights according to their status and bllink phase.
	int32_t lights = 0;
	int32_t bit = 1;
	for (int i = 0; i < 16; i++)
	{
		if (m_light_states[i] == 2 || (m_light_states[i] == 3 && ((m_blink_phase & 1) == 0)))
		{
			lights |= bit;
		} 
		bit <<= 1;
	}
	m_lights = lights;
}

TIMER_CALLBACK_MEMBER(esqpanel2x40_vfx_device::update_blink) {
	m_blink_phase = (m_blink_phase + 1) & 3;
	m_vfd->set_blink_on(m_blink_phase & 2);
	update_lights();
}

void esqpanel2x40_vfx_device::device_start()
{
	esqpanel_device::device_start();

	m_variant.resolve();
	m_lights.resolve();

	m_blink_timer = timer_alloc(FUNC(esqpanel2x40_vfx_device::update_blink), this);
	m_blink_timer->enable(false);
}

void esqpanel2x40_vfx_device::device_reset()
{
	esqpanel_device::device_reset();

	const std::string &shortname = owner()->shortname();
	bool has_seq = shortname.find("sd") != std::string::npos;
	bool has_bank_set = shortname.find("1") != std::string::npos;
	bool has_32_voices = shortname.find("32") != std::string::npos;

	u32 bit_seq = 1 << 0;
	u32 bit_bank_set = 1 << 1;
	u32 bit_32_voices = 1 << 2;

	m_variant = 
		(has_seq ? bit_seq : 0)
		| (has_bank_set ? bit_bank_set : 0)
		| (has_32_voices ? bit_32_voices : 0);

	if (m_blink_timer) {
		attotime sample_time(0, 250 * ATTOSECONDS_PER_MILLISECOND);
		attotime initial_delay(0, 250 * ATTOSECONDS_PER_MILLISECOND);

		m_blink_timer->adjust(initial_delay, 0, sample_time);
		m_blink_timer->enable(true);
	}
}

static INPUT_PORTS_START(esqpanel2x40_vfx_device)
	PORT_START("buttons_0")
	for (int i = 0; i < 32; i++)
	{
		PORT_BIT((1 << i), IP_ACTIVE_HIGH, IPT_KEYBOARD);
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esqpanel2x40_vfx_device::button_change), i)
	}

	PORT_START("buttons_32")
	for (int i = 0; i < 32; i++)
	{
		PORT_BIT((1 << i), IP_ACTIVE_HIGH, IPT_KEYBOARD);
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esqpanel2x40_vfx_device::button_change), 32 + i)
	}

	PORT_START("analog_data_entry")
	PORT_ADJUSTER(255, "Data Entry")
	PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esqpanel2x40_vfx_device::analog_value_change), 3)

	PORT_START("analog_volume")
	PORT_ADJUSTER(255, "Volume")
	PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(esqpanel2x40_vfx_device::analog_value_change), 5)

INPUT_PORTS_END

ioport_constructor esqpanel2x40_vfx_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(esqpanel2x40_vfx_device);
}

// A button is pressed on the internal panel
INPUT_CHANGED_MEMBER(esqpanel2x40_vfx_device::button_change)
{
	// Update the internal state
	esqpanel_device::set_button(param, newval != 0);
}

// An anlog value was changed on the internal panel
INPUT_CHANGED_MEMBER(esqpanel2x40_vfx_device::analog_value_change)
{
	if (oldval == 255)
	{
		// This is the initial write from the layout. Skip this.
		LOG("skipping initial write from internal panel to channnel %d\r\n", param);
		return;
	}

	int channel = param;
	int clamped = std::max(0, std::min(100, (int)newval));
	int value = (1023 << 6) * clamped / 100;
	esqpanel_device::set_analog_value(channel, value);
}

ioport_value esqpanel2x40_vfx_device::get_adjuster_value(required_ioport &ioport)
{
	auto field = ioport->fields().first();
	ioport_field::user_settings user_settings;
	field->get_user_settings(user_settings);
	return user_settings.value;
}

void esqpanel2x40_vfx_device::set_adjuster_value(required_ioport &ioport, const ioport_value & value)
{
	auto field = ioport->fields().first();
	ioport_field::user_settings user_settings;
	field->get_user_settings(user_settings);
	user_settings.value = value;
	field->set_user_settings(user_settings);
}

// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
void esqpanel2x16_sq1_device::device_add_mconfig(machine_config &config)
{
	ESQ2X16_SQ1(config, m_vfd, 60);
}

// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
esqpanel2x16_sq1_device::esqpanel2x16_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X16_SQ1, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}
