// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ASC-88 SCSI Adapter
    © 1985 Advanced Storage Concepts, Inc.

***************************************************************************/

#ifndef MAME_BUS_ISA_ASC88_H
#define MAME_BUS_ISA_ASC88_H 1

#pragma once

#include "isa.h"
#include "machine/eepromser.h"
#include "machine/ncr5380n.h"

class asc88_device : public device_t, public device_isa8_card_interface
{
public:
	asc88_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual u8 dack_r(int line) override;
	virtual void dack_w(int line, u8 data) override;

private:
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_WRITE_LINE_MEMBER(drq_w);

	void control_w(u8 data);
	u8 eeprom_r();

	void scsic_config(device_t *device);

	required_device<ncr5380n_device> m_scsic;
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
