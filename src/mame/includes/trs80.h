// license:BSD-3-Clause
// copyright-holders:Robbbert and unknown others
/*****************************************************************************
 *
 * includes/trs80.h
 *
 ****************************************************************************/

#ifndef TRS80_H_
#define TRS80_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "machine/ay31015.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "machine/wd_fdc.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "imagedev/snapquik.h"
#include "formats/trs_cas.h"


class trs80_state : public driver_device
{
public:
	trs80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_cent_status_in(*this, "cent_status_in"),
		m_ay31015(*this, "tr1602"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
		m_floppy2(*this, "fdc:2"),
		m_floppy3(*this, "fdc:3"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_p_videoram(*this, "p_videoram"),
		m_region_maincpu(*this, "maincpu"),
		m_io_config(*this, "CONFIG"),
		m_io_line0(*this, "LINE0"),
		m_io_line1(*this, "LINE1"),
		m_io_line2(*this, "LINE2"),
		m_io_line3(*this, "LINE3"),
		m_io_line4(*this, "LINE4"),
		m_io_line5(*this, "LINE5"),
		m_io_line6(*this, "LINE6"),
		m_io_line7(*this, "LINE7"),
		m_bank1(nullptr),
		m_bank2(nullptr),
		m_bank3(nullptr),
		m_bank4(nullptr),
		m_bank5(nullptr),
		m_bank6(nullptr),
		m_bank7(nullptr),
		m_bank8(nullptr),
		m_bank9(nullptr),
		m_bank11(nullptr),
		m_bank12(nullptr),
		m_bank13(nullptr),
		m_bank14(nullptr),
		m_bank15(nullptr),
		m_bank16(nullptr),
		m_bank17(nullptr),
		m_bank18(nullptr),
		m_bank19(nullptr) { }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<cpu_device> m_maincpu;
	optional_device<centronics_device> m_centronics;
	optional_device<output_latch_device> m_cent_data_out;
	optional_device<input_buffer_device> m_cent_status_in;
	optional_device<ay31015_device> m_ay31015;
	optional_device<fd1793_t> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	optional_device<floppy_connector> m_floppy2;
	optional_device<floppy_connector> m_floppy3;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	DECLARE_WRITE8_MEMBER ( trs80_ff_w );
	DECLARE_WRITE8_MEMBER ( lnw80_fe_w );
	DECLARE_WRITE8_MEMBER ( sys80_fe_w );
	DECLARE_WRITE8_MEMBER ( sys80_f8_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_ff_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_f4_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_ec_w );
	DECLARE_WRITE8_MEMBER ( trs80m4_eb_w );
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
	DECLARE_READ8_MEMBER ( trs80m4_eb_r );
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
	const UINT8 *m_p_chargen;
	optional_shared_ptr<UINT8> m_p_videoram;
	UINT8 *m_p_gfxram;
	UINT8 m_model4;
	UINT8 m_mode;
	UINT8 m_irq;
	UINT8 m_mask;
	UINT8 m_nmi_mask;
	UINT8 m_port_ec;
	UINT8 m_tape_unit;
	UINT8 m_reg_load;
	UINT8 m_nmi_data;
#ifdef USE_TRACK
	UINT8 m_track[4];
#endif
	UINT8 m_head;
#ifdef USE_SECTOR
	UINT8 m_sector[4];
#endif
	UINT8 m_cassette_data;
	emu_timer *m_cassette_data_timer;
	double m_old_cassette_val;
	UINT16 m_start_address;
	UINT8 m_crtc_reg;
	UINT8 m_size_store;
	DECLARE_DRIVER_INIT(trs80m4);
	DECLARE_DRIVER_INIT(trs80l2);
	DECLARE_DRIVER_INIT(trs80m4p);
	DECLARE_DRIVER_INIT(lnw80);
	DECLARE_DRIVER_INIT(trs80);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(trs80m4);
	DECLARE_MACHINE_RESET(lnw80);
	DECLARE_PALETTE_INIT(lnw80);
	UINT32 screen_update_trs80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_trs80m4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ht1080z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_lnw80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_radionic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_meritum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(trs80_rtc_interrupt);
	INTERRUPT_GEN_MEMBER(trs80_fdc_interrupt);
	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	DECLARE_WRITE_LINE_MEMBER(trs80_fdc_intrq_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER( trs80_cmd );

protected:
	required_memory_region m_region_maincpu;
	required_ioport m_io_config;
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
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

	void trs80_fdc_interrupt_internal();
};

#endif  /* TRS80_H_ */
