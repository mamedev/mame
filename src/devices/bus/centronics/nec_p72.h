// license:BSD-3-Clause
// copyright-holders:Ramiro Polla

#pragma once

#ifndef __NEC_P72__
#define __NEC_P72__

#include "emu.h"
#include "ctronics.h"
#include "cpu/nec/nec.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nec_p72_t

class nec_p72_t : public device_t,
					public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	nec_p72_t(const machine_config &mconfig, const char *tag,
				device_t *owner, UINT32 clock);
	nec_p72_t(const machine_config &mconfig, device_type type,
				const char *name, const char *tag, device_t *owner,
				UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();

private:
	required_device<cpu_device> m_maincpu;
};

// device type definition
extern const device_type NEC_P72;

#endif
