// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
    GRAM Kracker.
*****************************************************************************/

#ifndef MAME_BUS_TI99_GROMPORT_GKRACKER_H
#define MAME_BUS_TI99_GROMPORT_GKRACKER_H

#pragma once
#include "bus/ti99/ti99defs.h"
#include "cartridges.h"

namespace bus { namespace ti99 { namespace gromport {

class ti99_gkracker_device : public cartridge_connector_device, public device_nvram_interface
{
public:
	ti99_gkracker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
	DECLARE_WRITE_LINE_MEMBER(romgq_line) override;

	void set_gromlines(line_state mline, line_state moline, line_state gsq) override;
	DECLARE_WRITE_LINE_MEMBER(gclock_in) override;

	void insert(int index, ti99_cartridge_device* cart) override;
	void remove(int index) override;
	DECLARE_INPUT_CHANGED_MEMBER( gk_changed );

	// We may have a cartridge plugged into the GK
	bool is_grom_idle() override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry* device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	// device_nvram_interface
	void nvram_default() override;
	void nvram_read(emu_file &file) override;
	void nvram_write(emu_file &file) override;

private:
	int     m_gk_switch[6];         // Used to cache the switch settings.

	bool    m_romspace_selected;
	int     m_ram_page;
	int     m_grom_address;
	uint8_t*  m_ram_ptr;
	uint8_t*  m_grom_ptr;

	bool    m_waddr_LSB;

	ti99_cartridge_device *m_cartridge;     // guest cartridge

	// Just for proper initialization
	void gk_install_menu(const char* menutext, int len, int ptr, int next, int start);
};


} } } // end namespace bus::ti99::gromport

DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT_GK, bus::ti99::gromport, ti99_gkracker_device)

#endif // MAME_BUS_TI99_GROMPORT_GKRACKER_H
