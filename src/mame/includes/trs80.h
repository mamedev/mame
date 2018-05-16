// license:BSD-3-Clause
// copyright-holders:Robbbert and unknown others
/*****************************************************************************
 *
 * includes/trs80.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_TRS80_H
#define MAME_INCLUDES_TRS80_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "imagedev/snapquik.h"
#include "machine/ay31015.h"
#include "machine/buffer.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "sound/wave.h"

#include "formats/trs_cas.h"


class trs80_state : public driver_device
{
public:
	trs80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_region_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cent_status_in(*this, "cent_status_in")
		, m_uart(*this, "uart")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy2(*this, "fdc:2")
		, m_floppy3(*this, "fdc:3")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "LINE%u", 0)
		, m_bank1(nullptr)
		, m_bank2(nullptr)
		, m_bank3(nullptr)
		, m_bank4(nullptr)
		, m_bank5(nullptr)
		, m_bank6(nullptr)
		, m_bank7(nullptr)
		, m_bank8(nullptr)
		, m_bank9(nullptr)
		, m_bank11(nullptr)
		, m_bank12(nullptr)
		, m_bank13(nullptr)
		, m_bank14(nullptr)
		, m_bank15(nullptr)
		, m_bank16(nullptr)
		, m_bank17(nullptr)
		, m_bank18(nullptr)
		, m_bank19(nullptr)
		{ }

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_WRITE8_MEMBER ( trs80_ff_w );
	DECLARE_WRITE8_MEMBER ( lnw80_fe_w );
	DECLARE_WRITE8_MEMBER ( sys80_fe_w );
	DECLARE_WRITE8_MEMBER ( sys80_f8_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_ff_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_f4_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_ec_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_ea_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_e9_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_e8_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_e4_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_e0_w );
	DECLARE_WRITE8_MEMBER ( trs80m4p_9c_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_90_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_84_w );
	DECLARE_READ8_MEMBER ( lnw80_fe_r );
	DECLARE_READ8_MEMBER ( trs80_ff_r );
	DECLARE_READ8_MEMBER ( sys80_f9_r );
	DECLARE_READ8_MEMBER ( trs80m4_ff_r );
	DECLARE_READ8_MEMBER ( trs80m4_ec_r );
	DECLARE_READ8_MEMBER ( trs80m4_ea_r );
	DECLARE_READ8_MEMBER ( trs80m4_e8_r );
	DECLARE_READ8_MEMBER ( trs80m4_e4_r );
	DECLARE_READ8_MEMBER ( trs80m4_e0_r );
	DECLARE_READ8_MEMBER( trs80_irq_status_r );
	DECLARE_READ8_MEMBER( trs80_printer_r );
	DECLARE_WRITE8_MEMBER( trs80_printer_w );
	DECLARE_WRITE8_MEMBER( trs80_cassunit_w );
	DECLARE_WRITE8_MEMBER( trs80_motor_w );
	DECLARE_READ8_MEMBER( trs80_keyboard_r );
	DECLARE_WRITE8_MEMBER ( trs80m4_88_w );
	DECLARE_READ8_MEMBER( trs80_videoram_r );
	DECLARE_WRITE8_MEMBER( trs80_videoram_w );
	DECLARE_READ8_MEMBER( trs80_gfxram_r );
	DECLARE_WRITE8_MEMBER( trs80_gfxram_w );
	DECLARE_READ8_MEMBER (trs80_wd179x_r);
	DECLARE_READ8_MEMBER (cp500_a11_flipflop_toggle);
	void init_trs80m4();
	void init_trs80l2();
	void init_trs80m4p();
	void init_lnw80();
	void init_trs80();
	INTERRUPT_GEN_MEMBER(trs80_rtc_interrupt);
	INTERRUPT_GEN_MEMBER(trs80_fdc_interrupt);
	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	DECLARE_WRITE_LINE_MEMBER(trs80_fdc_intrq_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER( trs80_cmd );
	DECLARE_MACHINE_RESET(trs80m4);
	DECLARE_MACHINE_RESET(lnw80);
	DECLARE_MACHINE_RESET(cp500);
	DECLARE_PALETTE_INIT(lnw80);
	uint32_t screen_update_trs80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_trs80m4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ht1080z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lnw80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_radionic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_meritum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sys80(machine_config &config);
	void trs80(machine_config &config);
	void lnw80(machine_config &config);
	void model4p(machine_config &config);
	void meritum(machine_config &config);
	void model3(machine_config &config);
	void radionic(machine_config &config);
	void model1(machine_config &config);
	void ht1080z(machine_config &config);
	void cp500(machine_config &config);
	void model4(machine_config &config);
	void cp500_io(address_map &map);
	void lnw80_io(address_map &map);
	void lnw80_map(address_map &map);
	void meritum_io(address_map &map);
	void meritum_map(address_map &map);
	void model1_io(address_map &map);
	void model1_map(address_map &map);
	void model3_io(address_map &map);
	void model3_map(address_map &map);
	void model4_io(address_map &map);
	void model4p_io(address_map &map);
	void sys80_io(address_map &map);
	void trs80_io(address_map &map);
	void trs80_map(address_map &map);
private:
	uint8_t *m_p_gfxram;
	uint8_t m_model4;
	uint8_t m_mode;
	uint8_t m_irq;
	uint8_t m_mask;
	uint8_t m_nmi_mask;
	uint8_t m_port_ec;
	uint8_t m_tape_unit;
	uint8_t m_reg_load;
	uint8_t m_nmi_data;
#ifdef USE_TRACK
	uint8_t m_track[4];
#endif
	uint8_t m_head;
#ifdef USE_SECTOR
	uint8_t m_sector[4];
#endif
	uint8_t m_cassette_data;
	emu_timer *m_cassette_data_timer;
	double m_old_cassette_val;
	uint16_t m_start_address;
	uint8_t m_crtc_reg;
	uint8_t m_size_store;
	bool m_a11_flipflop;
	void trs80_fdc_interrupt_internal();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
	required_memory_region m_region_maincpu;
	required_region_ptr<u8> m_p_chargen;
	optional_shared_ptr<uint8_t> m_p_videoram;
	optional_device<centronics_device> m_centronics;
	optional_device<output_latch_device> m_cent_data_out;
	optional_device<input_buffer_device> m_cent_status_in;
	optional_device<ay31015_device> m_uart;
	optional_device<fd1793_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	optional_device<floppy_connector> m_floppy2;
	optional_device<floppy_connector> m_floppy3;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;
	memory_bank *m_bank1;
	memory_bank *m_bank2;
	memory_bank *m_bank3;
	memory_bank *m_bank4;
	memory_bank *m_bank5;
	memory_bank *m_bank6;
	memory_bank *m_bank7;
	memory_bank *m_bank8;
	memory_bank *m_bank9;
	memory_bank *m_bank11;
	memory_bank *m_bank12;
	memory_bank *m_bank13;
	memory_bank *m_bank14;
	memory_bank *m_bank15;
	memory_bank *m_bank16;
	memory_bank *m_bank17;
	memory_bank *m_bank18;
	memory_bank *m_bank19;
};

#endif // MAME_INCLUDES_TRS80_H
