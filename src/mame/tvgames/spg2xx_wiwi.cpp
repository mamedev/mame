// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

/* These all have some kind of startup check
   the Virtual Interactive 'Vi' probably also fits here

   The WiWi could also run NES games; SunPlus hardware was in the cartridge, it was a 'fake' system.

    ---

    some of these are just checking a external timer on a port, I think this is one of the SPG features
    so might need making more generic.
*/



#include "emu.h"
#include "spg2xx.h"


namespace {

class spg2xx_game_wiwi18_state : public spg2xx_game_state
{
public:
	spg2xx_game_wiwi18_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void init_wiwi18();

protected:

private:
	void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

class spg2xx_game_marc101_state : public spg2xx_game_state
{
public:
	spg2xx_game_marc101_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_prev_porta(0),
		m_prev_portb(0),
		m_toggle(false)
	{ }

	void marc101(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	virtual uint16_t porta_r();
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	TIMER_CALLBACK_MEMBER(toggle_tick);

	uint16_t m_prev_porta;
	uint16_t m_prev_portb;
	bool m_toggle;
	emu_timer *m_pulse_timer = nullptr;
};

class spg2xx_game_guitrbus_state : public spg2xx_game_marc101_state
{
public:
	spg2xx_game_guitrbus_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_marc101_state(mconfig, type, tag)
	{ }

	void guitrbus(machine_config &config);

protected:

	virtual uint16_t portb_r();
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

class spg2xx_game_ddmsup_state : public spg2xx_game_marc101_state
{
public:
	spg2xx_game_ddmsup_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_marc101_state(mconfig, type, tag)
	{ }

	void ddmsup(machine_config &config);

protected:

//  virtual uint16_t portb_r();
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
//  virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};


class spg2xx_game_marc250_state : public spg2xx_game_marc101_state
{
public:
	spg2xx_game_marc250_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_marc101_state(mconfig, type, tag),
		m_toggle2(false)
	{ }

	void init_m527();

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	virtual uint16_t porta_r() override;
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	TIMER_CALLBACK_MEMBER(toggle2_tick);

private:
	bool m_toggle2;
	emu_timer *m_pulse_timer2 = nullptr;
};

TIMER_CALLBACK_MEMBER(spg2xx_game_marc101_state::toggle_tick)
{
	m_toggle = !m_toggle;
}

TIMER_CALLBACK_MEMBER(spg2xx_game_marc250_state::toggle2_tick)
{
	m_toggle2 = !m_toggle2;
}


void spg2xx_game_marc101_state::machine_start()
{
	spg2xx_game_state::machine_start();
	m_pulse_timer = timer_alloc(FUNC(spg2xx_game_marc101_state::toggle_tick), this);
	m_pulse_timer->adjust(attotime::never);

}

void spg2xx_game_marc101_state::machine_reset()
{
	spg2xx_game_state::machine_reset();
	m_pulse_timer->adjust(attotime::never);
	m_prev_porta = 0;
	m_prev_portb = 0;
	m_toggle = false;
}

void spg2xx_game_marc250_state::machine_start()
{
	spg2xx_game_marc101_state::machine_start();
	m_pulse_timer2 = timer_alloc(FUNC(spg2xx_game_marc250_state::toggle2_tick), this);
	m_pulse_timer2->adjust(attotime::never);

	// hack, makes x-racer3 and some others more stable, TODO: find out what is really wrong
	m_maincpu->set_clock_scale(2.0000f);

}

void spg2xx_game_marc250_state::machine_reset()
{
	spg2xx_game_marc101_state::machine_reset();
	switch_bank(31);
	m_maincpu->reset();

	m_pulse_timer2->adjust(attotime::never);
	m_toggle2 = false;
}


void spg2xx_game_marc101_state::marc101(machine_config &config)
{
	spg2xx(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);

	m_maincpu->porta_in().set(FUNC(spg2xx_game_marc101_state::porta_r));
	m_maincpu->porta_out().set(FUNC(spg2xx_game_marc101_state::porta_w));
}

void spg2xx_game_guitrbus_state::guitrbus(machine_config &config)
{
	spg2xx(config);

	m_maincpu->portb_in().set(FUNC(spg2xx_game_guitrbus_state::portb_r));
	m_maincpu->porta_out().set(FUNC(spg2xx_game_guitrbus_state::porta_w));
	m_maincpu->portb_out().set(FUNC(spg2xx_game_guitrbus_state::portb_w));
}

void spg2xx_game_ddmsup_state::ddmsup(machine_config &config)
{
	spg2xx(config);

//  m_maincpu->portb_in().set(FUNC(spg2xx_game_ddmsup_state::portb_r));
	m_maincpu->porta_in().set(FUNC(spg2xx_game_ddmsup_state::porta_r));
	m_maincpu->porta_out().set(FUNC(spg2xx_game_ddmsup_state::porta_w));
//  m_maincpu->portb_out().set(FUNC(spg2xx_game_ddmsup_state::portb_w));
}



// are these Port A behaviors related to IO A Special mode on the SoC?
// the bits being tested do seem to be 'ExtClk2 / ExtClk1'

uint16_t spg2xx_game_marc101_state::porta_r()
{
	uint16_t ret = m_io_p1->read() &~ 0x2000;

	ret |= m_toggle ? 0x2000 : 0x0000;

	return ret;
}

uint16_t spg2xx_game_guitrbus_state::portb_r()
{
	uint16_t ret = m_io_p2->read() &~ 0x0008;

	ret |= m_toggle ? 0x0000 : 0x0008;

	return ret;
}

void spg2xx_game_guitrbus_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: porta_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	m_prev_porta = data;
}

void spg2xx_game_guitrbus_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portb_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	if ((data & 0x0002) != (m_prev_portb & 0x0002))
	{
		if ((data & 0x0002))
		{
			logerror("ext timer reset\n");
			m_pulse_timer->adjust(attotime::from_hz(32), 0, attotime::from_hz(32));
		}
	}

