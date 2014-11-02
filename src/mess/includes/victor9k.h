// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 / ACT Sirius 1 emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __VICTOR9K__
#define __VICTOR9K__

#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
#include "formats/victor9k_dsk.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "bus/centronics/ctronics.h"
#include "machine/6522via.h"
#include "bus/ieee488/ieee488.h"
#include "machine/mc6852.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/z80dart.h"
#include "machine/victor9kb.h"
#include "sound/hc55516.h"
#include "video/mc6845.h"

#define I8088_TAG       "8l"
#define I8048_TAG       "5d"
#define I8253_TAG       "13h"
#define I8259A_TAG      "7l"
#define UPD7201_TAG     "16e"
#define HD46505S_TAG    "11a"
#define MC6852_TAG      "11b"
#define HC55516_TAG     "15c"
#define M6522_1_TAG     "m6522_1"
#define M6522_2_TAG     "m6522_2"
#define M6522_3_TAG     "14l"
#define M6522_4_TAG     "1f"
#define M6522_5_TAG     "1k"
#define M6522_6_TAG     "1h"
#define DAC0808_0_TAG   "5b"
#define DAC0808_1_TAG   "5c"
#define CENTRONICS_TAG  "centronics"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define SCREEN_TAG      "screen"
#define VICTOR9K_KEYBOARD_TAG   "victor9kb"

class victor9k_state : public driver_device
{
public:
	victor9k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, I8088_TAG),
			m_fdc_cpu(*this, I8048_TAG),
			m_ieee488(*this, IEEE488_TAG),
			m_pic(*this, I8259A_TAG),
			m_upd7201(*this, UPD7201_TAG),
			m_ssda(*this, MC6852_TAG),
			m_via1(*this, M6522_1_TAG),
			m_via2(*this, M6522_2_TAG),
			m_via3(*this, M6522_3_TAG),
			m_via4(*this, M6522_4_TAG),
			m_via5(*this, M6522_5_TAG),
			m_via6(*this, M6522_6_TAG),
			m_cvsd(*this, HC55516_TAG),
			m_crtc(*this, HD46505S_TAG),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, I8048_TAG":0:525qd"),
			m_floppy1(*this, I8048_TAG":1:525qd"),
			m_kb(*this, VICTOR9K_KEYBOARD_TAG),
			m_rs232a(*this, RS232_A_TAG),
			m_rs232b(*this, RS232_B_TAG),
			m_video_ram(*this, "video_ram"),
			m_da(0),
			m_da0(0),
			m_da1(0),
			m_sel0(0),
			m_sel1(0),
			m_tach0(0),
			m_tach1(0),
			m_rdy0(0),
			m_rdy1(0),
			m_ds0(1),
			m_ds1(1),
			m_lms(0),
			m_brdy(1),
			m_sync(1),
			m_gcrerr(0),
			m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_fdc_cpu;
	required_device<ieee488_device> m_ieee488;
	required_device<pic8259_device> m_pic;
	required_device<upd7201_device> m_upd7201;
	required_device<mc6852_device> m_ssda;
	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<via6522_device> m_via3;
	required_device<via6522_device> m_via4;
	required_device<via6522_device> m_via5;
	required_device<via6522_device> m_via6;
	required_device<hc55516_device> m_cvsd;
	required_device<mc6845_device> m_crtc;
	required_device<ram_device> m_ram;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<victor9k_keyboard_device> m_kb;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;

	virtual void machine_start();

	DECLARE_READ8_MEMBER( floppy_p1_r );
	DECLARE_READ8_MEMBER( floppy_p2_r );
	DECLARE_WRITE8_MEMBER( floppy_p2_w );
	DECLARE_READ8_MEMBER( tach0_r );
	DECLARE_READ8_MEMBER( tach1_r );
	DECLARE_WRITE8_MEMBER( da_w );

	DECLARE_WRITE8_MEMBER( via1_pa_w );
	DECLARE_WRITE_LINE_MEMBER( write_nfrd );
	DECLARE_WRITE_LINE_MEMBER( write_ndac );
	DECLARE_WRITE8_MEMBER( via1_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
	DECLARE_WRITE_LINE_MEMBER( codec_vol_w );

	DECLARE_WRITE8_MEMBER( via2_pa_w );
	DECLARE_WRITE8_MEMBER( via2_pb_w );
	DECLARE_WRITE_LINE_MEMBER( write_ria );
	DECLARE_WRITE_LINE_MEMBER( write_rib );
	DECLARE_WRITE_LINE_MEMBER( via2_irq_w );

	DECLARE_WRITE8_MEMBER( via3_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via3_irq_w );

	DECLARE_WRITE8_MEMBER( via4_pa_w );
	DECLARE_WRITE8_MEMBER( via4_pb_w );
	DECLARE_WRITE_LINE_MEMBER( mode_w );
	DECLARE_WRITE_LINE_MEMBER( via4_irq_w );

	DECLARE_WRITE8_MEMBER( via5_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via5_irq_w );

	DECLARE_READ8_MEMBER( via6_pa_r );
	DECLARE_READ8_MEMBER( via6_pb_r );
	DECLARE_WRITE8_MEMBER( via6_pa_w );
	DECLARE_WRITE8_MEMBER( via6_pb_w );
	DECLARE_WRITE_LINE_MEMBER( drw_w );
	DECLARE_WRITE_LINE_MEMBER( erase_w );
	DECLARE_WRITE_LINE_MEMBER( kbrdy_w );
	DECLARE_WRITE_LINE_MEMBER( kbdata_w );
	DECLARE_WRITE_LINE_MEMBER( vert_w );
	DECLARE_WRITE_LINE_MEMBER( via6_irq_w );

	DECLARE_WRITE_LINE_MEMBER( ssda_irq_w );
	MC6845_UPDATE_ROW( crtc_update_row );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void ready0_cb(floppy_image_device *, int device);
	int load0_cb(floppy_image_device *device);
	void unload0_cb(floppy_image_device *device);
	void ready1_cb(floppy_image_device *, int device);
	int load1_cb(floppy_image_device *device);
	void unload1_cb(floppy_image_device *device);

	enum
	{
		LED_A = 0,
		LED_B
	};

	/* video state */
	required_shared_ptr<UINT8> m_video_ram;
	int m_brt;
	int m_cont;

	/* interrupts */
	int m_via1_irq;
	int m_via2_irq;
	int m_via3_irq;
	int m_via4_irq;
	int m_via5_irq;
	int m_via6_irq;
	int m_ssda_irq;

	/* floppy state */
	UINT8 m_da;
	UINT8 m_da0;
	UINT8 m_da1;
	int m_sel0;
	int m_sel1;
	int m_tach0;
	int m_tach1;
	int m_rdy0;
	int m_rdy1;
	int m_ds0;
	int m_ds1;
	UINT8 m_lms;                         /* motor speed */
	int m_st[2];                        /* stepper phase */
	int m_stp[2];                        /* stepper enable */
	int m_drive;                        /* selected drive */
	int m_side;                         /* selected side */
	int m_brdy;
	int m_sync;
	int m_gcrerr;

	DECLARE_WRITE_LINE_MEMBER(mux_serial_b_w);
	DECLARE_WRITE_LINE_MEMBER(mux_serial_a_w);
	required_device<palette_device> m_palette;
};

#endif
