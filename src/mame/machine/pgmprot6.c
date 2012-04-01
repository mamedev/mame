/***********************************************************************
 PGM 012 + 28 PGM protection emulation

 these are simulations of the IGS 012 and 025 protection combination
 used on the following PGM games

 Oriental Legend Super

 ----

 IGS28 is some kind of encrypted DMA device, works with data in an
 external ROM, more advaned version of IGS022?

 IGS025 is some kind of state machine, bitswaps etc.

 Simulation is incomplete

 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"

static UINT32 olds_prot_addr( UINT16 addr )
{
	UINT32 mode = addr & 0xff;
	UINT32 offset = addr >> 8;
	UINT32 realaddr;

	switch(mode)
	{
		case 0x0:
		case 0x5:
		case 0xa:
			realaddr = 0x402a00 + (offset << 2);
			break;

		case 0x2:
		case 0x8:
			realaddr = 0x402e00 + (offset << 2);
			break;

		case 0x1:
			realaddr = 0x40307e;
			break;

		case 0x3:
			realaddr = 0x403090;
			break;

		case 0x4:
			realaddr = 0x40309a;
			break;

		case 0x6:
			realaddr = 0x4030a4;
			break;

		case 0x7:
			realaddr = 0x403000;
			break;

		case 0x9:
			realaddr = 0x40306e;
			break;

		default:
			realaddr = 0;
	}
	return realaddr;
}

static UINT32 olds_read_reg( running_machine &machine, UINT16 addr )
{
	pgm_028_025_state *state = machine.driver_data<pgm_028_025_state>();
	UINT32 protaddr = (olds_prot_addr(addr) - 0x400000) / 2;
	return state->m_sharedprotram[protaddr] << 16 | state->m_sharedprotram[protaddr + 1];
}

static void olds_write_reg( running_machine &machine, UINT16 addr, UINT32 val )
{
	pgm_028_025_state *state = machine.driver_data<pgm_028_025_state>();
	state->m_sharedprotram[(olds_prot_addr(addr) - 0x400000) / 2]     = val >> 16;
	state->m_sharedprotram[(olds_prot_addr(addr) - 0x400000) / 2 + 1] = val & 0xffff;
}

static MACHINE_RESET( olds )
{
	pgm_028_025_state *state = machine.driver_data<pgm_028_025_state>();
	UINT16 *mem16 = (UINT16 *)machine.region("user2")->base();
	int i;

	MACHINE_RESET_CALL(pgm);

	/* populate shared protection ram with data read from pcb .. */
	for (i = 0; i < 0x4000 / 2; i++)
	{
		state->m_sharedprotram[i] = mem16[i];
	}

	//ROM:004008B4                 .word 0xFBA5
	for(i = 0; i < 0x4000 / 2; i++)
	{
		if (state->m_sharedprotram[i] == (0xffff - i))
			state->m_sharedprotram[i] = 0x4e75;
	}
}

static READ16_HANDLER( olds_r )
{
	pgm_028_025_state *state = space->machine().driver_data<pgm_028_025_state>();
	UINT16 res = 0;

	if (offset == 1)
	{
		if (state->m_kb_cmd == 1)
			res = state->m_kb_reg & 0x7f;
		if (state->m_kb_cmd == 2)
			res = state->m_olds_bs | 0x80;
		if (state->m_kb_cmd == 3)
			res = state->m_olds_cmd3;
		else if (state->m_kb_cmd == 5)
		{
			UINT32 protvalue = 0x900000 | input_port_read(space->machine(), "Region"); // region from protection device.
			res = (protvalue >> (8 * (state->m_kb_ptr - 1))) & 0xff; // includes region 1 = taiwan , 2 = china, 3 = japan (title = orlegend special), 4 = korea, 5 = hongkong, 6 = world

		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X\n", cpu_get_pc(&space->device()), state->m_kb_cmd, res);
	return res;
}

static WRITE16_HANDLER( olds_w )
{
	pgm_028_025_state *state = space->machine().driver_data<pgm_028_025_state>();
	if (offset == 0)
		state->m_kb_cmd = data;
	else //offset==2
	{
		logerror("%06X: ASIC25 W CMD %X  VAL %X\n",cpu_get_pc(&space->device()), state->m_kb_cmd, data);
		if (state->m_kb_cmd == 0)
			state->m_kb_reg = data;
		else if(state->m_kb_cmd == 2)	//a bitswap=
		{
			int reg = 0;
			if (data & 0x01)
				reg |= 0x40;
			if (data & 0x02)
				reg |= 0x80;
			if (data & 0x04)
				reg |= 0x20;
			if (data & 0x08)
				reg |= 0x10;
			state->m_olds_bs = reg;
		}
		else if (state->m_kb_cmd == 3)
		{
			UINT16 cmd = state->m_sharedprotram[0x3026 / 2];
			switch (cmd)
			{
				case 0x11:
				case 0x12:
						break;
				case 0x64:
					{
						UINT16 cmd0 = state->m_sharedprotram[0x3082 / 2];
						UINT16 val0 = state->m_sharedprotram[0x3050 / 2];	//CMD_FORMAT
						{
							if ((cmd0 & 0xff) == 0x2)
								olds_write_reg(space->machine(), val0, olds_read_reg(space->machine(), val0) + 0x10000);
						}
						break;
					}

				default:
						break;
			}
			state->m_olds_cmd3 = ((data >> 4) + 1) & 0x3;
		}
		else if (state->m_kb_cmd == 4)
			state->m_kb_ptr = data;
		else if(state->m_kb_cmd == 0x20)
		  state->m_kb_ptr++;
	}
}

static READ16_HANDLER( olds_prot_swap_r )
{
	if (cpu_get_pc(&space->device()) < 0x100000)		//bios
		return pgm_mainram[0x178f4 / 2];
	else						//game
		return pgm_mainram[0x178d8 / 2];

}

DRIVER_INIT( olds )
{
	pgm_028_025_state *state = machine.driver_data<pgm_028_025_state>();
	pgm_basic_init(machine);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xdcb400, 0xdcb403, FUNC(olds_r), FUNC(olds_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x8178f4, 0x8178f5, FUNC(olds_prot_swap_r));

	state->m_kb_cmd = 0;
	state->m_kb_reg = 0;
	state->m_kb_ptr = 0;
	state->m_olds_bs = 0;
	state->m_olds_cmd3 = 0;

	state->save_item(NAME(state->m_kb_cmd));
	state->save_item(NAME(state->m_kb_reg));
	state->save_item(NAME(state->m_kb_ptr));
	state->save_item(NAME(state->m_olds_bs));
	state->save_item(NAME(state->m_olds_cmd3));
}

static ADDRESS_MAP_START( olds_mem, AS_PROGRAM, 16, pgm_028_025_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x3fffff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0x400000, 0x403fff) AM_RAM AM_BASE( m_sharedprotram) // Shared with protection device
ADDRESS_MAP_END


MACHINE_CONFIG_START( pgm_028_025_ol, pgm_028_025_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(olds_mem)

	MCFG_MACHINE_RESET(olds)
MACHINE_CONFIG_END


INPUT_PORTS_START( olds )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0006, DEF_STR( Region ) )
	/* includes the following regions:
    1 = taiwan, 2 = china, 3 = japan (title = orlegend special),
    4 = korea, 5 = hong kong, 6 = world */
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( Japan ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0006, DEF_STR( World ) )
INPUT_PORTS_END
