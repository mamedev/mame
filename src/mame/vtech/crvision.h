// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder

#ifndef MAME_VTECH_CRVISION_H
#define MAME_VTECH_CRVISION_H

#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "bus/crvision/slot.h"
#include "bus/crvision/rom.h"
#include "machine/ram.h"
#include "sound/sn76496.h"
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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, M6502_TAG)
		, m_pia(*this, PIA6821_TAG)
		, m_psg(*this, SN76489_TAG)
		, m_cassette(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_centronics(*this, CENTRONICS_TAG)
		, m_cent_data_out(*this, "cent_data_out")
		, m_ram(*this, RAM_TAG)
		, m_io_keypad{{*this, "PA0.%u", 0U},{*this, "PA1.%u", 0U},{*this, "PA2.%u", 0U},{*this, "PA3.%u", 0U}}
	{ }

	void creativision(machine_config &config);
	void ntsc(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

protected:
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<sn76496_base_device> m_psg;
	required_device<cassette_image_device> m_cassette;
	required_device<crvision_cart_slot_device> m_cart;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<ram_device> m_ram;
	optional_ioport_array<8> m_io_keypad[4];

	virtual void machine_start() override ATTR_COLD;
	uint8_t m_keylatch = 0U;

private:
	void crvision_map(address_map &map) ATTR_COLD;
	uint8_t read_keyboard(u8 pa);

	void pia_pa_w(uint8_t data);
	uint8_t pia_pa_r();
	uint8_t pia_pb_r();
};

class crvision_pal_state : public crvision_state
{
public:
	crvision_pal_state(const machine_config &mconfig, device_type type, const char *tag)
		: crvision_state(mconfig, type, tag)
	{ }
	void pal(machine_config &config);
};

class laser2001_state : public crvision_state
{
public:
	laser2001_state(const machine_config &mconfig, device_type type, const char *tag)
		: crvision_state(mconfig, type, tag)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_inp_y(*this, "Y.%u", 0U)
		, m_inp_joy(*this, "JOY.%u", 0U)
	{ }

	void lasr2001(machine_config &config);

private:
	required_device<centronics_device> m_centronics;
	required_ioport_array<8> m_inp_y;
	required_ioport_array<4> m_inp_joy;

	uint8_t m_joylatch = 0U;
	int m_centronics_busy = 0;
	int m_psg_ready = 0;

	void write_centronics_busy(int state);
	void write_psg_ready(int state);
	uint8_t pia_pa_r();
	void pia_pa_w(uint8_t data);
	uint8_t pia_pb_r();
	void pia_pb_w(uint8_t data);
	int pia_ca1_r();
	void pia_ca2_w(int state);
	int pia_cb1_r();
	void pia_cb2_w(int state);

	void lasr2001_map(address_map &map) ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
};

#endif // MAME_VTECH_CRVISION_H
