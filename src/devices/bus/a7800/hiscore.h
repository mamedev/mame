// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A78_HISCORE_H
#define __A78_HISCORE_H

#include "a78_slot.h"
#include "rom.h"


// ======================> a78_hiscore_device

class a78_hiscore_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_hiscore_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_04xx);
	virtual DECLARE_READ8_MEMBER(read_10xx);
	virtual DECLARE_WRITE8_MEMBER(write_10xx);
	virtual DECLARE_READ8_MEMBER(read_30xx);
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	required_device<a78_cart_slot_device> m_hscslot;
};



// device type definition
extern const device_type A78_HISCORE;


#endif
