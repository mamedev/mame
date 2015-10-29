// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef __VCS_HARMONY_H
#define __VCS_HARMONY_H

#include "rom.h"
#include "cpu/arm7/lpc210x.h"


// ======================> a26_rom_harmony_device

class a26_rom_harmony_device : public a26_rom_f8_device
{
public:
	// construction/destruction
	a26_rom_harmony_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);

	DECLARE_READ32_MEMBER(armrom_r);
	DECLARE_WRITE32_MEMBER(armrom_w);
	
	DECLARE_READ8_MEMBER(read8_r);

	DECLARE_READ32_MEMBER(arm_E01FC088_r);

	void check_bankswitch(offs_t offset);

protected:
};


// device type definition
extern const device_type A26_ROM_HARMONY;

#endif
