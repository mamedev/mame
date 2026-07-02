// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_XEROX_XEROX820_H
#define MAME_XEROX_XEROX820_H

#pragma once

#include "bus/rs232/rs232.h"
#include "bus/xerox820/dbslot.h"
#include "bus/xerox820/copro.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/com8116.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "x820kb.h"
#include "xerox_lpk.h"
#include "sound/spkrdev.h"
#include "sound/beep.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "emupal.h"

#define SCREEN_TAG      "screen"

#define Z80_TAG         "u46"
#define Z80PIO_KB_TAG   "u105"
#define Z80PIO_GP_TAG   "u101"
#define Z80SIO_TAG      "u96"
#define Z80CTC_TAG      "u99"
#define FD1771_TAG      "u109"
#define COM8116_TAG     "u76"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define KEYBOARD_TAG    "kb"
#define DBSLOT_TAG      "dbslot"
#define COPRO_TAG       "copro"

#define XEROX820_VIDEORAM_SIZE  0x1000
#define XEROX820_VIDEORAM_MASK  0x0fff


// ======================> xerox820_base_state
//
//  Hardware common to every machine in the family: the Z80, the keyboard /
//  general-purpose / CTC / SIO peripherals, the COM8116 baud generator, the
//  video section and the banked-ROM memory view.  The disk controller (a
//  main-board FD1771 on the 820, a daughterboard slot on the 820-II) and the
//  16/8 coprocessor are added by the derived classes.

class xerox820_base_state : public driver_device
{
public:
	xerox820_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_kbpio(*this, Z80PIO_KB_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_sio(*this, Z80SIO_TAG),
		m_palette(*this, "palette"),
		m_kbd(*this, KEYBOARD_TAG),
		m_rom(*this, Z80_TAG),
		m_char_rom(*this, "chargen"),
		m_video_ram(*this, "video_ram"),
		m_view(*this, "view")
	{ }

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

protected:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);

	uint8_t kbpio_pa_r();
	virtual void kbpio_pa_w(uint8_t data); // Big Board overrides to add the beeper
	uint8_t kbpio_pb_r();
	void scroll_w(offs_t offset, uint8_t data);

	void io_common(address_map &map) ATTR_COLD; // the I/O ports shared by all machines (everything but 0x10-0x13)

	// FDC INTRQ/DRQ -> /HALT-gated /NMI (shared by the FD1771 main board and the
	// 820-II FDC daughterboard)
	void update_nmi();
	void cpu_halt_w(int state);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	// disk daughterboard personality hooks (driven by the system PIO port A)
	virtual void disk_drvsel_w(uint8_t data) { }    // PA bits 0-2: drive select / side
	virtual uint8_t disk_pa_bits() { return 0; }     // PA bits 0-1 (personality), 4-5 (media sense)

	virtual void machine_start() override ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_kbpio;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<palette_device> m_palette;
	required_device<xerox820_keyboard_device> m_kbd;   // the keyboard fitted to this machine (config-determined)
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	memory_view m_view;

	// video state
	uint8_t m_scroll = 0;       // vertical scroll
	uint8_t m_framecnt = 0;
	int m_ncset2 = 0;           // national character set
	int m_vatt = 0;             // X120 video attribute

	// FDC -> /NMI generator state
	bool m_fdc_irq = false;
	bool m_fdc_drq = false;
	bool m_cpu_halted = false;  // Z80 /HALT, tracked via halt_cb to gate the FDC /NMI
};


// ======================> xerox820_state
//
//  Xerox 820 / Big Board: the main-board FD1771 floppy controller (the
//  first-generation FD1771 data bus is inverted, m_fdc_xor).

class xerox820_state : public xerox820_base_state
{
public:
	xerox820_state(const machine_config &mconfig, device_type type, const char *tag) :
		xerox820_base_state(mconfig, type, tag),
		m_fdc(*this, FD1771_TAG),
		m_floppy0(*this, FD1771_TAG":0"),
		m_floppy1(*this, FD1771_TAG":1")
	{ }

