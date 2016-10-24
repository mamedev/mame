// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Irem M72 hardware

*************************************************************************/
#include "audio/m72.h"
#include "sound/dac.h"
#include "machine/pic8259.h"

#define M81_B_B_JUMPER_J3_S \
	PORT_START("JumperJ3") \
	PORT_CONFNAME( 0x0001, 0x0000, "M81-B-B Jumper J3" ) \
	PORT_CONFSETTING(      0x0000, "S" ) \
	/* PORT_CONFSETTING(      0x0001, "W" ) */

#define M81_B_B_JUMPER_J3_W \
	PORT_START("JumperJ3") \
	PORT_CONFNAME( 0x0001, 0x0001, "M81-B-B Jumper J3" ) \
	/* PORT_CONFSETTING(      0x0000, "S" ) */ \
	PORT_CONFSETTING(      0x0001, "W" )

class m72_state : public driver_device
{
public:
	m72_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_mcu(*this, "mcu"),
		m_dac(*this, "dac"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_m82_rowscrollram(*this, "majtitle_rowscr"),
		m_spriteram2(*this, "spriteram2"),
		m_soundram(*this, "soundram"),
		m_generic_paletteram_16(*this, "paletteram"),
		m_generic_paletteram2_16(*this, "paletteram2"),
		m_upd71059c(*this, "upd71059c"),
		m_fg_source(0),
		m_bg_source(0),
		m_m81_b_b_j3(*this, "JumperJ3"),
		m_m82_rowscroll(0),
		m_m82_tmcontrol(0)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_mcu;
	optional_device<dac_byte_interface> m_dac;
	optional_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_videoram1;
	required_shared_ptr<uint16_t> m_videoram2;
	optional_shared_ptr<uint16_t> m_m82_rowscrollram;
	optional_shared_ptr<uint16_t> m_spriteram2;
	optional_shared_ptr<uint8_t> m_soundram;
	required_shared_ptr<uint16_t> m_generic_paletteram_16;
	required_shared_ptr<uint16_t> m_generic_paletteram2_16;
	optional_device<pic8259_device> m_upd71059c;

	std::unique_ptr<uint16_t[]> m_protection_ram;
	emu_timer *m_scanline_timer;
	const uint8_t *m_protection_code;
	const uint8_t *m_protection_crc;
	uint32_t m_raster_irq_position;
	std::unique_ptr<uint16_t[]> m_buffered_spriteram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_large;
	int32_t m_scrollx1;
	int32_t m_scrolly1;
	int32_t m_scrollx2;
	int32_t m_scrolly2;
	int32_t m_video_off;

	int m_fg_source;
	int m_bg_source;
	optional_ioport m_m81_b_b_j3;

	//poundfor specific
	int m_prev[4];
	int m_diff[4];

	// majtitle specific
	int m_m82_rowscroll;
	uint16_t m_m82_tmcontrol;

	// m72_i8751 specific
	uint8_t m_mcu_snd_cmd_latch;
	uint8_t m_mcu_sample_latch;
	uint32_t m_mcu_sample_addr;

	// common
	uint16_t palette1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t palette2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void palette1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void palette2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t soundram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void soundram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// m72_i8751 specific
	void main_mcu_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void main_mcu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mcu_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_sample_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_snd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t snd_cpu_sample_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void irq_line_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scrollx1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scrollx2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scrolly1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scrolly2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dmaon_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void port02_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// game specific
	void bchopper_sample_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nspirit_sample_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void imgfight_sample_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void loht_sample_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dbreedm72_sample_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void airduelm72_sample_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dkgenm72_sample_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gallop_sample_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t poundfor_trackball_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rtype2_port02_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void m82_gfx_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void m82_tm_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void rtype2_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void rtype2_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	tilemap_memory_index m82_scan_rows(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	void machine_start() override;
	void machine_reset() override;
	void video_start_m72();
	void video_start_xmultipl();
	void video_start_hharry();
	void video_start_rtype2();
	void video_start_m82();
	void video_start_hharryu();
	void video_start_poundfor();
	void machine_start_kengo();
	void machine_reset_kengo();
	void init_dkgenm72();
	void init_bchopper();
	void init_gallop();
	void init_m72_8751();
	void init_dbreedm72();
	void init_airduelm72();
	void init_nspirit();
	void init_loht();
	void init_imgfight();

	void mcu_int(device_t &device);
	void fake_nmi(device_t &device);
	void synch_callback(void *ptr, int32_t param);
	void scanline_interrupt(void *ptr, int32_t param);
	void kengo_scanline_interrupt(void *ptr, int32_t param);
	void delayed_ram16_w(void *ptr, int32_t param);


	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_m81(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_m82(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void m72_m81_get_tile_info(tile_data &tileinfo,int tile_index,const uint16_t *vram,int gfxnum);
	inline void m82_m84_get_tile_info(tile_data &tileinfo,int tile_index,const uint16_t *vram,int gfxnum);
	void register_savestate();
	inline void changecolor(int color,int r,int g,int b);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void majtitle_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void copy_le(uint16_t *dest, const uint8_t *src, uint8_t bytes);
	void install_protection_handler(const uint8_t *code,const uint8_t *crc);
};
