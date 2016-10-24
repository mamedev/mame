// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Mitsubishi M50458 OSD chip

***************************************************************************/

#pragma once

#ifndef __M50458DEV_H__
#define __M50458DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_M50458_ADD(_tag,_freq,_screen) \
	MCFG_DEVICE_ADD(_tag, M50458,_freq) \
	MCFG_VIDEO_SET_SCREEN(_screen)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum m50458_state_t
{
	OSD_SET_ADDRESS = 0,
	OSD_SET_DATA
};

// ======================> m50458_device

class m50458_device :   public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	m50458_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write_bit(int state);
	void set_cs_line(int state);
	void set_clock_line(int state);
	void vreg_120_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vreg_121_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vreg_122_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vreg_123_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vreg_124_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vreg_125_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vreg_126_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vreg_127_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	int m_latch;
	int m_reset_line;
	int m_clock_line;
	uint16_t m_current_cmd;
	int m_cmd_stream_pos;
	uint16_t m_osd_addr;
	std::unique_ptr<uint8_t[]> m_shadow_gfx;

	uint8_t m_bg_pen;
	uint8_t m_phase;
	uint8_t m_scrf,m_scrr;
	uint8_t m_space;
	uint8_t m_hsz1,m_hsz2,m_hsz3;
	uint8_t m_vsz1,m_vsz2,m_vsz3;
	uint8_t m_blink;

	m50458_state_t m_osd_state;

private:
	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);

	const address_space_config      m_space_config;
};


// device type definition
extern const device_type M50458;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
