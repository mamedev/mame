// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __V1050__
#define __V1050__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6502/m6502.h"
#include "bus/centronics/ctronics.h"
#include "bus/scsi/s1410.h"
#include "machine/clock.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/msm58321.h"
#include "machine/ram.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "machine/v1050kb.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"

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

#define INT_RS_232          0x01
#define INT_WINCHESTER      0x02
#define INT_KEYBOARD        0x04
#define INT_FLOPPY          0x08
#define INT_VSYNC           0x10
#define INT_DISPLAY         0x20
#define INT_EXPANSION_B     0x40
#define INT_EXPANSION_A     0x80

class v1050_state : public driver_device
{
public:
	v1050_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_attr_ram(*this, "attr_ram"),
		m_rtc_ppi_pa(0),
		m_rtc_ppi_pc(0)
	{
	}

	DECLARE_READ8_MEMBER( kb_data_r );
	DECLARE_READ8_MEMBER( kb_status_r );
	DECLARE_WRITE8_MEMBER( v1050_i8214_w );
	DECLARE_READ8_MEMBER( vint_clr_r );
	DECLARE_WRITE8_MEMBER( vint_clr_w );
	DECLARE_READ8_MEMBER( dint_clr_r );
	DECLARE_WRITE8_MEMBER( dint_clr_w );
	DECLARE_WRITE8_MEMBER( bank_w );
	DECLARE_WRITE8_MEMBER( dint_w );
	DECLARE_WRITE8_MEMBER( dvint_clr_w );
	DECLARE_WRITE8_MEMBER( misc_ppi_pa_w );
	DECLARE_WRITE8_MEMBER( misc_ppi_pc_w );
	DECLARE_READ8_MEMBER( rtc_ppi_pa_r );
	DECLARE_WRITE8_MEMBER( rtc_ppi_pa_w );
	DECLARE_WRITE8_MEMBER( rtc_ppi_pb_w );
	DECLARE_READ8_MEMBER( rtc_ppi_pc_r );
	DECLARE_WRITE8_MEMBER( rtc_ppi_pc_w );
	DECLARE_WRITE_LINE_MEMBER( kb_rxrdy_w );
	DECLARE_WRITE_LINE_MEMBER( sio_rxrdy_w );
	DECLARE_WRITE_LINE_MEMBER( sio_txrdy_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_READ8_MEMBER( attr_r );
	DECLARE_WRITE8_MEMBER( attr_w );
	DECLARE_READ8_MEMBER( videoram_r );
	DECLARE_WRITE8_MEMBER( videoram_w );
	DECLARE_WRITE_LINE_MEMBER( crtc_vs_w );
	DECLARE_WRITE8_MEMBER(sasi_data_w);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_io);
	DECLARE_WRITE8_MEMBER( sasi_ctrl_w );

	WRITE_LINE_MEMBER( rtc_ppi_pa_0_w ){ m_rtc_ppi_pa = (m_rtc_ppi_pa & ~(1 << 0)) | ((state & 1) << 0); }
	WRITE_LINE_MEMBER( rtc_ppi_pa_1_w ){ m_rtc_ppi_pa = (m_rtc_ppi_pa & ~(1 << 1)) | ((state & 1) << 1); }
	WRITE_LINE_MEMBER( rtc_ppi_pa_2_w ){ m_rtc_ppi_pa = (m_rtc_ppi_pa & ~(1 << 2)) | ((state & 1) << 2); }
	WRITE_LINE_MEMBER( rtc_ppi_pa_3_w ){ m_rtc_ppi_pa = (m_rtc_ppi_pa & ~(1 << 3)) | ((state & 1) << 3); }
	WRITE_LINE_MEMBER( rtc_ppi_pc_3_w ){ m_rtc_ppi_pc = (m_rtc_ppi_pc & ~(1 << 3)) | ((state & 1) << 3); }

	TIMER_DEVICE_CALLBACK_MEMBER(v1050_keyboard_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(sasi_ack_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(sasi_rst_tick);
	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);
	DECLARE_WRITE_LINE_MEMBER(write_sio_clock);
	DECLARE_WRITE_LINE_MEMBER(pic_int_w);
	DECLARE_WRITE8_MEMBER(disp_ppi_pc_w);
	DECLARE_WRITE8_MEMBER(m6502_ppi_pc_w);
	DECLARE_READ8_MEMBER(misc_ppi_pc_r);
	IRQ_CALLBACK_MEMBER(v1050_int_ack);

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_perror);

	MC6845_UPDATE_ROW(crtc_update_row);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	void bankswitch();
	void update_fdc();
	void set_interrupt(UINT8 mask, int state);
	void scan_keyboard();
	void set_baud_sel(int sel);

public: // HACK for MC6845
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<i8214_device> m_pic;
	required_device<i8255_device> m_ppi_disp;
	required_device<i8255_device> m_ppi_6502;
	required_device<msm58321_device> m_rtc;
	required_device<i8251_device> m_uart_kb;
	required_device<i8251_device> m_uart_sio;
	required_device<mb8877_t> m_fdc;
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
	required_device<SCSI_PORT_DEVICE> m_sasibus;
	required_device<output_latch_device> m_sasi_data_out;
	required_device<input_buffer_device> m_sasi_data_in;
	required_device<input_buffer_device> m_sasi_ctrl_in;
	required_memory_region m_rom;
	required_shared_ptr<UINT8> m_video_ram;
	optional_shared_ptr<UINT8> m_attr_ram;

	// interrupt state
	UINT8 m_int_mask;           // interrupt mask
	UINT8 m_int_state;          // interrupt status
	int m_f_int_enb;            // floppy interrupt enable
	bool m_fdc_irq;
	bool m_fdc_drq;

	// keyboard state
	UINT8 m_keylatch;           // keyboard row select
	UINT8 m_keydata;
	int m_keyavail;

	// serial state
	int m_rxrdy;                // receiver ready
	int m_txrdy;                // transmitter ready
	int m_baud_sel;             // baud select

	// memory state
	UINT8 m_bank;               // bank register

	// video state
	UINT8 m_attr;               // attribute latch

	// sasi state
	UINT8 m_sasi_data;
	int m_sasi_data_enable;

	UINT8 m_rtc_ppi_pa;
	UINT8 m_rtc_ppi_pc;

	int m_centronics_busy;
	int m_centronics_perror;
};

//----------- defined in video/v1050.c -----------

MACHINE_CONFIG_EXTERN( v1050_video );

#endif
