// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese
#ifndef MAME_INCLUDES_MODEL2_H
#define MAME_INCLUDES_MODEL2_H

#pragma once

#include "audio/dsbz80.h"
#include "audio/segam1audio.h"
#include "cpu/i960/i960.h"
#include "cpu/mb86233/mb86233.h"
#include "cpu/sharc/sharc.h"
#include "cpu/mb86235/mb86235.h"
#include "machine/315-5881_crypt.h"
#include "machine/315-5838_317-0229_comp.h"
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "machine/gen_fifo.h"
#include "machine/i8251.h"
#include "machine/m2comm.h"
#include "machine/timer.h"
#include "sound/scsp.h"
#include "video/segaic24.h"
#include "video/poly.h"
#include "emupal.h"
#include "screen.h"

class model2_renderer;
struct raster_state;
struct geo_state;
struct triangle;

class model2_state : public driver_device
{
public:
	model2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_textureram0(*this, "textureram0"),
		m_textureram1(*this, "textureram1"),
		m_workram(*this, "workram"),
		m_bufferram(*this, "bufferram"),
		m_soundram(*this, "soundram"),
		m_maincpu(*this,"maincpu"),
		m_dsbz80(*this, DSBZ80_TAG),
		m_m1audio(*this, M1AUDIO_TAG),
		m_uart(*this, "uart"),
		m_m2comm(*this, "m2comm"),
		m_audiocpu(*this, "audiocpu"),
		m_copro_fifo_in(*this, "copro_fifo_in"),
		m_copro_fifo_out(*this, "copro_fifo_out"),
		m_drivecpu(*this, "drivecpu"),
		m_eeprom(*this, "eeprom"),
		m_tiles(*this, "tile"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_scsp(*this, "scsp"),
		m_timers(*this, "timer%u", 0U),
		m_cryptdevice(*this, "315_5881"),
		m_0229crypt(*this, "317_0229"),
		m_copro_data(*this, "copro_data"),
		m_in0(*this, "IN0"),
		m_gears(*this, "GEARS"),
		m_lightgun_ports(*this, {"P1_Y", "P1_X", "P2_Y", "P2_X"})
	{ }

	/* Public for access by the rendering functions */
	required_shared_ptr<uint32_t> m_textureram0;
	required_shared_ptr<uint32_t> m_textureram1;
	std::unique_ptr<uint16_t[]> m_palram;
	std::unique_ptr<uint16_t[]> m_colorxlat;
	std::unique_ptr<uint16_t[]> m_lumaram;
	uint8_t m_gamma_table[256];
	model2_renderer *m_poly;

	/* Public for access by the ioports */
	DECLARE_CUSTOM_INPUT_MEMBER(daytona_gearbox_r);

	/* Public for access by MCFG */
	TIMER_DEVICE_CALLBACK_MEMBER(model2_interrupt);
	uint16_t crypt_read_callback(uint32_t addr);
	DECLARE_MACHINE_START(model2);


	/* Public for access by GAME() */
	void init_overrev();
	void init_pltkids();
	void init_rchase2();
	void init_manxttdx();
	void init_doa();
	void init_zerogun();
	void init_sgt24h();
	void init_srallyc();
	void init_powsledm();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_bufferram;
	std::unique_ptr<uint16_t[]> m_fbvramA;
	std::unique_ptr<uint16_t[]> m_fbvramB;
	optional_shared_ptr<uint16_t> m_soundram;

	required_device<i960_cpu_device> m_maincpu;
	optional_device<dsbz80_device> m_dsbz80;    // Z80-based MPEG Digital Sound Board
	optional_device<segam1audio_device> m_m1audio;  // Model 1 standard sound board
	required_device<i8251_device> m_uart;
	optional_device<m2comm_device> m_m2comm;        // Model 2 communication board
	optional_device<cpu_device> m_audiocpu;
	required_device<generic_fifo_u32_device> m_copro_fifo_in;
	required_device<generic_fifo_u32_device> m_copro_fifo_out;
	optional_device<cpu_device> m_drivecpu;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<segas24_tile_device> m_tiles;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<scsp_device> m_scsp;
	required_device_array<timer_device, 4> m_timers;
	optional_device<sega_315_5881_crypt_device> m_cryptdevice;
	optional_device<sega_315_5838_comp_device> m_0229crypt;
	optional_memory_region m_copro_data;

	required_ioport m_in0;
	optional_ioport m_gears;
	optional_ioport_array<4> m_lightgun_ports;

	uint32_t m_timervals[4];
	uint32_t m_timerorig[4];
	int m_timerrun[4];
	int m_ctrlmode;
	uint16_t m_cmd_data;
	uint8_t m_driveio_comm_data;
	int m_iop_write_num;
	uint32_t m_iop_data;
	int m_geo_iop_write_num;
	uint32_t m_geo_iop_data;

	uint32_t m_geo_read_start_address;
	uint32_t m_geo_write_start_address;
	raster_state *m_raster;
	geo_state *m_geo;
	bitmap_rgb32 m_sys24_bitmap;
//  uint32_t m_soundack;
	void model2_check_irq_state();
	void model2_check_irqack_state(uint32_t data);
	uint8_t m_gearsel;
	uint8_t m_lightgun_mux;

	// Coprocessor communications
	DECLARE_READ32_MEMBER(copro_prg_r);
	DECLARE_WRITE32_MEMBER(copro_prg_w);
	DECLARE_READ32_MEMBER(copro_ctl1_r);
	DECLARE_WRITE32_MEMBER(copro_ctl1_w);
	DECLARE_READ32_MEMBER(copro_status_r);

	// Geometrizer communications
	DECLARE_WRITE32_MEMBER(geo_ctl1_w);
	DECLARE_READ32_MEMBER(geo_prg_r);
	DECLARE_WRITE32_MEMBER(geo_prg_w);
	DECLARE_READ32_MEMBER(geo_r);
	DECLARE_WRITE32_MEMBER(geo_w);

	// Everything else
	DECLARE_READ32_MEMBER(timers_r);
	DECLARE_WRITE32_MEMBER(timers_w);
	DECLARE_READ16_MEMBER(palette_r);
	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_READ16_MEMBER(colorxlat_r);
	DECLARE_WRITE16_MEMBER(colorxlat_w);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_READ8_MEMBER(in0_r);
	DECLARE_READ32_MEMBER(fifo_control_2a_r);
	DECLARE_READ32_MEMBER(videoctl_r);
	DECLARE_WRITE32_MEMBER(videoctl_w);
	DECLARE_READ8_MEMBER(rchase2_drive_board_r);
	DECLARE_WRITE8_MEMBER(rchase2_drive_board_w);
	DECLARE_WRITE8_MEMBER(drive_board_w);
	DECLARE_READ8_MEMBER(lightgun_data_r);
	DECLARE_READ8_MEMBER(lightgun_mux_r);
	DECLARE_WRITE8_MEMBER(lightgun_mux_w);
	DECLARE_READ8_MEMBER(lightgun_offscreen_r);
	DECLARE_READ32_MEMBER(irq_request_r);
	DECLARE_WRITE32_MEMBER(irq_ack_w);
	DECLARE_READ32_MEMBER(irq_enable_r);
	DECLARE_WRITE32_MEMBER(irq_enable_w);
	DECLARE_READ32_MEMBER(model2_serial_r);
	DECLARE_WRITE32_MEMBER(model2o_serial_w);
	DECLARE_WRITE32_MEMBER(model2_serial_w);
	DECLARE_WRITE16_MEMBER(horizontal_sync_w);
	DECLARE_WRITE16_MEMBER(vertical_sync_w);
	DECLARE_READ32_MEMBER(doa_prot_r);
	DECLARE_READ32_MEMBER(doa_unk_r);
	void sega_0229_map(address_map &map);
	int m_prot_a;

	void raster_init(memory_region *texture_rom);
	void geo_init(memory_region *polygon_rom);
	DECLARE_READ32_MEMBER(render_mode_r);
	DECLARE_WRITE32_MEMBER(render_mode_w);
	DECLARE_READ16_MEMBER(lumaram_r);
	DECLARE_WRITE16_MEMBER(lumaram_w);
	DECLARE_READ16_MEMBER(fbvram_bankA_r);
	DECLARE_WRITE16_MEMBER(fbvram_bankA_w);
	DECLARE_READ16_MEMBER(fbvram_bankB_r);
	DECLARE_WRITE16_MEMBER(fbvram_bankB_w);
	DECLARE_WRITE32_MEMBER(model2_3d_zclip_w);
	DECLARE_WRITE16_MEMBER(model2snd_ctrl);
	DECLARE_READ8_MEMBER(tgpid_r);
	DECLARE_READ32_MEMBER(polygon_count_r);

	DECLARE_READ8_MEMBER(driveio_portg_r);
	DECLARE_READ8_MEMBER(driveio_porth_r);
	DECLARE_WRITE8_MEMBER(driveio_port_w);
	void push_geo_data(uint32_t data);
	DECLARE_VIDEO_START(model2);
	void reset_model2_scsp();
	uint32_t screen_update_model2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
//  DECLARE_WRITE_LINE_MEMBER(screen_vblank_model2);
//  DECLARE_WRITE_LINE_MEMBER(sound_ready_w);
	TIMER_DEVICE_CALLBACK_MEMBER(model2_timer_cb);
	DECLARE_WRITE8_MEMBER(scsp_irq);

	void model2_3d_frame_start( void );
	void geo_parse( void );
	void model2_3d_frame_end( bitmap_rgb32 &bitmap, const rectangle &cliprect );
	void draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect );

