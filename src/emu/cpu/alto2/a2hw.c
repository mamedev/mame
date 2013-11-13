/*****************************************************************************
 *
 *   Portable Xerox AltoII memory mapped I/O hardware
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2.h"

/**
 * @brief read the UTILIN port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the UTILIN port
 */
UINT16 alto2_cpu_device::utilin_r(UINT32 addr)
{
	UINT16  data;
	// FIXME: update the printer status
	// printer_read();

	data = m_hw.utilin;

	LOG((LOG_HW,2,"	read UTILIN %#o (%#o)\n", addr, data));
	return data;
}

/**
 * @brief read the XBUS port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the XBUS port latch
 */
UINT16 alto2_cpu_device::xbus_r(UINT32 addr)
{
	UINT16 data = m_hw.xbus[addr & 3];

	LOG((LOG_HW,2,"	read XBUS[%d] %#o (%#o)\n", addr&3, addr, data));
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
void alto2_cpu_device::xbus_w(UINT32 addr, UINT16 data)
{
	LOG((LOG_HW,2,"	write XBUS[%d] %#o (%#o)\n", addr & 3, addr, data));
	m_hw.xbus[addr&3] = data;
}

/**
 * @brief read the UTILOUT port
 *
 * @param addr memory mapped I/O address to be read
 * @return current value on the UTILOUT port latch
 */
UINT16 alto2_cpu_device::utilout_r(UINT32 addr)
{
	UINT16 data = m_hw.utilout ^ 0177777;
	LOG((0,2,"	read UTILOUT %#o (%#o)\n", addr, data));
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
void alto2_cpu_device::utilout_w(UINT32 addr, UINT16 data)
{
	LOG((LOG_HW,2,"	write UTILOUT %#o (%#o)\n", addr, data));
	m_hw.utilout = data ^ 0177777;

	// FIXME: write printer data
	// printer_write();
}

/**
 * @brief clear all keys and install the mmio handler for KBDAD to KBDAD+3
 */
void alto2_cpu_device::init_hardware()
{
	memset(&m_hw, 0, sizeof(m_hw));

	// open inputs on UTILIN
	m_hw.utilin = 0177777;

	// open inputs on the XBUS (?)
	m_hw.xbus[0] = 0177777;
	m_hw.xbus[1] = 0177777;
	m_hw.xbus[2] = 0177777;
	m_hw.xbus[3] = 0177777;

	// install memory handlers
	install_mmio_fn(0177016, 0177016, &alto2_cpu_device::utilout_r, &alto2_cpu_device::utilout_w);
	install_mmio_fn(0177020, 0177023, &alto2_cpu_device::xbus_r, &alto2_cpu_device::xbus_w);
	install_mmio_fn(0177030, 0177033, &alto2_cpu_device::utilin_r, 0);
}

