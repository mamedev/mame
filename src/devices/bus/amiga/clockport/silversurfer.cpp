// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    SilverSurfer RS-232 card

***************************************************************************/

#include "emu.h"
#include "silversurfer.h"


DEFINE_DEVICE_TYPE(AMIGA_CLOCKPORT_SILVERSURFER, amiga_clockport_silversurfer_device, "amiga_clockport_silversurfer", "SilverSurfer")

amiga_clockport_silversurfer_device::amiga_clockport_silversurfer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CLOCKPORT_SILVERSURFER, tag, owner, clock),
	device_amiga_clockport_interface(mconfig, *this),
	m_uart(*this, "uart")
{
}

void amiga_clockport_silversurfer_device::device_start()
{
}

void amiga_clockport_silversurfer_device::device_add_mconfig(machine_config &config)
{
	NS16550(config, m_uart, 7.3728_MHz_XTAL);
	m_uart->out_tx_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	m_uart->out_int_callback().set([this] (int state) { m_host->int6_w(state); });

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(ns16550_device::rx_w));
	rs232.dcd_handler().set(m_uart, FUNC(ns16550_device::dcd_w));
	rs232.dsr_handler().set(m_uart, FUNC(ns16550_device::dsr_w));
	rs232.ri_handler().set(m_uart, FUNC(ns16550_device::ri_w));
	rs232.cts_handler().set(m_uart, FUNC(ns16550_device::cts_w));
}

uint8_t amiga_clockport_silversurfer_device::spare_r(offs_t offset)
{
	return m_uart->ins8250_r(offset & 7);
}

void amiga_clockport_silversurfer_device::spare_w(offs_t offset, uint8_t data)
{
	m_uart->ins8250_w(offset & 7, data);
}
