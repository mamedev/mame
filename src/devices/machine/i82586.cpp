// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
* An implementation of the Intel 82586 and 82596 Ethernet controller devices.
*
* Hopefully this might eventually support all of the following, but initial focus will be 82586 and 82596DX
* because these are used in the InterPro 2xxx and 6xxx series systems:
*
*   - 82586 - 16/24 data/address bus, 6/8/10 MHz, http://www.jbrick.org/82586.pdf (better source to follow)
*   - 82596SX - 16/32 data/address bus, 16/20 MHz, 
*   - 82596DX - 32/32 data/address bus, 25/33 MHz
*   - 82596CA - 32/32 data/address bus, 16/20/25/33 MHz, https://www.intel.com/assets/pdf/general/82596ca.pdf
*
* TODO
*   - virtually everything
*/

#include "emu.h"
#include "i82586.h"

DEFINE_DEVICE_TYPE(I82586, i82586_device, "i82586", "Intel 82586 IEEE 802.3 Ethernet LAN Coprocessor")
DEFINE_DEVICE_TYPE(I82596, i82596_device, "i82596", "Intel 82596DX and 82596SX High-Performance 32-Bit Local Area Network Coprocessor")

i82586_base_device::i82586_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_network_interface(mconfig, *this, 10.0f),
	m_out_irq(*this)
{}

i82586_device::i82586_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82586_base_device(mconfig, I82586, tag, owner, clock),
	m_sma_r(*this),
	m_sma_w(*this)
{}

i82596_device::i82596_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82586_base_device(mconfig, I82596, tag, owner, clock),
	m_sma_r(*this),
	m_sma_w(*this)
{}

void i82586_base_device::device_start()
{
	m_out_irq.resolve();
}

void i82586_base_device::device_reset()
{
}

WRITE_LINE_MEMBER(i82586_base_device::ca)
{
	logerror("%s: channel attention %s (%s)\n", state ? "asserted" : "deasserted", tag(), machine().describe_context());
}

void i82586_device::device_start()
{
	m_sma_r.resolve();
	m_sma_w.resolve();

	i82586_base_device::device_start();
}

void i82586_device::device_reset()
{
	i82586_base_device::device_reset();
}

void i82596_device::device_start()
{
	m_sma_r.resolve();
	m_sma_w.resolve();

	i82586_base_device::device_start();
}

void i82596_device::device_reset()
{
	i82586_base_device::device_reset();
}

