// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A800_SPARTA_H
#define __A800_SPARTA_H

#include "rom.h"


// ======================> a800_rom_spartados_device

class a800_rom_spartados_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_spartados_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx);
	virtual DECLARE_WRITE8_MEMBER(write_d5xx);

protected:
	int m_bank, m_subslot_enabled;
};



// device type definition
extern const device_type A800_ROM_SPARTADOS;


#endif
