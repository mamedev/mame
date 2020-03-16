// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

/* 'Zone' '32-bit' systems */

#include "emu.h"
#include "includes/spg2xx.h"


class zon32bit_state : public spg2xx_game_state
{
public:
	zon32bit_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_romregion(*this, "maincpu")
	{ }

	void zon32bit(machine_config& config);

	void mem_map_zon32bit(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_post_load() override;

	DECLARE_READ16_MEMBER(z32_rom_r);

	required_region_ptr<uint16_t> m_romregion;

	virtual DECLARE_READ16_MEMBER(porta_r);
	virtual DECLARE_READ16_MEMBER(portb_r);
	virtual DECLARE_READ16_MEMBER(portc_r);

	virtual DECLARE_WRITE16_MEMBER(porta_w) override;
	virtual DECLARE_WRITE16_MEMBER(portb_w) override;
	virtual DECLARE_WRITE16_MEMBER(portc_w) override;

	int m_porta_dat;
	int m_portb_dat;
	int m_portc_dat;
	int m_porta_mask;

	int m_upperbank;

	int m_basebank;
};

class mywicodx_state : public zon32bit_state
{
public:
	mywicodx_state(const machine_config& mconfig, device_type type, const char* tag) :
		zon32bit_state(mconfig, type, tag)
	{ }

protected:
	virtual DECLARE_WRITE16_MEMBER(porta_w) override;

	virtual void machine_reset() override;
};

class oplayer_100in1_state : public mywicodx_state
{
public:
	oplayer_100in1_state(const machine_config& mconfig, device_type type, const char* tag) :
		mywicodx_state(mconfig, type, tag)
	{ }

