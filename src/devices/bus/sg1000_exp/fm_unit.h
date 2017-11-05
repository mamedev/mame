// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega FM Sound Unit emulation

**********************************************************************/

#ifndef MAME_BUS_SG1000_EXP_FM_UNIT_H
#define MAME_BUS_SG1000_EXP_FM_UNIT_H

#pragma once


#include "sound/ym2413.h"
#include "sound/sn76496.h"
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
	sega_fm_unit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_sg1000_expansion_slot_interface overrides
	virtual DECLARE_READ8_MEMBER(peripheral_r) override;
	virtual DECLARE_WRITE8_MEMBER(peripheral_w) override;
	virtual bool is_readable(uint8_t offset) override;
	virtual bool is_writeable(uint8_t offset) override;
	void set_audio_control(uint8_t data);

private:
	required_device<ym2413_device> m_ym;
	optional_device<segapsg_device> m_psg;
	uint8_t m_audio_control;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA_FM_UNIT, sega_fm_unit_device)

#endif // MAME_BUS_SG1000_EXP_FM_UNIT_H
