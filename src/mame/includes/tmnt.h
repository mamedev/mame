// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/samples.h"
#include "sound/upd7759.h"
#include "sound/k007232.h"
#include "sound/k053260.h"
#include "sound/k054539.h"
#include "video/k053244_k053245.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/k053251.h"
#include "video/k053936.h"
#include "video/k054000.h"
#include "video/konami_helper.h"

class tmnt_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	tmnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_tmnt2_rom(*this, "tmnt2_rom"),
		m_sunset_104000(*this, "sunset_104000"),
		m_tmnt2_1c0800(*this, "tmnt2_1c0800"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232(*this, "k007232"),
		m_k053260(*this, "k053260"),
		m_k054539(*this, "k054539"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k053245(*this, "k053245"),
		m_k053251(*this, "k053251"),
		m_k053936(*this, "k053936"),
		m_k054000(*this, "k054000"),
		m_upd7759(*this, "upd"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_tmnt2_rom;
	optional_shared_ptr<uint16_t> m_sunset_104000;
	optional_shared_ptr<uint16_t> m_tmnt2_1c0800;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];
	int        m_sorted_layer[3];   // this might not be necessary, but tmnt2 uses it in a strange way...

	tilemap_t  *m_roz_tilemap;
	int        m_glfgreat_roz_rom_bank;
	int        m_glfgreat_roz_char_bank;
	int        m_glfgreat_roz_rom_mode;
	int        m_glfgreat_pixel;
	int        m_prmrsocr_sprite_bank;
	int        m_blswhstl_rombank;
	int        m_tmnt_priorityflag;
	int        m_lastdim;
	int        m_lasten;
	int        m_dim_c;
	int        m_dim_v; /* lgtnfght, ssriders, tmnt2 only */

	/* misc */
	int        m_tmnt_soundlatch;
	int        m_toggle;
	int        m_last;
	uint16_t     m_cuebrick_nvram[0x400 * 0x20 / 2];  // 32k paged in a 1k window

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<k007232_device> m_k007232;
	optional_device<k053260_device> m_k053260;
	optional_device<k054539_device> m_k054539;
	required_device<k052109_device> m_k052109;
	optional_device<k051960_device> m_k051960;
	optional_device<k05324x_device> m_k053245;
	optional_device<k053251_device> m_k053251;
	optional_device<k053936_device> m_k053936;
	optional_device<k054000_device> m_k054000;
	optional_device<upd7759_device> m_upd7759;
	optional_device<samples_device> m_samples;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory buffers */
	int16_t      m_sampledata[0x40000];

	uint8_t      m_irq5_mask;
	uint16_t k052109_word_noA12_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void k052109_word_noA12_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void punkshot_k052109_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void punkshot_k052109_word_noA12_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t k053245_scattered_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void k053245_scattered_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t k053244_word_noA1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void k053244_word_noA1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void prmrsocr_sound_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void prmrsocr_audio_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tmnt_sres_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tmnt_sres_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_arm_nmi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t punkshot_kludge_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ssriders_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ssriders_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t blswhstl_coin_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ssriders_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sunsetbl_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void blswhstl_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t thndrx2_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void thndrx2_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void prmrsocr_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cuebrick_nvbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ssriders_soundkludge_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void k053251_glfgreat_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tmnt2_1c0800_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t k054539_ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void k054539_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tmnt_0a0000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void punkshot_0a0020_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void lgtnfght_0a0018_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blswhstl_700300_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t glfgreat_rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void glfgreat_122000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ssriders_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ssriders_1c0300_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void prmrsocr_122000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t prmrsocr_rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tmnt_priority_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t glfgreat_ball_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void glfgreat_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tmnt_upd_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tmnt_upd_busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_mia();
	void init_tmnt();
	void init_cuebrick();
	void glfgreat_get_roz_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void prmrsocr_get_roz_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_common();
	void machine_reset_common();
	void video_start_cuebrick();
	void video_start_mia();
	void machine_reset_tmnt();
	void video_start_tmnt();
	void video_start_lgtnfght();
	void video_start_blswhstl();
	void video_start_glfgreat();
	void machine_start_prmrsocr();
	void video_start_prmrsocr();
	uint32_t screen_update_mia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tmnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_punkshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lgtnfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_glfgreat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tmnt2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_thndrx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_blswhstl(screen_device &screen, bool state);
	void tmnt_interrupt(device_t &device);
	void punkshot_interrupt(device_t &device);
	void lgtnfght_interrupt(device_t &device);
	inline uint32_t tmnt2_get_word( uint32_t addr );
	void tmnt2_put_word( address_space &space, uint32_t addr, uint16_t data );
	void volume_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	K051960_CB_MEMBER(mia_sprite_callback);
	K051960_CB_MEMBER(tmnt_sprite_callback);
	K051960_CB_MEMBER(punkshot_sprite_callback);
	K051960_CB_MEMBER(thndrx2_sprite_callback);
	K05324X_CB_MEMBER(lgtnfght_sprite_callback);
	K05324X_CB_MEMBER(blswhstl_sprite_callback);
	K05324X_CB_MEMBER(prmrsocr_sprite_callback);
	K052109_CB_MEMBER(mia_tile_callback);
	K052109_CB_MEMBER(cuebrick_tile_callback);
	K052109_CB_MEMBER(tmnt_tile_callback);
	K052109_CB_MEMBER(ssbl_tile_callback);
	K052109_CB_MEMBER(blswhstl_tile_callback);
	SAMPLES_START_CB_MEMBER(tmnt_decode_sample);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
