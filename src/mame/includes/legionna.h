/*----------- defined in video/legionna.c -----------*/

extern UINT16 *legionna_back_data,*legionna_fore_data,*legionna_mid_data,*legionna_scrollram16,*legionna_textram;
extern UINT16 legionna_layer_disable;

void heatbrl_setgfxbank(UINT16 data);
void denjinmk_setgfxbank(UINT16 data);
WRITE16_HANDLER( legionna_background_w );
WRITE16_HANDLER( legionna_foreground_w );
WRITE16_HANDLER( legionna_midground_w );
WRITE16_HANDLER( legionna_text_w );

VIDEO_START( legionna );
VIDEO_START( cupsoc );
VIDEO_START( denjinmk );
VIDEO_START( grainbow );
VIDEO_START( godzilla );
SCREEN_UPDATE( legionna );
SCREEN_UPDATE( godzilla );
SCREEN_UPDATE( grainbow );
