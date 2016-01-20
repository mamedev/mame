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
	huc6272_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );


protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	inline UINT32 read_dword(offs_t address);
	inline void write_dword(offs_t address, UINT32 data);
	UINT8 m_register;
	UINT32 m_kram_addr_r, m_kram_addr_w;
	UINT16 m_kram_inc_r,m_kram_inc_w;
	UINT8 m_kram_page_r,m_kram_page_w;
	UINT32 m_page_setting;
	UINT8 m_bgmode[4];

	struct{
		UINT8 addr;
		UINT8 ctrl;
		UINT16 data[16];
	}m_micro_prg;

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type huc6272;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
