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
	lordgun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_old;
	UINT8 m_aliencha_dip_sel;
	UINT16 *m_priority_ram;
	UINT16 m_priority;
	UINT16 *m_vram[4];
	UINT16 *m_scroll_x[4];
	UINT16 *m_scroll_y[4];
	UINT16 *m_scrollram;
	int m_whitescreen;
	lordgun_gun_data m_gun[2];
	tilemap_t *m_tilemap[4];
	bitmap_t *m_bitmaps[5];
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/lordgun.c -----------*/

WRITE16_HANDLER( lordgun_paletteram_w );
WRITE16_HANDLER( lordgun_vram_0_w );
WRITE16_HANDLER( lordgun_vram_1_w );
WRITE16_HANDLER( lordgun_vram_2_w );
WRITE16_HANDLER( lordgun_vram_3_w );

float lordgun_crosshair_mapper(const input_field_config *field, float linear_value);
void lordgun_update_gun(running_machine &machine, int i);

VIDEO_START( lordgun );
SCREEN_UPDATE( lordgun );