	m_prev_portb = data;
}




void spg2xx_game_marc101_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: porta_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	// see function at 053E19 (marc101)
	// it has several states
	// 00 - wait a while
	// 01 - set 0x0400 in port a high
	// 02 - clear 0x0400 in port a
	// 07 - measure number of times 0x2000 on port a changes, with min/max acceptable values and a timeout
	// ff - failure (causes blank screen / shutdown + inf loop)
	if ((data & 0x0400) != (m_prev_porta & 0x0400))
	{
		if (!(data & 0x0400))
		{
			//logerror("pulse / timer reset\n");
			m_pulse_timer->adjust(attotime::from_hz(32), 0, attotime::from_hz(32));
		}
	}

	m_prev_porta = data;
}

uint16_t spg2xx_game_marc250_state::porta_r()
{
	uint16_t ret = m_io_p1->read() &~ 0x6000;
	ret |= m_toggle ? 0x2000 : 0x0000;
	ret |= m_toggle2 ? 0x4000 : 0x0000;
//  printf("porta %04x\n", ret);

	return ret;
}


void spg2xx_game_marc250_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: porta_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	// see function at 8AB2 (marc250 menu)
	// it has several states
	// 00 - wait a while
	// 01 - clear 0x1000 in port a
	// 02 - wait a while
	// 05 - check rate of port a bit 0x4000

	// 06 - wait a while
	// 07 - clear 0x1000 in port a
	// 08 - wait a while
	// 09 - check rate of port a bit 0x2000

	// ff - failure (causes blank screen / shutdown + inf loop)

	// function is at 04D918 in x-racer 3 (values tweaked to pass)
	// but x-racer 3 also seems to need faster CPU or it randomly corrupts (or its an IRQ problem?)

	if ((data & 0x1000) != (m_prev_porta & 0x1000))
	{
		if (!(data & 0x1000))
		{
			// these values stop the marc250 menu from failing, but not the games
			//printf("pulse / timer reset\n");
			m_pulse_timer->adjust(attotime::from_hz(6), 0, attotime::from_hz(6));
			m_pulse_timer2->adjust(attotime::from_hz(12), 0, attotime::from_hz(12));
		}
	}

