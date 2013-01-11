/****************************************************************************

    TI-99 32K memory expansion
    See ti32kmem.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TI32K__
#define __TI32K__

#include "ti99defs.h"

extern const device_type TI_32KMEM;

class ti_32k_expcard_device : public ti_expansion_card_device
{
public:
	ti_32k_expcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	void crureadz(offs_t offset, UINT8 *value);
	void cruwrite(offs_t offset, UINT8 value);

protected:
	void device_start();
	const rom_entry *device_rom_region() const;
private:
	UINT8*  m_ram_ptr;
};

#endif
