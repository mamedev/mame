// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SCSI adapter card
    See scsicard.cpp for documentation

    Michael Zapf
    November 2021

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_SCSI_H
#define MAME_BUS_TI99_PEB_SCSI_H

#pragma once

#include "peribox.h"
#include "machine/ram.h"
#include "machine/ncr5380.h"

namespace bus::ti99::peb {
class whtscsi_pld_device;

class whtech_scsi_card_device : public device_t, public device_ti99_peribox_card_interface
{
	friend class whtscsi_pld_device;

public:
	whtech_scsi_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	void drq_w(int state);
	void irq_w(int state);

	void debug_read(offs_t offset, uint8_t* value);
	void debug_write(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// SCSI card on-board SRAM (32K)
	required_device<ram_device> m_buffer_ram;

	// Decoder PLD
	required_device<whtscsi_pld_device> m_pld;

	// SCSI controller chip
	required_device<ncr53c80_device> m_controller;

	// SCSI bus
	required_device<nscsi_bus_device> m_scsibus;

	// DSR ROM
	uint8_t* m_eprom;

	// Recent address
	int m_address;

	// Settings
	int m_sw2;

	// Debugging
	int m_dmacount;

	// Latches for the lines
	// Should be removed and accessor functions be added to ncr5380
	bool m_irq;
	bool m_drq;

	// State of the READY line
	bool m_readyset;

	// Accessor functions for the PLD
	int get_sw1();
	int get_sw2();

	// Determine the state of the READY line
	void operate_ready_line();

	// Send the EOP signal to the controller
	void signal_scsi_eop(int state);
};

class whtscsi_pld_device : public device_t
{
	friend class whtech_scsi_card_device;

public:
	whtscsi_pld_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void crureadz(offs_t offset, uint8_t *value);
	void cruwrite(offs_t offset, uint8_t data);

	bool card_selected();

	// Selector output lines of the PLD
	line_state sram_cs();
	line_state eprom_cs();
	line_state scsi_cs();

	int eprom_bank() { return m_eprom_bank; }
	int sram_bank() { return m_sram_bank; }

	bool ready();

	void update_line_states(int address, bool drq, bool irq);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	whtech_scsi_card_device* m_board;

	void set_board(whtech_scsi_card_device* board) { m_board = board; }
	bool busen();

	// Flags
	bool m_selected;
	bool m_sram_shadow;
	bool m_dma_lock;
	bool m_word_transfer;
	bool m_bank_swapped;
	bool m_readyout;
	int  m_eprom_bank;
	int  m_sram_bank;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_WHTSCSI, bus::ti99::peb, whtech_scsi_card_device)
DECLARE_DEVICE_TYPE_NS(WHTSCSI_PLD, bus::ti99::peb, whtscsi_pld_device)

#endif // MAME_BUS_TI99_PEB_SCSI_H