	void mk83(machine_config &config) ATTR_COLD;
	void xerox820(machine_config &config) ATTR_COLD;

protected:
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);

	virtual void disk_drvsel_w(uint8_t data) override;
	virtual uint8_t disk_pa_bits() override { return (m_8n5 << 4) | (m_400_460 << 5); }

	void xerox820_mem(address_map &map) ATTR_COLD;
	void xerox820_io(address_map &map) ATTR_COLD;
	void mk83_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<fd1771_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;

	int m_8n5 = 0;                  // 5.25"/8" drive select
	int m_400_460 = 0;              // double sided disk detect
	uint8_t m_fdc_xor = 0xff;       // FD1771 data-bus inversion
	bool m_drvsel_binary = false;   // false = Xerox 820 bit-per-drive; true = Big Board binary unit#
};


// ======================> bigboard_state

class bigboard_state : public xerox820_state
{
public:
	bigboard_state(const machine_config &mconfig, device_type type, const char *tag)
		: xerox820_state(mconfig, type, tag)
		, m_beeper(*this, "beeper")
		, m_beep_timer(*this, "beep_timer")
	{ }

	void bigboard(machine_config &config) ATTR_COLD;
	void bigboard5(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

	virtual void kbpio_pa_w(uint8_t data) override;

	TIMER_DEVICE_CALLBACK_MEMBER(beep_timer);

	required_device<beep_device> m_beeper;
	required_device<timer_device> m_beep_timer;

	bool m_bit5 = false;
};


// ======================> xerox820ii_state
//
//  Xerox 820-II / 16/8: the disk controller is a daughterboard slot (FD1797
//  floppy controller or SASI host adapter), and the 16/8 adds an 8086
//  coprocessor sharing the bus.

class xerox820ii_state : public xerox820_base_state
{
public:
	xerox820ii_state(const machine_config &mconfig, device_type type, const char *tag) :
		xerox820_base_state(mconfig, type, tag),
		m_dbslot(*this, DBSLOT_TAG),
		m_copro(*this, COPRO_TAG),
		m_speaker(*this, "speaker")
	{ }

	void xerox820ii(machine_config &config) ATTR_COLD;    // 8" floppy
	void xerox820ii5(machine_config &config) ATTR_COLD;   // 5.25" floppy
	void xerox820iis(machine_config &config) ATTR_COLD;   // 8" floppy + SASI rigid disk
	void xerox820iilp(machine_config &config) ATTR_COLD;  // low profile keyboard
	void xerox168(machine_config &config) ATTR_COLD;      // 16/8, 8" floppy
	void xerox1685(machine_config &config) ATTR_COLD;     // 16/8, 5.25" floppy
	void xerox168s(machine_config &config) ATTR_COLD;     // 16/8, 8" floppy + SASI rigid disk
	void xerox168em(machine_config &config) ATTR_COLD;    // 16/8 + Expansion Module II (EM-II)

protected:
	virtual void disk_drvsel_w(uint8_t data) override { m_dbslot->drvsel_w(data); }
	virtual uint8_t disk_pa_bits() override;

	void bell_w(offs_t offset, uint8_t data);
	void slden_w(offs_t offset, uint8_t data);
	void chrom_w(offs_t offset, uint8_t data);
	void lowlite_w(uint8_t data);
	void sync_w(offs_t offset, uint8_t data);

	void common(machine_config &config, const char *disk_card) ATTR_COLD; // shared 820-II hardware + disk slot (no keyboard)
	void add_820ii_kbd(machine_config &config) ATTR_COLD;     // the standard 820-II ASCII keyboard
	void add_lpk(machine_config &config) ATTR_COLD;           // the Low Profile Keyboard (16/8 + 820-II LP)
	void add_8086(machine_config &config) ATTR_COLD;          // the 16/8 8086 coprocessor card

	void xerox820ii_mem(address_map &map) ATTR_COLD;
	void xerox820ii_io(address_map &map) ATTR_COLD;
	void xerox1685_io(address_map &map) ATTR_COLD;            // FDC map + reconstructed RX024 controller ROM

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<xerox820_dbslot_device> m_dbslot;
	required_device<xerox820_copro_slot_device> m_copro; // coprocessor slot (empty, the 16/8 8086 board, or the EM-II)
	required_device<speaker_sound_device> m_speaker;

	int m_lowlite = 0;          // low light attribute (modelled, unused by the video)
	int m_chrom = 0;            // character ROM index (modelled, unused by the video)
};

#endif // MAME_XEROX_XEROX820_H
