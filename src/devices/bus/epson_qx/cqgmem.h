// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * commodity quote graphics 1 megabyte memory expansion
 *
 *******************************************************************/

#ifndef MAME_BUS_EPSON_QX_CQGMEM_H
#define MAME_BUS_EPSON_QX_CQGMEM_H

#pragma once

#include "option.h"

namespace bus::epson_qx {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* commodity quote graphics 1M memory expansion */

class cqgmem_device : public device_t, public device_option_expansion_interface, public device_memory_interface
{
public:
	// construction/destruction
	cqgmem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	void write(offs_t offset, uint8_t data);

	void io_map(address_map &map) ATTR_COLD;
	void xmem_map(address_map &map) ATTR_COLD;

	uint8_t m_banks_enabled;

	memory_share_creator<uint8_t> m_ram;
	memory_bank_array_creator<7> m_banks;
	required_ioport m_iobase;
	const address_space_config m_space_config;

	bool m_installed;
};

} // namespace bus::epson_qx

// device type definition
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_CQGMEM, bus::epson_qx, cqgmem_device)


#endif // MAME_BUS_EPSON_QX_CQGMEM_H
