// license:BSD-3-Clause
// copyright-holders:

/*
Biorhythm machine

Etched in copper on top of board
(c) THE BIO RHYTHM CO INC 1979
BIORHYTHM CONTROL CARD
SR. NO. F

Main components:
HD46802P
4.000 MHz XTAL
3x EF6821P
MC14028
NE555N
*/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"

#include "speaker.h"


namespace {

class biorhythm_state : public driver_device
{
public:
	biorhythm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void biorhythm(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map);
};


void biorhythm_state::machine_start()
{
}


void biorhythm_state::program_map(address_map &map)
{
	map.global_mask(0x1fff);

	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0804, 0x0807).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0808, 0x080b).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1fff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START(biorhythm)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void biorhythm_state::biorhythm(machine_config &config)
{
	M6802(config, m_maincpu, 4_MHz_XTAL); // Actually a HD46802P
	m_maincpu->set_addrmap(AS_PROGRAM, &biorhythm_state::program_map);

	PIA6821(config, "pia0");

	PIA6821(config, "pia1");

	PIA6821(config, "pia2");

	SPEAKER(config, "speaker").front_center();
}


ROM_START( biorhthm )
	ROM_REGION( 0x1000, "maincpu", 0 ) // hand-written labels
	ROM_LOAD( "1000_v2.0.u5", 0x000, 0x800, CRC(ba96c674) SHA1(12fd665e69177975438a9a2dc63b97897159bfae) )
	ROM_LOAD( "1800_2.0.u11", 0x800, 0x800, CRC(e499ccfa) SHA1(426c4462399a89253a17a99a3c814f65a95aa739) )
ROM_END

} // anonymous namespace


GAME( 1979?, biorhthm, 0, biorhythm, biorhythm, biorhythm_state, empty_init, ROT0, "Bio Rhythm", "Biorhythm", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // title may be wrong / incomplete
