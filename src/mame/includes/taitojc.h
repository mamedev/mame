#include "video/polynew.h"

#define TAITOJC_POLYGON_FIFO_SIZE		0x20000

struct taitojc_polydata
{
	int tex_base_x;
	int tex_base_y;
	int tex_wrap_x;
	int tex_wrap_y;
};

class taitojc_renderer : public poly_manager<float, taitojc_polydata, 6, 10000>
{
public:
	taitojc_renderer(running_machine &machine, bitmap_ind16 *fb, bitmap_ind16 *zb, const UINT8 *texture_ram)
		: poly_manager<float, taitojc_polydata, 6, 10000>(machine)
	{
		m_framebuffer = fb;
		m_zbuffer = zb;
		m_texture = texture_ram;
	}

	void render_solid_scan(INT32 scanline, const extent_t &extent, const taitojc_polydata &extradata, int threadid);
	void render_shade_scan(INT32 scanline, const extent_t &extent, const taitojc_polydata &extradata, int threadid);
	void render_texture_scan(INT32 scanline, const extent_t &extent, const taitojc_polydata &extradata, int threadid);

	void render_polygons(running_machine &machine, UINT16 *polygon_fifo, int length);

private:
	bitmap_ind16 *m_framebuffer;
	bitmap_ind16 *m_zbuffer;
	const UINT8 *m_texture;
};

class taitojc_state : public driver_device
{
public:
	taitojc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_dsp(*this,"dsp"),
		m_gfx2(*this, "gfx2"),
		m_vram(*this, "vram"),
		m_objlist(*this, "objlist"),
		m_snd_shared_ram(*this, "snd_shared"),
		m_main_ram(*this, "main_ram"),
		m_dsp_shared_ram(*this, "dsp_shared"),
		m_palette_ram(*this, "palette_ram")
	{
		m_mcu_output = 0;
		m_speed_meter = 0;
		m_brake_meter = 0;
	}

	// device/memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	required_memory_region m_gfx2;

	required_shared_ptr<UINT32> m_vram;
	required_shared_ptr<UINT32> m_objlist;
	optional_shared_ptr<UINT32> m_snd_shared_ram;
	required_shared_ptr<UINT32> m_main_ram;
	required_shared_ptr<UINT16> m_dsp_shared_ram;
	required_shared_ptr<UINT32> m_palette_ram;

	taitojc_renderer *m_renderer;

	int m_texture_x;
	int m_texture_y;

	UINT32 m_dsp_rom_pos;
	UINT16 m_dsp_tex_address;
	UINT16 m_dsp_tex_offset;

	int m_first_dsp_reset;
	INT16 m_viewport_data[3];
	INT16 m_projection_data[3];
	INT16 m_intersection_data[3];

	UINT8 *m_texture;
	bitmap_ind16 m_framebuffer;
	bitmap_ind16 m_zbuffer;

	int m_gfx_index;

	UINT32 *m_char_ram;
	UINT32 *m_tile_ram;
	tilemap_t *m_tilemap;

	UINT16 *m_polygon_fifo;
	int m_polygon_fifo_ptr;

	UINT8 m_mcu_comm_main;
	UINT8 m_mcu_comm_hc11;
	UINT8 m_mcu_data_main;
	UINT8 m_mcu_data_hc11;
	UINT8 m_mcu_output;

	UINT16 m_debug_dsp_ram[0x8000];

	UINT8 m_has_dsp_hack;

	int m_speed_meter;
	int m_brake_meter;

	DECLARE_READ32_MEMBER(mcu_comm_r);
	DECLARE_WRITE32_MEMBER(mcu_comm_w);
	DECLARE_READ32_MEMBER(dsp_shared_r);
	DECLARE_WRITE32_MEMBER(dsp_shared_w);
	DECLARE_READ32_MEMBER(snd_share_r);
	DECLARE_WRITE32_MEMBER(snd_share_w);
	DECLARE_READ8_MEMBER(jc_pcbid_r);
	DECLARE_READ8_MEMBER(jc_lan_r);
	DECLARE_WRITE8_MEMBER(jc_lan_w);
	DECLARE_WRITE8_MEMBER(dendego_speedmeter_w);
	DECLARE_WRITE8_MEMBER(dendego_brakemeter_w);

	DECLARE_READ8_MEMBER(hc11_comm_r);
	DECLARE_WRITE8_MEMBER(hc11_comm_w);
	DECLARE_WRITE8_MEMBER(hc11_output_w);
	DECLARE_READ8_MEMBER(hc11_data_r);
	DECLARE_READ8_MEMBER(hc11_output_r);
	DECLARE_WRITE8_MEMBER(hc11_data_w);
	DECLARE_READ8_MEMBER(hc11_analog_r);

	DECLARE_READ16_MEMBER(dsp_rom_r);
	DECLARE_WRITE16_MEMBER(dsp_rom_w);
	DECLARE_WRITE16_MEMBER(dsp_texture_w);
	DECLARE_READ16_MEMBER(dsp_texaddr_r);
	DECLARE_WRITE16_MEMBER(dsp_texaddr_w);
	DECLARE_WRITE16_MEMBER(dsp_polygon_fifo_w);
	DECLARE_WRITE16_MEMBER(dsp_unk2_w);
	DECLARE_READ16_MEMBER(dsp_to_main_r);
	DECLARE_WRITE16_MEMBER(dsp_to_main_w);

	DECLARE_WRITE16_MEMBER(dsp_math_viewport_w);
	DECLARE_WRITE16_MEMBER(dsp_math_projection_w);
	DECLARE_READ16_MEMBER(dsp_math_projection_y_r);
	DECLARE_READ16_MEMBER(dsp_math_projection_x_r);
	DECLARE_WRITE16_MEMBER(dsp_math_intersection_w);
	DECLARE_READ16_MEMBER(dsp_math_intersection_r);
	DECLARE_READ16_MEMBER(dsp_math_unk_r);

