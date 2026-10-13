// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_SOC_H
#define MAME_MACHINE_XAVIX_SOC_H

#pragma once

#include "xavix_adc.h"
#include "xavix_anport.h"
#include "xavix_dma.h"
#include "xavix_intc.h"
#include "xavix_io.h"
#include "xavix_math.h"
#include "xavix_pio.h"
#include "xavix_plotter.h"
#include "xavix_sound.h"
#include "xavix_timer.h"
#include "xavix_video.h"

#include "cpu/m6502/xavix2000.h"

#include <array>
#include <span>
#include <string>
#include <utility>

class xavix_soc_device : public device_t, public device_mixer_interface
{
public:
	xavix_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T, typename U> void set_program_map(T &object, void (U::*map)(address_map &))
	{
		m_program_map_device = &object;
		m_program_map = address_map_constructor(map, object.tag(), &downcast<U &>(object));
	}
	template <typename T, typename U> void set_external_map(T &object, void (U::*map)(address_map &))
	{
		m_external_map_device = &object;
		m_external_map = address_map_constructor(map, object.tag(), &downcast<U &>(object));
	}
	template <typename T> void set_screen(T &&tag)
	{
		m_screen.set_tag(std::forward<T>(tag));
		configure_screen();
	}
	template <typename T> void set_palette(T &&tag) { m_video.lookup()->set_palette(std::forward<T>(tag)); }
	template <typename T> void set_gfx_region(T &&tag) { m_video.lookup()->set_gfx_region(std::forward<T>(tag)); }
	void set_video_standard(xavix_video_device::video_standard standard) { m_video.lookup()->set_video_standard(standard); }
	void set_data_address_mask(u32 mask) { m_maincpu.lookup()->set_data_address_mask(mask); }

	auto sound_sample_read_callback() { return m_sound.lookup()->sample_read_callback(); }
	auto dma_source_read_callback() { return m_dma.lookup()->source_read_callback(); }
	auto lightgun_read_callback() { return m_lightgun_read_cb.bind(); }
	auto power_read_callback() { return m_power_read_cb.bind(); }
	auto power_off_callback() { return m_power_off_cb.bind(); }
	auto external_bus_ctrl_callback() { return m_external_bus_ctrl_cb.bind(); }
	template <typename... T> void set_video_read_callback(T &&... args) { m_video.lookup()->set_video_read_callback(std::forward<T>(args)...); }
	template <typename... T> void set_vblank_callback(T &&... args) { m_vblank_cb.set(std::forward<T>(args)...); }

	xavix_device &cpu() { return *m_maincpu; }
	xavix_io_device &io() { return m_io ? *m_io : *m_io.lookup(); }
	xavix_adc_device &adc() { return m_adc ? *m_adc : *m_adc.lookup(); }
	xavix_anport_device &anport() { return m_anport ? *m_anport : *m_anport.lookup(); }
	u8 *mainram() { return &m_mainram[0]; }
	std::span<u8> backup_ram() { return { m_mainram.end() - BACKUP_RAM_SIZE, BACKUP_RAM_SIZE }; }
	u8 read_low_bus_bypass(u32 address);
	void reset_w(int state);
	void cpu_reset_w(int state) { m_maincpu->set_input_line(INPUT_LINE_RESET, state); }
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	rectangle picture_area() const { return m_video->picture_area(); }

	void xavix_lowbus_map(address_map &map) ATTR_COLD;

protected:
	enum class generation { XAVIX, XAVIX2000, SUPERXAVIX };
	xavix_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, generation model);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<xavix_device> m_maincpu;
	required_device<xavix_intc_device> m_intc;
	required_device<xavix_io_device> m_io;
	required_device<xavix_adc_device> m_adc;
	required_device<xavix_anport_device> m_anport;
	required_device<xavix_math_device> m_math;
	required_device<xavix_sound_device> m_sound;
	required_device<xavix_timer_device> m_timer;
	required_device<xavix_dma_device> m_dma;
	required_device<xavix_video_device> m_video;

