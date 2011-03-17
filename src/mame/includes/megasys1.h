/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)


    This file contains definitions used across multiple megasys1
    and non megasys1 Jaleco games:

    * Scrolling layers handling
    * Code decryption handling

***************************************************************************/


class megasys1_state : public driver_device
{
public:
	megasys1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *spriteram;
	UINT16 ip_select;
	UINT16 ip_select_values[5];
	UINT8 ignore_oki_status;
	UINT16 protection_val;
	int bank;
	UINT16 *scrollram[3];
	UINT16 *objectram;
	UINT16 *vregs;
	UINT16 *ram;
	int scrollx[3];
	int scrolly[3];
	int active_layers;
	int bits_per_color_code;
	int scroll_flag[3];
	int sprite_bank;
	int screen_flag;
	int sprite_flag;
	int _8x8_scroll_factor[3];
	int _16x16_scroll_factor[3];
	tilemap_t *tmap[3];
	tilemap_t *tilemap[3][2][4];
	int hardware_type_z;
	UINT16 *buffer_objectram;
	UINT16 *buffer2_objectram;
	UINT16 *buffer_spriteram16;
	UINT16 *buffer2_spriteram16;
	int layers_order[16];
};


/*----------- defined in video/megasys1.c -----------*/

VIDEO_START( megasys1 );
SCREEN_UPDATE( megasys1 );
SCREEN_EOF( megasys1 );

PALETTE_INIT( megasys1 );

READ16_HANDLER( megasys1_vregs_C_r );

WRITE16_HANDLER( megasys1_vregs_A_w );
WRITE16_HANDLER( megasys1_vregs_C_w );
WRITE16_HANDLER( megasys1_vregs_D_w );

WRITE16_HANDLER( megasys1_scrollram_0_w );
WRITE16_HANDLER( megasys1_scrollram_1_w );
WRITE16_HANDLER( megasys1_scrollram_2_w );
