// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

#include "emu.h"
#include "spg2xx.h"

#include "pvmil.lh"


namespace {

class pvmil_state : public spg2xx_game_state
{
public:
	pvmil_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_portcdata(0x0000),
		m_latchcount(0),
		m_latchbit(0),
		m_outdat(0),
		m_p4inputs(*this, "EXTRA"),
		m_leds(*this, "led%u", 0U)
	{ }

	void pvmil(machine_config &config);

	int pvmil_p4buttons_r();

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

private:
	uint16_t m_portcdata;
	int m_latchcount;
	int m_latchbit;
	uint16_t m_outdat;
	optional_ioport m_p4inputs;
	output_finder<4> m_leds;
};


void pvmil_state::machine_start()
{
	spg2xx_game_state::machine_start();

	m_leds.resolve();
	save_item(NAME(m_portcdata));
	save_item(NAME(m_latchcount));
	save_item(NAME(m_latchbit));
	save_item(NAME(m_outdat));
}


void pvmil_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: pvmil_porta_w %04x\n", machine().describe_context(), data);
}

void pvmil_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: pvmil_portb_w %04x\n", machine().describe_context(), data);
}


int pvmil_state::pvmil_p4buttons_r()
{
	return m_latchbit;
}


void pvmil_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// ---- -432 1--- r-?c
	// 4,3,2,1 = player controller LEDs
	// r = reset input multiplexer
	// ? = unknown
	// m = input multiplexer clock

	// p4 input reading
	// the code to read them is interesting tho, it even includes loops that poll port a 16 times before/after, why?
	logerror("%s: pvmil_portc_w %04x\n", machine().describe_context(), data);

	uint16_t bit;

	// for logging bits changed on the port
	if (0)
	{
		for (int i = 0; i < 16; i++)
		{
			bit = 1 << i;
			if ((m_portcdata & bit) != (data & bit))
			{
				if (data & bit)
				{
					logerror("port c %04x low to high\n", bit);
				}
				else
				{
					logerror("port c %04x high to low\n", bit);
				}
			}

			if ((m_portcdata & 0x0400) != (data & 0x0400))
			{
				logerror("-------------------------------------------------\n");
			}
		}
	}

	// happens on startup, before it starts reading inputs for the first time, assume 'reset counter'
	bit = 0x0008;
	if ((m_portcdata & bit) != (data & bit))
	{
		if (data & bit)
		{
			logerror("reset read counter\n");
			m_latchcount = 0;
		}
	}

	bit = 0x0001;
	if ((m_portcdata & bit) != (data & bit))
	{
		if (!(data & bit))
		{
			//logerror("latch with count of %d (outbit is %d)\n", m_latchcount, (m_portcdata & 0x0002)>>1 );
			// what is bit 0x0002? it gets flipped in the same code as the inputs are read.
			// it doesn't follow any obvious pattern
			m_outdat &= ~(1 << m_latchcount);
			m_outdat |= ((data & 0x0002) >> 1) << m_latchcount;
			if (0)
				popmessage("%d %d %d %d   %d %d %d %d   %d %d %d %d   %d %d %d %d",
					(m_outdat & 0x8000) ? 1 : 0, (m_outdat & 0x4000) ? 1 : 0, (m_outdat & 0x2000) ? 1 : 0, (m_outdat & 0x1000) ? 1 : 0,
					(m_outdat & 0x0800) ? 1 : 0, (m_outdat & 0x0400) ? 1 : 0, (m_outdat & 0x0200) ? 1 : 0, (m_outdat & 0x0100) ? 1 : 0,
					(m_outdat & 0x0080) ? 1 : 0, (m_outdat & 0x0040) ? 1 : 0, (m_outdat & 0x0020) ? 1 : 0, (m_outdat & 0x0010) ? 1 : 0,
					(m_outdat & 0x0008) ? 1 : 0, (m_outdat & 0x0004) ? 1 : 0, (m_outdat & 0x0002) ? 1 : 0, (m_outdat & 0x0001) ? 1 : 0);


			m_latchbit = (((m_p4inputs->read()) << m_latchcount) & 0x8000) ? 1 : 0;

			m_latchcount++;
			if (m_latchcount == 16)
				m_latchcount = 0;
		}
	}

	m_portcdata = data;

	if (0)
		popmessage("%d %d %d %d   %d %d %d %d   %d %d %d %d   %d %d %d %d",
			(m_portcdata & 0x8000) ? 1 : 0, (m_portcdata & 0x4000) ? 1 : 0, (m_portcdata & 0x2000) ? 1 : 0, (m_portcdata & 0x1000) ? 1 : 0,
			(m_portcdata & 0x0800) ? 1 : 0, (m_portcdata & 0x0400) ? 1 : 0, (m_portcdata & 0x0200) ? 1 : 0, (m_portcdata & 0x0100) ? 1 : 0,
			(m_portcdata & 0x0080) ? 1 : 0, (m_portcdata & 0x0040) ? 1 : 0, (m_portcdata & 0x0020) ? 1 : 0, (m_portcdata & 0x0010) ? 1 : 0,
			(m_portcdata & 0x0008) ? 1 : 0, (m_portcdata & 0x0004) ? 1 : 0, (m_portcdata & 0x0002) ? 1 : 0, (m_portcdata & 0x0001) ? 1 : 0);

	m_leds[0] = (m_portcdata & 0x0080) ? 0 : 1;
	m_leds[1] = (m_portcdata & 0x0100) ? 0 : 1;
	m_leds[2] = (m_portcdata & 0x0200) ? 0 : 1;
	m_leds[3] = (m_portcdata & 0x0400) ? 0 : 1;
}


