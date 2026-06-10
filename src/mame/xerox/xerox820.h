// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_XEROX_XEROX820_H
#define MAME_XEROX_XEROX820_H

#pragma once

#include "bus/nscsi/sa1403d.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "cpu/i86/i86.h"
#include "machine/com8116.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/buffer.h"
#include "machine/output_latch.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "x820kb.h"
#include "xerox_lpk.h"
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

// ============================================================================
//  x820_sasi_host_device - the 820-II SASI host adapter (9R80758 daughterboard)
//
//  The Z80 reaches the SASI bus through the "u8" Z80PIO: port A carries the
//  data bus (PARDY clocks the U11 74LS74, generating the per-byte ACK pulse
//  on either a read or a write), port B carries the control lines (in:
//  bit0 BSY, bit1 MSG, bit2 C/D, bit3 REQ, bit4 I/O; out: bit5 SEL,
//  bit7 RST).  This device is the line-level bridge between those PIO
//  callbacks and the nscsi bus (the bigbord2 SASI host pattern).
// ============================================================================

class x820_sasi_host_device : public device_t, public nscsi_device_interface
{
public:
	x820_sasi_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t data_r();           // PIO port A in: bus data, ACK pulse (U11 via PARDY)
	void data_w(uint8_t data);  // PIO port A out: drive bus data, ACK pulse (U11 via PARDY)
	uint8_t ctrl_r();           // PIO port B in: phase lines
	void ctrl_w(uint8_t data);  // PIO port B out: SEL (bit 5), RST (bit 7)

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void scsi_ctrl_changed() override;

private:
	TIMER_CALLBACK_MEMBER(ack_off);

	static constexpr attotime SASI_PULSE = attotime::from_nsec(500);

	emu_timer *m_ack_timer = nullptr;
};

