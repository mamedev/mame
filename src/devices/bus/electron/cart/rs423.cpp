// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Electron RS423 Communications cartridge (Pace Micro Technology)

**********************************************************************/


#include "emu.h"
#include "rs423.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_RS423, electron_rs423_device, "electron_rs423", "Pace RS423 Communications cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_rs423_device::device_add_mconfig(machine_config &config)
{
	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(DEVICE_SELF_OWNER, FUNC(electron_cartslot_device::irq_w));
	m_duart->a_tx_cb().set("rs423", FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().set("rs423", FUNC(rs232_port_device::write_rts)).bit(0);

	rs232_port_device &rs232(RS232_PORT(config, "rs423", default_rs232_devices, "null_modem"));
	rs232.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	rs232.cts_handler().set(m_duart, FUNC(scn2681_device::ip2_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_rs423_device - constructor
//-------------------------------------------------

electron_rs423_device::electron_rs423_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_RS423, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_duart(*this, "duart")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_rs423_device::device_start()
{
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_rs423_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (infc)
	{
		if (offset >= 0x60 && offset < 0x70)
		{
			data = m_duart->read(offset & 0x0f);
		}
	}
	else if (oe)
	{
		data = m_rom[offset & 0x3fff];
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_rs423_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		if (offset >= 0x60 && offset < 0x70)
		{
			m_duart->write(offset & 0x0f, data);
		}
	}
}
