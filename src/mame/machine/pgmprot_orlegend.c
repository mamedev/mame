/***********************************************************************
 PGM ASIC3 PGM protection emulation

 this seems similar to the IGS025? Is the physical chip ASIC3, or is
 that just what the game calls it?

 Used by:

 Oriental Legend

 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"

/*** ASIC 3 (oriental legends protection) ****************************************/

void pgm_asic3_state::asic3_compute_hold()
{
	// The mode is dependent on the region
	static const int modes[4] = { 1, 1, 3, 2 };
	int mode = modes[ioport("Region")->read() & 3];

	switch (mode)
	{
	case 1:
		m_asic3_hold =
			(m_asic3_hold << 1)
				^ 0x2bad
				^ BIT(m_asic3_hold, 15) ^ BIT(m_asic3_hold, 10) ^ BIT(m_asic3_hold, 8) ^ BIT(m_asic3_hold, 5)
				^ BIT(m_asic3_z, m_asic3_y)
				^ (BIT(m_asic3_x, 0) << 1) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 2) << 10) ^ (BIT(m_asic3_x, 3) << 14);
		break;
	case 2:
		m_asic3_hold =
			(m_asic3_hold << 1)
				^ 0x2bad
				^ BIT(m_asic3_hold, 15) ^ BIT(m_asic3_hold, 7) ^ BIT(m_asic3_hold, 6) ^ BIT(m_asic3_hold, 5)
				^ BIT(m_asic3_z, m_asic3_y)
				^ (BIT(m_asic3_x, 0) << 4) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 2) << 10) ^ (BIT(m_asic3_x, 3) << 12);
		break;
	case 3:
		m_asic3_hold =
			(m_asic3_hold << 1)
				^ 0x2bad
				^ BIT(m_asic3_hold, 15) ^ BIT(m_asic3_hold, 10) ^ BIT(m_asic3_hold, 8) ^ BIT(m_asic3_hold, 5)
				^ BIT(m_asic3_z, m_asic3_y)
				^ (BIT(m_asic3_x, 0) << 4) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 2) << 10) ^ (BIT(m_asic3_x, 3) << 12);
		break;
	}
}

READ16_MEMBER(pgm_asic3_state::pgm_asic3_r )
{
	UINT8 res = 0;
	/* region is supplied by the protection device */

	switch (m_asic3_reg)
	{
	case 0x00: res = (m_asic3_latch[0] & 0xf7) | ((ioport("Region")->read() << 3) & 0x08); break;
	case 0x01: res = m_asic3_latch[1]; break;
	case 0x02: res = (m_asic3_latch[2] & 0x7f) | ((ioport("Region")->read() << 6) & 0x80); break;
	case 0x03:
		res = (BIT(m_asic3_hold, 15) << 0)
			| (BIT(m_asic3_hold, 12) << 1)
			| (BIT(m_asic3_hold, 13) << 2)
			| (BIT(m_asic3_hold, 10) << 3)
			| (BIT(m_asic3_hold, 7) << 4)
			| (BIT(m_asic3_hold, 9) << 5)
			| (BIT(m_asic3_hold, 2) << 6)
			| (BIT(m_asic3_hold, 5) << 7);
		break;
	case 0x20: res = 0x49; break;
	case 0x21: res = 0x47; break;
	case 0x22: res = 0x53; break;
	case 0x24: res = 0x41; break;
	case 0x25: res = 0x41; break;
	case 0x26: res = 0x7f; break;
	case 0x27: res = 0x41; break;
	case 0x28: res = 0x41; break;
	case 0x2a: res = 0x3e; break;
	case 0x2b: res = 0x41; break;
	case 0x2c: res = 0x49; break;
	case 0x2d: res = 0xf9; break;
	case 0x2e: res = 0x0a; break;
	case 0x30: res = 0x26; break;
	case 0x31: res = 0x49; break;
	case 0x32: res = 0x49; break;
	case 0x33: res = 0x49; break;
	case 0x34: res = 0x32; break;
	}

	return res;
}

WRITE16_MEMBER(pgm_asic3_state::pgm_asic3_w )
{
	if(ACCESSING_BITS_0_7)
	{
		if (m_asic3_reg < 3)
			m_asic3_latch[m_asic3_reg] = data << 1;
		else if (m_asic3_reg == 0xa0)
			m_asic3_hold = 0;
		else if (m_asic3_reg == 0x40)
		{
			m_asic3_h2 = m_asic3_h1;
			m_asic3_h1 = data;
		}
		else if (m_asic3_reg == 0x48)
		{
			m_asic3_x = 0;
			if (!(m_asic3_h2 & 0x0a))
				m_asic3_x |= 8;
			if (!(m_asic3_h2 & 0x90))
				m_asic3_x |= 4;
			if (!(m_asic3_h1 & 0x06))
				m_asic3_x |= 2;
			if (!(m_asic3_h1 & 0x90))
				m_asic3_x |= 1;
		}
		else if(m_asic3_reg >= 0x80 && m_asic3_reg <= 0x87)
		{
			m_asic3_y = m_asic3_reg & 7;
			m_asic3_z = data;
			asic3_compute_hold();
		}
	}
}

WRITE16_MEMBER(pgm_asic3_state::pgm_asic3_reg_w )
{
	if(ACCESSING_BITS_0_7)
		m_asic3_reg = data & 0xff;
}





/* Oriental Legend INIT */

DRIVER_INIT_MEMBER(pgm_asic3_state,orlegend)
{
	pgm_basic_init();

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xC0400e, 0xC0400f, read16_delegate(FUNC(pgm_asic3_state::pgm_asic3_r),this), write16_delegate(FUNC(pgm_asic3_state::pgm_asic3_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xC04000, 0xC04001, write16_delegate(FUNC(pgm_asic3_state::pgm_asic3_reg_w),this));

	m_asic3_reg = 0;
	m_asic3_latch[0] = 0;
	m_asic3_latch[1] = 0;
	m_asic3_latch[2] = 0;
	m_asic3_x = 0;
	m_asic3_y = 0;
	m_asic3_z = 0;
	m_asic3_h1 = 0;
	m_asic3_h2 = 0;
	m_asic3_hold = 0;

	save_item(NAME(m_asic3_reg));
	save_item(NAME(m_asic3_latch));
	save_item(NAME(m_asic3_x));
	save_item(NAME(m_asic3_y));
	save_item(NAME(m_asic3_z));
	save_item(NAME(m_asic3_h1));
	save_item(NAME(m_asic3_h2));
	save_item(NAME(m_asic3_hold));
}


INPUT_PORTS_START( orlegend )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0001, "World (duplicate)" ) // again?
	PORT_CONFSETTING(      0x0002, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( China ) )
INPUT_PORTS_END

INPUT_PORTS_START( orld105k )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Unused ) )   // region switch
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )      // if enabled, game gives
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )       // "incorrect version" error
INPUT_PORTS_END

MACHINE_CONFIG_START( pgm_asic3, pgm_asic3_state )
	MCFG_FRAGMENT_ADD(pgmbase)
MACHINE_CONFIG_END
