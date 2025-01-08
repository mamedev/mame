// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_VISUAL_V1050_H
#define MAME_VISUAL_V1050_H

#pragma once

#include "cpu/z80/z80.h"
#include "cpu/m6502/m6502.h"
#include "bus/centronics/ctronics.h"
#include "bus/scsi/s1410.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/msm58321.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "v1050kb.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "emupal.h"

#define SCREEN_TAG              "screen"

#define Z80_TAG                 "u80"
#define UPB8214_TAG             "u38"
#define I8255A_DISP_TAG         "u79"
#define I8255A_MISC_TAG         "u10"
#define I8255A_RTC_TAG          "u36"
#define I8251A_KB_TAG           "u85"
#define I8251A_SIO_TAG          "u8"
#define MSM58321RS_TAG          "u26"
#define MB8877_TAG              "u13"
#define FDC9216_TAG             "u25"
#define M6502_TAG               "u76"
#define I8255A_M6502_TAG        "u101"
#define H46505_TAG              "u75"
#define CENTRONICS_TAG          "centronics"
#define CLOCK_KB_TAG            "keyboard_clock"
#define CLOCK_SIO_TAG           "sio_clock"
#define TIMER_ACK_TAG           "timer_ack"
#define TIMER_RST_TAG           "timer_rst"
#define SASIBUS_TAG             "sasi"
#define RS232_TAG               "rs232"
#define V1050_KEYBOARD_TAG      "v1050kb"

#define V1050_VIDEORAM_SIZE     0x8000
#define V1050_VIDEORAM_MASK     0x7fff

#define INT_RS_232          0
#define INT_WINCHESTER      1
#define INT_KEYBOARD        2
#define INT_FLOPPY          3
#define INT_VSYNC           4
#define INT_DISPLAY         5
#define INT_EXPANSION_B     6
#define INT_EXPANSION_A     7

