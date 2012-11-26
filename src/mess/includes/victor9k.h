#pragma once

#ifndef __VICTOR9K__
#define __VICTOR9K__


#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/flopdrv.h"
#include "machine/ram.h"
#include "machine/ctronics.h"
#include "machine/6522via.h"
#include "machine/ieee488.h"
#include "machine/mc6852.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/upd7201.h"
#include "machine/victor9kb.h"
#include "sound/hc55516.h"
#include "video/mc6845.h"

#define SCREEN_TAG		"screen"
#define I8088_TAG		"8l"
#define I8048_TAG		"5d"
#define I8253_TAG		"13h"
#define I8259A_TAG		"7l"
#define UPD7201_TAG		"16e"
#define HD46505S_TAG	"11a"
#define MC6852_TAG		"13b"
#define HC55516_TAG		"1m"
#define M6522_1_TAG		"m6522_1"
#define M6522_2_TAG		"m6522_2"
#define M6522_3_TAG		"m6522_3"
#define M6522_4_TAG		"m6522_4"
#define M6522_5_TAG		"m6522_5"
#define M6522_6_TAG		"m6522_6"
#define CENTRONICS_TAG	"centronics"

class victor9k_state : public driver_device
{
public:
	victor9k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, I8088_TAG),
		  m_fdc_cpu(*this, I8048_TAG),
		  m_ieee488(*this, IEEE488_TAG),
		  m_pic(*this, I8259A_TAG),
		  m_ssda(*this, MC6852_TAG),
		  m_via1(*this, M6522_1_TAG),
		  m_via2(*this, M6522_2_TAG),
		  m_cvsd(*this, HC55516_TAG),
		  m_crtc(*this, HD46505S_TAG),
		  m_ram(*this, RAM_TAG),
		  m_floppy0(*this, FLOPPY_0),
		  m_floppy1(*this, FLOPPY_1),
		  m_kb(*this, VICTOR9K_KEYBOARD_TAG)
	,
		m_video_ram(*this, "video_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_fdc_cpu;
	required_device<ieee488_device> m_ieee488;
	required_device<pic8259_device> m_pic;
	required_device<mc6852_device> m_ssda;
	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<hc55516_device> m_cvsd;
	required_device<mc6845_device> m_crtc;
	required_device<ram_device> m_ram;
	required_device<legacy_floppy_image_device> m_floppy0;
	required_device<legacy_floppy_image_device> m_floppy1;
	required_device<victor9k_keyboard_device> m_kb;

	virtual void machine_start();

	DECLARE_WRITE_LINE_MEMBER( ssda_irq_w );
	DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
	DECLARE_WRITE_LINE_MEMBER( via2_irq_w );
	DECLARE_WRITE_LINE_MEMBER( via3_irq_w );
	DECLARE_WRITE_LINE_MEMBER( via4_irq_w );
	DECLARE_WRITE_LINE_MEMBER( via5_irq_w );
	DECLARE_WRITE_LINE_MEMBER( via6_irq_w );
	DECLARE_WRITE8_MEMBER( via1_pa_w );
	DECLARE_READ8_MEMBER( via1_pb_r );
	DECLARE_WRITE8_MEMBER( via1_pb_w );
	DECLARE_WRITE_LINE_MEMBER( codec_vol_w );
	DECLARE_READ8_MEMBER( via2_pa_r );
	DECLARE_WRITE8_MEMBER( via2_pa_w );
	DECLARE_WRITE8_MEMBER( via2_pb_w );
	DECLARE_READ8_MEMBER( via3_pa_r );
	DECLARE_READ8_MEMBER( via3_pb_r );
	DECLARE_WRITE8_MEMBER( via3_pa_w );
	DECLARE_WRITE8_MEMBER( via3_pb_w );
	DECLARE_WRITE8_MEMBER( via4_pa_w );
	DECLARE_WRITE8_MEMBER( via4_pb_w );
	DECLARE_WRITE_LINE_MEMBER( mode_w );
	DECLARE_READ8_MEMBER( via5_pa_r );
	DECLARE_WRITE8_MEMBER( via5_pb_w );
	DECLARE_READ8_MEMBER( via6_pa_r );
	DECLARE_READ8_MEMBER( via6_pb_r );
	DECLARE_WRITE8_MEMBER( via6_pa_w );
	DECLARE_WRITE8_MEMBER( via6_pb_w );
	DECLARE_WRITE_LINE_MEMBER( drw_w );
	DECLARE_WRITE_LINE_MEMBER( erase_w );
	DECLARE_WRITE_LINE_MEMBER( kbrdy_w );

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
	int m_lms[2];						/* motor speed */
	int m_st[2];						/* stepper phase */
	int m_se[2];						/* stepper enable */
	int m_drive;						/* selected drive */
	int m_side;							/* selected side */
	DECLARE_WRITE_LINE_MEMBER(mux_serial_b_w);
	DECLARE_WRITE_LINE_MEMBER(mux_serial_a_w);
};

#endif
