/*************************************************************************

    Konami Battlantis Hardware

*************************************************************************/

class battlnts_state : public driver_device
{
public:
	battlnts_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *      paletteram;    // this currently uses generic palette handling

	/* video-related */
	int m_spritebank;
	int m_layer_colorbase[2];


	/* devices */
	device_t *m_audiocpu;
	device_t *m_k007342;
	device_t *m_k007420;
	DECLARE_WRITE8_MEMBER(battlnts_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(battlnts_bankswitch_w);
	DECLARE_WRITE8_MEMBER(battlnts_spritebank_w);
};

/*----------- defined in video/battlnts.c -----------*/


SCREEN_UPDATE_IND16( battlnts );

void battlnts_tile_callback(running_machine &machine, int layer, int bank, int *code, int *color, int *flags);
void battlnts_sprite_callback(running_machine &machine, int *code, int *color);
