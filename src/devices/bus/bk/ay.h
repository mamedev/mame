// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    AY mod

***************************************************************************/

#ifndef MAME_BUS_BK_AY_H
#define MAME_BUS_BK_AY_H

#pragma once

#include "parallel.h"

#include "sound.h"
#include "sound/ay8910.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_ay_device

class bk_ay_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_ay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override {};
	virtual void device_reset() override {};

	virtual uint16_t io_r() override;
	virtual void io_w(uint16_t data, bool word) override;

private:
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<ym2149_device> m_ay;
	required_device<bk_parallel_slot_device> m_up;
};

// device type definition
DECLARE_DEVICE_TYPE(BK_AY, bk_ay_device)

#endif // MAME_BUS_BK_AY_H
