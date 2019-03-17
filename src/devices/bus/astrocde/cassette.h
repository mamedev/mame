// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_ASTROCDE_CASSETTE_H
#define MAME_BUS_ASTROCDE_CASSETTE_H

#pragma once

#include "ctrl.h"
#include "imagedev/cassette.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> astrocade_cassette_device

class astrocade_cassette_device : public device_t,
                                  public device_astrocade_ctrl_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::TAPE; }

	// construction/destruction
	astrocade_cassette_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~astrocade_cassette_device();

	// device_astrocade_ctrl_interface implementation
	virtual uint8_t read_handle() override;
	virtual uint8_t read_knob() override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cassette_image_device> m_cassette;
};


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(ASTROCADE_CASSETTE, astrocade_cassette_device)

#endif // MAME_BUS_ASTROCDE_CASSETTE_H
