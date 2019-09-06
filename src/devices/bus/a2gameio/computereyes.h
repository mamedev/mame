// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    Digital Vision ComputerEyes (original gameport version)

*********************************************************************/

#ifndef MAME_BUS_A2GAMEIO_COMPEYES_H
#define MAME_BUS_A2GAMEIO_COMPEYES_H 1

#pragma once

#include "bus/a2gameio/gameio.h"
#include "bitmap.h"
#include "imagedev/picture.h"

// ======================> apple2_compeyes_device

class apple2_compeyes_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_compeyes_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_a2gameio_interface overrides
	virtual DECLARE_READ_LINE_MEMBER(sw0_r) override;
	virtual DECLARE_READ_LINE_MEMBER(sw1_r) override;
	virtual DECLARE_WRITE_LINE_MEMBER(an0_w) override;
	virtual DECLARE_WRITE_LINE_MEMBER(an1_w) override;
	virtual DECLARE_WRITE_LINE_MEMBER(an2_w) override;
	virtual DECLARE_WRITE_LINE_MEMBER(an3_w) override;

private:
	required_device<picture_image_device> m_picture;
	int m_x, m_y, m_an1, m_an2, m_an3, m_level;
	u8 m_a2_bitmap[280*192];
	bitmap_argb32 *m_bitmap;
};

// device type declaration
DECLARE_DEVICE_TYPE(APPLE2_COMPUTEREYES, apple2_compeyes_device)

#endif // MAME_BUS_A2GAMEIO_COMPEYES_H
