/*----------- defined in machine/stfight.c -----------*/

DRIVER_INIT( empcity );
DRIVER_INIT( stfight );
MACHINE_RESET( stfight );
INTERRUPT_GEN( stfight_vb_interrupt );
READ8_HANDLER( stfight_dsw_r );
WRITE8_HANDLER( stfight_fm_w );
READ8_HANDLER( stfight_coin_r );
WRITE8_HANDLER( stfight_coin_w );
WRITE8_HANDLER( stfight_e800_w );
READ8_HANDLER( stfight_fm_r );
void stfight_adpcm_int(device_t *device);
WRITE8_DEVICE_HANDLER( stfight_adpcm_control_w );


/*----------- defined in video/stfight.c -----------*/

extern UINT8 *stfight_text_char_ram;
extern UINT8 *stfight_text_attr_ram;
extern UINT8 *stfight_vh_latch_ram;
extern UINT8 *stfight_sprite_ram;

PALETTE_INIT( stfight );
WRITE8_HANDLER( stfight_text_char_w );
WRITE8_HANDLER( stfight_text_attr_w );
WRITE8_HANDLER( stfight_vh_latch_w );
WRITE8_HANDLER( stfight_sprite_bank_w );
VIDEO_START( stfight );
VIDEO_UPDATE( stfight );
