// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-104 Memory Expander Plus emulation

**********************************************************************/

#ifndef MAME_BUS_POFO_HPC104_H
#define MAME_BUS_POFO_HPC104_H

#pragma once

#include "exp.h"
#include "bus/pofo/ccm.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pofo_hpc104_device

class pofo_hpc104_device :  public device_t,
				  public device_portfolio_expansion_slot_interface,
				  public device_nvram_interface
{
public:
	// construction/destruction
	pofo_hpc104_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	pofo_hpc104_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_portfolio_expansion_slot_interface implementation
	virtual bool nmd1() override { return m_ccm->cdet_r(); }

	virtual void ncc1_w(int state) override;

	virtual void iint_w(int state) override { m_exp->iint_w(state); }
	virtual void nmio_w(int state) override { m_exp->nmio_w(state); }

private:
	void update_ccm_b_tap();

	required_device<portfolio_memory_card_slot_device> m_ccm;
	required_device<portfolio_expansion_slot_device> m_exp;
	memory_share_creator<uint8_t> m_nvram;
	required_ioport m_io_sw1;

	bool m_sw1;
	bool m_ncc1_out;
	bool m_upstream_ccm_b;
};


// ======================> pofo_hpc104_2_device

class pofo_hpc104_2_device :  public pofo_hpc104_device
{
public:
	// construction/destruction
	pofo_hpc104_2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(POFO_HPC104,   pofo_hpc104_device)
DECLARE_DEVICE_TYPE(POFO_HPC104_2, pofo_hpc104_2_device)

#endif // MAME_BUS_POFO_HPC104_H
