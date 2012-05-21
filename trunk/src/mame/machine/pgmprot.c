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

static void asic3_compute_hold(running_machine &machine)
{
	pgm_asic3_state *state = machine.driver_data<pgm_asic3_state>();

	// The mode is dependent on the region
	static const int modes[4] = { 1, 1, 3, 2 };
	int mode = modes[machine.root_device().ioport("Region")->read() & 3];

	switch (mode)
	{
	case 1:
		state->m_asic3_hold =
			(state->m_asic3_hold << 1)
			 ^ 0x2bad
			 ^ BIT(state->m_asic3_hold, 15) ^ BIT(state->m_asic3_hold, 10) ^ BIT(state->m_asic3_hold, 8) ^ BIT(state->m_asic3_hold, 5)
			 ^ BIT(state->m_asic3_z, state->m_asic3_y)
			 ^ (BIT(state->m_asic3_x, 0) << 1) ^ (BIT(state->m_asic3_x, 1) << 6) ^ (BIT(state->m_asic3_x, 2) << 10) ^ (BIT(state->m_asic3_x, 3) << 14);
		break;
	case 2:
		state->m_asic3_hold =
			(state->m_asic3_hold << 1)
			 ^ 0x2bad
			 ^ BIT(state->m_asic3_hold, 15) ^ BIT(state->m_asic3_hold, 7) ^ BIT(state->m_asic3_hold, 6) ^ BIT(state->m_asic3_hold, 5)
			 ^ BIT(state->m_asic3_z, state->m_asic3_y)
			 ^ (BIT(state->m_asic3_x, 0) << 4) ^ (BIT(state->m_asic3_x, 1) << 6) ^ (BIT(state->m_asic3_x, 2) << 10) ^ (BIT(state->m_asic3_x, 3) << 12);
		break;
	case 3:
		state->m_asic3_hold =
			(state->m_asic3_hold << 1)
			 ^ 0x2bad
			 ^ BIT(state->m_asic3_hold, 15) ^ BIT(state->m_asic3_hold, 10) ^ BIT(state->m_asic3_hold, 8) ^ BIT(state->m_asic3_hold, 5)
			 ^ BIT(state->m_asic3_z, state->m_asic3_y)
			 ^ (BIT(state->m_asic3_x, 0) << 4) ^ (BIT(state->m_asic3_x, 1) << 6) ^ (BIT(state->m_asic3_x, 2) << 10) ^ (BIT(state->m_asic3_x, 3) << 12);
		break;
	}
}

READ16_HANDLER( pgm_asic3_r )
{
	pgm_asic3_state *state = space->machine().driver_data<pgm_asic3_state>();
	UINT8 res = 0;
	/* region is supplied by the protection device */

	switch (state->m_asic3_reg)
	{
	case 0x00: res = (state->m_asic3_latch[0] & 0xf7) | ((state->ioport("Region")->read() << 3) & 0x08); break;
	case 0x01: res = state->m_asic3_latch[1]; break;
	case 0x02: res = (state->m_asic3_latch[2] & 0x7f) | ((state->ioport("Region")->read() << 6) & 0x80); break;
	case 0x03:
		res = (BIT(state->m_asic3_hold, 15) << 0)
			| (BIT(state->m_asic3_hold, 12) << 1)
			| (BIT(state->m_asic3_hold, 13) << 2)
			| (BIT(state->m_asic3_hold, 10) << 3)
			| (BIT(state->m_asic3_hold, 7) << 4)
			| (BIT(state->m_asic3_hold, 9) << 5)
			| (BIT(state->m_asic3_hold, 2) << 6)
			| (BIT(state->m_asic3_hold, 5) << 7);
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

WRITE16_HANDLER( pgm_asic3_w )
{
	pgm_asic3_state *state = space->machine().driver_data<pgm_asic3_state>();

	if(ACCESSING_BITS_0_7)
	{
		if (state->m_asic3_reg < 3)
			state->m_asic3_latch[state->m_asic3_reg] = data << 1;
		else if (state->m_asic3_reg == 0xa0)
			state->m_asic3_hold = 0;
		else if (state->m_asic3_reg == 0x40)
		{
			state->m_asic3_h2 = state->m_asic3_h1;
			state->m_asic3_h1 = data;
		}
		else if (state->m_asic3_reg == 0x48)
		{
			state->m_asic3_x = 0;
			if (!(state->m_asic3_h2 & 0x0a))
				state->m_asic3_x |= 8;
			if (!(state->m_asic3_h2 & 0x90))
				state->m_asic3_x |= 4;
			if (!(state->m_asic3_h1 & 0x06))
				state->m_asic3_x |= 2;
			if (!(state->m_asic3_h1 & 0x90))
				state->m_asic3_x |= 1;
		}
		else if(state->m_asic3_reg >= 0x80 && state->m_asic3_reg <= 0x87)
		{
			state->m_asic3_y = state->m_asic3_reg & 7;
			state->m_asic3_z = data;
			asic3_compute_hold(space->machine());
		}
	}
}

WRITE16_HANDLER( pgm_asic3_reg_w )
{
	pgm_asic3_state *state = space->machine().driver_data<pgm_asic3_state>();

	if(ACCESSING_BITS_0_7)
		state->m_asic3_reg = data & 0xff;
}





/* Oriental Legend INIT */

DRIVER_INIT( orlegend )
{
	pgm_asic3_state *state = machine.driver_data<pgm_asic3_state>();
	pgm_basic_init(machine);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xC0400e, 0xC0400f, FUNC(pgm_asic3_r), FUNC(pgm_asic3_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xC04000, 0xC04001, FUNC(pgm_asic3_reg_w));

	state->m_asic3_reg = 0;
	state->m_asic3_latch[0] = 0;
	state->m_asic3_latch[1] = 0;
	state->m_asic3_latch[2] = 0;
	state->m_asic3_x = 0;
	state->m_asic3_y = 0;
	state->m_asic3_z = 0;
	state->m_asic3_h1 = 0;
	state->m_asic3_h2 = 0;
	state->m_asic3_hold = 0;

	state->save_item(NAME(state->m_asic3_reg));
	state->save_item(NAME(state->m_asic3_latch));
	state->save_item(NAME(state->m_asic3_x));
	state->save_item(NAME(state->m_asic3_y));
	state->save_item(NAME(state->m_asic3_z));
	state->save_item(NAME(state->m_asic3_h1));
	state->save_item(NAME(state->m_asic3_h2));
	state->save_item(NAME(state->m_asic3_hold));
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
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Unused ) )	// region switch
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )		// if enabled, game gives
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )		// "incorrect version" error
INPUT_PORTS_END

MACHINE_CONFIG_START( pgm_asic3, pgm_asic3_state )
	MCFG_FRAGMENT_ADD(pgmbase)
MACHINE_CONFIG_END
