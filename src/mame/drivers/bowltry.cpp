// license:BSD-3-Clause
// copyright-holders:David Haywood
/************************************************************************************************************

    Bowling Try

    (c)200? Atlus

    TODO:
    - Tight loops at 0x60e090-0x60e093, control status from video chip?
    - YGV631-B ... what's that?

    ATLUS PCB  BT-208001
    ------------------------

    At U12 the chip is Toshiba TA8428FG

    At U1 the chip is H8/3008

    At X1 on the crystal it is printed S753

    big gfx chip marked

    YAMAHA JAPAN
    YGV631-B
    0806LU004

************************************************************************************************************/


#include "emu.h"
#include "cpu/h8/h83008.h"
#include "screen.h"

#define HACK_ENABLED 0

class bowltry_state : public driver_device
{
public:
	bowltry_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void bowltry(machine_config &config);

protected:
	void bowltry_map(address_map &map);

	uint32_t screen_update_bowltry(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int m_test_x;
	int m_test_y;
	int m_start_offs;
#if HACK_ENABLED
	DECLARE_READ16_MEMBER(hack_r);
	DECLARE_WRITE16_MEMBER(hack_w);
	uint16_t m_hack[2];
#endif

	required_device<cpu_device> m_maincpu;
};

#if HACK_ENABLED
READ16_MEMBER(bowltry_state::hack_r)
{
	if(offset)
		return m_hack[1] & ~0x20;

	m_hack[0]^=1;
	return m_hack[0];
}

WRITE16_MEMBER(bowltry_state::hack_w)
{
	COMBINE_DATA(&m_hack[offset]);
}
#endif

void bowltry_state::bowltry_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x080000, 0x083fff).ram();
	map(0x600000, 0x60ffff).ram();
#if HACK_ENABLED
	map(0x60e090, 0x60e093).rw(FUNC(bowltry_state::hack_r), FUNC(bowltry_state::hack_w));
#endif

}

static INPUT_PORTS_START( bowltry )
INPUT_PORTS_END

uint32_t bowltry_state::screen_update_bowltry(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}



MACHINE_CONFIG_START(bowltry_state::bowltry)
	MCFG_DEVICE_ADD("maincpu", H83008, 16000000 )
	MCFG_DEVICE_PROGRAM_MAP( bowltry_map )
//  MCFG_DEVICE_VBLANK_INT_DRIVER("screen", bowltry_state,  irq0_line_hold) // uses vector $64, IMIAB according to the manual (timer/compare B, internal to the CPU)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bowltry_state, screen_update_bowltry)
	//MCFG_SCREEN_PALETTE("palette")

	//MCFG_PALETTE_ADD("palette", 65536)

	/* tt5665 sound */

MACHINE_CONFIG_END

ROM_START( bowltry )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u30_v1.00.u30", 0x000000, 0x080000, CRC(2bd47419) SHA1(8fc975340e47ddeedf96e454a6c5372328f28b72) )

	ROM_REGION( 0x800000, "gfx", 0 ) // ???
	ROM_LOAD16_BYTE( "u27_v1.00.u27", 0x000000, 0x400000, CRC(80f51c25) SHA1(53c21325e7796197c26ca0cf4f8e51bf1e0bdcd3) )
	ROM_LOAD16_BYTE( "u28_v1.00.u28", 0x000001, 0x400000, CRC(9cc8b577) SHA1(6ef5cbb83860f88c9c83d4410034c5b528b2138b) )

	ROM_REGION( 0x400000, "tt5665", 0 ) // sound
	ROM_LOAD( "u24_v1.00.u24", 0x000000, 0x400000, CRC(4e082d58) SHA1(d2eb58bc3d8ade2ea556960013d580f0fb952090) )
ROM_END


GAME( 200?, bowltry, 0, bowltry, bowltry, bowltry_state, empty_init, ROT0, "Atlus", "Bowling Try", MACHINE_IS_SKELETON )
