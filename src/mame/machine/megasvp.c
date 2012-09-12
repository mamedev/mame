/* Megadrive SVP emulation (Virtua Racing) */

#include "includes/megadriv.h"



/* SVP (virtua racing) */
cpu_device *_svp_cpu;


/****************************************** SVP related *****************************************/

/*
 * Emulator of memory controller in SVP chip
 *
 * Copyright 2008, Grazvydas Ignotas
 * based on RE work by Tasco Deluxe
 *
 * SSP1601 EXT registers are mapped as I/O ports due to their function
 * (they are interfaced through external bus), and are named as follows
 * (these are unofficial names, official ones are unknown):
 *   EXT0: PM0 - programmable register 0
 *   EXT1: PM1 - ... 1
 *   EXT2: PM2 - ... 2
 *   EXT3: XST - external status. Can also act as PM.
 *   EXT4: PM4 - ... 4
 *   EXT5: (unused)
 *   EXT6: PMC - programmable memory register control (PMAC).
 *   EXT7: AL  - although internal to SSP1601, it still causes bus access
 *
 * Depending on GPO bits in status register, PM0, PM1, PM2 and XST can act as
 * external status registers, os as programmable memory registers. PM4 always
 * acts as PM register (independent on GPO bits).
 */


#define SSP_PMC_HAVE_ADDR  1  // address written to PMAC, waiting for mode
#define SSP_PMC_SET        2  // PMAC is set, PMx can be programmed

static int get_inc(int mode)
{
	int inc = (mode >> 11) & 7;
	if (inc != 0) {
		if (inc != 7) inc--;
		inc = 1 << inc; // 0 1 2 4 8 16 32 128
		if (mode & 0x8000) inc = -inc; // decrement mode
	}
	return inc;
}

INLINE void overwrite_write(UINT16 *dst, UINT16 d)
{
	if (d & 0xf000) { *dst &= ~0xf000; *dst |= d & 0xf000; }
	if (d & 0x0f00) { *dst &= ~0x0f00; *dst |= d & 0x0f00; }
	if (d & 0x00f0) { *dst &= ~0x00f0; *dst |= d & 0x00f0; }
	if (d & 0x000f) { *dst &= ~0x000f; *dst |= d & 0x000f; }
}

static UINT32 pm_io(address_space *space, int reg, int write, UINT32 d)
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	if (state->m_emu_status & SSP_PMC_SET)
	{
		state->m_pmac_read[write ? reg + 6 : reg] = state->m_pmc.d;
		state->m_emu_status &= ~SSP_PMC_SET;
		return 0;
	}

	// just in case
	if (state->m_emu_status & SSP_PMC_HAVE_ADDR) {
		state->m_emu_status &= ~SSP_PMC_HAVE_ADDR;
	}

	if (reg == 4 || (space->device().state().state_int(SSP_ST) & 0x60))
	{
		#define CADDR ((((mode<<16)&0x7f0000)|addr)<<1)
		UINT16 *dram = (UINT16 *)state->m_dram;
		if (write)
		{
			int mode = state->m_pmac_write[reg]>>16;
			int addr = state->m_pmac_write[reg]&0xffff;
			if      ((mode & 0x43ff) == 0x0018) // DRAM
			{
				int inc = get_inc(mode);
				if (mode & 0x0400) {
				       overwrite_write(&dram[addr], d);
				} else dram[addr] = d;
				state->m_pmac_write[reg] += inc;
			}
			else if ((mode & 0xfbff) == 0x4018) // DRAM, cell inc
			{
				if (mode & 0x0400) {
				       overwrite_write(&dram[addr], d);
				} else dram[addr] = d;
				state->m_pmac_write[reg] += (addr&1) ? 31 : 1;
			}
			else if ((mode & 0x47ff) == 0x001c) // IRAM
			{
				int inc = get_inc(mode);
				((UINT16 *)state->m_iram)[addr&0x3ff] = d;
				state->m_pmac_write[reg] += inc;
			}
			else
			{
				logerror("ssp FIXME: PM%i unhandled write mode %04x, [%06x] %04x\n",
						reg, mode, CADDR, d);
			}
		}
		else
		{
			int mode = state->m_pmac_read[reg]>>16;
			int addr = state->m_pmac_read[reg]&0xffff;
			if      ((mode & 0xfff0) == 0x0800) // ROM, inc 1, verified to be correct
			{
				UINT16 *ROM = (UINT16 *) space->machine().root_device().memregion("maincpu")->base();
				state->m_pmac_read[reg] += 1;
				d = ROM[addr|((mode&0xf)<<16)];
			}
			else if ((mode & 0x47ff) == 0x0018) // DRAM
			{
				int inc = get_inc(mode);
				d = dram[addr];
				state->m_pmac_read[reg] += inc;
			}
			else
			{
				logerror("ssp FIXME: PM%i unhandled read  mode %04x, [%06x]\n",
						reg, mode, CADDR);
				d = 0;
			}
		}

		// PMC value corresponds to last PMR accessed (not sure).
		state->m_pmc.d = state->m_pmac_read[write ? reg + 6 : reg];

		return d;
	}

	return (UINT32)-1;
}

