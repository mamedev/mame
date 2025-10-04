// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Mono Covox interface

***************************************************************************/

#ifndef MAME_BUS_BK_COVOX_H
#define MAME_BUS_BK_COVOX_H

#pragma once

#include "parallel.h"

#include "sound/dac.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_covox_device

class bk_covox_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_covox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override { m_data = 0; };

	virtual uint16_t io_r() override;
	virtual void io_w(uint16_t data, bool word) override;

private:
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<dac_byte_interface> m_dac;
	required_device<bk_parallel_slot_device> m_up;

	uint16_t m_data;
};

// device type definition
DECLARE_DEVICE_TYPE(BK_COVOX, bk_covox_device)

#endif // MAME_BUS_BK_COVOX_H
