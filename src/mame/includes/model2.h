// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese
#include "video/poly.h"
#include "audio/dsbz80.h"
#include "audio/segam1audio.h"
#include "machine/eepromser.h"
#include "cpu/i960/i960.h"
#include "sound/scsp.h"
#include "machine/315-5881_crypt.h"
#include "machine/315-5838_317-0229_comp.h"
#include "machine/m2comm.h"

class model2_renderer;
struct raster_state;
struct geo_state;

class model2_state : public driver_device
{
public:
	model2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_bufferram(*this, "bufferram"),
		m_colorxlat(*this, "colorxlat"),
		m_textureram0(*this, "textureram0"),
		m_textureram1(*this, "textureram1"),
		m_lumaram(*this, "lumaram"),
		m_soundram(*this, "soundram"),
		m_tgp_program(*this, "tgp_program"),
		m_tgpx4_program(*this, "tgpx4_program"),
		m_maincpu(*this,"maincpu"),
		m_dsbz80(*this, DSBZ80_TAG),
		m_m1audio(*this, "m1audio"),
		m_m2comm(*this, "m2comm"),
		m_audiocpu(*this, "audiocpu"),
		m_tgp(*this, "tgp"),
		m_dsp(*this, "dsp"),
		m_tgpx4(*this, "tgpx4"),
		m_drivecpu(*this, "drivecpu"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_scsp(*this, "scsp"),
		m_cryptdevice(*this, "315_5881"),
		m_0229crypt(*this, "317_0229"),
		m_in0(*this, "IN0"),
		m_gears(*this, "GEARS"),
		m_analog_ports(*this, {"ANA0", "ANA1", "ANA2", "ANA3"}),
		m_lightgun_ports(*this, {"P1_Y", "P1_X", "P2_Y", "P2_X"})
		{ }

	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_bufferram;
	std::unique_ptr<uint16_t[]> m_palram;
	required_shared_ptr<uint32_t> m_colorxlat;
	required_shared_ptr<uint32_t> m_textureram0;
	required_shared_ptr<uint32_t> m_textureram1;
	required_shared_ptr<uint32_t> m_lumaram;
	optional_shared_ptr<uint16_t> m_soundram;
	optional_shared_ptr<uint32_t> m_tgp_program;
	optional_shared_ptr<uint64_t> m_tgpx4_program;

	required_device<i960_cpu_device> m_maincpu;
	optional_device<dsbz80_device> m_dsbz80;    // Z80-based MPEG Digital Sound Board
	optional_device<segam1audio_device> m_m1audio;  // Model 1 standard sound board
	optional_device<m2comm_device> m_m2comm;        // Model 2 communication board
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_tgp;
	optional_device<cpu_device> m_dsp;
	optional_device<cpu_device> m_tgpx4;
	optional_device<cpu_device> m_drivecpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<scsp_device> m_scsp;
	optional_device<sega_315_5881_crypt_device> m_cryptdevice;
	optional_device<sega_315_5838_comp_device> m_0229crypt;

	required_ioport m_in0;
	optional_ioport m_gears;
	optional_ioport_array<4> m_analog_ports;
	optional_ioport_array<4> m_lightgun_ports;

	uint32_t m_intreq;
	uint32_t m_intena;
	uint32_t m_coproctl;
	uint32_t m_coprocnt;
	uint32_t m_geoctl;
	uint32_t m_geocnt;
	uint32_t m_timervals[4];
	uint32_t m_timerorig[4];
	int m_timerrun[4];
	timer_device *m_timers[4];
	int m_ctrlmode;
	int m_analog_channel;
	int m_dsp_type;
	int m_copro_fifoin_rpos;
	int m_copro_fifoin_wpos;
	std::unique_ptr<uint32_t[]> m_copro_fifoin_data;
	int m_copro_fifoin_num;
	int m_copro_fifoout_rpos;
	int m_copro_fifoout_wpos;
	std::unique_ptr<uint32_t[]> m_copro_fifoout_data;
	int m_copro_fifoout_num;
	uint16_t m_cmd_data;
	uint8_t m_driveio_comm_data;
	int m_iop_write_num;
	uint32_t m_iop_data;
	int m_geo_iop_write_num;
	uint32_t m_geo_iop_data;
	int m_to_68k;

	int m_maxxstate;
	uint32_t m_geo_read_start_address;
	uint32_t m_geo_write_start_address;
	model2_renderer *m_poly;
	raster_state *m_raster;
	geo_state *m_geo;
	bitmap_rgb32 m_sys24_bitmap;
	uint32_t m_videocontrol;
	uint32_t m_soundack;
	void model2_check_irq_state();
	void model2_check_irqack_state(uint32_t data);
	uint8_t m_gearsel;
	uint8_t m_lightgun_mux;

	ioport_value _1c00000_r(ioport_field &field, void *param);
	ioport_value _1c0001c_r(ioport_field &field, void *param);
	ioport_value srallyc_gearbox_r(ioport_field &field, void *param);
	ioport_value rchase2_devices_r(ioport_field &field, void *param);
	uint32_t timers_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void timers_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t model2_palette_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void model2_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ctrl0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void analog_2b_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t fifoctl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t model2o_fifoctrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t videoctl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void videoctl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void rchase2_devices_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void srallyc_devices_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t copro_prg_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void copro_prg_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t copro_ctl1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void copro_ctl1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void copro_function_port_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t copro_fifo_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void copro_fifo_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void copro_sharc_iop_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void geo_ctl1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t geo_prg_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void geo_prg_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t geo_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void geo_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t hotd_lightgun_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void hotd_lightgun_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t daytona_unk_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t model2_irq_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void model2_irq_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t model2_serial_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void model2o_serial_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void model2_serial_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t model2_5881prot_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void model2_5881prot_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	int first_read;

	uint32_t maxx_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void mode_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void model2o_tex_w0(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void model2o_tex_w1(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void model2o_luma_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void model2_3d_zclip_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void model2snd_ctrl(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t copro_sharc_input_fifo_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void copro_sharc_output_fifo_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t copro_sharc_buffer_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void copro_sharc_buffer_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t copro_tgp_buffer_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void copro_tgp_buffer_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t tgpid_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint32_t copro_status_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t polygon_count_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	uint8_t driveio_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void driveio_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t driveio_port_str_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void push_geo_data(uint32_t data);
	void init_overrev();
	void init_pltkids();
	void init_rchase2();
	void init_genprot();
	void init_daytonam();
	void init_manxttdx();
	void init_srallyc();
	void init_doa();
	void init_zerogun();
	void init_sgt24h();
	void machine_start_model2();
	void machine_reset_model2o();
	void video_start_model2();
	void machine_reset_model2();
	void machine_reset_model2b();
	void machine_reset_model2c();
	void machine_reset_model2_common();
	void machine_reset_model2_scsp();
	uint32_t screen_update_model2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void model2_timer_cb(timer_device &timer, void *ptr, int32_t param);
	void model2_interrupt(timer_device &timer, void *ptr, int32_t param);
	void model2c_interrupt(timer_device &timer, void *ptr, int32_t param);
	void scsp_irq(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int copro_tgp_fifoin_pop_ok();
	uint32_t copro_tgp_fifoin_pop(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void copro_tgp_fifoout_push(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t virtuacop_lightgun_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t virtuacop_lightgun_offscreen_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint16_t crypt_read_callback(uint32_t addr);

	bool copro_fifoin_pop(device_t *device, uint32_t *result,uint32_t offset, uint32_t mem_mask);
	void copro_fifoin_push(device_t *device, uint32_t data, uint32_t offset, uint32_t mem_mask);
	uint32_t copro_fifoout_pop(address_space &space, uint32_t offset, uint32_t mem_mask);
	void copro_fifoout_push(device_t *device, uint32_t data,uint32_t offset,uint32_t mem_mask);

	void model2_3d_frame_end( bitmap_rgb32 &bitmap, const rectangle &cliprect );
};


/*****************************
 *
 * Modern polygon renderer
 *
 *****************************/

struct m2_poly_extra_data
{
	model2_state *  state;
	uint32_t      lumabase;
	uint32_t      colorbase;
	uint32_t *    texsheet;
	uint32_t      texwidth;
	uint32_t      texheight;
	uint32_t      texx, texy;
	uint8_t       texmirrorx;
	uint8_t       texmirrory;
};


static inline uint16_t get_texel( uint32_t base_x, uint32_t base_y, int x, int y, uint32_t *sheet )
{
	uint32_t  baseoffs = ((base_y/2)*512)+(base_x/2);
	uint32_t  texeloffs = ((y/2)*512)+(x/2);
	uint32_t  offset = baseoffs + texeloffs;
	uint32_t  texel = sheet[offset>>1];

	if ( offset & 1 )
		texel >>= 16;

	if ( (y & 1) == 0 )
		texel >>= 8;

	if ( (x & 1) == 0 )
		texel >>= 4;

	return (texel & 0x0f);
}

struct triangle;

class model2_renderer : public poly_manager<float, m2_poly_extra_data, 4, 4000>
{
public:
	typedef void (model2_renderer::*scanline_render_func)(int32_t scanline, const extent_t& extent, const m2_poly_extra_data& object, int threadid);

public:
	model2_renderer(model2_state& state)
		: poly_manager<float, m2_poly_extra_data, 4, 4000>(state.machine())
		, m_state(state)
		, m_destmap(state.m_screen->width(), state.m_screen->height())
	{
		m_renderfuncs[0] = &model2_renderer::model2_3d_render_0;
		m_renderfuncs[1] = &model2_renderer::model2_3d_render_1;
		m_renderfuncs[2] = &model2_renderer::model2_3d_render_2;
		m_renderfuncs[3] = &model2_renderer::model2_3d_render_3;
		m_renderfuncs[4] = &model2_renderer::model2_3d_render_4;
		m_renderfuncs[5] = &model2_renderer::model2_3d_render_5;
		m_renderfuncs[6] = &model2_renderer::model2_3d_render_6;
		m_renderfuncs[7] = &model2_renderer::model2_3d_render_7;
	}

	bitmap_rgb32& destmap() { return m_destmap; }

	void model2_3d_render(triangle *tri, const rectangle &cliprect);

	/* checker = 0, textured = 0, transparent = 0 */
	#define MODEL2_FUNC 0
	#define MODEL2_FUNC_NAME    model2_3d_render_0
	#include "video/model2rd.hxx"
	#undef MODEL2_FUNC
	#undef MODEL2_FUNC_NAME

	/* checker = 0, textured = 0, translucent = 1 */
	#define MODEL2_FUNC 1
	#define MODEL2_FUNC_NAME    model2_3d_render_1
	#include "video/model2rd.hxx"
	#undef MODEL2_FUNC
	#undef MODEL2_FUNC_NAME

	/* checker = 0, textured = 1, translucent = 0 */
	#define MODEL2_FUNC 2
	#define MODEL2_FUNC_NAME    model2_3d_render_2
	#include "video/model2rd.hxx"
	#undef MODEL2_FUNC
	#undef MODEL2_FUNC_NAME

	/* checker = 0, textured = 1, translucent = 1 */
	#define MODEL2_FUNC 3
	#define MODEL2_FUNC_NAME    model2_3d_render_3
	#include "video/model2rd.hxx"
	#undef MODEL2_FUNC
	#undef MODEL2_FUNC_NAME

	/* checker = 1, textured = 0, translucent = 0 */
	#define MODEL2_FUNC 4
	#define MODEL2_FUNC_NAME    model2_3d_render_4
	#include "video/model2rd.hxx"
	#undef MODEL2_FUNC
	#undef MODEL2_FUNC_NAME

	/* checker = 1, textured = 0, translucent = 1 */
	#define MODEL2_FUNC 5
	#define MODEL2_FUNC_NAME    model2_3d_render_5
	#include "video/model2rd.hxx"
	#undef MODEL2_FUNC
	#undef MODEL2_FUNC_NAME

	/* checker = 1, textured = 1, translucent = 0 */
	#define MODEL2_FUNC 6
	#define MODEL2_FUNC_NAME    model2_3d_render_6
	#include "video/model2rd.hxx"
	#undef MODEL2_FUNC
	#undef MODEL2_FUNC_NAME

	/* checker = 1, textured = 1, translucent = 1 */
	#define MODEL2_FUNC 7
	#define MODEL2_FUNC_NAME    model2_3d_render_7
	#include "video/model2rd.hxx"
	#undef MODEL2_FUNC
	#undef MODEL2_FUNC_NAME

	scanline_render_func m_renderfuncs[8];

private:
	model2_state& m_state;
	bitmap_rgb32 m_destmap;
};

typedef model2_renderer::vertex_t poly_vertex;


/*******************************************
 *
 *  Basic Data Types
 *
 *******************************************/

struct plane
{
	poly_vertex normal;
	float       distance;
};

struct texture_parameter
{
	float   diffuse;
	float   ambient;
	uint32_t  specular_control;
	float   specular_scale;
};

struct triangle
{
	void *              next;
	poly_vertex         v[3];
	uint16_t              z;
	uint16_t              texheader[4];
	uint8_t               luma;
	int16_t               viewport[4];
	int16_t               center[2];
};

struct quad_m2
{
	poly_vertex         v[4];
	uint16_t              z;
	uint16_t              texheader[4];
	uint8_t               luma;
};



/*----------- defined in video/model2.c -----------*/
void model2_3d_set_zclip( running_machine &machine, uint8_t clip );
