// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
#ifndef MAME_TVGAMES_XAVIX_SUPERXAVIX_H
#define MAME_TVGAMES_XAVIX_SUPERXAVIX_H

#pragma once

#include "xavix.h"
#include "xavix_io_anpanmdx_sticks.h"
#include "xavix_io_bkgj_survi_buster.h"
#include "xavix_io_image_sensor.h"
#include "xavix_io_ir_step_mat.h"
#include "xavix_io_ntpj_piano.h"
#include "xavix_io_ordj_sticks.h"
#include "xavix_io_rkmj_arm.h"
#include "xavix_io_suprtvpc_controller.h"
#include "xavix_io_tfit_rackets.h"
#include "xavix_io_xavbaseb_controllers.h"
#include "xavix_io_xavbassf_tackle.h"
#include "xavix_io_xavgolf_swing.h"
#include "xavix_io_xavixport.h"

#include "bus/xavix/suprtvpc_cart.h"
#include "bus/xavix/suprtvpc_exp.h"
#include "machine/intelfsh.h"
#include "machine/s35390a.h"
#include "sound/m61545.h"
#include "sound/m62429.h"

#include <array>

//**************************************************************************
//  Shared Super XaviX State and Interfaces
//**************************************************************************

class superxavix_state : public xavix_state
{
public:
	superxavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_super_soc(*this, "soc")
		, m_extra(*this, "extra")
		, m_pio_inputs(*this, "PIO%u", 0U)
	{ }

	void superxavix(machine_config &config) ATTR_COLD;
	void superxavix_4mb(machine_config &config) ATTR_COLD;
	void superxavix_nv_4mb(machine_config &config) ATTR_COLD;

	ioport_value primary_controls_r() { return m_in0->read(); }
	DECLARE_INPUT_CHANGED_MEMBER(xavixport_reset_button);

protected:
	void configure_mapped_memory() ATTR_COLD;
	xavixport_io_device &configure_xavixport_io(machine_config &config) ATTR_COLD;

	virtual u8 read_graphics_byte(u32 address);
	u8 read_bitmap_byte(u32 address);
	virtual u8 read_plotter_byte(u32 address);
	virtual void write_plotter_byte(u32 address, u8 data);

	template <unsigned Port> u8 pio_input_r(offs_t offset, u8 mem_mask);
	template <unsigned Port> void pio_output_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<superxavix_soc_device> m_super_soc;
	optional_region_ptr<u8> m_extra;
	std::array<u8, 3> m_pio_connected_inputs{};
	std::array<u8, 2> m_legacy_connected_inputs{};

private:
	static u8 connected_inputs(ioport_port &port);
	template <unsigned Port> u8 pio_input_mask_r();
	u8 legacy_input_mask_r(offs_t port);

	required_ioport_array<3> m_pio_inputs;
};

class superxavix_i2c_state : public superxavix_state
{
public:
	superxavix_i2c_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_state(mconfig, type, tag)
		, m_i2cmem(*this, "i2cmem")
	{ }

	// Reusable EEPROM configurations
	void superxavix_i2c_24c02(machine_config &config) ATTR_COLD;
	void superxavix_i2c_24c02_4mb(machine_config &config) ATTR_COLD;
	void superxavix_i2c_24c04(machine_config &config) ATTR_COLD;
	void superxavix_i2c_24c04_4mb(machine_config &config) ATTR_COLD;
	void superxavix_i2c_24c08(machine_config &config) ATTR_COLD;
	void superxavix_i2c_24c16(machine_config &config) ATTR_COLD;

	// Product configurations
	void superxavix_mrangbat(machine_config &config) ATTR_COLD;

protected:
	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<i2cmem_device> m_i2cmem;
};

class superxavix_i2c_camera_state : public superxavix_i2c_state
{
public:
	superxavix_i2c_camera_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
		, m_camera(*this, "camera")
	{ m_legacy_connected_inputs[1] |= 0x06; }

	void superxavix_xavbowl(machine_config &config) ATTR_COLD;
	void superxavix_epo_rgfj(machine_config &config) ATTR_COLD;
	void superxavix_ban_reflective_camera(machine_config &config) ATTR_COLD;
	void superxavix_ban_utmj(machine_config &config) ATTR_COLD;
	void superxavix_ban_ult(machine_config &config) ATTR_COLD;
	void superxavix_ban_um2j(machine_config &config) ATTR_COLD;
	void superxavix_ban_gkrj(machine_config &config) ATTR_COLD;

protected:
	xavix_analog_image_sensor_device &configure_dual_reflector_camera(machine_config &config) ATTR_COLD;

	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