	void init_oplayer();


protected:
	virtual DECLARE_READ16_MEMBER(porta_r) override;
	virtual DECLARE_READ16_MEMBER(portb_r) override;
	virtual DECLARE_READ16_MEMBER(portc_r) override;

};


void zon32bit_state::device_post_load()
{
	// load state can change the bank, so we must invalide cache
	m_maincpu->invalidate_cache();
}

READ16_MEMBER(zon32bit_state::porta_r)
{
	return m_porta_dat;
}


WRITE16_MEMBER(zon32bit_state::porta_w)
{
	if (0)
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

	m_porta_dat = data;

	// The banking on zon32bit doesn't seem the same as mywicodx.
	// The same values get written here both in the case of switching to upper bank and switching to lower bank, so presumably it must be some kind of toggle
	if (data == 0x0e01)
	{
		m_basebank ^= 1;
		logerror("bank is now %d\n", m_basebank);
		m_maincpu->invalidate_cache();
	}
}


WRITE16_MEMBER(mywicodx_state::porta_w)
{
	if (0)
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

	m_porta_dat = data;

	int oldbank = m_basebank;

	if (mem_mask & 0x0400)
	{
		if (data & 0x0400)
			m_basebank |= 1;
		else
			m_basebank &= ~1;
	}

	if (mem_mask & 0x0800)
	{
		if (data & 0x0800)
			m_basebank |= 2;
		else
			m_basebank &= ~2;
	}

	if (oldbank != m_basebank)
		m_maincpu->invalidate_cache();
}


READ16_MEMBER(zon32bit_state::portc_r)
{
	// 0x03ff seem to be inputs for buttons (and some kind of output?)
	// 0xfc00 gets masked for other reasons (including banking?)

	// returning same value written for 0x0400 means controls don't respond (some kind of direction flag?)

	uint16_t dat = m_io_p3->read() & ~0xf800;

	dat |= (m_portc_dat & 0xf800);

	return dat;
}


READ16_MEMBER(zon32bit_state::portb_r)
{
	return m_portb_dat;
}

WRITE16_MEMBER(zon32bit_state::portb_w)
{
	if (data != 0x0001)
		logerror("%s: portb_w %04x (%04x)\n", machine().describe_context(), data, mem_mask);

	m_portb_dat = data;
}

WRITE16_MEMBER(zon32bit_state::portc_w)
{
	// very noisy
	// is the code actually sending the sound to the remotes?
	if (0)
		logerror("%s: portc_w %04x (%04x) %c %c %c %c | %c %c %c %c | %c %c %c %c | %c %c %c %c\n", machine().describe_context(), data, mem_mask,
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

	int oldbank = m_upperbank;

	if (mem_mask & 0x1000)
	{
		if (data & 0x1000)
			m_upperbank |= 0x1000;
		else
			m_upperbank &= ~0x1000;
	}

	if (mem_mask & 0x0800)
	{
		if (data & 0x0800)
			m_upperbank |= 0x0800;
		else
			m_upperbank &= ~0x0800;
	}
		 
	if (oldbank != m_basebank)
		m_maincpu->invalidate_cache();

	m_portc_dat = data;
}


READ16_MEMBER(oplayer_100in1_state::portc_r)
{
	return m_io_p3->read();
}

READ16_MEMBER(oplayer_100in1_state::portb_r)
{
	return m_io_p2->read();
}

READ16_MEMBER(oplayer_100in1_state::porta_r)
{
	return 0x0ff8 | (machine().rand()&1);
}

void zon32bit_state::mem_map_zon32bit(address_map &map)
{
	map(0x000000, 0x3fffff).r(FUNC(zon32bit_state::z32_rom_r));
}

READ16_MEMBER(zon32bit_state::z32_rom_r)
{
	/*
		This has upper and lower bank, which can be changed independently.
		Banking hookup is currently very hacky as bank values are written
		to ports then erased at the moment, maybe they latch somehow?
	*/

	int base = 0x0000000;

	if (m_basebank & 2)  base |= 0x2000000;
	if (m_basebank & 1)  base |= 0x1000000;

	if (offset < 0x200000)
	{
		return m_romregion[offset + (base / 2)];
	}
	else
	{
		offset &= 0x1fffff;

		if (m_upperbank & 0x1000) base |= 0x0400000;
		if (m_upperbank & 0x0800) base |= 0x0800000;

		return m_romregion[offset + (base / 2)];
	}

	return 0x0000;// m_romregion[offset];
}

void zon32bit_state::machine_start()
{
	spg2xx_game_state::machine_start();

	save_item(NAME(m_porta_dat));
	save_item(NAME(m_portb_dat));
	save_item(NAME(m_portc_dat));
	save_item(NAME(m_porta_mask));
	save_item(NAME(m_upperbank));
	save_item(NAME(m_basebank));
}


void zon32bit_state::machine_reset()
{
	spg2xx_game_state::machine_reset();

	m_porta_dat = 0xffff;
	m_portb_dat = 0x0000;

	m_basebank = 0;
	m_maincpu->invalidate_cache();
}

void mywicodx_state::machine_reset()
{
	zon32bit_state::machine_reset();
	m_basebank = 3;
	m_maincpu->invalidate_cache();
}


static INPUT_PORTS_START( zon32bit )
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "P1" )
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

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "P2" )
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
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Up (vertical) Left (horizontal)")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Down (vertical) Right (horizontal)")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Left (vertical) Down (horizontal)")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Right (vertical) Up (horizontal)")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Pause / Menu")
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
INPUT_PORTS_END


static INPUT_PORTS_START( oplayer )
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "P1" )
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

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0xff80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_DIPNAME( 0x0001, 0x0001, "P3" )
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
INPUT_PORTS_END


void zon32bit_state::zon32bit(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &zon32bit_state::mem_map_zon32bit);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(zon32bit_state::porta_r));
	m_maincpu->portb_in().set(FUNC(zon32bit_state::portb_r));
	m_maincpu->portc_in().set(FUNC(zon32bit_state::portc_r));

	m_maincpu->porta_out().set(FUNC(zon32bit_state::porta_w));
	m_maincpu->portb_out().set(FUNC(zon32bit_state::portb_w));
	m_maincpu->portc_out().set(FUNC(zon32bit_state::portc_w));
}

ROM_START( mywicodx )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	// the first bank contains the Mi Guitar game, the 2nd half of the ROM is where the Menu starts
	ROM_LOAD16_WORD_SWAP( "mywicodx.u2", 0x0000000, 0x4000000, CRC(ec7c5d2f) SHA1(330fb839c485713f7bec5bf9d2d42841612c5b45))
ROM_END


