// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - Epoch Barcode Battler

**********************************************************************/

#pragma once

#ifndef __NES_BCBATTLE__
#define __NES_BCBATTLE__


#include "emu.h"
#include "ctrl.h"
#include "machine/bcreader.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_bcbattle_device

class nes_bcbattle_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_bcbattle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_exp(offs_t offset);
	int read_current_bit();

	static const device_timer_id TIMER_BATTLER = 1;
	required_device<barcode_reader_device> m_reader;
	UINT8 m_current_barcode[20];
	int m_pending_code, m_new_code, m_transmitting, m_cur_bit, m_cur_byte;
	emu_timer *battler_timer;
};

// device type definition
extern const device_type NES_BARCODE_BATTLER;

#endif
