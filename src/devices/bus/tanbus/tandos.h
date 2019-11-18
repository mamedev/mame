// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tangerine TANDOS (MT0078)

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TANDOS_H
#define MAME_BUS_TANBUS_TANDOS_H

#pragma once

#include "bus/tanbus/tanbus.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tandos_device :
	public device_t,
	public device_tanbus_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK; }

	// construction/destruction
	tanbus_tandos_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;
	virtual void set_inhibit_lines(offs_t offset, int &inhram, int &inhrom) override;

private:
	void control_w(uint8_t val);
	uint8_t status_r();
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_hld_w);

	required_memory_region m_dos_rom;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	floppy_image_device *m_floppy;

	uint8_t m_drive_control;
	int m_irq_enable;
	int m_drq_enable;

	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TANDOS, tanbus_tandos_device)


#endif // MAME_BUS_TANBUS_TANDOS_H
