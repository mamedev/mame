/*****************************************************************************
 *
 *   Portable Xerox AltoII memory mapped I/O keyboard
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2.h"

/**
 * @brief read the keyboard address matrix
 *
 * @param addr memory mapped I/O address to be read
 * @return keyboard matrix value for address modulo 4
 */
UINT16 alto2_cpu_device::kbd_ad_r(UINT32 addr)
{
	UINT16 data = 0177777;
	switch (addr & 3) {
	case 0:
		data = machine().root_device().ioport("ROW0")->read();
		break;
	case 1:
		data = machine().root_device().ioport("ROW1")->read();
		break;
	case 2:
		data = machine().root_device().ioport("ROW2")->read();
		break;
	case 3:
		data = machine().root_device().ioport("ROW3")->read();
		break;
	}
	m_kbd.matrix[addr & 03] = data;
	LOG((LOG_KBD,2,"	read KBDAD+%o (%#o)\n", addr & 3, data));
	if (0 == (addr & 3) && (m_kbd.bootkey != 0177777)) {
		LOG((0,2,"	boot keys (%#o & %#o)\n", data, m_kbd.bootkey));
		data &= m_kbd.bootkey;
		m_kbd.bootkey = 0177777;
	}
	return data;
}

void alto2_cpu_device::init_kbd(UINT16 bootkey)
{
	m_kbd.bootkey = bootkey;
	m_kbd.matrix[0] = 0177777;
	m_kbd.matrix[1] = 0177777;
	m_kbd.matrix[2] = 0177777;
	m_kbd.matrix[3] = 0177777;
	// install memory handler
	install_mmio_fn(0177034, 0177037, &alto2_cpu_device::kbd_ad_r, 0);
}

