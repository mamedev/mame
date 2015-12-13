// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef __XEROX820__
#define __XEROX820__

#include "emu.h"
#include "bus/scsi/sa1403d.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "cpu/i86/i86.h"
#include "machine/com8116.h"
#include "machine/ram.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "machine/wd_fdc.h"
#include "machine/x820kb.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "sound/speaker.h"
#include "sound/beep.h"

#define SCREEN_TAG      "screen"

#define Z80_TAG         "u46"
#define Z80PIO_KB_TAG   "u105"
#define Z80PIO_GP_TAG   "u101"
#define Z80PIO_RD_TAG   "u8"
#define Z80SIO_TAG      "u96"
#define Z80CTC_TAG      "u99"
#define FD1771_TAG      "u109"
#define FD1797_TAG      "u109"
#define COM8116_TAG     "u76"
#define I8086_TAG       "i8086"
#define SASIBUS_TAG     "sasi"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define KEYBOARD_TAG    "kb"

#define XEROX820_VIDEORAM_SIZE  0x1000
#define XEROX820_VIDEORAM_MASK  0x0fff

class xerox820_state : public driver_device
{
public:
	xerox820_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_kbpio(*this, Z80PIO_KB_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_sio(*this, Z80SIO_TAG),
		m_fdc(*this, FD1771_TAG),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette"),
		m_floppy0(*this, FD1771_TAG":0"),
		m_floppy1(*this, FD1771_TAG":1"),
		m_kb(*this, KEYBOARD_TAG),
		m_rom(*this, Z80_TAG),
		m_char_rom(*this, "chargen"),
		m_video_ram(*this, "video_ram"),
		m_fdc_irq(0),
		m_fdc_drq(0),
		m_8n5(0),
		m_400_460(0)
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_kbpio;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio0_device> m_sio;
	required_device<wd_fdc_t> m_fdc;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<xerox_820_keyboard_t> m_kb;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<UINT8> m_video_ram;

	DECLARE_READ8_MEMBER( fdc_r );
	DECLARE_WRITE8_MEMBER( fdc_w );
	DECLARE_WRITE8_MEMBER( scroll_w );
	//DECLARE_WRITE8_MEMBER( x120_system_w );
	DECLARE_READ8_MEMBER( kbpio_pa_r );
	DECLARE_WRITE8_MEMBER( kbpio_pa_w );
	DECLARE_READ8_MEMBER( kbpio_pb_r );
	DECLARE_WRITE_LINE_MEMBER( intrq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );
	DECLARE_WRITE_LINE_MEMBER( fr_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

	virtual void bankswitch(int bank);
	void update_nmi();

	/* video state */
	UINT8 m_scroll;                     /* vertical scroll */
	UINT8 m_framecnt;
	int m_ncset2;                       /* national character set */
	int m_vatt;                         /* X120 video attribute */
	int m_lowlite;                      /* low light attribute */
	int m_chrom;                        /* character ROM index */

	/* floppy state */
	bool m_fdc_irq;                     /* interrupt request */
	bool m_fdc_drq;                     /* data request */
	int m_8n5;                          /* 5.25" / 8" drive select */
	int m_400_460;                      /* double sided disk detect */

	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
};

class bigboard_state : public xerox820_state
{
public:
	bigboard_state(const machine_config &mconfig, device_type type, const char *tag)
		: xerox820_state(mconfig, type, tag),
			m_beeper(*this, "beeper")
	{ }

	required_device<beep_device> m_beeper;

	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER( kbpio_pa_w );

	bool m_bit5;

	TIMER_CALLBACK_MEMBER(bigboard_beepoff);
};

class xerox820ii_state : public xerox820_state
{
public:
	xerox820ii_state(const machine_config &mconfig, device_type type, const char *tag) :
		xerox820_state(mconfig, type, tag),
		m_speaker(*this, "speaker"),
		m_sasibus(*this, SASIBUS_TAG)
	{
	}

	required_device<speaker_sound_device> m_speaker;
	required_device<SCSI_PORT_DEVICE> m_sasibus;

	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER( bell_w );
	DECLARE_WRITE8_MEMBER( slden_w );
	DECLARE_WRITE8_MEMBER( chrom_w );
	DECLARE_WRITE8_MEMBER( lowlite_w );
	DECLARE_WRITE8_MEMBER( sync_w );

	DECLARE_WRITE8_MEMBER( rdpio_pb_w );
	DECLARE_WRITE_LINE_MEMBER( rdpio_pardy_w );

	void bankswitch(int bank) override;
};

#endif