	void model2_timers(machine_config &config);
	void model2_screen(machine_config &config);
	void model2_scsp(machine_config &config);

	void sj25_0207_01(machine_config &config);

	void drive_io_map(address_map &map);
	void drive_map(address_map &map);
	void geo_sharc_map(address_map &map);
	void model2_base_mem(address_map &map);
	void model2_5881_mem(address_map &map);
	void model2_0229_mem(address_map &map);
	void model2_snd(address_map &map);
	void scsp_map(address_map &map);

	void debug_init();
	void debug_commands( int ref, const std::vector<std::string> &params );
	void debug_geo_dasm_command(int ref, const std::vector<std::string> &params);
	void debug_tri_dump_command(int ref, const std::vector<std::string> &params);
	void debug_help_command(int ref, const std::vector<std::string> &params);

	virtual void video_start() override;

	uint32_t m_intreq;
	uint32_t m_intena;
	uint32_t m_coproctl;
	uint32_t m_coprocnt;

	virtual void copro_halt() = 0;
	virtual void copro_boot() = 0;

private:
	void tri_list_dump(FILE *dst);

	uint32_t m_geoctl;
	uint32_t m_geocnt;
	uint32_t m_videocontrol;

	bool m_render_unk;
	bool m_render_mode;
	bool m_render_test_mode;
	int16 m_crtc_xoffset, m_crtc_yoffset;

