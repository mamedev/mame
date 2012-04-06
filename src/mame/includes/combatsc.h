/*************************************************************************

    Combat School

*************************************************************************/

class combatsc_state : public driver_device
{
public:
	combatsc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_scrollram;
	UINT8 *    m_io_ram;
	UINT8 *    m_paletteram;
	UINT8 *    m_spriteram[2];

	/* video-related */
	tilemap_t *m_bg_tilemap[2];
	tilemap_t *m_textlayer;
	UINT8 m_scrollram0[0x40];
	UINT8 m_scrollram1[0x40];
	int m_priority;

	int  m_vreg;
	int  m_bank_select; /* 0x00..0x1f */
	int  m_video_circuit; /* 0 or 1 */
	UINT8 *m_page[2];

	/* misc */
	UINT8 m_pos[4];
	UINT8 m_sign[4];
	int m_prot[2];
	int m_boost;
	emu_timer *m_interleave_timer;


	/* devices */
	cpu_device *m_audiocpu;
	device_t *m_k007121_1;
	device_t *m_k007121_2;
	DECLARE_WRITE8_MEMBER(combatsc_vreg_w);
	DECLARE_WRITE8_MEMBER(combatscb_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(combatscb_io_r);
	DECLARE_WRITE8_MEMBER(combatscb_priority_w);
	DECLARE_WRITE8_MEMBER(combatsc_bankselect_w);
	DECLARE_WRITE8_MEMBER(combatscb_io_w);
	DECLARE_WRITE8_MEMBER(combatscb_bankselect_w);
	DECLARE_WRITE8_MEMBER(combatsc_coin_counter_w);
	DECLARE_READ8_MEMBER(trackball_r);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(protection_clock_w);
	DECLARE_WRITE8_MEMBER(combatsc_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(combatsc_video_r);
	DECLARE_WRITE8_MEMBER(combatsc_video_w);
	DECLARE_WRITE8_MEMBER(combatsc_pf_control_w);
	DECLARE_READ8_MEMBER(combatsc_scrollram_r);
	DECLARE_WRITE8_MEMBER(combatsc_scrollram_w);
};


/*----------- defined in video/combatsc.c -----------*/



PALETTE_INIT( combatsc );
PALETTE_INIT( combatscb );
VIDEO_START( combatsc );
VIDEO_START( combatscb );
SCREEN_UPDATE_IND16( combatscb );
SCREEN_UPDATE_IND16( combatsc );
