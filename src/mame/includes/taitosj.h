/*----------- defined in machine/taitosj.c -----------*/

MACHINE_START( taitosj );
MACHINE_RESET( taitosj );
WRITE8_HANDLER( taitosj_bankswitch_w );
READ8_HANDLER( taitosj_fake_data_r );
READ8_HANDLER( taitosj_fake_status_r );
WRITE8_HANDLER( taitosj_fake_data_w );
READ8_HANDLER( taitosj_mcu_data_r );
READ8_HANDLER( taitosj_mcu_status_r );
WRITE8_HANDLER( taitosj_mcu_data_w );
READ8_HANDLER( taitosj_68705_portA_r );
READ8_HANDLER( taitosj_68705_portB_r );
READ8_HANDLER( taitosj_68705_portC_r );
WRITE8_HANDLER( taitosj_68705_portA_w );
WRITE8_HANDLER( taitosj_68705_portB_w );

READ8_HANDLER( spacecr_prot_r );
WRITE8_HANDLER( alpine_protection_w );
WRITE8_HANDLER( alpinea_bankswitch_w );
READ8_HANDLER( alpine_port_2_r );


/*----------- defined in video/taitosj.c -----------*/

extern UINT8 *taitosj_videoram_1;
extern UINT8 *taitosj_videoram_2;
extern UINT8 *taitosj_videoram_3;
extern UINT8 *taitosj_spriteram;
extern UINT8 *taitosj_paletteram;
extern UINT8 *taitosj_characterram;
extern UINT8 *taitosj_scroll;
extern UINT8 *taitosj_colscrolly;
extern UINT8 *taitosj_gfxpointer;
extern UINT8 *taitosj_colorbank;
extern UINT8 *taitosj_video_mode;
extern UINT8 *taitosj_video_priority;
extern UINT8 *taitosj_collision_reg;
extern UINT8 *kikstart_scrollram;

READ8_HANDLER( taitosj_gfxrom_r );
WRITE8_HANDLER( taitosj_characterram_w );
WRITE8_HANDLER( junglhbr_characterram_w );
WRITE8_HANDLER( taitosj_collision_reg_clear_w );
VIDEO_START( taitosj );
VIDEO_UPDATE( taitosj );
VIDEO_UPDATE( kikstart );
