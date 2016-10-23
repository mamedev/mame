// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/galeb.h
 *
 ****************************************************************************/

#ifndef GALEB_H_
#define GALEB_H_

#include "sound/dac.h"

class galeb_state : public driver_device
{
public:
	galeb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_keyboard(*this, "LINE%u", 0),
		m_dac(*this, "dac"),
		m_dac_state(0)
	{
	}

	required_shared_ptr<uint8_t> m_video_ram;
	void dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tape_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tape_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tape_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void video_start() override;
	uint32_t screen_update_galeb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_ioport_array<8> m_keyboard;
	required_device<dac_1bit_device> m_dac;

protected:
	virtual void machine_start() override;

private:
	int m_dac_state;
};

/*----------- defined in video/galeb.c -----------*/

extern const gfx_layout galeb_charlayout;

#endif /* GALEB_H_ */
