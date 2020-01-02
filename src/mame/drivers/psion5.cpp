// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Ash Wolf
/***************************************************************************

        Psion 5mx (EPOC R5) series

        Skeleton driver by Ryan Holtz, ported from work by Ash Wolf

        TODO:
        - everything

        More info:
            https://github.com/Treeki/WindEmu

****************************************************************************/


#include "emu.h"
#include "cpu/arm7/arm7.h"

#include "screen.h"

class psion5_state : public driver_device
{
public:
	psion5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void psion5mx(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void main_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<arm710t_cpu_device> m_maincpu;
};

void psion5_state::machine_start()
{
}

void psion5_state::machine_reset()
{
}

void psion5_state::main_map(address_map &map)
{
	map(0x00000000, 0x009fffff).rom().region("maincpu", 0);
	map(0xc0000000, 0xc03fffff).ram().mirror(0x1fc00000);
}

uint32_t psion5_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* Input ports */
INPUT_PORTS_START( psion5 )
INPUT_PORTS_END

/* basic configuration for 2 lines display */
void psion5_state::psion5mx(machine_config &config)
{
	/* basic machine hardware */
	ARM710T(config, m_maincpu, 18000000); // 18MHz, per wikipedia
	m_maincpu->set_addrmap(AS_PROGRAM, &psion5_state::main_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0)); /* not accurate */
	screen.set_screen_update(FUNC(psion5_state::screen_update));
	screen.set_size(640, 240);
	screen.set_visarea(0, 640-1, 0, 240-1);
}

/* ROM definition */

ROM_START( psion5mx )
	ROM_REGION( 0xa00000, "maincpu", 0 )
	ROM_LOAD( "5mx.rom", 0x000000, 0xa00000, CRC(a1e2d038) SHA1(4c082321264e1ae7fe77699e59b8960460690fa6) )
ROM_END

/* Driver */

//    YEAR  NAME        PARENT   COMPAT  MACHINE    INPUT   CLASS         INIT        COMPANY  FULLNAME  FLAGS
COMP( 1999, psion5mx,   0,       0,      psion5mx,  psion5, psion5_state, empty_init, "Psion", "5mx",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
