// license:BSD-3-Clause
// copyright-holders:
// PINBALL

// Skeleton driver for Mirco's Lucky Draw
// Hardware listing and ROM definitions from PinMAME.


#include "emu.h"
#include "machine/genpin.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/nvram.h"

class lckydraw_state : public genpin_class
{
public:
	lckydraw_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

	void lckydraw(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void maincpu_map(address_map &map);
};

void lckydraw_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
}


static INPUT_PORTS_START( lckydraw )
INPUT_PORTS_END

void lckydraw_state::lckydraw(machine_config &config)
{
	/* basic machine hardware */
	I8035(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &lckydraw_state::maincpu_map);

	/* video hardware */
	//config.set_default_layout();

	/* sound hardware */
	genpin_audio(config);
}


ROM_START(lckydraw)
	ROM_REGION( 0xc00, "maincpu", 0)
	ROM_LOAD( "lckydrw1.rom", 0x0000, 0x0400, CRC(58ebb50f) SHA1(016ed66b4ee9979aa109c0ce085597a62d33bf8d) )
	ROM_LOAD( "lckydrw2.rom", 0x0400, 0x0400, CRC(816b9e20) SHA1(0dd8acc633336f250960ebe89cc707fd115afeee) )
	ROM_LOAD( "lckydrw3.rom", 0x0800, 0x0400, CRC(464155bb) SHA1(5bbf784dba9149575444e6b1250ac9b5c2bced87) )

ROM_END


GAME( 1979, lckydraw, 0, lckydraw, lckydraw, lckydraw_state, empty_init, ROT0, "Mirco", "Lucky Draw (Pinball)", MACHINE_IS_SKELETON_MECHANICAL )
