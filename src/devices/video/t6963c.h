// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Toshiba T6963C Dot Matrix LCD Controller
    Sharp LM24014H Dot Matrix LCD Unit

**********************************************************************/

#ifndef MAME_VIDEO_T6963C_H
#define MAME_VIDEO_T6963C_H

#pragma once

#include "emupal.h"
#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> t6963c_device

class t6963c_device : public device_t, public device_memory_interface
{
	enum class auto_mode : u8
	{
		READ = 0,
		WRITE = 1,
		NONE = 2
	};

public:
	// construction/destruction
	t6963c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// CPU read/write access
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// Display configurations
	void set_md(u8 data);   // MD0, MD1, MD2, MD3, MDS
	void set_fs(u8 data);   // FS0, FS1

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

protected:
	// device-specific overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// internal helpers
	void do_command(u8 cmd);

	// configuration and object finders
	address_space_config m_display_config;
	required_region_ptr<u8> m_cgrom;
	address_space *m_display_ram;

	// internal state
	u16 m_data;
	u16 m_adp;
	auto_mode m_auto_mode;
	u16 m_graphic_home;
	u16 m_text_home;
	u16 m_graphic_area;
	u16 m_text_area;
	u16 m_cgram_offset;
	u8 m_mode;
	u8 m_display_mode;
	u8 m_font_size;
	u8 m_number_cols;
	u8 m_number_lines;
	u8 m_read_data;
};

// ======================> lm24014h_device

class lm24014h_device : public device_t
{
public:
	// construction/destruction
	lm24014h_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// CPU read/write access
	u8 read(offs_t offset) { return m_lcdc->read(offset); }
	void write(offs_t offset, u8 data) { m_lcdc->write(offset, data); }
	void set_fs(u8 data) { m_fs = data & 1; }

protected:
	// device-specific overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void lcd_palette(palette_device &palette) const;

private:
	// internal configuration
	void ram_map(address_map &map);

	// internal LCD controller
	required_device<t6963c_device> m_lcdc;
	int m_fs;
};

// device type declarations
DECLARE_DEVICE_TYPE(T6963C, t6963c_device)
DECLARE_DEVICE_TYPE(LM24014H, lm24014h_device)

#endif // MAME_VIDEO_T6963C_H
