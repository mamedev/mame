// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Barry Rodewald
/*************************************************************************

    Malzak

*************************************************************************/

#include "machine/s2636.h"
#include "video/saa5050.h"

class malzak_state : public driver_device
{
public:
	malzak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_s2636_0(*this, "s2636_0"),
			m_s2636_1(*this, "s2636_1"),
			m_trom(*this, "saa5050"),
			m_videoram(*this, "videoram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<s2636_device> m_s2636_0;
	required_device<s2636_device> m_s2636_1;
	required_device<saa5050_device> m_trom;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	/* misc */
//  int playfield_x[256];
//  int playfield_y[256];
	int m_playfield_code[256];
	int m_malzak_x;
	int m_malzak_y;
	int m_collision_counter;

	/* devices */
	device_t *m_saa5050;
	uint8_t fake_VRLE_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t s2636_portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t s2650_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port40_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port60_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portc0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t collision_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void malzak_playfield_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette_init_malzak(palette_device &palette);
	uint32_t screen_update_malzak(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
