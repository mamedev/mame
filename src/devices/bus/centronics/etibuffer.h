// license:BSD-3-Clause
// copyright-holders:Golden Child
/***********************************************************************

    Electronics Today International Print Buffer 48K

    Designed by Nick Sawyer and published in the July and August 1985
       issues of Electronics Today International (UK Edition)

    https://worldradiohistory.com/UK/Electronics-Today-UK/80s/Electronics-Today-1985-08.pdf
    https://worldradiohistory.com/UK/Electronics-Today-UK/80s/Electronics-Today-1985-07.pdf

    Electronics-Today-1985-07.pdf
        (pages 33-37 part 1 : schematic, parts list, description)
        (pages 54-55 pcb foil pattern)

    Electronics-Today-1985-08.pdf
        (pages 48-50 part 2 : rom listing, memory map)

    Electronics-Today-1985-09.pdf (page 52) (follow up notes)
    Electronics-Today-1985-10.pdf (page 58) (follow up notes)
    Electronics-Today-1985-11.pdf (page 13) (follow up notes)

     Some notes about the rom code:

    * All state is held in the cpu registers so entire 48k ram can be used.
    * Uses no subroutine calls and avoids use of the stack.
    * For refresh, executes continuous opcodes located between XX80 and XXF0
        in the memory map at least once every 4ms.

    * Rom code is entered from the scanned magazine,
       with a bug fixed by inserting a 0x00 nop at 0x382,
       shifting bytes 0x382-0x3ee to 0x383-0x3ef to make the hardcoded
       addresses point at the correct locations.

    * Memory map is simple:

    0x0000-0x7ff    rom code
    0x1000-0x1fff   reads parallel data in, strobes ack and clears busy
    0x2000-0x2fff   writes data to parallel out
    0x3000-0x3fff   write will strobe the parallel out
    0x4000-0xffff   ram (all used for buffer)

    * IO map:

    0x0000-0xffff   any read returns status of parallel in strobe received in bit 6,
                    and not busy parallel out in bit 7

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
	DECLARE_WRITE_LINE_MEMBER( busy_w );  // centronics output busy

	etiprintbuffer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	void etiprintbuffer_device_memmap(address_map &map);
	void etiprintbuffer_device_iomap(address_map &map);

	uint8_t eti_status_r(offs_t offset);
	uint8_t eti_read_1000(offs_t offset);
	void    eti_write_2000(offs_t offset, uint8_t data);
	void    eti_write_3000(offs_t offset, uint8_t data);

	required_device<z80_device> m_maincpu;
	optional_device<screen_device> m_screen;
	required_device<centronics_device> m_ctx;
	required_device<output_latch_device> m_ctx_data_out;

	output_finder<> m_printerready_led;
	output_finder<> m_bufferready_led;

	u8 m_data;              // data from centronics input
	u8 m_datalatch;         // latch data when strobed
	bool m_strobereceived;  // flip flop to handle input strobe
	bool m_strobe;          // current state of input strobe line
	bool m_busy;            // current state of output busy line

	emu_timer *m_ack_timer;
	emu_timer *m_strobeout_timer;

	enum { TIMER_ACK, TIMER_STROBEOUT };

	u8 m_ram[48 * 1024];  // 48k ram

};

// device type definition
DECLARE_DEVICE_TYPE(ETIPRINTBUFFER, etiprintbuffer_device)

#endif // MAME_BUS_CENTRONICS_ETIBUFFERDEV_H
