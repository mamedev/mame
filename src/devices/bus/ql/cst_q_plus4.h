// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CST Q+4 emulation

**********************************************************************/

#ifndef MAME_BUS_QL_CST_Q_PLUS4_H
#define MAME_BUS_QL_CST_Q_PLUS4_H

#pragma once

#include "exp.h"
#include "machine/6821pia.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cst_q_plus4_device

class cst_q_plus4_device : public device_t, public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	cst_q_plus4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_ql_expansion_card_interface overrides
	virtual uint8_t read(offs_t offset, uint8_t data) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	void exp1_extintl_w(int state) { m_exp1_extinl = state; update_extintl(); }
	void exp2_extintl_w(int state) { m_exp2_extinl = state; update_extintl(); }
	void exp3_extintl_w(int state) { m_exp3_extinl = state; update_extintl(); }
	void exp4_extintl_w(int state) { m_exp4_extinl = state; update_extintl(); }

	void update_extintl() { m_slot->extintl_w(m_exp1_extinl || m_exp2_extinl || m_exp3_extinl || m_exp4_extinl); }

	required_device<ql_expansion_slot_device> m_exp1;
	required_device<ql_expansion_slot_device> m_exp2;
	required_device<ql_expansion_slot_device> m_exp3;
	required_device<ql_expansion_slot_device> m_exp4;
	required_memory_region m_rom;

	int m_exp1_extinl;
	int m_exp2_extinl;
	int m_exp3_extinl;
	int m_exp4_extinl;
};


// device type definition
DECLARE_DEVICE_TYPE(CST_Q_PLUS4, cst_q_plus4_device)

#endif // MAME_BUS_QL_CST_Q_PLUS4_H
