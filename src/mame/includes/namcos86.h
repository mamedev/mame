// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/watchdog.h"
#include "sound/namco.h"

class namcos86_state : public driver_device
{
public:
	namcos86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu1(*this, "cpu1")
		, m_cpu2(*this, "cpu2")
		, m_watchdog(*this, "watchdog")
		, m_cus30(*this, "namco")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_rthunder_videoram1(*this, "videoram1")
		, m_rthunder_videoram2(*this, "videoram2")
		, m_rthunder_spriteram(*this, "spriteram")
		, m_user1_ptr(*this, "user1")
	{
	}

	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_rthunder_videoram1;
	required_shared_ptr<uint8_t> m_rthunder_videoram2;
	required_shared_ptr<uint8_t> m_rthunder_spriteram;
	optional_region_ptr<uint8_t> m_user1_ptr;

	uint8_t *m_spriteram;
	int m_wdog;
	int m_tilebank;
	int m_xscroll[4];
	int m_yscroll[4];
	tilemap_t *m_bg_tilemap[4];
	int m_backcolor;
	const uint8_t *m_tile_address_prom;
	int m_copy_sprites;

	void bankswitch1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch1_ext_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dsw0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void int_ack1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void int_ack2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void watchdog1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void watchdog2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cus115_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t readFF(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void videoram1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tilebank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void backcolor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_namco86();
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_namcos86(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void scroll_w(address_space &space, int offset, int data, int layer);

private:
	inline void get_tile_info(tile_data &tileinfo,int tile_index,int layer,uint8_t *vram);
	void set_scroll(int layer);
};