	DECLARE_READ16_MEMBER(taitojc_dsp_idle_skip_r);
	DECLARE_READ16_MEMBER(dendego2_dsp_idle_skip_r);
	DECLARE_WRITE16_MEMBER(dsp_idle_skip_w);

	DECLARE_READ32_MEMBER(taitojc_palette_r);
	DECLARE_WRITE32_MEMBER(taitojc_palette_w);
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
SCREEN_UPDATE_IND16(dendego);

// lookup tables for densha de go analog controls/meters
const int dendego_odometer_table[0x100] =
{
	   0,    3,    7,   10,   14,   17,   21,   24,   28,   31,   34,   38,   41,   45,   48,   52,
	  55,   59,   62,   66,   69,   72,   76,   79,   83,   86,   90,   93,   97,  100,  105,  111,
	 116,  121,  126,  132,  137,  142,  147,  153,  158,  163,  168,  174,  179,  184,  189,  195,
	 200,  206,  211,  217,  222,  228,  233,  239,  244,  250,  256,  261,  267,  272,  278,  283,
	 289,  294,  300,  306,  311,  317,  322,  328,  333,  339,  344,  350,  356,  361,  367,  372,
	 378,  383,  389,  394,  400,  406,  412,  418,  424,  429,  435,  441,  447,  453,  459,  465,
	 471,  476,  482,  488,  494,  500,  505,  511,  516,  521,  526,  532,  537,  542,  547,  553,
	 558,  563,  568,  574,  579,  584,  589,  595,  600,  607,  613,  620,  627,  633,  640,  647,
	 653,  660,  667,  673,  680,  687,  693,  700,  705,  711,  716,  721,  726,  732,  737,  742,
	 747,  753,  758,  763,  768,  774,  779,  784,  789,  795,  800,  806,  812,  818,  824,  829,
	 835,  841,  847,  853,  859,  865,  871,  876,  882,  888,  894,  900,  906,  911,  917,  922,
	 928,  933,  939,  944,  950,  956,  961,  967,  972,  978,  983,  989,  994, 1000, 1005, 1011,
	1016, 1021, 1026, 1032, 1037, 1042, 1047, 1053, 1058, 1063, 1068, 1074, 1079, 1084, 1089, 1095,
	1100, 1107, 1113, 1120, 1127, 1133, 1140, 1147, 1153, 1160, 1167, 1173, 1180, 1187, 1193, 1200,
	1203, 1206, 1209, 1212, 1216, 1219, 1222, 1225, 1228, 1231, 1234, 1238, 1241, 1244, 1247, 1250,
	1253, 1256, 1259, 1262, 1266, 1269, 1272, 1275, 1278, 1281, 1284, 1288, 1291, 1294, 1297, 1300,
};

const int dendego_pressure_table[0x100] =
{
	   0,    0,    0,    0,    5,   10,   14,   19,   24,   29,   33,   38,   43,   48,   52,   57,
	  62,   67,   71,   76,   81,   86,   90,   95,  100,  106,  112,  119,  125,  131,  138,  144,
	 150,  156,  162,  169,  175,  181,  188,  194,  200,  206,  212,  219,  225,  231,  238,  244,
	 250,  256,  262,  269,  275,  281,  288,  294,  300,  306,  312,  318,  324,  329,  335,  341,
	 347,  353,  359,  365,  371,  376,  382,  388,  394,  400,  407,  413,  420,  427,  433,  440,
	 447,  453,  460,  467,  473,  480,  487,  493,  500,  507,  514,  521,  529,  536,  543,  550,
	 557,  564,  571,  579,  586,  593,  600,  607,  614,  621,  629,  636,  643,  650,  657,  664,
	 671,  679,  686,  693,  700,  706,  712,  719,  725,  731,  738,  744,  750,  756,  762,  769,
	 775,  781,  788,  794,  800,  807,  814,  821,  829,  836,  843,  850,  857,  864,  871,  879,
	 886,  893,  900,  907,  914,  921,  929,  936,  943,  950,  957,  964,  971,  979,  986,  993,
	1000, 1008, 1015, 1023, 1031, 1038, 1046, 1054, 1062, 1069, 1077, 1085, 1092, 1100, 1108, 1115,
	1123, 1131, 1138, 1146, 1154, 1162, 1169, 1177, 1185, 1192, 1200, 1207, 1214, 1221, 1229, 1236,
	1243, 1250, 1257, 1264, 1271, 1279, 1286, 1293, 1300, 1307, 1314, 1321, 1329, 1336, 1343, 1350,
	1357, 1364, 1371, 1379, 1386, 1393, 1400, 1407, 1414, 1421, 1429, 1436, 1443, 1450, 1457, 1464,
	1471, 1479, 1486, 1493, 1500, 1504, 1507, 1511, 1515, 1519, 1522, 1526, 1530, 1533, 1537, 1541,
	1544, 1548, 1552, 1556, 1559, 1563, 1567, 1570, 1574, 1578, 1581, 1585, 1589, 1593, 1596, 1600,
};

const int dendego_brake_table[11] = { 0x00, 0x05, 0x1d, 0x35, 0x4d, 0x65, 0x7d, 0x95, 0xad, 0xc5, 0xd4 };

// Mascon must always be in a defined state, Densha de Go 2 in particular returns black screen if the Mascon input is undefined
const ioport_value dendego_mascon_table[6] = { 0x76, 0x67, 0x75, 0x57, 0x73, 0x37 };
