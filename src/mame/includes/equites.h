// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Nicola Salmoria
/*************************************************************************

    Equites, Splendor Blast driver

*************************************************************************/

#include "machine/alpha8201.h"
#include "machine/gen_latch.h"
#include "sound/samples.h"
#include "sound/msm5232.h"
#include "sound/dac.h"


class equites_state : public driver_device
{
public:
	equites_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_mcuram(*this, "mcuram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_alpha_8201(*this, "alpha_8201"),
		m_fakemcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_dac_1(*this, "dac1"),
		m_dac_2(*this, "dac2"),
		m_soundlatch(*this, "soundlatch")
	{ }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg_videoram;
	std::unique_ptr<uint8_t[]> m_fg_videoram;    // 8bits
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_spriteram_2;
	optional_shared_ptr<uint8_t> m_mcuram;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int       m_fg_char_bank;
	uint8_t     m_bgcolor;
	uint16_t    m_splndrbt_bg_scrollx;
	uint16_t    m_splndrbt_bg_scrolly;

	/* misc */
	int       m_sound_prom_address;
	uint8_t     m_dac_latch;
	uint8_t     m_eq8155_port_b;
	uint8_t     m_eq8155_port_a;
	uint8_t     m_eq8155_port_c;
	uint8_t     m_ay_port_a;
	uint8_t     m_ay_port_b;
	uint8_t     m_eq_cymbal_ctrl;
	emu_timer *m_nmi_timer;
	emu_timer *m_adjuster_timer;
	float     m_cymvol;
	float     m_hihatvol;
	int       m_timer_count;
	int       m_gekisou_unknown_bit;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<alpha_8201_device> m_alpha_8201;
	optional_device<cpu_device> m_fakemcu;
	required_device<msm5232_device> m_msm;
	required_device<dac_byte_interface> m_dac_1;
	required_device<dac_byte_interface> m_dac_2;
	required_device<generic_latch_8_device> m_soundlatch;

	void equites_c0f8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void equites_cymbal_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void equites_dac_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void equites_8155_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void equites_8155_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gekisou_unknown_bit_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t equites_spriteram_kludge_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t mcu_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_start_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mcu_switch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t equites_fg_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void equites_fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void equites_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void equites_bgcolor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void equites_scrollreg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void splndrbt_selchar_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void equites_flipw_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void equites_flipb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void splndrbt_bg_scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void splndrbt_bg_scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value gekisou_unknown_bit_r(ioport_field &field, void *param);
	void equites_8910porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void equites_8910portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_splndrbt();
	void init_equites();
	void equites_fg_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void splndrbt_fg_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void equites_bg_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void splndrbt_bg_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_equites();
	void palette_init_equites(palette_device &palette);
	void video_start_splndrbt();
	void palette_init_splndrbt(palette_device &palette);
	uint32_t screen_update_equites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_splndrbt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void equites_nmi_callback(void *ptr, int32_t param);
	void equites_frq_adjuster_callback(void *ptr, int32_t param);
	void equites_scanline(timer_device &timer, void *ptr, int32_t param);
	void splndrbt_scanline(timer_device &timer, void *ptr, int32_t param);
	void equites_msm5232_gate(int state);
	void equites_draw_sprites_block(bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end);
	void equites_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void splndrbt_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void splndrbt_copy_bg(bitmap_ind16 &dst_bitmap, const rectangle &cliprect);
	void equites_update_dac();
	void unpack_block(const char *region, int offset, int size);
	void unpack_region(const char *region);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};
