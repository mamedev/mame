// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Machine Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/
/******************************************************************************
Memo:

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/nb1413m3.h"


#define NB1413M3_DEBUG  0

#define NB1413M3_TIMER_BASE 20000000

const device_type NB1413M3 = &device_creator<nb1413m3_device>;

nb1413m3_device::nb1413m3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NB1413M3, "NB1413M3 Mahjong Custom", tag, owner, clock, "nb1413m3", __FILE__),
	m_sndromrgntag("voice"),
	m_sndrombank1(0),
	m_sndrombank2(0),
	m_busyctr(0),
	m_busyflag(1),
	m_outcoin_flag(1),
	m_inputport(0xff),
	m_74ls193_counter(0),
	m_nmi_count(0),
	m_nmi_clock(0),
	m_nmi_enable(0),
	m_counter(0),
	m_gfxradr_l(0),
	m_gfxradr_h(0),
	m_gfxrombank(0),
	m_outcoin_enable(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nb1413m3_device::device_start()
{
	m_timer_cb = timer_alloc(TIMER_CB);
	m_timer_cb->adjust(attotime::zero);

	save_item(NAME(m_nb1413m3_type));
	save_item(NAME(m_sndrombank1));
	save_item(NAME(m_sndrombank2));
	save_item(NAME(m_busyctr));
	save_item(NAME(m_busyflag));
	save_item(NAME(m_inputport));
	save_item(NAME(m_74ls193_counter));
	save_item(NAME(m_nmi_count));
	save_item(NAME(m_nmi_clock));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_counter));
	save_item(NAME(m_gfxradr_l));
	save_item(NAME(m_gfxradr_h));
	save_item(NAME(m_gfxrombank));
	save_item(NAME(m_outcoin_enable));
	save_item(NAME(m_outcoin_flag));
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void nb1413m3_device::device_reset()
{
	m_nmi_clock = 0;
	m_nmi_enable = 0;
	m_nmi_count = 0;
	m_74ls193_counter = 0;
	m_counter = 0;
	m_sndromrgntag = "voice";
	m_sndrombank1 = 0;
	m_sndrombank2 = 0;
	m_busyctr = 0;
	m_busyflag = 1;
	m_gfxradr_l = 0;
	m_gfxradr_h = 0;
	m_gfxrombank = 0;
	m_inputport = 0xff;
	m_outcoin_flag = 1;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void nb1413m3_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_CB:
			timer_callback(ptr, param);
			break;
		default:
			assert_always(FALSE, "Unknown id in nb1413m3_device::device_timer");
	}
}

/* TODO: is all of this actually programmable? */
TIMER_CALLBACK_MEMBER( nb1413m3_device::timer_callback )
{
	m_timer_cb->adjust(attotime::from_hz(NB1413M3_TIMER_BASE) * 256);

	m_74ls193_counter++;
	m_74ls193_counter &= 0x0f;

	if (m_74ls193_counter == 0x0f)
	{
		if (m_nmi_enable)
		{
			machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
			m_nmi_count++;
		}

		switch (m_nb1413m3_type)
		{
			case NB1413M3_TAIWANMB:
				m_74ls193_counter = 0x05;
				break;
			case NB1413M3_OMOTESND:
				m_74ls193_counter = 0x05;
				break;
			case NB1413M3_PASTELG:
				m_74ls193_counter = 0x02;
				break;
			case NB1413M3_HYHOO:
			case NB1413M3_HYHOO2:
				m_74ls193_counter = 0x05;
				break;
		}
	}
}


