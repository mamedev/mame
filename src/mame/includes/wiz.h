class wiz_state : public driver_device
{
public:
	wiz_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/wiz.c -----------*/

extern UINT8 *wiz_videoram2;
extern UINT8 *wiz_colorram2;
extern UINT8 *wiz_attributesram;
extern UINT8 *wiz_attributesram2;
extern UINT8 *wiz_sprite_bank;

WRITE8_HANDLER( wiz_char_bank_select_w );
WRITE8_HANDLER( wiz_palettebank_w );
WRITE8_HANDLER( wiz_bgcolor_w );
WRITE8_HANDLER( wiz_flipx_w );
WRITE8_HANDLER( wiz_flipy_w );

VIDEO_START( wiz );
PALETTE_INIT( wiz );
VIDEO_UPDATE( wiz );
VIDEO_UPDATE( stinger );
VIDEO_UPDATE( kungfut );
