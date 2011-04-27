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
};

/*----------- defined in video/ssv.c -----------*/

READ16_HANDLER( ssv_vblank_r );
WRITE16_HANDLER( ssv_scroll_w );
WRITE16_HANDLER( paletteram16_xrgb_swap_word_w );
WRITE16_HANDLER( gdfs_tmapram_w );
void ssv_enable_video(running_machine &machine, int enable);

VIDEO_START( ssv );
VIDEO_START( eaglshot );
VIDEO_START( gdfs );

SCREEN_UPDATE( ssv );
SCREEN_UPDATE( eaglshot );
SCREEN_UPDATE( gdfs );