	uint32_t *geo_process_command( geo_state *geo, uint32_t opcode, uint32_t *input, bool *end_code );
	// geo commands
	uint32_t *geo_nop( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_object_data( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_direct_data( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_window_data( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_texture_data( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_polygon_data( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_texture_parameters( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_mode( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_zsort_mode( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_focal_distance( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_light_source( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_matrix_write( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_translate_write( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_data_mem_push( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_test( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_end( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_dummy( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_log_data( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_lod( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_code_upload( geo_state *geo, uint32_t opcode, uint32_t *input );
	uint32_t *geo_code_jump( geo_state *geo, uint32_t opcode, uint32_t *input );
	// geo code drawing paths
	void geo_parse_np_ns( geo_state *geo, uint32_t *input, uint32_t count );
	void geo_parse_np_s( geo_state *geo, uint32_t *input, uint32_t count );
	void geo_parse_nn_ns( geo_state *geo, uint32_t *input, uint32_t count );
	void geo_parse_nn_s( geo_state *geo, uint32_t *input, uint32_t count );

	// raster functions
	// main data input port
	void model2_3d_push( raster_state *raster, uint32_t input );
	// quad & triangle push paths
	void model2_3d_process_quad( raster_state *raster, uint32_t attr );
	void model2_3d_process_triangle( raster_state *raster, uint32_t attr );

	// inliners
	inline void model2_3d_project( triangle *tri );
	inline uint16_t float_to_zval( float floatval );
	inline bool check_culling( raster_state *raster, uint32_t attr, float min_z, float max_z );
};

/*****************************
 *
 * Model 2/2A TGP support
 *
 *****************************/

class model2_tgp_state : public model2_state
{
public:
	model2_tgp_state(const machine_config &mconfig, device_type type, const char *tag)
		: model2_state(mconfig, type, tag),
		  m_copro_tgp(*this, "copro_tgp"),
		  m_copro_tgp_program(*this, "copro_tgp_program"),
		  m_copro_tgp_tables(*this, "copro_tgp_tables"),
		  m_copro_tgp_bank(*this, "copro_tgp_bank")
	{}

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<mb86234_device> m_copro_tgp;
	required_shared_ptr<uint32_t> m_copro_tgp_program;
	required_region_ptr<uint32_t> m_copro_tgp_tables;
	required_device<address_map_bank_device> m_copro_tgp_bank;

	u32 m_copro_tgp_bank_reg;
	u32 m_copro_sincos_base;
	u32 m_copro_inv_base;
	u32 m_copro_isqrt_base;
	u32 m_copro_atan_base[4];

	DECLARE_READ32_MEMBER(copro_tgp_buffer_r);
	DECLARE_WRITE32_MEMBER(copro_tgp_buffer_w);
	DECLARE_WRITE32_MEMBER(copro_function_port_w);
	DECLARE_READ32_MEMBER(copro_fifo_r);
	DECLARE_WRITE32_MEMBER(copro_fifo_w);
	DECLARE_WRITE32_MEMBER(tex0_w);
	DECLARE_WRITE32_MEMBER(tex1_w);

	DECLARE_READ32_MEMBER(copro_tgp_fifoin_pop);
	DECLARE_WRITE32_MEMBER(copro_tgp_fifoout_push);
	DECLARE_WRITE32_MEMBER(copro_tgp_bank_w);
	DECLARE_READ32_MEMBER(copro_tgp_memory_r);
	DECLARE_WRITE32_MEMBER(copro_tgp_memory_w);

	DECLARE_WRITE32_MEMBER(copro_sincos_w);
	DECLARE_READ32_MEMBER(copro_sincos_r);
	DECLARE_WRITE32_MEMBER(copro_inv_w);
	DECLARE_READ32_MEMBER(copro_inv_r);
	DECLARE_WRITE32_MEMBER(copro_isqrt_w);
	DECLARE_READ32_MEMBER(copro_isqrt_r);
	DECLARE_WRITE32_MEMBER(copro_atan_w);
	DECLARE_READ32_MEMBER(copro_atan_r);

	void model2_tgp_mem(address_map &map);

	void copro_tgp_prog_map(address_map &map);
	void copro_tgp_data_map(address_map &map);
	void copro_tgp_bank_map(address_map &map);
	void copro_tgp_io_map(address_map &map);
	void copro_tgp_rf_map(address_map &map);

	virtual void copro_halt() override;
	virtual void copro_boot() override;
};

/*****************************
 *
 * Model 2 support
 *
 *****************************/

class model2o_state : public model2_tgp_state
{
public:
	model2o_state(const machine_config &mconfig, device_type type, const char *tag)
		: model2_tgp_state(mconfig, type, tag)
	{}

	void model2o(machine_config &config);
	void daytona(machine_config &config);
	void desert(machine_config &config);
	void vcop(machine_config &config);

protected:
	DECLARE_READ32_MEMBER(fifo_control_2o_r);
	DECLARE_WRITE8_MEMBER(daytona_output_w);
	DECLARE_WRITE8_MEMBER(desert_output_w);
	DECLARE_WRITE8_MEMBER(vcop_output_w);

	void model2o_mem(address_map &map);
};

/*****************************
 *
 * Daytona To The Maxx
 *
 *****************************/

class model2o_maxx_state : public model2o_state
{
public:
	model2o_maxx_state(const machine_config &mconfig, device_type type, const char *tag)
		: model2o_state(mconfig, type, tag)
	{}

	DECLARE_READ32_MEMBER(maxx_r);
	void daytona_maxx(machine_config &config);
	void model2o_maxx_mem(address_map &map);

private:
	int m_maxxstate;
};

/*****************************
 *
 * Daytona GTX 2004 Edition
 *
 *****************************/

class model2o_gtx_state : public model2o_state
{
public:
	model2o_gtx_state(const machine_config &mconfig, device_type type, const char *tag)
		: model2o_state(mconfig, type, tag)
	{}

	DECLARE_READ8_MEMBER(gtx_r);
	void daytona_gtx(machine_config &config);
	void model2o_gtx_mem(address_map &map);

private:
	int m_gtx_state;
};

/*****************************
 *
 * Model 2A
 *
 *****************************/

class model2a_state : public model2_tgp_state
{
public:
	model2a_state(const machine_config &mconfig, device_type type, const char *tag)
		: model2_tgp_state(mconfig, type, tag)
	{}

	void manxtt(machine_config &config);
	void manxttdx(machine_config &config);
	void model2a(machine_config &config);
	void model2a_0229(machine_config &config);
	void model2a_5881(machine_config &config);
	void srallyc(machine_config &config);
	void vcop2(machine_config &config);
	void skytargt(machine_config &config);
	void zeroguna(machine_config &config);

protected:
	virtual void machine_reset() override;

	void model2a_crx_mem(address_map &map);
	void model2a_5881_mem(address_map &map);
	void model2a_0229_mem(address_map &map);
};

/*****************************
 *
 * Model 2B
 *
 *****************************/

class model2b_state : public model2_state
{
public:
	model2b_state(const machine_config &mconfig, device_type type, const char *tag)
		: model2_state(mconfig, type, tag),
		  m_copro_adsp(*this, "copro_adsp")
	{}

	void model2b(machine_config &config);
	void model2b_0229(machine_config &config);
	void model2b_5881(machine_config &config);
	void indy500(machine_config &config);
	void powsled(machine_config &config);
	void rchase2(machine_config &config);
	void gunblade(machine_config &config);
	void dynabb(machine_config &config);
	void zerogun(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<adsp21062_device> m_copro_adsp;

	DECLARE_WRITE32_MEMBER(copro_function_port_w);
	DECLARE_READ32_MEMBER(copro_fifo_r);
	DECLARE_WRITE32_MEMBER(copro_fifo_w);
	DECLARE_WRITE32_MEMBER(copro_sharc_iop_w);
	DECLARE_READ32_MEMBER(copro_sharc_buffer_r);
	DECLARE_WRITE32_MEMBER(copro_sharc_buffer_w);

	void model2b_crx_mem(address_map &map);
	void model2b_5881_mem(address_map &map);
	void model2b_0229_mem(address_map &map);
	// TODO: split into own class
	void rchase2_iocpu_map(address_map &map);
	void rchase2_ioport_map(address_map &map);

	void copro_sharc_map(address_map &map);

	virtual void copro_halt() override;
	virtual void copro_boot() override;
};

/*****************************
 *
 * Model 2C
 *
 *****************************/

class model2c_state : public model2_state
{
public:
	model2c_state(const machine_config &mconfig, device_type type, const char *tag)
		: model2_state(mconfig, type, tag),
		  m_copro_tgpx4(*this, "copro_tgpx4"),
		  m_copro_tgpx4_program(*this, "copro_tgpx4_program")
	{}

	void model2c(machine_config &config);
	void model2c_5881(machine_config &config);
	void skisuprg(machine_config &config);
	void stcc(machine_config &config);
	void waverunr(machine_config &config);
	void bel(machine_config &config);
	void hotd(machine_config &config);
	void overrev2c(machine_config &config);
	void segawski(machine_config &config);
	void topskatr(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<mb86235_device> m_copro_tgpx4;
	required_shared_ptr<uint64_t> m_copro_tgpx4_program;

	DECLARE_WRITE32_MEMBER(copro_function_port_w);
	DECLARE_READ32_MEMBER(copro_fifo_r);
	DECLARE_WRITE32_MEMBER(copro_fifo_w);

	TIMER_DEVICE_CALLBACK_MEMBER(model2c_interrupt);

	void model2c_crx_mem(address_map &map);
	void model2c_5881_mem(address_map &map);
	void copro_tgpx4_map(address_map &map);
	void copro_tgpx4_data_map(address_map &map);

	virtual void copro_halt() override;
	virtual void copro_boot() override;
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

// 0x10000 = size of the tri_sorted_list array
class model2_renderer : public poly_manager<float, m2_poly_extra_data, 4, 0x10000>
{
public:
	typedef void (model2_renderer::*scanline_render_func)(int32_t scanline, const extent_t& extent, const m2_poly_extra_data& object, int threadid);

public:
	model2_renderer(model2_state& state)
		: poly_manager<float, m2_poly_extra_data, 4, 0x10000>(state.machine())
		, m_state(state)
		, m_destmap(512, 512)
	{
		m_renderfuncs[0] = &model2_renderer::model2_3d_render_0;
		m_renderfuncs[1] = &model2_renderer::model2_3d_render_1;
		m_renderfuncs[2] = &model2_renderer::model2_3d_render_2;
		m_renderfuncs[3] = &model2_renderer::model2_3d_render_3;
		m_renderfuncs[4] = &model2_renderer::model2_3d_render_4;
		m_renderfuncs[5] = &model2_renderer::model2_3d_render_5;
		m_renderfuncs[6] = &model2_renderer::model2_3d_render_6;
		m_renderfuncs[7] = &model2_renderer::model2_3d_render_7;
		m_xoffs = 90;
		m_yoffs = -8;
	}

	bitmap_rgb32& destmap() { return m_destmap; }

	void model2_3d_render(triangle *tri, const rectangle &cliprect);
	void set_xoffset(int16 xoffs) { m_xoffs = xoffs; }
	void set_yoffset(int16 yoffs) { m_yoffs = yoffs; }

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
	int16_t m_xoffs,m_yoffs;
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

#endif // MAME_INCLUDES_MODEL2_H