	required_device<xavix_analog_image_sensor_device> m_camera;

private:
	static constexpr u32 REFLECTIVE_EXPOSURE_USEC = 500;
};

class superxavix_power_state : public superxavix_state
{
public:
	superxavix_power_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_state(mconfig, type, tag)
	{ }

protected:
	virtual void configure_board(machine_config &config) override ATTR_COLD;
};

class superxavix_i2c_power_state : public superxavix_i2c_state
{
public:
	superxavix_i2c_power_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
	{ }

protected:
	virtual void configure_board(machine_config &config) override ATTR_COLD;
};

class superxavix_io0_panel_state : public superxavix_i2c_state
{
public:
	superxavix_io0_panel_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
		, m_panel(*this, "PANEL")
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual u8 io0_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;

private:
	required_ioport m_panel;
	u8 m_panel_shift;
	bool m_panel_selected;
	bool m_panel_clock;
};

class superxavix_i2c_pio1_state : public superxavix_i2c_state
{
public:
	superxavix_i2c_pio1_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
		, m_rtc(*this, "rtc")
	{ }

	// XaviXPORT sports and fitness configurations
	void superxavix_xavbox(machine_config &config) ATTR_COLD;
	void superxavix_xavcheck(machine_config &config) ATTR_COLD;
	void superxavix_xavpowrk(machine_config &config) ATTR_COLD;
	void superxavix_domfitex(machine_config &config) ATTR_COLD;

protected:
	void superxavix_i2c_24c64(machine_config &config) ATTR_COLD;
	template <unsigned Port> void configure_rtc_pio(machine_config &config) ATTR_COLD;
	microphone_device &configure_stereo_music_input(machine_config &config) ATTR_COLD;
	microphone_device &configure_xavix_music_input(machine_config &config) ATTR_COLD;
	void configure_xavix_step_sensor(machine_config &config) ATTR_COLD;

private:
	xavixport_io_device &configure_xavix_step_mat(machine_config &config) ATTR_COLD;
	void configure_pio() ATTR_COLD;

	template <unsigned Port> u8 pio_input_r(offs_t offset, u8 mem_mask);
	template <unsigned Port> void pio_output_w(offs_t offset, u8 data, u8 mem_mask);
	template <unsigned Port> u8 pio_rtc_input_r(offs_t offset, u8 mem_mask);
	template <unsigned Port> void pio_rtc_output_w(offs_t offset, u8 data, u8 mem_mask);

	optional_device<s35390a_device> m_rtc;
};

//**************************************************************************
//  Super XaviX Product State
//**************************************************************************

class superxavix_xavjmat_state : public superxavix_i2c_pio1_state
{
public:
	superxavix_xavjmat_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_pio1_state(mconfig, type, tag)
		, m_mat(*this, "mat")
	{ }

	void superxavix_xavjmat(machine_config &config) ATTR_COLD;
	void superxavix_xavaero(machine_config &config) ATTR_COLD;

private:
	void configure_mat(machine_config &config, xavix_ir_step_mat_device::protocol type) ATTR_COLD;

	required_device<xavix_ir_step_mat_device> m_mat;
};

class superxavix_xavmusic_state : public superxavix_i2c_pio1_state
{
public:
	superxavix_xavmusic_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_pio1_state(mconfig, type, tag)
		, m_music_volume(*this, "music_volume")
	{ }

	void superxavix_xavmusic(machine_config &config) ATTR_COLD;

private:
	void pio0_volume_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<m61545_device> m_music_volume;
};

class superxavix_xavmusicj_state : public superxavix_i2c_pio1_state
{
public:
	superxavix_xavmusicj_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_pio1_state(mconfig, type, tag)
		, m_music_volume(*this, "music_volume")
	{ }

	void superxavix_xavmusicj(machine_config &config) ATTR_COLD;

private:
	void pio0_volume_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<m62429_device> m_music_volume;
};

class superxavix_domdance_state : public superxavix_i2c_pio1_state, public device_sound_interface
{
public:
	superxavix_domdance_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_pio1_state(mconfig, type, tag)
		, device_sound_interface(mconfig, *this)
	{ }

