// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef __HUEBLER__
#define __HUEBLER__

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define Z80CTC_TAG      "z80ctc"
#define Z80SIO_TAG      "z80sio"
#define Z80PIO1_TAG     "z80pio1"
#define Z80PIO2_TAG     "z80pio2"

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"

class amu880_state : public driver_device
{
public:
	amu880_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_cassette(*this, "cassette"),
			m_z80sio(*this, Z80SIO_TAG),
			m_palette(*this, "palette"),
			m_kb_rom(*this, "keyboard"),
			m_char_rom(*this, "chargen"),
			m_video_ram(*this, "video_ram"),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_y8(*this, "Y8"),
			m_y9(*this, "Y9"),
			m_y10(*this, "Y10"),
			m_y11(*this, "Y11"),
			m_y12(*this, "Y12"),
			m_y13(*this, "Y13"),
			m_y14(*this, "Y14"),
			m_y15(*this, "Y15"),
			m_special(*this, "SPECIAL"),
			m_key_d6(0),
			m_key_d7(0),
			m_key_a8(1)
	{ }

	required_device<cassette_image_device> m_cassette;
	required_device<z80dart_device> m_z80sio;
	required_device<palette_device> m_palette;
	required_memory_region m_kb_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<UINT8> m_video_ram;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_y10;
	required_ioport m_y11;
	required_ioport m_y12;
	required_ioport m_y13;
	required_ioport m_y14;
	required_ioport m_y15;
	required_ioport m_special;

	virtual void machine_start();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( keyboard_r );
	TIMER_DEVICE_CALLBACK_MEMBER( tape_tick );

	void scan_keyboard();

	// keyboard state
	ioport_port* m_key_row[16];
	int m_key_d6;
	int m_key_d7;
	int m_key_a4;
	int m_key_a5;
	int m_key_a8;

	// video state
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	DECLARE_READ_LINE_MEMBER(cassette_r);
	DECLARE_WRITE_LINE_MEMBER(cassette_w);
};

#endif
