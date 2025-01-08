// license:BSD-3-Clause
// copyright-holders: Robbbert
// PINBALL

// Skeleton driver for Mirco's Lucky Draw
// Hardware listing and ROM definitions from PinMAME.
/***************************************************************************************************
Made mostly working on 2021-10-12 [Robbbert]

The hardware is capable of 64 solenoids, 64 lamps, 64 contactors (switches) and 32 dips,
however this game only uses a small fraction of that.

To play, either set game to free play, or insert a coin. Press start. Press letters A thru S to
score. Press X to lose the ball (the outhole).

Status:
- Game is playable

ToDo:
- Test button doesn't work
- DIP labels are incomplete
- If the match digit matches, you don't get a reward.
- Schematic doesn't show interconnections between boards, so had to guess a few things. Also
  there's no operations manual, so don't know if there's a special procedure to activate the tests.

***************************************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/mcs48/mcs48.h"
#include "lckydraw.lh"


namespace {

class lckydraw_state : public genpin_class
{
public:
	lckydraw_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_dips(*this, "DSW%d", 0U)
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		{ }

	void lckydraw(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void p1_w(u8);
	void p2_w(u8);
	u8 bus_r(offs_t);
	void bus_w(offs_t,u8);
	u8 m_bank_sw = 0U;
	u8 m_ram[256]{};
	u8 m_p2_out[16]{};
	u8 m_segment[3]{};
	u8 m_p1 = 0U;
	u8 m_p2 = 0U;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<i8035_device> m_maincpu;
	required_ioport_array<4> m_io_dips;
	required_ioport_array<8> m_io_keyboard;
	output_finder<48> m_digits;
	output_finder<48> m_io_outputs;  // 16 solenoids + 32 lamps
};

void lckydraw_state::mem_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
}

void lckydraw_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(lckydraw_state::bus_r),FUNC(lckydraw_state::bus_w));
}

void lckydraw_state::p1_w(u8 data)
{
	m_p1 = data;
}

void lckydraw_state::p2_w(u8 data)
{
	m_p2 = data;
	m_bank_sw = BIT(data, 4, 4);
}

u8 lckydraw_state::bus_r(offs_t offset)
{
	u8 data = m_ram[offset];
	switch(m_bank_sw)
	{
		case 0:
			{
				data = 0xff;
				for (u8 i = 0; i < 4; i++)
					if (!BIT(m_p1, i))
						data &= m_io_keyboard[i]->read();
			}
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			data = m_io_dips[m_bank_sw & 3]->read();
			break;
		default:
			break;
	}
	return data;
}

void lckydraw_state::bus_w(offs_t offset, u8 data)
{
	m_p2_out[m_bank_sw] = data;
	switch(m_bank_sw)
	{
		case 0:
		case 14:
			m_ram[offset] = data;
			break;
		case 2:
			// solenoids
			{
				for (u8 i = 0; i < 2; i++)
					if (BIT(m_p2_out[1], i))
						for (u8 j = 0; j < 8; j++)
							m_io_outputs[i*8+j] = BIT(data, j);
				for (u8 i = 0; i < 16; i++)
				{
					bool state = m_io_outputs[i];
					switch (i)
					{
						case 0:
							if (state)
								m_samples->start(0, 9);  // outhole
							break;
						case 1:
							if (state)
								m_samples->start(0, 10);  // kicker
							break;
						case 2:
						case 3:
							if (state)
								m_samples->start(0, 0);  // bumpers
							break;
						case 8:
							if (state)
								m_samples->start(0, 6);  // knocker
							break;
						case 9:
							if (state)
								m_samples->start(1, 1);  // 1000 chime
							break;
						case 10:
							if (state)
								m_samples->start(2, 2);  // 100 chime
							break;
						case 11:
							if (state)
								m_samples->start(3, 3);  // 10 chime
							break;
						case 14:
							machine().bookkeeping().coin_counter_w(0, state);
							break;
						default:
							break;
					}
				}
			}
			break;
		case 8:
		case 9:
		case 10:
			// display
			m_segment[m_bank_sw & 3] = data;
			break;
		case 13:
			// Lamps
			{
				for (u8 i = 0; i < 4; i++)
					if (BIT(m_p2_out[12], i))
						for (u8 j = 0; j < 8; j++)
							m_io_outputs[16+i*8+j] = BIT(data, j);
			}
			// Displays
			{
				u8 i,n = 7;
				for (i = 0; i < 6; i++)
					if (BIT(data, i))
						n = i;
				static const uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
				for (i = 0; i < 3; i++)
					m_digits[i*20+n] = patterns[BIT(m_segment[i],0,4)];
				for (i = 0; i < 2; i++)
					m_digits[i*20+10+n] = patterns[BIT(m_segment[i],4,4)];
			}
			break;
		default:
			break;
	}
}


static INPUT_PORTS_START( lckydraw )
	// Game Switches (== settings)
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "GS01")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "GS05")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "GS09")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x08, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "Balls")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x10, "5")
	PORT_DIPNAME( 0x20, 0x20, "GS21")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x40, "GS25")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "GS29")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "GS02")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "GS06")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "GS10")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x08, "GS14")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "Startup Tune")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ))
	PORT_DIPNAME( 0x20, 0x20, "Attract-mode Lamps")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ))
	PORT_DIPNAME( 0x40, 0x40, "GS26")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "GS30")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "GS03")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "GS07")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "GS11")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x08, "GS15")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "GS19")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "GS23")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x40, "GS27")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "GS31")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Free Play")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "GS08")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "GS12")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x08, "GS16")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "GS20")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "GS24")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x40, "GS28")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "GS32")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))

	// Test button
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Test") PORT_CODE(KEYCODE_0_PAD)

	// Contactors
	PORT_START("X0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("KD")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("KS")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("KH")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("KC")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Exit L")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Exit R")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Spinner")

	PORT_START("X1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("QD")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("QC")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("QH")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("10SIDE")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("100 POP R")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("100 POP L")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Kicker")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("AD")

	PORT_START("X2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("JD")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("JS")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("10D")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("X3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Plumb Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Mercury Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper")

	PORT_START("X4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void lckydraw_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_bank_sw));
	save_item(NAME(m_ram));
	save_item(NAME(m_p2_out));
	save_item(NAME(m_segment));
}

void lckydraw_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}

void lckydraw_state::lckydraw(machine_config &config)
{
	/* basic machine hardware */
	I8035(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &lckydraw_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lckydraw_state::io_map);
	m_maincpu->p1_out_cb().set(FUNC(lckydraw_state::p1_w));
	m_maincpu->p2_out_cb().set(FUNC(lckydraw_state::p2_w));
	m_maincpu->p1_in_cb().set([this]() { return m_p1; });
	m_maincpu->p2_in_cb().set([this]() { return m_p2; });
	m_maincpu->t0_in_cb().set([this]() { return ioport("TEST")->read() ? 1: 0; });

	/* video hardware */
	config.set_default_layout(layout_lckydraw);

	/* sound hardware */
	genpin_audio(config);
}