static READ16_HANDLER( read_PM0 )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 d = pm_io(space, 0, 0, 0);
	if (d != (UINT32)-1) return d;
	d = state->m_XST2;
	state->m_XST2 &= ~2; // ?
	return d;
}

static WRITE16_HANDLER( write_PM0 )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 r = pm_io(space, 0, 1, data);
	if (r != (UINT32)-1) return;
	state->m_XST2 = data; // ?
}

static READ16_HANDLER( read_PM1 )
{
	UINT32 r = pm_io(space, 1, 0, 0);
	if (r != (UINT32)-1) return r;
	logerror("svp: PM1 acces in non PM mode?\n");
	return 0;
}

static WRITE16_HANDLER( write_PM1 )
{
	UINT32 r = pm_io(space, 1, 1, data);
	if (r != (UINT32)-1) return;
	logerror("svp: PM1 acces in non PM mode?\n");
}

static READ16_HANDLER( read_PM2 )
{
	UINT32 r = pm_io(space, 2, 0, 0);
	if (r != (UINT32)-1) return r;
	logerror("svp: PM2 acces in non PM mode?\n");
	return 0;
}

static WRITE16_HANDLER( write_PM2 )
{
	UINT32 r = pm_io(space, 2, 1, data);
	if (r != (UINT32)-1) return;
	logerror("svp: PM2 acces in non PM mode?\n");
}

static READ16_HANDLER( read_XST )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 d = pm_io(space, 3, 0, 0);
	if (d != (UINT32)-1) return d;

	return state->m_XST;
}

static WRITE16_HANDLER( write_XST )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 r = pm_io(space, 3, 1, data);
	if (r != (UINT32)-1) return;

	state->m_XST2 |= 1;
	state->m_XST = data;
}

static READ16_HANDLER( read_PM4 )
{
	return pm_io(space, 4, 0, 0);
}

static WRITE16_HANDLER( write_PM4 )
{
	pm_io(space, 4, 1, data);
}

static READ16_HANDLER( read_PMC )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	if (state->m_emu_status & SSP_PMC_HAVE_ADDR) {
		state->m_emu_status |= SSP_PMC_SET;
		state->m_emu_status &= ~SSP_PMC_HAVE_ADDR;
		return ((state->m_pmc.w.l << 4) & 0xfff0) | ((state->m_pmc.w.l >> 4) & 0xf);
	} else {
		state->m_emu_status |= SSP_PMC_HAVE_ADDR;
		return state->m_pmc.w.l;
	}
}

static WRITE16_HANDLER( write_PMC )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	if (state->m_emu_status & SSP_PMC_HAVE_ADDR) {
		state->m_emu_status |= SSP_PMC_SET;
		state->m_emu_status &= ~SSP_PMC_HAVE_ADDR;
		state->m_pmc.w.h = data;
	} else {
		state->m_emu_status |= SSP_PMC_HAVE_ADDR;
		state->m_pmc.w.l = data;
	}
}

static READ16_HANDLER( read_AL )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	state->m_emu_status &= ~(SSP_PMC_SET|SSP_PMC_HAVE_ADDR);
	return 0;
}

static WRITE16_HANDLER( write_AL )
{
}



static READ16_HANDLER( svp_68k_io_r )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 d;
	switch (offset)
	{
		// 0xa15000, 0xa15002
		case 0:
		case 1:  return state->m_XST;
		// 0xa15004
		case 2:  d = state->m_XST2; state->m_XST2 &= ~1; return d;
		default: logerror("unhandled SVP reg read @ %x\n", offset<<1);
	}
	return 0;
}

