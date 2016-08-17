// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Horizon Ramdisk
    See horizon.c for documentation

    Michael Zapf

    February 2012

*****************************************************************************/

#ifndef __HORIZON__
#define __HORIZON__

#include "emu.h"
#include "peribox.h"
#include "machine/ram.h"

extern const device_type TI99_HORIZON;

class horizon_ramdisk_device : public ti_expansion_card_device, public device_nvram_interface
{
public:
	horizon_ramdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;

	DECLARE_INPUT_CHANGED_MEMBER( hs_changed );

protected:
	void device_start() override;
	void device_reset() override;

	machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void nvram_default() override;
	void nvram_read(emu_file &file) override;
	void nvram_write(emu_file &file) override;

private:
	void    setbit(int& page, int pattern, bool set);

	required_device<ram_device> m_ram;
	required_device<ram_device> m_nvram;
	required_device<ram_device> m_ros;

	int     m_select6_value;
	int     m_select_all;

	int     m_page;

	int     m_cru_horizon;
	int     m_cru_phoenix;
	bool    m_timode;
	bool    m_32k_installed;
	bool    m_split_mode;
	bool    m_rambo_mode;
	bool    m_hideswitch;
	bool    m_use_rambo;
};

#endif
