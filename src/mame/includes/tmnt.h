#include "sound/samples.h"

class tmnt_state : public driver_device
{
public:
	tmnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_tmnt2_rom(*this, "tmnt2_rom"),
		m_sunset_104000(*this, "sunset_104000"),
		m_tmnt2_1c0800(*this, "tmnt2_1c0800"){ }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_tmnt2_rom;
	optional_shared_ptr<UINT16> m_sunset_104000;
	optional_shared_ptr<UINT16> m_tmnt2_1c0800;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling
//  UINT8 *     m_nvram;    // currently cuebrick uses generic nvram handling
//  UINT8 *     m_cuebrick_nvram;

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
	int        m_cuebrick_snd_irqlatch;
	int        m_cuebrick_nvram_bank;
	int        m_toggle;
	int        m_last;
	UINT16     m_cuebrick_nvram[0x400 * 0x20];  // 32k paged in a 1k window

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_k007232;
	device_t *m_k053260;
	device_t *m_k054539;
	device_t *m_k052109;
	device_t *m_k051960;
	device_t *m_k053245;
	device_t *m_k053251;
	device_t *m_k053936;
	device_t *m_k054000;
	device_t *m_upd;
	samples_device *m_samples;

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
	DECLARE_WRITE16_MEMBER(tmnt_sound_command_w);
	DECLARE_READ16_MEMBER(prmrsocr_sound_r);
	DECLARE_WRITE16_MEMBER(prmrsocr_sound_cmd_w);
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
	DECLARE_READ16_MEMBER(cuebrick_nv_r);
	DECLARE_WRITE16_MEMBER(cuebrick_nv_w);
	DECLARE_WRITE16_MEMBER(cuebrick_nvbank_w);
	DECLARE_WRITE16_MEMBER(ssriders_soundkludge_w);
	DECLARE_WRITE16_MEMBER(k053251_glfgreat_w);
	DECLARE_WRITE16_MEMBER(tmnt2_1c0800_w);
	DECLARE_READ8_MEMBER(k054539_ctrl_r);
	DECLARE_WRITE8_MEMBER(k054539_ctrl_w);
	DECLARE_WRITE16_MEMBER(tmnt_paletteram_word_w);
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
	DECLARE_READ8_MEMBER(punkshot_sound_r);
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
	INTERRUPT_GEN_MEMBER(cuebrick_interrupt);
	INTERRUPT_GEN_MEMBER(punkshot_interrupt);
	INTERRUPT_GEN_MEMBER(lgtnfght_interrupt);
	INTERRUPT_GEN_MEMBER(tmnt_vblank_irq);
	TIMER_CALLBACK_MEMBER(nmi_callback);
};


/*----------- defined in video/tmnt.c -----------*/

extern void mia_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void cuebrick_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void tmnt_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void ssbl_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void blswhstl_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void mia_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void tmnt_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void punkshot_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);
extern void thndrx2_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);
extern void lgtnfght_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
extern void blswhstl_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
extern void prmrsocr_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
