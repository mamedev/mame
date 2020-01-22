// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

#include "includes/spg2xx.h"

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
	
	void init_taikeegr();

protected:
	DECLARE_READ16_MEMBER(porta_r);
	virtual  DECLARE_WRITE16_MEMBER(porta_w) override;

private:
	uint16_t m_porta_data;
	int m_shiftamount;

};


// Shredmaster Jr uses the same input order as the regular Taikee Guitar, but reads all inputs through a single multplexed bit
WRITE16_MEMBER(shredmjr_game_state::porta_w)
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

READ16_MEMBER(shredmjr_game_state::porta_r)
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
	m_maincpu->set_pal(true);

	spg2xx_base(config);

	m_screen->set_refresh_hz(50);
//  m_screen->set_size(320, 312);

	m_maincpu->porta_in().set_ioport("P1");
//  m_maincpu->portb_in().set_ioport("P2");
//  m_maincpu->portc_in().set_ioport("P3");
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

ROM_START( shredmjr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "shredmasterjr.bin", 0x000000, 0x800000, CRC(95a6dcf1) SHA1(44893cd6ebe6b7f33a73817b72ae7be70c3126dc) )
ROM_END

// there are multiple versions of this with different songs, was also sold by dreamGEAR as 'Shredmaster Jr.' (different title screen)
// for the UK version the title screen always shows "Guitar Rock", however there are multiple boxes with different titles and song selections.
// ROM is glued on the underside and soldered to the PCB, very difficult to remove without damaging.
CONS( 2007, taikeegr,    0,        0,        taikeegr,     taikeegr, shredmjr_game_state, init_taikeegr, "TaiKee", "Rockstar Guitar / Guitar Rock (PAL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // bad music timings (too slow)
CONS( 2007, shredmjr,    taikeegr, 0,        shredmjr,     taikeegr, shredmjr_game_state, init_taikeegr, "dreamGEAR", "Shredmaster Jr (NTSC)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // bad music timings (too slow)
