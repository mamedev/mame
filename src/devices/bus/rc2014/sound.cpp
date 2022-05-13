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
public:
	// construction/destruction
	rc2014_ym_ay_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
    virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
    virtual ioport_constructor device_input_ports() const override;
private:
	required_device<ay8910_device> m_ay8910;
    required_ioport_array<6> m_jp;
};

rc2014_ym_ay_device::rc2014_ym_ay_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_YM_AY_SOUND, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_ay8910(*this, "ay8190")
    , m_jp(*this, "JP%u", 1U)
{
}

void rc2014_ym_ay_device::device_start()
{
}

void rc2014_ym_ay_device::device_reset()
{
    // Set clock
    m_ay8910->set_clock(clock() / m_jp[4]->read()); // JP5
	uint8_t base = m_jp[3]->read() << 4; // JP4
    uint8_t offset = (m_jp[2]->read() == 1) ? 8 : (m_jp[2]->read() == 2) ? 4 : 0; // JP3
    m_bus->installer(AS_IO)->install_write_handler(base, base, 0, 0, 0, write8smo_delegate(m_ay8910, FUNC(ay8910_device::data_w)));
    m_bus->installer(AS_IO)->install_readwrite_handler(base+offset, base+offset, 0, 0, 0, read8smo_delegate(m_ay8910, FUNC(ay8910_device::data_r)), write8smo_delegate(m_ay8910, FUNC(ay8910_device::address_w)));
}

void rc2014_ym_ay_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	AY8910(config, m_ay8910, 0);
	m_ay8910->add_route(0, "lspeaker", 0.25);
	m_ay8910->add_route(2, "lspeaker", 0.25);
	m_ay8910->add_route(1, "rspeaker", 0.25);
	m_ay8910->add_route(2, "rspeaker", 0.25);
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
	PORT_CONFNAME( 0xf, 0xd, "Base Address" )
	PORT_CONFSETTING( 0x0, "0x00" )
	PORT_CONFSETTING( 0x1, "0x10" )
    PORT_CONFSETTING( 0x2, "0x20" )
    PORT_CONFSETTING( 0x3, "0x30" )
    PORT_CONFSETTING( 0x4, "0x40" )
    PORT_CONFSETTING( 0x5, "0x50" )
    PORT_CONFSETTING( 0x6, "0x60" )
    PORT_CONFSETTING( 0x7, "0x70" )
    PORT_CONFSETTING( 0x8, "0x80" )
    PORT_CONFSETTING( 0x9, "0x90" )
    PORT_CONFSETTING( 0xa, "0xa0" )
    PORT_CONFSETTING( 0xb, "0xb0" )
    PORT_CONFSETTING( 0xc, "0xc0" )
    PORT_CONFSETTING( 0xd, "0xd0" )
    PORT_CONFSETTING( 0xe, "0xe0" )
    PORT_CONFSETTING( 0xf, "0xf0" )
	PORT_START("JP5")
	PORT_CONFNAME( 0x7, 0x4, "Divide by" )
	PORT_CONFSETTING( 0x2, "2" )
	PORT_CONFSETTING( 0x4, "4" )
	PORT_START("JP6") // Unused for now
	PORT_CONFNAME( 0x1, 0x0, "YM2149 half clock" )
	PORT_CONFSETTING( 0x0, DEF_STR( No ) )
	PORT_CONFSETTING( 0x1, DEF_STR( Yes ) )
INPUT_PORTS_END

ioport_constructor rc2014_ym_ay_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_ym_ay_jumpers );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_YM_AY_SOUND, device_rc2014_card_interface, rc2014_ym_ay_device, "rc2014_ym_ay", "RC2014 YM2149F/AY-3-8190 Sound card")
