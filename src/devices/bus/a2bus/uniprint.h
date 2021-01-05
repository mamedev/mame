// license:BSD-3-Clause
// copyright-holders:Vas Crabb, R. Belmont
/***********************************************************************

    Videx Uniprint Parallel Interface Card

    Manual, including schematic and theory of operation:
    https://www.apple.asimov.net/documentation/hardware/io/Videx%20UniPrint%20Manual.pdf

        Connector P2 pinout (pins 1 and 11 are at the top of the card):
        STB    1  11  GND
        D0     2  12  GND
        D1     3  13  GND
        D2     4  14  GND
        D3     5  15  GND
        D4     6  16  GND
        D5     7  17  GND
        D6     8  18  GND
        D7     9  19  GND
        ACK   10  20  GND

***********************************************************************/
#ifndef MAME_BUS_A2BUS_UNIPRINT_H
#define MAME_BUS_A2BUS_UNIPRINT_H

#pragma once

#include "a2bus.h"
#include "bus/centronics/ctronics.h"

class a2bus_uniprint_device : public device_t, public device_a2bus_card_interface
{
public:
	a2bus_uniprint_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// DIP switch handlers
	DECLARE_INPUT_CHANGED_MEMBER(sw_msb);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
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

	required_device<centronics_device>      m_printer_conn;
	required_device<output_latch_device>    m_printer_out;
	required_ioport                         m_s1, m_s2;
	required_region_ptr<u8>                 m_rom;
	emu_timer *                             m_strobe_timer;

	u8  m_data_latch;   // U6
	u8  m_ack_latch;    // U2 (pin 9)
	u8  m_next_strobe;  // U4 (pin 5)
	u8  m_ack_in;       // printer connector pin 10
};


DECLARE_DEVICE_TYPE(A2BUS_UNIPRINT, a2bus_uniprint_device)

#endif // MAME_BUS_A2BUS_UNIPRINT_H
