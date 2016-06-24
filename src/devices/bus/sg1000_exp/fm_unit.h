// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega FM Sound Unit emulation

**********************************************************************/

#pragma once

#ifndef __SEGA_FM_UNIT__
#define __SEGA_FM_UNIT__


#include "emu.h"
#include "sound/2413intf.h"
#include "sg1000exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sega_fm_unit_device

class sega_fm_unit_device : public device_t,
							public device_sg1000_expansion_slot_interface
{
public:
	// construction/destruction
	sega_fm_unit_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_sg1000_expansion_slot_interface overrides
	virtual DECLARE_READ8_MEMBER(peripheral_r) override;
	virtual DECLARE_WRITE8_MEMBER(peripheral_w) override;
	virtual bool is_readable(UINT8 offset) override;
	virtual bool is_writeable(UINT8 offset) override;

private:
	required_device<ym2413_device> m_ym;
	UINT8 m_audio_control;
};


// device type definition
extern const device_type SEGA_FM_UNIT;


#endif
