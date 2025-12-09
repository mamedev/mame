// license:BSD-3-Clause
// copyright-holders:

/*
Biorhythm machine by For-Play

Two different PCB sets were dumped, with identical ROM contents

First PCB:

Etched in copper on main board  BIO-RHYTHM LOGIC
                                FOR-PLAY MADE IN USA REV G
                                ADJUST FOR 2V ON BLACK
                                WILL BE LESS THAN 1V ON WHITE

Etched in copper on CPU board   FP 4X-1280 COMPUTER REV A   FOR-PLAY
                                MADE IN USA

Etched in copper on back of driver board    BIO-RHYTHM DRIVER
                                            REV E
                                            FOR-PLAY
                                            MADE IN USA

Etched in copper on back of little vertical board   PHOTOCELL
                                                    AUTO
                                                    ADJUST

Second PCB:

Etched in copper on main board      BIO-RHYTHM LOGIC
                                    FOR-PLAY MADE IN USA REV H

Etched in copper on CPU board       FP 4X-1280 COMPUTER REV A   FOR-PLAY
                                    MADE IN USA

Etched in copper on back of driver board    BIO-RHYTHM DRIVER
                                            REV E
                                            FOR-PLAY
                                            MADE IN USA

Main components:
C4004 CPU
P4002-1 RAM
5x P4001 ROM
various TTL chips
*/


#include "emu.h"

#include "cpu/mcs40/mcs40.h"

#include "speaker.h"


namespace {

class biorhythm_fp_state : public driver_device
{
public:
	biorhythm_fp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void biortmfp(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void memory_map(address_map &map) ATTR_COLD;
	void rom_map(address_map &map) ATTR_COLD;
	void status_map(address_map &map) ATTR_COLD;
};


void biorhythm_fp_state::rom_map(address_map &map)
{
	map(0x0000, 0x04ff).rom().region("maincpu", 0x0000);
}

void biorhythm_fp_state::memory_map(address_map &map)
{
	map(0x0000, 0x003f).ram();
}

void biorhythm_fp_state::status_map(address_map &map)
{
	map(0x0000, 0x000f).ram();
}


static INPUT_PORTS_START(biortmfp)
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


void biorhythm_fp_state::biortmfp(machine_config &config)
{
	i4040_cpu_device &cpu(I4040(config, "maincpu", 750'000)); // clock unknown
	cpu.set_rom_map(&biorhythm_fp_state::rom_map);
	cpu.set_ram_memory_map(&biorhythm_fp_state::memory_map);
	cpu.set_ram_status_map(&biorhythm_fp_state::status_map);
}


ROM_START( biortmfp )
	ROM_REGION( 0x500, "maincpu", 0 )
	ROM_LOAD( "p4001_0846.0", 0x000, 0x100, CRC(bd483c96) SHA1(c73ca9446a5da4d78d9a918350db26a3071d2940) )
	ROM_LOAD( "p4001_0848.1", 0x100, 0x100, CRC(9799470a) SHA1(b0ece482b3369cb8907554f509ca1c21424119d9) )
	ROM_LOAD( "p4001_0849.2", 0x200, 0x100, CRC(81bea767) SHA1(b8d413916530bf55333a7bb69eaa9f478e456361) )
	ROM_LOAD( "p4001_0847.3", 0x300, 0x100, CRC(eddf6e11) SHA1(01fac158e330b8c7f6336e9b66eeb20e535dc5b3) )
	ROM_LOAD( "p4001_0850.4", 0x400, 0x100, CRC(6c66b740) SHA1(03013ee965c95d0e08830e3b95532b6430573909) )
ROM_END

} // anonymous namespace


GAME( 197?, biortmfp, 0, biortmfp, biortmfp, biorhythm_fp_state, empty_init, ROT0, "For-Play", "Biorhythm (For-Play)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
