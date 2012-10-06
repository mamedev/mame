#pragma once

#ifndef __V1050__
#define __V1050__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "machine/ctronics.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/msm58321.h"
#include "machine/ram.h"
#include "machine/scsibus.h"
#include "machine/v1050kb.h"
#include "machine/wd17xx.h"
#include "video/mc6845.h"

#define SCREEN_TAG				"screen"

#define Z80_TAG					"u80"
#define UPB8214_TAG				"u38"
#define I8255A_DISP_TAG			"u79"
#define I8255A_MISC_TAG			"u10"
#define I8255A_RTC_TAG			"u36"
#define I8251A_KB_TAG			"u85"
#define I8251A_SIO_TAG			"u8"
#define MSM58321RS_TAG			"u26"
#define MB8877_TAG				"u13"
#define FDC9216_TAG				"u25"
#define M6502_TAG				"u76"
#define I8255A_M6502_TAG		"u101"
#define H46505_TAG				"u75"
#define CENTRONICS_TAG			"centronics"
#define TIMER_KB_TAG			"timer_kb"
#define TIMER_SIO_TAG			"timer_sio"
#define TIMER_ACK_TAG			"timer_ack"
#define TIMER_RST_TAG			"timer_rst"
#define SASIBUS_TAG				"sasi"

#define V1050_VIDEORAM_SIZE		0x8000
#define V1050_VIDEORAM_MASK		0x7fff

#define INT_RS_232			0x01
#define INT_WINCHESTER		0x02
#define INT_KEYBOARD		0x04
#define INT_FLOPPY			0x08
#define INT_VSYNC			0x10
#define INT_DISPLAY			0x20
#define INT_EXPANSION_B		0x40
#define INT_EXPANSION_A		0x80

class v1050_state : public driver_device
{
public:
	v1050_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_subcpu(*this, M6502_TAG),
		  m_pic(*this, UPB8214_TAG),
		  m_rtc(*this, MSM58321RS_TAG),
		  m_uart_kb(*this, I8251A_KB_TAG),
		  m_uart_sio(*this, I8251A_SIO_TAG),
		  m_fdc(*this, MB8877_TAG),
		  m_crtc(*this, H46505_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_ram(*this, RAM_TAG),
		  m_floppy0(*this, FLOPPY_0),
		  m_floppy1(*this, FLOPPY_1),
		  m_timer_sio(*this, TIMER_SIO_TAG),
		  m_timer_ack(*this, TIMER_ACK_TAG),
		  m_timer_rst(*this, TIMER_RST_TAG),
		  m_sasibus(*this, SASIBUS_TAG ":host")
	,
		m_video_ram(*this, "video_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<i8214_device> m_pic;
	required_device<msm58321_device> m_rtc;
	required_device<i8251_device> m_uart_kb;
	required_device<i8251_device> m_uart_sio;
	required_device<device_t> m_fdc;
	required_device<mc6845_device> m_crtc;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_device<device_t> m_floppy0;
	required_device<device_t> m_floppy1;
	required_device<timer_device> m_timer_sio;
	required_device<timer_device> m_timer_ack;
	required_device<timer_device> m_timer_rst;
	required_device<scsicb_device> m_sasibus;

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

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
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_WRITE8_MEMBER( keyboard_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_WRITE8_MEMBER( misc_ppi_pa_w );
	DECLARE_WRITE8_MEMBER( misc_ppi_pc_w );
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
	DECLARE_READ8_MEMBER( sasi_status_r );
	DECLARE_WRITE8_MEMBER( sasi_ctrl_w );

	void bankswitch();
	void set_interrupt(UINT8 mask, int state);
	void scan_keyboard();

	// interrupt state
	UINT8 m_int_mask;			// interrupt mask
	UINT8 m_int_state;			// interrupt status
	int m_f_int_enb;			// floppy interrupt enable

	// keyboard state
	UINT8 m_keylatch;			// keyboard row select
	UINT8 m_keydata;
	int m_keyavail;

	// serial state
	int m_rxrdy;				// receiver ready
	int m_txrdy;				// transmitter ready
	int m_baud_sel;				// baud select

	// memory state
	UINT8 m_bank;				// bank register

	// video state
	required_shared_ptr<UINT8> m_video_ram; 			// video RAM
	UINT8 *m_attr_ram;			// attribute RAM
	UINT8 m_attr;				// attribute latch
	TIMER_DEVICE_CALLBACK_MEMBER(v1050_keyboard_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(sasi_ack_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(sasi_rst_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(kb_8251_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(sio_8251_tick);
	DECLARE_WRITE_LINE_MEMBER(pic_int_w);
	DECLARE_WRITE8_MEMBER(disp_ppi_pc_w);
	DECLARE_WRITE8_MEMBER(m6502_ppi_pc_w);
	DECLARE_WRITE8_MEMBER(misc_ppi_pb_w);
	DECLARE_READ8_MEMBER(misc_ppi_pc_r);
};

//----------- defined in video/v1050.c -----------

MACHINE_CONFIG_EXTERN( v1050_video );

#endif
