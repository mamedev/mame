// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II coprocessor slot

    The 16/8 option board plugs an 8086 (16-bit) coprocessor onto the
    820-II, sharing the Z80 bus.  The Z80 reaches the 8086's resident
    RAM through the banked-ROM memory view window (0x4000-0xBFFF ->
    8086 0xF8000+), and controls the coprocessor (stop / start / lock /
    doorbell) through I/O ports A0/A1, which the card installs into the
    Z80 I/O space when it starts.  An empty slot is a plain 820-II.

**********************************************************************/

#ifndef MAME_BUS_XEROX820_COPRO_H
#define MAME_BUS_XEROX820_COPRO_H

#pragma once

#include "cpu/z80/z80.h"


class xerox820_copro_slot_device;


// ======================> device_xerox820_copro_card_interface

class device_xerox820_copro_card_interface : public device_interface
{
	friend class xerox820_copro_slot_device;

public:
	// Z80 shared-RAM window (offset into the 8086 resident RAM at 0xF8000)
	virtual uint8_t shared_ram_r(offs_t offset) { return 0xff; }
	virtual void shared_ram_w(offs_t offset, uint8_t data) { }

protected:
	device_xerox820_copro_card_interface(const machine_config &mconfig, device_t &device);

	xerox820_copro_slot_device *m_slot;
};


// ======================> xerox820_copro_slot_device

class xerox820_copro_slot_device : public device_t,
		public device_single_card_slot_interface<device_xerox820_copro_card_interface>
{
public:
	template <typename T>
	xerox820_copro_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: xerox820_copro_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	xerox820_copro_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// the host Z80 (the card installs its control ports into its I/O space and
	// times the shared-RAM bus arbiter against its cycle count)
	template <typename T> void set_maincpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	z80_device &maincpu() const { return *m_maincpu; }

	// Z80 shared-RAM window (returns 0xFF / ignores writes when the slot is empty)
	uint8_t shared_ram_r(offs_t offset) { return m_card ? m_card->shared_ram_r(offset) : 0xff; }
	void shared_ram_w(offs_t offset, uint8_t data) { if (m_card) m_card->shared_ram_w(offset, data); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	device_xerox820_copro_card_interface *m_card;
};


// device type declarations
DECLARE_DEVICE_TYPE(XEROX820_COPRO_SLOT, xerox820_copro_slot_device)

// the 16/8 8086 board and the EM-II (DEM) variants are private to copro.cpp and
// are exposed only through the card interface, so they are declared against it
// (paired with DEFINE_DEVICE_TYPE_PRIVATE in the source file)
DECLARE_DEVICE_TYPE(XEROX820_16_8,       device_xerox820_copro_card_interface) // the 16/8 8086 board
DECLARE_DEVICE_TYPE(XEROX820_EMII_RGD5,  device_xerox820_copro_card_interface) // EM-II rigid + floppy (box id 0x21)
DECLARE_DEVICE_TYPE(XEROX820_EMII_FLPY5, device_xerox820_copro_card_interface) // EM-II floppy only    (box id 0x20)


// slot option registration
void xerox820_copro_cards(device_slot_interface &device);

#endif // MAME_BUS_XEROX820_COPRO_H
