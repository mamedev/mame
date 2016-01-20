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
	m72_state(const machine_config &mconfig, device_type type, std::string tag)
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
	optional_device<dac_device> m_dac;
	optional_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_videoram1;
	required_shared_ptr<UINT16> m_videoram2;
	optional_shared_ptr<UINT16> m_m82_rowscrollram;
	optional_shared_ptr<UINT16> m_spriteram2;
	optional_shared_ptr<UINT8> m_soundram;
	required_shared_ptr<UINT16> m_generic_paletteram_16;
	required_shared_ptr<UINT16> m_generic_paletteram2_16;
	optional_device<pic8259_device> m_upd71059c;

	std::unique_ptr<UINT16[]> m_protection_ram;
	emu_timer *m_scanline_timer;
	const UINT8 *m_protection_code;
	const UINT8 *m_protection_crc;
	UINT32 m_raster_irq_position;
	std::unique_ptr<UINT16[]> m_buffered_spriteram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_large;
	INT32 m_scrollx1;
	INT32 m_scrolly1;
	INT32 m_scrollx2;
	INT32 m_scrolly2;
	INT32 m_video_off;

	int m_fg_source;
	int m_bg_source;
	optional_ioport m_m81_b_b_j3;

	//poundfor specific
	int m_prev[4];
	int m_diff[4];

	// majtitle specific
	int m_m82_rowscroll;
	UINT16 m_m82_tmcontrol;

	// m72_i8751 specific
	UINT8 m_mcu_snd_cmd_latch;
	UINT8 m_mcu_sample_latch;
	UINT32 m_mcu_sample_addr;

	// common
	DECLARE_READ16_MEMBER(palette1_r);
	DECLARE_READ16_MEMBER(palette2_r);
	DECLARE_WRITE16_MEMBER(palette1_w);
	DECLARE_WRITE16_MEMBER(palette2_w);
	DECLARE_WRITE16_MEMBER(videoram1_w);
	DECLARE_WRITE16_MEMBER(videoram2_w);
	DECLARE_READ16_MEMBER(soundram_r);
	DECLARE_WRITE16_MEMBER(soundram_w);

	// m72_i8751 specific
	DECLARE_WRITE16_MEMBER(main_mcu_sound_w);
	DECLARE_WRITE16_MEMBER(main_mcu_w);
	DECLARE_WRITE8_MEMBER(mcu_data_w);
	DECLARE_READ8_MEMBER(mcu_data_r);
	DECLARE_READ8_MEMBER(mcu_sample_r);
	DECLARE_WRITE8_MEMBER(mcu_ack_w);
	DECLARE_READ8_MEMBER(mcu_snd_r);
	DECLARE_READ8_MEMBER(mcu_port_r);
	DECLARE_WRITE8_MEMBER(mcu_port_w);
	DECLARE_WRITE8_MEMBER(mcu_low_w);
	DECLARE_WRITE8_MEMBER(mcu_high_w);
	DECLARE_READ8_MEMBER(snd_cpu_sample_r);
	DECLARE_WRITE16_MEMBER(irq_line_w);
	DECLARE_WRITE16_MEMBER(scrollx1_w);
	DECLARE_WRITE16_MEMBER(scrollx2_w);
	DECLARE_WRITE16_MEMBER(scrolly1_w);
	DECLARE_WRITE16_MEMBER(scrolly2_w);
	DECLARE_WRITE16_MEMBER(dmaon_w);
	DECLARE_WRITE16_MEMBER(port02_w);
	DECLARE_READ16_MEMBER(protection_r);
	DECLARE_WRITE16_MEMBER(protection_w);

	// game specific
	DECLARE_WRITE16_MEMBER(bchopper_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(nspirit_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(imgfight_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(loht_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(dbreedm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(airduelm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(dkgenm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(gallop_sample_trigger_w);
	DECLARE_READ16_MEMBER(poundfor_trackball_r);
	DECLARE_WRITE16_MEMBER(rtype2_port02_w);
	DECLARE_WRITE16_MEMBER(m82_gfx_ctrl_w);
	DECLARE_WRITE16_MEMBER(m82_tm_ctrl_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	TILE_GET_INFO_MEMBER(rtype2_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(rtype2_get_fg_tile_info);

	TILEMAP_MAPPER_MEMBER(m82_scan_rows);

	void machine_start() override;
	void machine_reset() override;
	DECLARE_VIDEO_START(m72);
	DECLARE_VIDEO_START(xmultipl);
	DECLARE_VIDEO_START(hharry);
	DECLARE_VIDEO_START(rtype2);
	DECLARE_VIDEO_START(m82);
	DECLARE_VIDEO_START(hharryu);
	DECLARE_VIDEO_START(poundfor);
	DECLARE_MACHINE_START(kengo);
	DECLARE_MACHINE_RESET(kengo);
	DECLARE_DRIVER_INIT(dkgenm72);
	DECLARE_DRIVER_INIT(bchopper);
	DECLARE_DRIVER_INIT(gallop);
	DECLARE_DRIVER_INIT(m72_8751);
	DECLARE_DRIVER_INIT(dbreedm72);
	DECLARE_DRIVER_INIT(airduelm72);
	DECLARE_DRIVER_INIT(nspirit);
	DECLARE_DRIVER_INIT(loht);
	DECLARE_DRIVER_INIT(imgfight);

	INTERRUPT_GEN_MEMBER(mcu_int);
	INTERRUPT_GEN_MEMBER(fake_nmi);
	TIMER_CALLBACK_MEMBER(synch_callback);
	TIMER_CALLBACK_MEMBER(scanline_interrupt);
	TIMER_CALLBACK_MEMBER(kengo_scanline_interrupt);
	TIMER_CALLBACK_MEMBER(delayed_ram16_w);


	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_m81(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_m82(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void m72_m81_get_tile_info(tile_data &tileinfo,int tile_index,const UINT16 *vram,int gfxnum);
	inline void m82_m84_get_tile_info(tile_data &tileinfo,int tile_index,const UINT16 *vram,int gfxnum);
	void register_savestate();
	inline void changecolor(int color,int r,int g,int b);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void majtitle_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void copy_le(UINT16 *dest, const UINT8 *src, UINT8 bytes);
	void install_protection_handler(const UINT8 *code,const UINT8 *crc);
};