	void superxavix_domdance(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	u8 analyser_data_r();
	void analyser_clock_w(offs_t offset, u8 data, u8 mem_mask);
	void analyser_select_w(offs_t offset, u8 data, u8 mem_mask);

	sound_stream *m_analyser_stream;
	u32 m_analyser_shift;
	u8 m_analyser_bits;
	bool m_analyser_selected;
	bool m_analyser_clock;
	bool m_analyser_write;
	bool m_analyser_data;
	bool m_analyser_running;
	float m_energy_sum;
	float m_energy_average;
	u16 m_energy_samples;
	u8 m_music_level;
	u8 m_beat_hold;
	u8 m_beat_delay;
};

class superxavix_xavbox_state : public superxavix_i2c_pio1_state
{
public:
	superxavix_xavbox_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_pio1_state(mconfig, type, tag)
		, m_camera(*this, "camera")
	{ m_legacy_connected_inputs[1] |= 0x06; }

	void superxavix_xavbox(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	required_device<xavix_analog_image_sensor_device> m_camera;
};

class superxavix_xavtenni_state : public superxavix_io0_panel_state
{
public:
	superxavix_xavtenni_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_io0_panel_state(mconfig, type, tag)
		, m_controllers(*this, "controllers")
	{ }

	void superxavix_xavtenni(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_ir_motion_device> m_controllers;
};

class superxavix_xavbaseb_state : public superxavix_io0_panel_state
{
public:
	superxavix_xavbaseb_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_io0_panel_state(mconfig, type, tag)
		, m_controllers(*this, "controllers")
	{ }

	void superxavix_xavbaseb(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_baseball_controllers_device> m_controllers;
};

class superxavix_xavgolf_state : public superxavix_i2c_state
{
public:
	superxavix_xavgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
		, m_swing_sensor(*this, "swing_sensor")
	{ }

	void superxavix_xavgolf(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_golf_swing_device> m_swing_sensor;
};

class superxavix_xavbassf_state : public superxavix_i2c_state
{
public:
	superxavix_xavbassf_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
		, m_fishing_cart_mcu(*this, "fishing_cart_mcu")
		, m_fishing_cart_eeprom(*this, "fishing_cart_eeprom")
		, m_fishing_tackle(*this, "fishing_tackle")
	{
		m_legacy_connected_inputs[0] |= 0x80;
		m_legacy_connected_inputs[1] |= 0x04;
	}