WRITE8_MEMBER( nb1413m3_device::nmi_clock_w )
{
	m_nmi_clock = data;

	switch (m_nb1413m3_type)
	{
		case NB1413M3_APPAREL:
		case NB1413M3_CITYLOVE:
		case NB1413M3_MCITYLOV:
		case NB1413M3_SECOLOVE:
		case NB1413M3_SEIHA:
		case NB1413M3_SEIHAM:
		case NB1413M3_IEMOTO:
		case NB1413M3_IEMOTOM:
		case NB1413M3_BIJOKKOY:
		case NB1413M3_BIJOKKOG:
		case NB1413M3_RYUUHA:
		case NB1413M3_OJOUSAN:
		case NB1413M3_OJOUSANM:
		case NB1413M3_KORINAI:
		case NB1413M3_KORINAIM:
		case NB1413M3_HOUSEMNQ:
		case NB1413M3_HOUSEMN2:
		case NB1413M3_LIVEGAL:
		case NB1413M3_ORANGEC:
		case NB1413M3_ORANGECI:
		case NB1413M3_VIPCLUB:
		case NB1413M3_MMSIKAKU:
		case NB1413M3_KANATUEN:
		case NB1413M3_KYUHITO:
			m_nmi_clock -= 1;
			break;
#if 1
		case NB1413M3_NIGHTLOV:
			m_nmi_enable = ((data & 0x08) >> 3);
			m_nmi_enable |= ((data & 0x01) ^ 0x01);
			m_nmi_clock -= 1;

			m_sndrombank1 = 1;
			break;
#endif
	}

	m_74ls193_counter = ((m_nmi_clock & 0xf0) >> 4);

}

READ8_MEMBER( nb1413m3_device::sndrom_r )
{
	int rombank;

	/* get top 8 bits of the I/O port address */
	offset = (offset << 8) | (space.device().state().state_int(Z80_BC) >> 8);

	switch (m_nb1413m3_type)
	{
		case NB1413M3_IEMOTO:
		case NB1413M3_IEMOTOM:
		case NB1413M3_SEIHA:
		case NB1413M3_SEIHAM:
		case NB1413M3_RYUUHA:
		case NB1413M3_OJOUSAN:
		case NB1413M3_OJOUSANM:
		case NB1413M3_MJSIKAKU:
		case NB1413M3_MMSIKAKU:
		case NB1413M3_KORINAI:
		case NB1413M3_KORINAIM:
			rombank = (m_sndrombank2 << 1) + (m_sndrombank1 & 0x01);
			break;
		case NB1413M3_HYHOO:
		case NB1413M3_HYHOO2:
			rombank = (m_sndrombank1 & 0x01);
			break;
		case NB1413M3_APPAREL:      // no samples
		case NB1413M3_NIGHTLOV:     // 0-1
		case NB1413M3_SECOLOVE:     // 0-1
		case NB1413M3_CITYLOVE:     // 0-1
		case NB1413M3_MCITYLOV:     // 0-1
		case NB1413M3_HOUSEMNQ:     // 0-1
		case NB1413M3_HOUSEMN2:     // 0-1
		case NB1413M3_LIVEGAL:      // 0-1
		case NB1413M3_ORANGEC:      // 0-1
		case NB1413M3_ORANGECI:     // 0-1
		case NB1413M3_VIPCLUB:      // 0-1
		case NB1413M3_KAGUYA:       // 0-3
		case NB1413M3_KAGUYA2:      // 0-3 + 4-5 for protection
		case NB1413M3_BIJOKKOY:     // 0-7
		case NB1413M3_BIJOKKOG:     // 0-7
		case NB1413M3_OTONANO:      // 0-7
		case NB1413M3_MJCAMERA:     // 0 + 4-5 for protection
		case NB1413M3_IDHIMITU:     // 0 + 4-5 for protection
		case NB1413M3_KANATUEN:     // 0 + 6 for protection
			rombank = m_sndrombank1;
			break;
		case NB1413M3_TAIWANMB:
		case NB1413M3_OMOTESND:
		case NB1413M3_SCANDAL:
		case NB1413M3_SCANDALM:
		case NB1413M3_MJFOCUSM:
		case NB1413M3_BANANADR:
			offset = (((offset & 0x7f00) >> 8) | ((offset & 0x0080) >> 0) | ((offset & 0x007f) << 8));
			rombank = (m_sndrombank1 >> 1);
			break;
		case NB1413M3_MMCAMERA:
		case NB1413M3_MSJIKEN:
		case NB1413M3_HANAMOMO:
		case NB1413M3_TELMAHJN:
		case NB1413M3_GIONBANA:
		case NB1413M3_MGION:
		case NB1413M3_MGMEN89:
		case NB1413M3_MJFOCUS:
		case NB1413M3_GALKOKU:
		case NB1413M3_HYOUBAN:
		case NB1413M3_MJNANPAS:
		case NB1413M3_MLADYHTR:
		case NB1413M3_CLUB90S:
		case NB1413M3_OHPAIPEE:
		case NB1413M3_TOGENKYO:
		case NB1413M3_LOVEHOUS:
		case NB1413M3_CHINMOKU:
		case NB1413M3_GALKAIKA:
		case NB1413M3_MCONTEST:
		case NB1413M3_UCHUUAI:
		case NB1413M3_TOKIMBSJ:
		case NB1413M3_TOKYOGAL:
		case NB1413M3_MAIKO:
		case NB1413M3_MMAIKO:
		case NB1413M3_HANAOJI:
		case NB1413M3_PAIRSNB:
		case NB1413M3_PAIRSTEN:
		default:
			rombank = (m_sndrombank1 >> 1);
			break;
	}

	offset += 0x08000 * rombank;

#if NB1413M3_DEBUG
	popmessage("Sound ROM %02X:%05X [B1:%02X B2:%02X]", rombank, offset, m_sndrombank1, m_sndrombank2);
#endif

	if (offset < space.machine().root_device().memregion(m_sndromrgntag)->bytes())
		return space.machine().root_device().memregion(m_sndromrgntag)->base()[offset];
	else
	{
		popmessage("read past sound ROM length (%05x[%02X])",offset, rombank);
		return 0;
	}
}