class v1050_state : public driver_device
{
public:
	v1050_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_subcpu(*this, M6502_TAG),
		m_pic(*this, UPB8214_TAG),
		m_ppi_disp(*this, I8255A_DISP_TAG),
		m_ppi_6502(*this, I8255A_M6502_TAG),
		m_rtc(*this, MSM58321RS_TAG),
		m_uart_kb(*this, I8251A_KB_TAG),
		m_uart_sio(*this, I8251A_SIO_TAG),
		m_fdc(*this, MB8877_TAG),
		m_crtc(*this, H46505_TAG),
		m_palette(*this, "palette"),
		m_centronics(*this, CENTRONICS_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy0(*this, MB8877_TAG":0"),
		m_floppy1(*this, MB8877_TAG":1"),
		m_floppy2(*this, MB8877_TAG":2"),
		m_floppy3(*this, MB8877_TAG":3"),
		m_clock_sio(*this, CLOCK_SIO_TAG),
		m_timer_ack(*this, TIMER_ACK_TAG),
		m_timer_rst(*this, TIMER_RST_TAG),
		m_sasibus(*this, SASIBUS_TAG),
		m_sasi_data_out(*this, "scsi_data_out"),
		m_sasi_data_in(*this, "scsi_data_in"),
		m_sasi_ctrl_in(*this, "scsi_ctrl_in"),
		m_rom(*this, Z80_TAG),
		m_video_ram(*this, "video_ram"),
		m_attr_ram(*this, "attr_ram", V1050_VIDEORAM_SIZE, ENDIANNESS_LITTLE),
		m_int_mask(0),
		m_int_state(0),
		m_fdc_irq(0),
		m_fdc_drq(0),
		m_rtc_ppi_pa(0),
		m_rtc_ppi_pc(0)
	{
	}

	void v1050(machine_config &config);
	void v1050_video(machine_config &config);

private:
	uint8_t kb_data_r();
	uint8_t kb_status_r();
	void v1050_i8214_w(uint8_t data);
	uint8_t vint_clr_r();
	void vint_clr_w(uint8_t data);
	uint8_t dint_clr_r();
	void dint_clr_w(uint8_t data);
	void bank_w(uint8_t data);
	void dint_w(uint8_t data);
	void dvint_clr_w(uint8_t data);
	void misc_ppi_pa_w(uint8_t data);
	void misc_ppi_pc_w(uint8_t data);
	uint8_t rtc_ppi_pa_r();
	void rtc_ppi_pa_w(uint8_t data);
	void rtc_ppi_pb_w(uint8_t data);
	uint8_t rtc_ppi_pc_r();
	void rtc_ppi_pc_w(uint8_t data);
	void kb_rxrdy_w(int state);
	void sio_rxrdy_w(int state);
	void sio_txrdy_w(int state);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	uint8_t attr_r();
	void attr_w(uint8_t data);
	uint8_t videoram_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void crtc_vs_w(int state);
	void sasi_data_w(uint8_t data);
	void write_sasi_io(int state);
	void sasi_ctrl_w(uint8_t data);

	void rtc_ppi_pa_0_w(int state) { m_rtc_ppi_pa = (m_rtc_ppi_pa & ~(1 << 0)) | ((state & 1) << 0); }
	void rtc_ppi_pa_1_w(int state) { m_rtc_ppi_pa = (m_rtc_ppi_pa & ~(1 << 1)) | ((state & 1) << 1); }
	void rtc_ppi_pa_2_w(int state) { m_rtc_ppi_pa = (m_rtc_ppi_pa & ~(1 << 2)) | ((state & 1) << 2); }
	void rtc_ppi_pa_3_w(int state) { m_rtc_ppi_pa = (m_rtc_ppi_pa & ~(1 << 3)) | ((state & 1) << 3); }
	void rtc_ppi_pc_3_w(int state) { m_rtc_ppi_pc = (m_rtc_ppi_pc & ~(1 << 3)) | ((state & 1) << 3); }

	TIMER_DEVICE_CALLBACK_MEMBER(v1050_keyboard_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(sasi_ack_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(sasi_rst_tick);
	void write_keyboard_clock(int state);
	void write_sio_clock(int state);
	void pic_int_w(int state);
	void disp_ppi_pc_w(uint8_t data);
	void m6502_ppi_pc_w(uint8_t data);
	uint8_t misc_ppi_pc_r();
	IRQ_CALLBACK_MEMBER(v1050_int_ack);

	void write_centronics_busy(int state);
	void write_centronics_perror(int state);

	MC6845_UPDATE_ROW(crtc_update_row);

	void v1050_crt_mem(address_map &map) ATTR_COLD;
	void v1050_io(address_map &map) ATTR_COLD;
	void v1050_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

	void bankswitch();
	void update_fdc();
	void set_interrupt(int line, int state);
	void scan_keyboard();
	void set_baud_sel(int sel);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<i8214_device> m_pic;
	required_device<i8255_device> m_ppi_disp;
	required_device<i8255_device> m_ppi_6502;
	required_device<msm58321_device> m_rtc;
	required_device<i8251_device> m_uart_kb;
	required_device<i8251_device> m_uart_sio;
	required_device<mb8877_device> m_fdc;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<clock_device> m_clock_sio;
	required_device<timer_device> m_timer_ack;
	required_device<timer_device> m_timer_rst;
	required_device<scsi_port_device> m_sasibus;
	required_device<output_latch_device> m_sasi_data_out;
	required_device<input_buffer_device> m_sasi_data_in;
	required_device<input_buffer_device> m_sasi_ctrl_in;
	required_memory_region m_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	memory_share_creator<uint8_t> m_attr_ram;

	// interrupt state
	uint8_t m_int_mask;           // interrupt mask
	uint8_t m_int_state;
	int m_f_int_enb = 0;            // floppy interrupt enable
	bool m_fdc_irq;
	bool m_fdc_drq;

	// keyboard state
	uint8_t m_keylatch = 0;           // keyboard row select
	uint8_t m_keydata = 0;
	int m_keyavail = 0;

	// serial state
	int m_rxrdy = 0;                // receiver ready
	int m_txrdy = 0;                // transmitter ready
	int m_baud_sel = 0;             // baud select

	// memory state
	uint8_t m_bank = 0;               // bank register

	// video state
	uint8_t m_attr = 0;               // attribute latch

	// sasi state
	uint8_t m_sasi_data = 0;
	int m_sasi_data_enable = 0;

	uint8_t m_rtc_ppi_pa;
	uint8_t m_rtc_ppi_pc;

	int m_centronics_busy = 0;
	int m_centronics_perror = 0;
};

#endif // MAME_VISUAL_V1050_H
