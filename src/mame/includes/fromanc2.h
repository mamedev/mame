// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki

#include "machine/gen_latch.h"
#include "machine/eepromser.h"

class fromanc2_state : public driver_device
{
public:
	fromanc2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lpalette(*this, "lpalette"),
		m_rpalette(*this, "rpalette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2") { }

	/* memory pointers */
	std::unique_ptr<uint16_t[]>   m_videoram[2][4];
	std::unique_ptr<uint8_t[]>    m_bankedram;

	/* video-related */
	tilemap_t  *m_tilemap[2][4];
	int      m_scrollx[2][4];
	int      m_scrolly[2][4];
	int      m_gfxbank[2][4];

	/* misc */
	int      m_portselect;
	uint8_t    m_subcpu_int_flag;
	uint8_t    m_subcpu_nmi_flag;
	uint8_t    m_sndcpu_nmi_flag;
	uint16_t   m_datalatch1;
	uint8_t    m_datalatch_2h;
	uint8_t    m_datalatch_2l;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_lpalette;
	required_device<palette_device> m_rpalette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	void fromanc2_sndcmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_portselect_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t fromanc2_keymatrix_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void fromancr_gfxbank_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_subcpu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t fromanc2_subcpu_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t fromanc2_maincpu_r_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t fromanc2_maincpu_r_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fromanc2_maincpu_w_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromanc2_maincpu_w_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromanc2_subcpu_nmi_clr(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fromanc2_sndcpu_nmi_clr(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fromanc2_subcpu_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fromanc2_videoram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_videoram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_videoram_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_videoram_3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_gfxreg_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_gfxreg_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_gfxreg_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_gfxreg_3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_gfxbank_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc2_gfxbank_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromancr_videoram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromancr_videoram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromancr_videoram_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromancr_gfxreg_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromancr_gfxreg_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc4_videoram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc4_videoram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc4_videoram_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc4_gfxreg_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc4_gfxreg_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fromanc4_gfxreg_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value subcpu_int_r(ioport_field &field, void *param);
	ioport_value sndcpu_nmi_r(ioport_field &field, void *param);
	ioport_value subcpu_nmi_r(ioport_field &field, void *param);
	void init_fromanc4();
	void init_fromanc2();
	void fromanc2_get_v0_l0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromanc2_get_v0_l1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromanc2_get_v0_l2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromanc2_get_v0_l3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromanc2_get_v1_l0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromanc2_get_v1_l1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromanc2_get_v1_l2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromanc2_get_v1_l3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromancr_get_v0_l0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromancr_get_v0_l1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromancr_get_v0_l2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromancr_get_v1_l0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromancr_get_v1_l1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fromancr_get_v1_l2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	void machine_start_fromanc2();
	void video_start_fromanc2();
	void video_start_fromancr();
	void machine_start_fromanc4();
	void video_start_fromanc4();
	uint32_t screen_update_fromanc2_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_fromanc2_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fromanc2_interrupt(device_t &device);
	inline void fromanc2_get_tile_info( tile_data &tileinfo, int tile_index, int vram, int layer );
	inline void fromancr_get_tile_info( tile_data &tileinfo, int tile_index, int vram, int layer );
	inline void fromanc2_dispvram_w( offs_t offset, uint16_t data, uint16_t mem_mask, int vram, int layer );
	inline void fromancr_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask, int layer );
	void fromancr_gfxbank_w( int data );
	inline void fromanc4_vram_w( offs_t offset, uint16_t data, uint16_t mem_mask, int layer );
};
