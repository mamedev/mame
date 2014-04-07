/*************************************************************************

    Irem M72 hardware

*************************************************************************/
#include "audio/m72.h"
#include "sound/dac.h"

class m72_state : public driver_device
{
public:
	m72_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_majtitle_rowscrollram(*this, "majtitle_rowscr"),
		m_spriteram2(*this, "spriteram2"),
		m_soundram(*this, "soundram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_mcu(*this, "mcu"),
		m_dac(*this, "dac"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_videoram1;
	required_shared_ptr<UINT16> m_videoram2;
	optional_shared_ptr<UINT16> m_majtitle_rowscrollram;
	optional_shared_ptr<UINT16> m_spriteram2;
	optional_shared_ptr<UINT8> m_soundram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_mcu;
	optional_device<dac_device> m_dac;
	optional_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT16 *m_protection_ram;
	emu_timer *m_scanline_timer;
	UINT8 m_irq_base;
	UINT8 m_mcu_snd_cmd_latch;
	UINT8 m_mcu_sample_latch;
	UINT32 m_mcu_sample_addr;
	const UINT8 *m_protection_code;
	const UINT8 *m_protection_crc;
	int m_prev[4];
	int m_diff[4];
	UINT32 m_raster_irq_position;
	UINT16 *m_buffered_spriteram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	INT32 m_scrollx1;
	INT32 m_scrolly1;
	INT32 m_scrollx2;
	INT32 m_scrolly2;
	INT32 m_video_off;
	int m_majtitle_rowscroll;
	DECLARE_WRITE16_MEMBER(m72_main_mcu_sound_w);
	DECLARE_WRITE16_MEMBER(m72_main_mcu_w);
	DECLARE_WRITE8_MEMBER(m72_mcu_data_w);
	DECLARE_READ8_MEMBER(m72_mcu_data_r);
	DECLARE_READ8_MEMBER(m72_mcu_sample_r);
	DECLARE_WRITE8_MEMBER(m72_mcu_ack_w);
	DECLARE_READ8_MEMBER(m72_mcu_snd_r);
	DECLARE_READ8_MEMBER(m72_mcu_port_r);
	DECLARE_WRITE8_MEMBER(m72_mcu_port_w);
	DECLARE_WRITE8_MEMBER(m72_mcu_low_w);
	DECLARE_WRITE8_MEMBER(m72_mcu_high_w);
	DECLARE_READ8_MEMBER(m72_snd_cpu_sample_r);
	DECLARE_WRITE16_MEMBER(bchopper_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(nspirit_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(imgfight_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(loht_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(xmultiplm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(dbreedm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(airduel_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(dkgenm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(gallop_sample_trigger_w);
	DECLARE_READ16_MEMBER(protection_r);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_READ16_MEMBER(soundram_r);
	DECLARE_WRITE16_MEMBER(soundram_w);
	DECLARE_READ16_MEMBER(poundfor_trackball_r);
	DECLARE_READ16_MEMBER(m72_palette1_r);
	DECLARE_READ16_MEMBER(m72_palette2_r);
	DECLARE_WRITE16_MEMBER(m72_palette1_w);
	DECLARE_WRITE16_MEMBER(m72_palette2_w);
	DECLARE_WRITE16_MEMBER(m72_videoram1_w);
	DECLARE_WRITE16_MEMBER(m72_videoram2_w);
	DECLARE_WRITE16_MEMBER(m72_irq_line_w);
	DECLARE_WRITE16_MEMBER(m72_scrollx1_w);
	DECLARE_WRITE16_MEMBER(m72_scrollx2_w);
	DECLARE_WRITE16_MEMBER(m72_scrolly1_w);
	DECLARE_WRITE16_MEMBER(m72_scrolly2_w);
	DECLARE_WRITE16_MEMBER(m72_dmaon_w);
	DECLARE_WRITE16_MEMBER(m72_port02_w);
	DECLARE_WRITE16_MEMBER(rtype2_port02_w);
	DECLARE_WRITE16_MEMBER(majtitle_gfx_ctrl_w);
	DECLARE_WRITE8_MEMBER(m72_snd_cpu_sample_w);
	DECLARE_DRIVER_INIT(dkgenm72);
	DECLARE_DRIVER_INIT(bchopper);
	DECLARE_DRIVER_INIT(gallop);
	DECLARE_DRIVER_INIT(m72_8751);
	DECLARE_DRIVER_INIT(xmultiplm72);
	DECLARE_DRIVER_INIT(mrheli);
	DECLARE_DRIVER_INIT(dbreedm72);
	DECLARE_DRIVER_INIT(airduel);
	DECLARE_DRIVER_INIT(nspirit);
	DECLARE_DRIVER_INIT(loht);
	DECLARE_DRIVER_INIT(imgfight);
	TILE_GET_INFO_MEMBER(m72_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(m72_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(hharry_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(rtype2_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(rtype2_get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(majtitle_scan_rows);
	DECLARE_MACHINE_START(m72);
	DECLARE_MACHINE_RESET(m72);
	DECLARE_VIDEO_START(m72);
	DECLARE_MACHINE_RESET(xmultipl);
	DECLARE_VIDEO_START(xmultipl);
	DECLARE_VIDEO_START(hharry);
	DECLARE_VIDEO_START(rtype2);
	DECLARE_VIDEO_START(majtitle);
	DECLARE_VIDEO_START(hharryu);
	DECLARE_VIDEO_START(poundfor);
	DECLARE_MACHINE_START(kengo);
	DECLARE_MACHINE_RESET(kengo);
	UINT32 screen_update_m72(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_majtitle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(m72_mcu_int);
	INTERRUPT_GEN_MEMBER(fake_nmi);
	TIMER_CALLBACK_MEMBER(synch_callback);
	TIMER_CALLBACK_MEMBER(m72_scanline_interrupt);
	TIMER_CALLBACK_MEMBER(kengo_scanline_interrupt);
	TIMER_CALLBACK_MEMBER(delayed_ram16_w);
	inline void m72_get_tile_info(tile_data &tileinfo,int tile_index,const UINT16 *vram,int gfxnum);
	inline void rtype2_get_tile_info(tile_data &tileinfo,int tile_index,const UINT16 *vram,int gfxnum);
	void register_savestate();
	inline void changecolor(int color,int r,int g,int b);
	void m72_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void majtitle_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	int find_sample(int num);
	void copy_le(UINT16 *dest, const UINT8 *src, UINT8 bytes);
	void install_protection_handler(const UINT8 *code,const UINT8 *crc);
};
