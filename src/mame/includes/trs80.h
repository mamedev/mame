// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Robbbert
//*****************************************************************************

#ifndef MAME_INCLUDES_TRS80_H
#define MAME_INCLUDES_TRS80_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/com8116.h"
#include "bus/rs232/rs232.h"
#include "machine/buffer.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "formats/trs80_dsk.h"
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
		, m_uart_clock(*this, "uart_clock")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "floppy%u", 0U)
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_baud(*this, "BAUD")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "LINE%u", 0U)
	{ }

	void sys80(machine_config &config);
	void sys80p(machine_config &config);
	void level1(machine_config &config);
	void level2(machine_config &config);
	void ht1080z(machine_config &config);

	void init_trs80l2();

protected:
	void machine_start() override;
	void machine_reset() override;

	void port_ff_w(u8 data);
	void sys80_fe_w(u8 data);
	void sys80_f8_w(u8 data);
	void port_ea_w(u8 data);
	void port_e8_w(u8 data);
	u8 port_ff_r();
	u8 sys80_f9_r();
	u8 port_ea_r();
	u8 port_e8_r();
	u8 irq_status_r();
	u8 printer_r();
	void printer_w(u8 data);
	void cassunit_w(u8 data);
	void motor_w(u8 data);
	u8 keyboard_r(offs_t offset);
	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);

	INTERRUPT_GEN_MEMBER(rtc_interrupt);
	INTERRUPT_GEN_MEMBER(fdc_interrupt);
	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	u8 m_irq = 0U;
	u8 m_mask = 0U;
	u8 m_tape_unit = 1U;
	bool m_reg_load = true;
	bool m_cassette_data = false;
	emu_timer *m_cassette_data_timer = 0;
	double m_old_cassette_val = 0;
	bool m_cpl = 0;
	u8 m_cols = 0xffU;
	bool m_7bit = false;
	u16 m_timeout = 600U;
	void trs80_io(address_map &map);
	floppy_image_device *m_fdd;
	required_device<z80_device> m_maincpu;
	required_memory_region m_region_maincpu;
	required_region_ptr<u8> m_p_chargen;
	optional_shared_ptr<u8> m_p_videoram;
	optional_device<centronics_device> m_centronics;
	optional_device<output_latch_device> m_cent_data_out;
	optional_device<input_buffer_device> m_cent_status_in;
	optional_device<ay31015_device> m_uart;
	optional_device<clock_device> m_uart_clock;
	optional_device<fd1771_device> m_fdc;
	optional_device_array<floppy_connector, 4> m_floppy;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	optional_ioport m_io_baud;
	optional_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;

private:
	void m1_io(address_map &map);
	void m1_mem(address_map &map);
	void sys80_io(address_map &map);
	void trs80_mem(address_map &map);
	void ht1080z_io(address_map &map);

	u32 screen_update_trs80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_sys80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_ht1080z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	static void floppy_formats(format_registration &fr);
};

#endif // MAME_INCLUDES_TRS80_H
