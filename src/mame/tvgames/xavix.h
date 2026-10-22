// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
#ifndef MAME_TVGAMES_XAVIX_H
#define MAME_TVGAMES_XAVIX_H

#pragma once

#include "xavix_io_bb_ball.h"
#include "xavix_io_bistro_sensor.h"
#include "xavix_io_card.h"
#include "xavix_io_cardnight.h"
#include "xavix_io_comt_baton.h"
#include "xavix_io_crok_controllers.h"
#include "xavix_io_daig_robots.h"
#include "xavix_io_duelmast_scanner.h"
#include "xavix_io_ebox_gloves.h"
#include "xavix_io_evio_bow.h"
#include "xavix_io_image_sensor.h"
#include "xavix_io_ir.h"
#include "xavix_io_ir_motion.h"
#include "xavix_io_ltv_tam_ir.h"
#include "xavix_io_mail_mascot.h"
#include "xavix_io_omt_drive.h"
#include "xavix_io_stad_bat.h"
#include "xavix_io_wamg_sensor.h"
#include "xavix_io_wdg_granblade.h"
#include "xavix_io_wdg_webknight.h"
#include "xavix_io_xavbaseb_controllers.h"

#include "bus/xavix/cart.h"
#include "bus/xavix/tvpc_mouse.h"
#include "machine/i2chle.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/xavix_soc.h"
#include "sound/flt_vol.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>
#include <array>
#include <memory>

INPUT_PORTS_EXTERN(xavix);
INPUT_PORTS_EXTERN(xavix_i2c);

INPUT_PORTS_EXTERN(xavix_soft_power_control);
INPUT_PORTS_EXTERN(xavix_hard_power_control);
INPUT_PORTS_EXTERN(xavix_reset_control);

//**************************************************************************
//  Shared Platform State
//**************************************************************************

class xavix_state : public driver_device
{
public:
	xavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_power(*this, "POWER"),
		m_reel(*this, "REEL"),
		m_lightgun1(*this, "GUN1_%u", 0U),
		m_lightgun2(*this, "GUN2_%u", 0U),
		m_soc(*this, "soc"),
		m_nvram(*this, "nvram"),
		m_screen(*this, "screen"),
		m_power_hold_timer(*this, "power_hold"),
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
	void xavix_tak_beyu(machine_config &config) ATTR_COLD;

	// XaviX2000 configurations (xavix_xavix2000.cpp)
	void xavix2000(machine_config &config) ATTR_COLD;
	void xavix2000_4mb(machine_config &config) ATTR_COLD;
	void xavix2000_nv(machine_config &config) ATTR_COLD;
	void xavix2000_nv_4mb(machine_config &config) ATTR_COLD;
	void xavix2000_epo_sdb(machine_config &config) ATTR_COLD;

	int fishing_reel_r();

	DECLARE_INPUT_CHANGED_MEMBER(power_off);
	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	virtual void vblank_extra() { }

protected:
	using video_standard = xavix_video_device::video_standard;

	class screen_output
	{
	public:
		void start(running_machine &machine) ATTR_COLD;
		void stop(running_machine &machine) ATTR_COLD;
		void update(screen_device &screen, bitmap_rgb32 const &bitmap, rectangle const &cliprect, rectangle area);
		void restore(screen_device &screen);

	private:
		// Keep the published picture stable across partial updates and raster-size changes.
		bitmap_rgb32 m_bitmap[2];
		render_texture *m_texture = nullptr;
		unsigned m_index = 0;
	};

