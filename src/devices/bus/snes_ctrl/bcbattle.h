// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom - Epoch Barcode Battler

**********************************************************************/

#ifndef MAME_BUS_SNES_CTRL_BCBATTLE_H
#define MAME_BUS_SNES_CTRL_BCBATTLE_H

#pragma once


#include "ctrl.h"
#include "machine/bcreader.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> snes_bcbattle_device

class snes_bcbattle_device : public device_t,
							public device_snes_control_port_interface
{
public:
	// construction/destruction
	snes_bcbattle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read_pin4() override;
	virtual void write_strobe(uint8_t data) override;
	virtual void port_poll() override;

	int read_current_bit();

	TIMER_CALLBACK_MEMBER(scan_tick);

private:
	required_device<barcode_reader_device> m_reader;
	uint8_t m_current_barcode[20];
	int m_pending_code, m_new_code, m_transmitting, m_cur_bit, m_cur_byte;
	emu_timer *battler_timer;

	int m_strobe, m_idx;
};

// device type definition
DECLARE_DEVICE_TYPE(SNES_BARCODE_BATTLER, snes_bcbattle_device)

#endif // MAME_BUS_SNES_CTRL_BCBATTLE_H