static INPUT_PORTS_START( pvmil ) // hold "console start" + "console select" on boot for test mode
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Player 1 A")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Player 1 B")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Player 1 C")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Player 1 D")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Player 2 A")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Player 2 B")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Player 2 C")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("Player 2 D")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("Player 3 A")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("Player 3 B")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("Player 3 C")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3) PORT_NAME("Player 3 D")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Player 1 Lifeline")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_START ) PORT_CODE(KEYCODE_1) PORT_NAME("Console Start")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CODE(KEYCODE_5) PORT_NAME("Console Select")

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(pvmil_state, pvmil_p4buttons_r) // Player 4 buttons read through here
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("Player 2 Lifeline")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(3) PORT_NAME("Player 3 Lifeline")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(4) PORT_NAME("Player 4 Lifeline")
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("Player 4 A")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("Player 4 B")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("Player 4 C")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("Player 4 D")
INPUT_PORTS_END


void pvmil_state::pvmil(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &pvmil_state::mem_map_4m);
	m_maincpu->set_pal(true);

	spg2xx_base(config);

	m_screen->set_refresh_hz(50);
	//m_screen->set_size(320, 312);

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->porta_out().set(FUNC(pvmil_state::porta_w));
	m_maincpu->portb_out().set(FUNC(pvmil_state::portb_w));
	m_maincpu->portc_out().set(FUNC(pvmil_state::portc_w));

	config.set_default_layout(layout_pvmil);
}

ROM_START( pvmil )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) // Fujitsu 29Z0002TN, read as ST M29W320FB
	ROM_LOAD16_WORD_SWAP( "millionare4.bin", 0x000000, 0x400000, CRC(9c43d0f2) SHA1(fb4ba0115000b10b7c0e0d44b9fa3234c900e694) )
ROM_END

} // anonymous namespace


// see note for the pvmil8 set in tvgames/elan_eu3a05.cpp
CONS( 2006, pvmil,       0,     0,        pvmil,        pvmil,    pvmil_state, empty_init, "Play Vision", "Who Wants to Be a Millionaire? (Play Vision, Plug and Play, UK, 16-bit version)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
