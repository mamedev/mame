// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Nigel Barnes
/*****************************************************************************

    Acorn Electron ULA

 ****************************************************************************/

#ifndef MAME_ACORN_ELECTRON_ULA_H
#define MAME_ACORN_ELECTRON_ULA_H

#pragma once

#include "emupal.h"


class electron_ula_device : public device_t
	, public device_memory_interface
	, public device_palette_interface
	, public device_sound_interface
	, public device_video_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	electron_ula_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	typedef device_delegate<double ()> casin_delegate;
	typedef device_delegate<void (double)> casout_delegate;

	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	template <typename... T> void cas_in_cb(T &&... args) { m_cas_in_cb.set(std::forward<T>(args)...); }
	template <typename... T> void cas_out_cb(T &&... args) { m_cas_out_cb.set(std::forward<T>(args)...); }

	auto cas_mo_cb() { return m_cas_mo_cb.bind(); }
	auto irq_cb() { return m_irq_cb.bind(); }
	auto caps_lock_cb() { return m_caps_lock_cb.bind(); }
	auto kbd_cb() { return m_kbd_cb.bind(); }

	void set_rom(uint8_t *rom) { m_rom = rom; }
	void set_ram(uint8_t *ram) { m_ram = ram; }

	uint8_t read(offs_t offset) { return space(0).read_byte(offset); }
	void write(offs_t offset, uint8_t data) { space(0).write_byte(offset, data); }

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 16; }

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_space_config;
	required_device<cpu_device> m_maincpu;

	uint8_t *m_rom;
	uint8_t *m_ram;

	sound_stream *m_sound_stream;             // stream number
	bool m_sound_enable;                      // enable beep
	uint32_t m_sound_freq;                    // set frequency
	int32_t m_sound_incr;                     // initial wave state
	sound_stream::sample_t m_sound_signal;   // current signal

	casin_delegate m_cas_in_cb;
	casout_delegate m_cas_out_cb;
	devcb_write_line m_cas_mo_cb;
	devcb_write_line m_irq_cb;
	devcb_write_line m_caps_lock_cb;
	devcb_read8 m_kbd_cb;

	emu_timer *m_tape_timer = nullptr;
	emu_timer *m_scanline_timer = nullptr;

	TIMER_CALLBACK_MEMBER(tape_timer_handler);
	TIMER_CALLBACK_MEMBER(scanline_interrupt);

	uint8_t rom_r(offs_t offset);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	inline uint8_t read_vram(uint16_t addr);
	void update_palette(offs_t offset);
	void update_interrupts();

	void set_sound_state(int state);              // enable/disable sound output
	void set_sound_frequency(uint32_t frequency); // output frequency
	void set_cpu_clock(offs_t offset);

	void waitforramsync();
	void tape_start();
	void tape_stop();

	// interrupts
	static constexpr uint8_t INT_POWER_ON_RESET = 0x02;
	static constexpr uint8_t INT_DISPLAY_END    = 0x04;
	static constexpr uint8_t INT_RTC            = 0x08;
	static constexpr uint8_t INT_RECEIVE_FULL   = 0x10;
	static constexpr uint8_t INT_TRANSMIT_EMPTY = 0x20;
	static constexpr uint8_t INT_HIGH_TONE      = 0x40;

	// registers
	uint8_t m_int_status = 0;
	uint8_t m_int_control = 0;
	uint8_t m_rompage = 0;
	uint16_t m_screen_start = 0;
	uint16_t m_screen_addr = 0;
	uint8_t m_screen_ra = 0;
	uint8_t m_comms_mode = 0;
	uint8_t m_disp_mode = 0;
	uint8_t m_palette[8] = { 0 };
	bool m_cass_motor = false;

	// screen mode properties
	const uint16_t mode_size[8] = { 0x5000, 0x5000, 0x5000, 0x4000, 0x2800, 0x2800, 0x2000, 0x2000 };
	const int mode_dispend[8]   = { 256, 256, 256 ,250 ,256, 256, 250, 250 };
	const int mode_max_ra[8]    = { 8, 8, 8 ,10 ,8, 8, 10, 10 };
	const int mode_bpp[8]       = { 1, 2, 4 ,1 ,1, 2, 1, 1 };

	// tape related
	uint32_t m_tape_value = 0;
	int m_tape_steps = 0;
	int m_bit_count = 0;
	int m_high_tone_set = 0;
	int m_start_bit = 0;
	int m_stop_bit = 0;
	bool m_tape_running = false;
	uint8_t m_tape_byte = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_ULA, electron_ula_device)

#endif // MAME_ACORN_ELECTRON_ULA_H
