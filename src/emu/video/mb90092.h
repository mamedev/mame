/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __MB90092DEV_H__
#define __MB90092DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MB90092_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, MB90092, _freq) \


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum
{
	OSD_COMMAND = 0,
	OSD_DATA
};


// ======================> mb90092_device

class mb90092_device :	public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	mb90092_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

private:
	UINT8 m_cmd_ff;
	UINT8 m_cmd,m_cmd_param;
	UINT16 m_osd_addr;

	inline UINT16 read_word(offs_t address);
	inline void write_word(offs_t address, UINT16 data);

	const address_space_config		m_space_config;
};


// device type definition
extern const device_type MB90092;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
