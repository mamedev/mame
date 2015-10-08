// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 / ACT Sirius 1 emulation

**********************************************************************/

#pragma once

#ifndef __VICTOR9K__
#define __VICTOR9K__

#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "cpu/i86/i86.h"
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
#include "machine/victor9k_fdc.h"
#include "sound/hc55516.h"
#include "video/mc6845.h"

#define I8088_TAG       "8l"
#define I8253_TAG       "13h"
#define I8259A_TAG      "7l"
#define UPD7201_TAG     "16e"
#define HD46505S_TAG    "11a"
#define MC6852_TAG      "11b"
#define HC55516_TAG     "15c"
#define M6522_1_TAG     "m6522_1"
#define M6522_2_TAG     "m6522_2"
#define M6522_3_TAG     "14l"
#define DAC0808_0_TAG   "5b"
#define DAC0808_1_TAG   "5c"
#define CENTRONICS_TAG  "centronics"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define SCREEN_TAG      "screen"
#define KB_TAG          "kb"
#define FDC_TAG         "fdc"

class victor9k_state : public driver_device
{
public:
	victor9k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8088_TAG),
		m_ieee488(*this, IEEE488_TAG),
		m_pic(*this, I8259A_TAG),
		m_upd7201(*this, UPD7201_TAG),
		m_ssda(*this, MC6852_TAG),
		m_via1(*this, M6522_1_TAG),
		m_via2(*this, M6522_2_TAG),
		m_via3(*this, M6522_3_TAG),
		m_cvsd(*this, HC55516_TAG),
		m_crtc(*this, HD46505S_TAG),
		m_ram(*this, RAM_TAG),
		m_kb(*this, KB_TAG),
		m_fdc(*this, FDC_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_rs232a(*this, RS232_A_TAG),
		m_rs232b(*this, RS232_B_TAG),
		m_palette(*this, "palette"),
		m_rom(*this, I8088_TAG),
		m_video_ram(*this, "video_ram"),
		m_brt(0),
		m_cont(0),
		m_via1_irq(CLEAR_LINE),
		m_via2_irq(CLEAR_LINE),
		m_via3_irq(CLEAR_LINE),
		m_fdc_irq(CLEAR_LINE),
		m_ssda_irq(CLEAR_LINE),
		m_kbrdy(1),
		m_kbackctl(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ieee488_device> m_ieee488;
	required_device<pic8259_device> m_pic;
	required_device<upd7201_device> m_upd7201;
	required_device<mc6852_device> m_ssda;
	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<via6522_device> m_via3;
	required_device<hc55516_device> m_cvsd;
	required_device<mc6845_device> m_crtc;
	required_device<ram_device> m_ram;
	required_device<victor_9000_keyboard_t> m_kb;
	required_device<victor_9000_fdc_t> m_fdc;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<palette_device> m_palette;
	required_memory_region m_rom;
	required_shared_ptr<UINT8> m_video_ram;

	virtual void machine_start();
	virtual void machine_reset();

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

	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );

	DECLARE_WRITE_LINE_MEMBER( ssda_irq_w );

	DECLARE_WRITE_LINE_MEMBER( kbrdy_w );
	DECLARE_WRITE_LINE_MEMBER( kbdata_w );
	DECLARE_WRITE_LINE_MEMBER( vert_w );

	MC6845_UPDATE_ROW( crtc_update_row );

	DECLARE_WRITE_LINE_MEMBER( mux_serial_b_w );
	DECLARE_WRITE_LINE_MEMBER( mux_serial_a_w );

	// video state
	int m_brt;
	int m_cont;

	// interrupts
	int m_via1_irq;
	int m_via2_irq;
	int m_via3_irq;
	int m_fdc_irq;
	int m_ssda_irq;

	// keyboard
	int m_kbrdy;
	int m_kbackctl;

	void update_kback();
};

#endif
