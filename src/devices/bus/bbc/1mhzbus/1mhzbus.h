// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC 1MHz Bus emulation

**********************************************************************

  Pinout:

             0V   1   2  R/W
             0V   3   4  1MHzE
             0V   5   6  NNMI
             0V   7   8  NIRQ
             0V   9  10  NPGFC
             0V  11  12  NPGFD
             0V  13  14  NRST
             0V  15  16  Analog In
             0V  17  18  D0
             D1  19  20  D2
             D3  21  22  D4
             D5  23  24  D6
             D7  25  26  0V
             A0  27  28  A1
             A2  29  30  A3
             A4  31  32  A5
             A6  33  34  A7

  Signal Definitions:

              0 volts - This is connected to the main system 0 volts line. The reason for putting
                        0V lines between the active signal lines is to reduce the interference
                        between different signals.
          R/W (pin 2) - This is the read-not-write signal from the 6502 CPU, buffered by two
                        74LS04 inverters.
        1MHzE (pin 4) - This is the 1MHz system timing clock. It is a 50% duty-cycle square wave.
                        The 6502 CPU is operating at 2MHz, so the main processor clock is stretched
                        whenever 1MHz bus peripherals are being accessed. The trailing edges of the
                        1MHz and 2MHz processor clock are then coincidental. The processor clock
                        is only ever truly 2MHz when accessing main memory.
         NNMI (pin 6) - Not Non-Maskable Interrupt. This is connected directly to the 6502 NMI input.
                        It is pulled up to +5 volts with a 3K3 resistor. Both Disc and Econet systems
                        rely heavily upon NMIs for their operation so take care. Note that NMIs are
                        triggered on negative going edges of NMI signals.
         NIRQ (pin 8) - Not Interrupt Request. This is connected directly to the 6502 IRQ input. Any
                        devices connected to this input should have open collector outputs. The line
                        is pulled up to +5 volts with a 3K3 resistor. Interrupts from the 1MHz bus
                        must not occur until the software on the main system is able to cope with
                        them. All interrupts must therefore be disabled after a reset. Note that the
                        main system software may operate very slowly if considerable use is made of
                        interrupts. Certain functions such as the real time clock which is
                        incremented every 10 mS will be affected if interrupts are masked for more
                        than this period.
       NPGFC (pin 10) - Not page &FC. This signal is derived from the 6502 address bus. It goes low
                        whenever page &FC is written to or read from. FRED is the name given to this
                        page in memory.
      NPGGFD (pin 12) - Not page &FD. This signal is derived from the 6502 address bus. It goes low
                        whenever page &FD is accessed. JIM is the name given to this page in memory.
        NRST (pin 14) - Not RESET. This is an active low output from the system reset line. It may
                        be used to initialise peripherals whenever a power up or a BREAK causes a
                        reset.
   Analog In (pin 16) - This is an input to the audio amplifier on the main computer. The amplified
                        signal is produced over the speaker on the keyboard. Its input impedance is
                        9K Ohms and a 3 volt RMS signal will produce maximum volume on the speaker.
                        Note however that signals as large as this will cause distortion if the sound
                        or speech is used at the same time.
   D0-D7 (pins 18-24) - This is a bi-directional 8 bit data bus which is connected via a 74LS245
                        buffer (IC72) to the CPU. The direction of data transfer is determined by
                        the R/W line signal. The buffer is enabled whenever FRED or JIM are accessed.
   A0-A7 (pins 27-34) - These are connected directly to the lower 8 CPU address lines via a 74LS244
                        buffer (IC71) which is always enabled.

**********************************************************************/

#ifndef MAME_BUS_BBC_1MHZBUS_1MHZBUS_H
#define MAME_BUS_BBC_1MHZBUS_1MHZBUS_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_1mhzbus_slot_device

class device_bbc_1mhzbus_interface;

class bbc_1mhzbus_slot_device : public device_t, public device_single_card_slot_interface<device_bbc_1mhzbus_interface>
{
public:
	// construction/destruction
	template <typename T>
	bbc_1mhzbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&slot_options, const char *default_option)
		: bbc_1mhzbus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_1mhzbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	virtual uint8_t fred_r(offs_t offset);
	virtual void fred_w(offs_t offset, uint8_t data);
	virtual uint8_t jim_r(offs_t offset);
	virtual void jim_w(offs_t offset, uint8_t data);

	void irq_w(int state) { m_irq_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	device_bbc_1mhzbus_interface *m_card;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;
};


// ======================> device_bbc_1mhzbus_interface

class device_bbc_1mhzbus_interface : public device_interface
{
public:
	virtual uint8_t fred_r(offs_t offset) { return 0xff; }
	virtual void fred_w(offs_t offset, uint8_t data) { }
	virtual uint8_t jim_r(offs_t offset) { return 0xff; }
	virtual void jim_w(offs_t offset, uint8_t data) { }

protected:
	device_bbc_1mhzbus_interface(const machine_config &mconfig, device_t &device);

	bbc_1mhzbus_slot_device *const m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_1MHZBUS_SLOT, bbc_1mhzbus_slot_device)

void bbc_1mhzbus_devices(device_slot_interface &device);
void bbcm_1mhzbus_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_1MHZBUS_1MHZBUS_H
