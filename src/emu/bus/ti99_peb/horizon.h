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

extern const device_type TI99_HORIZON;

class horizon_ramdisk_device : public ti_expansion_card_device, public device_nvram_interface
{
public:
	horizon_ramdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_INPUT_CHANGED_MEMBER( ks_changed );

protected:
	virtual void device_start();
	virtual void device_reset();

	virtual const rom_entry *device_rom_region() const;
	virtual ioport_constructor device_input_ports() const;

	void nvram_default();
	void nvram_read(emu_file &file);
	void nvram_write(emu_file &file);

private:
	void    setbit(int& page, int pattern, bool set);
	int     get_size();
	UINT8*  m_ram;
	UINT8*  m_nvram;
	UINT8*  m_ros;

	int     m_select6_value;
	int     m_select_all;

	int     m_page;

	int     m_cru_horizon;
	int     m_cru_phoenix;
	bool    m_timode;
	bool    m_32k_installed;
	bool    m_split_mode;
	bool    m_rambo_mode;
	bool    m_killswitch;
	bool    m_use_rambo;
};

#endif
