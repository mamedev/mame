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

	tmnt_state(const machine_config &mconfig, device_type type, std::string tag)
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
	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_tmnt2_rom;
	optional_shared_ptr<UINT16> m_sunset_104000;
	optional_shared_ptr<UINT16> m_tmnt2_1c0800;

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
	UINT16     m_cuebrick_nvram[0x400 * 0x20 / 2];  // 32k paged in a 1k window

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
	INT16      m_sampledata[0x40000];

	UINT8      m_irq5_mask;
	DECLARE_READ16_MEMBER(k052109_word_noA12_r);
	DECLARE_WRITE16_MEMBER(k052109_word_noA12_w);
	DECLARE_WRITE16_MEMBER(punkshot_k052109_word_w);
	DECLARE_WRITE16_MEMBER(punkshot_k052109_word_noA12_w);
	DECLARE_READ16_MEMBER(k053245_scattered_word_r);
	DECLARE_WRITE16_MEMBER(k053245_scattered_word_w);
	DECLARE_READ16_MEMBER(k053244_word_noA1_r);
	DECLARE_WRITE16_MEMBER(k053244_word_noA1_w);
	DECLARE_WRITE16_MEMBER(prmrsocr_sound_irq_w);
	DECLARE_WRITE8_MEMBER(prmrsocr_audio_bankswitch_w);
	DECLARE_READ8_MEMBER(tmnt_sres_r);
	DECLARE_WRITE8_MEMBER(tmnt_sres_w);
	DECLARE_WRITE8_MEMBER(sound_arm_nmi_w);
	DECLARE_READ16_MEMBER(punkshot_kludge_r);
	DECLARE_READ16_MEMBER(ssriders_protection_r);
	DECLARE_WRITE16_MEMBER(ssriders_protection_w);
	DECLARE_READ16_MEMBER(blswhstl_coin_r);
	DECLARE_READ16_MEMBER(ssriders_eeprom_r);
	DECLARE_READ16_MEMBER(sunsetbl_eeprom_r);
	DECLARE_WRITE16_MEMBER(blswhstl_eeprom_w);
	DECLARE_READ16_MEMBER(thndrx2_eeprom_r);
	DECLARE_WRITE16_MEMBER(thndrx2_eeprom_w);
	DECLARE_WRITE16_MEMBER(prmrsocr_eeprom_w);
	DECLARE_WRITE8_MEMBER(cuebrick_nvbank_w);
	DECLARE_WRITE16_MEMBER(ssriders_soundkludge_w);
	DECLARE_WRITE16_MEMBER(k053251_glfgreat_w);
	DECLARE_WRITE16_MEMBER(tmnt2_1c0800_w);
	DECLARE_READ8_MEMBER(k054539_ctrl_r);
	DECLARE_WRITE8_MEMBER(k054539_ctrl_w);
	DECLARE_WRITE16_MEMBER(tmnt_0a0000_w);
	DECLARE_WRITE16_MEMBER(punkshot_0a0020_w);
	DECLARE_WRITE16_MEMBER(lgtnfght_0a0018_w);
	DECLARE_WRITE16_MEMBER(blswhstl_700300_w);
	DECLARE_READ16_MEMBER(glfgreat_rom_r);
	DECLARE_WRITE16_MEMBER(glfgreat_122000_w);
	DECLARE_WRITE16_MEMBER(ssriders_eeprom_w);
	DECLARE_WRITE16_MEMBER(ssriders_1c0300_w);
	DECLARE_WRITE16_MEMBER(prmrsocr_122000_w);
	DECLARE_READ16_MEMBER(prmrsocr_rom_r);
	DECLARE_WRITE16_MEMBER(tmnt_priority_w);
	DECLARE_READ16_MEMBER(glfgreat_ball_r);
	DECLARE_WRITE8_MEMBER(glfgreat_sound_w);
	DECLARE_WRITE8_MEMBER(tmnt_upd_start_w);
	DECLARE_READ8_MEMBER(tmnt_upd_busy_r);
	DECLARE_DRIVER_INIT(mia);
	DECLARE_DRIVER_INIT(tmnt);
	DECLARE_DRIVER_INIT(cuebrick);
	TILE_GET_INFO_MEMBER(glfgreat_get_roz_tile_info);
	TILE_GET_INFO_MEMBER(prmrsocr_get_roz_tile_info);
	DECLARE_MACHINE_START(common);
	DECLARE_MACHINE_RESET(common);
	DECLARE_VIDEO_START(cuebrick);
	DECLARE_VIDEO_START(mia);
	DECLARE_MACHINE_RESET(tmnt);
	DECLARE_VIDEO_START(tmnt);
	DECLARE_VIDEO_START(lgtnfght);
	DECLARE_VIDEO_START(blswhstl);
	DECLARE_VIDEO_START(glfgreat);
	DECLARE_MACHINE_START(prmrsocr);
	DECLARE_VIDEO_START(prmrsocr);
	UINT32 screen_update_mia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tmnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_punkshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_lgtnfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_glfgreat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tmnt2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_thndrx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_blswhstl(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(tmnt_interrupt);
	INTERRUPT_GEN_MEMBER(punkshot_interrupt);
	INTERRUPT_GEN_MEMBER(lgtnfght_interrupt);
	inline UINT32 tmnt2_get_word( UINT32 addr );
	void tmnt2_put_word( address_space &space, UINT32 addr, UINT16 data );
	DECLARE_WRITE8_MEMBER(volume_callback);
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
