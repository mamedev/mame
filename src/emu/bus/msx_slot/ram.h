#ifndef __MSX_SLOT_RAM_H
#define __MSX_SLOT_RAM_H

#include "slot.h"

#define MCFG_MSX_SLOT_RAM_ADD(_tag, _startpage, _numpages) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_RAM, _startpage, _numpages) \

class msx_slot_ram_device : public device_t,
							public msx_internal_slot_interface
{
public:
	msx_slot_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start();

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

private:
	dynamic_array<UINT8> m_ram;
};

extern const device_type MSX_SLOT_RAM;

#endif
