// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef CEDAR_MAGNET_PLANE_DEF
#define CEDAR_MAGNET_PLANE_DEF

#include "cedar_magnet_board.h"

extern const device_type CEDAR_MAGNET_PLANE;

#define MCFG_CEDAR_MAGNET_PLANE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CEDAR_MAGNET_PLANE, 0)


class cedar_magnet_plane_device :  public cedar_magnet_board_device
{
public:
	// construction/destruction
	cedar_magnet_plane_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	DECLARE_READ8_MEMBER(pio0_pa_r);
	DECLARE_WRITE8_MEMBER(pio0_pa_w);
//	DECLARE_READ8_MEMBER(pio0_pb_r);
	DECLARE_WRITE8_MEMBER(pio0_pb_w);

//	DECLARE_READ8_MEMBER(pio1_pa_r);
	DECLARE_WRITE8_MEMBER(pio1_pa_w);
//	DECLARE_READ8_MEMBER(pio1_pb_r);
	DECLARE_WRITE8_MEMBER(pio1_pb_w);

	DECLARE_WRITE8_MEMBER(plane_portcc_w);
	DECLARE_WRITE8_MEMBER(plane_portcd_w);
	DECLARE_WRITE8_MEMBER(plane_portce_w);
	DECLARE_WRITE8_MEMBER(plane_portcf_w);

	UINT8 m_framebuffer[0x10000];
	int m_curline;
	int m_lineoffset;

	UINT8 m_pio0_pa_data;
	UINT8 m_pio0_pb_data;
	UINT8 m_scrollx;
	UINT8 m_scrolly;
	int m_direction;

	UINT8 m_cd_data;
	UINT8 m_cf_data;
	UINT32 draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palbase);
protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//	virtual void device_reset() override;

private:
};

#endif
