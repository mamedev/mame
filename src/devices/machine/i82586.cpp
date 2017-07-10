// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
* An implementation of the Intel 82586 and 82596 Ethernet controller devices.
*
* Hopefully this might eventually support all of the following, but initial focus will be 82586 and 82596DX
* because these are used in the InterPro 2xxx and 6xxx series systems:
*
*   - 82586 - 16/24 data/address bus, 6/8/10 MHz
*   - 82596SX - 16/32 data/address bus, 16/20 MHz
*   - 82596DX - 32/32 data/address bus, 25/33 MHz
*   - 82596CA - 32/32 data/address bus, 16/20/25/33 MHz
*
* Some documents covering the above include:
*
*   http://bitsavers.org/pdf/intel/_dataBooks/1991_Microcommunications.pdf
*   http://bitsavers.org/pdf/intel/_dataBooks/1996_Networking.pdf
*   https://www.intel.com/assets/pdf/general/82596ca.pdf
*
* TODO
*   - virtually everything
*/

#include "emu.h"
#include "i82586.h"

DEFINE_DEVICE_TYPE(I82586, i82586_device, "i82586", "Intel 82586 IEEE 802.3 Ethernet LAN Coprocessor")
DEFINE_DEVICE_TYPE(I82596SX, i82596sx_device, "i82596sx", "Intel 82596SX High-Performance 32-Bit Local Area Network Coprocessor")
DEFINE_DEVICE_TYPE(I82596DX, i82596dx_device, "i82596dx", "Intel 82596DX High-Performance 32-Bit Local Area Network Coprocessor")

i82586_base_device::i82586_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 datawidth, u8 addrwidth)
	: device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_network_interface(mconfig, *this, 10.0f),
	m_space_config("shared", ENDIANNESS_LITTLE, datawidth, addrwidth),
	m_out_irq(*this)
{}

i82586_device::i82586_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82586_base_device(mconfig, I82586, tag, owner, clock, 16, 24)
{}

i82596_base_device::i82596_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 datawidth, u8 addrwidth)
	: i82586_base_device(mconfig, type, tag, owner, clock, datawidth, addrwidth)
{}

i82596sx_device::i82596sx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82596_base_device(mconfig, I82596SX, tag, owner, clock, 16, 32)
{}

i82596dx_device::i82596dx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82596_base_device(mconfig, I82596DX, tag, owner, clock, 32, 32)
{}

void i82586_base_device::device_start()
{
	m_space = &space(0);

	m_out_irq.resolve();
}

void i82586_base_device::device_reset()
{
}

device_memory_interface::space_config_vector i82586_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

WRITE_LINE_MEMBER(i82586_base_device::ca)
{
	logerror("%s: channel attention %s (%s)\n", state ? "asserted" : "deasserted", tag(), machine().describe_context());
}

void i82586_device::device_start()
{
	i82586_base_device::device_start();
}

void i82586_device::device_reset()
{
	i82586_base_device::device_reset();
}

void i82596_base_device::device_start()
{
	i82586_base_device::device_start();
}

void i82596_base_device::device_reset()
{
	i82586_base_device::device_reset();
}

void i82596_base_device::port(u32 data)
{
	switch (data & 0xf)
	{
	case 0:
		// execute a software reset
		break;

	case 1:
		// execute a self-test
		break;

	case 2:
		// write an alterantive system configuration pointer address
		break;

	case 3:
		// write an alternative dump area pointer and perform dump
		break;
	}
}
