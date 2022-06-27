// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

#include "emu.h"
#include "spg2xx.h"


class shredmjr_game_state : public spg2xx_game_state
{
public:
	shredmjr_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_porta_data(0x0000),
		m_shiftamount(0)
	{ }

	void shredmjr(machine_config &config);
	void taikeegr(machine_config &config);
	void taikeegrp(machine_config &config);

	void init_taikeegr();

protected:
	uint16_t porta_r();
	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

private:
	uint16_t m_porta_data;
	int m_shiftamount;

};


// Shredmaster Jr uses the same input order as the regular Taikee Guitar, but reads all inputs through a single multiplexed bit
void shredmjr_game_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (data != m_porta_data)
	{
		if ((data & 0x0800) != (m_porta_data & 0x0800))
		{
			if (data & 0x0800)
			{
				//logerror("0x0800 low -> high\n");
			}
			else
			{
				//logerror("0x0800 high -> low\n");
			}
		}

		if ((data & 0x0200) != (m_porta_data & 0x0200))
		{
			if (data & 0x0200)
			{
				//logerror("0x0200 low -> high\n");
				m_shiftamount++;
			}
			else
			{
				//logerror("0x0200 high -> low\n");
			}
		}

		if ((data & 0x0100) != (m_porta_data & 0x0100))
		{
			if (data & 0x0100)
			{
				//logerror("0x0100 low -> high\n");
				m_shiftamount = 0;
			}
			else
			{
				//logerror("0x0100 high -> low\n");
			}
		}
	}

	m_porta_data = data;
}

uint16_t shredmjr_game_state::porta_r()
{
	//logerror("porta_r with shift amount %d \n", m_shiftamount);
	uint16_t ret = 0x0000;

	uint16_t portdata = m_io_p1->read();

	portdata = (portdata >> m_shiftamount) & 0x1;

	if (portdata)
		ret |= 0x0400;

	return ret;
}

static INPUT_PORTS_START( taikeegr )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )   PORT_NAME("Strum Bar Down")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Strum Bar Up")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Whamming Bar")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Yellow")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Green")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Red")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Blue")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Pink")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( guitarstp )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )   PORT_NAME("Strum Bar Down")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Strum Bar Up")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Whamming Bar")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Yellow")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Blue")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Red")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Green")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Orange")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void shredmjr_game_state::shredmjr(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &shredmjr_game_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(shredmjr_game_state::porta_r));
	m_maincpu->porta_out().set(FUNC(shredmjr_game_state::porta_w));
}

void shredmjr_game_state::taikeegr(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &shredmjr_game_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set_ioport("P1");
}

void shredmjr_game_state::taikeegrp(machine_config &config)
{
	taikeegr(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);
}


void shredmjr_game_state::init_taikeegr()
{
	u16 *src = (u16*)memregion("maincpu")->base();

	for (int i = 0x00000; i < 0x800000/2; i++)
	{
		u16 dat = src[i];
		dat = bitswap<16>(dat,  15,14,13,12,   11,10,9,8,    7,6,5,4,   0,1,2,3 );
		src[i] = dat;
	}

	std::vector<u16> buffer(0x800000/2);

	for (int i = 0; i < 0x800000/2; i++)
	{
		int j = 0;

		switch (i & 0x00e00)
		{
		case 0x00000: j = (i & 0xfff1ff) | 0x000; break;
		case 0x00200: j = (i & 0xfff1ff) | 0x800; break;
		case 0x00400: j = (i & 0xfff1ff) | 0x400; break;
		case 0x00600: j = (i & 0xfff1ff) | 0xc00; break;
		case 0x00800: j = (i & 0xfff1ff) | 0x200; break;
		case 0x00a00: j = (i & 0xfff1ff) | 0xa00; break;
		case 0x00c00: j = (i & 0xfff1ff) | 0x600; break;
		case 0x00e00: j = (i & 0xfff1ff) | 0xe00; break;
		}

		buffer[j] = src[i];
	}

	std::copy(buffer.begin(), buffer.end(), &src[0]);
}


ROM_START( taikeegr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "taikee_guitar.bin", 0x000000, 0x800000, CRC(8cbe2feb) SHA1(d72e816f259ba6a6260d6bbaf20c5e9b2cf7140b) )
ROM_END

ROM_START( rockstar )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "29gl064.bin", 0x000000, 0x800000, CRC(40de50ff) SHA1(b33ae7a3d32911addf833998d7419f4830be5a07) )
ROM_END

ROM_START( shredmjr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "shredmasterjr.bin", 0x000000, 0x800000, CRC(95a6dcf1) SHA1(44893cd6ebe6b7f33a73817b72ae7be70c3126dc) )
ROM_END

ROM_START( guitarst )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "guitarstar_s29gl064m11tfir4_0001227e.bin", 0x000000, 0x800000, CRC(feaace47) SHA1(dd426bb4f03a16b1b96b63b4e0d79ea75097bf72) )
ROM_END

ROM_START( guitarstp )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "29gl064.u2", 0x000000, 0x800000, CRC(1dbcff73) SHA1(b179e4da6f38e7d5ec796bf846a63492d30eb0f5) )
ROM_END






// These were all sold as different products, use a different sets of songs / presentation styles (2D or perspective gameplay, modified titlescreens etc.)
// and sometimes even slightly different hardware, so aren't set as clones of each other

// box title not confirmed, Guitar Rock on title screen, has Bon Jovi etc.
CONS( 2007, taikeegr,    0,        0,        taikeegrp,    taikeegr, shredmjr_game_state, init_taikeegr, "TaiKee", "Guitar Rock (PAL)", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // timing not quite correct yet

// Plug 'N' Play Rockstar Guitar on box, Guitar Rock on title screen, has Manic Street Preachers etc.
CONS( 2007, rockstar,    0,        0,        taikeegrp,    taikeegr, shredmjr_game_state, init_taikeegr, "Ultimate Products / TaiKee", "Plug 'N' Play Rockstar Guitar / Guitar Rock (PAL)", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // timing not quite correct yet

// dreamGEAR branded presentation, modified hardware (buttons read in a different way) same song seletion as taikeegr
CONS( 2007, shredmjr,    0,        0,        shredmjr,     taikeegr, shredmjr_game_state, init_taikeegr, "dreamGEAR", "Shredmaster Jr (NTSC)", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // ^

// doesn't have a Senario logo ingame, but does on box.  unique song selection
CONS( 200?, guitarst,    0,        0,        taikeegr,     taikeegr, shredmjr_game_state, init_taikeegr, "Senario", "Guitar Star (US, Senario, NTSC)", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // ^

// This one has the same songs as 'rockstar' but different game style / presentation.
// Unit found in Ireland "imported by Cathay Product Sourcing Ltd." on the box, with address in Ireland
// ITEM #01109 on instruction sheet, no manufacturer named on either box or instructions
CONS( 200?, guitarstp,   0,        0,        taikeegrp,    guitarstp,shredmjr_game_state, init_taikeegr, "<unknown>", "Guitar Star (Europe, PAL)", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // ^
