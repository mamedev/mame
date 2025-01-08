// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 P-Code Card emulation.
    See p_code.c for documentation

    Michael Zapf
    July 2009
    Revised July 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_PCODE_H
#define MAME_BUS_TI99_PEB_PCODE_H

#pragma once

#include "peribox.h"
#include "machine/74259.h"
#include "machine/tmc0430.h"

namespace bus::ti99::peb {

class ti_pcode_card_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	ti_pcode_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void clock_in(int state) override;

	DECLARE_INPUT_CHANGED_MEMBER( switch_changed );

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void ready_line(int state);

	void pcpage_w(int state);
	void ekrpg_w(int state);

	void debugger_read(offs_t addr, uint8_t& value);

	required_device_array<tmc0430_device, 8> m_groms;

	required_device<ls259_device> m_crulatch;

	uint8_t* m_rom;
	int      m_bank_select;
	bool     m_active;
	int      m_clock_count;
	bool     m_clockhigh;

	// Address in card area
	bool    m_inDsrArea;
	bool    m_isrom0;
	bool    m_isrom12;
	bool    m_isgrom;

	// Recent address
	int m_address;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_P_CODE,  bus::ti99::peb, ti_pcode_card_device)

#endif // MAME_BUS_TI99_PEB_PCODE_H
