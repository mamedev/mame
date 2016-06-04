// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Myarc memory expansion
    See myarcmem.c for documentation

    Michael Zapf, September 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __MYARCMEM__
#define __MYARCMEM__

#include "emu.h"
#include "peribox.h"
#include "machine/ram.h"

extern const device_type TI99_MYARCMEM;

class myarc_memory_expansion_device : public ti_expansion_card_device
{
public:
	myarc_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;

protected:
	void device_start(void) override;
	void device_reset(void) override;
	const rom_entry *device_rom_region(void) const override;
	ioport_constructor device_input_ports() const override;
	machine_config_constructor device_mconfig_additions() const override;

private:
	int     get_base(int offset);
	required_device<ram_device> m_ram;
	UINT8*  m_dsrrom;
	int     m_bank;
	int     m_size;
};

#endif
