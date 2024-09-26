// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

/* Shenzhen Senca Technology Co., Ltd developed "Family Sport" systems - sold by various manufacturers */

#include "emu.h"
#include "spg2xx.h"


namespace {

class zon32bit_state : public spg2xx_game_state
{
public:
	zon32bit_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_romregion(*this, "maincpu")
	{ }

	void zon32bit(machine_config& config);
	void zon32bit_bat(machine_config& config);

	void mem_map_zon32bit(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	uint16_t z32_rom_r(offs_t offset);

	required_region_ptr<uint16_t> m_romregion;

	virtual uint16_t porta_r();
	virtual uint16_t portb_r();
	virtual uint16_t portc_r();

	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	uint16_t an3_r();

	int m_porta_dat = 0;
	int m_portb_dat = 0;
	int m_portc_dat = 0;
	int m_porta_mask = 0;

	int m_upperbank = 0;

	int m_basebank = 0;
};

class mywicogt_state : public zon32bit_state
{
public:
	mywicogt_state(const machine_config& mconfig, device_type type, const char* tag) :
		zon32bit_state(mconfig, type, tag)
	{ }

protected:
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};


class mywicodx_state : public zon32bit_state
{
public:
	mywicodx_state(const machine_config& mconfig, device_type type, const char* tag) :
		zon32bit_state(mconfig, type, tag)
	{ }

protected:
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	virtual void machine_reset() override ATTR_COLD;
};

class oplayer_100in1_state : public mywicodx_state
{
public:
	oplayer_100in1_state(const machine_config& mconfig, device_type type, const char* tag) :
		mywicodx_state(mconfig, type, tag)
	{ }

	void init_oplayer();
	void init_m505neo();
	void init_cdlyoko();

protected:
	virtual uint16_t porta_r() override;
	virtual uint16_t portb_r() override;
	virtual uint16_t portc_r() override;
};

class denver_200in1_state : public mywicodx_state
{
public:
	denver_200in1_state(const machine_config& mconfig, device_type type, const char* tag) :
		mywicodx_state(mconfig, type, tag)
	{ }

	void init_denver();
	void init_m521neo();

protected:
	virtual void machine_reset() override ATTR_COLD;

	virtual uint16_t porta_r() override;
	virtual uint16_t portb_r() override;
	virtual uint16_t portc_r() override;

	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

uint16_t zon32bit_state::an3_r()
{
	int status = ioport("BATT")->read();
	if (status)
		return 0xfff;
	else
		return 0x000;
}

void zon32bit_state::device_post_load()
{
	// load state can change the bank, so we must invalide cache
	m_maincpu->invalidate_cache();
}

uint16_t zon32bit_state::porta_r()
{
	return m_porta_dat;
}


void zon32bit_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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


void mywicogt_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (1)
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

//[:] ':maincpu' (000508): porta_w 0b00 (0f00) x x x x | 1 0 1 1 | x x x x | x x x x
//[:] ':maincpu' (000510): porta_w 0b00 (0f00) x x x x | 1 0 1 1 | x x x x | x x x x
//[:] ':maincpu' (000518): porta_w 0f00 (0f00) x x x x | 1 1 1 1 | x x x x | x x x x

	if (m_maincpu->pc() < 0x1000)
	{
		if (data == 0x0f00)
		{
			m_basebank = 1;
			m_maincpu->invalidate_cache();
			logerror("changing to bank 1\n");

		}
		else if (data == 0x0b00)
		{
			m_basebank = 0;
			m_maincpu->invalidate_cache();
			logerror("changing to bank 0\n");
		}
	}

	m_porta_dat = data;
}


void mywicodx_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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


void denver_200in1_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (0)
	{
		if (m_maincpu->pc() < 0x10000)
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
		}
	}

