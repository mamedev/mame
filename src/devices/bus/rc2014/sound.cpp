// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Sound

****************************************************************************/

#include "emu.h"
#include "sound.h"
#include "sound/ay8910.h"
#include "speaker.h"

namespace {

//**************************************************************************
//  RC2014 YM2149F/AY-3-8190 Sound card
//  Module author: Ed Brindley
//**************************************************************************

class rc2014_ym_ay_device : public device_t, public device_rc2014_card_interface
{
protected:
	// construction/destruction
	rc2014_ym_ay_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// base class members
	required_device<ay8910_device> m_psg;
	required_ioport_array<6> m_jp;
};

rc2014_ym_ay_device::rc2014_ym_ay_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_psg(*this, "psg")
	, m_jp(*this, "JP%u", 1U)
{
}

void rc2014_ym_ay_device::device_start()
{
}

void rc2014_ym_ay_device::device_reset()
{
	uint16_t base;
	uint16_t reg;
	// A13-A8 and A0 not connected
	uint16_t mask = 0x3f01;
	// JP2 - Addressing mode
	if (m_jp[1]->read())
	{
		base = 0x8000; // A15 for Spectrum mode
		mask |= 0x0F00; // A7-A4 are not connected
	}
	else
	{
		base = m_jp[3]->read() << 4; // JP4
		base |= m_jp[0]->read() ? 0x10 : 0x00; // JP1
		mask |= 0x8000; // A15 not connected
	}
	// JP3 - Register mode
	switch(m_jp[2]->read())
	{
		case 0 : // A14 (Spectrum)
			reg = 0x4000; // A14
			mask |= 0x000c; // A2 and A3 not connected
			break;
		case 1 : // A3 (Base + 8)
			reg = 0x0008; // A3
			mask |= 0x4004; // A14 and A2 not connected
			break;
		default : // A2 (Base + 4)
			reg = 0x0004; // A2
			mask |= 0x4008; // A14 and A3 not connected
			break;
	}
	m_bus->installer(AS_IO)->install_write_handler(base, base, 0, mask, 0, write8smo_delegate(m_psg, FUNC(ay8910_device::data_w)));
	m_bus->installer(AS_IO)->install_readwrite_handler(base + reg, base + reg, 0, mask, 0, read8smo_delegate(m_psg, FUNC(ay8910_device::data_r)), write8smo_delegate(m_psg, FUNC(ay8910_device::address_w)));
	// JP5 - Clock divider
	m_psg->set_clock(clock() / m_jp[4]->read());
}

static INPUT_PORTS_START( rc2014_ym_ay_jumpers )
	PORT_START("JP1") // JP1 and JP7
	PORT_CONFNAME( 0x1, 0x1, "A4 Chip Enable" )
	PORT_CONFSETTING( 0x0, "Low" )
	PORT_CONFSETTING( 0x1, "High" )
	PORT_START("JP2")
	PORT_CONFNAME( 0x1, 0x0, "Addressing mode" )
	PORT_CONFSETTING( 0x0, "Default" )
	PORT_CONFSETTING( 0x1, "Spectrum 128" )
	PORT_START("JP3")
	PORT_CONFNAME( 0x3, 0x1, "Register mode" )
	PORT_CONFSETTING( 0x0, "A14 (Spectrum)" )
	PORT_CONFSETTING( 0x1, "A3 (Base + 8)" )
	PORT_CONFSETTING( 0x2, "A2 (Base + 4)" )
	PORT_START("JP4")
	PORT_CONFNAME( 0xf, 0xc, "Base Address" )
	PORT_CONFSETTING( 0x0, "0x00 / 0x10" )
	PORT_CONFSETTING( 0x8, "0x80 / 0x90" )
	PORT_CONFSETTING( 0x4, "0x40 / 0x50" )
	PORT_CONFSETTING( 0xc, "0xC0 / 0xD0" )
	PORT_CONFSETTING( 0x2, "0x20 / 0x30" )
	PORT_CONFSETTING( 0xa, "0xA0 / 0xB0" )
	PORT_CONFSETTING( 0x6, "0x60 / 0x70" )
	PORT_CONFSETTING( 0xe, "0xE0 / 0xF0" )
	PORT_START("JP5")
	PORT_CONFNAME( 0x7, 0x4, "Divide by" )
	PORT_CONFSETTING( 0x2, "2" )
	PORT_CONFSETTING( 0x4, "4" )
	PORT_START("JP6")
	PORT_CONFNAME( 0x1, 0x0, "YM2149 half clock" )
	PORT_CONFSETTING( 0x0, DEF_STR( No ) )
	PORT_CONFSETTING( 0x1, DEF_STR( Yes ) )
INPUT_PORTS_END

ioport_constructor rc2014_ym_ay_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_ym_ay_jumpers );
}

//**************************************************************************
//  With YM2149F chip
//**************************************************************************

class rc2014_ym2149_device : public rc2014_ym_ay_device
{
public:
	// construction/destruction
	rc2014_ym2149_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

rc2014_ym2149_device::rc2014_ym2149_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: rc2014_ym_ay_device(mconfig, RC2014_YM2149_SOUND, tag, owner, clock)
{
}

void rc2014_ym2149_device::device_reset()
{
	rc2014_ym_ay_device::device_reset();
	// JP6 - YM2149 half clock
	if (m_jp[5]->read())
	{
		m_psg->set_pin26_low_w();
	}
}

void rc2014_ym2149_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2149(config, m_psg, 0);
	m_psg->add_route(0, "rspeaker", 0.25);
	m_psg->add_route(2, "rspeaker", 0.25);
	m_psg->add_route(1, "lspeaker", 0.25);
	m_psg->add_route(2, "lspeaker", 0.25);
}

//**************************************************************************
//  With AY-3-8190 chip
//**************************************************************************

class rc2014_ay8190_device : public rc2014_ym_ay_device
{
public:
	// construction/destruction
	rc2014_ay8190_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

rc2014_ay8190_device::rc2014_ay8190_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: rc2014_ym_ay_device(mconfig, RC2014_AY8190_SOUND, tag, owner, clock)
{
}

void rc2014_ay8190_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	AY8910(config, m_psg, 0);
	m_psg->add_route(0, "rspeaker", 0.25);
	m_psg->add_route(2, "rspeaker", 0.25);
	m_psg->add_route(1, "lspeaker", 0.25);
	m_psg->add_route(2, "lspeaker", 0.25);
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_YM2149_SOUND, device_rc2014_card_interface, rc2014_ym2149_device, "rc2014_ym2149", "RC2014 YM2149F Sound card")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_AY8190_SOUND, device_rc2014_card_interface, rc2014_ay8190_device, "rc2014_ay8190", "RC2014 AY-3-8190 Sound card")
