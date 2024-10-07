// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    rs6000_type7xxx.cpp - IBM RS/6000 PowerPC based MicroChannel systems
    Skeleton by R. Belmont

    Type 7009: PPC 601 @ 80 MHz, serial console, SCSI, 3-digit 7-segment display,
    4 MCA slots, Ethernet, and a 2.88MB floppy with an unknown controller

    The NetBSD boot log for a Type 7011 indicates the integrated SCSI is
    an NCR 53C720 appearing as an MCA board with the ID word 0x8fba.
    The "I/O planar" appears as an MCA board with the ID word 0x8f98 and
    it contains two NS16550A UARTs and an unknown Ethernet chip.

    References:
    https://www.ibm.com/common/ssi/ShowDoc.wss?docURL=/common/ssi/rep_sm/0/897/ENUS7009-C10/index.html&lang=en_US
    http://mail-index.netbsd.org/port-prep/2008/01/15/msg000001.html

************************************************************************/

#include "emu.h"

#include "cpu/powerpc/ppc.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "speaker.h"

namespace {

class type7xxx_state : public driver_device
{
public:
	type7xxx_state(const machine_config &mconfig, device_type type, const char *tag);

	void type7009(machine_config &config);

private:
	required_device<ppc_device> m_maincpu;

	void type7009_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

type7xxx_state::type7xxx_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
{
}

void type7xxx_state::type7009_map(address_map &map)
{
	map(0xfff00000, 0xfff7ffff).rom().region("bootrom", 0);
}

void type7xxx_state::type7009(machine_config &config)
{
	PPC601(config, m_maincpu, 80000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &type7xxx_state::type7009_map);
}

static INPUT_PORTS_START( type7009 )
INPUT_PORTS_END

ROM_START( rs6k7009 )
	ROM_REGION64_BE(0x80000, "bootrom", 0)
	ROM_LOAD( "am27c040@dip32_ibm_7009_c10.bin", 0x000000, 0x080000, CRC(ff889bba) SHA1(b633584a0707913ac42ec127fcc567aa27d8af06) )
ROM_END

}   // anonymous namespace

COMP( 1994, rs6k7009,  0, 0, type7009, type7009, type7xxx_state, empty_init, "International Business Machines", "RS/6000 Type 7009 Model C10 Server",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND_HW )
