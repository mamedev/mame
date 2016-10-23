// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __huc6272DEV_H__
#define __huc6272DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_HUC6272_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, huc6272, _freq)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> huc6272_device

class huc6272_device :  public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);


protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	inline uint32_t read_dword(offs_t address);
	inline void write_dword(offs_t address, uint32_t data);
	uint8_t m_register;
	uint32_t m_kram_addr_r, m_kram_addr_w;
	uint16_t m_kram_inc_r,m_kram_inc_w;
	uint8_t m_kram_page_r,m_kram_page_w;
	uint32_t m_page_setting;
	uint8_t m_bgmode[4];

	struct{
		uint8_t addr;
		uint8_t ctrl;
		uint16_t data[16];
	}m_micro_prg;

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type huc6272;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
