// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __SEGACDBLOCKDEV_H__
#define __SEGACDBLOCKDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEGACDBLOCK_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SEGACDBLOCK, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> segacdblock_device

class segacdblock_device : public device_t,
						   public device_memory_interface
{
public:
	// construction/destruction
	segacdblock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );
	
	DECLARE_READ16_MEMBER( cr0_r );
	DECLARE_READ16_MEMBER( cr1_r );
	DECLARE_READ16_MEMBER( cr2_r );
	DECLARE_READ16_MEMBER( cr3_r );	

	DECLARE_WRITE16_MEMBER( cr0_w );
	DECLARE_WRITE16_MEMBER( cr1_w );
	DECLARE_WRITE16_MEMBER( cr2_w );
	DECLARE_WRITE16_MEMBER( cr3_w );
	
	DECLARE_READ16_MEMBER( hirq_r );	
	DECLARE_WRITE16_MEMBER( hirq_w );

	DECLARE_READ16_MEMBER( hirq_mask_r );	
	DECLARE_WRITE16_MEMBER( hirq_mask_w );	
protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
	address_space_config        m_space_config;
	address_space *m_space;
	
private:

	UINT16 m_cr[4];
	UINT16 m_hirq_mask;
	UINT16 m_hirq;
	bool m_hs;
	
	void sh1_writes_registers(UINT16 r1, UINT16 r2, UINT16 r3, UINT16 r4);
	void set_flag(UINT16 which);
	void clear_flag(UINT16 which);
};


// device type definition
extern const device_type SEGACDBLOCK;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