DECLARE_DEVICE_TYPE(X820_SASI_HOST, x820_sasi_host_device)


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
		m_lpk(*this, "lpk"),
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
	optional_device<wd_fdc_device_base> m_fdc;          // absent on the SASI personality (the FD1797 add-in board is replaced by the SASI host adapter)
	optional_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	optional_device<floppy_connector> m_floppy0;        // absent on the SASI personality
	optional_device<floppy_connector> m_floppy1;
	optional_device<xerox_820_keyboard_device> m_kb;   // standard ASCII keyboard (absent on 16/8)
	optional_device<xerox_lpk_device> m_lpk;           // Low Profile Keyboard (16/8 only); shares the "kb" tag
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	memory_view m_view;


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
	void update_nmi();
	bool m_cpu_halted = false;              /* Z80 /HALT, tracked via halt_cb to gate the FDC /NMI */
	bool m_drvsel_binary = false;           /* drive select: false = Xerox 820 bit-per-drive; true = Big Board binary unit# */
	bool m_fdc_single_floppy = false;       /* x1685: the RX024 expansion-box 5.25" drive is permanently cabled, so keep floppy0 attached to the FD1797 even when the system PIO selects no drive */
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
	void bigboard5(machine_config &config);
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
		m_i8086(*this, I8086_TAG),
		m_sasibus(*this, SASIBUS_TAG),
		m_sasi_host(*this, "sasi_host"),
		m_sasi_hd(*this, SASIBUS_TAG ":0:sa1403d")
	{
	}

	uint8_t kbpio_pa_r(); // adds the 820-II disk-board personality bits to the base PA read

	void bell_w(offs_t offset, uint8_t data);
	void slden_w(offs_t offset, uint8_t data);
	void chrom_w(offs_t offset, uint8_t data);
	void lowlite_w(uint8_t data);
	void sync_w(offs_t offset, uint8_t data);

	void dbell_w(u8 data);                 // 16/8 OUT A1: Z80->8086 doorbell (counted latch)
	IRQ_CALLBACK_MEMBER(i8086_dbell_inta); // 8086 INTA: floating-bus vector 0xFF, pops one ring

	// 16/8 shared-RAM window (Z80 0x4000-0xBFFF in the banked-ROM view -> 8086
	// 0xF8000+offset) with the bus arbiter that makes the Z80's unlocked
	// read-modify-write lock operations atomic against the 8086 (see shared_ram_r)
	u8 shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, u8 data);


	// The 820-II disk daughterboard (9R80758) came in three personalities, modelled
	// as separate machines that share xerox820ii_common(): a main-board FD1797 floppy
	// controller with either 5.25" or 8" drives, or a Shugart SASI host adapter
	// (SA1403D) carrying the 8" floppies + an ST-506 rigid disk.  The 16/8 (8086 add-in
	// board) layers on top of any of the three.
	void xerox820ii_common(machine_config &config); // shared 820-II hardware (no disk drives)
	void xerox820ii_sasi(machine_config &config, bool rgd5 = false); // add the SA1403D SASI host adapter (rgd5 = the 5.25" drive complement)
	void xerox820ii(machine_config &config);   // 8" floppy
	void xerox820ii5(machine_config &config);  // 5.25" floppy
	void xerox820iis(machine_config &config);  // 8" floppy + SASI rigid disk
	void xerox168(machine_config &config);     // 16/8, 8" floppy
	void xerox1685(machine_config &config);    // 16/8, 5.25" floppy
	void xerox168s(machine_config &config);    // 16/8, 8" floppy + SASI rigid disk
	void xerox1685s(machine_config &config);
	void xerox820iilp(machine_config &config);   // 16/8 low profile, 5.25" rigid disk unit (rgd5 box)
	void xerox168_mem(address_map &map) ATTR_COLD;
	void xerox820ii_io(address_map &map) ATTR_COLD;  // FD1797 floppy controller at 0x10-0x13
	void xerox820iis_io(address_map &map) ATTR_COLD; // SASI host-adapter PIO at 0x10-0x13
	void xerox1685_io(address_map &map) ATTR_COLD;   // FD1797 floppy map + the reconstructed RX024 controller ROM
	uint8_t rx024_rom_r(offs_t offset);              // RX024 port-mapped controller ROM: ddskld loads the WDVR floppy driver from it
	void xerox1685s_io(address_map &map) ATTR_COLD;  // SASI PIO + the rgd5 unit's controller ROM
	uint8_t rgd5_rom_r(offs_t offset);               // rgd5 port-mapped controller ROM: ddskld loads the SDVR SASI driver from it
	void rx024_select_w(uint8_t data);               // RX024 box select latch: raw bus write routes drive/side (the 16/8 PIO keeps D0-D2 as inputs)
	void xerox820ii_mem(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void add_8086(machine_config &config); // the 16/8 add-in 8086 board

	uint16_t m_dbell_count = 0; // outstanding (un-acknowledged) Z80->8086 doorbell rings

	// Z80 shared-RAM RMW reservation (see shared_ram_r/w)
	offs_t m_rmw_addr = ~offs_t(0); // window offset of the last Z80 read (~0 = none)
	u8 m_rmw_val = 0;               // value that read returned
	u64 m_rmw_cycles = 0;           // Z80 total_cycles() at that read

	required_device<speaker_sound_device> m_speaker;
	optional_device<i8086_cpu_device> m_i8086;          // 16/8 add-in board (held in reset until modelled)
	optional_device<nscsi_bus_device> m_sasibus;        // SASI personality only
	optional_device<x820_sasi_host_device> m_sasi_host; // u8-PIO to SASI line-level bridge
	optional_device<nscsi_sa1403d_device> m_sasi_hd;    // SA1403D controller (serves the floppies + rigid disk over SASI)
	int m_sasi_board = 0;      // disk daughterboard: 0 = FD1797 floppy, 1 = SASI host adapter (set per machine)
};

#endif // MAME_XEROX_XEROX820_H
