#pragma once

#ifndef __TEK405X__
#define __TEK405X__

#include "machine/ram.h"

#define MC6800_TAG			"u61"
#define MC6820_Y_TAG		"u561"
#define MC6820_X_TAG		"u565"
#define MC6820_TAPE_TAG		"u361"
#define MC6820_KB_TAG		"u461"
#define MC6820_GPIB_TAG		"u265"
#define MC6820_COM_TAG		"u5"
#define MC6850_TAG			"u25"
#define RS232_TAG			"rs232"
#define SCREEN_TAG			"screen"

#define AM2901A_TAG			"am2901a"

class tek4051_state : public driver_device
{
public:
	tek4051_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, MC6800_TAG),
		  m_gpib_pia(*this, MC6820_GPIB_TAG),
		  m_com_pia(*this, MC6820_COM_TAG),
		  m_acia(*this, MC6850_TAG),
		  m_gpib(*this, IEEE488_TAG),
		  m_speaker(*this, SPEAKER_TAG),
		  m_ram(*this, RAM_TAG)
	 { }

	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_gpib_pia;
	required_device<device_t> m_com_pia;
	required_device<acia6850_device> m_acia;
	required_device<ieee488_device> m_gpib;
	required_device<device_t> m_speaker;
	required_device<ram_device> m_ram;

	virtual void machine_start();

	virtual void video_start();

	void bankswitch(UINT8 data);
	void update_irq();
	void update_nmi();
	void scan_keyboard();

	DECLARE_WRITE8_MEMBER( lbs_w );

	DECLARE_READ8_MEMBER( x_pia_pa_r );
	DECLARE_WRITE8_MEMBER( x_pia_pa_w );
	DECLARE_WRITE8_MEMBER( x_pia_pb_w );
	DECLARE_WRITE_LINE_MEMBER( adot_w );
	DECLARE_READ_LINE_MEMBER( viewcause_r );
	DECLARE_WRITE_LINE_MEMBER( bufclk_w );
	DECLARE_WRITE_LINE_MEMBER( x_pia_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( x_pia_irqb_w );

	DECLARE_READ8_MEMBER( sa_r );
	DECLARE_WRITE8_MEMBER( y_pia_pa_w );
	DECLARE_WRITE8_MEMBER( sb_w );
	DECLARE_READ_LINE_MEMBER( rdbyte_r );
	DECLARE_READ_LINE_MEMBER( mdata_r );
	DECLARE_READ_LINE_MEMBER( fmark_r );
	DECLARE_WRITE_LINE_MEMBER( sot_w );
	DECLARE_WRITE_LINE_MEMBER( y_pia_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( y_pia_irqb_w );

	DECLARE_READ8_MEMBER( kb_pia_pa_r );
	DECLARE_READ8_MEMBER( kb_pia_pb_r );
	DECLARE_WRITE8_MEMBER( kb_pia_pb_w );
	DECLARE_READ_LINE_MEMBER( key_r );
	DECLARE_WRITE_LINE_MEMBER( kb_halt_w );
	DECLARE_WRITE_LINE_MEMBER( kb_pia_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( kb_pia_irqb_w );

	DECLARE_READ8_MEMBER( tape_pia_pa_r );
	DECLARE_WRITE8_MEMBER( tape_pia_pa_w );
	DECLARE_WRITE8_MEMBER( tape_pia_pb_w );
	DECLARE_READ_LINE_MEMBER( rmark_r );
	DECLARE_READ_LINE_MEMBER( lohole_r );
	DECLARE_READ_LINE_MEMBER( filfnd_r );
	DECLARE_READ_LINE_MEMBER( uphole_r );
	DECLARE_WRITE_LINE_MEMBER( tape_pia_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( tape_pia_irqb_w );

	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_READ8_MEMBER( gpib_pia_pb_r );
	DECLARE_WRITE8_MEMBER( gpib_pia_pb_w );
	DECLARE_WRITE_LINE_MEMBER( talk_w );
	DECLARE_WRITE_LINE_MEMBER( gpib_pia_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( gpib_pia_irqb_w );

	DECLARE_WRITE8_MEMBER( com_pia_pa_w );
	DECLARE_READ8_MEMBER( com_pia_pb_r );
	DECLARE_WRITE8_MEMBER( com_pia_pb_w );
	DECLARE_WRITE_LINE_MEMBER( com_pia_irqa_w );
	DECLARE_WRITE_LINE_MEMBER( com_pia_irqb_w );
	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );

	// interrupts
	int m_x_pia_irqa;
	int m_x_pia_irqb;
	int m_y_pia_irqa;
	int m_y_pia_irqb;
	int m_tape_pia_irqa;
	int m_tape_pia_irqb;
	int m_kb_pia_irqa;
	int m_kb_pia_irqb;
	int m_gpib_pia_irqa;
	int m_gpib_pia_irqb;
	int m_com_pia_irqa;
	int m_com_pia_irqb;
	int m_acia_irq;

	// keyboard
	int m_kbhalt;
	int m_kc;
	int m_key;

	// GPIB
	int m_talk;
};

class tek4052_state : public driver_device
{
public:
	tek4052_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, AM2901A_TAG),
		  m_ram(*this, RAM_TAG)
	 { }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	virtual void machine_start();

	virtual void video_start();
};

#endif
