// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "emu.h"

#include "cpu/ht1130/ht1130.h"

#include "speaker.h"

#include "brke23p2.lh"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class ht1130_brickgame_state : public driver_device
{
public:
	ht1130_brickgame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seg(*this, "seg%u", 0U)
	{ }

	void ht1130_brickgame(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<ht1130_device> m_maincpu;

	void display_offset_w(uint8_t data);
	void display_data_w(uint8_t data);

	u8 m_displayoffset = 0;
	output_finder<512> m_seg;
};

void ht1130_brickgame_state::machine_start()
{
	m_seg.resolve();
}

void ht1130_brickgame_state::machine_reset()
{
}

static INPUT_PORTS_START( ht1130_brickgame )
INPUT_PORTS_END

void ht1130_brickgame_state::display_offset_w(uint8_t data)
{
	m_displayoffset = data;
}

void ht1130_brickgame_state::display_data_w(uint8_t data)
{
	for (int i = 0; i < 4; i++)
	{
		m_seg[i | (m_displayoffset * 4)] = (data >> i) & 1;
	}
}


void ht1130_brickgame_state::ht1130_brickgame(machine_config &config)
{
	HT1130(config, m_maincpu, 1000000);
	m_maincpu->display_offset_out_cb().set(FUNC(ht1130_brickgame_state::display_offset_w));
	m_maincpu->display_data_out_cb().set(FUNC(ht1130_brickgame_state::display_data_w));
	SPEAKER(config, "speaker").front_center();

	config.set_default_layout(layout_brke23p2);
}

ROM_START( brke23p2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "e23plusmarkii96in1.bin", 0x0000, 0x1000, CRC(8045fac4) SHA1(a36213309e6add31f31e4248f02f17de9914a5c1) ) // visual decap
ROM_END

} // anonymous namespace


CONS( 200?, brke23p2, 0, 0, ht1130_brickgame, ht1130_brickgame, ht1130_brickgame_state, empty_init, "<unknown>", "Brick Game 96 in 1 (E-23 Plus Mark II)", MACHINE_IS_SKELETON )
