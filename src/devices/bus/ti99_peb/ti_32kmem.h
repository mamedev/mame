// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 32K memory expansion
    See ti32kmem.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TI32K__
#define __TI32K__

#include "emu.h"
#include "peribox.h"
#include "machine/ram.h"

extern const device_type TI_32KMEM;

class ti_32k_expcard_device : public ti_expansion_card_device
{
public:
	ti_32k_expcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override { };
	DECLARE_WRITE8_MEMBER(cruwrite) override { };

protected:
	void device_start() override;
	machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<ram_device> m_ram;
};

#endif
