// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ASC-88 SCSI Adapter
    © 1985 Advanced Storage Concepts, Inc.

***************************************************************************/

#ifndef MAME_BUS_ISA_ASC88_H
#define MAME_BUS_ISA_ASC88_H

#pragma once

#include "isa.h"
#include "machine/eepromser.h"
#include "machine/ncr5380.h"

class asc88_device : public device_t, public device_isa8_card_interface
{
public:
	asc88_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual u8 dack_r(int line) override;
	virtual void dack_w(int line, u8 data) override;

private:
	void irq_w(int state);
	void drq_w(int state);

	void control_w(u8 data);
	u8 eeprom_r();

	void scsic_config(device_t *device);

	required_device<ncr5380_device> m_scsic;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_ioport m_baseaddr;

	u8 m_control;
	bool m_irq;
	bool m_drq;
	std::unique_ptr<u8[]> m_ram;
};

// device type declaration
DECLARE_DEVICE_TYPE(ASC88, asc88_device)

#endif // MAME_BUS_ISA_ASC88_H