	static constexpr u32 NTSC_MASTER_CLOCK = 42'954'540;
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
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	u32 peripheral_clock() const { return m_soc->clock() / 2; }
	void configure_microphone(machine_config &config) ATTR_COLD;
	virtual void configure_board(machine_config &config) ATTR_COLD;
	void configure_power_hold(machine_config &config) ATTR_COLD;
	void configure_ir(machine_config &config, required_device<xavix_ir_motion_device> &controllers, xavix_ir_motion_device::controller_type type) ATTR_COLD;
	void configure_lightgun_scanlines(machine_config &config, u8 event_mask) ATTR_COLD;
	u8 lightgun_position_r(unsigned player, offs_t offset, u16 x_offset = 0, u16 y_scale = 0x100, u16 y_offset = 0, u16 x_scale = 0x100);
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
	optional_ioport m_reel;
	optional_ioport_array<2> m_lightgun1;
	optional_ioport_array<2> m_lightgun2;
	u8 m_lightgun_event_mask = 0;
	required_device<xavix_soc_device> m_soc;
	optional_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	virtual void xavix_extbus_map(address_map &map) ATTR_COLD;

	void xavix_4mb_extbus_map(address_map &map) ATTR_COLD;
	void xavix_2mb_extbus_map(address_map &map) ATTR_COLD;
	void xavix_1mb_extbus_map(address_map &map) ATTR_COLD;

	// screen updates
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	screen_output m_screen_output;

	virtual u8 external_code_r(offs_t offset);
	virtual u8 external_data_r(offs_t offset);

	// Selected boards alternate IO1 bit 7 to maintain an external RC power hold.
	optional_device<timer_device> m_power_hold_timer;
	u8 m_power_hold_level;
	void io1_power_hold_w(offs_t offset, u8 data, u8 mem_mask);
	TIMER_DEVICE_CALLBACK_MEMBER(power_hold_expired);

	u8 power_status_r();

	virtual u8 lightgun_r(offs_t offset);
	virtual u8 lightgun_event_mask() { return m_lightgun_event_mask; }
	TIMER_DEVICE_CALLBACK_MEMBER(lightgun_scanline_cb);

	required_region_ptr<u8> m_rom;

	required_device<palette_device> m_palette;

	virtual void external_bus_ctrl_changed(unsigned index, u8 data) { }

private:
	xavix_baseball_ball_device &configure_baseball(machine_config &config) ATTR_COLD;
};

//**************************************************************************
//  Original XaviX Interfaces and Product States
//**************************************************************************

class xavix_io0_matrix_state : public xavix_state
{
public:
	xavix_io0_matrix_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_matrix(*this, "ROW%u", 0U)
	{ }

protected:
	virtual u8 io0_input_r(u8 direction) override;

private:
	required_ioport_array<2> m_matrix;
};

class xavix_io0_matrix_power_state : public xavix_io0_matrix_state
{
public:
	xavix_io0_matrix_power_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_io0_matrix_state(mconfig, type, tag)
	{ }

protected:
	virtual void configure_board(machine_config &config) override ATTR_COLD;
};

class xavix_rad_crdn_state : public xavix_io0_matrix_power_state
{
public:
	xavix_rad_crdn_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_io0_matrix_power_state(mconfig, type, tag)
		, m_remote(*this, "remote")
	{ }

	void xavix_rad_crdn(machine_config &config) ATTR_COLD;
	void xavix_rad_crdnp(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(remote_reset);

protected:
	virtual u8 io0_input_r(u8 direction) override;
	virtual u8 io1_input_r(u8 direction) override;

private:
	void configure_remote(machine_config &config) ATTR_COLD;
	required_device<xavix_cardnight_remote_device> m_remote;
};

class xavix_power_state : public xavix_state
{
public:
	xavix_power_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
	{ }

	void xavix_tomthr(machine_config &config) ATTR_COLD;

protected:
	virtual void configure_board(machine_config &config) override ATTR_COLD;
};

class xavix_epo_quiz_state : public xavix_power_state
{
public:
	xavix_epo_quiz_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_power_state(mconfig, type, tag)
		, m_round_pads(*this, "ROUND_PADS")
		, m_an6(*this, "AN6")
	{ }

