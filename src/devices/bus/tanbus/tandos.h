// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tangerine TANDOS (MT0078)

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TANDOS_H
#define MAME_BUS_TANBUS_TANDOS_H

#pragma once

#include "tanbus.h"
#include "machine/wd_fdc.h"
#include "machine/tms9914.h"
#include "bus/ieee488/ieee488.h"
#include "imagedev/floppy.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tandos_device : public device_t, public device_tanbus_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK; }

	// construction/destruction
	tanbus_tandos_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;
	virtual void set_inhibit_lines(offs_t offset, int &inhram, int &inhrom) override;

private:
	void control_w(uint8_t data);
	uint8_t status_r();
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void fdc_hld_w(int state);

	required_memory_region m_dos_rom;
	required_device<ieee488_device> m_ieee;
	required_device<tms9914_device> m_tms9914;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	floppy_image_device *m_floppy;

	uint8_t m_status;
	int m_irq_enable;
	int m_drq_enable;

	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TANDOS, tanbus_tandos_device)


#endif // MAME_BUS_TANBUS_TANDOS_H
