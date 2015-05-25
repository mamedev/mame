// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VCS_SCHARGER_H
#define __VCS_SCHARGER_H

#include "rom.h"
#include "imagedev/cassette.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a26_rom_ss_device

class a26_rom_ss_device : public a26_rom_f6_device
{
public:
	// construction/destruction
	a26_rom_ss_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_reset();

	required_device<cassette_image_device> m_cassette;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);

private:
	cpu_device   *m_maincpu;
	inline UINT8 read_byte(UINT32 offset);

	int m_base_banks[2];
	UINT8 m_reg;
	UINT8 m_write_delay, m_ram_write_enabled, m_rom_enabled;
	UINT32 m_byte_started;
	UINT16 m_last_address;
	UINT32 m_diff_adjust;
};


// device type definition
extern const device_type A26_ROM_SUPERCHARGER;

#endif
