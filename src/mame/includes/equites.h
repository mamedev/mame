
#define POPDRUMKIT 0


typedef struct _equites_state equites_state;
struct _equites_state
{
	/* memory pointers */
	UINT16 *  bg_videoram;
	UINT8  *  fg_videoram;	// 8bits
	UINT16 *  spriteram;
	UINT16 *  spriteram_2;
	UINT16 *  workram;
	UINT8  *  mcu_ram;	// 8bits
//  UINT16 *  nvram;    // currently this uses generic nvram handling

	/* video-related */
	tilemap_t   *fg_tilemap, *bg_tilemap;
	int       fg_char_bank;
	UINT8     bgcolor;
	UINT16    splndrbt_bg_scrollx, splndrbt_bg_scrolly;

	/* misc */
	int       sound_prom_address;
	UINT8     dac_latch;
	UINT8     eq8155_port_b;
	UINT8     eq8155_port_a,eq8155_port_c,ay_port_a,ay_port_b,eq_cymbal_ctrl;
	emu_timer *nmi_timer, *adjuster_timer;
	float     cymvol,hihatvol;
	int       timer_count;
	int       unknown_bit;	// Gekisou special handling
#if POPDRUMKIT
	int       hihat,cymbal;
#endif

	/* devices */
	const device_config *mcu;
	const device_config *audio_cpu;
	const device_config *msm;
	const device_config *dac_1;
	const device_config *dac_2;
};


/*----------- defined in video/equites.c -----------*/

extern READ16_HANDLER(equites_fg_videoram_r);
extern WRITE16_HANDLER(equites_fg_videoram_w);
extern WRITE16_HANDLER(equites_bg_videoram_w);
extern WRITE16_HANDLER(equites_scrollreg_w);
extern WRITE16_HANDLER(equites_bgcolor_w);
extern WRITE16_HANDLER(splndrbt_selchar0_w);
extern WRITE16_HANDLER(splndrbt_selchar1_w);
extern WRITE16_HANDLER(equites_flip0_w);
extern WRITE16_HANDLER(equites_flip1_w);
extern WRITE16_HANDLER(splndrbt_flip0_w);
extern WRITE16_HANDLER(splndrbt_flip1_w);
extern WRITE16_HANDLER(splndrbt_bg_scrollx_w);
extern WRITE16_HANDLER(splndrbt_bg_scrolly_w);

extern PALETTE_INIT( equites );
extern VIDEO_START( equites );
extern VIDEO_UPDATE( equites );
extern PALETTE_INIT( splndrbt );
extern VIDEO_START( splndrbt );
extern VIDEO_UPDATE( splndrbt );
