// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Blue Alpha Sound Sampler for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_BLUE_SAMPLER_H
#define MAME_BUS_SAMCOUPE_EXPANSION_BLUE_SAMPLER_H

#pragma once

#include "expansion.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/dac.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_blue_sound_sampler_device

class sam_blue_sound_sampler_device : public device_t, public device_samcoupe_expansion_interface
{
public:
	// construction/destruction
	sam_blue_sound_sampler_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(clock_callback);
	void ppi_portb_w(uint8_t data);
	uint8_t ppi_portc_r();

	required_device<i8255_device> m_ppi;
	required_device<zn426e_device> m_dac;

	uint8_t m_portc;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_BLUE_SOUND_SAMPLER, sam_blue_sound_sampler_device)

#endif // MAME_BUS_SAMCOUPE_EXPANSION_BLUE_SAMPLER_H
