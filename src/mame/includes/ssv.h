#include "cpu/upd7725/upd7725.h"

class ssv_state : public driver_device
{
public:
	ssv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_dsp(*this, "dsp")
	{ }

	optional_device<upd96050_device> m_dsp;

	UINT16 *m_scroll;
	UINT16 *m_paletteram;
	UINT16 *m_spriteram;
	UINT16 *m_spriteram2;

	int m_tile_code[16];

	int m_enable_video;
	int m_shadow_pen_mask;
	int m_shadow_pen_shift;

	UINT8 m_requested_int;
	UINT16 *m_irq_vectors;
	UINT16 m_irq_enable;
	UINT16 *m_mainram;

	UINT16 *m_dsp_ram;

	UINT16 *m_eaglshot_gfxram;
	UINT16 *m_gdfs_tmapram;
	UINT16 *m_gdfs_tmapscroll;

	tilemap_t *m_gdfs_tmap;

	int m_interrupt_ultrax;

	UINT16 *m_input_sel;
	int m_gdfs_gfxram_bank;
	int m_gdfs_lightgun_select;
	UINT16 *m_gdfs_blitram;

	UINT16 m_sxyreact_serial;
	int m_sxyreact_dial;
	UINT16 m_gdfs_eeprom_old;

	UINT32 m_latches[8];

	UINT8 m_trackball_select;
	UINT8 m_gfxrom_select;
	DECLARE_WRITE16_MEMBER(ssv_irq_ack_w);
	DECLARE_WRITE16_MEMBER(ssv_irq_enable_w);
	DECLARE_WRITE16_MEMBER(ssv_lockout_w);
	DECLARE_WRITE16_MEMBER(ssv_lockout_inv_w);
	DECLARE_READ16_MEMBER(dsp_dr_r);
	DECLARE_WRITE16_MEMBER(dsp_dr_w);
	DECLARE_READ16_MEMBER(dsp_r);
	DECLARE_WRITE16_MEMBER(dsp_w);
	DECLARE_READ16_MEMBER(fake_r);
	DECLARE_READ16_MEMBER(drifto94_rand_r);
	DECLARE_READ16_MEMBER(gdfs_gfxram_r);
	DECLARE_WRITE16_MEMBER(gdfs_gfxram_w);
	DECLARE_READ16_MEMBER(gdfs_blitram_r);
	DECLARE_WRITE16_MEMBER(gdfs_blitram_w);
	DECLARE_READ16_MEMBER(hypreact_input_r);
	DECLARE_READ16_MEMBER(ssv_mainram_r);
	DECLARE_WRITE16_MEMBER(ssv_mainram_w);
	DECLARE_READ16_MEMBER(srmp4_input_r);
	DECLARE_READ16_MEMBER(srmp7_irqv_r);
	DECLARE_WRITE16_MEMBER(srmp7_sound_bank_w);
	DECLARE_READ16_MEMBER(srmp7_input_r);
	DECLARE_READ16_MEMBER(sxyreact_ballswitch_r);
	DECLARE_READ16_MEMBER(sxyreact_dial_r);
	DECLARE_WRITE16_MEMBER(sxyreact_dial_w);
	DECLARE_WRITE16_MEMBER(sxyreact_motor_w);
	DECLARE_READ32_MEMBER(latch32_r);
	DECLARE_WRITE32_MEMBER(latch32_w);
	DECLARE_READ16_MEMBER(latch16_r);
	DECLARE_WRITE16_MEMBER(latch16_w);
	DECLARE_READ16_MEMBER(eaglshot_gfxrom_r);
	DECLARE_WRITE16_MEMBER(eaglshot_gfxrom_w);
	DECLARE_READ16_MEMBER(eaglshot_trackball_r);
	DECLARE_WRITE16_MEMBER(eaglshot_trackball_w);
	DECLARE_READ16_MEMBER(eaglshot_gfxram_r);
	DECLARE_WRITE16_MEMBER(eaglshot_gfxram_w);
	DECLARE_WRITE16_MEMBER(gdfs_tmapram_w);
	DECLARE_READ16_MEMBER(ssv_vblank_r);
	DECLARE_WRITE16_MEMBER(ssv_scroll_w);
	DECLARE_WRITE16_MEMBER(paletteram16_xrgb_swap_word_w);
};

/*----------- defined in video/ssv.c -----------*/

void ssv_enable_video(running_machine &machine, int enable);

VIDEO_START( ssv );
VIDEO_START( eaglshot );
VIDEO_START( gdfs );

SCREEN_UPDATE_IND16( ssv );
SCREEN_UPDATE_IND16( eaglshot );
SCREEN_UPDATE_IND16( gdfs );
