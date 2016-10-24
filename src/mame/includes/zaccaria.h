// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "audio/zaccaria.h"

class zaccaria_state : public driver_device
{
public:
	zaccaria_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_audiopcb(*this, "audiopcb")
		, m_videoram(*this, "videoram")
		, m_attributesram(*this, "attributesram")
		, m_spriteram(*this, "spriteram")
		, m_spriteram2(*this, "spriteram2")
		, m_dsw_port(*this, "DSW.%u", 0)
	{ }

	uint8_t dsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t prot1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t prot2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void attributes_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ressound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dsw_sel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_zaccaria(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,uint8_t *spriteram,int color,int section);

protected:
	required_device<cpu_device>                 m_maincpu;
	required_device<gfxdecode_device>           m_gfxdecode;
	required_device<palette_device>             m_palette;
	required_device<zac1b11142_audio_device>    m_audiopcb;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_attributesram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;

	required_ioport_array<3> m_dsw_port;

	int m_dsw_sel;
	tilemap_t *m_bg_tilemap;
	uint8_t m_nmi_mask;
};
