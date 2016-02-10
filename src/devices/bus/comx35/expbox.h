// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35E Expansion Box emulation

**********************************************************************/

#pragma once

#ifndef __COMX_EB__
#define __COMX_EB__

#include "exp.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define MAX_EB_SLOTS    4



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_eb_device

class comx_eb_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_eb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( slot1_irq_w ) { m_irq[0] = state; set_irq(); }
	DECLARE_WRITE_LINE_MEMBER( slot2_irq_w ) { m_irq[1] = state; set_irq(); }
	DECLARE_WRITE_LINE_MEMBER( slot3_irq_w ) { m_irq[2] = state; set_irq(); }
	DECLARE_WRITE_LINE_MEMBER( slot4_irq_w ) { m_irq[3] = state; set_irq(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_comx_expansion_card_interface overrides
	virtual int comx_ef4_r() override;
	virtual void comx_q_w(int state) override;
	virtual UINT8 comx_mrd_r(address_space &space, offs_t offset, int *extrom) override;
	virtual void comx_mwr_w(address_space &space, offs_t offset, UINT8 data) override;
	virtual UINT8 comx_io_r(address_space &space, offs_t offset) override;
	virtual void comx_io_w(address_space &space, offs_t offset, UINT8 data) override;

	void set_irq() { m_slot->irq_w(m_irq[0] || m_irq[1] || m_irq[2] || m_irq[3]); }

private:
	required_memory_region m_rom;

	comx_expansion_slot_device  *m_expansion_slot[MAX_EB_SLOTS];
	int m_irq[MAX_EB_SLOTS];

	UINT8 m_select;
};


// device type definition
extern const device_type COMX_EB;


#endif
