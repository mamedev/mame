// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Turbo Drive by Innovative Creations in Entertainment (ICE)

http://www.arcade-museum.com/game_detail.php?game_id=10658


Slot car type race game, coin operated.

Device is a 27c128

--
not sure what the actual inputs / outputs would be on this, maybe just track position / lap sensors?


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"

class ice_tbd_state : public driver_device
{
public:
	ice_tbd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ice_tbd(machine_config &config);

private:
	void ice_tbd_io_map(address_map &map);
	void ice_tbd_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};



void ice_tbd_state::ice_tbd_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
}

void ice_tbd_state::ice_tbd_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


static INPUT_PORTS_START( ice_tbd )
INPUT_PORTS_END

void ice_tbd_state::machine_start()
{
}

void ice_tbd_state::machine_reset()
{
}

MACHINE_CONFIG_START(ice_tbd_state::ice_tbd)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80,8000000)         /* ? MHz */
	MCFG_DEVICE_PROGRAM_MAP(ice_tbd_map)
	MCFG_DEVICE_IO_MAP(ice_tbd_io_map)

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set_nop(); // ?
	ppi.out_pb_callback().set_nop(); // ?
	ppi.in_pc_callback().set_constant(0); // ?
MACHINE_CONFIG_END


ROM_START( ice_tbd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "turbo-dr.ive", 0x0000, 0x4000, CRC(d7c79ac4) SHA1(a01d93411e604e36a3ced58063f2ab81e431b82a) )
ROM_END


GAME( 1988, ice_tbd, 0, ice_tbd, ice_tbd, ice_tbd_state, empty_init, ROT0, "Innovative Creations in Entertainment", "Turbo Drive (ICE)", MACHINE_IS_SKELETON_MECHANICAL )
