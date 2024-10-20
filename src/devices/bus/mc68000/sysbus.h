// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    mc-68000-Computer System Bus

    96-pin slot

    32  +5V     +5V      +5V
    31  +12V    +5V      -12V
    30  BA8     _IRQ1    BA1
    29  BA9     _IRQ2    BA2
    28  BA10    _IRQ3    BA3
    27  BA11    _IRQ4    BA4
    26  BA12    _IRQ5    BA5
    25  BA13    _IRQ6    BA6
    24  BA14    _IRQ7    BA7
    23  BA15    GND      _EXP
    22  BA16    _FLOPPY  FC2
    21  BA17    NC       FC1
    20  BA18    GND      FC0
    19  BA19    +12V     GND
    18  BA20    +12V     _BAS
    17  BA21    GND      GND
    16  BA22    NC       _DTACK
    15  BA23    NC       GND
    14  GND     LPSTB    BR/_W
    13  NC      _VPA     _BLDS
    12  _RESET  _BR      _BUDS
    11  _BERR   _EXT     GND
    10  HALT    NC       16 MHZ
     9  GND     _POR     GND
     8  BD15    NC       BD7
     7  BD14    B_R/W    BD6
     6  BD13    _BVMA    BD5
     5  BD12    BE       BD4
     4  BD11    _BG      BD3
     3  BD10    NC       BD2
     2  BD9     _BGACK   BD1
     1  BD8     _SSEL    BD0

***************************************************************************/

#ifndef MAME_BUS_MC68000_SYSBUS_H
#define MAME_BUS_MC68000_SYSBUS_H

#pragma once


// forward declaration
class device_mc68000_sysbus_card_interface;


//**************************************************************************
//  BUS DEVICE
//**************************************************************************

class mc68000_sysbus_device : public device_t
{
public:
	// construction/destruction
	mc68000_sysbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~mc68000_sysbus_device();

	void add_card(int slot, device_mc68000_sysbus_card_interface *card);

	auto irq1_cb() { return m_irq1_cb.bind(); }
	auto irq2_cb() { return m_irq2_cb.bind(); }
	auto irq3_cb() { return m_irq3_cb.bind(); }
	auto irq4_cb() { return m_irq4_cb.bind(); }
	auto irq5_cb() { return m_irq5_cb.bind(); }
	auto irq6_cb() { return m_irq6_cb.bind(); }
	auto irq7_cb() { return m_irq7_cb.bind(); }

	// from cards
	void irq1_w(int state);
	void irq2_w(int state);
	void irq3_w(int state);
	void irq4_w(int state);
	void irq5_w(int state);
	void irq6_w(int state);
	void irq7_w(int state);

	// called from host
	uint16_t slot_r(int slot, offs_t offset, uint16_t mem_mask = ~0);
	void slot_w(int slot, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t floppy_r(offs_t offset, uint16_t mem_mask = ~0);
	void floppy_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_mc68000_sysbus_card_interface *m_device_list[8];

	devcb_write_line m_irq1_cb;
	devcb_write_line m_irq2_cb;
	devcb_write_line m_irq3_cb;
	devcb_write_line m_irq4_cb;
	devcb_write_line m_irq5_cb;
	devcb_write_line m_irq6_cb;
	devcb_write_line m_irq7_cb;
};

// device type definition
DECLARE_DEVICE_TYPE(MC68000_SYSBUS, mc68000_sysbus_device)


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

class mc68000_sysbus_slot_device : public device_t, public device_single_card_slot_interface<device_mc68000_sysbus_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	mc68000_sysbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: mc68000_sysbus_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	mc68000_sysbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mc68000_sysbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(MC68000_SYSBUS_SLOT, mc68000_sysbus_slot_device)


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

class device_mc68000_sysbus_card_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_mc68000_sysbus_card_interface();

	void set_bus(mc68000_sysbus_device *bus, const char *slot_tag);

	device_mc68000_sysbus_card_interface *next() const { return m_next; }

	virtual uint16_t slot_r(offs_t offset, uint16_t mem_mask = ~0) { return 0xffff; }
	virtual void slot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { }

	virtual uint16_t floppy_r(offs_t offset, uint16_t mem_mask = ~0) { return 0xffff; }
	virtual void floppy_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { }

protected:
	device_mc68000_sysbus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	mc68000_sysbus_device *m_bus;

	device_mc68000_sysbus_card_interface *m_next;

	const char *m_slot_tag;
	int m_slot;
};


// include here so drivers don't need to
#include "cards.h"


#endif // MAME_BUS_MC68000_SYSBUS_H
