// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
/*
    Ensoniq panel/display device
*/
#include "emu.h"
#include "esqpanel.h"
#include "extpanel.h"

#include "main.h"

//**************************************************************************
// External panel support
//**************************************************************************

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
	m_external_panel(*this, "esq_external_panel"),
	m_light_states(0x3f), // maximum number of lights
	m_write_tx(*this),
	m_write_analog(*this)
{
	std::fill(std::begin(m_xmitring), std::end(m_xmitring), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void esqpanel_device::device_start()
{
	m_external_panel_timer = timer_alloc(FUNC(esqpanel_device::check_external_panel_server), this);
	m_external_panel_timer->enable(false);
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

	if (m_external_panel && m_external_panel->is_running()) {
		m_external_panel_timer->adjust(initial_delay, 0, sample_time);
		m_external_panel_timer->enable(true);
	}
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void esqpanel_device::device_stop()
{
	device_t::device_stop();
	m_external_panel_timer->enable(false);
}

void esqpanel_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

//  if (data >= 0xe0) printf("Got %02x from motherboard (second %s)\n", data, m_expect_calibration_second_byte ? "yes" : "no");

	send_to_display(data);

	if (m_expect_calibration_second_byte)
	{
//      printf("second byte is %02x\n", data);
		if (data == 0xfd)   // calibration request
		{
//          printf("let's send reply!\n");
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

		// TODO: do something with the button information!
		// printf("Setting light %d to %s\n", light_number, lightState == 3 ? "Blink" : lightState == 2 ? "On" : "Off");
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
//  printf("panel Tx complete\n");
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
//  printf("Panel: xmit %02x\n", data);

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

void esqpanel_device::set_button(uint8_t button, bool pressed)
{
	uint8_t sendme = (pressed ? 0x80 : 0) | (button & 0xff);
	// printf("button %d %s : sending char to mainboard: %02x\n", button, down ? "down" : "up", sendme);
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

TIMER_CALLBACK_MEMBER(esqpanel_device::check_external_panel_server) {
	esq_external_panel_device::Command c;
	while (m_external_panel->get_next_command(c))
	{
		switch (c.kind) {
			case esq_external_panel_device::Command::ButtonKind:
				set_button(c.button, c.pressed);
				break;

			case esq_external_panel_device::Command::AnalogValueKind:
				set_analog_value(c.channel, c.value);
				break;
				
			case esq_external_panel_device::Command::SendKind:
				send_display_contents();
				send_button_states();
				send_light_states();
				send_analog_values();
				break;
		}
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

void esqpanel2x40_vfx_device::device_add_mconfig(machine_config &config)
{
	ESQ2X40_VFX(config, m_vfd, 60);
	ESQ_EXTERNAL_PANEL_DEVICE(config, m_external_panel, 0);
}

esqpanel2x40_vfx_device::esqpanel2x40_vfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X40_VFX, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}

void esqpanel2x40_vfx_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

//  if (data >= 0xe0) printf("Got %02x from motherboard (second %s)\n", data, m_expect_calibration_second_byte ? "yes" : "no");

	if (m_expect_calibration_second_byte)
	{
//      printf("second byte is %02x\n", data);
		if (data == 0xfd)   // calibration request
		{
//          printf("let's send reply!\n");
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
		m_external_panel->set_light(light_number, light_state);
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

				case 0xd6:  // clear screen
					m_vfd->clear();
					m_external_panel->clear_display();
					m_cursx = m_cursy = 0;
					m_curattr = 0;
					break;

				case 0xf5:  // save cursor position
					m_savedx = m_cursx;
					m_savedy = m_cursy;
					break;

				case 0xf6:  // restore cursor position
					m_cursx = m_savedx;
					m_cursy = m_savedy;
					m_curattr = m_vfd->get_attr(m_cursy, m_cursx);
					break;

				case 0xfd: // also clear screen?
					m_vfd->clear();
					m_external_panel->clear_display();
					m_cursx = m_cursy = 0;
					m_curattr = 0;
					break;

				default:
					// printf("Unknown control code %02x\n", data);
					break;
			}
		}
		else
		{
			if ((data >= 0x20) && (data <= 0x5f))
			{
				m_vfd->set_char(m_cursy, m_cursx, data - ' ', m_curattr);
				m_external_panel->set_char(m_cursy, m_cursx, data - ' ', m_curattr);

				m_cursx++;

				if (m_cursx >= 39)
				{
					m_cursx = 39;
				}
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(esqpanel2x40_vfx_device::update_blink) {
	m_blink_phase = (m_blink_phase + 1) & 3;
	m_vfd->set_blink_on(m_blink_phase & 2);
	m_external_panel->set_blink_phase(m_blink_phase);
}

void esqpanel2x40_vfx_device::device_start()
{
	esqpanel_device::device_start();

	m_external_panel->set_keyboard(owner()->shortname());
	m_external_panel->set_version("1");

	m_blink_timer = timer_alloc(FUNC(esqpanel2x40_vfx_device::update_blink), this);
	m_blink_timer->enable(false);
}

void esqpanel2x40_vfx_device::device_reset()
{
	esqpanel_device::device_reset();

	if (m_blink_timer) {
		attotime sample_time(0, 250 * ATTOSECONDS_PER_MILLISECOND);
		attotime initial_delay(0, 250 * ATTOSECONDS_PER_MILLISECOND);

		m_blink_timer->adjust(initial_delay, 0, sample_time);
		m_blink_timer->enable(true);
	}
}

void esqpanel2x40_vfx_device::device_reset_after_children()
{
	esqpanel_device::device_reset_after_children();
}

void esqpanel2x40_vfx_device::send_display_contents()
{
	std::vector<std::pair<uint8_t, uint8_t>> contents;
	contents.reserve(80);
	for (int row = 0; row < 2; row++)
	{
		for (int column = 0; column < 40; column++) 
		{
			contents.emplace_back(std::make_pair(m_vfd->get_char(row, column), m_vfd->get_attr(row, column)));
		}
	}
	m_external_panel->set_display_contents(contents);
}

void esqpanel2x40_vfx_device::send_analog_values()
{
	// TODO(cbrunschen): read analog values from the emulated keyboard and send them to the panel(s)
}

void esqpanel2x40_vfx_device::send_button_states()
{
	std::vector<std::pair<uint8_t, bool>> button_states;
	button_states.reserve(64);
	for (int i = 0; i < 64; i++)
	{
		button_states.emplace_back(std::make_pair(i, m_pressed_buttons.find(i) != m_pressed_buttons.end()));
	}
	m_external_panel->set_button_states(button_states);
}

void esqpanel2x40_vfx_device::send_light_states()
{
	std::vector<std::pair<uint8_t, uint8_t>> light_states;
	light_states.reserve(16);
	for (int i = 0; i < 16; i++)
	{
		light_states.emplace_back(std::make_pair(i, m_light_states[i]));
	}
	m_external_panel->set_light_states(light_states);
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