private:
	static constexpr unsigned BACKUP_RAM_SIZE = 0x1000;

	void configure_screen() ATTR_COLD;
	void xavix_map(address_map &map) ATTR_COLD;
	void xavix_extbus_map(address_map &map) ATTR_COLD;
	TIMER_CALLBACK_MEMBER(reset_changed);
	INTERRUPT_GEN_MEMBER(vblank);
	u8 lightgun_r(offs_t offset);
	u8 lightgun2_r(offs_t offset);
	u8 rom_access_ctrl_r();
	void rom_access_ctrl_w(u8 data);
	u8 ram_access_ctrl_r();
	void ram_access_ctrl_w(u8 data);
	u8 external_map_ctrl_r();
	void external_map_ctrl_w(u8 data);
	void refresh_external_bus_ctrl();
	void internal_arbiter_select_w(offs_t offset, u8 data);
	u8 internal_arbiter_ctrl_r();
	void internal_arbiter_ctrl_w(u8 data);
	u8 external_arbiter_ctrl_r();
	void external_arbiter_ctrl_w(u8 data);
	void sleep_ctrl_w(u8 data);
	u8 power_status_r();
	void power_ctrl_w(u8 data);
	u8 version_r();
	void watchdog_reset_w(u8 data);
	void service_watchdog();
	void synchronize_watchdog();
	void schedule_watchdog();
	TIMER_CALLBACK_MEMBER(watchdog_expired);

	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_mainram;
	devcb_read8 m_lightgun_read_cb;
	devcb_read8 m_power_read_cb;
	devcb_write_line m_power_off_cb;
	devcb_write8 m_external_bus_ctrl_cb;
	device_delegate<void ()> m_vblank_cb;
	std::string m_screen_tag;
	device_t *m_program_map_device;
	device_t *m_external_map_device;
	address_map_constructor m_program_map;
	address_map_constructor m_external_map;
	generation const m_generation;

	u8 m_rom_access_ctrl;
	u8 m_ram_access_ctrl;
	u8 m_external_map_ctrl;
	std::array<u8, 16> m_internal_arbiter_select;
	u8 m_internal_arbiter_ctrl;
	u8 m_external_arbiter_ctrl;
	u8 m_power_ctrl;
	bool m_watchdog_port_mode;
	u32 m_watchdog_count;
	attotime m_watchdog_time;
	attotime m_watchdog_last_expiry;
	emu_timer *m_watchdog_timer;
	memory_passthrough_handler m_watchdog_tap;
};

class xavix2000_soc_device : public xavix_soc_device
{
public:
	xavix2000_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	xavix2000_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, generation model);
};

class superxavix_soc_device : public xavix2000_soc_device
{
public:
	superxavix_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	superxavix_pio_device &pio() { return m_pio ? *m_pio : *m_pio.lookup(); }
	superxavix_plotter_device &plotter() { return m_plotter ? *m_plotter : *m_plotter.lookup(); }
	template <typename... T> void set_graphics_read_callback(T &&... args)
	{ downcast<superxavix_video_device &>(*m_video.lookup()).set_graphics_read_callback(std::forward<T>(args)...); }
	template <typename... T> void set_bitmap_read_callback(T &&... args)
	{ downcast<superxavix_video_device &>(*m_video.lookup()).set_bitmap_read_callback(std::forward<T>(args)...); }
	void decode_cartridge_graphics(memory_region &region)
	{ downcast<superxavix_video_device &>(*m_video).decode_cartridge_graphics(region); }
	void superxavix_lowbus_map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 cpu_memory_control_r(offs_t offset);
	u8 instruction_burst_ctrl_r();
	void instruction_burst_ctrl_w(u8 data);
	template <unsigned Region> u8 external_legacy_access_ctrl_r();
	template <unsigned Region> void external_legacy_access_ctrl_w(u8 data);
	template <unsigned Region> u8 external_region_access_ctrl_r();
	template <unsigned Region> void external_region_access_ctrl_w(u8 data);
	void output_clock_w(offs_t offset, u8 data);

	required_device<superxavix_pio_device> m_pio;
	required_device<superxavix_plotter_device> m_plotter;
	std::array<u8, 3> m_external_region_access_ctrl;
	u8 m_instruction_burst_ctrl;
};

DECLARE_DEVICE_TYPE(XAVIX_SOC, xavix_soc_device)
DECLARE_DEVICE_TYPE(XAVIX2000_SOC, xavix2000_soc_device)
DECLARE_DEVICE_TYPE(SUPERXAVIX_SOC, superxavix_soc_device)

#endif // MAME_MACHINE_XAVIX_SOC_H
