// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC75x - shared base for the RC759 Piccoline and the
    RC750 Partner.

    The two machines share a common core (Intel 80186 CPU, external 8259A
    PIC, Intel 8255 PPI, Intel 82730 CRT text coprocessor, MM58167 RTC,
    bank-switched 256x4 CMOS NVM, SN76489A sound, HLE keyboard). Only the
    floppy/serial/expansion side and the boot ROMs differ:

      - RC759 Piccoline: WD2797 FDC, cassette, iSBX slot, Centronics
        (see rc759.cpp), Concurrent CP/M-86.
      - RC750 Partner: WD1797 FDC, Intel 8274 serial, SCSI bus, optional
        8087 (see rc750.cpp), Concurrent DOS.

    rc75x_state holds the shared devices and handlers; rc759_state and
    rc750_state derive from it (NEITHER derives from the other).

***************************************************************************/

#ifndef MAME_REGNECENTRALEN_RC75X_H
#define MAME_REGNECENTRALEN_RC75X_H

#pragma once

#include "rc759_kbd.h"

#include "cpu/i86/i186.h"
#include "machine/i8255.h"
#include "machine/mm58167.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "sound/sn76496.h"
#include "sound/spkrdev.h"
#include "video/i82730.h"
#include "emupal.h"


// ======================> rc75x_state

class rc75x_state : public driver_device
{
public:
	rc75x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic"),
		m_nvram(*this, "nvram"),
		m_ppi(*this, "ppi"),
		m_txt(*this, "txt"),
		m_palette(*this, "palette"),
		m_speaker(*this, "speaker"),
		m_snd(*this, "snd"),
		m_rtc(*this, "rtc"),
		m_vram(*this, "vram"),
		m_config(*this, "config"),
		m_kbd(*this, "kbd"),
		m_drq_source(0),
		m_nvram_bank(0),
		m_gfx_mode(0)
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;

	// Adds the devices common to both models to the machine config. The
	// caller (rc759()/rc750()) must have created m_maincpu first, since the
	// 82730, the screen vblank and the slave-ack callback reference it.
	void add_common_devices(machine_config &config);

	// shared core devices
	required_device<i80186_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<nvram_device> m_nvram;
	required_device<i8255_device> m_ppi;
	required_device<i82730_device> m_txt;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	required_device<sn76489a_device> m_snd;
	required_device<mm58167_device> m_rtc;
	required_shared_ptr<uint16_t> m_vram;
	required_ioport m_config;
	required_device<rc759_kbd_hle_device> m_kbd;

	// shared 82730 text/video
	I82730_UPDATE_ROW(txt_update_row);
	void txt_ca_w(uint16_t data);
	void txt_irst_w(uint16_t data);
	uint8_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);

	// shared bank-switched 256x4 CMOS NVM (I/O 0x80-0xfe)
	void nvram_init(nvram_device &nvram, void *data, size_t size);
	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);

	// shared MM58167 RTC glue (I/O 0x5a/0x5c)
	void rtc_data_w(uint8_t data);
	uint8_t rtc_data_r();
	void rtc_addr_w(uint8_t data);

	// shared misc
	void i186_timer1_w(int state);
	uint8_t irq_callback();

	std::vector<uint8_t> m_nvram_mem;

	int m_drq_source;
	int m_nvram_bank;
	int m_gfx_mode;

	uint8_t m_rtc_read_addr = 0;
	uint8_t m_rtc_read_data = 0;
	uint8_t m_rtc_write_addr = 0;
	uint8_t m_rtc_write_data = 0;
	uint8_t m_rtc_strobe = 0;
};

#endif // MAME_REGNECENTRALEN_RC75X_H
