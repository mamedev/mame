// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    Apple II Parallel Interface Card (670-0021)

    DB25 connector, with Centronics assignments:

         Data In, Bit 0   1
          Signal Ground   2  19  GND
         Data In, Bit 2   3
          Signal Ground   4
        Data Out, Bit 0   5   2  D0
        Data Out, Bit 1   6   3  D1
             (blocked)*   7
        Data Out, Bit 2   8   4  D2
                     --   9
                     --  10
        Data Out, Bit 5  11   7  D5
        Data Out, Bit 6  12   8  D6
        Data Out, Bit 7  13   9  D7
         Data In, Bit 4  14
             Strobe Out  15   1  /STROBE
         Acknowledge In  16  10  /ACK
         Data In, Bit 1  17
         Data In, Bit 7  18  18  +5V PULLUP
         Data In, Bit 5  19  12  POUT
          Signal Ground  20
         Data In, Bit 6  21  13  SEL
        Data Out, Bit 3  22   5  D3
        Data Out, Bit 4  23   6  D4
          Signal Ground  24  16  0V
         Data In, Bit 3  25  32  /FAULT

        *wired to 500ns strobe but connector hole is blocked

    This card has significant flaws:
    * The strobe pulse begins on the same rising edge of the phase 1
      clock as the data is latched.  The parallel load enable input
      to the strobe time counter (6A) is delayed slightly, but the
      load happens on the rising phase 1 clock edge.  This could
      glitch on a real printer.  MAME always sets the data outputs
      before starting the strobe pulse.
    * Acknowledge is ignored while the strobe output is active.  If
      the printer acknowledges the data before the end of the strobe
      pulse, the card will miss it and wait forever.

***********************************************************************/
#ifndef MAME_BUS_A2BUS_A2PIC_H
#define MAME_BUS_A2BUS_A2PIC_H

#pragma once

#include "a2bus.h"
#include "bus/centronics/ctronics.h"

class a2bus_pic_device : public device_t, public device_a2bus_card_interface
{
public:
	a2bus_pic_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// DIP switch/jumper handlers
	DECLARE_INPUT_CHANGED_MEMBER(sw1_strobe);
	DECLARE_INPUT_CHANGED_MEMBER(sw1_ack);
	DECLARE_INPUT_CHANGED_MEMBER(sw1_firmware);
	DECLARE_INPUT_CHANGED_MEMBER(sw1_irq);
	DECLARE_INPUT_CHANGED_MEMBER(x_data_out);
	DECLARE_INPUT_CHANGED_MEMBER(x_char_width);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// printer status inputs
	DECLARE_WRITE_LINE_MEMBER(ack_w);
	DECLARE_WRITE_LINE_MEMBER(perror_w);
	DECLARE_WRITE_LINE_MEMBER(select_w);
	DECLARE_WRITE_LINE_MEMBER(fault_w);

	// timer handlers
	TIMER_CALLBACK_MEMBER(release_strobe);

	// helpers
	void reset_mode();
	void start_strobe();
	void set_ack_latch();
	void clear_ack_latch();
	void enable_irq();
	void disable_irq();

	required_device<centronics_device>      m_printer_conn;
	required_device<output_latch_device>    m_printer_out;
	required_ioport                         m_input_sw1;
	required_ioport                         m_input_x;
	required_region_ptr<u8>                 m_prom;
	emu_timer *                             m_strobe_timer;

	u16 m_firmware_base;        // controlled by SW6
	u8  m_data_latch;           // 9B
	u8  m_autostrobe_disable;   // 2A pin 8
	u8  m_ack_latch;            // 2A pin 6
	u8  m_irq_enable;           // 2B pin 9
	u8  m_ack_in;
	u8  m_perror_in;            // DI5
	u8  m_select_in;            // DI6
	u8  m_fault_in;             // DI3
};


DECLARE_DEVICE_TYPE(A2BUS_PIC, a2bus_pic_device)

#endif // MAME_BUS_A2BUS_A2PIC_H
