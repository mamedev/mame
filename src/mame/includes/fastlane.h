// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Fast Lane

*************************************************************************/
#include "sound/k007232.h"
#include "video/k007121.h"
#include "video/k051733.h"

class fastlane_state : public driver_device
{
public:
	fastlane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_k007121_regs(*this, "k007121_regs"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k007121(*this, "k007121"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_k007121_regs;
	required_shared_ptr<uint8_t> m_videoram1;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_layer0;
	tilemap_t    *m_layer1;
	rectangle  m_clip0;
	rectangle  m_clip1;

	/* devices */
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k007121_device> m_k007121;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void k007121_registers_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastlane_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastlane_vram1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastlane_vram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fastlane_k1_k007232_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fastlane_k1_k007232_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fastlane_k2_k007232_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fastlane_k2_k007232_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_fastlane(palette_device &palette);
	uint32_t screen_update_fastlane(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fastlane_scanline(timer_device &timer, void *ptr, int32_t param);
	void volume_callback0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void volume_callback1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};