static WRITE16_HANDLER( svp_68k_io_w )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	switch (offset)
	{
		// 0xa15000, 0xa15002
		case 0:
		case 1:  state->m_XST = data; state->m_XST2 |= 2; break;
		// 0xa15006
		case 3:  break; // possibly halts SSP1601
		default: logerror("unhandled SVP reg write %04x @ %x\n", data, offset<<1);
	}
}

static READ16_HANDLER( svp_68k_cell1_r )
{
	// this is rewritten 68k test code
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 a1 = offset;
	a1 = (a1 & 0x7001) | ((a1 & 0x3e) << 6) | ((a1 & 0xfc0) >> 5);
	return ((UINT16 *)state->m_dram)[a1];
}

static READ16_HANDLER( svp_68k_cell2_r )
{
	// this is rewritten 68k test code
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 a1 = offset;
	a1 = (a1 & 0x7801) | ((a1 & 0x1e) << 6) | ((a1 & 0x7e0) >> 4);
	return ((UINT16 *)state->m_dram)[a1];
}

ADDRESS_MAP_START( svp_ssp_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x0000, 0x03ff) AM_ROMBANK("bank3")
	AM_RANGE(0x0400, 0xffff) AM_ROMBANK("bank4")
ADDRESS_MAP_END

ADDRESS_MAP_START( svp_ext_map, AS_IO, 16, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0xf)
	AM_RANGE(0*2, 0*2+1) AM_READWRITE_LEGACY(read_PM0, write_PM0)
	AM_RANGE(1*2, 1*2+1) AM_READWRITE_LEGACY(read_PM1, write_PM1)
	AM_RANGE(2*2, 2*2+1) AM_READWRITE_LEGACY(read_PM2, write_PM2)
	AM_RANGE(3*2, 3*2+1) AM_READWRITE_LEGACY(read_XST, write_XST)
	AM_RANGE(4*2, 4*2+1) AM_READWRITE_LEGACY(read_PM4, write_PM4)
	AM_RANGE(6*2, 6*2+1) AM_READWRITE_LEGACY(read_PMC, write_PMC)
	AM_RANGE(7*2, 7*2+1) AM_READWRITE_LEGACY(read_AL, write_AL)
ADDRESS_MAP_END


/* emulate testmode plug */
static UINT8 megadrive_io_read_data_port_svp(running_machine &machine, int portnum)
{
	if (portnum == 0 && machine.root_device().ioport("MEMORY_TEST")->read_safe(0x00))
	{
		return (megadrive_io_data_regs[0] & 0xc0);
	}
	return megadrive_io_read_data_port_3button(machine, portnum);
}


static READ16_HANDLER( svp_speedup_r )
{
	 space->device().execute().spin_until_time(attotime::from_usec(100));
	return 0x0425;
}


void svp_init(running_machine &machine)
{
	mdsvp_state *state = machine.driver_data<mdsvp_state>();
	UINT8 *ROM;

	memset(state->m_pmac_read, 0, ARRAY_LENGTH(state->m_pmac_read));
	memset(state->m_pmac_write, 0, ARRAY_LENGTH(state->m_pmac_write));
	state->m_pmc.d = 0;
	state->m_pmc.w.l = 0;
	state->m_pmc.w.h = 0;
	state->m_emu_status = 0;
	state->m_XST = 0;
	state->m_XST2 = 0;

	/* SVP stuff */
	state->m_dram = auto_alloc_array(machine, UINT8, 0x20000);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_ram(0x300000, 0x31ffff, state->m_dram);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15000, 0xa150ff, FUNC(svp_68k_io_r), FUNC(svp_68k_io_w));
	// "cell arrange" 1 and 2
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x390000, 0x39ffff, FUNC(svp_68k_cell1_r));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x3a0000, 0x3affff, FUNC(svp_68k_cell2_r));

	machine.device("svp")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x438, 0x438, FUNC(svp_speedup_r));

	state->m_iram = auto_alloc_array(machine, UINT8, 0x800);
	state->membank("bank3")->set_base(state->m_iram);
	/* SVP ROM just shares m68k region.. */
	ROM = state->memregion("maincpu")->base();
	state->membank("bank4")->set_base(ROM + 0x800);

	megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_svp;
}
