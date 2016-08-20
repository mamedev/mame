// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-104 Memory Expander Plus emulation

**********************************************************************/

#pragma once

#ifndef __HPC104__
#define __HPC104__

#include "emu.h"
#include "exp.h"
#include "bus/pofo/ccm.h"
#include "machine/nvram.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hpc104_t

class hpc104_t :  public device_t,
				  public device_portfolio_expansion_slot_interface,
				  public device_nvram_interface
{
public:
	// construction/destruction
	hpc104_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	hpc104_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override { }
	virtual void nvram_read(emu_file &file) override { if (m_nvram != nullptr) { file.read(m_nvram, m_nvram.bytes()); } }
	virtual void nvram_write(emu_file &file) override { if (m_nvram != nullptr) { file.write(m_nvram, m_nvram.bytes()); } }

	// device_portfolio_expansion_slot_interface overrides
	virtual bool nmd1() override { return m_ccm->cdet_r(); }

	virtual UINT8 nrdi_r(address_space &space, offs_t offset, UINT8 data, bool iom, bool bcom, bool ncc1) override;
	virtual void nwri_w(address_space &space, offs_t offset, UINT8 data, bool iom, bool bcom, bool ncc1) override;

private:
	required_device<portfolio_memory_card_slot_t> m_ccm;
	required_device<portfolio_expansion_slot_t> m_exp;
	optional_shared_ptr<UINT8> m_nvram;
	required_ioport m_io_sw1;

	bool m_sw1;
	bool m_ncc1_out;
};


// ======================> hpc104_2_t

class hpc104_2_t :  public hpc104_t
{
public:
	// construction/destruction
	hpc104_2_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
extern const device_type HPC104;
extern const device_type HPC104_2;



#endif
