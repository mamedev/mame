// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
/*
    Ensoniq panel/display device
*/
#include "emu.h"
#include "esqpanel.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ESQPANEL1X22,     esqpanel1x22_device,     "esqpanel122",     "Ensoniq front panel with 1x22 VFD")
DEFINE_DEVICE_TYPE(ESQPANEL2X40,     esqpanel2x40_device,     "esqpanel240",     "Ensoniq front panel with 2x40 VFD")
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
	m_write_tx(*this),
	m_write_analog(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void esqpanel_device::device_start()
{
	m_write_tx.resolve_safe();
	m_write_analog.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void esqpanel_device::device_reset()
{
	// panel comms is at 62500 baud (double the MIDI rate), 8N2
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rcv_rate(62500);
	set_tra_rate(62500);

	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;
	m_bCalibSecondByte = false;
	m_bButtonLightSecondByte = false;
}

void esqpanel_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

void esqpanel_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

//  if (data >= 0xe0) printf("Got %02x from motherboard (second %s)\n", data, m_bCalibSecondByte ? "yes" : "no");

	send_to_display(data);

	if (m_bCalibSecondByte)
	{
//      printf("second byte is %02x\n", data);
		if (data == 0xfd)   // calibration request
		{
//          printf("let's send reply!\n");
			xmit_char(0xff);   // this is the correct response for "calibration OK"
		}
		m_bCalibSecondByte = false;
	}
	else if (m_bButtonLightSecondByte)
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
//      int lightNumber = data & 0x3f;

		// Light states:
		// 0 = Off
		// 2 = On
		// 3 = Blinking
//      int lightState = (data & 0xc0) >> 6;

		// TODO: do something with the button information!
		// printf("Setting light %d to %s\n", lightNumber, lightState == 3 ? "Blink" : lightState == 2 ? "On" : "Off");
		m_bButtonLightSecondByte = false;
	}
	else if (data == 0xfb)   // request calibration
	{
		m_bCalibSecondByte = true;
	}
	else if (data == 0xff)  // button light state command
	{
		m_bButtonLightSecondByte = true;
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

/* panel with 1x22 VFD display used in the EPS-16 and EPS-16 Plus */

MACHINE_CONFIG_MEMBER(esqpanel1x22_device::device_add_mconfig)
	MCFG_ESQ1X22_ADD("vfd")
MACHINE_CONFIG_END


esqpanel1x22_device::esqpanel1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL1X22, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = true;
}

/* panel with 2x40 VFD display used in the ESQ-1, VFX-SD, SD-1, and others */

MACHINE_CONFIG_MEMBER(esqpanel2x40_device::device_add_mconfig)
	MCFG_ESQ2X40_ADD("vfd")
MACHINE_CONFIG_END


esqpanel2x40_device::esqpanel2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X40, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}

// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
MACHINE_CONFIG_MEMBER(esqpanel2x16_sq1_device::device_add_mconfig)
	MCFG_ESQ2X16_SQ1_ADD("vfd")
MACHINE_CONFIG_END

// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
esqpanel2x16_sq1_device::esqpanel2x16_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqpanel_device(mconfig, ESQPANEL2X16_SQ1, tag, owner, clock),
	m_vfd(*this, "vfd")
{
	m_eps_mode = false;
}
