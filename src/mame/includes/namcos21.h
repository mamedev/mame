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
#include "machine/namcos21_dsp_c67.h"


#define NAMCOS21_POLY_FRAME_WIDTH 496
#define NAMCOS21_POLY_FRAME_HEIGHT 480


#define NAMCOS21_NUM_COLORS 0x8000




class namcos21_state : public driver_device
{
public:
	namcos21_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
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
		m_mpDualPortRAM(*this,"mpdualportram"),
		m_io_gearbox(*this, "gearbox"),
		m_gpu_intc(*this, "gpu_intc"),
		m_namcos21_dsp(*this, "namcos21dsp"),
		m_namcos21_dsp_c67(*this, "namcos21dsp_c67")
	{ }

	void configure_c148_standard(machine_config &config);
	void driveyes(machine_config &config);
	void winrun(machine_config &config);
	void namcos21(machine_config &config);
	void cybsled(machine_config &config);
	void solvalou(machine_config &config);
	void aircomb(machine_config &config);
	void starblad(machine_config &config);

	void init_winrun();
	void init_solvalou();

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

	required_shared_ptr<uint8_t> m_mpDualPortRAM;

	optional_device<namcoio_gearbox_device> m_io_gearbox;
	optional_device<namco_c148_device> m_gpu_intc;
	optional_device<namcos21_dsp_device> m_namcos21_dsp;
	optional_device<namcos21_dsp_c67_device> m_namcos21_dsp_c67;

	std::unique_ptr<uint8_t[]> m_gpu_videoram;
	std::unique_ptr<uint8_t[]> m_gpu_maskram;

	uint16_t m_video_enable;

	uint16_t m_winrun_color;
	uint16_t m_winrun_gpu_register[0x10/2];
	DECLARE_READ16_MEMBER(namcos21_video_enable_r);
	DECLARE_WRITE16_MEMBER(namcos21_video_enable_w);

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

	void configure_c65_namcos21(machine_config &config);
	void configure_c68_namcos21(machine_config &config);

	void driveyes_common_map(address_map &map);
	void driveyes_master_map(address_map &map);
	void driveyes_slave_map(address_map &map);

	void common_map(address_map &map);
	void master_map(address_map &map);
	void slave_map(address_map &map);

	void winrun_master_map(address_map &map);
	void winrun_slave_map(address_map &map);

	void winrun_gpu_map(address_map &map);

	void mcu_map(address_map &map);

	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_NAMCOS21_H