WRITE8_MEMBER( nb1413m3_device::sndrombank1_w )
{
	// if (data & 0x02) coin counter ?
	outcoin_w(space, 0, data);             // (data & 0x04) >> 2;
	m_nmi_enable = ((data & 0x20) >> 5);
	m_sndrombank1 = (((data & 0xc0) >> 5) | ((data & 0x10) >> 4));
}

WRITE8_MEMBER( nb1413m3_device::sndrombank2_w )
{
	m_sndrombank2 = (data & 0x03);
}

READ8_MEMBER( nb1413m3_device::gfxrom_r )
{
	UINT8 *GFXROM = space.machine().root_device().memregion("gfx1")->base();

	return GFXROM[(0x20000 * (m_gfxrombank | ((m_sndrombank1 & 0x02) << 3))) + ((0x0200 * m_gfxradr_h) + (0x0002 * m_gfxradr_l)) + (offset & 0x01)];
}

WRITE8_MEMBER( nb1413m3_device::gfxrombank_w )
{
	m_gfxrombank = (((data & 0xc0) >> 4) + (data & 0x03));
}

WRITE8_MEMBER( nb1413m3_device::gfxradr_l_w )
{
	m_gfxradr_l = data;
}

WRITE8_MEMBER( nb1413m3_device::gfxradr_h_w )
{
	m_gfxradr_h = data;
}

WRITE8_MEMBER( nb1413m3_device::inputportsel_w )
{
	m_inputport = data;
}

READ8_MEMBER( nb1413m3_device::inputport0_r )
{
	return ((space.machine().root_device().ioport("SYSTEM")->read() & 0xfd) | ((m_outcoin_flag & 0x01) << 1));
}

