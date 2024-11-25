// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SNUG HSGPL card emulation.
    See hsgpl.c for documentation

    Original code by Raphael Nabet, 2003.

    Michael Zapf
    October 2010: Rewritten as device
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_HSGPL_H
#define MAME_BUS_TI99_PEB_HSGPL_H

#pragma once

#include "peribox.h"
#include "machine/at29x.h"
#include "machine/ram.h"

namespace bus::ti99::peb {

class snug_high_speed_gpl_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	snug_high_speed_gpl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<at29c040a_device> m_dsr_eeprom;
	required_device<at29c040a_device> m_rom6_eeprom;
	required_device<at29c040a_device> m_grom_a_eeprom;
	required_device<at29c040a_device> m_grom_b_eeprom;

	required_device<ram_device>      m_ram6_memory;
	required_device<ram_device>      m_gram_memory;

	void            dsrspace_readz(offs_t offset, uint8_t* value);
	void            cartspace_readz(offs_t offset, uint8_t* value);
	void            grom_readz(offs_t offset, uint8_t* value);

	void            cartspace_write(offs_t offset, uint8_t data);
	void            grom_write(offs_t offset, uint8_t data);

	bool            m_dsr_enabled;
	bool            m_gram_enabled;
	bool            m_bank_inhibit;
	int             m_dsr_page;
	bool            m_card_enabled;
	bool            m_write_enabled;
	bool            m_supercart_enabled;
	bool            m_led_on;
	bool            m_mbx_enabled;
	bool            m_ram_enabled;
	bool            m_flash_mode;

	int             m_current_grom_port;
	int             m_current_bank;

	int             m_module_bank;

	// GROM emulation
	bool            m_waddr_LSB;
	bool            m_raddr_LSB;
	int             m_grom_address;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_HSGPL, bus::ti99::peb, snug_high_speed_gpl_device)

#endif // MAME_BUS_TI99_PEB_HSGPL_H
