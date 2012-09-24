/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/


#define CPUTAG_MCU "mcu"


class champbas_state : public driver_device
{
public:
	champbas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram_2(*this, "spriteram_2"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram_2;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	UINT8          m_gfx_bank;
	UINT8          m_palette_bank;

	/* misc */
	int            m_watchdog_count;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_mcu;

	UINT8          m_irq_mask;
	DECLARE_WRITE8_MEMBER(champbas_watchdog_reset_w);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(champbas_mcu_switch_w);
	DECLARE_WRITE8_MEMBER(champbas_mcu_halt_w);
	DECLARE_READ8_MEMBER(champbja_alt_protection_r);
	DECLARE_WRITE8_MEMBER(champbas_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(champbas_gfxbank_w);
	DECLARE_WRITE8_MEMBER(champbas_palette_bank_w);
	DECLARE_WRITE8_MEMBER(champbas_flipscreen_w);
	DECLARE_CUSTOM_INPUT_MEMBER(champbas_watchdog_bit2);
	DECLARE_WRITE8_MEMBER(champbas_dac_w);
	DECLARE_WRITE8_MEMBER(champbas_dac1_w);
	DECLARE_WRITE8_MEMBER(champbas_dac2_w);
	DECLARE_DRIVER_INIT(exctsccr);
	DECLARE_DRIVER_INIT(champbas);
	TILE_GET_INFO_MEMBER(champbas_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(exctsccr_get_bg_tile_info);
	DECLARE_MACHINE_START(champbas);
	DECLARE_MACHINE_RESET(champbas);
	DECLARE_VIDEO_START(champbas);
	DECLARE_PALETTE_INIT(champbas);
	DECLARE_MACHINE_START(exctsccr);
	DECLARE_VIDEO_START(exctsccr);
	DECLARE_PALETTE_INIT(exctsccr);
	UINT32 screen_update_champbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_exctsccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_champbas(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	TIMER_CALLBACK_MEMBER(exctsccr_fm_callback);
};
