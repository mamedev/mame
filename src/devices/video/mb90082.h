// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

 Fujitsu MB90082 OSD

***************************************************************************/

#pragma once

#ifndef __MB90082DEV_H__
#define __MB90082DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MB90082_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, MB90082, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum
{
	OSD_COMMAND = 0,
	OSD_DATA
};


// ======================> mb90082_device

class mb90082_device :  public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	mb90082_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( set_cs_line );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual const rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	UINT8 m_cmd_ff;
	UINT8 m_cmd,m_cmd_param;
	UINT8 m_reset_line;

	UINT16 m_osd_addr;
	UINT8 m_fil;
	UINT8 m_uc;
	UINT8 m_attr;

	inline UINT16 read_word(offs_t address);
	inline void write_word(offs_t address, UINT16 data);

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type MB90082;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
