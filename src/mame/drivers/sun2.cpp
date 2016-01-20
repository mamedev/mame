// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Sun-2 Models
        ------------

    2/120
        Processor(s):   68010 @ 10MHz
        CPU:            501-1007/1051
        Chassis type:   deskside
        Bus:            Multibus (9 slots)
        Memory:         7M physical
        Notes:          First machines in deskside chassis. Serial
                        microswitch keyboard (type 2), Mouse Systems
                        optical mouse (Sun-2).

    2/100U
        Processor(s):   68010 @ 10MHz
        CPU:            501-1007
        Bus:            Multibus
        Notes:          Upgraded Sun 100. Replaced CPU and memory boards
                        with first-generation Sun-2 CPU and memory
                        boards so original customers could run SunOS
                        1.x. Still has parallel kb/mouse interface so
                        type 1 keyboards and Sun-1 mice could be
                        connected.

    2/150U
        Notes:          Apparently also an upgraded Sun-1.

    2/170
        Chassis type:   rackmount
        Bus:            Multibus (15 slots)
        Notes:          Rackmount version of 2/120, with more slots.

    2/50
        Processor(s):   68010 @ 10MHz
        CPU:            501-1141/1142/1143/1426/1427/1428
        Chassis type:   wide pizza box
        Bus:            VME (2 slots)
        Memory:         7M physical
        Notes:          The (type 2) keyboard and mouse attach via an
                        adapter that accepts two modular plugs and
                        attaches to a DB15 port; later on, units were
                        apparently shipped with type 3 keyboards. The
                        CPU boards have a double-width back panel but
                        are otherwise identical to those in the 2/130
                        and 2/160.

    2/130
    2/160
        Processor(s):   68010 @ 10MHz
        CPU:            501-1144/1145/1146/1429/1430/1431
        Chassis type:   deskside
        Bus:            VME (12 slots)
        Memory:         7M physical
        Notes:          First machine in 12-slot deskside VME chassis.
                        Has four-fan cooling tray instead of six as in
                        later machines, which led to cooling problems
                        with lots of cards. Backplane has only four P2
                        memory connectors bussed instead of six as in
                        later 12-slot backplanes; SCSI passthrough is in
                        slot 6 instead of 7 as in later 12-slot
                        backplanes. Upgradeable to a 3/160 by replacing
                        the CPU board. No information on the differences
                        between the 2/130 and the 2/160.


        25/08/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"


class sun2_state : public driver_device
{
public:
	sun2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	,
		m_p_ram(*this, "p_ram"){ }

	required_device<cpu_device> m_maincpu;
	virtual void machine_reset() override;

	required_shared_ptr<UINT16> m_p_ram;
};

static ADDRESS_MAP_START(sun2_mem, AS_PROGRAM, 16, sun2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM AM_SHARE("p_ram") // 8MB
	AM_RANGE(0x00ef0000, 0x00ef7fff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sun2 )
INPUT_PORTS_END


void sun2_state::machine_reset()
{
	UINT8* user1 = memregion("user1")->base();

	memcpy((UINT8*)m_p_ram.target(),user1,0x8000);

	m_maincpu->reset();
}


static MACHINE_CONFIG_START( sun2, sun2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68010, 16670000)
	MCFG_CPU_PROGRAM_MAP(sun2_mem)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sun2_120 )
	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "sun2-multi-rev-r.bin", 0x0000, 0x8000, CRC(4df0df77) SHA1(4d6bcf09ddc9cc8f5823847b8ea88f98fe4a642e))
ROM_END

ROM_START( sun2_50)
	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "250_q_8.rom", 0x0001, 0x4000, CRC(5bfacb5c) SHA1(ec7fb3fb0217b0138ba4748b7c79b8ff0cad896b))
	ROM_LOAD16_BYTE( "250_q_0.rom", 0x0000, 0x4000, CRC(2ee29abe) SHA1(82f52b9f25e92387329581f7c8ba50a171784968))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY         FULLNAME       FLAGS */
COMP( 1984, sun2_50,   0,       0,       sun2,      sun2, driver_device,     0,  "Sun Microsystems", "Sun 2/50", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1984, sun2_120,  0,       0,       sun2,      sun2, driver_device,     0,  "Sun Microsystems", "Sun 2/120", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
