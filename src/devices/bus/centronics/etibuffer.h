// license:BSD-3-Clause
// copyright-holders:Golden Child
/***********************************************************************

    Electronics Today International Print Buffer

    Designed by Nick Sawyer and published in the July and August 1985
       issues of Electronics Today International (UK Edition)

    Electronics-Today-1985-07.pdf
        (pages 33-37 part 1 : schematic, parts list, description)
        (pages 54-55 pcb foil pattern)

    Electronics-Today-1985-08.pdf
        (pages 48-50 part 2 : rom listing, memory map)

    Electronics-Today-1985-09.pdf (page 52) (follow up notes)
    Electronics-Today-1985-10.pdf (page 58) (follow up notes)
    Electronics-Today-1985-11.pdf (page 13) (follow up notes)

     Some interesting notes about the rom code:

    * All state is held in the registers so entire 48k ram can be used.
    * Uses no subroutine calls and avoids use of the stack.
    * For refresh, executes continuous opcodes located between XX80 and XXF0
        in the memory map at least once every 4ms.

    * Rom code is entered from the scanned magazine,
       with a bug fixed by inserting an 0x00 nop at 0x382,
       shifting bytes 0x382-0x3ee to 0x383-0x3ef.

***********************************************************************/

#ifndef MAME_BUS_CENTRONICS_ETIBUFFERDEV_H
#define MAME_BUS_CENTRONICS_ETIBUFFERDEV_H

#pragma once

#include "ctronics.h"
#include "screen.h"
#include "cpu/z80/z80.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class etiprintbuffer_device : public device_t, public device_centronics_peripheral_interface
{

public:
	etiprintbuffer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_strobe ) override;
	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) override { if (state) m_data |= 0x01; else m_data &= ~0x01; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) override { if (state) m_data |= 0x02; else m_data &= ~0x02; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) override { if (state) m_data |= 0x04; else m_data &= ~0x04; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) override { if (state) m_data |= 0x08; else m_data &= ~0x08; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) override { if (state) m_data |= 0x10; else m_data &= ~0x10; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) override { if (state) m_data |= 0x20; else m_data &= ~0x20; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) override { if (state) m_data |= 0x40; else m_data &= ~0x40; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) override { if (state) m_data |= 0x80; else m_data &= ~0x80; }

	DECLARE_INPUT_CHANGED_MEMBER(test_sw);  // test buffer switch
	DECLARE_INPUT_CHANGED_MEMBER(clear_sw); // clear buffer switch

protected:
	DECLARE_WRITE_LINE_MEMBER( busy_w );

	etiprintbuffer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_device<z80_device> m_maincpu;
	optional_device<screen_device> m_screen;
	required_device<centronics_device> m_ctx;
	required_device<output_latch_device> m_ctx_data_out;

	output_finder<> m_printerready_led;
	output_finder<> m_bufferready_led;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	void etiprintbuffer_device_memmap(address_map &map);
	void etiprintbuffer_device_iomap(address_map &map);

	uint32_t screen_update_etibuffer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void drawbar(double xval1, double xval2, double x1, double x2, double y1, double y2, int width, bitmap_rgb32 &bitmap, u32 color);
	void draw7seg(u8 digit, int x0, int y0, int w, int h, int t, bitmap_rgb32 &bitmap, u32 color);

	uint8_t eti_status_r(offs_t offset);
	uint8_t eti_read_1000(offs_t offset);
	void    eti_write_2000(offs_t offset, uint8_t data);
	void    eti_write_3000(offs_t offset, uint8_t data);

	u8 m_data;
	u8 m_datalatch;         // latch data when strobed
	bool m_strobereceived;  // flip flop to handle strobe
	bool m_strobe;          // flip flop to handle ack status
	bool m_busy;

	emu_timer *m_ack_timer;
	emu_timer *m_strobeout_timer;

	enum { TIMER_ACK, TIMER_STROBEOUT };

	u16 m_buffersize = 0;  // used to draw graphic representation of buffer
	u16 m_bufferhead = 0;
	u16 m_buffertail = 0;

	u8 m_ram[48 * 1024];  // 48k ram
};

// device type definition
DECLARE_DEVICE_TYPE(ETIPRINTBUFFER, etiprintbuffer_device)

#endif // MAME_BUS_CENTRONICS_ETIBUFFERDEV_H
