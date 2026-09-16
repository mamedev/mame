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
#include "emupal.h"

class amu880_state : public driver_device
{
public:
	amu880_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_cassette(*this, "cassette")
		, m_sio(*this, Z80SIO_TAG)
		, m_palette(*this, "palette")
		, m_kb_rom(*this, "keyboard")
		, m_char_rom(*this, "chargen")
		, m_video_ram(*this, "video_ram")
		, m_key_row(*this, "Y%u", 0)
		, m_special(*this, "SPECIAL")
		, m_key_d6(0)
		, m_key_d7(0)
		, m_key_a8(1)
	{ }

	void amu880(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { m_maincpu->set_pc(0xf000); }

private:
	required_device<z80_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<z80sio_device> m_sio;
	required_device<palette_device> m_palette;
	required_memory_region m_kb_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	required_ioport_array<16> m_key_row;
	required_ioport m_special;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t keyboard_r(offs_t offset);

	void scan_keyboard();

	// keyboard state
	int m_key_d6;
	int m_key_d7;
	int m_key_a4 = 0;
	int m_key_a5 = 0;
	int m_key_a8;

	// video state
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);
	void ctc_z0_w(int state);
	void ctc_z2_w(int state);
	void cassette_w(int state);
	void amu880_io(address_map &map) ATTR_COLD;
	void amu880_mem(address_map &map) ATTR_COLD;

	// cassette variables
	u8 m_cnt = 0;
	bool m_cassbit = false;
};

#endif // MAME_DDR_HUEBLER_H
