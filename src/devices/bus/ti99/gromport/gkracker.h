// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
    GRAM Kracker.
*****************************************************************************/

#ifndef MAME_BUS_TI99_GROMPORT_GKRACKER_H
#define MAME_BUS_TI99_GROMPORT_GKRACKER_H

#pragma once
#include "cartridges.h"

namespace bus::ti99::gromport {

class ti99_gkracker_device : public cartridge_connector_device, public device_nvram_interface
{
public:
	ti99_gkracker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;
	void romgq_line(int state) override;

	void set_gromlines(line_state mline, line_state moline, line_state gsq) override;
	void gclock_in(int state) override;

	DECLARE_INPUT_CHANGED_MEMBER( gk_changed );

	// We may have a cartridge plugged into the GK
	bool is_grom_idle() override;

protected:
	void device_start() override;
	void device_reset() override;

	void device_add_mconfig(machine_config &config) override;
	const tiny_rom_entry* device_rom_region() const override;
	ioport_constructor device_input_ports() const override;

	// device_nvram_interface
	void nvram_default() override;
	bool nvram_read(util::read_stream &file) override;
	bool nvram_write(util::write_stream &file) override;

private:
	int     m_gk_switch[6];         // Used to cache the switch settings.

	bool    m_romspace_selected;
	int     m_ram_page;
	int     m_grom_address;
	uint8_t*  m_ram_ptr;
	uint8_t*  m_grom_ptr;

	bool    m_waddr_LSB;

	required_device<ti99_cartridge_device> m_cartridge;

	// Just for proper initialization
	void gk_install_menu(const char* menutext, int len, int ptr, int next, int start);
};


} // end namespace bus::ti99::gromport

DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT_GK, bus::ti99::gromport, ti99_gkracker_device)

#endif // MAME_BUS_TI99_GROMPORT_GKRACKER_H
