#pragma once

#ifndef __TRS80M2__
#define __TRS80M2__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/flopdrv.h"
#include "machine/ctronics.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/trs80m2kb.h"
#include "machine/wd17xx.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "video/mc6845.h"

#define SCREEN_TAG		"screen"
#define Z80_TAG			"u12"
#define Z80CTC_TAG		"u19"
#define Z80DMA_TAG		"u20"
#define Z80PIO_TAG		"u22"
#define Z80SIO_TAG		"u18"
#define FD1791_TAG		"u6"
#define MC6845_TAG		"u11"
#define CENTRONICS_TAG	"j2"
#define M68000_TAG		"m16_u22"
#define AM9519A_TAG		"m16_u11"

class trs80m2_state : public driver_device
{
public:
	trs80m2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_ctc(*this, Z80CTC_TAG),
		  m_pio(*this, Z80PIO_TAG),
		  m_crtc(*this, MC6845_TAG),
		  m_fdc(*this, FD1791_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_floppy(*this, FLOPPY_0),
		  m_ram(*this, RAM_TAG),
		  m_kb(*this, TRS80M2_KEYBOARD_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device>m_ctc;
	required_device<z80pio_device> m_pio;
	required_device<mc6845_device> m_crtc;
	required_device<fd1791_device> m_fdc;
	required_device<centronics_device> m_centronics;
	required_device<legacy_floppy_image_device> m_floppy;
	required_device<ram_device> m_ram;
	required_device<trs80m2_keyboard_device> m_kb;

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( drvslt_w );
	DECLARE_WRITE8_MEMBER( rom_enable_w );
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_READ8_MEMBER( rtc_r );
	DECLARE_READ8_MEMBER( nmi_r );
	DECLARE_WRITE8_MEMBER( nmi_w );
	DECLARE_READ8_MEMBER( keyboard_busy_r );
	DECLARE_READ8_MEMBER( keyboard_data_r );
	DECLARE_WRITE8_MEMBER( keyboard_ctrl_w );
	DECLARE_WRITE8_MEMBER( keyboard_latch_w );
	DECLARE_READ8_MEMBER( fdc_r );
	DECLARE_WRITE8_MEMBER( fdc_w );
	DECLARE_WRITE_LINE_MEMBER( de_w );
	DECLARE_WRITE_LINE_MEMBER( vsync_w );
	DECLARE_READ8_MEMBER( pio_pa_r );
	DECLARE_WRITE8_MEMBER( pio_pa_w );
	DECLARE_WRITE_LINE_MEMBER( strobe_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( kb_clock_w );

	void scan_keyboard();

	// memory state
	int m_boot_rom;
	int m_bank;
	int m_msel;

	// keyboard state
	UINT8 m_key_latch;
	UINT8 m_key_data;
	int m_key_bit;
	int m_kbclk;
	int m_kbdata;
	int m_kbirq;

	// video state
	UINT8 *m_video_ram;
	UINT8 *m_char_rom;
	int m_blnkvid;
	int m_80_40_char_en;
	int m_de;
	int m_rtc_int;
	int m_enable_rtc_int;
	TIMER_DEVICE_CALLBACK_MEMBER(trs80m2_keyboard_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
};

class trs80m16_state : public trs80m2_state
{
public:
	trs80m16_state(const machine_config &mconfig, device_type type, const char *tag)
		: trs80m2_state(mconfig, type, tag),
		  m_subcpu(*this, M68000_TAG),
		  m_pic(*this, AM9519A_TAG)
	{ }

	required_device<cpu_device> m_subcpu;
	required_device<pic8259_device> m_pic;

	virtual void machine_start();

	DECLARE_WRITE8_MEMBER( ual_w );
	DECLARE_WRITE8_MEMBER( tcl_w );

	UINT16 m_ual;
	UINT8 m_limit[2];
	UINT8 m_offset[2];
};

#endif
