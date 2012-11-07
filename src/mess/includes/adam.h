#pragma once

#ifndef ADAM_H_
#define ADAM_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cartslot.h"
#include "machine/adamnet.h"
#include "machine/coleco.h"
#include "machine/ram.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"

#define Z80_TAG         "u1"
#define M6801_TAG       "u6"
#define SN76489A_TAG    "u20"
#define TMS9928A_TAG    "tms9928a"
#define SCREEN_TAG      "screen"

class adam_state : public driver_device
{
public:
	adam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_netcpu(*this, M6801_TAG),
			m_ram(*this, RAM_TAG),
			m_adamnet(*this, ADAMNET_TAG),
			m_dma(1),
			m_bwr(1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_netcpu;
	required_device<ram_device> m_ram;
	required_device<adamnet_device> m_adamnet;

	virtual void machine_start();
	virtual void machine_reset();

	void bankswitch();

	DECLARE_WRITE_LINE_MEMBER( os3_w );

	DECLARE_READ8_MEMBER( adamnet_r );
	DECLARE_WRITE8_MEMBER( adamnet_w );
	DECLARE_READ8_MEMBER( mioc_r );
	DECLARE_WRITE8_MEMBER( mioc_w );
	DECLARE_WRITE8_MEMBER( paddle_w );
	DECLARE_WRITE8_MEMBER( joystick_w );
	DECLARE_READ8_MEMBER( input1_r );
	DECLARE_READ8_MEMBER( input2_r );

	DECLARE_WRITE8_MEMBER( m6801_p1_w );
	DECLARE_READ8_MEMBER( m6801_p2_r );
	DECLARE_WRITE8_MEMBER( m6801_p2_w );
	DECLARE_READ8_MEMBER( m6801_p3_r );
	DECLARE_WRITE8_MEMBER( m6801_p3_w );
	DECLARE_WRITE8_MEMBER( m6801_p4_w );

	DECLARE_WRITE_LINE_MEMBER( vdc_int_w );

	// memory state
	UINT8 m_mioc;
	int m_game;

	// ADAMnet state
	UINT8 m_an;

	// DMA state
	UINT16 m_ba;
	int m_dma;
	int m_bwr;
	UINT8 m_data_in;
	UINT8 m_data_out;

	// paddle state
	int m_joy_mode;
	UINT8 m_joy_status0;
	UINT8 m_joy_status1;

	// video state
	int m_vdp_nmi;

	TIMER_DEVICE_CALLBACK_MEMBER(paddle_tick);
};

#endif