	m_prev_porta = data;
}

void spg2xx_game_ddmsup_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: porta_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	if (data & 0x4000)
	{
		m_pulse_timer->adjust(attotime::from_hz(6), 0, attotime::from_hz(6));
	}

	m_prev_porta = data;
}


static INPUT_PORTS_START( wiwi18 )
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Possible Input" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( lexifit )
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_CUSTOM )
INPUT_PORTS_END

static INPUT_PORTS_START( m489 )
	PORT_START("P1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) // acts as Button 1 autofire (not connected on unit?)
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( guitrbus )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Red / Do")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Yellow / Re")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Purple / Mi")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Green / Fa")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Menu")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Blue / Sol")

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Logo (either WinFun or no logo)

	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Whammy Bar")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Strum")

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END




static INPUT_PORTS_START( ddmsup )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("P1 Pad Up") PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("P1 Pad Left") PORT_16WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P1 Pad Right") PORT_16WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("P1 Pad Down") PORT_16WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Back")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Select")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Pad Up-Left")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Pad Up-Right")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Pad Down-Left")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Pad Down-Right")
	PORT_BIT( 0x1c00, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED ) // must toggle at rate measured by code for ddmsup
	PORT_BIT( 0xc000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("P2 Pad Up") PORT_PLAYER(2) PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("P2 Pad Left") PORT_PLAYER(2) PORT_16WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P2 Pad Right") PORT_PLAYER(2) PORT_16WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("P2 Pad Down") PORT_PLAYER(2) PORT_16WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Pad Up-Left") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Pad Up-Right") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Pad Down-Left") PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Pad Down-Right") PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void spg2xx_game_wiwi18_state::init_wiwi18()
{
	// workaround for security checks on startup

	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	if (rom[0x1ca259]==0x4e04) rom[0x1ca259] = 0xf165; // wiwi18
	if (rom[0x1355a4]==0x4e04) rom[0x1355a4] = 0xf165; // foxsport

	if (rom[0x362e1c]==0x4e04) rom[0x362e1c] = 0xf165; // lexifit
	if (rom[0x4c7f4d]==0x4e04) rom[0x4c7f4d] = 0xf165; // lexifit (2nd bank)

}

