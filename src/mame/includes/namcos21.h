// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/**
 * @file namcos21.h
 */
#ifndef MAME_INCLUDES_NAMCOS21_H
#define MAME_INCLUDES_NAMCOS21_H

#pragma once


#include "machine/namcoio_gearbox.h"
#include "machine/timer.h"
#include "machine/namco_c139.h"
#include "machine/namco_c148.h"
#include "machine/timer.h"
#include "sound/c140.h"
#include "video/c45.h"
#include "machine/namco65.h"
#include "machine/namco68.h"
#include "machine/namco_c67.h"
#include "video/namco_c355spr.h"
#include "video/namcos2_sprite.h"
#include "video/namcos2_roz.h"
#include "video/namcos21_3d.h"
#include "machine/namcos21_dsp.h"


#define NAMCOS21_POLY_FRAME_WIDTH 496
#define NAMCOS21_POLY_FRAME_HEIGHT 480


#define NAMCOS21_NUM_COLORS 0x8000

#define DSP_BUF_MAX (4096*12)
struct dsp_state
{
	unsigned masterSourceAddr;
	uint16_t slaveInputBuffer[DSP_BUF_MAX];
	unsigned slaveBytesAvailable;
	unsigned slaveBytesAdvertised;
	unsigned slaveInputStart;
	uint16_t slaveOutputBuffer[DSP_BUF_MAX];
	unsigned slaveOutputSize;
	uint16_t masterDirectDrawBuffer[256];
	unsigned masterDirectDrawSize;
	int masterFinished;
	int slaveActive;
};


class namcos21_state : public driver_device
{
public:
	enum
	{	/* Namco System21 */
		NAMCOS21_AIRCOMBAT = 0x4000,
		NAMCOS21_STARBLADE,
		NAMCOS21_CYBERSLED,
		NAMCOS21_SOLVALOU,
		NAMCOS21_WINRUN91,
		NAMCOS21_DRIVERS_EYES,
	};

