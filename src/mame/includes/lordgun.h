/*************************************************************************

                      -= IGS Lord Of Gun =-

*************************************************************************/

typedef struct _lordgun_gun_data lordgun_gun_data;
struct _lordgun_gun_data
{
	int		scr_x,	scr_y;
	UINT16	hw_x,	hw_y;
};

class lordgun_state : public driver_device
{
public:
	lordgun_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 old;
	UINT8 aliencha_dip_sel;
	UINT16 *priority_ram;
	UINT16 priority;
	UINT16 *vram_0;
	UINT16 *scroll_x_0;
	UINT16 *scroll_y_0;
	UINT16 *vram_1;
	UINT16 *scroll_x_1;
	UINT16 *scroll_y_1;
	UINT16 *vram_2;
	UINT16 *scroll_x_2;
	UINT16 *scroll_y_2;
	UINT16 *vram_3;
	UINT16 *scroll_x_3;
	UINT16 *scroll_y_3;
	UINT16 *scrollram;
	int whitescreen;
	lordgun_gun_data gun[2];
	tilemap_t *tilemap_0;
	tilemap_t *tilemap_1;
	tilemap_t *tilemap_2;
	tilemap_t *tilemap_3;
	bitmap_t *bitmaps[5];
};


/*----------- defined in video/lordgun.c -----------*/

WRITE16_HANDLER( lordgun_paletteram_w );
WRITE16_HANDLER( lordgun_vram_0_w );
WRITE16_HANDLER( lordgun_vram_1_w );
WRITE16_HANDLER( lordgun_vram_2_w );
WRITE16_HANDLER( lordgun_vram_3_w );

float lordgun_crosshair_mapper(const input_field_config *field, float linear_value);
void lordgun_update_gun(running_machine *machine, int i);

VIDEO_START( lordgun );
SCREEN_UPDATE( lordgun );
