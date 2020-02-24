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

	void init_zon32bit() { m_game = 0; };
	void init_mywicodx() { m_game = 1; };

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ16_MEMBER(z32_rom_r);

	required_region_ptr<uint16_t> m_romregion;

	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_READ16_MEMBER(portb_r);
	DECLARE_READ16_MEMBER(portc_r);

	virtual DECLARE_WRITE16_MEMBER(porta_w) override;
	virtual DECLARE_WRITE16_MEMBER(portb_w) override;
	virtual DECLARE_WRITE16_MEMBER(portc_w) override;

private:
	int m_porta_dat;
	int m_portb_dat;
	int m_portc_dat;
	int m_porta_mask;
	int m_porta_real;

	int m_upperbank;

	int m_hackbank;
	int m_game;
};

READ16_MEMBER(zon32bit_state::porta_r)
{
	return m_porta_real;
}


WRITE16_MEMBER(zon32bit_state::porta_w)
{
	if ((m_porta_dat != data) || (m_porta_mask != mem_mask))
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
	m_porta_mask = mem_mask;

	m_porta_real = (m_porta_real & ~mem_mask) | (data & mem_mask);

	// where is the banking?! this gets written from the RAM-based code when the lower bank needs to change, but the upper bank needs to change in places too
	// (and all these bits get unset again after this write, so this probably isn't the bank)
	if (data == 0x0e01)
	{
		m_hackbank ^= 1;
		logerror("bank is now %d\n", m_hackbank);
	}

	/*
	if (data == 0x0335)
	{
	    logerror("%s: port a write 0x0355, port c is %04x %04X\n", machine().describe_context(), data, data & 0x1800);

	    m_upperbank = (m_portc_dat & 0x1800);
	}
	*/
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
		 
	m_portc_dat = data;
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

	if (m_game == 0) // zon32bit
	{
		if (m_hackbank) base |= 0x1000000;
	}
	else if (m_game == 1) // mywicodx
	{
		if (m_porta_real & 0x0800)  base |= 0x2000000;
		if (m_porta_real & 0x0400)  base |= 0x1000000;
	}

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
}


void zon32bit_state::machine_reset()
{
	spg2xx_game_state::machine_reset();

	m_porta_dat = 0xffff;
	m_porta_mask = 0xffff;
	m_portb_dat = 0x0000;
	m_porta_real = 0xffff;

	m_hackbank = 0;
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

void zon32bit_state::zon32bit(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &zon32bit_state::mem_map_zon32bit);
	m_maincpu->set_force_no_drc(true); // uses JVS opcode, not implemented in recompiler

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


// Box advertises this as '40 Games Included' but the cartridge, which was glued directly to the PCB, not removable, is a 41-in-1.  Maybe some versions exist with a 40 game selection.
CONS( 200?, zon32bit,  0, 0, zon32bit, zon32bit, zon32bit_state,  init_zon32bit,      "Jungle Soft / Ultimate Products (HK) Ltd",    "Zone 32-bit Gaming Console System (Family Sport 41-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// My Wico Deluxe was also available under the MiWi brand (exact model unknown, but it was a cart there instead of built in)
// Box claimed 53 Arcade Games + 8 Sports games + 24 Music games, although it's unclear where 24 Music Games comes from, there are 3, which are identical aside from the title screen.
// The Mi Guitar menu contains 24 games, but they're dupes, and just counting those would exclude the other Mi Fit and Mi Papacon menus (which also contain dupes)
CONS( 200?, mywicodx,  0, 0, zon32bit, zon32bit, zon32bit_state,  init_mywicodx,      "<unknown>",                                   "My Wico Deluxe (Family Sport 85-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

