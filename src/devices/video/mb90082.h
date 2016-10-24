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
	mb90082_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void set_cs_line(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	uint8_t m_cmd_ff;
	uint8_t m_cmd,m_cmd_param;
	uint8_t m_reset_line;

	uint16_t m_osd_addr;
	uint8_t m_fil;
	uint8_t m_uc;
	uint8_t m_attr;

	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type MB90082;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
