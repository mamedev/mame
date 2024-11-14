// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        Electron Cartridge slot emulation

**********************************************************************

    Pinout:
              A  B
        +5V   1  1   +5
        nOE   2  2   A10
       nRST   3  3   D3
         RW   4  4   A11
         A8   5  5   A9
        A13   6  6   D7
        A12   7  7   D6
       PHI2   8  8   D5
        -5V   9  9   D4
         NC  10  10  nOE2
      READY  11  11  BA7
       nNMI  12  12  BA6
       nIRQ  13  13  BA5
      nINFC  14  14  BA4
      nINFD  15  15  BA3
      ROMQA  16  16  BA2
      16MHz  17  17  BA1
    nROMSTB  18  18  BA0
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
    4      RW - Read/Write : Input with CMOS levels. This pin is the CPU read/write line.
    5      A8 - Address line 8 : Input with TTL levels
    6     A13 - Address line 13 : Input with TTL levels
    7     A12 - Address line 12 : Input with TTL levels
    8    PH12 - CPU clock : Input with CMOS levels
                This input is the host computer PH12out.
    9     -5V - The negative supply voltage.
   10      NC - This is a "no connect" on the Electron.
   11   READY - CPU wait state control : Open collector output
   12    nNMI - Non maskable interrupt : Open collector output
                This signal is connected to the system NMI line. It is active low.
   13    nIRQ - Interrupt request : Open collector output
                This signal is connected to the system IRQ line. It is active low.
   14   nINFC - Internal Page &FC : Memory active decode input : TTL active low
   15   nINFD - Internal page &FD : Memory active decode input : TTL active low
   16   ROMQA - Memory paging select : Input with TTL levels
                This is the least significant bit of the ROM select latch located at &FE05 in the Electron.
   17   Clock - Clock is a 16MHz input with TTL levels.
   18 nROMSTB - nROMSTB is an active low input using TTL levels which selects the location &FC73.
                This is intended to be used as a paging register.
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
   10    nOE2 - Output Enable : Input with TTL levels
                This line provides an additional active low output enable for ROMs in the Electron.
                This corresponds to ROM position 13 and consequently responds quickly to service
                calls. It is low during the active low portion of PH12.
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
#ifndef MAME_BUS_ELECTRON_CARTSLOT_H
#define MAME_BUS_ELECTRON_CARTSLOT_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define ELECTRON_CART_ROM_REGION_TAG ":cart:rom"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_cartslot_device

class device_electron_cart_interface;

class electron_cartslot_device : public device_t,
									public device_cartrom_image_interface,
									public device_single_card_slot_interface<device_electron_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	electron_cartslot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&slot_options, const char *default_option)
		: electron_cartslot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	electron_cartslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "electron_cart"; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }

	// device_image_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2);
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2);

	virtual bool present() { return is_loaded() || loaded_through_softlist(); }

	void irq_w(int state) { m_irq_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

protected:
	electron_cartslot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	device_electron_cart_interface *m_cart;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;
};


// ======================> device_electron_cart_interface

class device_electron_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_electron_cart_interface();

	// reading and writing
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) { }

	void rom_alloc(uint32_t size, const char *tag);
	void ram_alloc(uint32_t size);
	void nvram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint8_t* get_nvram_base() { return &m_nvram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }
	uint32_t get_nvram_size() { return m_nvram.size(); }

protected:
	device_electron_cart_interface(const machine_config &mconfig, device_t &device);

	electron_cartslot_device *m_slot;

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
	std::vector<uint8_t> m_nvram;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_CARTSLOT, electron_cartslot_device)

void electron_cart(device_slot_interface &device);


#endif // MAME_BUS_ELECTRON_CARTSLOT_H
