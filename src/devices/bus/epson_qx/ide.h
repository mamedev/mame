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

#include "machine/atastorage.h"

namespace bus::epson_qx {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* Epson IDE Device */

// TODO: this thing should probably implement the ATA controller interface rather than having an IDE hard disk with no slot
class ide_device : public ide_hdd_device_base, public device_option_expansion_interface
{
public:
	// construction/destruction
	ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void map(address_map &map) ATTR_COLD;

private:
	// ata_hle_device_base implementation
	virtual void set_irq_out(int state) override { }
	virtual void set_dmarq_out(int state) override { }
	virtual void set_dasp_out(int state) override { }
	virtual void set_pdiag_out(int state) override { }

	required_ioport m_iobase;

	bool m_installed;
};

} // namespace bus::epson_qx

// device type definition
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_IDE, bus::epson_qx, ide_device)

#endif // MAME_BUS_EPSON_QX_IDE_H