ROM_START( zon32bit )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 ) // probably should just swap upper 2 line of ROM, as pinout was unknown
	ROM_LOAD16_WORD_SWAP( "41sports.bin", 0x0000000, 0x0800000, CRC(86eee6e0) SHA1(3f6cab6649aebf596de5a8af21658bb1a27edb10) )
	ROM_CONTINUE(0x1000000, 0x0800000)
	ROM_CONTINUE(0x0800000, 0x0800000)
	ROM_CONTINUE(0x1800000, 0x0800000)
ROM_END


/*
	Following pinout was used for dumping (it is possible upper 2 address lines are swapped, we must swap them to match the mywicodx banking at least)

      +------------------+
  VCC -|01              70|- VCC
  A24 -|02              69|- A21
  VCC -|03              68|- A22
  A18 -|04              67|- A08
  A23 -|05              66|- A09
  A07 -|06              65|- A10
  A06 -|07              64|- A11
  A05 -|08              63|- A12
  A04 -|09              62|- A13
  A03 -|10              61|- A14
  A02 -|11              60|- A15
  A01 -|12              59|- A16
  A00 -|13              58|- A17
   NC -|14              57|- A20
   NC -|15              56|- A19
  /CE -|16              55|- VCC (/BYTE)
   NC -|17              54|- NC
  GND -|18   55LV512    53|- GND
   NC -|19              52|- NC
   NC -|20              51|- NC
   NC -|21              50|- NC
   NC -|22              49|- NC
   NC -|23              48|- NC
  /OE -|24              47|- NC
   NC -|25              46|- NC
  D08 -|26              45|- NC
  D09 -|27              44|- D00
  D10 -|28              43|- D01
  D11 -|29              42|- D02
   NC -|30              41|- D03
  D12 -|31              40|- D04
  D13 -|32              39|- D05
  D14 -|33              38|- D06
  D15 -|34              37|- D07
  GND -|35              36|- VCC
       +------------------+
*/

// Sunplus QL8041C die
ROM_START( oplayer )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "oplayer.bin", 0x0000000, 0x1000000, CRC(3dfa79e4) SHA1(fb658437e2db7114af03b2915e098871aa30b235) )
	ROM_CONTINUE(0x2000000, 0x1000000)
	ROM_CONTINUE(0x1000000, 0x1000000)
	ROM_CONTINUE(0x3000000, 0x1000000)
ROM_END


void oplayer_100in1_state::init_oplayer()
{
	// TODO: remove these hacks
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	// port a checks when starting a game in any given bank / starting the system
	rom[0x0f851 + (0x0000000 / 2)] = 0xf165;
	rom[0xaad1e + (0x1000000 / 2)] = 0xf165;
	rom[0x47d2d + (0x2000000 / 2)] = 0xf165;
	rom[0x1fb00 + (0x3000000 / 2)] = 0xf165;
	// port a checks when exiting a game in any given bank
	rom[0x7a506 + (0x0000000 / 2)] = 0xf165;
	rom[0xad051 + (0x1000000 / 2)] = 0xf165;
	rom[0xc351e + (0x3000000 / 2)] = 0xf165;
}



// Box advertises this as '40 Games Included' but the cartridge, which was glued directly to the PCB, not removable, is a 41-in-1.  Maybe some versions exist with a 40 game selection.
CONS( 200?, zon32bit,  0, 0, zon32bit, zon32bit, zon32bit_state,  empty_init,      "Jungle Soft / Ultimate Products (HK) Ltd",    "Zone 32-bit Gaming Console System (Family Sport 41-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// My Wico Deluxe was also available under the MiWi brand (exact model unknown, but it was a cart there instead of built in)
// Box claimed 53 Arcade Games + 8 Sports games + 24 Music games, although it's unclear where 24 Music Games comes from, there are 3, which are identical aside from the title screen.
// The Mi Guitar menu contains 24 games, but they're dupes, and just counting those would exclude the other Mi Fit and Mi Papacon menus (which also contain dupes)
CONS( 200?, mywicodx,  0, 0, zon32bit, zon32bit, mywicodx_state,  empty_init,      "<unknown>",                                   "My Wico Deluxe (Family Sport 85-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, oplayer,   0, 0, zon32bit, oplayer, oplayer_100in1_state, init_oplayer, "OPlayer", "OPlayer Mobile Game Console (MGS03-white) (Family Sport 100-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )