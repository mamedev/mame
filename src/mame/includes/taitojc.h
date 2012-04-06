#include "video/poly.h"

class taitojc_state : public driver_device
{
public:
	taitojc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

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
	bitmap_ind16 m_framebuffer;
	bitmap_ind16 m_zbuffer;

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

	UINT8 m_has_dsp_hack;

	double m_speed_meter;
	double m_brake_meter;
	UINT32 m_outputs;
	DECLARE_READ32_MEMBER(taitojc_palette_r);
	DECLARE_WRITE32_MEMBER(taitojc_palette_w);
	DECLARE_READ32_MEMBER(jc_control_r);
	DECLARE_WRITE32_MEMBER(jc_coin_counters_w);
	DECLARE_WRITE32_MEMBER(jc_control_w);
	DECLARE_WRITE32_MEMBER(jc_control1_w);
	DECLARE_READ32_MEMBER(mcu_comm_r);
	DECLARE_WRITE32_MEMBER(mcu_comm_w);
	DECLARE_READ8_MEMBER(jc_pcbid_r);
	DECLARE_READ32_MEMBER(dsp_shared_r);
	DECLARE_WRITE32_MEMBER(dsp_shared_w);
	DECLARE_READ32_MEMBER(f3_share_r);
	DECLARE_WRITE32_MEMBER(f3_share_w);
	DECLARE_WRITE32_MEMBER(jc_meters_w);
	DECLARE_READ32_MEMBER(jc_lan_r);
	DECLARE_READ8_MEMBER(hc11_comm_r);
	DECLARE_WRITE8_MEMBER(hc11_comm_w);
	DECLARE_READ8_MEMBER(hc11_data_r);
	DECLARE_WRITE8_MEMBER(hc11_data_w);
	DECLARE_READ8_MEMBER(hc11_analog_r);
	DECLARE_READ16_MEMBER(dsp_rom_r);
	DECLARE_WRITE16_MEMBER(dsp_rom_w);
	DECLARE_WRITE16_MEMBER(dsp_texture_w);
	DECLARE_READ16_MEMBER(dsp_texaddr_r);
	DECLARE_WRITE16_MEMBER(dsp_texaddr_w);
	DECLARE_WRITE16_MEMBER(dsp_polygon_fifo_w);
	DECLARE_READ16_MEMBER(dsp_unk_r);
	DECLARE_WRITE16_MEMBER(dsp_viewport_w);
	DECLARE_WRITE16_MEMBER(dsp_projection_w);
	DECLARE_READ16_MEMBER(dsp_projection_r);
	DECLARE_WRITE16_MEMBER(dsp_unk2_w);
	DECLARE_WRITE16_MEMBER(dsp_intersection_w);
	DECLARE_READ16_MEMBER(dsp_intersection_r);
	DECLARE_READ16_MEMBER(dsp_to_main_r);
	DECLARE_WRITE16_MEMBER(dsp_to_main_w);
	DECLARE_READ16_MEMBER(taitojc_dsp_idle_skip_r);
	DECLARE_READ16_MEMBER(dendego2_dsp_idle_skip_r);
	DECLARE_WRITE16_MEMBER(dsp_idle_skip_w);
	DECLARE_READ32_MEMBER(taitojc_tile_r);
	DECLARE_READ32_MEMBER(taitojc_char_r);
	DECLARE_WRITE32_MEMBER(taitojc_tile_w);
	DECLARE_WRITE32_MEMBER(taitojc_char_w);
};


/*----------- defined in video/taitojc.c -----------*/

void taitojc_clear_frame(running_machine &machine);
void taitojc_render_polygons(running_machine &machine, UINT16 *polygon_fifo, int length);

VIDEO_START(taitojc);
SCREEN_UPDATE_IND16(taitojc);

