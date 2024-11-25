// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1520 Plotter emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_VIC1520_H
#define MAME_BUS_CBMIEC_VIC1520_H

#pragma once

#include "cbmiec.h"
#include "cpu/m6502/m6500_1.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1520_device

class vic1520_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	vic1520_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_cbm_iec_interface overrides
	void cbm_iec_atn(int state) override;
	void cbm_iec_clk(int state) override;
	void cbm_iec_data(int state) override;
	void cbm_iec_reset(int state) override;

private:
	void port_w(u8 data);
	u8 select_r();
	void led_w(u8 data);
	void pen_w(u8 data);
	void motor_w(u8 data);

	required_device<m6500_1_device> m_mcu;

	u8 m_pa_data;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1520, vic1520_device)


#endif // MAME_BUS_CBMIEC_VIC1520_H