	void superxavix_xavbassf(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(fishing_reset_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual u8 io0_input_r(u8 direction) override;
	virtual u8 io1_input_r(u8 direction) override;
	virtual void io0_output_w(u8 data, u8 direction) override;

private:
	u8 fishing_cart_mcu_port_r();
	void fishing_cart_mcu_port_w(offs_t offset, u8 data, u8 mem_mask);
	TIMER_CALLBACK_MEMBER(fishing_host_sync);
	TIMER_CALLBACK_MEMBER(fishing_cart_mcu_sync);

	required_device<nrf24e1_device> m_fishing_cart_mcu;
	required_device<eeprom_spi_25320_device> m_fishing_cart_eeprom;
	required_device<xavix_bassfishing_device> m_fishing_tackle;
	u8 m_fishing_host_data = 0;
	u8 m_fishing_host_direction = 0;
	u8 m_fishing_cart_mcu_pins = 0xff;
};

class superxavix_tmy_rkmj_state : public superxavix_i2c_power_state
{
public:
	superxavix_tmy_rkmj_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_power_state(mconfig, type, tag)
		, m_arm_controller(*this, "arm_controller")
	{ }

	void superxavix_tmy_rkmj(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_rockman_arm_device> m_arm_controller;
};

class superxavix_epo_tfit_state : public superxavix_i2c_power_state
{
public:
	superxavix_epo_tfit_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_power_state(mconfig, type, tag)
		, m_rackets(*this, "rackets")
	{ }

	void superxavix_epo_tfit(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_tfit_rackets_device> m_rackets;
};

class superxavix_ban_bkgj_state : public superxavix_i2c_state
{
public:
	superxavix_ban_bkgj_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
		, m_survi_buster(*this, "survi_buster")
	{ }

	void superxavix_ban_bkgj(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void io1_output_w(u8 data, u8 direction) override;

private:
	TIMER_CALLBACK_MEMBER(power_off_expired);

	required_device<xavix_survi_buster_device> m_survi_buster;
	emu_timer *m_power_off_timer;
};

class superxavix_ban_ordj_state : public superxavix_i2c_state
{
public:
	superxavix_ban_ordj_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
		, m_sticks(*this, "sticks")
	{ }

	void superxavix_ban_ordj(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_hibiki_sticks_device> m_sticks;
};

class superxavix_anpanmdx_state : public superxavix_i2c_power_state
{
public:
	superxavix_anpanmdx_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_power_state(mconfig, type, tag)
		, m_sticks(*this, "sticks")
	{ }

	void superxavix_anpanmdx(machine_config &config) ATTR_COLD;

private:
	required_device<xavix_anpanman_sticks_device> m_sticks;
};

class superxavix_ban_pr2j_state : public superxavix_i2c_camera_state
{
public:
	superxavix_ban_pr2j_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_camera_state(mconfig, type, tag)
	{ }

	void superxavix_ban_pr2j(machine_config &config) ATTR_COLD;

protected:
	virtual void io1_output_w(u8 data, u8 direction) override;
};

class superxavix_udance_state : public superxavix_i2c_power_state
{
public:
	superxavix_udance_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_power_state(mconfig, type, tag)
		, m_camera(*this, "camera")
	{ }

	void superxavix_udance(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;
	virtual u8 external_data_r(offs_t address) override;

private:
	u8 rom_window_r(offs_t offset);
	u8 pio0_input_r(offs_t offset, u8 mem_mask);
	void pio0_output_w(offs_t offset, u8 data, u8 mem_mask);
	void rom_bank_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<xavix_serial_tracking_sensor_device> m_camera;
	u8 m_rom_bank;
};

class superxavix_tmy_thom_state : public superxavix_i2c_power_state
{
public:
	superxavix_tmy_thom_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_power_state(mconfig, type, tag)
		, m_wheel(*this, "WHEEL")
	{ m_legacy_connected_inputs[0] |= 0x01; }

protected:
	virtual u8 io0_input_r(u8 direction) override;

private:
	required_ioport m_wheel;
};

class superxavix_ndpbj_state : public superxavix_i2c_power_state
{
public:
	superxavix_ndpbj_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_power_state(mconfig, type, tag)
		, m_camera(*this, "camera")
		, m_panel_io(*this, "panel_io")
	{ }

	void superxavix_ndpbj(machine_config &config) ATTR_COLD;

private:
	u8 pio2_input_r(offs_t offset, u8 mem_mask);

	required_device<xavix_digital_image_sensor_device> m_camera;
	required_device<xavixport_io_device> m_panel_io;
};

class superxavix_ndpmj_state : public superxavix_i2c_power_state
{
public:
	superxavix_ndpmj_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_power_state(mconfig, type, tag)
		, m_rtc(*this, "rtc")
	{ }

	void superxavix_ndpmj(machine_config &config) ATTR_COLD;

private:
	u8 pio1_rtc_input_r(offs_t offset, u8 mem_mask);
	void pio1_rtc_output_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<s35390a_device> m_rtc;
};

class superxavix_tvpc_state : public superxavix_state
{
public:
	superxavix_tvpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_state(mconfig, type, tag)
		, m_tvpc(*this, "tvpc")
		, m_tvpc_pio(*this, "PIO%u", 0U)
	{ }

protected:
	void configure_tvpc_controller(machine_config &config) ATTR_COLD;
	virtual u8 mouse_buttons_r() = 0;
	u8 tvpc_pio0_input_r(offs_t offset, u8 mem_mask);
	void tvpc_pio0_output_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<xavix_tvpc_controller_device> m_tvpc;
	required_ioport_array<3> m_tvpc_pio;
};

class superxavix_suprtvpc_state : public superxavix_tvpc_state
{
public:
	superxavix_suprtvpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_tvpc_state(mconfig, type, tag)
		, m_i2cmem(*this, "i2cmem")
		, m_exp(*this, "exp")
		, m_mouse(*this, "mouse%u", 1U)
		, m_cart(*this, "cartslot")
		, m_rombank(*this, "rombank")
		, m_factory_loopbacks(*this, "TEST_LOOPBACKS")
	{ }

	void superxavix_suprtvpc(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual u8 mouse_buttons_r() override;
	virtual u8 external_data_r(offs_t address) override;
	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;

private:
	u8 cartridge_rom_r(offs_t offset);
	u8 pio0_input_r(offs_t offset, u8 mem_mask);
	u8 pio0_input_mask_r();
	u8 factory_loopback_mask() const;
	u8 pio1_input_r(offs_t offset, u8 mem_mask);
	void pio0_output_w(offs_t offset, u8 data, u8 mem_mask);
	void pio1_output_w(offs_t offset, u8 data, u8 mem_mask);
	u8 pio2_input_r(offs_t offset, u8 mem_mask);
	void pio2_output_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<i2cmem_device> m_i2cmem;
	required_device<suprtvpc_exp_port_device> m_exp;
	required_device_array<tvpc_mouse_port_device, 2> m_mouse;
	required_device<suprtvpc_cart_slot_device> m_cart;
	required_memory_bank m_rombank;
	required_ioport m_factory_loopbacks;
	u8 m_loopback_data = 0;
	u8 m_loopback_drive = 0;
};

class superxavix_epo_ntpj_state : public superxavix_tvpc_state
{
public:
	superxavix_epo_ntpj_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_tvpc_state(mconfig, type, tag)
		, m_i2cmem(*this, "i2cmem")
		, m_piano(*this, "piano")
		, m_mouse(*this, "mouse1")
	{ }

	void superxavix_epo_ntpj(machine_config &config) ATTR_COLD;

protected:
	virtual void configure_board(machine_config &config) override ATTR_COLD;

	virtual u8 mouse_buttons_r() override;

private:
	u8 pio1_input_r(offs_t offset, u8 mem_mask);
	u8 pio2_input_r(offs_t offset, u8 mem_mask);
	void pio0_output_w(offs_t offset, u8 data, u8 mem_mask);
	void pio2_output_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<i2cmem_device> m_i2cmem;
	required_device<xavix_piano_controller_device> m_piano;
	required_device<tvpc_mouse_port_device> m_mouse;
};

class superxavix_epo_kabj_state : public superxavix_power_state
{
public:
	superxavix_epo_kabj_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_power_state(mconfig, type, tag)
		, m_keyboard(*this, "KEY%u", 0U)
		, m_panel_controls(*this, "PIO1")
		, m_card_reader(*this, "card_reader")
	{ }

	void superxavix_epo_kabj(machine_config &config) ATTR_COLD;

protected:
	virtual u8 io0_input_r(u8 direction) override;
	virtual u8 io1_input_r(u8 direction) override;

private:
	// Each key passively joins one pair of 12 bidirectional GPIO lines.
	static constexpr std::array<std::array<u8, 2>, 60> KEY_CONTACTS{{
		{ 2, 4 }, { 3, 4 }, { 0, 5 }, { 1, 5 }, { 2, 5 }, { 4, 5 }, { 0, 6 }, { 1, 6 },
		{ 2, 6 }, { 3, 6 }, { 5, 6 }, { 0, 7 }, { 1, 7 }, { 2, 7 }, { 3, 7 }, { 5, 7 },
		{ 6, 7 }, { 0, 8 }, { 1, 8 }, { 2, 8 }, { 4, 8 }, { 5, 8 }, { 6, 8 }, { 7, 8 },
		{ 0, 9 }, { 2, 9 }, { 3, 9 }, { 4, 9 }, { 5, 9 }, { 6, 9 }, { 8, 9 }, { 0, 10 },
		{ 1, 10 }, { 2, 10 }, { 3, 10 }, { 5, 10 }, { 1, 9 }, { 7, 10 }, { 7, 9 }, { 9, 10 },
		{ 1, 11 }, { 2, 11 }, { 3, 11 }, { 4, 11 }, { 5, 11 }, { 7, 11 }, { 8, 11 }, { 9, 11 },
		{ 10, 11 }, { 0, 11 }, { 6, 10 }, { 8, 10 }, { 6, 11 }, { 3, 8 }, { 4, 10 }, { 0, 1 },
		{ 1, 2 }, { 1, 3 }, { 1, 4 }, { 4, 6 }
	}};

	u16 keyboard_levels_r();

	required_ioport_array<8> m_keyboard;
	required_ioport m_panel_controls;
	required_device<xavix_epo_kabj_card_reader_device> m_card_reader;
};

class superxavix_doradraw_state : public superxavix_power_state
{
public:
	superxavix_doradraw_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_power_state(mconfig, type, tag)
		, m_data_flash(*this, "data")
	{ }

	void superxavix_doradraw(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;
	virtual u8 external_data_r(offs_t address) override;
	virtual u8 read_graphics_byte(u32 address) override;
	virtual u8 read_plotter_byte(u32 address) override;
	virtual void write_plotter_byte(u32 address, u8 data) override;

private:
	static constexpr u32 DATA_FLASH_BASE = 0xc00000;
	static constexpr u32 DATA_FLASH_SIZE = 0x200000;

	void flush_plotter_write();
	u8 data_flash_r(offs_t offset);

	required_device<intelfsh16_device> m_data_flash;
	u32 m_flash_word_address;
	u16 m_flash_word_data;
	u8 m_flash_byte_mask;
};

#endif // MAME_TVGAMES_XAVIX_SUPERXAVIX_H