	if (m_maincpu->pc() < 0x10000)
	{
		int oldbank = m_basebank;

		if (mem_mask & 0x0200)
		{
			if (data & 0x0200)
			{
				m_basebank |= 4;
			}
			else
			{
				m_basebank &= ~4;
			}
		}

		if (mem_mask & 0x0400)
		{
			if (data & 0x0400)
			{
				m_basebank |= 1;
			}
			else
			{
				m_basebank &= ~1;
			}
		}

		if (mem_mask & 0x0800)
		{
			if (data & 0x0800)
			{
				m_basebank |= 2;
			}
			else
			{
				m_basebank &= ~2;
			}
		}

		if (oldbank != m_basebank)
			m_maincpu->invalidate_cache();
	}
}


uint16_t zon32bit_state::portc_r()
{
	// 0x03ff seem to be inputs for buttons (and some kind of output?)
	// 0xfc00 gets masked for other reasons (including banking?)

	// returning same value written for 0x0400 means controls don't respond (some kind of direction flag?)

	uint16_t dat = m_io_p3->read() & ~0xf800;

	dat |= (m_portc_dat & 0xf800);

	return dat;
}


uint16_t zon32bit_state::portb_r()
{
	return m_portb_dat;
}

void zon32bit_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (data != 0x0001)
		logerror("%s: portb_w %04x (%04x)\n", machine().describe_context(), data, mem_mask);

	m_portb_dat = data;
}

void zon32bit_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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


uint16_t oplayer_100in1_state::portc_r()
{
	return m_io_p3->read();
}

uint16_t oplayer_100in1_state::portb_r()
{
	return m_io_p2->read();
}

uint16_t oplayer_100in1_state::porta_r()
{
	return 0x0ff8 | (machine().rand()&1);
}

void zon32bit_state::mem_map_zon32bit(address_map &map)
{
	map(0x000000, 0x3fffff).r(FUNC(zon32bit_state::z32_rom_r));
}

uint16_t zon32bit_state::z32_rom_r(offs_t offset)
{
	/*
	    This has upper and lower bank, which can be changed independently.
	    Banking hookup is currently very hacky as bank values are written
	    to ports then erased at the moment, maybe they latch somehow?
	*/

	int base = 0x0000000;

	if (m_basebank & 4)  base |= 0x4000000;
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

uint16_t denver_200in1_state::portc_r()
{
	return m_io_p3->read();
}

uint16_t denver_200in1_state::portb_r()
{
	return m_io_p2->read();
}

uint16_t denver_200in1_state::porta_r()
{
	return 0x0ff8 | (machine().rand()&1);
}


void zon32bit_state::machine_start()
{
	spg2xx_game_state::machine_start();

	m_basebank = 0;

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

void denver_200in1_state::machine_reset()
{
	zon32bit_state::machine_reset();
	m_basebank = 6;
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

static INPUT_PORTS_START( mywicogt )
	PORT_START("P1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Green / Left")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Red / Right")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Yellow")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Blue")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Orange")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start / Pause / Menu")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Back")
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNUSED )
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

	PORT_START("BATT")
	PORT_CONFNAME( 0x0001,  0x0001, "Battery Status" )
	PORT_CONFSETTING(       0x0000, "Low" )
	PORT_CONFSETTING(       0x0001, "High" )
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

