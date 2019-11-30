// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    computereyes2.h

    Implemention of the Digital Vision ComputerEyes/2 card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_COMPEYES2_H
#define MAME_BUS_A2BUS_COMPEYES2_H

#pragma once

#include "a2bus.h"
#include "bitmap.h"
#include "imagedev/picture.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_computereyes2_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_computereyes2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_computereyes2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:
	required_device<picture_image_device> m_picture;
	int m_x, m_y, m_cer0, m_cer1, m_cer2;
	u8 m_a2_bitmap[280*193];
	u8 m_threshold;
	bool m_bActive;
	bitmap_argb32 *m_bitmap;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_COMPUTEREYES2, a2bus_computereyes2_device)

#endif // MAME_BUS_A2BUS_COMPEYES2_H
