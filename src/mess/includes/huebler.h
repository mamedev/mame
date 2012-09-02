#ifndef __HUEBLER__
#define __HUEBLER__

#define SCREEN_TAG		"screen"
#define Z80_TAG			"z80"
#define Z80CTC_TAG		"z80ctc"
#define Z80SIO_TAG		"z80sio"
#define Z80PIO1_TAG		"z80pio1"
#define Z80PIO2_TAG		"z80pio2"

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
		  m_cassette(*this, CASSETTE_TAG),
		  m_key_d6(0),
		  m_key_d7(0),
		  m_key_a8(1),
		  m_video_ram(*this, "video_ram")
	{ }

	required_device<cassette_image_device> m_cassette;

	virtual void machine_start();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( keyboard_r );

	void scan_keyboard();

	// keyboard state
	const UINT8 *m_kb_rom;
	int m_key_d6;
	int m_key_d7;
	int m_key_a4;
	int m_key_a5;
	int m_key_a8;

	// video state
	required_shared_ptr<UINT8> m_video_ram;
	const UINT8 *m_char_rom;
};

#endif
