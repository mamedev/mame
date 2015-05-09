// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Geneve "Memex" memory expansion
    See memex.c for documentation

    Michael Zapf, February 2011
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __MEMEXMEM__
#define __MEMEXMEM__

#include "emu.h"
#include "peribox.h"

extern const device_type TI99_MEMEX;

class geneve_memex_device : public ti_expansion_card_device
{
public:
	geneve_memex_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_READ8Z_MEMBER(crureadz) { };
	DECLARE_WRITE8_MEMBER(cruwrite) { };

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual ioport_constructor device_input_ports() const;
	virtual const rom_entry *device_rom_region() const;

private:
	bool    access_enabled(offs_t offset);

	UINT8*  m_ram;
	UINT8   m_dip_switch[8];
	int     m_genmod;
};

#endif