void zon32bit_state::zon32bit_bat(machine_config& config)
{
	zon32bit(config);
	m_maincpu->adc_in<3>().set(FUNC(zon32bit_state::an3_r));
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

ROM_START( mywicogt )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mywicoguitar.bin", 0x0000000, 0x0800000, CRC(3c037c50) SHA1(3b9a28fb643c9f90563d653be0f38eba09c26f26) )
	ROM_CONTINUE(0x1000000, 0x0800000)
	ROM_CONTINUE(0x0800000, 0x0800000)
	ROM_CONTINUE(0x1800000, 0x0800000)
ROM_END


/*
    Following pinout was used for dumping

      +------------------+
  VCC -|01              70|- VCC
  A23 -|02              69|- A21
  VCC -|03              68|- A22
  A18 -|04              67|- A08
  A24 -|05              66|- A09
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
	ROM_LOAD16_WORD_SWAP( "oplayer.bin", 0x0000000, 0x4000000, CRC(aa09c358) SHA1(df2855cdfdf2b693636cace8768e579b9d5bc657) )
ROM_END

ROM_START( cdlyoko ) // P1-25IN1-MAIN-V11 2011.05.09 on PCB
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "clg001z.u2", 0x0000000, 0x1000000, CRC(1e73b910) SHA1(6f6cece054fcf91ff78962358804471794c58615) )
	ROM_RELOAD(0x1000000,0x1000000)
	ROM_RELOAD(0x2000000,0x1000000)
	ROM_RELOAD(0x3000000,0x1000000)
ROM_END

ROM_START( dnv200fs )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "famsport200in1.u2", 0x0000000, 0x8000000, CRC(f59221e2) SHA1(d532cf5a80ffe9d527efcccbf380a7a860f0fbd9) )
ROM_END

ROM_START( m505neo )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	// 1st and 2nd half are identical apart from one bit.  Likely used double capacity ROM and only wired up half of it because ROM had a problem?
	// the data segment in question is identical to one in oplayer and suggests the first half of the ROM here is correct with the bit being set
	// incorrectly in the 2nd half of the ROM.
	ROM_LOAD16_WORD_SWAP( "m505arcadeneo.u2", 0x0000000, 0x4000000, CRC(b72bdbe1) SHA1(263b60148980ac1f82546e2449b1dd938b7b827c) )
	ROM_IGNORE(0x4000000)
ROM_END

ROM_START( m521neo )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 ) // was this dumped with some address lines swapped?
	ROM_LOAD16_WORD_SWAP( "6gu-1cd-a.u2", 0x0000000, 0x800000, CRC(7cb31b4c) SHA1(8de44756747a292c5d39bd491048d6fac4219953) )
	ROM_CONTINUE(0x01000000, 0x800000)
	ROM_CONTINUE(0x00800000, 0x800000)
	ROM_CONTINUE(0x01800000, 0x800000)

	ROM_CONTINUE(0x02000000, 0x800000)
	ROM_CONTINUE(0x03000000, 0x800000)
	ROM_CONTINUE(0x02800000, 0x800000)
	ROM_CONTINUE(0x03800000, 0x800000)

	ROM_CONTINUE(0x04000000, 0x800000)
	ROM_CONTINUE(0x05000000, 0x800000)
	ROM_CONTINUE(0x04800000, 0x800000)
	ROM_CONTINUE(0x05800000, 0x800000)

	ROM_CONTINUE(0x06000000, 0x800000)
	ROM_CONTINUE(0x07000000, 0x800000)
	ROM_CONTINUE(0x06800000, 0x800000)
	ROM_CONTINUE(0x07800000, 0x800000)
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

/*

oplayer   m505
---------------
0001 |    0001    0
0002 |    0100    8
0004 |    0002    1
0008 |    0200    9
0010 |    0004    2
0020 |    0400   10
0040 |    0008    3
0080 |    0800   11


0100 |    8000   15
0200 |    0080    7
0400 |    4000   14
0800 |    0040    6
1000 |    2000   13
2000 |    0020    5
4000 |    1000   12
8000 |    0010    4

*/

void oplayer_100in1_state::init_m505neo()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < size / 2; i++)
	{
		ROM[i] = bitswap<16>(ROM[i],
									 11,  3,   10,  2,
									 9,  1,  8,  0,

									 4, 12, 5, 13,
									 6, 14,  7,  15
			);

	}

	// TODO: remove these hacks
	// port a checks when starting the system
	ROM[0x43c30 + (0x2000000 / 2)] = 0xf165; // boot main bank
}

void oplayer_100in1_state::init_cdlyoko()
{
	uint16_t* rom16 = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	std::vector<u16> buffer(size / 2);

	for (int i = 0; i < size / 2; i++)
	{
		buffer[bitswap<25>(i, 0x18, 0x17, 0x16, 0x15, 0x13, 0x8, 0x12, 0x11, 0x14, 0x10, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0)] = rom16[i];
	}

	std::copy(buffer.begin(), buffer.end(), &rom16[0]);

	// patch a startup check, like oplayer
	for (int i = 0; i < 4; i++)
		rom16[0x493ed + ((0x1000000 * i) / 2)] = 0xf165;
}


