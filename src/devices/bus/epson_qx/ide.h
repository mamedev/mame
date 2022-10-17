// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * IDE Compact Flash Adapter
 *
 *******************************************************************/

#ifndef MAME_BUS_EPSON_QX_IDE_H
#define MAME_BUS_EPSON_QX_IDE_H

#pragma once

#include "option.h"

#include "bus/ata/idehd.h"

namespace bus::epson_qx {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* Epson IDE Device */

class ide_device : public device_t, public bus::epson_qx::device_option_expansion_interface
{
public:
	// construction/destruction
	ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void map(address_map &map);

private:
	required_device<ide_hdd_device> m_hdd;
	required_ioport m_iobase;

	bool m_installed;
};

} // namespace bus::epson_qx

// device type definition
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_IDE, bus::epson_qx, ide_device)

#endif // MAME_BUS_EPSON_QX_IDE_H
