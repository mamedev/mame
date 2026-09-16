// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    IDE adapter card
    See tn_ide.cpp for documentation

    Michael Zapf
    April 2020

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_TN_IDE_H
#define MAME_BUS_TI99_PEB_TN_IDE_H

#pragma once

#include "peribox.h"
#include "bus/ata/ataintf.h"
#include "bus/ti99/internal/buffram.h"
#include "machine/rtc65271.h"
#include "machine/bq4847.h"
#include "machine/bq48x2.h"
#include "machine/74259.h"
#include "machine/74543.h"

namespace bus::ti99::peb {

class nouspikel_ide_card_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	nouspikel_ide_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;
	DECLARE_INPUT_CHANGED_MEMBER( mode_changed );

private:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override;
	ioport_constructor device_input_ports() const override;

	required_device<rtc65271_device> m_rtc65;
	required_device<bq4847_device> m_rtc47;
	required_device<bq4842_device> m_rtc42;
	required_device<bq4852_device> m_rtc52;

	required_device<ata_interface_device> m_ata;
	required_device<bus::ti99::internal::buffered_ram_device> m_sram;
	required_device<ls259_device> m_crulatch;
	required_device<ttl74543_device> m_latch0_7;
	required_device<ttl74543_device> m_latch8_15;

	// Latches the IDE interrupt
	bool m_ideint;

	// TI mode or Geneve mode (or off)
	int m_mode;

	// Stores the currently selected page (74ALS373)
	int m_page;

	// Is the SRAM buffered?
	bool m_srammap;

	// RTC type
	int m_rtctype;

	// Genmod decoding. If not used, the AME line is pulled up, and the AMD line is pulled down
	bool m_genmod;

	template<int rtctype> void rtc_int_callback(int state);
	void ide_interrupt_callback(int state);
	void resetdr_callback(int state);

	void decode(offs_t offset, bool& mmap, bool& sramsel, bool& xramsel, bool& rtcsel, bool& cs1fx, bool& cs3fx);

	int m_rtc_int;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_IDE, bus::ti99::peb, nouspikel_ide_card_device)

#endif // MAME_BUS_TI99_PEB_TN_IDE_H
