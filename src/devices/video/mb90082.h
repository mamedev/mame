// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

 Fujitsu MB90082 OSD

***************************************************************************/

#ifndef MAME_VIDEO_MB90082_H
#define MAME_VIDEO_MB90082_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> mb90082_device

class mb90082_device :  public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	mb90082_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(uint8_t data);
	void set_cs_line(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	enum
	{
		OSD_COMMAND = 0,
		OSD_DATA
	};

	uint8_t m_cmd_ff;
	uint8_t m_cmd, m_cmd_param;
	uint8_t m_reset_line;

	uint16_t m_osd_addr;
	uint8_t m_fil;
	uint8_t m_uc;
	uint8_t m_attr;

	const address_space_config m_space_config;
	required_memory_region m_char_rom;

	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);

	void mb90082_vram(address_map &map) ATTR_COLD;
};


// device type declaration
DECLARE_DEVICE_TYPE(MB90082, mb90082_device)

#endif // MAME_VIDEO_MB90082_H
