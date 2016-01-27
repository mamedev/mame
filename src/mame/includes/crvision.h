// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder

#ifndef __CRVISION__
#define __CRVISION__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "bus/crvision/slot.h"
#include "bus/crvision/rom.h"
#include "machine/ram.h"
#include "sound/sn76496.h"
#include "sound/wave.h"
#include "video/tms9928a.h"

#define SCREEN_TAG      "screen"
#define M6502_TAG       "u2"
#define TMS9929_TAG     "u3"
#define PIA6821_TAG     "u21"
#define SN76489_TAG     "u22"
#define CENTRONICS_TAG  "centronics"

#define BANK_ROM1       "bank1"
#define BANK_ROM2       "bank2"

class crvision_state : public driver_device
{
public:
	crvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, M6502_TAG),
		m_pia(*this, PIA6821_TAG),
		m_psg(*this, SN76489_TAG),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_cent_data_out(*this, "cent_data_out"),
		m_ram(*this, RAM_TAG),
		m_inp_pa0(*this, "PA0"),
		m_inp_pa1(*this, "PA1"),
		m_inp_pa2(*this, "PA2"),
		m_inp_pa3(*this, "PA3")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<sn76496_base_device> m_psg;
	required_device<cassette_image_device> m_cassette;
	required_device<crvision_cart_slot_device> m_cart;
	required_device<output_latch_device> m_cent_data_out;
	required_device<ram_device> m_ram;
	optional_ioport_array<8> m_inp_pa0;
	optional_ioport_array<8> m_inp_pa1;
	optional_ioport_array<8> m_inp_pa2;
	optional_ioport_array<8> m_inp_pa3;

	UINT8 m_keylatch;
	UINT8 read_keyboard(int pa);

	DECLARE_WRITE8_MEMBER( pia_pa_w );
	DECLARE_READ8_MEMBER( pia_pa_r );
	DECLARE_READ8_MEMBER( pia_pb_r );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

protected:
	virtual void machine_start() override;
};

class crvision_pal_state : public crvision_state
{
public:
	crvision_pal_state(const machine_config &mconfig, device_type type, const char *tag)
		: crvision_state(mconfig, type, tag)
	{ }
};

class laser2001_state : public crvision_state
{
public:
	laser2001_state(const machine_config &mconfig, device_type type, const char *tag)
		: crvision_state(mconfig, type, tag),
		m_centronics(*this, CENTRONICS_TAG),
		m_inp_y(*this, "Y"),
		m_inp_joy(*this, "JOY")
	{ }

	required_device<centronics_device> m_centronics;
	required_ioport_array<8> m_inp_y;
	required_ioport_array<4> m_inp_joy;

	UINT8 m_joylatch;
	int m_centronics_busy;
	int m_psg_ready;

	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_psg_ready );
	DECLARE_READ8_MEMBER( pia_pa_r );
	DECLARE_WRITE8_MEMBER( pia_pa_w );
	DECLARE_READ8_MEMBER( pia_pb_r );
	DECLARE_WRITE8_MEMBER( pia_pb_w );
	DECLARE_READ_LINE_MEMBER( pia_ca1_r );
	DECLARE_WRITE_LINE_MEMBER( pia_ca2_w );
	DECLARE_READ_LINE_MEMBER( pia_cb1_r );
	DECLARE_WRITE_LINE_MEMBER( pia_cb2_w );

protected:
	virtual void machine_start() override;
};

#endif // __CRVISION__
