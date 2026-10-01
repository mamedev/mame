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
#include "machine/alpsdpg23.h"



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
	void cbm_iec_reset(int state) override;

private:
	required_device<m6500_1_device> m_mcu;
	required_device<alps_dpg23_device> m_plotter;
	required_ioport m_pb;
	output_finder<> m_led;

	uint8_t pa_r();
	void pa_w(uint8_t data);
	uint8_t pb_r();
	void pb_w(uint8_t data);
	uint8_t pc_r();

	void update_iec_data();

	bool m_attn_ack = true;
	bool m_nrfd = true;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1520, vic1520_device)


#endif // MAME_BUS_CBMIEC_VIC1520_H
