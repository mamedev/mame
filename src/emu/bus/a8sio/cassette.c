// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  cassette.h - Atari 8 bit cassette player(s)


Known cassette players:
- Atari XC11
- Atari XC12 (no SIO connection for an additional device)

TODO:
- Implement cassette reading
- Implement cassette writing
- Add audio support
- Add SIO connector for a next device

***************************************************************************/

#include "emu.h"
#include "cassette.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A8SIO_CASSETTE = &device_creator<a8sio_cassette_device>;

static MACHINE_CONFIG_FRAGMENT( cassette )
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)
	MCFG_CASSETTE_INTERFACE("atari8bit_cass")
MACHINE_CONFIG_END

machine_config_constructor a8sio_cassette_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cassette );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a8sio_cassette_device::a8sio_cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, A8SIO_CASSETTE, "Atari 8 bit cassette", tag, owner, clock, "a8sio_cass", __FILE__)
	, device_a8sio_card_interface(mconfig, *this)
	, m_cassette(*this, "cassette")
{
}

a8sio_cassette_device::a8sio_cassette_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_a8sio_card_interface(mconfig, *this)
	, m_cassette(*this, "cassette")
{
}

void a8sio_cassette_device::device_start()
{
	set_a8sio_device();
}

void a8sio_cassette_device::device_reset()
{
}

WRITE_LINE_MEMBER( a8sio_cassette_device::motor_w )
{
	//printf("a8sio_cassette::motor_w %d\n", state);
}