READ8_MEMBER( nb1413m3_device::inputport1_r )
{
	device_t &root = space.machine().root_device();
	switch (m_nb1413m3_type)
	{
		case NB1413M3_HYHOO:
		case NB1413M3_HYHOO2:
			switch ((m_inputport ^ 0xff) & 0x07)
			{
				case 0x01:  return root.ioport("IN0")->read();
				case 0x02:  return root.ioport("IN1")->read();
				case 0x04:  return 0xff;
				default:    return 0xff;
			}
		case NB1413M3_MSJIKEN:
		case NB1413M3_TELMAHJN:
			if (root.ioport("DSWA")->read() & 0x80)
			{
				switch ((m_inputport ^ 0xff) & 0x1f)
				{
					case 0x01:  return root.ioport("KEY0")->read();
					case 0x02:  return root.ioport("KEY1")->read();
					case 0x04:  return root.ioport("KEY2")->read();
					case 0x08:  return root.ioport("KEY3")->read();
					case 0x10:  return root.ioport("KEY4")->read();
					default:    return (root.ioport("KEY0")->read() & root.ioport("KEY1")->read() & root.ioport("KEY2")->read()
										& root.ioport("KEY3")->read() & root.ioport("KEY4")->read());
				}
			}
			else return root.ioport("JAMMA2")->read();
		case NB1413M3_PAIRSNB:
		case NB1413M3_PAIRSTEN:
		case NB1413M3_OHPAIPEE:
		case NB1413M3_TOGENKYO:
			return root.ioport("P1")->read();
		default:
			switch ((m_inputport ^ 0xff) & 0x1f)
			{
				case 0x01:  return root.ioport("KEY0")->read();
				case 0x02:  return root.ioport("KEY1")->read();
				case 0x04:  return root.ioport("KEY2")->read();
				case 0x08:  return root.ioport("KEY3")->read();
				case 0x10:  return root.ioport("KEY4")->read();
				default:    return (root.ioport("KEY0")->read() & root.ioport("KEY1")->read() & root.ioport("KEY2")->read()
									& root.ioport("KEY3")->read() & root.ioport("KEY4")->read());
			}
	}
}

READ8_MEMBER( nb1413m3_device::inputport2_r )
{
	device_t &root = space.machine().root_device();
	switch (m_nb1413m3_type)
	{
		case NB1413M3_HYHOO:
		case NB1413M3_HYHOO2:
			switch ((m_inputport ^ 0xff) & 0x07)
			{
				case 0x01:  return 0xff;
				case 0x02:  return 0xff;
				case 0x04:  return root.ioport("IN2")->read();
				default:    return 0xff;
			}
		case NB1413M3_MSJIKEN:
		case NB1413M3_TELMAHJN:
			if (root.ioport("DSWA")->read() & 0x80)
			{
				switch ((m_inputport ^ 0xff) & 0x1f)
				{
					case 0x01:  return root.ioport("KEY5")->read();
					case 0x02:  return root.ioport("KEY6")->read();
					case 0x04:  return root.ioport("KEY7")->read();
					case 0x08:  return root.ioport("KEY8")->read();
					case 0x10:  return root.ioport("KEY9")->read();
					default:    return (root.ioport("KEY5")->read() & root.ioport("KEY6")->read() & root.ioport("KEY7")->read()
										& root.ioport("KEY8")->read() & root.ioport("KEY9")->read());
				}
			}
			else return root.ioport("JAMMA1")->read();
		case NB1413M3_PAIRSNB:
		case NB1413M3_PAIRSTEN:
		case NB1413M3_OHPAIPEE:
		case NB1413M3_TOGENKYO:
			return root.ioport("P2")->read();
		default:
			switch ((m_inputport ^ 0xff) & 0x1f)
			{
				case 0x01:  return root.ioport("KEY5")->read();
				case 0x02:  return root.ioport("KEY6")->read();
				case 0x04:  return root.ioport("KEY7")->read();
				case 0x08:  return root.ioport("KEY8")->read();
				case 0x10:  return root.ioport("KEY9")->read();
				default:    return (root.ioport("KEY5")->read() & root.ioport("KEY6")->read() & root.ioport("KEY7")->read()
									& root.ioport("KEY8")->read() & root.ioport("KEY9")->read());
			}
	}
}

