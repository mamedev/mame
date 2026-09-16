// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Ralph Allen Disc Controller Card

**********************************************************************/


#ifndef MAME_BUS_TANBUS_RADISC_H
#define MAME_BUS_TANBUS_RADISC_H

#pragma once

#include "bus/tanbus/tanbus.h"
#include "machine/wd_fdc.h"
#include "machine/mc146818.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "sound/beep.h"
#include "imagedev/floppy.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_radisc_device : public device_t, public device_tanbus_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK; }

	// construction/destruction
	tanbus_radisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	enum { IRQ_FDC, IRQ_VIA, IRQ_RTC };

	void control_w(uint8_t data);
	uint8_t status_r();
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void irq_w(int state);

	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	required_device<mc146818_device> m_rtc;
	required_device<via6522_device> m_via;
	required_device<input_merger_device> m_irq_line;
	required_device<beep_device> m_beeper;

	int m_beeper_state;
	uint8_t m_status;
	int m_irq_enable;
	int m_drq_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_RADISC, tanbus_radisc_device)


#endif // MAME_BUS_TANBUS_RADISC_H
