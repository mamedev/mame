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
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override { };
	DECLARE_WRITE8_MEMBER(cruwrite) override { };

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const rom_entry *device_rom_region() const override;

private:
	bool    access_enabled(offs_t offset);

	UINT8*  m_ram;
	UINT8   m_dip_switch[8];
	int     m_genmod;
};

#endif
