// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Standard Microsystems Corp. COM52C50 Twinax Interface Circuit (TIC)

    Skeleton device.

***************************************************************************/

#include "emu.h"
#include "com52c50.h"


// device type definition
DEFINE_DEVICE_TYPE(COM52C50, com52c50_device, "com52c50", "SMC COM52C50 TIC")

com52c50_device::com52c50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COM52C50, tag, owner, clock)
	, m_int1_callback(*this)
	, m_int2_callback(*this)
	, m_rx_dma_callback(*this)
	, m_tx_dma_callback(*this)
	, m_int_mask(0)
	, m_int_status(0)
	, m_zero_fill(0)
	, m_present_address(0)
	, m_rx_status(0)
	, m_tx_status(0)
	, m_control(0)
	, m_mode(0)
{
}

void com52c50_device::map(address_map &map)
{
	map(0, 0).w(FUNC(com52c50_device::mode_w));
	map(1, 1).rw(FUNC(com52c50_device::int_status_r), FUNC(com52c50_device::int_mask_w));
	map(2, 2).rw(FUNC(com52c50_device::rx_status_r), FUNC(com52c50_device::address_select_w));
	map(3, 3).rw(FUNC(com52c50_device::rx_buffer_r), FUNC(com52c50_device::control_w));
	map(4, 4).w(FUNC(com52c50_device::zero_fill_w));
	map(5, 5).rw(FUNC(com52c50_device::present_address_r), FUNC(com52c50_device::present_address_w));
	map(6, 6).mirror(1).r(FUNC(com52c50_device::tx_status_r));
	map(6, 6).w(FUNC(com52c50_device::tx_buffer_w));
	map(7, 7).w(FUNC(com52c50_device::tx_buffer_eom_w));
}

void com52c50_device::device_resolve_objects()
{
	m_int1_callback.resolve_safe();
	m_int2_callback.resolve_safe();
	m_rx_dma_callback.resolve_safe();
	m_tx_dma_callback.resolve_safe();
}

void com52c50_device::device_start()
{
	save_item(NAME(m_int_mask));
	save_item(NAME(m_int_status));
	save_item(NAME(m_zero_fill));
	save_item(NAME(m_present_address));
	save_item(NAME(m_rx_status));
	save_item(NAME(m_tx_status));
	save_item(NAME(m_control));
	save_item(NAME(m_mode));
}

void com52c50_device::device_reset()
{
	m_int_mask = 0;
	m_int_status = 0;
	m_zero_fill = 0;
	m_rx_status = 0;
	m_tx_status = 0;
	m_control = 0x01;
	m_mode = 0;

	m_int1_callback(CLEAR_LINE);
	m_int2_callback(CLEAR_LINE);
}

u8 com52c50_device::rx_buffer_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Reading data from RX buffer\n", machine().describe_context());
	return 0;
}

void com52c50_device::tx_buffer_w(u8 data)
{
	logerror("%s: Writing %02X to TX buffer\n", machine().describe_context(), data);
}

void com52c50_device::tx_buffer_eom_w(u8 data)
{
	logerror("%s: Writing %02X to TX buffer EOM\n", machine().describe_context(), data);
}

void com52c50_device::zero_fill_w(u8 data)
{
	logerror("%s: Writing %02X to zero fill register\n", machine().describe_context(), data);
	m_zero_fill = data;
}

void com52c50_device::int_mask_w(u8 data)
{
	logerror("%s: Writing %02X to interrupt mask register\n", machine().describe_context(), data);
	m_int_mask = data;
}

void com52c50_device::address_select_w(u8 data)
{
	logerror("%s: Writing %02X to address select register\n", machine().describe_context(), data);
}

u8 com52c50_device::present_address_r()
{
	return m_present_address;
}

void com52c50_device::present_address_w(u8 data)
{
	logerror("%s: Writing %02X to present address register\n", machine().describe_context(), data);
	m_present_address = data & 0x07;
}

u8 com52c50_device::int_status_r()
{
	return m_int_status;
}

u8 com52c50_device::rx_status_r()
{
	return m_rx_status | (m_present_address << 1);
}

u8 com52c50_device::tx_status_r()
{
	return m_tx_status;
}

void com52c50_device::control_w(u8 data)
{
	if (!BIT(data, 0))
	{
		logerror("%s: Software reset\n", machine().describe_context());

		m_int_mask = 0;
		m_int_status = 0;
		m_zero_fill = 0;
		m_rx_status = 0;
		m_tx_status = 0;
		m_mode = 0;
	}

	logerror("%s: Writing %02X to control register\n", machine().describe_context(), data);
	m_control = data;
}

void com52c50_device::mode_w(u8 data)
{
	logerror("%s: Writing %02X to mode register\n", machine().describe_context(), data);
	m_mode = data;
}

