// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/nvram.h"
#include "sound/msm5205.h"

class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_adpcmrom(*this, "adpcm"),
		m_videoram(*this, "videoram"),
		m_row(*this, "ROW.%u", 0) { }

	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_region_ptr<uint8_t> m_adpcmrom;
	required_shared_ptr<uint8_t> m_videoram;

	required_ioport_array<12> m_row;

	int m_adpcm_pos;
	int m_adpcm_end;
	int m_keyb;
	bool m_nmi_enable;
	bool m_display_enable;
	tilemap_t *m_bg_tilemap;

	ioport_value keyboard_r(ioport_field &field, void *param);
	void keyboard_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjkjidai_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjkjidai_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_int(int state);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_mjkjidai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
