// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese
#include "video/polylgcy.h"
#include "audio/dsbz80.h"
#include "audio/segam1audio.h"
#include "machine/eepromser.h"
#include "cpu/i960/i960.h"
#include "sound/scsp.h"
#include "machine/315-5881_crypt.h"
#include "machine/315-5838_317-0229_comp.h"

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
		m_0229crypt(*this, "317_0229")

		{ }

	required_shared_ptr<UINT32> m_workram;
	required_shared_ptr<UINT32> m_bufferram;
	UINT16 *m_palram;
	required_shared_ptr<UINT32> m_colorxlat;
	required_shared_ptr<UINT32> m_textureram0;
	required_shared_ptr<UINT32> m_textureram1;
	required_shared_ptr<UINT32> m_lumaram;
	optional_shared_ptr<UINT16> m_soundram;
	optional_shared_ptr<UINT32> m_tgp_program;
	optional_shared_ptr<UINT32> m_tgpx4_program;

	required_device<i960_cpu_device> m_maincpu;
	optional_device<dsbz80_device> m_dsbz80;    // Z80-based MPEG Digital Sound Board
	optional_device<segam1audio_device> m_m1audio;  // Model 1 standard sound board
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

	UINT32 m_intreq;
	UINT32 m_intena;
	UINT32 m_coproctl;
	UINT32 m_coprocnt;
	UINT32 m_geoctl;
	UINT32 m_geocnt;
	UINT32 m_timervals[4];
	UINT32 m_timerorig[4];
	int m_timerrun[4];
	timer_device *m_timers[4];
	int m_ctrlmode;
	int m_analog_channel;
	int m_dsp_type;
	int m_copro_fifoin_rpos;
	int m_copro_fifoin_wpos;
	UINT32 *m_copro_fifoin_data;
	int m_copro_fifoin_num;
	int m_copro_fifoout_rpos;
	int m_copro_fifoout_wpos;
	UINT32 *m_copro_fifoout_data;
	int m_copro_fifoout_num;
	UINT16 m_cmd_data;
	UINT8 m_driveio_comm_data;
	int m_iop_write_num;
	UINT32 m_iop_data;
	int m_geo_iop_write_num;
	UINT32 m_geo_iop_data;
	int m_to_68k;

	int m_maxxstate;
	UINT32 m_netram[0x8000/4];
	int m_zflagi;
	int m_zflag;
	int m_sysres;
	int m_jnet_time_out;
	UINT32 m_geo_read_start_address;
	UINT32 m_geo_write_start_address;
	legacy_poly_manager *m_poly;
	raster_state *m_raster;
	geo_state *m_geo;
	bitmap_rgb32 m_sys24_bitmap;
	UINT32 m_videocontrol;
	UINT32 m_soundack;
	void model2_check_irq_state();
	void model2_check_irqack_state(UINT32 data);
	UINT8 m_gearsel;
	UINT8 m_lightgun_mux;

	DECLARE_CUSTOM_INPUT_MEMBER(_1c00000_r);
	DECLARE_CUSTOM_INPUT_MEMBER(_1c0001c_r);
	DECLARE_CUSTOM_INPUT_MEMBER(srallyc_gearbox_r);
	DECLARE_CUSTOM_INPUT_MEMBER(rchase2_devices_r);
	DECLARE_READ32_MEMBER(timers_r);
	DECLARE_WRITE32_MEMBER(timers_w);
	DECLARE_READ16_MEMBER(model2_palette_r);
	DECLARE_WRITE16_MEMBER(model2_palette_w);
	DECLARE_WRITE32_MEMBER(ctrl0_w);
	DECLARE_WRITE32_MEMBER(analog_2b_w);
	DECLARE_READ32_MEMBER(fifoctl_r);
	DECLARE_READ32_MEMBER(model2o_fifoctrl_r);
	DECLARE_READ32_MEMBER(videoctl_r);
	DECLARE_WRITE32_MEMBER(videoctl_w);
	DECLARE_WRITE32_MEMBER(rchase2_devices_w);
	DECLARE_WRITE32_MEMBER(srallyc_devices_w);
	DECLARE_READ32_MEMBER(copro_prg_r);
	DECLARE_WRITE32_MEMBER(copro_prg_w);
	DECLARE_READ32_MEMBER(copro_ctl1_r);
	DECLARE_WRITE32_MEMBER(copro_ctl1_w);
	DECLARE_WRITE32_MEMBER(copro_function_port_w);
	DECLARE_READ32_MEMBER(copro_fifo_r);
	DECLARE_WRITE32_MEMBER(copro_fifo_w);
	DECLARE_WRITE32_MEMBER(copro_sharc_iop_w);
	DECLARE_WRITE32_MEMBER(geo_ctl1_w);
	DECLARE_WRITE32_MEMBER(geo_sharc_ctl1_w);
	DECLARE_READ32_MEMBER(geo_sharc_fifo_r);
	DECLARE_WRITE32_MEMBER(geo_sharc_fifo_w);
	DECLARE_WRITE32_MEMBER(geo_sharc_iop_w);
	DECLARE_READ32_MEMBER(geo_prg_r);
	DECLARE_WRITE32_MEMBER(geo_prg_w);
	DECLARE_READ32_MEMBER(geo_r);
	DECLARE_WRITE32_MEMBER(geo_w);
	DECLARE_READ32_MEMBER(hotd_lightgun_r);
	DECLARE_WRITE32_MEMBER(hotd_lightgun_w);
	DECLARE_READ32_MEMBER(sonic_unk_r);
	DECLARE_READ32_MEMBER(daytona_unk_r);
	DECLARE_READ32_MEMBER(desert_unk_r);
	DECLARE_READ32_MEMBER(model2_irq_r);
	DECLARE_WRITE32_MEMBER(model2_irq_w);
	DECLARE_READ32_MEMBER(model2_serial_r);
	DECLARE_WRITE32_MEMBER(model2o_serial_w);
	DECLARE_WRITE32_MEMBER(model2_serial_w);
	DECLARE_READ32_MEMBER(model2_5881prot_r);
	DECLARE_WRITE32_MEMBER(model2_5881prot_w);
	int first_read;

	DECLARE_READ32_MEMBER(maxx_r);
	DECLARE_READ32_MEMBER(network_r);
	DECLARE_WRITE32_MEMBER(network_w);
	DECLARE_WRITE32_MEMBER(copro_w);
	DECLARE_WRITE32_MEMBER(mode_w);
	DECLARE_WRITE32_MEMBER(model2o_tex_w0);
	DECLARE_WRITE32_MEMBER(model2o_tex_w1);
	DECLARE_WRITE32_MEMBER(model2o_luma_w);
	DECLARE_WRITE32_MEMBER(model2_3d_zclip_w);
	DECLARE_READ16_MEMBER(m1_snd_68k_latch_r);
	DECLARE_READ16_MEMBER(m1_snd_v60_ready_r);
	DECLARE_WRITE16_MEMBER(m1_snd_68k_latch1_w);
	DECLARE_WRITE16_MEMBER(m1_snd_68k_latch2_w);
	DECLARE_WRITE16_MEMBER(model2snd_ctrl);
	DECLARE_READ32_MEMBER(copro_sharc_input_fifo_r);
	DECLARE_WRITE32_MEMBER(copro_sharc_output_fifo_w);
	DECLARE_READ32_MEMBER(copro_sharc_buffer_r);
	DECLARE_WRITE32_MEMBER(copro_sharc_buffer_w);
	DECLARE_READ32_MEMBER(copro_tgp_buffer_r);
	DECLARE_WRITE32_MEMBER(copro_tgp_buffer_w);
	DECLARE_READ8_MEMBER(tgpid_r);
	DECLARE_READ32_MEMBER(copro_status_r);
	DECLARE_READ32_MEMBER(polygon_count_r);

	DECLARE_READ8_MEMBER(driveio_port_r);
	DECLARE_WRITE8_MEMBER(driveio_port_w);
	DECLARE_READ8_MEMBER(driveio_port_str_r);
	DECLARE_READ32_MEMBER(jaleco_network_r);
	DECLARE_WRITE32_MEMBER(jaleco_network_w);
	void push_geo_data(UINT32 data);
	DECLARE_WRITE16_MEMBER(m1_snd_mpcm_bnk1_w);
	DECLARE_WRITE16_MEMBER(m1_snd_mpcm_bnk2_w);
	DECLARE_DRIVER_INIT(overrev);
	DECLARE_DRIVER_INIT(pltkids);
	DECLARE_DRIVER_INIT(rchase2);
	DECLARE_DRIVER_INIT(genprot);
	DECLARE_DRIVER_INIT(daytonam);
	DECLARE_DRIVER_INIT(manxttdx);
	DECLARE_DRIVER_INIT(srallyc);
	DECLARE_DRIVER_INIT(doa);
	DECLARE_DRIVER_INIT(zerogun);
	DECLARE_DRIVER_INIT(sgt24h);
	DECLARE_MACHINE_START(model2);
	DECLARE_MACHINE_RESET(model2o);
	DECLARE_VIDEO_START(model2);
	DECLARE_MACHINE_RESET(model2);
	DECLARE_MACHINE_RESET(model2b);
	DECLARE_MACHINE_RESET(model2c);
	DECLARE_MACHINE_RESET(model2_common);
	DECLARE_MACHINE_RESET(model2_scsp);
	UINT32 screen_update_model2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(model2_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(model2_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(model2c_interrupt);
	void model2_exit();
	DECLARE_WRITE8_MEMBER(scsp_irq);
	DECLARE_READ_LINE_MEMBER(copro_tgp_fifoin_pop_ok);
	DECLARE_READ32_MEMBER(copro_tgp_fifoin_pop);
	DECLARE_WRITE32_MEMBER(copro_tgp_fifoout_push);
	DECLARE_READ8_MEMBER(virtuacop_lightgun_r);
	DECLARE_READ8_MEMBER(virtuacop_lightgun_offscreen_r);

	UINT16 crypt_read_callback(UINT32 addr);

	bool copro_fifoin_pop(device_t *device, UINT32 *result,UINT32 offset, UINT32 mem_mask);
	void copro_fifoin_push(device_t *device, UINT32 data, UINT32 offset, UINT32 mem_mask);
	UINT32 copro_fifoout_pop(address_space &space, UINT32 offset, UINT32 mem_mask);
	void copro_fifoout_push(device_t *device, UINT32 data,UINT32 offset,UINT32 mem_mask);

	void model2_3d_frame_end( bitmap_rgb32 &bitmap, const rectangle &cliprect );
};

/*----------- defined in video/model2.c -----------*/
void model2_3d_set_zclip( running_machine &machine, UINT8 clip );
