// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC9

******************************************************************************/

#ifndef MAME_MACHINE_PSION_ASIC9_H
#define MAME_MACHINE_PSION_ASIC9_H

#pragma once

#include "machine/ram.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_asic9_device

class psion_asic9_device : public device_t,
	public device_memory_interface,
	public device_video_interface
{
public:
	psion_asic9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T, typename U> void set_ram_rom(T &&ram_tag, U &&rom_tag) { m_ram.set_tag(std::forward<T>(ram_tag)); m_rom.set_tag(std::forward<U>(rom_tag)); }

	// callbacks
	auto buz_cb() { return m_buz_cb.bind(); }
	auto col_cb() { return m_col_cb.bind(); }
	auto port_ab_r() { return m_port_ab_r.bind(); }
	auto port_ab_w() { return m_port_ab_w.bind(); }
	auto pcm_in() { return m_pcm_in.bind(); }
	auto pcm_out() { return m_pcm_out.bind(); }

	template <unsigned N> auto data_r() { static_assert(N < 8); return m_data_r[N].bind(); }
	template <unsigned N> auto data_w() { static_assert(N < 8); return m_data_w[N].bind(); }

	address_space &io_space() const { return m_v30->space(AS_IO); }

	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint8_t col_r();

	IRQ_CALLBACK_MEMBER(inta_cb);

	void sds_int_w(int state);
	void eint0_w(int state);
	void eint1_w(int state);
	void eint2_w(int state);
	void medchng_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	psion_asic9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

private:
	required_device<cpu_device> m_v30;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	static constexpr int AS_A9_RAM = AS_OPCODES + 1;
	static constexpr int AS_A9_ROM = AS_OPCODES + 2;

	const address_space_config m_ram_config;
	const address_space_config m_rom_config;

	address_space *m_ram_space;
	address_space *m_rom_space;

	emu_timer *m_tick_timer;
	emu_timer *m_frc1_timer;
	emu_timer *m_frc2_timer;
	emu_timer *m_watchdog_timer;
	emu_timer *m_rtc_timer;
	emu_timer *m_snd_timer;

	TIMER_CALLBACK_MEMBER(tick);
	TIMER_CALLBACK_MEMBER(frc1);
	TIMER_CALLBACK_MEMBER(frc2);
	TIMER_CALLBACK_MEMBER(watchdog);
	TIMER_CALLBACK_MEMBER(rtc);
	TIMER_CALLBACK_MEMBER(snd);

	void update_interrupts();
	bool is_protected(offs_t offset);
	offs_t translate_address(offs_t offset);

	uint8_t m_ram_type;
	uint32_t ram_device_size(uint8_t device_type);
	void configure_ram(uint8_t device_type = 0);
	void configure_rom();

	uint8_t m_post;
	uint16_t m_a9_control;
	uint16_t m_a9_status;
	uint16_t m_a9_lcd_size;
	uint8_t m_a9_interrupt_status;
	uint8_t m_a9_interrupt_mask;
	uint16_t m_frc1_count;
	uint16_t m_frc1_reload;
	uint16_t m_frc2_count;
	uint16_t m_frc2_reload;
	int m_buz_toggle;
	uint8_t m_watchdog_count;
	bool m_a9_protection_mode;
	uint32_t m_a9_protection_upper;
	uint32_t m_a9_protection_lower;
	uint16_t m_a9_port_ab_ddr;
	uint8_t m_a9_port_c_ddr;
	uint8_t m_a9_port_d_ddr;
	uint8_t m_a9_psel_6000;
	uint8_t m_a9_psel_7000;
	uint8_t m_a9_psel_8000;
	uint8_t m_a9_psel_9000;
	uint16_t m_a9_control_extra;
	uint32_t m_rtc;
	util::fifo<uint8_t, 16> m_snd_fifo;

	uint8_t m_a9_serial_data;
	uint8_t m_a9_serial_control;
	uint8_t m_a9_channel_select;

	devcb_write_line m_buz_cb;
	devcb_write8 m_col_cb;
	devcb_read16 m_port_ab_r;
	devcb_write16 m_port_ab_w;
	devcb_read8 m_pcm_in;
	devcb_write8 m_pcm_out;

	devcb_read8::array<8> m_data_r;
	devcb_write16::array<8> m_data_w;

	emu_timer *m_busy_timer;

	TIMER_CALLBACK_MEMBER(busy);

	bool channel_active(int channel);
	void transmit_frame(uint16_t data);
	uint8_t receive_frame();

	static constexpr uint16_t NULL_FRAME    = 0x000;
	static constexpr uint16_t CONTROL_FRAME = 0x100;
	static constexpr uint16_t DATA_FRAME    = 0x200;
};


// ======================> psion_asic9mx_device

class psion_asic9mx_device : public psion_asic9_device
{
public:
	// construction/destruction
	psion_asic9mx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_ASIC9, psion_asic9_device)
DECLARE_DEVICE_TYPE(PSION_ASIC9MX, psion_asic9mx_device)

#endif // MAME_MACHINE_PSION_ASIC9_H