READ8_MEMBER( nb1413m3_device::inputport3_r )
{
	switch (m_nb1413m3_type)
	{
		case NB1413M3_TAIWANMB:
		case NB1413M3_IEMOTOM:
		case NB1413M3_OJOUSANM:
		case NB1413M3_SEIHAM:
		case NB1413M3_RYUUHA:
		case NB1413M3_KORINAIM:
		case NB1413M3_HYOUBAN:
		case NB1413M3_TOKIMBSJ:
		case NB1413M3_MJFOCUSM:
		case NB1413M3_SCANDALM:
		case NB1413M3_BANANADR:
		case NB1413M3_FINALBNY:
		case NB1413M3_MMSIKAKU:
			return ((m_outcoin_flag & 0x01) << 1);
		default:
			return 0xff;
	}
}

READ8_MEMBER( nb1413m3_device::dipsw1_r )
{
	device_t &root = space.machine().root_device();
	switch (m_nb1413m3_type)
	{
		case NB1413M3_KANATUEN:
		case NB1413M3_KYUHITO:
			return root.ioport("DSWB")->read();
		case NB1413M3_TAIWANMB:
			return ((root.ioport("DSWA")->read() & 0xf0) | ((root.ioport("DSWB")->read() & 0xf0) >> 4));
		case NB1413M3_OTONANO:
		case NB1413M3_MJCAMERA:
		case NB1413M3_IDHIMITU:
		case NB1413M3_KAGUYA2:
			return (((root.ioport("DSWA")->read() & 0x0f) << 4) | (root.ioport("DSWB")->read() & 0x0f));
		case NB1413M3_SCANDAL:
		case NB1413M3_SCANDALM:
		case NB1413M3_MJFOCUSM:
		case NB1413M3_GALKOKU:
		case NB1413M3_HYOUBAN:
		case NB1413M3_GALKAIKA:
		case NB1413M3_MCONTEST:
		case NB1413M3_UCHUUAI:
		case NB1413M3_TOKIMBSJ:
		case NB1413M3_TOKYOGAL:
			return ((root.ioport("DSWA")->read() & 0x0f) | ((root.ioport("DSWB")->read() & 0x0f) << 4));
		case NB1413M3_TRIPLEW1:
		case NB1413M3_NTOPSTAR:
		case NB1413M3_PSTADIUM:
		case NB1413M3_TRIPLEW2:
		case NB1413M3_VANILLA:
		case NB1413M3_FINALBNY:
		case NB1413M3_MJLSTORY:
		case NB1413M3_QMHAYAKU:
		case NB1413M3_MJGOTTUB:
			return (((root.ioport("DSWB")->read() & 0x01) >> 0) | ((root.ioport("DSWB")->read() & 0x04) >> 1) |
					((root.ioport("DSWB")->read() & 0x10) >> 2) | ((root.ioport("DSWB")->read() & 0x40) >> 3) |
					((root.ioport("DSWA")->read() & 0x01) << 4) | ((root.ioport("DSWA")->read() & 0x04) << 3) |
					((root.ioport("DSWA")->read() & 0x10) << 2) | ((root.ioport("DSWA")->read() & 0x40) << 1));
		default:
			return space.machine().root_device().ioport("DSWA")->read();
	}
}

