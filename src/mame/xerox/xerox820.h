// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_XEROX_XEROX820_H
#define MAME_XEROX_XEROX820_H

#pragma once

#include "bus/scsi/sa1403d.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "cpu/i86/i86.h"
#include "machine/com8116.h"
#include "machine/ram.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "machine/buffer.h"
#include "machine/output_latch.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "x820kb.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "sound/beep.h"
#include "machine/timer.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "emupal.h"

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
		m_view(*this, "view"),
		m_fdc_irq(0),
		m_fdc_drq(0),
		m_8n5(0),
		m_400_460(0)
	{ }

	void mk83(machine_config &config);
	void xerox820(machine_config &config);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	void scroll_w(offs_t offset, uint8_t data);
	//void x120_system_w(uint8_t data);
	uint8_t kbpio_pa_r();
	void kbpio_pa_w(uint8_t data);
	uint8_t kbpio_pb_r();
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	void cpu_halt_w(int state);

protected:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);


	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);

	void mk83_mem(address_map &map) ATTR_COLD;
	void xerox820_io(address_map &map) ATTR_COLD;
	void xerox820_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_kbpio;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<wd_fdc_device_base> m_fdc;
	optional_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<xerox_820_keyboard_device> m_kb;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	memory_view m_view;

	void update_nmi();

	/* video state */
	uint8_t m_scroll = 0;                     /* vertical scroll */
	uint8_t m_framecnt = 0;
	int m_ncset2 = 0;                       /* national character set */
	int m_vatt = 0;                         /* X120 video attribute */
	int m_lowlite = 0;                      /* low light attribute */
	int m_chrom = 0;                        /* character ROM index */

	/* floppy state */
	bool m_fdc_irq = false;                     /* interrupt request */
	bool m_fdc_drq = false;                     /* data request */
	int m_8n5 = 0;                          /* 5.25" / 8" drive select */
	int m_400_460 = 0;                      /* double sided disk detect */
	uint8_t m_fdc_xor = 0xff;               /* FDC data-bus inversion (0xff = first-gen FD1771; 0x00 = 820-II FD1797) */
	bool m_cpu_halted = false;              /* Z80 /HALT, tracked via halt_cb to gate the FDC /NMI */
};

class bigboard_state : public xerox820_state
{
public:
	bigboard_state(const machine_config &mconfig, device_type type, const char *tag)
		: xerox820_state(mconfig, type, tag)
		, m_beeper(*this, "beeper")
		, m_beep_timer(*this, "beep_timer")
	{ }

	void kbpio_pa_w(uint8_t data);

	void bigboard(machine_config &config);
protected:
	virtual void machine_reset() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(beep_timer);

	required_device<beep_device> m_beeper;
	required_device<timer_device> m_beep_timer;

	bool m_bit5 = false;
};

class xerox820ii_state : public xerox820_state
{
public:
	xerox820ii_state(const machine_config &mconfig, device_type type, const char *tag) :
		xerox820_state(mconfig, type, tag),
		m_speaker(*this, "speaker"),
		m_sasibus(*this, SASIBUS_TAG),
		m_sasi_hd(*this, SASIBUS_TAG ":1:harddisk")
	{
	}

	uint8_t kbpio_pa_r(); // adds the 820-II disk-board personality bits to the base PA read

	void bell_w(offs_t offset, uint8_t data);
	void slden_w(offs_t offset, uint8_t data);
	void chrom_w(offs_t offset, uint8_t data);
	void lowlite_w(uint8_t data);
	void sync_w(offs_t offset, uint8_t data);

	void rdpio_pb_w(uint8_t data);
	uint8_t rdpio_pb_r();
	void rdpio_pardy_w(int state);
	void sasi_data_w(uint8_t data); // port-A out -> SASI data bus + assert ACK (U11 74LS74 set via PARDY)
	void sasi_req_w(int state);     // SASI REQ -> status bit3 + clears ACK FF (handshake)
	void sasi_io_w(int state);      // SASI I/O -> status bit4 + releases the host data latch when the target drives (input)

	// The 820-II disk daughterboard (9R80758) came in three personalities, modelled
	// as separate machines that share xerox820ii_common(): a main-board FD1797 floppy
	// controller with either 5.25" or 8" drives, or a Shugart SASI host adapter
	// (SA1403D) carrying the 8" floppies + an ST-506 rigid disk.  The 16/8 (8086 add-in
	// board) layers on top of any of the three.
	void xerox820ii_common(machine_config &config); // shared 820-II hardware (no disk drives)
	void xerox820ii_sasi(machine_config &config);    // add the SA1403D SASI host adapter
	void xerox820ii(machine_config &config);   // 8" floppy
	void xerox820ii5(machine_config &config);  // 5.25" floppy
	void xerox820iis(machine_config &config);  // 8" floppy + SASI rigid disk
	void xerox168(machine_config &config);     // 16/8, 8" floppy
	void xerox1685(machine_config &config);    // 16/8, 5.25" floppy
	void xerox168s(machine_config &config);    // 16/8, 8" floppy + SASI rigid disk
	void xerox168_mem(address_map &map) ATTR_COLD;
	void xerox820ii_io(address_map &map) ATTR_COLD;  // FD1797 floppy controller at 0x10-0x13
	void xerox820iis_io(address_map &map) ATTR_COLD; // SASI host-adapter PIO at 0x10-0x13
	void xerox820ii_mem(address_map &map) ATTR_COLD;
protected:
	virtual void machine_reset() override ATTR_COLD;

	void add_8086(machine_config &config); // the 16/8 add-in 8086 board

	required_device<speaker_sound_device> m_speaker;
	optional_device<scsi_port_device> m_sasibus;        // SASI personality only
	optional_device<sa1403d_device> m_sasi_hd;          // SA1403D controller (serves the floppies + rigid disk over SASI)
	optional_device<input_buffer_device> m_sasi_ctrl_in{*this, "sasi_ctrl_in"};
	optional_device<output_latch_device> m_sasi_data_out{*this, "sasi_data_out"};
	bool m_sasi_ackff = false; // U11 74LS74 Q -> SASI ACK (set by port-A write/PARDY, cleared by REQ)
	int m_sasi_board = 0;      // disk daughterboard: 0 = FD1797 floppy, 1 = SASI host adapter (set per machine)
};

#endif // MAME_XEROX_XEROX820_H