void spg2xx_game_wiwi18_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portb_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
		(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
		(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
		(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
		(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
		(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
		(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
		(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
		(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
		(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
		(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
		(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
		(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
		(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
		(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
		(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
		(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');

	if (m_maincpu->pc() < 0x2000)
	{
		if ((data & 0x0003) == 0x0000)
			switch_bank(1);
		else
			switch_bank(0);
	}
}



void spg2xx_game_marc101_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_maincpu->pc() < 0x2000)
	{
		logerror("%s: portb_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
			(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
			(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
			(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
			(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
			(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
			(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
			(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
			(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
			(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
			(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
			(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
			(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
			(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
			(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
			(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
			(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');
	}


	if (m_maincpu->pc() < 0x2000)
	{
		// bit 0x1000 isn't set as an output, but clearly needs to be treated as one
		switch (data & 0x1003)
		{
		case 0x0000: switch_bank(7); break;
		case 0x0001: switch_bank(6); break; // good - heroic pilot
		case 0x0002: switch_bank(5); break; // good - horrific collapser
		case 0x0003: switch_bank(4); break; // good - under sea war
		case 0x1000: switch_bank(3); break; // good - roadman etc.
		case 0x1001: switch_bank(2); break; // good - santa etc
		case 0x1002: switch_bank(1); break;
		case 0x1003: switch_bank(0); break;
		}
	}
}

void spg2xx_game_marc250_state::init_m527()
{

	// the code being patched turns the IRQ off, code looks a bit like smarttv code..  is SoC IRQ handling wrong?
	// or is this more security, or something else? games don't run with the IRQ off...
#if 1
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();

	uint16_t ident2[6] = { 0x9512, 0x2862, 0xa70a, 0x0002, 0xd71b, 0x2862 };

	for (int i = 0; i < (0x10000000 / 2) - 6; i++)
	{
		bool found = true;
		for (int j = 0; j < 6; j++)
		{
			uint16_t cmp1 = ident2[j];
			uint16_t cmp2 = rom[i + j];

			if (cmp1 != cmp2)
				found = false;
		}

		if (found)
		{
			printf("found at %08x\n", i + 3);
			rom[i + 3] = 0x0003;
		}
	}


	// pass maze road
	//rom[((12 * 0x800000) / 2) | 0x0284b5] = 0x0003;
	// learn numbers
	//rom[((13 * 0x800000) / 2) | 0x00c055] = 0x0003;
	// bowling
	//rom[((17 * 0x800000) / 2) | 0x015e58] = 0x0003;
	// cliff overhang / gym dancing
	//rom[((18 * 0x800000) / 2) | 0x01cab4] = 0x0003;
	rom[((18 * 0x800000) / 2) | 0x021e25] = 0xffff; // secondary 'turn off'
	// jump chess
	//rom[((19 * 0x800000) / 2) | 0x012c3a] = 0x0003;
	// boxing, basketball etc.
	//rom[((23 * 0x800000) / 2) | 0x00c1bd] = 0x0003;
	// dash motor
	//rom[((24 * 0x800000) / 2) | 0x00d62c] = 0x0003;
	// same for curling
	//rom[((25 * 0x800000) / 2) | 0x0103c5] = 0x0003;
	// balloon puyo
	//rom[((26 * 0x800000) / 2) | 0x0084dd] = 0x0003;
	// 4 score
	//rom[((27 * 0x800000) / 2) | 0x145cdd] = 0x0003;
#endif
}

void spg2xx_game_marc250_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_maincpu->pc() < 0x2000)
	{
		logerror("%s: portb_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c  \n", machine().describe_context(), data, mem_mask,
			(mem_mask & 0x8000) ? ((data & 0x8000) ? '1' : '0') : 'x',
			(mem_mask & 0x4000) ? ((data & 0x4000) ? '1' : '0') : 'x',
			(mem_mask & 0x2000) ? ((data & 0x2000) ? '1' : '0') : 'x',
			(mem_mask & 0x1000) ? ((data & 0x1000) ? '1' : '0') : 'x',
			(mem_mask & 0x0800) ? ((data & 0x0800) ? '1' : '0') : 'x',
			(mem_mask & 0x0400) ? ((data & 0x0400) ? '1' : '0') : 'x',
			(mem_mask & 0x0200) ? ((data & 0x0200) ? '1' : '0') : 'x',
			(mem_mask & 0x0100) ? ((data & 0x0100) ? '1' : '0') : 'x',
			(mem_mask & 0x0080) ? ((data & 0x0080) ? '1' : '0') : 'x',
			(mem_mask & 0x0040) ? ((data & 0x0040) ? '1' : '0') : 'x',
			(mem_mask & 0x0020) ? ((data & 0x0020) ? '1' : '0') : 'x',
			(mem_mask & 0x0010) ? ((data & 0x0010) ? '1' : '0') : 'x',
			(mem_mask & 0x0008) ? ((data & 0x0008) ? '1' : '0') : 'x',
			(mem_mask & 0x0004) ? ((data & 0x0004) ? '1' : '0') : 'x',
			(mem_mask & 0x0002) ? ((data & 0x0002) ? '1' : '0') : 'x',
			(mem_mask & 0x0001) ? ((data & 0x0001) ? '1' : '0') : 'x');
	}

	// bank 0  = (unused) 'ROM 18 64M' (dupe)
	// bank 1  = (unused) 'ROM 18 64M' (dupe)
	// bank 2  = (unused) 'ROM 18 64M' (dupe)
	// bank 3  = (unused) 'ROM 18 64M' (dupe)
	// bank 4  = (unused) 'ROM 18 64M' (dupe)
	// bank 5  = (unused) 'ROM 18 64M' (dupe)
	// bank 6  = (used) boots 'kung fu high style' (no bank number check)
	// bank 7  = (unused) 'ROM 18 64M' (dupe)

	// bank 8  = (used) 'ROM 24 64M'
	// bank 9  = (used) 'ROM 23 64M'
	// bank 10 = (unused) 'ROM 18 64M' (dupe)
	// bank 11 = (unused) 'ROM 18 64M' (dupe)
	// bank 12 = (used) (doesn't boot)
	// bank 13 = (used) (doesn't boot)
	// bank 14 = (used) 'ROM 18 64M'
	// bank 15 = (unused) 'ROM 18 64M' (dupe)

	// bank 16 = (used) 'ROM 16 64M' (error)
	// bank 17 = (used) (plays music)
	// bank 18 = (used) (doesn't boot)
	// bank 19 = (used)
	// bank 20 = (used)
	// bank 21 = (used)
	// bank 22 = (used)
	// bank 23 = (used)

	// bank 24 = (used)
	// bank 25 = (used)
	// bank 26 = (used)
	// bank 27 = (used)
	// bank 28 = (used)
	// bank 29 = (used)
	// bank 30 = (used) 'ROM 2 64M'
	// bank 31 = (used) menu (no bank number check)

	// bits 0x1804 aren't set as an output, but clearly need to be treated as output, also bad values of 0x00ff and 0x0000 are written on startup, with direction bits set!
	// this seems similar to issues with banking in some other units.
	if ((m_maincpu->pc() < 0x2000) && (data != 0x00ff) && (data != 0x0000))
	{
		switch (data & 0x1807)
		{
		case 0x0000: switch_bank(0); break; // unused
		case 0x0001: switch_bank(1); break; // unused
		case 0x0002: switch_bank(2); break; // unused
		case 0x0003: switch_bank(3); break; // unused

		case 0x1000: switch_bank(4); break; // unused
		case 0x1001: switch_bank(5); break; // unused
		case 0x1002: switch_bank(6); break; // kung fu (ok)
		case 0x1003: switch_bank(7); break; // unused

		case 0x0800: switch_bank(8); break; // jogging (ok)
		case 0x0801: switch_bank(9); break; // power kick (ok)
		case 0x0802: switch_bank(10); break; // unused
		case 0x0803: switch_bank(11); break; // unused

		case 0x1800: switch_bank(12); break; // pass maze road
		case 0x1801: switch_bank(13); break; // learn number
		case 0x1802: switch_bank(14); break; // final race, motor race
		case 0x1803: switch_bank(15); break; // unused

		case 0x0004: switch_bank(16); break; // thunder race, speed race, night shooter
		case 0x0005: switch_bank(17); break; // rescue bees
		case 0x0006: switch_bank(18); break; // cliff overhang
		case 0x0007: switch_bank(19); break; // jump chess

		case 0x1004: switch_bank(20); break; // mind link, mystery cave
		case 0x1005: switch_bank(21); break; // bumper cars, stone age, dance party (need hack)
		case 0x1006: switch_bank(22); break; // xracer (need hack)
		case 0x1007: switch_bank(23); break; // basketball (but irq gets turned off?)

		case 0x0804: switch_bank(24); break; // button jumper  dash motor
		case 0x0805: switch_bank(25); break; // cross river
		case 0x0806: switch_bank(26); break; // balloon puyu
		case 0x0807: switch_bank(27); break; // 4 Score

		case 0x1804: switch_bank(28); break; // bubble, malibu
		case 0x1805: switch_bank(29); break; // goalkeeper
		case 0x1806: switch_bank(30); break; // fish journey, brick, happy zoo, crazy tramcar
		case 0x1807: switch_bank(31); break; // menu

		}
	}
}

ROM_START( foxsport )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "foxsports.bin", 0x000000, 0x1000000, CRC(a5092f49) SHA1(feb4d432486b17d6cd2aed8cefd3d084f77c1733) ) // only a tiny amount of data in the 2nd half, what's it used for (if anything)
ROM_END

ROM_START( wiwi18 )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "26gl128.bin", 0x000000, 0x1000000, CRC(0b103ac9) SHA1(14434908f429942096fb8db5b5630603fd54fb2c) )
ROM_END

ROM_START( lexifit )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "lexibook_tv_fitness_center.bin", 0x000000, 0x1000000, CRC(38021230) SHA1(2b949d723a475bfac23d9da4d1a30ea71b332ccb) )
ROM_END

ROM_START( marc101 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "m489.u6", 0x0000000, 0x4000000, CRC(0a01695f) SHA1(1a13c5eb9dffdc91fc68a98e8f35bd8a019a8373) )
	ROM_IGNORE(0x4000000) // 2nd half is just random fill unrelated to the game, ROM was twice needed capacity
ROM_END

ROM_START( marc250 )
	ROM_REGION( 0x10000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "m527.u6", 0x0000000, 0x10000000, CRC(4b856cab) SHA1(41c66bbdb0bb1442d7e11da18e9e6b20048445ba) )
ROM_END

ROM_START( guitrbus )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "winfunguitar.bin", 0x000000, 0x400000, CRC(17419a27) SHA1(19377fcd18b08d3ae8e20de0244b3aaef1b5a66a) )
ROM_END

ROM_START( ddmsup )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "dancemaniasupreme.bin", 0x000000, 0x800000, CRC(4066a36b) SHA1(1fc8553b03b81bc02c3fcbcdafa2e54d1f2c2306) )
ROM_END

ROM_START( ddmmeg12 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "doubledancemania.bin", 0x000000, 0x800000, CRC(fcffb21e) SHA1(802a3c2d6bcbd729b9a483d42d912ca8b9abdccd) )
ROM_END

} // anonymous namespace


// box marked 'Wireless game console' 'Drahtlose Spielekonsole' 87 Sports games included : 18 hyper sports games, 69 arcade games.
// Unit marked 'Hamy System' 'WiWi'
// actually a cartridge, but all hardware is in the cart, overriding any internal hardware entirely.  see nes_vt.cp 'mc_sp69' for the '69 arcade game' part
CONS( 200?, wiwi18,   0,        0, rad_skat, wiwi18,  spg2xx_game_wiwi18_state, init_wiwi18, "Hamy System",           "WiWi 18-in-1 Sports Game", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2009, lexifit,  0,        0, rad_skat, lexifit, spg2xx_game_wiwi18_state, init_wiwi18, "Lexibook",              "TV Fitness Center (Lexibook)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, foxsport, 0,        0, rad_skat, wiwi18,  spg2xx_game_wiwi18_state, init_wiwi18, "Excalibur Electronics", "Fox Sports 7 in 1 Sports Games Plug n' Play", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// thtere is another 'Drahtlose Spielekonsole 48-in-1' with '11 hyper sports games' (including Running) which are clearly SunPlus and would fit here, with the 37 non-hyper sports games presumably again being a NES/Famiclone cart

CONS( 2014, marc101,     0,        0, marc101, m489,  spg2xx_game_marc101_state, empty_init, "Millennium 2000 GmbH", "Millennium Arcade 101 (M489) (Game Station 2 101-in-1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, ddmsup,      0,        0, ddmsup, ddmsup,  spg2xx_game_ddmsup_state, empty_init,    "Senario", "Double Dance Mania: Supreme / Dance Supreme", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // titlescreen is just 'Dance Supreme'

CONS( 2007, ddmmeg12,    0,        0, ddmsup, ddmsup,  spg2xx_game_ddmsup_state,  empty_init,    "Senario",  "Double Dance Mania: Mega 12",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2015, marc250,     0,        0, marc101, m489,  spg2xx_game_marc250_state, init_m527, "Millennium 2000 GmbH", "Millennium Arcade 250 (M527)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// has the following strings at the start of the ROM
// "Copyright(C) 2009-2012 ShenZhen Multi-Content Software CO., LTD"
// "LisencedTo: MCS"
CONS( 2012, guitrbus,   0,        0, guitrbus,  guitrbus,  spg2xx_game_guitrbus_state,          empty_init,    "WinFun",  "Guitar Buster", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
