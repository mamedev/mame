// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "emu.h"

#include "cpu/ht1130/ht1130.h"

#include "speaker.h"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class ht1130_brickgame_state : public driver_device
{
public:
	ht1130_brickgame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void ht1130_brickgame(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<ht1130_device> m_maincpu;
};

void ht1130_brickgame_state::machine_start()
{
}

void ht1130_brickgame_state::machine_reset()
{
}

static INPUT_PORTS_START( ht1130_brickgame )
INPUT_PORTS_END


void ht1130_brickgame_state::ht1130_brickgame(machine_config &config)
{
	HT1130(config, m_maincpu, 1000000);

	SPEAKER(config, "speaker").front_center();
}

ROM_START( brke23p2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "e23plusmarkii96in1.bin", 0x0000, 0x1000, CRC(8045fac4) SHA1(a36213309e6add31f31e4248f02f17de9914a5c1) ) // visual decap
ROM_END

} // anonymous namespace


CONS( 200?, brke23p2, 0, 0, ht1130_brickgame, ht1130_brickgame, ht1130_brickgame_state, empty_init, "<unknown>", "E23 Plus Mark II 96-in-1", MACHINE_IS_SKELETON )
