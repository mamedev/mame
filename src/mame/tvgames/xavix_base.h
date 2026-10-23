// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
#ifndef MAME_TVGAMES_XAVIX_BASE_H
#define MAME_TVGAMES_XAVIX_BASE_H

#pragma once

#include "xavix_io_bb_ball.h"
#include "xavix_io_ir_motion.h"

#include "bus/xavix/cart.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "machine/xavix_soc.h"

#include "emupal.h"
#include "screen.h"

#include <array>

INPUT_PORTS_EXTERN(xavix);
INPUT_PORTS_EXTERN(xavix_i2c);

INPUT_PORTS_EXTERN(xavix_soft_power_control);
INPUT_PORTS_EXTERN(xavix_hard_power_control);
INPUT_PORTS_EXTERN(xavix_reset_control);

// Board interfaces shared by the original, XaviX2000 and SuperXaviX drivers.

class xavix_state : public driver_device
{
public:
	xavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_power(*this, "POWER"),
		m_soc(*this, "soc"),
		m_nvram(*this, "nvram"),
		m_screen(*this, "screen"),
		m_rom(*this, "bios"),
		m_palette(*this, "palette")
	{ }

	// Original XaviX: NTSC and ROM/NVRAM variants
	void xavix(machine_config &config) ATTR_COLD;
	void xavix_1mb(machine_config &config) ATTR_COLD;
	void xavix_2mb(machine_config &config) ATTR_COLD;
	void xavix_4mb(machine_config &config) ATTR_COLD;
	void xavix_nv(machine_config &config) ATTR_COLD;
	void xavix_nv_1mb(machine_config &config) ATTR_COLD;
	void xavix_nv_2mb(machine_config &config) ATTR_COLD;
	void xavix_nv_4mb(machine_config &config) ATTR_COLD;

	// Original XaviX: PAL variants
	void xavixp(machine_config &config) ATTR_COLD;
	void xavixp_1mb(machine_config &config) ATTR_COLD;
	void xavixp_2mb(machine_config &config) ATTR_COLD;
	void xavixp_4mb(machine_config &config) ATTR_COLD;
	void xavixp_nv(machine_config &config) ATTR_COLD;
	void xavixp_nv_1mb(machine_config &config) ATTR_COLD;
	void xavixp_nv_2mb(machine_config &config) ATTR_COLD;
	void xavixp_nv_4mb(machine_config &config) ATTR_COLD;

	// Original XaviX product configurations
	void xavix_rad_bb(machine_config &config) ATTR_COLD;
	void xavix_rad_bb2(machine_config &config) ATTR_COLD;
	void xavix_rad_mtrk(machine_config &config) ATTR_COLD;
	void xavix_rad_mtrkp(machine_config &config) ATTR_COLD;
	void xavix_rad_madf(machine_config &config) ATTR_COLD;
	void xavix_tom_tvho(machine_config &config) ATTR_COLD;
	void xavix_tomthr(machine_config &config) ATTR_COLD;
	void xavix_tak_beyu(machine_config &config) ATTR_COLD;

