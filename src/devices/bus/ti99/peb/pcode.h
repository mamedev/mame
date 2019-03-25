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

namespace bus { namespace ti99 { namespace peb {

class ti_pcode_card_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	ti_pcode_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_WRITE_LINE_MEMBER(clock_in) override;

	DECLARE_INPUT_CHANGED_MEMBER( switch_changed );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_config_complete() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	DECLARE_WRITE_LINE_MEMBER(ready_line);

	DECLARE_WRITE_LINE_MEMBER(pcpage_w);
	DECLARE_WRITE_LINE_MEMBER(ekrpg_w);

	void debugger_read(address_space& space, uint16_t addr, uint8_t& value);

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

} } } // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_P_CODE,  bus::ti99::peb, ti_pcode_card_device)

#endif // MAME_BUS_TI99_PEB_PCODE_H
