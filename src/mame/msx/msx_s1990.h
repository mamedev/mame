// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX_S1990_H
#define MAME_MSX_MSX_S1990_H

#pragma once

#include "cpu/z80/r800.h"
#include "cpu/z80/z80.h"

DECLARE_DEVICE_TYPE(MSX_S1990, msx_s1990_device)

class msx_s1990_device : public device_t,
	public device_memory_interface
{
public:
	msx_s1990_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::TIMING; }

	// configuration
	template <typename T> void set_z80_tag(T &&tag) { m_z80.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_r800_tag(T &&tag) { m_r800.set_tag(std::forward<T>(tag)); }
	auto firmware_switch_callback() { return m_firmware_switch_cb.bind(); }
	auto pause_led_callback() { return m_pause_led_cb.bind(); }
	auto r800_led_callback() { return m_r800_led_cb.bind(); }
	auto dac_write_callback() { return m_dac_write_cb.bind(); }
	auto sample_hold_callback() { return m_sample_hold_cb.bind(); }
	auto select_callback() { return m_select_cb.bind(); }
	auto filter_callback() { return m_filter_cb.bind(); }
	auto muting_callback() { return m_muting_cb.bind(); }
	auto comp_callback() { return m_comp_cb.bind(); }

	void pause_w(u8 data);

	void reg_index_write(u8 data);
	u8 regs_read();
	void regs_write(u8 data);

	void mem_write(offs_t offset, u8 data);
	u8 mem_read(offs_t offset);
	void io_write(offs_t offset, u8 data);
	u8 io_read(offs_t offset);

	void counter_write(u8 data);
	u8 counter_read(offs_t offset);

	void pmdac(u8 data);
	u8 pmcnt();
	void pmcntl(u8 data);
	u8 pmstat();

	DECLARE_INPUT_CHANGED_MEMBER(pause_callback);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;
	virtual u32 translate_memory_address(u16 address) { return address; }

private:
	TIMER_CALLBACK_MEMBER(dac_write);

	static constexpr u32 PCM_FREQUENCY = 15750;
	const address_space_config m_program_config;
	const address_space_config m_io_config;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;
	devcb_read_line m_firmware_switch_cb;
	devcb_write_line m_pause_led_cb;
	devcb_write_line m_r800_led_cb;
	devcb_write8 m_dac_write_cb;
	devcb_write_line m_sample_hold_cb;
	devcb_write_line m_select_cb;
	devcb_write_line m_filter_cb;
	devcb_write_line m_muting_cb;
	devcb_read_line m_comp_cb;
	required_device<z80_device> m_z80;
	required_device<r800_device> m_r800;

	u8 m_regs[16];
	u8 m_reg_index;
	attotime m_last_counter_reset;
	u64 m_last_pcm_write_ticks;
	u8 m_pcm_control;
	u8 m_pcm_data;
	emu_timer *m_dac_timer;
	bool m_z80_halt_enabled;
};

#endif // MAME_MSX_MSX_S1990_H
