/*****************************************************************************
 *
 *   Portable Xerox AltoII memory mapped I/O hardware
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2cpu.h"

/**
 * @brief read the UTILIN port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the UTILIN port
 */
READ16_MEMBER( alto2_cpu_device::utilin_r )
{
	UINT16  data;
	// FIXME: update the printer status
	// printer_read();

	data = m_hw.utilin;

	if (!space.debugger_access()) {
		LOG((LOG_HW,2,"	UTILIN rd %#o (%#o)\n", offset, data));
	}
	return data;
}

/**
 * @brief read the XBUS port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the XBUS port latch
 */
READ16_MEMBER( alto2_cpu_device::xbus_r )
{
	UINT16 data = m_hw.xbus[offset & 3];

	if (!space.debugger_access()) {
		LOG((LOG_HW,2,"	XBUS[%d] rd %#o (%#o)\n", offset & 3, offset, data));
	}
	return data;
}

/**
 * @brief write the XBUS port
 *
 * The actual outputs are active-low.
 *
 * @param addr memory mapped I/O address to be read
 * @param data value to write to the XBUS port latch
 */
WRITE16_MEMBER( alto2_cpu_device::xbus_w )
{
	if (!space.debugger_access()) {
		LOG((LOG_HW,2,"	XBUS[%d] wr %#o (%#o)\n", offset & 3, offset, data));
	}
	m_hw.xbus[offset&3] = data;
}

/**
 * @brief read the UTILOUT port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the UTILOUT port latch
 */
READ16_MEMBER( alto2_cpu_device::utilout_r )
{
	UINT16 data = m_hw.utilout ^ 0177777;
	if (!space.debugger_access()) {
		LOG((0,2,"	UTILOUT rd %#o (%#o)\n", offset, data));
	}
	return data;
}

/**
 * @brief write the UTILOUT port
 *
 * The actual outputs are active-low.
 *
 * @param addr memory mapped I/O address to be read
 * @param data value to write to the UTILOUT port latch
 */
WRITE16_MEMBER( alto2_cpu_device::utilout_w )
{
	if (!space.debugger_access()) {
		LOG((LOG_HW,2,"	UTILOUT wr %#o (%#o)\n", offset, data));
	}
	m_hw.utilout = data ^ 0177777;

	// FIXME: write printer data
	// printer_write();
}

/**
 * @brief clear all keys and install the mmio handler for KBDAD to KBDAD+3
 */
void alto2_cpu_device::init_hw()
{
	memset(&m_hw, 0, sizeof(m_hw));

	// open inputs on UTILIN
	m_hw.utilin = 0177777;

	// open inputs on the XBUS (?)
	m_hw.xbus[0] = 0177777;
	m_hw.xbus[1] = 0177777;
	m_hw.xbus[2] = 0177777;
	m_hw.xbus[3] = 0177777;
}

void alto2_cpu_device::exit_hw()
{
	// nothing to do yet
}

