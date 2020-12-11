// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    Orange Micro Grappler+ Printer Interface

    With references to schematic from Grappler™+ Printer Interface
    Series Operators Manual (© Orange Micro, Inc. 1982).  This uses
    the IC locations on the "long" version of the card.  The newer
    "short" version of the card has slightly different circuitry
    and completely different IC locations.

    26-pin two-row header to printer:

        STB    1   2  GND
        D0     3   4  GND
        D1     5   6  GND
        D2     7   8  GND
        D3     9  10  GND
        D4    11  12  GND
        D5    23  14  GND
        D6    15  16  GND
        D7    17  18  GND
        ACK   19  20  GND
        BUSY  21  22  GND
        P.E.  23  24  GND
        SLCT  25  26  GND

***********************************************************************/
#ifndef MAME_BUS_A2BUS_GRAPPLERPLUS_H
#define MAME_BUS_A2BUS_GRAPPLERPLUS_H

#pragma once

#include "a2bus.h"
#include "bus/centronics/ctronics.h"

class a2bus_grapplerplus_device : public device_t, public device_a2bus_card_interface
{
public:
	a2bus_grapplerplus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// DIP switch handlers
	DECLARE_INPUT_CHANGED_MEMBER(sw_msb);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;
	virtual u8 read_c800(u16 offset) override;

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// timer handlers
	TIMER_CALLBACK_MEMBER(update_strobe);

private:
	// printer status inputs
	DECLARE_WRITE_LINE_MEMBER(ack_w);
	DECLARE_WRITE_LINE_MEMBER(busy_w);
	DECLARE_WRITE_LINE_MEMBER(pe_w);
	DECLARE_WRITE_LINE_MEMBER(slct_w);

	required_device<centronics_device>      m_printer_conn;
	required_device<output_latch_device>    m_printer_out;
	required_ioport                         m_s1;
	required_region_ptr<u8>                 m_rom;
	emu_timer *                             m_strobe_timer;

	u16 m_rom_bank;     // U2D (pin 13)
	u8  m_data_latch;   // U10
	u8  m_ack_latch;    // U2C (pin 9)
	u8  m_irq_disable;  // U2A (pin 4)
	u8  m_irq;          // U3D (pin 13)
	u8  m_next_strobe;  // U5A (pin 5)
	u8  m_ack_in;       // printer connector pin 19
	u8  m_busy_in;      // printer connector pin 21
	u8  m_pe_in;        // printer connector pin 23
	u8  m_slct_in;      // printer connector pin 25
};


DECLARE_DEVICE_TYPE(A2BUS_GRAPPLERPLUS, a2bus_grapplerplus_device)

#endif // MAME_BUS_A2BUS_GRAPPLERPLUS_H
