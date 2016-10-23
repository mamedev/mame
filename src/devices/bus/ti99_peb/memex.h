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
#include "machine/ram.h"

extern const device_type TI99_MEMEX;

class geneve_memex_device : public ti_expansion_card_device
{
public:
	geneve_memex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	DECLARE_READ8Z_MEMBER(crureadz) override { };
	void cruwrite(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { };

protected:
	void device_start() override;
	void device_reset() override;
	machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	bool    access_enabled(offs_t offset);
	required_device<ram_device> m_ram;
	uint8_t   m_switches;
};

#endif
