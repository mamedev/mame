// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        Electron Expansion Port emulation

**********************************************************************

    Pinout:

     18V AC   2  1   18V AC
  AC RETURN   4  3   AC RETURN
        -5V   6  5   -5V
         0V   8  7   0V
        +5V  10  9   +5V
      16MHz  12  11  SOUND O/P
    PHI OUT  14  13  /13 IN
        NMI  16  15  RST
        R/W  18  17  IRQ
         D6  20  19  D7
         D4  22  21  D5
         D2  24  23  D3
         D0  26  25  D1
         NC  28  27  RDY
       SLOT  30  29  SLOT
        A14  32  31  A15
        A12  34  33  A13
        A10  36  35  A11
         A0  38  37  A9
         A2  40  39  A1
         A4  42  41  A3
         A6  44  43  A5
         A8  46  45  A7
         0V  48  47  0V
        +5V  50  49  +5V

    Signal Definitions:

    18V AC (pins 1,2) - These lines are connected directly to the output from the Electron mains power
AC RETURNS (pins 3,4) - adaptor. A total of 6W may be drawn from these lines as long as no power is
                        taken from +5V (pins 9,10,49,50). For safety reasons these lines must never
                        be used as an AC input to the Electron.
       -5V (pins 5,6) - A -5V supply from the Electron. Up to 20mA (total) may safely be drawn
                        from this line by expansion modules.
  0V (pins 7,8,47,48) - Ground. Expansion modules with their own power supply must have the 0V
                        lines commoned with the Electron.
+5V (pins 9,10,49,50) - A +5V supply from the Electron. Up to 500mA (total) may safely be drawn
                        from this line by expansion modules as long as no power is taken from 18V
                        AC (pins 1,2,3,4).
   SOUND O/P (pin 11) - Sound output. A 3V peak to peak source via a 1K series resistor from the
                        Electron ULA.
      16 MHz (pin 12) - 16 Megahertz from the Electron main oscillator. This output may be used
                        for clock generation within an expansion module.
      /13 IN (pin 13) - 16 Megahertz divided by 13. This output may be used for baud rate
                        generation. If divided by 1024 it will give approximately 1200Hz.
     PHI OUT (pin 14) - The 6502 input clock. The low time is nominally 250ns. The high time may
                        be 250ns (2MHz operation when reading ROMs) or 750ns or 1250ns
                        (stretched clock for a 1MHz access, the length depending on the phase of the
                        2MHz clock) or up to 40us (if in modes 0-3)
         RST (pin 15) - Reset (active low). This is an OUTPUT ONLY for the system reset line. It
                        may be used to initialise expansion modules on power up and when the
                        BREAK key is pressed.
         NMI (pin 16) - Non-Maskable Interrupt (negative edge triggered). This is the system NMI
                        line which is open collector (wire-OR) and may be asserted by an expansion
                        module. The pull-up resistor on this line inside the ULA is 3k3. Care must
                        be taken to avoid masking other interrupts by holding the line low. Using
                        NMI on the Electron requires knowledge of operating system protocols.
         IRQ (pin 17) - Interrupt Request (active low). This is the system IRQ line which is open
                        collector (wire-OR) and may be asserted by an expansion module. The pull-
                        up resistor on this line inside the ULA is 3k3. It is essential for the correct
                        operation of the machine that interrupts to not occur until the software is
                        capable of dealing with them. Interrupts on the Electron expansion bus should
                        therefore be disabled on power-up and reset. Significant use of interrupt
                        service time may affect other machine functions, eg the real time clock.
         R/W (pin 18) - The system read/write line from the 6502.
   D7-D0 (pins 19-26) - Bi-directional data bus. The direction of data is determined by R/W.
         RDY (pin 27) - 6502 ready line (active low). May be asserted by an expansion module to
                        stop the processor when reading slow memory. This line works on read only
                        (R/W=1).
             (pin 28) - No connection
         (pins 29,30) - Polarising key connector.
  A0-A15 (pins 31-46) - 6502 address bus.

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_EXP_H
#define MAME_BUS_ELECTRON_EXP_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_expansion_slot_device

class device_electron_expansion_interface;

class electron_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_electron_expansion_interface>
{
public:
	// construction/destruction
	template <typename T>
	electron_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: electron_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	electron_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	uint8_t expbus_r(offs_t offset);
	void expbus_w(offs_t offset, uint8_t data);

	void irq_w(int state) { m_irq_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_electron_expansion_interface *m_card;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;
};


// ======================> device_electron_expansion_interface

class device_electron_expansion_interface : public device_interface
{
public:
	virtual uint8_t expbus_r(offs_t offset) { return 0xff; }
	virtual void expbus_w(offs_t offset, uint8_t data) { }

protected:
	device_electron_expansion_interface(const machine_config &mconfig, device_t &device);

	electron_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_EXPANSION_SLOT, electron_expansion_slot_device)

void electron_expansion_devices(device_slot_interface &device);


#endif // MAME_BUS_ELECTRON_EXP_H
