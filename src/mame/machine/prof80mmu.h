// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Conitec PROF-80 Memory Management Unit emulation

**********************************************************************/

#pragma once

#ifndef __PROF80_MMU__
#define __PROF80_MMU__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_PROF80_MMU_ADD(_tag, _program_map) \
	MCFG_DEVICE_ADD(_tag, PROF80_MMU, 0) \
	MCFG_DEVICE_ADDRESS_MAP(AS_PROGRAM, _program_map)



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> prof80_mmu_device

class prof80_mmu_device : public device_t,
							public device_memory_interface
{
public:
	prof80_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(z80_program_map, 8);

	void par_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mme_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	uint8_t program_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void program_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	const address_space_config m_program_space_config;

	uint8_t m_blk[16];
	bool m_enabled;
};


// device type definition
extern const device_type PROF80_MMU;



#endif