READ8_MEMBER( nb1413m3_device::dipsw2_r )
{
	device_t &root = space.machine().root_device();
	switch (m_nb1413m3_type)
	{
		case NB1413M3_KANATUEN:
		case NB1413M3_KYUHITO:
			return root.ioport("DSWA")->read();
		case NB1413M3_TAIWANMB:
			return (((root.ioport("DSWA")->read() & 0x0f) << 4) | (root.ioport("DSWB")->read() & 0x0f));
		case NB1413M3_OTONANO:
		case NB1413M3_MJCAMERA:
		case NB1413M3_IDHIMITU:
		case NB1413M3_KAGUYA2:
			return ((root.ioport("DSWA")->read() & 0xf0) | ((root.ioport("DSWB")->read() & 0xf0) >> 4));
		case NB1413M3_SCANDAL:
		case NB1413M3_SCANDALM:
		case NB1413M3_MJFOCUSM:
		case NB1413M3_GALKOKU:
		case NB1413M3_HYOUBAN:
		case NB1413M3_GALKAIKA:
		case NB1413M3_MCONTEST:
		case NB1413M3_UCHUUAI:
		case NB1413M3_TOKIMBSJ:
		case NB1413M3_TOKYOGAL:
			return (((root.ioport("DSWA")->read() & 0xf0) >> 4) | (root.ioport("DSWB")->read() & 0xf0));
		case NB1413M3_TRIPLEW1:
		case NB1413M3_NTOPSTAR:
		case NB1413M3_PSTADIUM:
		case NB1413M3_TRIPLEW2:
		case NB1413M3_VANILLA:
		case NB1413M3_FINALBNY:
		case NB1413M3_MJLSTORY:
		case NB1413M3_QMHAYAKU:
		case NB1413M3_MJGOTTUB:
			return (((root.ioport("DSWB")->read() & 0x02) >> 1) | ((root.ioport("DSWB")->read() & 0x08) >> 2) |
					((root.ioport("DSWB")->read() & 0x20) >> 3) | ((root.ioport("DSWB")->read() & 0x80) >> 4) |
					((root.ioport("DSWA")->read() & 0x02) << 3) | ((root.ioport("DSWA")->read() & 0x08) << 2) |
					((root.ioport("DSWA")->read() & 0x20) << 1) | ((root.ioport("DSWA")->read() & 0x80) << 0));
		default:
			return space.machine().root_device().ioport("DSWB")->read();
	}
}

READ8_MEMBER( nb1413m3_device::dipsw3_l_r )
{
	return ((space.machine().root_device().ioport("DSWC")->read() & 0xf0) >> 4);
}

READ8_MEMBER( nb1413m3_device::dipsw3_h_r )
{
	return ((space.machine().root_device().ioport("DSWC")->read() & 0x0f) >> 0);
}

WRITE8_MEMBER( nb1413m3_device::outcoin_w )
{
	m_outcoin_enable = (data & 0x04) >> 2;

	switch (m_nb1413m3_type)
	{
		case NB1413M3_TAIWANMB:
		case NB1413M3_IEMOTOM:
		case NB1413M3_OJOUSANM:
		case NB1413M3_SEIHAM:
		case NB1413M3_RYUUHA:
		case NB1413M3_KORINAIM:
		case NB1413M3_MMSIKAKU:
		case NB1413M3_HYOUBAN:
		case NB1413M3_TOKIMBSJ:
		case NB1413M3_MJFOCUSM:
		case NB1413M3_SCANDALM:
		case NB1413M3_BANANADR:
		case NB1413M3_MGION:
		case NB1413M3_HANAOJI:
		case NB1413M3_FINALBNY:
		case NB1413M3_LOVEHOUS:
		case NB1413M3_MMAIKO:
			if (m_outcoin_enable)
			{
				if (m_counter++ == 2)
				{
					m_outcoin_flag ^= 1;
					m_counter = 0;
				}
			}
			break;
		default:
			break;
	}

	set_led_status(space.machine(), 2, m_outcoin_flag);      // out coin
}

WRITE8_MEMBER( nb1413m3_device::vcrctrl_w )
{
	if (data & 0x08)
	{
		popmessage(" ** VCR CONTROL ** ");
		set_led_status(space.machine(), 2, 1);
	}
	else
	{
		set_led_status(space.machine(), 2, 0);
	}
}

/* Nichibutsu Mahjong games share a common control panel */
INPUT_PORTS_START( nbmjcontrols )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Hanafuda controls share part of the mahjong panel. Notice that some of the remaining
inputs are detected in Service Mode, even if we label them as IPT_UNKNOWN because they
do not correspond to actual inputs */
INPUT_PORTS_START( nbhf1_ctrl ) // used by gionbana, mgion, abunai
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_G )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_H )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( nbhf2_ctrl ) // used by maiko, hanaoji, hnxmasev and hnageman
	PORT_INCLUDE( nbhf1_ctrl )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )

	PORT_MODIFY("KEY2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)

	PORT_MODIFY("KEY6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)

	PORT_MODIFY("KEY7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END
