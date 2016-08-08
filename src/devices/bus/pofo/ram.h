// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio 128KB RAM card emulation

**********************************************************************/

#pragma once

#ifndef __PORTFOLIO_RAM_CARD__
#define __PORTFOLIO_RAM_CARD__

#include "emu.h"
#include "ccm.h"
#include "machine/nvram.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> portfolio_ram_card_t

class portfolio_ram_card_t :  public device_t,
				  			  public device_portfolio_memory_card_slot_interface,
							  public device_nvram_interface
{
public:
	// construction/destruction
	portfolio_ram_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override { }
	virtual void nvram_read(emu_file &file) override { if (m_nvram != nullptr) { file.read(m_nvram, m_nvram.bytes()); } }
	virtual void nvram_write(emu_file &file) override { if (m_nvram != nullptr) { file.write(m_nvram, m_nvram.bytes()); } }

	// device_portfolio_memory_card_slot_interface overrides
	bool cdet() override { return 0; }

	virtual UINT8 nrdi_r(address_space &space, offs_t offset) override;
	virtual void nwri_w(address_space &space, offs_t offset, UINT8 data) override;
};


// device type definition
extern const device_type PORTFOLIO_RAM_CARD;



#endif
