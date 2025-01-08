// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb
#ifndef MAME_BUS_VSMILE_MAT_H
#define MAME_BUS_VSMILE_MAT_H

#pragma once

#include "pad.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> vsmile_mat_device

class vsmile_mat_device : public vsmile_pad_device
{
public:
	// construction/destruction
	vsmile_mat_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~vsmile_mat_device();

	// input handlers
	DECLARE_INPUT_CHANGED_MEMBER(mat_joy_changed);
	DECLARE_INPUT_CHANGED_MEMBER(mat_color_changed);
	DECLARE_INPUT_CHANGED_MEMBER(mat_button_changed);

	enum stale_mat_inputs : uint16_t
	{
		STALE_NONE          = 0U,
		STALE_YELLOW_RIGHT  = 1U << 0,
		STALE_RED_LEFT      = 1U << 1,
		STALE_CENTER        = 1U << 2,
		STALE_UP            = 1U << 3,
		STALE_DOWN          = 1U << 4,
		STALE_GREEN         = 1U << 5,
		STALE_OK            = 1U << 6,
		STALE_QUIT          = 1U << 7,
		STALE_HELP          = 1U << 8,
		STALE_BLUE          = 1U << 9,

		STALE_JOY           = STALE_YELLOW_RIGHT | STALE_RED_LEFT,
		STALE_COLORS        = STALE_CENTER | STALE_UP | STALE_DOWN | STALE_GREEN,
		STALE_BUTTONS       = STALE_OK | STALE_QUIT | STALE_HELP | STALE_BLUE,
		STALE_ALL           = STALE_JOY | STALE_COLORS | STALE_BUTTONS
	};

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// vsmile_pad_device implementation
	virtual void tx_complete() override;

	uint16_t stale_all() { return STALE_ALL; }

private:
	required_ioport m_io_joy;
	required_ioport m_io_colors;
	required_ioport m_io_buttons;
};


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(VSMILE_MAT, vsmile_mat_device)

#endif // MAME_BUS_VSMILE_MAT_H
