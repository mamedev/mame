// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CST Q+4 emulation

**********************************************************************/

#pragma once

#ifndef __CST_Q_PLUS4__
#define __CST_Q_PLUS4__

#include "exp.h"
#include "machine/6821pia.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cst_q_plus4_t

class cst_q_plus4_t : public device_t,
						public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	cst_q_plus4_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE_LINE_MEMBER( exp1_extintl_w ) { m_exp1_extinl = state; update_extintl(); }
	DECLARE_WRITE_LINE_MEMBER( exp2_extintl_w ) { m_exp2_extinl = state; update_extintl(); }
	DECLARE_WRITE_LINE_MEMBER( exp3_extintl_w ) { m_exp3_extinl = state; update_extintl(); }
	DECLARE_WRITE_LINE_MEMBER( exp4_extintl_w ) { m_exp4_extinl = state; update_extintl(); }

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_ql_expansion_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data) override;
	virtual void write(address_space &space, offs_t offset, UINT8 data) override;

private:
	void update_extintl() { m_slot->extintl_w(m_exp1_extinl || m_exp2_extinl || m_exp3_extinl || m_exp4_extinl); }

	required_device<ql_expansion_slot_t> m_exp1;
	required_device<ql_expansion_slot_t> m_exp2;
	required_device<ql_expansion_slot_t> m_exp3;
	required_device<ql_expansion_slot_t> m_exp4;
	required_memory_region m_rom;

	int m_exp1_extinl;
	int m_exp2_extinl;
	int m_exp3_extinl;
	int m_exp4_extinl;
};



// device type definition
extern const device_type CST_Q_PLUS4;



#endif
