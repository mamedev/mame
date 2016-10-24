// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Namco NA-1 System hardware

***************************************************************************/

#include "machine/eeprompar.h"
#include "sound/c140.h"

enum
{
	NAMCO_CGANGPZL,
	NAMCO_EMERALDA,
	NAMCO_KNCKHEAD,
	NAMCO_BKRTMAQ,
	NAMCO_EXBANIA,
	NAMCO_QUIZTOU,
	NAMCO_SWCOURT,
	NAMCO_TINKLPIT,
	NAMCO_NUMANATH,
	NAMCO_FA,
	NAMCO_XDAY2
};


class namcona1_state : public driver_device
{
public:
	namcona1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_c140(*this, "c140"),
		m_muxed_inputs(*this, { { "P4", "DSW", "P1", "P2" } }),
		m_io_p3(*this, "P3"),
		m_workram(*this,"workram"),
		m_vreg(*this,"vreg"),
		m_paletteram(*this, "paletteram"),
		m_cgram(*this, "cgram"),
		m_videoram(*this,"videoram"),
		m_scroll(*this,"scroll"),
		m_spriteram(*this,"spriteram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<c140_device> m_c140;

	required_ioport_array<4> m_muxed_inputs;
	required_ioport          m_io_p3;

	required_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_vreg;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_cgram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_spriteram;

	// this has to be uint8_t to be in the right byte order for the tilemap system
	std::vector<uint8_t> m_shaperam;

	uint16_t *m_prgrom;
	uint16_t *m_maskrom;
	int m_mEnableInterrupts;
	int m_gametype;
	uint16_t m_count;
	uint32_t m_keyval;
	uint16_t m_mcu_mailbox[8];
	uint8_t m_mcu_port4;
	uint8_t m_mcu_port5;
	uint8_t m_mcu_port6;
	uint8_t m_mcu_port8;
	tilemap_t *m_bg_tilemap[4+1];
	int m_palette_is_dirty;

	uint16_t custom_key_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void custom_key_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vreg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mcu_mailbox_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mcu_mailbox_w_68k(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mcu_mailbox_w_mcu(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t na1mcu_shared_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void na1mcu_shared_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t port4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port7_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port8_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portana_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void simulate_mcu();
	void write_version_info();
	int transfer_dword(uint32_t dest, uint32_t source);
	void blit();
	void UpdatePalette(int offset);
	void videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gfxram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gfxram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pdraw_tile( screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, uint32_t code, int color,
		int sx, int sy, int flipx, int flipy, int priority, int bShadow, int bOpaque, int gfx_region );
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int primask );
	uint16_t snd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void snd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_bkrtmaq();
	void init_quiztou();
	void init_emeralda();
	void init_numanath();
	void init_fa();
	void init_cgangpzl();
	void init_tinklpit();
	void init_swcourt();
	void init_knckhead();
	void init_xday2();
	void init_exbania();
	void init_emeraldj();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void tilemap_get_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tilemap_get_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tilemap_get_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tilemap_get_info3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void roz_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void interrupt(timer_device &timer, void *ptr, int32_t param);
	void postload();

private:
	void tilemap_get_info(tile_data &tileinfo, int tile_index, const uint16_t *tilemap_videoram, bool use_4bpp_gfx);
};