	void xavix_epo_quiz(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io0_input_r(u8 direction) override;
	virtual u8 io1_input_r(u8 direction) override;

private:
	u8 an6_r();

	required_ioport m_round_pads;
	required_ioport m_an6;
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

class xavix_i2c_power_state : public xavix_i2c_state
{
public:
	xavix_i2c_power_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

protected:
	virtual void configure_board(machine_config &config) override ATTR_COLD;
};

class xavix_tomplc_state : public xavix_i2c_power_state
{
public:
	xavix_tomplc_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_power_state(mconfig, type, tag)
		, m_lever_test(*this, "LEVER_TEST")
	{ }

	void xavix_tomplc(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io0_input_r(u8 direction) override;
	virtual u8 io1_input_r(u8 direction) override;

private:
	required_ioport m_lever_test;
};

class xavix_ir_state : public xavix_state
{
public:
	xavix_ir_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_controllers(*this, "controllers")
	{ }

	void xavix_ir(machine_config &config) ATTR_COLD;
	void xavix_ir_1mb(machine_config &config) ATTR_COLD;
	void xavix_epo_epp(machine_config &config) ATTR_COLD;
	void xavix_epo_epp2(machine_config &config) ATTR_COLD;
	void xavix_epo_strk(machine_config &config) ATTR_COLD;
	void xavixp_epo_strk(machine_config &config) ATTR_COLD;
	void xavix_epo_tenn(machine_config &config) ATTR_COLD;
	void xavixp_ir_1mb(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_ir_motion_device> m_controllers;
};

class xavix_ir_power_state : public xavix_ir_state
{
public:
	xavix_ir_power_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_ir_state(mconfig, type, tag)
	{ }

protected:
	virtual void configure_board(machine_config &config) override ATTR_COLD;
};

class xavix_epo_baseball_state : public xavix_state
{
public:
	xavix_epo_baseball_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_controllers(*this, "controllers")
	{ }

	void xavix_epo_esdx(machine_config &config) ATTR_COLD;
	void xavix_epo_esht(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;

private:
	static constexpr unsigned IR_CARRIER_GAP_US = 60;
	static constexpr u8 IR_CARRIER_MIN_PULSES = 3;

	void configure_controllers(machine_config &config) ATTR_COLD;
	TIMER_CALLBACK_MEMBER(ir_receiver_quiet);

	required_device<xavix_baseball_controllers_device> m_controllers;
	emu_timer *m_ir_receiver_timer;
	attotime m_ir_last_carrier_edge;
	u8 m_ir_carrier_pulses = 0;
	bool m_ir_tx_level = false;
	bool m_ir_receiver_active = false;
};

class xavix_epo_stad_state : public xavix_power_state
{
public:
	xavix_epo_stad_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_power_state(mconfig, type, tag)
		, m_controllers(*this, "controllers")
	{ }

	void xavix_epo_stad(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_stadium_bat_device> m_controllers;
};

class xavix_tak_comt_state : public xavix_power_state
{
public:
	xavix_tak_comt_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_power_state(mconfig, type, tag)
		, m_baton_controller(*this, "baton")
	{ }

	void xavix_tak_comt(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io1_input_r(u8 direction) override;

private:
	required_device<xavix_comet_baton_device> m_baton_controller;
};

class xavix_tak_wdg_state : public xavix_power_state
{
public:
	xavix_tak_wdg_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_power_state(mconfig, type, tag)
		, m_granblade(*this, "granblade")
		, m_webknight(*this, "webknight")
	{ }

	void xavix_tak_wdg(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_granblade_device> m_granblade;
	required_device<xavix_webknight_device> m_webknight;
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

class xavix_ekara_state : public xavix_cart_state
{
public:
	xavix_ekara_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig, type, tag),
		m_extra0(*this, "EXTRA0"),
		m_extra1(*this, "EXTRA1"),
		m_digits(*this, "digit%u", 0U),
		m_matrix_select(0)
	{ }

	void xavix_ekara(machine_config &config) ATTR_COLD;
	void xavix_isinger(machine_config &config) ATTR_COLD;
	void xavix_epitch(machine_config &config) ATTR_COLD;

	template <unsigned Column> int matrix_column_r() { return matrix_column_r(Column); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	int matrix_column_r(unsigned column);

	required_ioport m_extra0;
	required_ioport m_extra1;
	output_finder<2> m_digits;

	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

	u8 m_matrix_select;
	bool m_has_segment_display = false;
	bool m_segment_data_pending;
};

// CT675A-F register stub; video processing is not emulated.
class xavix_hikara_i2c_device : public device_t, public i2c_hle_interface
{
public:
	xavix_hikara_i2c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

protected:
	virtual void device_start() override ATTR_COLD { save_item(NAME(m_regs)); }
	virtual void device_reset() override ATTR_COLD { std::ranges::fill(m_regs, 0); }
	virtual u8 read_data(u16 offset) override { return m_regs[offset & 0xff]; }
	virtual void write_data(u16 offset, u8 data) override { m_regs[offset & 0xff] = data; }
	virtual const char *get_tag() override { return tag(); }

private:
	u8 m_regs[0x100];
};

DECLARE_DEVICE_TYPE(XAVIX_HIKARA_I2C, xavix_hikara_i2c_device)

class xavix_hikara_state : public xavix_ekara_state
{
public:
	xavix_hikara_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_ekara_state(mconfig, type, tag),
		m_extra2(*this, "EXTRA2"),
		m_extra3(*this, "EXTRA3"),
		m_ct675_i2c(*this, "ct675_i2c%u", 0U),
		m_lcd_screen(*this, "lcd"),
		m_microphone(*this, "microphone%u", 1U),
		m_microphone_volume(*this, "MICVOLUME%u", 1U),
		m_volume_filter(*this, "volume%u", 0U)
	{ }

	void xavix_hikara(machine_config &config) ATTR_COLD;

	int matrix_column0_r();
	int matrix_column1_r();
	int matrix_column2_r();
	int matrix_column3_r();
	DECLARE_INPUT_CHANGED_MEMBER(microphone_volume_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void device_stop() override ATTR_COLD;

private:
	struct lcd_state
	{
		std::array<u16, 16> regs{};
		u16 shift = 0;
		u16 read_shift = 0;
		u8 bits = 0;
		bool select = true;
		bool clock = true;
		bool reading = false;
		bool output = true;
		s16 brightness = 0;
	};

	template <unsigned Index> void ct675_i2c_sda_w(int state) { m_ct675_i2c_sda[Index] = state; }
	void reset_lcd_registers();
	void lcd_serial_w(bool select, bool clock, bool data);
	void lcd_register_w(unsigned address, u16 data);
	void lcd_vblank(int state);
	void lcd_screen_vblank(int state);
	u32 screen_update_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_volume();

	required_ioport m_extra2;
	required_ioport m_extra3;
	required_device_array<xavix_hikara_i2c_device, 3> m_ct675_i2c;
	bool m_ct675_i2c_sda[3]{ true, true, true };

	required_device<screen_device> m_lcd_screen;
	required_device_array<microphone_device, 2> m_microphone;
	required_ioport_array<2> m_microphone_volume;
	required_device_array<filter_volume_device, 2> m_volume_filter;
	bitmap_rgb32 m_lcd_source_bitmap;
	screen_output m_lcd_output;
	lcd_state m_lcd;
	u8 m_volume_step;
	bool m_volume_clock;
};

class xavix_popira2_state : public xavix_cart_state
{
public:
	xavix_popira2_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig,type,tag),
		m_p2(*this, "P2")
	{ }

	void xavix_popira2(machine_config &config) ATTR_COLD;

protected:
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	u8 player2_adc0_r();
	u8 player2_adc1_r();

	required_ioport m_p2;
};

class xavix_evio_state : public xavix_cart_state
{
public:
	xavix_evio_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig,type,tag)
		, m_bow(*this, "bow%u", 1U)
		, m_digit(*this, "digit")
	{ }

	void xavix_evio(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	required_device_array<xavix_evio_bow_device, 2> m_bow;
	output_finder<> m_digit;
};

class xavix_tak_daig_state : public xavix_cart_state
{
public:
	xavix_tak_daig_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig, type, tag)
		, m_robots(*this, "robots")
	{ }

	void xavix_tak_daig(machine_config &config) ATTR_COLD;

private:
	virtual u8 lightgun_r(offs_t offset) override;

	required_device<xavix_daigunder_robots_device> m_robots;
};

class xavix_gcslottv_state : public xavix_cart_state
{
public:
	xavix_gcslottv_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig, type, tag, 0x800000)
	{ }

	void xavix_gcslottv(machine_config &config) ATTR_COLD;
	int payout_home_r() { return m_payout_home; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	static constexpr unsigned PAYOUT_PHASE_MS = 100; // TODO: Measure dispenser feedback timing.
	TIMER_CALLBACK_MEMBER(payout_position_changed);

	emu_timer *m_payout_timer;
	bool m_payout_motor = false;
	bool m_payout_home = true;
};

class xavix_has_wamg_state : public xavix_i2c_power_state
{
public:
	xavix_has_wamg_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_power_state(mconfig, type, tag)
		, m_sensor(*this, "putter")
	{ }

	void xavix_has_wamg(machine_config &config) ATTR_COLD;

private:
	virtual void io0_output_w(u8 data, u8 direction) override;

	required_device<xavix_wamg_sensor_device> m_sensor;
};

class xavix_rad_hnt_state : public xavix_power_state
{
public:
	xavix_rad_hnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_power_state(mconfig, type, tag)
	{ }

	void xavix_rad_hnt(machine_config &config) ATTR_COLD;
	void xavix_rad_hnt2(machine_config &config) ATTR_COLD;

private:
	virtual void vblank_extra() override;
	virtual u8 lightgun_r(offs_t offset) override;
};

class xavix_gungunad_state : public xavix_power_state
{
public:
	xavix_gungunad_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_power_state(mconfig, type, tag)
	{ }

	void xavix_gungunad(machine_config &config) ATTR_COLD;

private:
	virtual u8 lightgun_r(offs_t offset) override;
	virtual u8 lightgun_event_mask() override { return m_lightgun_event_mask & (BIT(m_in0->read(), 6) ? 0xff : 0xfd); }
};

class xavix_gungunrv_state : public xavix_i2c_state
{
public:
	xavix_gungunrv_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
		, m_gun_recoil(*this, "Player1_Gun_Recoil")
	{ }

	void xavix_gungunrv(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;
	virtual u8 lightgun_r(offs_t offset) override;
	virtual u8 lightgun_event_mask() override { return m_lightgun_event_mask & (BIT(m_in1->read(), 1) ? 0xff : 0xfd); }

	output_finder<> m_gun_recoil;
};

class xavix_pl1000_state : public xavix_state
{
public:
	xavix_pl1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
	{ }

	void init_pl1000();
	void xavix_pl1000(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	virtual void io1_output_w(u8 data, u8 direction) override;
	virtual u8 lightgun_r(offs_t offset) override;
	u8 m_io1_out;
};

class xavix_tomshoot_state : public xavix_i2c_power_state
{
public:
	xavix_tomshoot_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_power_state(mconfig, type, tag)
	{ }

private:
	virtual void vblank_extra() override;
	virtual u8 lightgun_r(offs_t offset) override;
};

class xavix_epo_dmon_state : public xavix_i2c_state
{
public:
	xavix_epo_dmon_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	void xavix_epo_dmon(machine_config &config) ATTR_COLD;

private:
	virtual void vblank_extra() override;
	virtual u8 lightgun_r(offs_t offset) override;
};

class xavix_epo_crok_state : public xavix_i2c_power_state
{
public:
	xavix_epo_crok_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_power_state(mconfig, type, tag)
		, m_controllers(*this, "controllers")
		, m_banker(*this, "BANKER")
	{ }

	void xavix_epo_crok(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(factory_link_send);
	int factory_link_r() { return m_factory_link_line; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<xavix_croket_controllers_device> m_controllers;
	required_ioport m_banker;

	TIMER_CALLBACK_MEMBER(factory_link_tick);
	emu_timer *m_factory_link_timer;
	u8 m_factory_link_phase;
	u8 m_factory_link_bit;
	u8 m_factory_link_line = 0;
};

class xavix_jarajal_state : public xavix_power_state
{
public:
	xavix_jarajal_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_power_state(mconfig, type, tag)
		, m_hopper(*this, "hopper")
	{ }

	void xavix_jarajal(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void io0_output_w(u8 data, u8 direction) override;

private:
	void hopper_sensor_w(int state);

	required_device<hopper_device> m_hopper;
};

class xavix_epo_mj_state : public xavix_i2c_power_state
{
public:
	xavix_epo_mj_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_power_state(mconfig, type, tag)
	{ }

	void xavix_epo_mj(machine_config &config) ATTR_COLD;

protected:
	virtual void io1_output_w(u8 data, u8 direction) override;
};

class xavix_ltv_tam_state : public xavix_i2c_state
{
public:
	xavix_ltv_tam_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
		, m_tamagotchi(*this, "tamagotchi")
	{ }

	void xavix_ltv_tam(machine_config &config) ATTR_COLD;

private:
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<xavix_tamagotchi_ir_device> m_tamagotchi;
};

class xavix_epo_guru_state : public xavix_state
{
public:
	xavix_epo_guru_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_motor_drive(*this, "pedal_motor_drive_%u", 0U)
	{ }

	void xavix_epo_guru(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	virtual void io0_output_w(u8 data, u8 direction) override;

	output_finder<2> m_motor_drive;
};

class xavix_rad_rh_state : public xavix_state
{
public:
	xavix_rad_rh_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_matrix(*this, "ROW%u", 0U)
	{ }

protected:
	virtual u8 io0_input_r(u8 direction) override;

private:
	required_ioport_array<2> m_matrix;
};

class xavix_tak_zuba_state : public xavix_i2c_state
{
public:
	xavix_tak_zuba_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
		, m_controllers(*this, "controllers")
	{ }

	void xavix_tak_zuba(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_ir_motion_device> m_controllers;
};

class xavix_tvpc_state : public xavix_i2c_power_state
{
public:
	xavix_tvpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_power_state(mconfig, type, tag)
		, m_keyboard(*this, "KEY%u", 0U)
		, m_mouse(*this, "mouse1")
	{ }

	void xavix_tvpc(machine_config &config) ATTR_COLD;
	void xavix_tvpc_hk(machine_config &config) ATTR_COLD;

private:
	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;
	virtual u8 io0_input_r(u8 direction) override;
	u8 keyboard_r(offs_t offset);

	required_ioport_array<8> m_keyboard;
	required_device<tvpc_mouse_port_device> m_mouse;
};

class xavix_bistro_state : public xavix_power_state
{
public:
	xavix_bistro_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_power_state(mconfig, type, tag)
		, m_sensor(*this, "sensor")
	{ }

	void xavix_bistro(machine_config &config) ATTR_COLD;

private:
	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<xavix_bistro_sensor_device> m_sensor;
};

class xavix_epo_mail_state : public xavix_i2c_power_state
{
public:
	xavix_epo_mail_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_power_state(mconfig, type, tag)
		, m_keyboard(*this, "KEY%u", 0U)
		, m_mascot(*this, "mascot")
	{ }

	void xavix_epo_mail(machine_config &config) ATTR_COLD;

private:
	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;
	u8 keyboard_r();

	required_ioport_array<9> m_keyboard;
	required_device<xavix_mailcot_mascot_device> m_mascot;
};

class xavix_hippofr_state : public xavix_state
{
public:
	xavix_hippofr_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_keyboard(*this, "KEY%u", 0U)
		, m_directions(*this, "DIRECTIONS")
		, m_activity(*this, "ACTIVITY")
	{ }

protected:
	virtual u8 io0_input_r(u8 direction) override;
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	// Each scanned button passively joins one pair of ten bidirectional GPIO lines.
	static constexpr std::array<std::array<u8, 2>, 30> KEY_CONTACTS{{
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 8 },
		{ 0, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 5, 7 }, { 6, 8 }, { 7, 9 },
		{ 0, 3 }, { 1, 4 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 6, 9 }, { 2, 6 },
		{ 3, 7 }, { 4, 8 }, { 1, 5 }, { 0, 9 }, { 0, 5 }, { 0, 4 }
	}};

	u16 panel_levels_r();

	required_ioport_array<4> m_keyboard;
	required_ioport m_directions;
	required_ioport m_activity;
};

class xavix_multiplt_state : public xavix_state
{
public:
	xavix_multiplt_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_controls(*this, "CONTROLS")
		, m_wheel(*this, "WHEEL")
		, m_lever(*this, "LEVER")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void xavix_multiplt(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual u8 io0_input_r(u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	required_ioport m_controls;
	required_ioport m_wheel;
	required_ioport m_lever;
	output_finder<3> m_lamps;
};

//**************************************************************************
//  XaviX2000 Product State
//**************************************************************************

class xavix2000_epo_ebox_state : public xavix_state
{
public:
	xavix2000_epo_ebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_gloves(*this, "gloves")
	{ }

	void xavix2000_epo_ebox(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_ebox_gloves_device> m_gloves;
};

class xavix2000_epo_es2j_state : public xavix_state
{
public:
	xavix2000_epo_es2j_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_controllers(*this, "P%u", 1U)
		, m_card_reader(*this, "card_reader")
	{ }

	void xavix2000_epo_es2j(machine_config &config) ATTR_COLD;

private:
	virtual u8 io0_input_r(u8 direction) override;

	required_ioport_array<2> m_controllers;
	required_device<xavix_epo_es2j_card_reader_device> m_card_reader;
};

class xavix2000_duelmast_state : public xavix_i2c_cart_state
{
public:
	xavix2000_duelmast_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_cart_state(mconfig, type, tag, 0x800000)
		, m_scanner(*this, "scanner")
	{ }

	void xavix2000_duelmast(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;

private:
	required_device<xavix_duelmast_scanner_device> m_scanner;
};

class xavix2000_camera_state : public xavix_state
{
public:
	xavix2000_camera_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_camera(*this, "camera")
	{ }

	void xavix2000_epo_hamc(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	required_device<xavix_analog_image_sensor_device> m_camera;
};

class xavix2000_i2c_camera_state : public xavix_i2c_state
{
public:
	xavix2000_i2c_camera_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
		, m_camera(*this, "camera")
	{ }

	void xavix2000_epo_golf(machine_config &config) ATTR_COLD;
	void xavix2000_epo_bowl(machine_config &config) ATTR_COLD;
	void xavix2000_epo_mini(machine_config &config) ATTR_COLD;
	void xavix2000_ttv_sw(machine_config &config) ATTR_COLD;
	void xavix2000_i2c_24c02_camera(machine_config &config) ATTR_COLD;
	void xavix2000_ban_onep(machine_config &config) ATTR_COLD;
	void xavix2000_tom_dpgm(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<xavix_analog_image_sensor_device> m_camera;

	void configure_camera(machine_config &config, xavix_analog_image_sensor_device::target_shape shape) ATTR_COLD;
};

class xavix2000_drgqst_state : public xavix2000_i2c_camera_state
{
public:
	xavix2000_drgqst_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix2000_i2c_camera_state(mconfig, type, tag)
		, m_factory_contacts(*this, "FACTORY")
	{ }

	void xavix2000_drgqst(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(factory_contact_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	void update_eeprom_bus();

	required_ioport m_factory_contacts;
	u8 m_io1_data;
	u8 m_io1_direction;
};

class xavix2000_ban_omt_state : public xavix2000_i2c_camera_state
{
public:
	xavix2000_ban_omt_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix2000_i2c_camera_state(mconfig, type, tag)
		, m_drive(*this, "drive")
	{ }

	void xavix2000_ban_omt(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_omt_drive_device> m_drive;
};

#endif // MAME_TVGAMES_XAVIX_H