	namcos21_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_master_dsp_code(*this,"master_dsp_code"),
		m_gametype(0),
		m_c67master(*this, "dspmaster"),
		m_c67slave(*this, "dspslave%u", 0U),
		m_namcos21_3d(*this, "namcos21_3d"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_slave(*this, "slave"),
		m_c65(*this, "c65mcu"),
		m_c68(*this, "c68mcu"),
		m_sci(*this, "sci"),
		m_master_intc(*this, "master_intc"),
		m_slave_intc(*this, "slave_intc"),
		m_c140(*this, "c140"),
		m_c355spr(*this, "c355spr"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_audiobank(*this, "audiobank"),
		m_dspram16(*this,"dspram16"),
		m_mpDualPortRAM(*this,"mpdualportram"),
		m_ptrom24(*this,"point24"),
		m_io_gearbox(*this, "gearbox"),
		m_gpu_intc(*this, "gpu_intc"),
		m_namcos21_dsp(*this, "namcos21dsp")
	{ }

	void configure_c148_standard(machine_config &config);
	void driveyes(machine_config &config);
	void winrun(machine_config &config);
	void namcos21(machine_config &config);

	void init_driveyes();
	void init_winrun();
	void init_starblad();
	void init_solvalou();
	void init_cybsled();
	void init_aircomb();

	optional_shared_ptr<uint16_t> m_master_dsp_code;
	int m_mbNeedsKickstart;
	std::unique_ptr<dsp_state> m_mpDspState;

	int m_gametype;

	optional_device<cpu_device> m_c67master;
	optional_device_array<cpu_device,4> m_c67slave;
	optional_device<namcos21_3d_device> m_namcos21_3d;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_slave;
	optional_device<namcoc65_device> m_c65;
	optional_device<namcoc68_device> m_c68;
	optional_device<namco_c139_device> m_sci;
	optional_device<namco_c148_device> m_master_intc;
	optional_device<namco_c148_device> m_slave_intc;
	optional_device<c140_device> m_c140;
	optional_device<namco_c355spr_device> m_c355spr; 
	required_device<palette_device> m_palette;

	optional_device<screen_device> m_screen;
	optional_memory_bank m_audiobank;

	optional_shared_ptr<uint16_t> m_dspram16;
	required_shared_ptr<uint8_t> m_mpDualPortRAM;

	optional_region_ptr<int32_t> m_ptrom24;

	optional_device<namcoio_gearbox_device> m_io_gearbox;
	optional_device<namco_c148_device> m_gpu_intc;
	optional_device<namcos21_dsp_device> m_namcos21_dsp;

	

	std::unique_ptr<uint8_t[]> m_videoram;
	std::unique_ptr<uint8_t[]> m_maskram;

	uint16_t m_video_enable;
	std::unique_ptr<uint8_t[]> m_pointram;
	int m_pointram_idx;
	uint16_t m_pointram_control;
	uint32_t m_pointrom_idx;
	uint8_t m_mPointRomMSB;
	int m_mbPointRomDataAvailable;
	int m_irq_enable;
	uint8_t m_depthcue[2][0x400];

	uint16_t m_winrun_color;
	uint16_t m_winrun_gpu_register[0x10/2];
	DECLARE_READ16_MEMBER(namcos21_video_enable_r);
	DECLARE_WRITE16_MEMBER(namcos21_video_enable_w);
	DECLARE_WRITE16_MEMBER(dspcuskey_w);
	DECLARE_READ16_MEMBER(dspcuskey_r);
	DECLARE_READ16_MEMBER(dspram16_r);
	template<bool maincpu> DECLARE_WRITE16_MEMBER(dspram16_w);
	DECLARE_READ16_MEMBER(dsp_port0_r);
	DECLARE_WRITE16_MEMBER(dsp_port0_w);
	DECLARE_READ16_MEMBER(dsp_port1_r);
	DECLARE_WRITE16_MEMBER(dsp_port1_w);
	DECLARE_READ16_MEMBER(dsp_port2_r);
	DECLARE_WRITE16_MEMBER(dsp_port2_w);
	DECLARE_READ16_MEMBER(dsp_port3_idc_rcv_enable_r);
	DECLARE_WRITE16_MEMBER(dsp_port3_w);
	DECLARE_WRITE16_MEMBER(dsp_port4_w);
	DECLARE_READ16_MEMBER(dsp_port8_r);
	DECLARE_WRITE16_MEMBER(dsp_port8_w);
	DECLARE_READ16_MEMBER(dsp_port9_r);
	DECLARE_READ16_MEMBER(dsp_porta_r);
	DECLARE_WRITE16_MEMBER(dsp_porta_w);
	DECLARE_READ16_MEMBER(dsp_portb_r);
	DECLARE_WRITE16_MEMBER(dsp_portb_w);
	DECLARE_WRITE16_MEMBER(dsp_portc_w);
	DECLARE_READ16_MEMBER(dsp_portf_r);
	DECLARE_WRITE16_MEMBER(dsp_xf_w);
	DECLARE_READ16_MEMBER(slave_port0_r);
	DECLARE_WRITE16_MEMBER(slave_port0_w);
	DECLARE_READ16_MEMBER(slave_port2_r);
	DECLARE_READ16_MEMBER(slave_port3_r);
	DECLARE_WRITE16_MEMBER(slave_port3_w);
	DECLARE_WRITE16_MEMBER(slave_XF_output_w);
	DECLARE_READ16_MEMBER(slave_portf_r);
	DECLARE_WRITE16_MEMBER(pointram_control_w);
	DECLARE_READ16_MEMBER(pointram_data_r);
	DECLARE_WRITE16_MEMBER(pointram_data_w);
	DECLARE_READ16_MEMBER(namcos21_depthcue_r);
	DECLARE_WRITE16_MEMBER(namcos21_depthcue_w);
	DECLARE_READ16_MEMBER(namcos2_68k_dualportram_word_r);
	DECLARE_WRITE16_MEMBER(namcos2_68k_dualportram_word_w);
	DECLARE_READ8_MEMBER(namcos2_dualportram_byte_r);
	DECLARE_WRITE8_MEMBER(namcos2_dualportram_byte_w);


	DECLARE_READ16_MEMBER(winrun_gpu_color_r);
	DECLARE_WRITE16_MEMBER(winrun_gpu_color_w);
	DECLARE_READ16_MEMBER(winrun_gpu_register_r);
	DECLARE_WRITE16_MEMBER(winrun_gpu_register_w);
	DECLARE_WRITE16_MEMBER(winrun_gpu_videoram_w);
	DECLARE_READ16_MEMBER(winrun_gpu_videoram_r);

	DECLARE_WRITE8_MEMBER( namcos2_68k_eeprom_w );
	DECLARE_READ8_MEMBER( namcos2_68k_eeprom_r );

	DECLARE_WRITE8_MEMBER( namcos2_sound_bankselect_w );

	DECLARE_WRITE8_MEMBER(sound_reset_w);
	DECLARE_WRITE8_MEMBER(system_reset_w);
	void reset_all_subcpus(int state);

	std::unique_ptr<uint8_t[]> m_eeprom;

	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

	uint8_t m_gearbox_state;
	DECLARE_CUSTOM_INPUT_MEMBER(driveyes_gearbox_r);

	DECLARE_MACHINE_START(namcos21);
	DECLARE_MACHINE_RESET(namcos21);
	DECLARE_VIDEO_START(namcos21);
	uint32_t screen_update_namcos21(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_winrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_driveyes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void winrun_bitmap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect);

	int32_t read_pointrom_data(unsigned offset);
	void transmit_word_to_slave(uint16_t data);
	void transfer_dsp_data();
	uint16_t read_word_from_slave_input();
	uint16_t get_input_bytes_advertised_for_slave();
	int init_dsp();
	void render_slave_output(uint16_t data);
	void init(int game_type);
	void configure_c65_namcos21(machine_config &config);
	void configure_c68_namcos21(machine_config &config);
	void common_map(address_map &map);
	void driveyes_common_map(address_map &map);
	void driveyes_master_map(address_map &map);
	void driveyes_slave_map(address_map &map);
	void master_dsp_data(address_map &map);
	void master_dsp_io(address_map &map);
	void master_dsp_program(address_map &map);
	void master_map(address_map &map);
	void mcu_map(address_map &map);
	void slave_dsp_data(address_map &map);
	void slave_dsp_io(address_map &map);
	void slave_dsp_program(address_map &map);
	void slave_map(address_map &map);
	void sound_map(address_map &map);

	void winrun_gpu_map(address_map &map);
	void winrun_master_map(address_map &map);
	void winrun_slave_map(address_map &map);
};

#endif // MAME_INCLUDES_NAMCOS21_H
