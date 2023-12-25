// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "emu.h"

#include "cpu/ht1130/ht1130.h"

#include "speaker.h"

#include "brke23p2.lh"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class ht11xx_brickgame_state : public driver_device
{
public:
	ht11xx_brickgame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seg(*this, "seg%u", 0U),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2")
	{ }

	void ht11xx_brickgame(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<ht1130_device> m_maincpu;
	output_finder<512> m_seg;
	required_ioport m_in1;
	required_ioport m_in2;

	void display_offset_w(u8 data);
	void display_data_w(u8 data);

	u8 port_pm_r();
	u8 port_ps_r();
	u8 port_pp_r();

	u8 m_displayoffset = 0;
};

void ht11xx_brickgame_state::machine_start()
{
	m_seg.resolve();
}

void ht11xx_brickgame_state::machine_reset()
{
}

static INPUT_PORTS_START( ht11xx_brickgame )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Mute")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Power")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Rotate")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Drop")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Left")
INPUT_PORTS_END

void ht11xx_brickgame_state::display_offset_w(u8 data)
{
	m_displayoffset = data;
}

void ht11xx_brickgame_state::display_data_w(u8 data)
{
	for (int i = 0; i < 4; i++)
	{
		m_seg[i | (m_displayoffset * 4)] = (data >> i) & 1;
	}
}

u8 ht11xx_brickgame_state::port_ps_r()
{
	return m_in1->read() & 0xf;
}

u8 ht11xx_brickgame_state::port_pp_r()
{
	return m_in2->read() & 0xf;
}

void ht11xx_brickgame_state::ht11xx_brickgame(machine_config &config)
{
	HT1190(config, m_maincpu, 1000000/8); // frequency?
	m_maincpu->display_offset_out_cb().set(FUNC(ht11xx_brickgame_state::display_offset_w));
	m_maincpu->display_data_out_cb().set(FUNC(ht11xx_brickgame_state::display_data_w));

	m_maincpu->ps_in_cb().set(FUNC(ht11xx_brickgame_state::port_ps_r));
	m_maincpu->pp_in_cb().set(FUNC(ht11xx_brickgame_state::port_pp_r));

	SPEAKER(config, "speaker").front_center();

	config.set_default_layout(layout_brke23p2);
}

ROM_START( brke23p2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "e23plusmarkii96in1.bin", 0x0000, 0x1000, CRC(8045fac4) SHA1(a36213309e6add31f31e4248f02f17de9914a5c1) ) // visual decap
ROM_END

} // anonymous namespace

// some dieshots have 1996 on them, it is also possible the software is from Holtek
CONS( 1996?, brke23p2, 0, 0, ht11xx_brickgame, ht11xx_brickgame, ht11xx_brickgame_state, empty_init, "Holtek", "Brick Game 96 in 1 (E-23 Plus Mark II)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // needs SVG screen
