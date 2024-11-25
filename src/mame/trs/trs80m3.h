// license:BSD-3-Clause
// copyright-holders:Robbbert
//*****************************************************************************

#ifndef MAME_TRS_TRS80M3_H
#define MAME_TRS_TRS80M3_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/bankdev.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
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
		, m_floppy(*this, "fdc%u", 0U)
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "LINE%u", 0U)
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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);
	void port_ff_w(uint8_t data);
	void port_f4_w(uint8_t data);
	void port_ec_w(uint8_t data);
	void port_ea_w(uint8_t data);
	void port_e8_w(uint8_t data);
	void port_e4_w(uint8_t data);
	void port_e0_w(uint8_t data);
	void port_9c_w(uint8_t data);
	void port_90_w(uint8_t data);
	void port_88_w(offs_t offset, uint8_t data);
	void port_84_w(uint8_t data);
	uint8_t port_ff_r();
	uint8_t port_ec_r();
	uint8_t port_ea_r();
	uint8_t port_e8_r();
	uint8_t port_e4_r();
	uint8_t port_e0_r();
	uint8_t printer_r();
	void printer_w(uint8_t data);
	uint8_t keyboard_r(offs_t offset);
	uint8_t wd179x_r();
	uint8_t cp500_port_f4_r();

	INTERRUPT_GEN_MEMBER(rtc_interrupt);
	INTERRUPT_GEN_MEMBER(fdc_interrupt);
	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	void intrq_w(int state);
	void drq_w(int state);
	uint32_t screen_update_trs80m3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cp500_io(address_map &map) ATTR_COLD;
	void m3_io(address_map &map) ATTR_COLD;
	void m3_mem(address_map &map) ATTR_COLD;
	void m4_mem(address_map &map) ATTR_COLD;
	void m4p_mem(address_map &map) ATTR_COLD;
	void m4_io(address_map &map) ATTR_COLD;
	void m4p_io(address_map &map) ATTR_COLD;
	void m4_banked_mem(address_map &map) ATTR_COLD;
	void m4p_banked_mem(address_map &map) ATTR_COLD;

	uint8_t m_model4 = 0U;
	uint8_t m_mode = 0U;
	uint8_t m_irq = 0U;
	uint8_t m_mask = 0U;
	uint8_t m_nmi_mask = 0U;
	uint8_t m_port_ec = 0U;
	bool m_reg_load = false;
	uint8_t m_nmi_data = 0U;
	uint8_t m_cassette_data = 0U;
	emu_timer *m_cassette_data_timer = nullptr;
	double m_old_cassette_val = 0;
	uint16_t m_start_address = 0U;
	uint8_t m_crtc_reg = 0U;
	uint8_t m_size_store = 0U;
	bool m_a11_flipflop = false;
	uint16_t m_timeout = 0U;
	bool m_wait = false;
	bool m_drq_off = false;
	bool m_intrq_off = false;
	floppy_image_device *m_fdd = nullptr;
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
	optional_device_array<floppy_connector, 2> m_floppy;
	required_device<speaker_sound_device> m_speaker;
	optional_device<cassette_image_device> m_cassette;
	optional_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;
	optional_device<ram_device>                 m_mainram;
	optional_device<address_map_bank_device>    m_m4_bank;
	optional_device<address_map_bank_device>    m_m4p_bank;
	optional_memory_bank_array<2>               m_32kbanks;
	optional_memory_bank                        m_16kbank;
	optional_memory_bank                        m_vidbank;
};

#endif // MAME_TRS_TRS80M3_H
