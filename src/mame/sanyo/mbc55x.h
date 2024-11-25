// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith
/*
    mbc55x.h
    Includes for the Sanyo MBC-550, MBC-555.

    Phill Harvey-Smith
    2011-01-29.
*/
#ifndef MAME_SANYO_MBC55X_H
#define MAME_SANYO_MBC55X_H

#pragma once

#include "bus/a2gameio/gameio.h"
#include "bus/centronics/ctronics.h"
#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"


#define MAINCPU_TAG "maincpu"

#define SCREEN_TAG      "screen"
#define SCREEN_WIDTH_PIXELS     640
#define SCREEN_HEIGHT_LINES     250
#define SCREEN_NO_COLOURS       8

#define VIDEO_MEM_SIZE      (32*1024)
#define VID_MC6845_NAME     "mc6845"

// Red and blue colour planes sit at a fixed location, green
// is in main memory.

#define COLOUR_PLANE_SIZE   0x4000

#define RED_PLANE_OFFSET    (0*COLOUR_PLANE_SIZE)
#define BLUE_PLANE_OFFSET   (1*COLOUR_PLANE_SIZE)

#define COLOUR_PLANE_MEMBASE    0xF0000
#define RED_PLANE_MEMBASE   (COLOUR_PLANE_MEMBASE+RED_PLANE_OFFSET)
#define BLUE_PLANE_MEMBASE  (COLOUR_PLANE_MEMBASE+BLUE_PLANE_OFFSET)

#define RED_PLANE_TAG       "red"
#define BLUE_PLANE_TAG      "blue"

#define PPI8255_TAG     "ppi8255"
#define PIC8259_TAG     "pic8259"
#define PIT8253_TAG     "pit8253"

#define MONO_TAG                "mono"
#define I8251A_KB_TAG           "i8251a_kb"
#define FDC_TAG                 "wd1793"



class mbc55x_state : public driver_device
{
public:
	mbc55x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU_TAG),
		m_iodecode(*this, "iodecode"),
		m_crtc(*this, VID_MC6845_NAME),
		m_kb_uart(*this, I8251A_KB_TAG),
		m_pit(*this, PIT8253_TAG),
		m_ppi(*this, PPI8255_TAG),
		m_pic(*this, PIC8259_TAG),
		m_fdc(*this, FDC_TAG),
		m_floppy(*this, FDC_TAG ":%u", 0U),
		m_gameio(*this, "gameio"),
		m_printer(*this, "printer"),
		m_speaker(*this, "speaker"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette")
	{
	}

	void mbc55x(machine_config &config);

	required_device<i8086_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_iodecode;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

private:
	uint8_t iodecode_r(offs_t offset);
	void iodecode_w(offs_t offset, uint8_t data);

	uint8_t vram_page_r();
	void vram_page_w(uint8_t data);
	uint8_t game_io_r();
	uint8_t printer_status_r();
	void printer_data_w(uint8_t data);
	void disk_select_w(uint8_t data);
	void printer_busy_w(int state);
	void printer_paper_end_w(int state);
	void printer_select_w(int state);

	void vid_hsync_changed(int state);
	void vid_vsync_changed(int state);

	MC6845_UPDATE_ROW(crtc_update_row);
	void mbc55x_palette(palette_device &palette) const;

	void mbc55x_io(address_map &map) ATTR_COLD;
	void mbc55x_mem(address_map &map) ATTR_COLD;
	void mbc55x_iodecode(address_map &map) ATTR_COLD;

	void set_ram_size();

	required_device<mc6845_device> m_crtc;
	required_device<i8251_device> m_kb_uart;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<pic8259_device> m_pic;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<apple2_gameio_device> m_gameio;
	required_device<centronics_device> m_printer;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;

	uint8_t       m_video_mem[VIDEO_MEM_SIZE];
	uint8_t       m_vram_page = 0;
	uint8_t       m_printer_status = 0;

	double        m_x_calibration = 0, m_y_calibration = 0;
	bool          m_ls123_strobe = false;
	double        m_ls123_clear_time[4];
};

/*----------- defined in machine/mbc55x.c -----------*/

/* Memory controller */
#define RAM_BANK00_TAG  "bank0"
#define RAM_BANK01_TAG  "bank1"
#define RAM_BANK02_TAG  "bank2"
#define RAM_BANK03_TAG  "bank3"
#define RAM_BANK04_TAG  "bank4"
#define RAM_BANK05_TAG  "bank5"
#define RAM_BANK06_TAG  "bank6"
#define RAM_BANK07_TAG  "bank7"
#define RAM_BANK08_TAG  "bank8"
#define RAM_BANK09_TAG  "bank9"
#define RAM_BANK0A_TAG  "banka"
#define RAM_BANK0B_TAG  "bankb"
#define RAM_BANK0C_TAG  "bankc"
#define RAM_BANK0D_TAG  "bankd"
#define RAM_BANK0E_TAG  "banke"

#define RAM_BANK_SIZE   (64*1024)
#define RAM_BANK_COUNT  15

/* Floppy drive interface */

#define FDC_PAUSE               10000


/*----------- defined in video/mbc55x.c -----------*/

#define LINEAR_ADDR(seg,ofs)    ((seg<<4)+ofs)

#define OUTPUT_SEGOFS(mess,seg,ofs)  logerror("%s=%04X:%04X [%08X]\n",mess,seg,ofs,((seg<<4)+ofs))

#endif // MAME_SANYO_MBC55X_H