	// XaviX2000 configurations (xavix_xavix2000.cpp)
	void xavix2000(machine_config &config) ATTR_COLD;
	void xavix2000_4mb(machine_config &config) ATTR_COLD;
	void xavix2000_nv(machine_config &config) ATTR_COLD;
	void xavix2000_nv_4mb(machine_config &config) ATTR_COLD;
	void xavix2000_epo_sdb(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(power_off);
	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	virtual void vblank_extra() { }

protected:
	using video_standard = xavix_video_device::video_standard;

	static constexpr XTAL NTSC_MASTER_CLOCK = 3.579545_MHz_XTAL * 12;
	static constexpr u32 PAL_MASTER_CLOCK = 42'562'740;
	static constexpr u32 NTSC_LINE_CLOCKS = xavix_video_device::NTSC_LINE_CLOCKS;
	static constexpr u32 PAL_LINE_CLOCKS = xavix_video_device::PAL_LINE_CLOCKS;
	static constexpr u32 NTSC_RASTER_WIDTH = (NTSC_LINE_CLOCKS + xavix_video_device::PICTURE_PIXEL_CLOCKS - 1) / xavix_video_device::PICTURE_PIXEL_CLOCKS;
	static constexpr u32 PAL_RASTER_WIDTH = (PAL_LINE_CLOCKS + xavix_video_device::PICTURE_PIXEL_CLOCKS - 1) / xavix_video_device::PICTURE_PIXEL_CLOCKS;
	static constexpr u32 NTSC_RASTER_HEIGHT = (xavix_video_device::NTSC_FIELD_HALF_LINES + 1) / 2;
	static constexpr u32 PAL_RASTER_HEIGHT = (xavix_video_device::PAL_FIELD_HALF_LINES + 1) / 2;
	// Independent blanking boundaries on the original-chip raster.
	static constexpr u32 NTSC_HBLANK_END = 61;
	static constexpr u32 PAL_HBLANK_END = 64;
	static constexpr u32 NTSC_VBLANK_END = 20;
	static constexpr u32 PAL_VBLANK_END = 28;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;

	u32 peripheral_clock() const { return m_soc->clock() / 2; }
	void configure_microphone(machine_config &config) ATTR_COLD;
	virtual void configure_board(machine_config &config) ATTR_COLD;
	void configure_power_hold(machine_config &config) ATTR_COLD;
	void configure_ir(machine_config &config, required_device<xavix_ir_motion_device> &controllers, xavix_ir_motion_device::controller_type type) ATTR_COLD;
	void power_off_w(int state);

	virtual u8 io0_input_r(u8 direction);
	virtual u8 io1_input_r(u8 direction);
	virtual void io0_output_w(u8 data, u8 direction);
	virtual void io1_output_w(u8 data, u8 direction);
	template <typename T, std::size_t Lines>
	static T propagate_passive_levels(T levels, std::array<T, Lines> const &contacts)
	{
		for (std::size_t pass = 0; pass != Lines; ++pass)
		{
			T const previous = levels;
			for (std::size_t line = 0; line != Lines; ++line)
				if (BIT(levels, line))
					levels |= contacts[line];
			if (levels == previous)
				break;
		}

		return levels;
	}
	u8 io0_input_cb(offs_t offset, u8 mem_mask);
	u8 io1_input_cb(offs_t offset, u8 mem_mask);
	void io0_output_cb(offs_t offset, u8 data, u8 mem_mask);
	void io1_output_cb(offs_t offset, u8 data, u8 mem_mask);
	void io1_power_off_w(offs_t offset, u8 data, u8 mem_mask);

	required_ioport m_in0;
	required_ioport m_in1;
	optional_ioport m_power;
	required_device<xavix_soc_device> m_soc;
	optional_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	virtual void xavix_extbus_map(address_map &map) ATTR_COLD;

	void xavix_4mb_extbus_map(address_map &map) ATTR_COLD;
	void xavix_2mb_extbus_map(address_map &map) ATTR_COLD;
	void xavix_1mb_extbus_map(address_map &map) ATTR_COLD;

	virtual u8 external_code_r(offs_t offset);
	virtual u8 external_data_r(offs_t offset);

	u8 power_status_r();

	virtual u8 lightgun_r(offs_t offset);

	required_region_ptr<u8> m_rom;

	required_device<palette_device> m_palette;

	virtual void external_bus_ctrl_changed(unsigned index, u8 data) { }

private:
	xavix_baseball_ball_device &configure_baseball(machine_config &config) ATTR_COLD;
};

// Boards whose firmware drives IO1.7 high to switch off the supply.
template <typename Base>
class xavix_power_state_base : public Base
{
public:
	xavix_power_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: Base(mconfig, type, tag)
	{ }

protected:
	virtual void configure_board(machine_config &config) override ATTR_COLD
	{
		Base::configure_board(config);
		this->m_soc->template io_output_callback<1>().append(*this, FUNC(xavix_power_state_base::io1_power_off_w));
	}
};

class xavix_i2c_state : public xavix_state
{
public:
	xavix_i2c_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	// Original XaviX EEPROM configurations
	void xavix_i2c_24c02(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c02_2mb(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c02_4mb(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c04(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c04_1mb(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c04_2mb(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c04_4mb(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c08(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c08_4mb(machine_config &config) ATTR_COLD;
	void xavix_i2c_24c16(machine_config &config) ATTR_COLD;

	// XaviX2000 EEPROM configurations (xavix_xavix2000.cpp)
	void xavix2000_i2c_24c02(machine_config &config) ATTR_COLD;
	void xavix2000_i2c_24c04(machine_config &config) ATTR_COLD;
	void xavix2000_i2c_24c04_2mb(machine_config &config) ATTR_COLD;
	void xavix2000_i2c_24c04_4mb(machine_config &config) ATTR_COLD;
	void xavix2000_i2c_24c08(machine_config &config) ATTR_COLD;
	void xavix2000_i2c_24c08_4mb(machine_config &config) ATTR_COLD;
	void xavix2000_tak_chq(machine_config &config) ATTR_COLD;

protected:
	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<i2cmem_device> m_i2cmem;
};

class xavix_cart_state : public xavix_state
{
public:
	xavix_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig, type, tag, 0x600000)
	{ }

	void xavix_cart(machine_config &config) ATTR_COLD;
	void xavix_popira(machine_config &config) ATTR_COLD;
	void xavix_popirak(machine_config &config) ATTR_COLD;
	void xavix_ddrfammt(machine_config &config) ATTR_COLD;

	int cart_sda_r();

protected:
	xavix_cart_state(const machine_config &mconfig, device_type type, const char *tag, offs_t cart_limit)
		: xavix_state(mconfig, type, tag),
		m_cart_limit(cart_limit),
		m_cartslot(*this, "cartslot")
	{ }

	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;

	u8 cart_r(offs_t offset);
	void cart_w(offs_t offset, u8 data);

	virtual u8 external_code_r(offs_t offset) override;

	virtual void external_bus_ctrl_changed(unsigned index, u8 data) override;

	// The default cartridge window covers 0x000000-0x5fffff, even for 4 MiB cartridges.
	offs_t m_cart_limit; // exclusive external-bus byte address limit
	required_device<xavix_cart_slot_device> m_cartslot;
};

class xavix_i2c_cart_state : public xavix_cart_state
{
public:
	xavix_i2c_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_cart_state(mconfig, type, tag, 0x600000)
	{ }

	void xavix_taikodp(machine_config &config) ATTR_COLD;
	void xavix_jpopira(machine_config &config) ATTR_COLD;

protected:
	xavix_i2c_cart_state(const machine_config &mconfig, device_type type, const char *tag, offs_t cart_limit)
		: xavix_cart_state(mconfig, type, tag, cart_limit),
		m_i2cmem(*this, "i2cmem")
	{ }

	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<i2cmem_device> m_i2cmem;
};

#endif // MAME_TVGAMES_XAVIX_BASE_H
