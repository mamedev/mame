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
	cedar_magnet_plane_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t pio0_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio0_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
//  uint8_t pio0_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio0_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

//  uint8_t pio1_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio1_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
//  uint8_t pio1_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio1_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void plane_portcc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void plane_portcd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void plane_portce_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void plane_portcf_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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
	uint32_t draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palbase);
protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset() override;

private:
};

#endif
