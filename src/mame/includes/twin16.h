// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "video/bufsprite.h"
#include "sound/upd7759.h"
#include "sound/k007232.h"

class twin16_state : public driver_device
{
public:
	twin16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232(*this, "k007232"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_gfxrombank(*this, "gfxrombank"),
		m_fixram(*this, "fixram"),
		m_videoram(*this, "videoram.%u", 0),
		m_zipram(*this, "zipram"),
		m_sprite_gfx_ram(*this, "sprite_gfx_ram"),
		m_gfxrom(*this, "gfxrom") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_memory_bank m_gfxrombank;
	required_shared_ptr<uint16_t> m_fixram;
	required_shared_ptr_array<uint16_t, 2> m_videoram;
	optional_shared_ptr<uint16_t> m_zipram;
	optional_shared_ptr<uint16_t> m_sprite_gfx_ram;
	required_region_ptr<uint16_t> m_gfxrom;

	uint16_t m_CPUA_register;
	uint16_t m_CPUB_register;
	bool m_is_fround;
	uint16_t m_sprite_buffer[0x800];
	emu_timer *m_sprite_timer;
	int m_sprite_busy;
	int m_need_process_spriteram;
	uint16_t m_scrollx[3];
	uint16_t m_scrolly[3];
	uint16_t m_video_register;
	tilemap_t *m_fixed_tmap;
	tilemap_t *m_scroll_tmap[2];

	void CPUA_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void CPUB_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t sprite_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void video_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fixram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void zipram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint8_t upd_busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void upd_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void upd_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_twin16();

	void fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void layer0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void layer1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update_twin16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_twin16(screen_device &screen, bool state);
	void CPUA_interrupt(device_t &device);
	void CPUB_interrupt(device_t &device);
	void sprite_tick(void *ptr, int32_t param);
	void volume_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void tile_get_info(tile_data &tileinfo, uint16_t data, int color_base);
private:
	int set_sprite_timer();
	void spriteram_process();
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap );
	int spriteram_process_enable();
	void twin16_postload();
};

class fround_state : public twin16_state
{
public:
	fround_state(const machine_config &mconfig, device_type type, const char *tag)
		: twin16_state(mconfig, type, tag)
	{}

	void fround_CPU_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gfx_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_fround();

protected:
	virtual void video_start() override;
	virtual void tile_get_info(tile_data &tileinfo, uint16_t data, int color_base) override;

private:
	uint8_t m_gfx_bank[4];
};

class cuebrickj_state : public twin16_state
{
public:
	cuebrickj_state(const machine_config &mconfig, device_type type, const char *tag)
		: twin16_state(mconfig, type, tag)
	{}

	void nvram_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_cuebrickj();

private:
	uint16_t m_nvram[0x400 * 0x20 / 2];
};
