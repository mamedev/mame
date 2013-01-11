#pragma once

#ifndef __TIKI100__
#define __TIKI100__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "formats/tiki100_dsk.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80pio.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define Z80DART_TAG     "z80dart"
#define Z80PIO_TAG      "z80pio"
#define Z80CTC_TAG      "z80ctc"
#define FD1797_TAG      "fd1797"
#define AY8912_TAG      "ay8912"

#define TIKI100_VIDEORAM_SIZE   0x8000
#define TIKI100_VIDEORAM_MASK   0x7fff

#define BANK_ROM        0
#define BANK_RAM        1
#define BANK_VIDEO_RAM  2

class tiki100_state : public driver_device
{
public:
	tiki100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_ctc(*this, Z80CTC_TAG),
			m_fdc(*this, FD1797_TAG),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, FD1797_TAG":0"),
			m_floppy1(*this, FD1797_TAG":1"),
			m_video_ram(*this, "video_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<fd1797_t> m_fdc;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	optional_shared_ptr<UINT8> m_video_ram;

	virtual void machine_start();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( gfxram_r );
	DECLARE_WRITE8_MEMBER( gfxram_w );
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_WRITE8_MEMBER( keyboard_w );
	DECLARE_WRITE8_MEMBER( video_mode_w );
	DECLARE_WRITE8_MEMBER( palette_w );
	DECLARE_WRITE8_MEMBER( system_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE8_MEMBER( video_scroll_w );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void bankswitch();

	/* memory state */
	int m_rome;
	int m_vire;

	/* video state */
	UINT8 m_scroll;
	UINT8 m_mode;
	UINT8 m_palette;

	/* keyboard state */
	int m_keylatch;
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
};

#endif
