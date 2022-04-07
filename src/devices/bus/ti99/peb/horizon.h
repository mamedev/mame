// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Horizon Ramdisk
    See horizon.c for documentation

    Michael Zapf

    February 2012

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_HORIZON_H
#define MAME_BUS_TI99_PEB_HORIZON_H

#pragma once

#include "peribox.h"
#include "machine/ram.h"
#include "machine/74259.h"

namespace bus::ti99::peb {

class horizon_ramdisk_device : public device_t, public device_ti99_peribox_card_interface, public device_nvram_interface
{
public:
	horizon_ramdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	DECLARE_INPUT_CHANGED_MEMBER( hs_changed );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual bool nvram_can_write() override;

private:
	required_device<ram_device> m_ram;
	required_device<ram_device> m_dsrram;
	required_device<ram_device> m_optram;
	required_device<ls259_device> m_crulatch_u4;
	required_device<ls259_device> m_crulatch_u3;

	void    get_mem_size(int& ramsize, int& dsrsize);
	void    get_address_prefix();
	void    read_write(offs_t offset, uint8_t *value, bool write);

	bool    m_32k_installed;
	bool    m_phoenix_accessed;
	bool    m_dsr32k;
	bool    m_128kx8;
	bool    m_geneve_mode;
	bool    m_phoenix_split;
	bool    m_hideswitch;
	bool    m_rambo_supported;

	// Do not save if nothing was modified.
	bool    m_modified;

	int     m_page;
	int     m_bank;
	int     m_ramsize;

	int     m_cru_base_horizon;
	int     m_cru_base_phoenix;

	// Debugging
	int     m_current_bank;
	int     m_current_page;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_HORIZON, bus::ti99::peb, horizon_ramdisk_device)

#endif // MAME_BUS_TI99_PEB_HORIZON_H
