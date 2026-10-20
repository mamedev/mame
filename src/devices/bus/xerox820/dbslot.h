// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II disk-controller daughterboard slot

    The 820-II main board carries a single connector for the 9R80758
    disk-controller daughterboard.  Two boards plug into it:

      - the FD1797 floppy controller (5.25" or 8" Shugart drives), or
      - the Shugart SASI host adapter (an SA1403D carrying the floppies
        on LUN 0-2 and an ST-506 rigid disk on LUN 3 over SASI).

    Both occupy the Z80 I/O window 0x10-0x13.  The main board's system
    PIO drives the drive-select / side lines to the connector and reads
    the media-size / two-sided sense back; the FDC's INTRQ and DRQ
    return to the main board's /HALT-gated /NMI generator.  The SASI
    board's "u8" Z80PIO joins the Z80 IM2 daisy chain.

**********************************************************************/

#ifndef MAME_BUS_XEROX820_DBSLOT_H
#define MAME_BUS_XEROX820_DBSLOT_H

#pragma once

#include "machine/z80daisy.h"


class xerox820_dbslot_device;


// ======================> device_xerox820_dbslot_card_interface

class device_xerox820_dbslot_card_interface : public device_interface
{
	friend class xerox820_dbslot_device;

public:
	// main board <-> card I/O window (Z80 ports 0x10-0x13)
	virtual uint8_t io_r(offs_t offset) { return 0xff; }
	virtual void io_w(offs_t offset, uint8_t data) { }

	// system PIO port A: drive-select / side (bits 0-2) out, media sense back
	virtual void drvsel_w(uint8_t data) { }
	virtual bool media_8inch() { return false; }   // PA4: 8"/5.25" media sense
	virtual bool media_twosided() { return false; } // PA5: double-sided sense

	// system latch: density select (1 = FM/single, 0 = MFM/double)
	virtual void density_w(int fm) { }

	// 16/8 RX024 expansion-box select latch (raw I/O-bus write to the box's own
	// drive/side latch; only the single-floppy 5.25" box card acts on it)
	virtual void box_select_w(uint8_t data) { }

	// 820-II "signon" personality reported on system PIO PA bits 0-1
	// (bit0 = FD1797 floppy controller, bit1 = SASI host adapter)
	virtual uint8_t personality() { return 0; }

	// Z80 daisy chain (only the SASI board's u8 PIO participates)
	virtual int z80daisy_irq_state() { return 0; }
	virtual int z80daisy_irq_ack() { return 0; }
	virtual void z80daisy_irq_reti() { }

protected:
	device_xerox820_dbslot_card_interface(const machine_config &mconfig, device_t &device);

	xerox820_dbslot_device *m_slot;
};


// ======================> xerox820_dbslot_device

class xerox820_dbslot_device : public device_t,
		public device_single_card_slot_interface<device_xerox820_dbslot_card_interface>,
		public device_z80daisy_interface
{
public:
	// construction/destruction
	template <typename T>
	xerox820_dbslot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: xerox820_dbslot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	xerox820_dbslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// the FDC's INTRQ/DRQ return to the main board's /NMI generator
	auto intrq_wr_callback() { return m_intrq_cb.bind(); }
	auto drq_wr_callback() { return m_drq_cb.bind(); }
	// the SASI board's u8 PIO drives the Z80 maskable interrupt (IM2 daisy)
	auto int_wr_callback() { return m_int_cb.bind(); }

	// main board -> card
	uint8_t io_r(offs_t offset) { return m_card ? m_card->io_r(offset) : 0xff; }
	void io_w(offs_t offset, uint8_t data) { if (m_card) m_card->io_w(offset, data); }
	void drvsel_w(uint8_t data) { if (m_card) m_card->drvsel_w(data); }
	void density_w(int fm) { if (m_card) m_card->density_w(fm); }
	void box_select_w(uint8_t data) { if (m_card) m_card->box_select_w(data); }
	bool media_8inch() { return m_card && m_card->media_8inch(); }
	bool media_twosided() { return m_card && m_card->media_twosided(); }
	uint8_t personality() { return m_card ? m_card->personality() : 0; }

	// card -> main board
	void intrq_w(int state) { m_intrq_cb(state); }
	void drq_w(int state) { m_drq_cb(state); }
	void int_w(int state) { m_int_cb(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_state() override { return m_card ? m_card->z80daisy_irq_state() : 0; }
	virtual int z80daisy_irq_ack() override { return m_card ? m_card->z80daisy_irq_ack() : 0; }
	virtual void z80daisy_irq_reti() override { if (m_card) m_card->z80daisy_irq_reti(); }

private:
	devcb_write_line m_intrq_cb;
	devcb_write_line m_drq_cb;
	devcb_write_line m_int_cb;

	device_xerox820_dbslot_card_interface *m_card;
};


// device type declaration
DECLARE_DEVICE_TYPE(XEROX820_DBSLOT, xerox820_dbslot_device)


// slot option registration
void xerox820_dbslot_cards(device_slot_interface &device);

#endif // MAME_BUS_XEROX820_DBSLOT_H
