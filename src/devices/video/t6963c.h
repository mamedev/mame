// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Toshiba T6963C Dot Matrix LCD Controller
    Sharp LM24014H Dot Matrix LCD Unit

**********************************************************************/

#ifndef MAME_VIDEO_T6963C_H
#define MAME_VIDEO_T6963C_H 1

#pragma once

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

	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

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

protected:
	// device-specific overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	// internal configuration
	void ram_map(address_map &map);

	// internal LCD controller
	required_device<t6963c_device> m_lcdc;
};

// device type declarations
DECLARE_DEVICE_TYPE(T6963C, t6963c_device)
DECLARE_DEVICE_TYPE(LM24014H, lm24014h_device)

#endif // MAME_VIDEO_T6963C_H
