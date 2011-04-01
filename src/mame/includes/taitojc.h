#include "video/poly.h"

class taitojc_state : public driver_device
{
public:
	taitojc_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_texture_x;
	int m_texture_y;

	UINT32 m_dsp_rom_pos;
	UINT16 m_dsp_tex_address;
	UINT16 m_dsp_tex_offset;


	int m_first_dsp_reset;
	int m_viewport_data[3];

	INT32 m_projected_point_x;
	INT32 m_projected_point_y;
	INT32 m_projection_data[3];

	INT32 m_intersection_data[3];

	UINT8 *m_texture;
	bitmap_t *m_framebuffer;
	bitmap_t *m_zbuffer;

	UINT32 *m_vram;
	UINT32 *m_objlist;

	//int debug_tex_pal;

	int m_gfx_index;

	UINT32 *m_char_ram;
	UINT32 *m_tile_ram;
	tilemap_t *m_tilemap;

	poly_manager *m_poly;

	UINT32 *m_f3_shared_ram;
	UINT32 *m_main_ram;
	UINT16 *m_dsp_shared_ram;
	UINT32 *m_palette_ram;

	UINT16 *m_polygon_fifo;
	int m_polygon_fifo_ptr;

	UINT8 m_mcu_comm_main;
	UINT8 m_mcu_comm_hc11;
	UINT8 m_mcu_data_main;
	UINT8 m_mcu_data_hc11;

	UINT16 m_debug_dsp_ram[0x8000];
};


/*----------- defined in video/taitojc.c -----------*/

READ32_HANDLER(taitojc_tile_r);
WRITE32_HANDLER(taitojc_tile_w);
READ32_HANDLER(taitojc_char_r);
WRITE32_HANDLER(taitojc_char_w);
void taitojc_clear_frame(running_machine &machine);
void taitojc_render_polygons(running_machine &machine, UINT16 *polygon_fifo, int length);

VIDEO_START(taitojc);
SCREEN_UPDATE(taitojc);
