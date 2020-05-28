// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        BBC Master Cartridge slot emulation

**********************************************************************

    Pinout:
              A  B
        +5V   1  1   +5
        nOE   2  2   A10
       nRST   3  3   D3
       CSRW   4  4   A11
         A8   5  5   A9
        A13   6  6   D7
        A12   7  7   D6
       PHI2   8  8   D5
        -5V   9  9   D4
CSYNC/MADET  10  10  LPSTB
        R/W  11  11  BA7
       nNMI  12  12  BA6
       nIRQ  13  13  BA5
      nINFC  14  14  BA4
      nINFD  15  15  BA3
      ROMQA  16  16  BA2
      Clock  17  17  BA1
   nCRTCRST  18  18  BA0
      ADOUT  19  19  D0
       AGND  20  20  D2
         NC  21  21  D1
         0V  22  22  0V

    Signal Definitions:

    SIDE 'A'
    1     +5V - Power supply. This is the system logic supply rail.
    2     n0E - Output Enable : Input with CMOS levels. This is an active low signal during the PH12
                period of the system clock.
    3    nRST - System Reset : Input with CMOS levels. This signal is active low during system reset.
    4    CSRW - Changes function according to the memory region that the CPU is addressing.
                During accesses to &FC00 through &FEFF it is equivalent to the CPU Read/Write line
                during nPH12. For all other accesses it is an Active High chip select for memory devices.
    5      A8 - Address line 8 : Input with TTL levels
    6     A13 - Address line 13 : Input with TTL levels
    7     A12 - Address line 12 : Input with TTL levels
    8    PH12 - CPU clock : Input with CMOS levels
                This input is the host computer PH12out.
    9     -5V - The negative supply voltage.
   10   MADET - There are two functions dependent upon link 12 in the computer:
                E/nB - the default function. It enables cartridges to know which machine they are plugged
                       into. It is connected to 0V in the Master.
                CSYNC - Composite Sync. Input from TTL levels.
                        System Vertical & Horizontal sync is made available for Genlock use.
   11     R/W - Data Direction Control. Input from TTL levels
   12    nNMI - Non maskable interrupt : Open collector output
                This signal is connected to the system NMI line. It is active low.
   13    nIRQ - Interrupt request : Open collector output
                This signal is connected to the system IRQ line. It is active low.
   14   nINFC - Internal Page &FC : When bit IFJ in the Master ACCON register (via &FE34) is set, all accesses to
                &FC00 through &FCFF will cause this select to become active. : TTL active low
   15   nINFD - Internal page &FD : When bit IFJ in the Master ACCON register (via &FE34) is set, all accesses to
                &FD00 through &FDFF will cause this select to become active. : TTL active low
   16   ROMQA - Memory paging select : Input with TTL levels
                This is the least significant bit of the ROM select latch located at &FE30 in the Master.
   17   Clock - Links on the computer select one of two functions:
                a) 16Mhz Output to computer (Link DB only).
                b) 8 Mhz Input to cartridge (Link CD in addition to AB).
   18 nCRTCRST - nCRTCRST is an Active Low Output signal of the system CRTC reset input. It is
                provided for Genlock use.
   19   ADOUT - System audio output
   20    AGND - Audio Ground
   21    ADIN - Cartridge audio output
   22      0V - Zero volts

    SIDE 'B'
    1     +5V - Power supply. This is the system logic supply rail.
    2     A10 - Address line 10 : Input with TTL levels
    3      D3 - Data bus line 3 : Input/Output with TTL levels
    4     A11 - Address line 11 : Input with TTL levels
    5      A9 - Address line 9 : Input with TTL levels
    6      D7 - Most significant data bus line : Input/Output with TTL levels
    7      D6 - Data bus line 6 : Input/Output with TTL levels
    8      D5 - Data bus line 5 : Input/Output with TTL levels
    9      D4 - Data bus line 4 : Input/Output with TTL levels
   10   LPSTB - With link 21 removed in the computer, this pin provides a connection between the
                two cartridges. With the link in place, the pin forms a connection to a pull-up resistor in the
                computer to +5V. The connection is also made to the CRTC Light-Pen Strobe and interrupt
                structure
   11     BA7 - Buffered address line 7 : Input with TTL levels
   12     BA6 - Buffered address line 6 : Input with TTL levels
   13     BA5 - Buffered address line 5 : Input with TTL levels
   14     BA4 - Buffered address line 4 : Input with TTL levels
   15     BA3 - Buffered address line 3 : Input with TTL levels
   16     BA2 - Buffered address line 2 : Input with TTL levels
   17     BA1 - Buffered address line 1 : Input with TTL levels
   18     BA0 - Buffered address line 0 : Input with TTL levels
   19      D0 - Data bus line 0 : Input/Output with TTL levels
   20      D2 - Data bus line 2 : Input/Output with TTL levels
   21      D1 - Data bus line 1 : Input/Output with TTL levels
   22      0V - Zero volts.

**********************************************************************/
#ifndef MAME_BUS_BBC_CARTSLOT_H
#define MAME_BUS_BBC_CARTSLOT_H

#pragma once

#include "bus/electron/cart/slot.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_cartslot_device

class device_bbc_cart_interface;

class bbc_cartslot_device : public electron_cartslot_device
{
public:
	// construction/destruction
	template <typename T>
	bbc_cartslot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&slot_options, const char *default_option)
		: electron_cartslot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_cartslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const char *image_interface() const noexcept override { return "bbcm_cart"; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }
};


// ======================> device_bbc_cart_interface

class device_bbc_cart_interface : public device_electron_cart_interface
{
public:
	// construction/destruction
	virtual ~device_bbc_cart_interface();

protected:
	device_bbc_cart_interface(const machine_config &mconfig, device_t &device);
};


// device type definition
DECLARE_DEVICE_TYPE(BBCM_CARTSLOT, bbc_cartslot_device)

void bbcm_cart(device_slot_interface &device);


#endif // MAME_BUS_BBC_CARTSLOT_H