const double taitojc_odometer_table[0x100] =
{
	0.0,   0.3,   0.7,   1.0,   1.4,   1.7,   2.1,   2.4,   2.8,   3.1,   3.4,   3.8,   4.1,   4.5,   4.8,   5.2,
	5.5,   5.9,   6.2,   6.6,   6.9,   7.2,   7.6,   7.9,   8.3,   8.6,   9.0,   9.3,   9.7,  10.0,  10.5,  11.1,
	11.6,  12.1,  12.6,  13.2,  13.7,  14.2,  14.7,  15.3,  15.8,  16.3,  16.8,  17.4,  17.9,  18.4,  18.9,  19.5,
	20.0,  20.6,  21.1,  21.7,  22.2,  22.8,  23.3,  23.9,  24.4,  25.0,  25.6,  26.1,  26.7,  27.2,  27.8,  28.3,
	28.9,  29.4,  30.0,  30.6,  31.1,  31.7,  32.2,  32.8,  33.3,  33.9,  34.4,  35.0,  35.6,  36.1,  36.7,  37.2,
	37.8,  38.3,  38.9,  39.4,  40.0,  40.6,  41.2,  41.8,  42.4,  42.9,  43.5,  44.1,  44.7,  45.3,  45.9,  46.5,
	47.1,  47.6,  48.2,  48.8,  49.4,  50.0,  50.5,  51.1,  51.6,  52.1,  52.6,  53.2,  53.7,  54.2,  54.7,  55.3,
	55.8,  56.3,  56.8,  57.4,  57.9,  58.4,  58.9,  59.5,  60.0,  60.7,  61.3,  62.0,  62.7,  63.3,  64.0,  64.7,
	65.3,  66.0,  66.7,  67.3,  68.0,  68.7,  69.3,  70.0,  70.5,  71.1,  71.6,  72.1,  72.6,  73.2,  73.7,  74.2,
	74.7,  75.3,  75.8,  76.3,  76.8,  77.4,  77.9,  78.4,  78.9,  79.5,  80.0,  80.6,  81.2,  81.8,  82.4,  82.9,
	83.5,  84.1,  84.7,  85.3,  85.9,  86.5,  87.1,  87.6,  88.2,  88.8,  89.4,  90.0,  90.6,  91.1,  91.7,  92.2,
	92.8,  93.3,  93.9,  94.4,  95.0,  95.6,  96.1,  96.7,  97.2,  97.8,  98.3,  98.9,  99.4, 100.0, 100.5, 101.1,
	101.6, 102.1, 102.6, 103.2, 103.7, 104.2, 104.7, 105.3, 105.8, 106.3, 106.8, 107.4, 107.9, 108.4, 108.9, 109.5,
	110.0, 110.7, 111.3, 112.0, 112.7, 113.3, 114.0, 114.7, 115.3, 116.0, 116.7, 117.3, 118.0, 118.7, 119.3, 120.0,
	120.3, 120.6, 120.9, 121.2, 121.6, 121.9, 122.2, 122.5, 122.8, 123.1, 123.4, 123.8, 124.1, 124.4, 124.7, 125.0,
	125.3, 125.6, 125.9, 126.2, 126.6, 126.9, 127.2, 127.5, 127.8, 128.1, 128.4, 128.8, 129.1, 129.4, 129.7, 130.0,
};

const double taitojc_brake_table[0x100] =
{
	 0.00,  0.00,  0.00,  0.00,  0.05,  0.10,  0.14,  0.19,  0.24,  0.29,  0.33,  0.38,  0.43,  0.48,  0.52,  0.57,
	 0.62,  0.67,  0.71,  0.76,  0.81,  0.86,  0.90,  0.95,  1.00,  1.06,  1.12,  1.19,  1.25,  1.31,  1.38,  1.44,
	 1.50,  1.56,  1.62,  1.69,  1.75,  1.81,  1.88,  1.94,  2.00,  2.06,  2.12,  2.19,  2.25,  2.31,  2.38,  2.44,
	 2.50,  2.56,  2.62,  2.69,  2.75,  2.81,  2.88,  2.94,  3.00,  3.06,  3.12,  3.18,  3.24,  3.29,  3.35,  3.41,
	 3.47,  3.53,  3.59,  3.65,  3.71,  3.76,  3.82,  3.88,  3.94,  4.00,  4.07,  4.13,  4.20,  4.27,  4.33,  4.40,
	 4.47,  4.53,  4.60,  4.67,  4.73,  4.80,  4.87,  4.93,  5.00,  5.07,  5.14,  5.21,  5.29,  5.36,  5.43,  5.50,
	 5.57,  5.64,  5.71,  5.79,  5.86,  5.93,  6.00,  6.07,  6.14,  6.21,  6.29,  6.36,  6.43,  6.50,  6.57,  6.64,
	 6.71,  6.79,  6.86,  6.93,  7.00,  7.06,  7.12,  7.19,  7.25,  7.31,  7.38,  7.44,  7.50,  7.56,  7.62,  7.69,
	 7.75,  7.81,  7.88,  7.94,  8.00,  8.07,  8.14,  8.21,  8.29,  8.36,  8.43,  8.50,  8.57,  8.64,  8.71,  8.79,
	 8.86,  8.93,  9.00,  9.07,  9.14,  9.21,  9.29,  9.36,  9.43,  9.50,  9.57,  9.64,  9.71,  9.79,  9.86,  9.93,
	10.00, 10.08, 10.15, 10.23, 10.31, 10.38, 10.46, 10.54, 10.62, 10.69, 10.77, 10.85, 10.92, 11.00, 11.08, 11.15,
	11.23, 11.31, 11.38, 11.46, 11.54, 11.62, 11.69, 11.77, 11.85, 11.92, 12.00, 12.07, 12.14, 12.21, 12.29, 12.36,
	12.43, 12.50, 12.57, 12.64, 12.71, 12.79, 12.86, 12.93, 13.00, 13.07, 13.14, 13.21, 13.29, 13.36, 13.43, 13.50,
	13.57, 13.64, 13.71, 13.79, 13.86, 13.93, 14.00, 14.07, 14.14, 14.21, 14.29, 14.36, 14.43, 14.50, 14.57, 14.64,
	14.71, 14.79, 14.86, 14.93, 15.00, 15.04, 15.07, 15.11, 15.15, 15.19, 15.22, 15.26, 15.30, 15.33, 15.37, 15.41,
	15.44, 15.48, 15.52, 15.56, 15.59, 15.63, 15.67, 15.70, 15.74, 15.78, 15.81, 15.85, 15.89, 15.93, 15.96, 16.00,
};
