// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Slogger Click cartridge emulation

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Slogger_Click.html

***************************************************************************/

#include "emu.h"
#include "click.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_CLICK, electron_click_device, "electron_click", "Slogger Click cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_click_device::device_add_mconfig(machine_config &config)
{
	/* rtc */
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(DEVICE_SELF_OWNER, FUNC(electron_cartslot_device::irq_w));
}

//-------------------------------------------------
//  INPUT_PORTS( click )
//-------------------------------------------------

INPUT_PORTS_START(click)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Click") PORT_CODE(KEYCODE_HOME) PORT_CHANGED_MEMBER(DEVICE_SELF, electron_click_device, click_button, nullptr)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor electron_click_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(click);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_click_device - constructor
//-------------------------------------------------

electron_click_device::electron_click_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_CLICK, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
	, m_page_register(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_click_device::device_start()
{
	save_item(NAME(m_page_register));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_click_device::device_reset()
{
	m_page_register = 0;
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_click_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0xf8:
		case 0xf9:
			data = m_rtc->read(offset & 0x01);
			break;
		case 0xfc:
			data = m_page_register;
			break;
		}
	}
	else if (oe)
	{
		offs_t rom_page_offset = (m_page_register & 0x03) * 0x2000;
		offs_t ram_page_offset = ((m_page_register & 0x0c) >> 2) * 0x2000;

		if (offset >= 0x0000 && offset < 0x2000)
		{
			data = m_rom[rom_page_offset + (offset & 0x1fff)];
		}
		else if (offset >= 0x2000 && offset < 0x4000)
		{
			data = m_nvram[ram_page_offset + (offset & 0x1fff)];
		}
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_click_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0xf8:
		case 0xf9:
			m_rtc->write(offset & 0x01, data);
			break;
		case 0xfc:
			m_page_register = data;
			break;
		}
	}
	else if (oe)
	{
		offs_t ram_page_offset = ((m_page_register & 0x0c) >> 2) * 0x2000;

		if (offset >= 0x2000 && offset < 0x4000)
		{
			m_nvram[ram_page_offset + (offset & 0x1fff)] = data;
		}
	}
}

INPUT_CHANGED_MEMBER(electron_click_device::click_button)
{
	if (newval && !oldval)
	{
		m_slot->irq_w(ASSERT_LINE);
	}
	else
	{
		m_slot->irq_w(CLEAR_LINE);
	}
}