ROM_START(lckydraw)
	ROM_REGION( 0xc00, "maincpu", 0)
	ROM_LOAD( "lckydrw1.rom", 0x0000, 0x0400, CRC(58ebb50f) SHA1(016ed66b4ee9979aa109c0ce085597a62d33bf8d) )
	ROM_LOAD( "lckydrw2.rom", 0x0400, 0x0400, CRC(816b9e20) SHA1(0dd8acc633336f250960ebe89cc707fd115afeee) )
	ROM_LOAD( "lckydrw3.rom", 0x0800, 0x0400, CRC(464155bb) SHA1(5bbf784dba9149575444e6b1250ac9b5c2bced87) )
ROM_END

ROM_START(lckydrawa)  // stronger flippers
	ROM_REGION( 0xc00, "maincpu", 0)
	ROM_LOAD( "lckydrw1.rom", 0x0000, 0x0400, CRC(58ebb50f) SHA1(016ed66b4ee9979aa109c0ce085597a62d33bf8d) )
	ROM_LOAD( "lckydrw2.rom", 0x0400, 0x0400, CRC(816b9e20) SHA1(0dd8acc633336f250960ebe89cc707fd115afeee) )
	ROM_LOAD( "lckydrw3b.rom",0x0800, 0x0400, CRC(810de93e) SHA1(0740241f437657f04fbc103a90d25ff6b6db0af5) )
ROM_END

} // Anonymous namespace


GAME( 1979, lckydraw,  0,        lckydraw, lckydraw, lckydraw_state, empty_init, ROT0, "Mirco", "Lucky Draw (pinball, set 1)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1979, lckydrawa, lckydraw, lckydraw, lckydraw, lckydraw_state, empty_init, ROT0, "Mirco", "Lucky Draw (pinball, set 2)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
