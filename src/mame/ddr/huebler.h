// license:BSD-3-Clause
// copyright-holders:Curt Coder

#ifndef MAME_DDR_HUEBLER_H
#define MAME_DDR_HUEBLER_H

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define Z80CTC_TAG      "z80ctc"
#define Z80SIO_TAG      "z80sio"
#define Z80PIO1_TAG     "z80pio1"
#define Z80PIO2_TAG     "z80pio2"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/z80ctc.h"
#include "sound/spkrdev.h"
#include "emupal.h"

class amu880_state : public driver_device
{
public:
	amu880_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_cassette(*this, "cassette")
		, m_sio(*this, Z80SIO_TAG)
		, m_speaker(*this, "speaker")
		, m_palette(*this, "palette")
		, m_boot(*this, "boot")
		, m_rom(*this, Z80_TAG)
		, m_kb_rom(*this, "keyboard")
		, m_char_rom(*this, "chargen")
		, m_video_ram(*this, "video_ram")
		, m_key_row(*this, "Y%u", 0)
		, m_special(*this, "SPECIAL")
		, m_cass_config(*this, "CASSETTE")
		, m_shift_led(*this, "shift_led")
		, m_poweroff_led(*this, "poweroff_led")
		, m_key_d6(0)
		, m_key_d7(0)
	{ }

	void amu880(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(shift_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<z80sio_device> m_sio;
	required_device<speaker_sound_device> m_speaker;
	required_device<palette_device> m_palette;
	memory_view m_boot;
	required_region_ptr<uint8_t> m_rom;
	required_memory_region m_kb_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	required_ioport_array<16> m_key_row;
	required_ioport m_special;
	required_ioport m_cass_config;
	output_finder<> m_shift_led;
	output_finder<> m_poweroff_led;
	emu_timer *m_key_release_timer = nullptr;
	emu_timer *m_key_repeat_timer = nullptr;
	emu_timer *m_tone_timer = nullptr;
	emu_timer *m_poweroff_timer = nullptr;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t boot_r(offs_t offset);
	uint8_t keyboard_r(offs_t offset);
	uint8_t power_off_r();
	void power_off_w(uint8_t data);
	void power_off_clock(int rd);
	void tone_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(tone_tick);
	TIMER_CALLBACK_MEMBER(power_off);

	void scan_keyboard();
	TIMER_CALLBACK_MEMBER(key_release);
	TIMER_CALLBACK_MEMBER(key_repeat);

	// keyboard state
	int m_key_d6;
	int m_key_d7;
	int m_key_a4 = 0;
	int m_key_a5 = 0;
	bool m_key_down = false;
	bool m_key_repeat = true;
	bool m_shift_lock = false;

	// I/O card state
	uint8_t m_poweroff_sr = 0;
	bool m_tone_on = false;
	bool m_tone_q = true;

	// video state
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);
	void ctc_z0_w(int state);
	void ctc_z2_w(int state);
	void cassette_w(int state);
	void update_cassette();
	void amu880_io(address_map &map) ATTR_COLD;
	void amu880_mem(address_map &map) ATTR_COLD;

	// cassette variables
	bool m_cass_t = false;
	u8 m_tx_count = 0;
	bool m_txc = false;
	bool m_cassbit = true;
	bool m_rx_d = false;
	bool m_rx_mono = false;
	u8 m_rx_count = 0;
};

#endif // MAME_DDR_HUEBLER_H
