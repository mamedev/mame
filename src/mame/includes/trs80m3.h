// license:BSD-3-Clause
// copyright-holders:Robbbert
//*****************************************************************************

#ifndef MAME_INCLUDES_TRS80M3_H
#define MAME_INCLUDES_TRS80M3_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/bankdev.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/ay31015.h"
#include "machine/com8116.h"
#include "bus/rs232/rs232.h"
#include "machine/buffer.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "emupal.h"

#include "formats/trs_cas.h"


class trs80m3_state : public driver_device
{
public:
	trs80m3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_region_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_bankdev(*this, "bankdev")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cent_status_in(*this, "cent_status_in")
		, m_uart(*this, "uart")
		, m_brg(*this, "brg")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "LINE%u", 0)
		, m_mainram(*this, RAM_TAG)
		, m_m4_bank(*this, "m4_banked_mem")
		, m_m4p_bank(*this, "m4p_banked_mem")
		, m_32kbanks(*this, "bank%u", 0U)
		, m_16kbank(*this, "16kbank")
		, m_vidbank(*this, "vidbank")
	{ }

	void model4p(machine_config &config);
	void model3(machine_config &config);
	void cp500(machine_config &config);
	void model4(machine_config &config);

	void init_trs80m3();
	void init_trs80m4();
	void init_trs80m4p();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_WRITE8_MEMBER(port_ff_w);
	DECLARE_WRITE8_MEMBER(port_f4_w);
	DECLARE_WRITE8_MEMBER(port_ec_w);
	DECLARE_WRITE8_MEMBER(port_ea_w);
	DECLARE_WRITE8_MEMBER(port_e8_w);
	DECLARE_WRITE8_MEMBER(port_e4_w);
	DECLARE_WRITE8_MEMBER(port_e0_w);
	DECLARE_WRITE8_MEMBER(port_9c_w);
	DECLARE_WRITE8_MEMBER(port_90_w);
	DECLARE_WRITE8_MEMBER(port_88_w);
	DECLARE_WRITE8_MEMBER(port_84_w);
	DECLARE_READ8_MEMBER(port_ff_r);
	DECLARE_READ8_MEMBER(port_ec_r);
	DECLARE_READ8_MEMBER(port_ea_r);
	DECLARE_READ8_MEMBER(port_e8_r);
	DECLARE_READ8_MEMBER(port_e4_r);
	DECLARE_READ8_MEMBER(port_e0_r);
	DECLARE_READ8_MEMBER(printer_r);
	DECLARE_WRITE8_MEMBER(printer_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(wd179x_r);
	DECLARE_READ8_MEMBER(cp500_port_f4_r);

	INTERRUPT_GEN_MEMBER(rtc_interrupt);
	INTERRUPT_GEN_MEMBER(fdc_interrupt);
	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	DECLARE_WRITE_LINE_MEMBER(drq_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	uint32_t screen_update_trs80m3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cp500_io(address_map &map);
	void m3_io(address_map &map);
	void m3_mem(address_map &map);
	void m4_mem(address_map &map);
	void m4p_mem(address_map &map);
	void m4_io(address_map &map);
	void m4p_io(address_map &map);
	void m4_banked_mem(address_map &map);
	void m4p_banked_mem(address_map &map);

	uint8_t m_model4;
	uint8_t m_mode;
	uint8_t m_irq;
	uint8_t m_mask;
	uint8_t m_nmi_mask;
	uint8_t m_port_ec;
	bool m_reg_load;
	uint8_t m_nmi_data;
	uint8_t m_cassette_data;
	emu_timer *m_cassette_data_timer;
	double m_old_cassette_val;
	uint16_t m_start_address;
	uint8_t m_crtc_reg;
	uint8_t m_size_store;
	bool m_a11_flipflop;
	uint16_t m_timeout;
	bool m_wait;
	bool m_drq_off;
	bool m_intrq_off;
	floppy_image_device *m_floppy;
	required_device<cpu_device> m_maincpu;
	required_memory_region m_region_maincpu;
	required_region_ptr<u8> m_p_chargen;
	optional_shared_ptr<u8> m_p_videoram;
	optional_device<address_map_bank_device> m_bankdev;
	optional_device<centronics_device> m_centronics;
	optional_device<output_latch_device> m_cent_data_out;
	optional_device<input_buffer_device> m_cent_status_in;
	optional_device<ay31015_device> m_uart;
	optional_device<com8116_device> m_brg;
	optional_device<fd1793_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	optional_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;
	optional_device<ram_device>                 m_mainram;
	optional_device<address_map_bank_device>    m_m4_bank;
	optional_device<address_map_bank_device>    m_m4p_bank;
	optional_memory_bank_array<2>               m_32kbanks;
	optional_memory_bank                        m_16kbank;
	optional_memory_bank                        m_vidbank;
};

#endif // MAME_INCLUDES_TRS80M3_H