void denver_200in1_state::init_denver()
{
	// TODO: remove these hacks

	// patch checks when booting each bank, similar to oplayer
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	//rom[0x175f7 + (0x0000000 / 2)] = 0xf165;
	//rom[0x18f47 + (0x1000000 / 2)] = 0xf165;
	//rom[0x33488 + (0x2000000 / 2)] = 0xf165;
	//rom[0x87f81 + (0x3000000 / 2)] = 0xf165;
	//rom[0x764d9 + (0x4000000 / 2)] = 0xf165;
	//rom[0xb454e + (0x5000000 / 2)] = 0xf165;
	rom[0x43c30 + (0x6000000 / 2)] = 0xf165; // boot main bank
	//rom[0x1fb00 + (0x7000000 / 2)] = 0xf165;
}

void denver_200in1_state::init_m521neo()
{
	uint16_t* rom = (uint16_t*)memregion("maincpu")->base();
	rom[0x43c30 + (0x6000000 / 2)] = 0xf165; // boot main bank
}

} // anonymous namespace


// Box advertises this as '40 Games Included' but the cartridge, which was glued directly to the PCB, not removable, is a 41-in-1.  Maybe some versions exist with a 40 game selection.
CONS( 200?, zon32bit,  0, 0, zon32bit, zon32bit, zon32bit_state,  empty_init,      "Ultimate Products (HK) Ltd / Senca",    "Zone 32-bit Gaming Console System (Family Sport 41-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// My Wico Deluxe was also available under the MiWi brand (exact model unknown, but it was a cart there instead of built in)
// Box claimed 53 Arcade Games + 8 Sports games + 24 Music games, although it's unclear where 24 Music Games comes from, there are 3, which are identical aside from the title screen.
// The Mi Guitar menu contains 24 games, but they're dupes, and just counting those would exclude the other Mi Fit and Mi Papacon menus (which also contain dupes)
CONS( 200?, mywicodx,  0, 0, zon32bit, zon32bit, mywicodx_state,  empty_init,      "<unknown> / Senca",                                   "My Wico Deluxe (Family Sport 85-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Shows Mi Guitar 2 in the menu, it seems likely that there was an earlier version on VT1682 hardware as there is a very similar Guitar game (with the same song selection) in those multigames
CONS( 200?, mywicogt,  0, 0, zon32bit, mywicogt, mywicogt_state,  empty_init,      "<unknown> / Senca",                                   "My Wico Guitar", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, oplayer,   0, 0, zon32bit_bat, oplayer, oplayer_100in1_state, init_oplayer, "OPlayer / Senca", "OPlayer Mobile Game Console (MGS03-white) (Family Sport 100-in-1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2012, m505neo,   0, 0, zon32bit_bat, oplayer, oplayer_100in1_state, init_m505neo, "Millennium 2000 GmbH / Senca", "Millennium M505 Arcade Neo Portable Spielkonsole (Family Sport 100-in-1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2011, cdlyoko,   0, 0, zon32bit_bat, oplayer, oplayer_100in1_state, init_cdlyoko, "Ingo Devices SL / Senca", "Code Lyoko (25-in-1 handheld)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// a version of this exists with the 'newer' style title screen seen in m505neo
CONS( 2012, m521neo,   0, 0, zon32bit_bat, oplayer, denver_200in1_state,  init_m521neo, "Millennium 2000 GmbH / Senca", "Millennium M521 Arcade Neo 2.0 (Family Sport 220-in-1) ", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

/*
DENVER(r)

PO:9075
Model: GMP-270CMK2
OPERATED BY 3 X AAA-BATTERIES (NOT INCL)
OR MINI-USB POWER SOURCE
Power consumption:0.6W/hour
Standby consumption:0.25mW/hour
Imported by:
DENVER ELECTRONICS A/S
Stavneagervej 22
DK-8250 EGAA
DENMARK

*/

CONS( 200?, dnv200fs,   0, 0, zon32bit_bat, oplayer, denver_200in1_state, init_denver, "Denver / Senca", "Denver (GMP-270CMK2) (Family Sport 200-in-1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )


