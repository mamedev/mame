// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Robbbert
//*****************************************************************************

#ifndef MAME_INCLUDES_TRS80_H
#define MAME_INCLUDES_TRS80_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "bus/rs232/rs232.h"
#include "machine/buffer.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "sound/wave.h"
#include "emupal.h"

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
		, m_p_gfxram(*this, "gfxram")  // LNW80 only
		, m_lnw_bank(*this, "lnw_banked_mem")  // LNW80 only
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cent_status_in(*this, "cent_status_in")
		, m_uart(*this, "uart")
		, m_uart_clock(*this, "uart_clock")
		, m_ppi(*this, "ppi")  // Radionic only
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy2(*this, "fdc:2")
		, m_floppy3(*this, "fdc:3")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_baud(*this, "BAUD")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "LINE%u", 0)
	{ }

	void sys80(machine_config &config);
	void sys80p(machine_config &config);
	void trs80(machine_config &config);
	void lnw80(machine_config &config);
	void radionic(machine_config &config);
	void model1(machine_config &config);
	void ht1080z(machine_config &config);

	void init_trs80l2();
	void init_trs80();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_WRITE8_MEMBER(port_ff_w);
	DECLARE_WRITE8_MEMBER(lnw80_fe_w);
	DECLARE_WRITE8_MEMBER(sys80_fe_w);
	DECLARE_WRITE8_MEMBER(sys80_f8_w);
	DECLARE_WRITE8_MEMBER(port_ea_w);
	DECLARE_WRITE8_MEMBER(port_e8_w);
	DECLARE_READ8_MEMBER(lnw80_fe_r);
	DECLARE_READ8_MEMBER(port_ff_r);
	DECLARE_READ8_MEMBER(sys80_f9_r);
	DECLARE_READ8_MEMBER(port_ea_r);
	DECLARE_READ8_MEMBER(port_e8_r);
	DECLARE_READ8_MEMBER(irq_status_r);
	DECLARE_READ8_MEMBER(printer_r);
	DECLARE_WRITE8_MEMBER(printer_w);
	DECLARE_WRITE8_MEMBER(cassunit_w);
	DECLARE_WRITE8_MEMBER(motor_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(wd179x_r);

	INTERRUPT_GEN_MEMBER(rtc_interrupt);
	INTERRUPT_GEN_MEMBER(fdc_interrupt);
	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER( trs80_cmd );
	DECLARE_MACHINE_RESET(lnw80);
	void lnw80_palette(palette_device &palette) const;
	uint32_t screen_update_trs80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ht1080z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lnw80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_radionic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void lnw80_io(address_map &map);
	void lnw80_mem(address_map &map);
	void lnw_banked_mem(address_map &map);
	void m1_io(address_map &map);
	void m1_mem(address_map &map);
	void sys80_io(address_map &map);
	void trs80_io(address_map &map);
	void trs80_mem(address_map &map);
	void ht1080z_io(address_map &map);
	void radionic_mem(address_map &map);

	uint8_t m_mode;
	uint8_t m_irq;
	uint8_t m_mask;
	uint8_t m_tape_unit;
	bool m_reg_load;
	u8 m_lnw_mode;
	bool m_cassette_data;
	emu_timer *m_cassette_data_timer;
	double m_old_cassette_val;
	uint8_t m_size_store;
	uint16_t m_timeout;
	floppy_image_device *m_floppy;
	required_device<cpu_device> m_maincpu;
	required_memory_region m_region_maincpu;
	required_region_ptr<u8> m_p_chargen;
	optional_shared_ptr<u8> m_p_videoram;
	optional_shared_ptr<u8> m_p_gfxram;
	optional_device<address_map_bank_device> m_lnw_bank;
	optional_device<centronics_device> m_centronics;
	optional_device<output_latch_device> m_cent_data_out;
	optional_device<input_buffer_device> m_cent_status_in;
	optional_device<ay31015_device> m_uart;
	optional_device<clock_device> m_uart_clock;
	optional_device<i8255_device> m_ppi;
	optional_device<fd1793_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	optional_device<floppy_connector> m_floppy2;
	optional_device<floppy_connector> m_floppy3;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	optional_ioport m_io_baud;
	optional_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;
};

#endif // MAME_INCLUDES_TRS80_H
