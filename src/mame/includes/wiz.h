class wiz_state : public driver_device
{
public:
	wiz_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	int dsc0;
	int dsc1;
	UINT8 *videoram2;
	UINT8 *colorram2;
	UINT8 *attributesram;
	UINT8 *attributesram2;
	UINT8 *sprite_bank;
	INT32 flipx;
	INT32 flipy;
	INT32 bgpen;
	UINT8 char_bank[2];
	UINT8 palbank[2];
	int palette_bank;
	UINT8 *spriteram;
	UINT8 *spriteram2;
	size_t spriteram_size;
};


/*----------- defined in video/wiz.c -----------*/

WRITE8_HANDLER( wiz_char_bank_select_w );
WRITE8_HANDLER( wiz_palettebank_w );
WRITE8_HANDLER( wiz_bgcolor_w );
WRITE8_HANDLER( wiz_flipx_w );
WRITE8_HANDLER( wiz_flipy_w );

VIDEO_START( wiz );
PALETTE_INIT( wiz );
SCREEN_UPDATE( wiz );
SCREEN_UPDATE( stinger );
SCREEN_UPDATE( kungfut );
