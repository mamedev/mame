// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_CEDAR_MAGNET_PLANE_H
#define MAME_MACHINE_CEDAR_MAGNET_PLANE_H

#pragma once


#include "machine/cedar_magnet_board.h"
#include "machine/z80pio.h"

extern const device_type CEDAR_MAGNET_PLANE;
DECLARE_DEVICE_TYPE(CEDAR_MAGNET_PLANE, cedar_magnet_plane_device)

#define MCFG_CEDAR_MAGNET_PLANE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CEDAR_MAGNET_PLANE, 0)


class cedar_magnet_plane_device : public device_t, public cedar_magnet_board_interface
{
public:
	// construction/destruction
	cedar_magnet_plane_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(pio0_pa_r);
	DECLARE_WRITE8_MEMBER(pio0_pa_w);
//  DECLARE_READ8_MEMBER(pio0_pb_r);
	DECLARE_WRITE8_MEMBER(pio0_pb_w);

//  DECLARE_READ8_MEMBER(pio1_pa_r);
	DECLARE_WRITE8_MEMBER(pio1_pa_w);
//  DECLARE_READ8_MEMBER(pio1_pb_r);
	DECLARE_WRITE8_MEMBER(pio1_pb_w);

	DECLARE_WRITE8_MEMBER(plane_portcc_w);
	DECLARE_WRITE8_MEMBER(plane_portcd_w);
	DECLARE_WRITE8_MEMBER(plane_portce_w);
	DECLARE_WRITE8_MEMBER(plane_portcf_w);

	INTERRUPT_GEN_MEMBER(vblank_irq);

	uint32_t draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palbase);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

private:
	uint8_t m_framebuffer[0x10000];
	int m_curline;
	int m_lineoffset;

	uint8_t m_pio0_pa_data;
	uint8_t m_pio0_pb_data;
	uint8_t m_scrollx;
	uint8_t m_scrolly;
	int m_direction;

	uint8_t m_cd_data;
	uint8_t m_cf_data;
};

#endif // MAME_MACHINE_CEDAR_MAGNET_PLANE_H
