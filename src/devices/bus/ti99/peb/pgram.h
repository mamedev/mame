// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*******************************************************************************
    PGRAM(+) Memory expansion and GROM simulator

    The PGRAM card is a battery-buffered RAM card. It also contains a circuitry
    to simulate GROMs (TMC0430).

    Michael Zapf
    March 2020

*******************************************************************************/

#ifndef MAME_BUS_TI99_PEB_PGRAM_H
#define MAME_BUS_TI99_PEB_PGRAM_H

#pragma once

#include "peribox.h"
#include "machine/ram.h"
#include "machine/mm58167.h"
#include "machine/74259.h"
#include "machine/7474.h"
#include "machine/74161.h"
#include "bus/ti99/internal/buffram.h"

namespace bus::ti99::peb {

class pgram_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	pgram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void write(offs_t offset, uint8_t data) override;
	void readz(offs_t offset, uint8_t *value) override;
	void crureadz(offs_t offset, uint8_t *value) override { }
	void cruwrite(offs_t offset, uint8_t data) override;
	DECLARE_INPUT_CHANGED_MEMBER( sw1_changed );
	DECLARE_INPUT_CHANGED_MEMBER( sw2_changed );

private:
	void device_start() override;
	void device_reset() override;
	void device_add_mconfig(machine_config &config) override;

	// Settings
	bool m_active;
	uint16_t m_crubase;
	bool m_pgramplus;

	ioport_constructor device_input_ports() const override;

	required_device<bus::ti99::internal::buffered_ram_device> m_gram3;
	required_device<bus::ti99::internal::buffered_ram_device> m_gram4567;
	required_device<bus::ti99::internal::buffered_ram_device> m_dsrram;
	required_device<mm58167_device> m_clock;
	required_device<ls259_device> m_crulatch;
	required_device<ttl7474_device> m_bankff;

	// GROM address counter
	required_device<ttl74161_device> m_count0;
	required_device<ttl74161_device> m_count1;
	required_device<ttl74161_device> m_count2;
	required_device<ttl74161_device> m_count3;

	// Methods
	void gram_read(offs_t offset, uint8_t *value);
	void dsr_ram_read(offs_t offset, uint8_t *value);

	void gram_write(offs_t offset, uint8_t data);
	void dsr_ram_write(offs_t offset, uint8_t data);

	void set_gram_address(uint8_t addrbyte);

	// GRAM implementation
	void clock_gram_counter(int state);
	void set_load_gram_counter(int state);
	uint16_t get_gram_address();

	// For debugging
	bool m_lowbyte;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_PGRAM, bus::ti99::peb, pgram_device)

#endif // MAME_BUS_TI99_PEB_PGRAM_H
