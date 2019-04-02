// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Xilinx XC1700E family of serial configuration PROM
 * devices, including:
 *
 *   Part       Capacity  Variants
 *   --------  ---------  ----------------
 *   XC1736E      36,288
 *   XC1765E      65,536  XC1765EL
 *   XC17128E    131,072  XC17128EL
 *   XC17256E    262,144  XC17256EL
 *   XC17512L    524,288
 *   XC1701    1,048,576  XC1701L, XQ1701L
 *   XC1702L   2,097,152
 *   XC1704L   4,194,304
 *
 * The input C̅E̅ and CLK lines have not been explicitly implemented; these are
 * assumed to be asserted as expected before/during data read. The effect of
 * C̅E̅O̅ can be obtained by connecting the cascade_r callback to the data_r of
 * the cascaded device and resetting all devices when needed.
 *
 * This implementation assumes the input data is stored least-significant bit
 * first; i.e. the first bit read out of the device is held in the LSB of the
 * first byte of the memory region. This convention matches at least one
 * dumping device tested.
 *
 * Sources:
 *
 *   http://www.xilinx.com/support/documentation/data_sheets/ds027.pdf
 *
 */

#include "emu.h"
#include "xc1700e.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XC1736E,  xc1736e_device,  "xc1736e",  "Xilinx 36,288 bit Serial PROM")
DEFINE_DEVICE_TYPE(XC1765E,  xc1765e_device,  "xc1765e",  "Xilinx 65,536 bit Serial PROM")
DEFINE_DEVICE_TYPE(XC17128E, xc17128e_device, "xc17128e", "Xilinx 131,072 bit Serial PROM")
DEFINE_DEVICE_TYPE(XC17256E, xc17256e_device, "xc17256e", "Xilinx 262,144 bit Serial PROM")
DEFINE_DEVICE_TYPE(XC17512L, xc17512l_device, "xc17512l", "Xilinx 524,288 bit Serial PROM")
DEFINE_DEVICE_TYPE(XC1701,   xc1701_device,   "xc1701",   "Xilinx 1,048,576 bit Serial PROM")
DEFINE_DEVICE_TYPE(XC1702L,  xc1702l_device,  "xc1702l",  "Xilinx 2,097,152 bit Serial PROM")
DEFINE_DEVICE_TYPE(XC1704L,  xc1704l_device,  "xc1704l",  "Xilinx 4,194,304 bit Serial PROM")

base_xc1700e_device::base_xc1700e_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 capacity)
	: device_t(mconfig, type, tag, owner, clock)
	, m_capacity(capacity)
	, m_region(*this, DEVICE_SELF)
	, m_cascade_cb(*this)
	, m_reset(true)
	, m_address(0)
{
}

void base_xc1700e_device::device_start()
{
	m_cascade_cb.resolve_safe(1);

	save_item(NAME(m_reset));
	save_item(NAME(m_address));
}

void base_xc1700e_device::reset_w(int state)
{
	if (state)
		m_address = 0;

	m_reset = bool(state);
}

int base_xc1700e_device::data_r()
{
	if (m_reset)
		return 1;

	if (m_address < m_capacity)
	{
		int const data = BIT(m_region->as_u8(m_address >> 3), m_address & 7);

		m_address++;

		return data;
	}
	else
		return m_cascade_cb();
}
