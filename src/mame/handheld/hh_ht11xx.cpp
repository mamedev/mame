// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

The following should fit here

HT1132A Space War
HT1134A Pin Ball
HT1136A Football
HT1137A Motorcycle
HT113AA Streetfighters
HT113FA Submarine War
HT113JA Baseball
HT113RA Poker and Black Jack
HT113SA Casino Game 5-in-1
HT113LA Original "Tea" Brick Game
HTG1395 3-in-1 (Car racing, Soccer, The eagle preys on the chicken)

(and likely many more)

TODO:
- add LCD deflicker like hh_sm510?

*/

#include "emu.h"

#include "cpu/ht1130/ht1130.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "hh_ht11xx_single.lh"


namespace {

class hh_ht11xx_state : public driver_device
{
public:
	hh_ht11xx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_in(*this, "IN%u", 1)
	{ }

	virtual DECLARE_INPUT_CHANGED_MEMBER(input_wakeup);

	void brke23p2(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void mcfg_svg_screen(machine_config &config, u16 width, u16 height, const char *tag = "screen");

	void segment_w(offs_t offset, u64 data);

	required_device<ht1130_device> m_maincpu;
	output_finder<8, 40> m_out_x;
	required_ioport_array<2> m_in;
};

void hh_ht11xx_state::machine_start()
{
	m_out_x.resolve();
}

INPUT_CHANGED_MEMBER(hh_ht11xx_state::input_wakeup)
{
	m_maincpu->set_input_line(HT1130_EXT_WAKEUP_LINE, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( brke23p2 )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Mute")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POWER_ON ) PORT_NAME("Power") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_ht11xx_state, input_wakeup, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2") // not a joystick, but buttons are used for directional inputs in the snake game etc.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Up / Rotate")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Down / Drop")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Left")
INPUT_PORTS_END


void hh_ht11xx_state::segment_w(offs_t offset, u64 data)
{
	for (int i = 0; i < 40; i++)
	{
		// output to x.y where x = COM# and y = SEG#
		m_out_x[offset][i] = BIT(data, i);
	}
}

void hh_ht11xx_state::mcfg_svg_screen(machine_config &config, u16 width, u16 height, const char *tag)
{
	screen_device &screen(SCREEN(config, tag, SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(width, height);
	screen.set_visarea_full();

	config.set_default_layout(layout_hh_ht11xx_single);
}

void hh_ht11xx_state::brke23p2(machine_config &config)
{
	HT1190(config, m_maincpu, 1000000/8); // frequency?
	m_maincpu->segment_out_cb().set(FUNC(hh_ht11xx_state::segment_w));

	m_maincpu->ps_in_cb().set_ioport(m_in[0]);
	m_maincpu->pp_in_cb().set_ioport(m_in[1]);

	SPEAKER(config, "speaker").front_center();

	mcfg_svg_screen(config, 755, 1080);
}

ROM_START( brke23p2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "brke23p2.bin", 0x0000, 0x1000, CRC(8045fac4) SHA1(a36213309e6add31f31e4248f02f17de9914a5c1) ) // visual decap

	ROM_REGION( 160500, "screen", 0)
	ROM_LOAD( "brke23p2.svg", 0, 160500, CRC(9edf8aab) SHA1(f2ab907d23517612196648f1b5b0cb9b4a1ab3bd) )
ROM_END

} // anonymous namespace

// some other dieshots have 1996 on them, it is also possible the software is from Holtek
CONS( 1993, brke23p2, 0, 0, brke23p2, brke23p2, hh_ht11xx_state, empty_init, "E-Star", "Brick Game 96 in 1 (E-23 Plus Mark II)", MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND )
