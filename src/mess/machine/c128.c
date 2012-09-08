/***************************************************************************

    commodore c128 home computer

    peter.trauner@jk.uni-linz.ac.at
    documentation:
     iDOC (http://www.softwolves.pp.se/idoc)
           Christian Janoff  mepk@c64.org

***************************************************************************/

#include "emu.h"
#include "includes/c128.h"
#include "includes/c64_legacy.h"
#include "includes/cbm.h"
#include "machine/cbmiec.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/6526cia.h"
#include "sound/sid6581.h"
#include "video/mos6566.h"
#include "video/mc6845.h"

#define MMU_PAGE1 ((((state->m_mmu[10]&0xf)<<8)|state->m_mmu[9])<<8)
#define MMU_PAGE0 ((((state->m_mmu[8]&0xf)<<8)|state->m_mmu[7])<<8)
#define MMU_VIC_ADDR ((state->m_mmu[6]&0xc0)<<10)
#define MMU_RAM_RCR_ADDR ((state->m_mmu[6]&0x30)<<14)
#define MMU_SIZE (c128_mmu_helper[state->m_mmu[6]&3])
#define MMU_BOTTOM (state->m_mmu[6]&4)
#define MMU_TOP (state->m_mmu[6]&8)
#define MMU_CPU8502 (state->m_mmu[5]&1)	   /* else z80 */
/* fastio output (c128_mmu[5]&8) else input */
#define MMU_FSDIR(s) ((s)->m_mmu[5]&0x08)
#define MMU_GAME_IN (state->m_mmu[5]&0x10)
#define MMU_EXROM_IN (state->m_mmu[5]&0x20)
#define MMU_64MODE (state->m_mmu[5]&0x40)
#define MMU_40_IN (state->m_mmu[5]&0x80)

#define MMU_RAM_CR_ADDR ((state->m_mmu[0]&0xc0)<<10)
#define MMU_RAM_LO (state->m_mmu[0]&2)	   /* else rom at 0x4000 */
#define MMU_RAM_MID ((state->m_mmu[0]&0xc)==0xc)	/* 0x8000 - 0xbfff */
#define MMU_ROM_MID ((state->m_mmu[0]&0xc)==0)
#define MMU_EXTERNAL_ROM_MID ((state->m_mmu[0]&0xc)==8)
#define MMU_INTERNAL_ROM_MID ((state->m_mmu[0]&0xc)==4)

#define MMU_IO_ON (!(state->m_mmu[0]&1))   /* io window at 0xd000 */
#define MMU_ROM_HI ((state->m_mmu[0]&0x30)==0)	/* rom at 0xc000 */
#define MMU_EXTERNAL_ROM_HI ((state->m_mmu[0]&0x30)==0x20)
#define MMU_INTERNAL_ROM_HI ((state->m_mmu[0]&0x30)==0x10)
#define MMU_RAM_HI ((state->m_mmu[0]&0x30)==0x30)

#define MMU_RAM_ADDR (MMU_RAM_RCR_ADDR|MMU_RAM_CR_ADDR)

static const int c128_mmu_helper[4] =
{0x400, 0x1000, 0x2000, 0x4000};

#define VERBOSE_LEVEL 0
#define DBG_LOG( MACHINE, N, M, A ) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", MACHINE.time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)

/*
 two cpus share 1 memory system, only 1 cpu can run
 controller is the mmu

 mame has different memory subsystems for each cpu
*/


/* m8502 port
 in c128 mode
 bit 0 color
 bit 1 lores
 bit 2 hires
 3 textmode
 5 graphics (turned on ram at 0x1000 for video chip
*/







static void c128_nmi( running_machine &machine )
{
	c128_state *state = machine.driver_data<c128_state>();
	device_t *cia_1 = machine.device("cia_1");
	int cia1irq = mos6526_irq_r(cia_1);

	if (state->m_nmilevel != (machine.root_device().ioport("SPECIAL")->read() & 0x80) || cia1irq)	/* KEY_RESTORE */
	{
		if (1) // this was never valid, there is no active CPU during a timer firing!  cpu_getactivecpu() == 0)
		{
			/* z80 */
			cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, (machine.root_device().ioport("SPECIAL")->read() & 0x80) || cia1irq);
		}
		else
		{
			cputag_set_input_line(machine, "m8502", INPUT_LINE_NMI, (machine.root_device().ioport("SPECIAL")->read() & 0x80) || cia1irq);
		}

		state->m_nmilevel = (machine.root_device().ioport("SPECIAL")->read() & 0x80) || cia1irq;
	}
}

/***********************************************

    CIA Interfaces

***********************************************/

/*
 *  CIA 0 - Port A keyboard line select
 *  CIA 0 - Port B keyboard line read
 *
 *  flag cassette read input, serial request in
 *  irq to irq connected
 *
 *  see machine/cbm.c
 */

static READ8_DEVICE_HANDLER( c128_cia0_port_a_r )
{
	UINT8 cia0portb = mos6526_pb_r(device->machine().device("cia_0"), 0);

	return cbm_common_cia0_port_a_r(device, cia0portb);
}

static READ8_DEVICE_HANDLER( c128_cia0_port_b_r )
{
	c128_state *state = device->machine().driver_data<c128_state>();
	UINT8 value = 0xff;
	UINT8 cia0porta = mos6526_pa_r(device->machine().device("cia_0"), 0);
	device_t *vic2e = device->machine().device("vic2e");
	vic2e_device_interface *intf = dynamic_cast<vic2e_device_interface*>(vic2e);

	value &= cbm_common_cia0_port_b_r(device, cia0porta);

	if (!intf->k0_r())
		value &= state->m_keyline[0];
	if (!intf->k1_r())
		value &= state->m_keyline[1];
	if (!intf->k2_r())
		value &= state->m_keyline[2];

	return value;
}

static WRITE8_DEVICE_HANDLER( c128_cia0_port_b_w )
{
	mos6566_device *vic2e = device->machine().device<mos6566_device>("vic2e");

	vic2e->lp_w(BIT(data, 4));
}

static void c128_irq( running_machine &machine, int level )
{
	c128_state *state = machine.driver_data<c128_state>();
	if (level != state->m_old_level)
	{
		DBG_LOG(machine, 3, "mos6510", ("irq %s\n", level ? "start" : "end"));

		if (0) // && (cpu_getactivecpu() == 0))
		{
			cputag_set_input_line(machine, "maincpu", 0, level);
		}
		else
		{
			cputag_set_input_line(machine, "m8502", M6510_IRQ_LINE, level);
		}

		state->m_old_level = level;
	}
}

static void c128_cia0_interrupt( device_t *device, int level )
{
	c128_state *state = device->machine().driver_data<c128_state>();
	c128_irq(device->machine(), level || state->m_vicirq);
}

WRITE_LINE_MEMBER( c128_state::vic_interrupt )
{
	device_t *cia_0 = machine().device("cia_0");
#if 1
	if (state  != m_vicirq)
	{
		c128_irq (machine(), state || mos6526_irq_r(cia_0));
		m_vicirq = state;
	}
#endif
}

static void c128_iec_data_out_w(running_machine &machine)
{
	c128_state *state = machine.driver_data<c128_state>();
	int data = !state->m_data_out;

	/* fast serial data */
	if (MMU_FSDIR(state)) data &= state->m_sp1;

	state->m_iec->data_w(data);
}

static void c128_iec_srq_out_w(running_machine &machine)
{
	c128_state *state = machine.driver_data<c128_state>();
	int srq = 1;

	/* fast serial clock */
	if (MMU_FSDIR(state)) srq &= state->m_cnt1;

	state->m_iec->srq_w(srq);
}

static WRITE_LINE_DEVICE_HANDLER( cia0_cnt_w )
{
	c128_state *drvstate = device->machine().driver_data<c128_state>();
	/* fast clock out */
	drvstate->m_cnt1 = state;
	c128_iec_srq_out_w(device->machine());
}

static WRITE_LINE_DEVICE_HANDLER( cia0_sp_w )
{
	c128_state *drvstate = device->machine().driver_data<c128_state>();
	/* fast data out */
	drvstate->m_sp1 = state;
	c128_iec_data_out_w(device->machine());
}

const mos6526_interface c128_ntsc_cia0 =
{
	60,
	DEVCB_LINE(c128_cia0_interrupt),
	DEVCB_NULL,	/* pc_func */
	DEVCB_LINE(cia0_cnt_w),
	DEVCB_LINE(cia0_sp_w),
	DEVCB_HANDLER(c128_cia0_port_a_r),
	DEVCB_NULL,
	DEVCB_HANDLER(c128_cia0_port_b_r),
	DEVCB_HANDLER(c128_cia0_port_b_w)
};

const mos6526_interface c128_pal_cia0 =
{
	50,
	DEVCB_LINE(c128_cia0_interrupt),
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(c128_cia0_port_a_r),
	DEVCB_NULL,
	DEVCB_HANDLER(c128_cia0_port_b_r),
	DEVCB_HANDLER(c128_cia0_port_b_w)
};

WRITE_LINE_DEVICE_HANDLER( c128_iec_srq_w )
{
	c128_state *drvstate = device->machine().driver_data<c128_state>();

	if (!MMU_FSDIR(drvstate))
	{
		mos6526_flag_w(device, state);
		mos6526_cnt_w(device, state);
	}
}

WRITE_LINE_DEVICE_HANDLER( c128_iec_data_w )
{
	c128_state *drvstate = device->machine().driver_data<c128_state>();

	if (!MMU_FSDIR(drvstate))
	{
		mos6526_sp_w(device, state);
	}
}

/*
 * CIA 1 - Port A
 * bit 7 serial bus data input
 * bit 6 serial bus clock input
 * bit 5 serial bus data output
 * bit 4 serial bus clock output
 * bit 3 serial bus atn output
 * bit 2 rs232 data output
 * bits 1-0 vic-chip system memory bank select
 *
 * CIA 1 - Port B
 * bit 7 user rs232 data set ready
 * bit 6 user rs232 clear to send
 * bit 5 user
 * bit 4 user rs232 carrier detect
 * bit 3 user rs232 ring indicator
 * bit 2 user rs232 data terminal ready
 * bit 1 user rs232 request to send
 * bit 0 user rs232 received data
 *
 * flag restore key or rs232 received data input
 * irq to nmi connected ?
 */
static READ8_DEVICE_HANDLER( c128_cia1_port_a_r )
{
	c128_state *state = device->machine().driver_data<c128_state>();
	UINT8 value = 0xff;

	if (!state->m_iec->clk_r())
		value &= ~0x40;

	if (!state->m_iec->data_r())
		value &= ~0x80;

	return value;
}

static WRITE8_DEVICE_HANDLER( c128_cia1_port_a_w )
{
	c128_state *state = device->machine().driver_data<c128_state>();
	static const int helper[4] = {0xc000, 0x8000, 0x4000, 0x0000};

	state->m_data_out = BIT(data, 5);
	c128_iec_data_out_w(device->machine());

	state->m_iec->clk_w(!BIT(data, 4));

	state->m_iec->atn_w(!BIT(data, 3));

	state->m_vicaddr = state->m_memory + helper[data & 0x03];
	state->m_c128_vicaddr = state->m_memory + helper[data & 0x03] + state->m_va1617;
}

static void c128_cia1_interrupt( device_t *device, int level )
{
	c128_nmi(device->machine());
}

const mos6526_interface c128_ntsc_cia1 =
{
	60,
	DEVCB_LINE(c128_cia1_interrupt),
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(c128_cia1_port_a_r),
	DEVCB_HANDLER(c128_cia1_port_a_w),
	DEVCB_NULL,
	DEVCB_NULL
};

const mos6526_interface c128_pal_cia1 =
{
	50,
	DEVCB_LINE(c128_cia1_interrupt),
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(c128_cia1_port_a_r),
	DEVCB_HANDLER(c128_cia1_port_a_w),
	DEVCB_NULL,
	DEVCB_NULL
};

/***********************************************

    Memory Handlers

***********************************************/
static WRITE8_HANDLER( c128_dma8726_port_w )
{
	DBG_LOG(space->machine(), 1, "dma write", ("%.3x %.2x\n",offset,data));
}

static READ8_HANDLER( c128_dma8726_port_r )
{
	DBG_LOG(space->machine(), 1, "dma read", ("%.3x\n",offset));
	return 0xff;
}

WRITE8_HANDLER( c128_write_d000 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	device_t *cia_0 = space->machine().device("cia_0");
	device_t *cia_1 = space->machine().device("cia_1");
	device_t *sid = space->machine().device("sid6581");
	mos6566_device *vic2e = space->machine().device<mos6566_device>("vic2e");
	mos8563_device *vdc8563 = space->machine().device<mos8563_device>("vdc8563");

	UINT8 c64_port6510 = m6510_get_port(space->machine().device<legacy_cpu_device>("m8502"));

	if (!state->m_write_io)
	{
		if (offset + 0xd000 >= state->m_ram_top)
			state->m_memory[0xd000 + offset] = data;
		else
			state->m_ram[0xd000 + offset] = data;
	}
	else
	{
		switch ((offset&0xf00)>>8)
		{
		case 0:case 1: case 2: case 3:
			vic2e->write(*space, offset & 0x3f, data);
			break;
		case 4:
			sid6581_w(sid, offset & 0x3f, data);
			break;
		case 5:
			c128_mmu8722_port_w(space, offset & 0xff, data);
			break;
		case 6: case 7:
			if (offset & 0x01)
				vdc8563->register_w(*space, 0, data);
			else
				vdc8563->address_w(*space, 0, data);
			break;
		case 8: case 9: case 0xa: case 0xb:
		    if (state->m_c64mode)
				state->m_colorram[(offset & 0x3ff)] = data | 0xf0;
		    else
				state->m_colorram[(offset & 0x3ff)|((c64_port6510&3)<<10)] = data | 0xf0; // maybe all 8 bit connected!
		    break;
		case 0xc:
			mos6526_w(cia_0, offset, data);
			break;
		case 0xd:
			mos6526_w(cia_1, offset, data);
			break;
		case 0xf:
			c128_dma8726_port_w(space, offset&0xff,data);
			break;
		case 0xe:
			DBG_LOG(space->machine(), 1, "io write", ("%.3x %.2x\n", offset, data));
			break;
		}
	}
}



static READ8_HANDLER( c128_read_io )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	device_t *cia_0 = space->machine().device("cia_0");
	device_t *cia_1 = space->machine().device("cia_1");
	device_t *sid = space->machine().device("sid6581");
	mos6566_device *vic2e = space->machine().device<mos6566_device>("vic2e");
	mos8563_device *vdc8563 = space->machine().device<mos8563_device>("vdc8563");

	if (offset < 0x400)
		return vic2e->read(*space, offset & 0x3f);
	else if (offset < 0x500)
		return sid6581_r(sid, offset & 0xff);
	else if (offset < 0x600)
		return c128_mmu8722_port_r(space, offset & 0xff);
	else if (offset < 0x800)
			if (offset & 0x01)
				return vdc8563->register_r(*space, 0);
			else
				return vdc8563->status_r(*space, 0);
	else if (offset < 0xc00)
		return state->m_colorram[offset & 0x3ff];
	else if (offset == 0xc00)
		{
			cia_set_port_mask_value(cia_0, 0, state->ioport("CTRLSEL")->read() & 0x80 ? c64_keyline[8] : c64_keyline[9] );
			return mos6526_r(cia_0, offset);
		}
	else if (offset == 0xc01)
		{
			cia_set_port_mask_value(cia_0, 1, space->machine().root_device().ioport("CTRLSEL")->read() & 0x80 ? c64_keyline[9] : c64_keyline[8] );
			return mos6526_r(cia_0, offset);
		}
	else if (offset < 0xd00)
		return mos6526_r(cia_0, offset);
	else if (offset < 0xe00)
		return mos6526_r(cia_1, offset);
	else if ((offset >= 0xf00) & (offset <= 0xfff))
		return c128_dma8726_port_r(space, offset&0xff);
	DBG_LOG(space->machine(), 1, "io read", ("%.3x\n", offset));
	return 0xff;
}

void c128_bankswitch_64( running_machine &machine, int reset )
{
	c128_state *state = machine.driver_data<c128_state>();
	int data, loram, hiram, charen;

	if (!state->m_c64mode)
		return;

	data = m6510_get_port(machine.device<legacy_cpu_device>("m8502")) & 0x07;
	if ((state->m_old_data == data) && (state->m_old_exrom == state->m_exrom) && (state->m_old_game == state->m_game) && !reset)
		return;

	DBG_LOG(machine, 1, "bankswitch", ("%d\n", data & 7));
	loram = (data & 1) ? 1 : 0;
	hiram = (data & 2) ? 1 : 0;
	charen = (data & 4) ? 1 : 0;

	if ((!state->m_game && state->m_exrom) || (loram && hiram && !state->m_exrom))
		state->membank("bank8")->set_base(state->m_roml);
	else
		state->membank("bank8")->set_base(state->m_memory + 0x8000);

	if ((!state->m_game && state->m_exrom && hiram) || (!state->m_exrom))
		state->membank("bank9")->set_base(state->m_romh);
	else if (loram && hiram)
		state->membank("bank9")->set_base(state->m_basic);
	else
		state->membank("bank9")->set_base(state->m_memory + 0xa000);

	if ((!state->m_game && state->m_exrom) || (charen && (loram || hiram)))
	{
		machine.device("m8502")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xd000, 0xdfff, FUNC(c128_read_io));
		state->m_write_io = 1;
	}
	else
	{
		machine.device("m8502")->memory().space(AS_PROGRAM)->install_read_bank(0xd000, 0xdfff, "bank5");
		state->m_write_io = 0;
		if ((!charen && (loram || hiram)))
			state->membank("bank13")->set_base(state->m_chargen);
		else
			state->membank("bank13")->set_base(state->m_memory + 0xd000);
	}

	if (!state->m_game && state->m_exrom)
	{
		state->membank("bank14")->set_base(state->m_romh);
		state->membank("bank15")->set_base(state->m_romh+0x1f00);
		state->membank("bank16")->set_base(state->m_romh+0x1f05);
	}
	else
	{
		if (hiram)
		{
			state->membank("bank14")->set_base(state->m_kernal);
			state->membank("bank15")->set_base(state->m_kernal+0x1f00);
			state->membank("bank16")->set_base(state->m_kernal+0x1f05);
		}
		else
		{
			state->membank("bank14")->set_base(state->m_memory + 0xe000);
			state->membank("bank15")->set_base(state->m_memory + 0xff00);
			state->membank("bank16")->set_base(state->m_memory + 0xff05);
		}
	}
	state->m_old_data = data;
	state->m_old_exrom = state->m_exrom;
	state->m_old_game =state->m_game;
}

/* typical z80 configuration
   0x3f 0x3f 0x7f 0x3e 0x7e 0xb0 0x0b 0x00 0x00 0x01 0x00 */
static void c128_bankswitch_z80( running_machine &machine )
{
	c128_state *state = machine.driver_data<c128_state>();
	 state->m_ram = state->m_memory + MMU_RAM_ADDR;
	 state->m_va1617 = MMU_VIC_ADDR;
#if 1
	 state->membank("bank10")->set_base(state->m_z80);
	 state->membank("bank11")->set_base(state->m_ram + 0x1000);
	 if ( (( (machine.root_device().ioport("SPECIAL")->read() & 0x06) == 0x02 ) && (MMU_RAM_ADDR >= 0x40000))
		  || (( (machine.root_device().ioport("SPECIAL")->read() & 0x06) == 0x00) && (MMU_RAM_ADDR >= 0x20000)) )
		 state->m_ram = NULL;
#else
	 if (MMU_BOTTOM)
		 state->m_ram_bottom = MMU_SIZE;
	 else
		 state->m_ram_bottom = 0;

	 if (MMU_RAM_ADDR==0) { /* this is used in z80 mode for rom on/off switching !*/
		 state->membank("bank10")->set_base(state->m_z80);
		 state->membank("bank11")->set_base(state->m_z80 + 0x400);
	 }
	 else
	 {
		 state->membank("bank10")->set_base((state->m_ram_bottom > 0 ? state->m_memory : state->m_ram));
		 state->membank("bank11")->set_base((state->m_ram_bottom > 0x400 ? state->m_memory : state->m_ram) + 0x400);
	 }

	 state->membank("bank1")->set_base((state->m_ram_bottom > 0 ? state->m_memory : state->m_ram));
	 state->membank("bank2")->set_base((state->m_ram_bottom > 0x400 ? state->m_memory : state->m_ram) + 0x400);

	 state->membank("bank3")->set_base((state->m_ram_bottom > 0x1000 ? state->m_memory : state->m_ram) + 0x1000);
	 state->membank("bank4")->set_base((state->m_ram_bottom > 0x2000 ? state->m_memory : state->m_ram) + 0x2000);
	 state->membank("bank5")->set_base(state->m_ram + 0x4000);

	 if (MMU_TOP)
		 state->m_ram_top = 0x10000 - MMU_SIZE;
	 else
		 state->m_ram_top = 0x10000;

	 if (state->m_ram_top > 0xc000)
		state->membank("bank6")->set_base(state->m_ram + 0xc000);
	 else
		state->membank("bank6")->set_base(state->m_memory + 0xc000);

	 if (state->m_ram_top > 0xe000)
		state->membank("bank7")->set_base(state->m_ram + 0xe000);
	 else
		state->membank("bank7")->set_base(state->m_memory + 0xd000);

	 if (state->m_ram_top > 0xf000)
		state->membank("bank8")->set_base(state->m_ram + 0xf000);
	 else
		state->membank("bank8")->set_base(state->m_memory + 0xe000);

	 if (state->m_ram_top > 0xff05)
		state->membank("bank9")->set_base(state->m_ram + 0xff05);
	 else
		state->membank("bank9")->set_base(state->m_memory + 0xff05);

	 if ( (( (machine.root_device().ioport("SPECIAL")->read() & 0x06) == 0x02 ) && (MMU_RAM_ADDR >= 0x40000))
		  || (( (machine.root_device().ioport("SPECIAL")->read() & 0x06) == 0x00) && (MMU_RAM_ADDR >= 0x20000)) )
		 state->m_ram = NULL;
#endif
}

static void c128_bankswitch_128( running_machine &machine, int reset )
{
	c128_state *state = machine.driver_data<c128_state>();
	state->m_c64mode = MMU_64MODE;
	if (state->m_c64mode)
	{
		/* mmu works also in c64 mode, but this can wait */
		state->m_ram = state->m_memory;
		state->m_va1617 = 0;
		state->m_ram_bottom = 0;
		state->m_ram_top = 0x10000;

		state->membank("bank1")->set_base(state->m_memory);
		state->membank("bank2")->set_base(state->m_memory + 0x100);

		state->membank("bank3")->set_base(state->m_memory + 0x200);
		state->membank("bank4")->set_base(state->m_memory + 0x400);
		state->membank("bank5")->set_base(state->m_memory + 0x1000);
		state->membank("bank6")->set_base(state->m_memory + 0x2000);

		state->membank("bank7")->set_base(state->m_memory + 0x4000);

		state->membank("bank12")->set_base(state->m_memory + 0xc000);

		c128_bankswitch_64(machine, reset);
	}
	else
	{
		state->m_ram = state->m_memory + MMU_RAM_ADDR;
		state->m_va1617 = MMU_VIC_ADDR;
		state->membank("bank1")->set_base(state->m_memory + state->m_mmu_page0);
		state->membank("bank2")->set_base(state->m_memory + state->m_mmu_page1);
		if (MMU_BOTTOM)
			{
				state->m_ram_bottom = MMU_SIZE;
			}
		else
			state->m_ram_bottom = 0;
		state->membank("bank3")->set_base((state->m_ram_bottom > 0x200 ? state->m_memory : state->m_ram) + 0x200);
		state->membank("bank4")->set_base((state->m_ram_bottom > 0x400 ? state->m_memory : state->m_ram) + 0x400);
		state->membank("bank5")->set_base((state->m_ram_bottom > 0x1000 ? state->m_memory : state->m_ram) + 0x1000);
		state->membank("bank6")->set_base((state->m_ram_bottom > 0x2000 ? state->m_memory : state->m_ram) + 0x2000);

		if (MMU_RAM_LO)
		{
			state->membank("bank7")->set_base(state->m_ram + 0x4000);
		}
		else
		{
			state->membank("bank7")->set_base(state->m_c128_basic);
		}

		if (MMU_RAM_MID)
		{
			state->membank("bank8")->set_base(state->m_ram + 0x8000);
			state->membank("bank9")->set_base(state->m_ram + 0xa000);
		}
		else if (MMU_ROM_MID)
		{
			state->membank("bank8")->set_base(state->m_c128_basic + 0x4000);
			state->membank("bank9")->set_base(state->m_c128_basic + 0x6000);
		}
		else if (MMU_INTERNAL_ROM_MID)
		{
			state->membank("bank8")->set_base(state->m_internal_function);
			state->membank("bank9")->set_base(state->m_internal_function + 0x2000);
		}
		else
		{
			state->membank("bank8")->set_base(state->m_external_function);
			state->membank("bank9")->set_base(state->m_external_function + 0x2000);
		}

		if (MMU_TOP)
		{
			state->m_ram_top = 0x10000 - MMU_SIZE;
		}
		else
			state->m_ram_top = 0x10000;

		machine.device("m8502")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xff00, 0xff04, FUNC(c128_mmu8722_ff00_r));

		if (MMU_IO_ON)
		{
			state->m_write_io = 1;
			machine.device("m8502")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xd000, 0xdfff, FUNC(c128_read_io));
		}
		else
		{
			state->m_write_io = 0;
			machine.device("m8502")->memory().space(AS_PROGRAM)->install_read_bank(0xd000, 0xdfff, "bank13");
		}


		if (MMU_RAM_HI)
		{
			if (state->m_ram_top > 0xc000)
			{
				state->membank("bank12")->set_base(state->m_ram + 0xc000);
			}
			else
			{
				state->membank("bank12")->set_base(state->m_memory + 0xc000);
			}
			if (!MMU_IO_ON)
			{
				if (state->m_ram_top > 0xd000)
				{
					state->membank("bank13")->set_base(state->m_ram + 0xd000);
				}
				else
				{
					state->membank("bank13")->set_base(state->m_memory + 0xd000);
				}
			}
			if (state->m_ram_top > 0xe000)
			{
				state->membank("bank14")->set_base(state->m_ram + 0xe000);
			}
			else
			{
				state->membank("bank14")->set_base(state->m_memory + 0xe000);
			}
			if (state->m_ram_top > 0xff05)
			{
				state->membank("bank16")->set_base(state->m_ram + 0xff05);
			}
			else
			{
				state->membank("bank16")->set_base(state->m_memory + 0xff05);
			}
		}
		else if (MMU_ROM_HI)
		{
			state->membank("bank12")->set_base(state->m_editor);
			if (!MMU_IO_ON) {
				state->membank("bank13")->set_base(state->m_c128_chargen);
			}
			state->membank("bank14")->set_base(state->m_c128_kernal);
			state->membank("bank16")->set_base(state->m_c128_kernal + 0x1f05);
		}
		else if (MMU_INTERNAL_ROM_HI)
		{
			state->membank("bank12")->set_base(state->m_internal_function);
			if (!MMU_IO_ON) {
				state->membank("bank13")->set_base(state->m_internal_function + 0x1000);
			}
			state->membank("bank14")->set_base(state->m_internal_function + 0x2000);
			state->membank("bank16")->set_base(state->m_internal_function + 0x3f05);
		}
		else					   /*if (MMU_EXTERNAL_ROM_HI) */
		{
			state->membank("bank12")->set_base(state->m_external_function);
			if (!MMU_IO_ON) {
				state->membank("bank13")->set_base(state->m_external_function + 0x1000);
			}
			state->membank("bank14")->set_base(state->m_external_function + 0x2000);
			state->membank("bank16")->set_base(state->m_external_function + 0x3f05);
		}

		if ( (( (machine.root_device().ioport("SPECIAL")->read() & 0x06) == 0x02 ) && (MMU_RAM_ADDR >= 0x40000))
				|| (( (machine.root_device().ioport("SPECIAL")->read() & 0x06) == 0x00) && (MMU_RAM_ADDR >= 0x20000)) )
			state->m_ram = NULL;
	}
}

// 128u4
// FIX-ME: are the bankswitch functions working in the expected way without the memory_set_context?
static void c128_bankswitch( running_machine &machine, int reset )
{
	c128_state *state = machine.driver_data<c128_state>();
	if (state->m_mmu_cpu != MMU_CPU8502)
	{
		if (!MMU_CPU8502)
		{
//          DBG_LOG(machine, 1, "switching to z80", ("active %d\n",cpu_getactivecpu()) );
//          memory_set_context(machine, 0);
			c128_bankswitch_z80(machine);
//          memory_set_context(machine, 1);
			cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, CLEAR_LINE);
			cputag_set_input_line(machine, "m8502", INPUT_LINE_HALT, ASSERT_LINE);
		}
		else
		{
//          DBG_LOG(machine, 1, "switching to m6502", ("active %d\n",cpu_getactivecpu()) );
//          memory_set_context(machine, 1);
			c128_bankswitch_128(machine, reset);
//          memory_set_context(machine, 0);
			cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, ASSERT_LINE);
			cputag_set_input_line(machine, "m8502", INPUT_LINE_HALT, CLEAR_LINE);

			/* NPW 19-Nov-2005 - In the C128, CPU #0 starts out and hands over
             * control to CPU #1.  CPU #1 seems to execute garbage from 0x0000
             * up to 0x1100, at which point it finally hits some code
             * (presumably set up by CPU #1.) This always worked, but when I
             * changed the m8502 CPU core to use an internal memory map, it
             * started BRK-ing forever when trying to execute 0x0000.
             *
             * I am not sure whether the C128 actually executes this invalid
             * memory or if this is a bug in the C128 driver.  In any case, the
             * driver used to work with this behavior, so I am doing this hack
             * where I set CPU #1's PC to 0x1100 on reset.
             */
			if (cpu_get_reg(machine.device("m8502"), STATE_GENPC) == 0x0000)
				cpu_set_reg(machine.device("m8502"), STATE_GENPC, 0x1100);
		}
		state->m_mmu_cpu = MMU_CPU8502;
		return;
	}
	if (!MMU_CPU8502)
		c128_bankswitch_z80(machine);
	else
		c128_bankswitch_128(machine, reset);
}

static void c128_mmu8722_reset( running_machine &machine )
{
	c128_state *state = machine.driver_data<c128_state>();
	memset(state->m_mmu, 0, sizeof (state->m_mmu));
	state->m_mmu[5] |= 0x38;
	state->m_mmu[10] = 1;
	state->m_mmu_cpu = 0;
	state->m_mmu_page0 = 0;
	state->m_mmu_page1 = 0x0100;
	c128_bankswitch (machine, 1);
}

WRITE8_HANDLER( c128_mmu8722_port_w )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	offset &= 0xf;
	switch (offset)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 8:
	case 10:
		state->m_mmu[offset] = data;
		break;
	case 5:
		state->m_mmu[offset] = data;
		c128_bankswitch (space->machine(), 0);
		c128_iec_srq_out_w(space->machine());
		c128_iec_data_out_w(space->machine());
		break;
	case 0:
	case 6:
		state->m_mmu[offset] = data;
		c128_bankswitch (space->machine(), 0);
		break;
	case 7:
		state->m_mmu[offset] = data;
		state->m_mmu_page0=MMU_PAGE0;
		break;
	case 9:
		state->m_mmu[offset] = data;
		state->m_mmu_page1=MMU_PAGE1;
		c128_bankswitch (space->machine(), 0);
		break;
	case 0xb:
		break;
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		break;
	}
}

READ8_HANDLER( c128_mmu8722_port_r )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	int data;

	offset &= 0x0f;
	switch (offset)
	{
	case 5:
		data = state->m_mmu[offset] | 6;
		if ( /*disk enable signal */ 0)
			data &= ~8;
		if (!state->m_game)
			data &= ~0x10;
		if (!state->m_exrom)
			data &= ~0x20;
		if (space->machine().root_device().ioport("SPECIAL")->read() & 0x10)
			data &= ~0x80;
		break;
	case 0xb:
		/* hinybble number of 64 kb memory blocks */
		if ((space->machine().root_device().ioport("SPECIAL")->read() & 0x06) == 0x02)		// 256KB RAM
			data = 0x4f;
		else if ((state->ioport("SPECIAL")->read() & 0x06) == 0x04)	//  1MB
			data = 0xf;
		else
			data = 0x2f;
		break;
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		data=0xff;
		break;
	default:
		data=state->m_mmu[offset];
	}
	return data;
}

WRITE8_HANDLER( c128_mmu8722_ff00_w )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	switch (offset)
	{
	case 0:
		state->m_mmu[offset] = data;
		c128_bankswitch (space->machine(), 0);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
#if 1
		state->m_mmu[0]= state->m_mmu[offset];
#else
		state->m_mmu[0]|= state->m_mmu[offset];
#endif
		c128_bankswitch (space->machine(), 0);
		break;
	}
}

 READ8_HANDLER( c128_mmu8722_ff00_r )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	return state->m_mmu[offset];
}

WRITE8_HANDLER( c128_write_0000 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (state->m_ram != NULL)
		state->m_ram[0x0000 + offset] = data;
}

WRITE8_HANDLER( c128_write_1000 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (state->m_ram != NULL)
		state->m_ram[0x1000 + offset] = data;
}

WRITE8_HANDLER( c128_write_4000 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (state->m_ram != NULL)
		state->m_ram[0x4000 + offset] = data;
}

WRITE8_HANDLER( c128_write_8000 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (state->m_ram != NULL)
		state->m_ram[0x8000 + offset] = data;
}

WRITE8_HANDLER( c128_write_a000 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (state->m_ram != NULL)
		state->m_ram[0xa000 + offset] = data;
}

WRITE8_HANDLER( c128_write_c000 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (state->m_ram != NULL)
		state->m_ram[0xc000 + offset] = data;
}

WRITE8_HANDLER( c128_write_e000 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (offset + 0xe000 >= state->m_ram_top)
		state->m_memory[0xe000 + offset] = data;
	else if (state->m_ram != NULL)
		state->m_ram[0xe000 + offset] = data;
}

WRITE8_HANDLER( c128_write_ff00 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (!state->m_c64mode)
		c128_mmu8722_ff00_w(space, offset, data);
	else if (state->m_ram!=NULL)
		state->m_memory[0xff00 + offset] = data;
}

WRITE8_HANDLER( c128_write_ff05 )
{
	c128_state *state = space->machine().driver_data<c128_state>();
	if (offset + 0xff05 >= state->m_ram_top)
		state->m_memory[0xff05 + offset] = data;
	else if (state->m_ram!=NULL)
		state->m_ram[0xff05 + offset] = data;
}

/*
 * only 14 address lines
 * a15 and a14 portlines
 * 0x1000-0x1fff, 0x9000-0x9fff char rom
 */
READ8_MEMBER( c128_state::vic_dma_read )
{
	UINT8 c64_port6510 = m6510_get_port(machine().device<legacy_cpu_device>("m8502"));

	/* main memory configuration to include */
	if (m_c64mode)
	{
		if (!m_game && m_exrom)
		{
			if (offset < 0x3000)
				return m_memory[offset];
			return m_romh[offset & 0x1fff];
		}
		if (((m_vicaddr - m_memory + offset) & 0x7000) == 0x1000)
			return m_chargen[offset & 0xfff];
		return m_vicaddr[offset];
	}
	if (!(c64_port6510 & 4) && (((m_c128_vicaddr - m_memory + offset) & 0x7000) == 0x1000))
		return m_c128_chargen[offset & 0xfff];
	return m_c128_vicaddr[offset];
}

READ8_MEMBER( c128_state::vic_dma_read_color )
{
	UINT8 c64_port6510 = m6510_get_port(machine().device<legacy_cpu_device>("m8502"));

	if (m_c64mode)
		return m_colorram[offset & 0x3ff] & 0xf;
	else
		return m_colorram[(offset & 0x3ff)|((c64_port6510 & 0x3) << 10)] & 0xf;
}

/* 2008-09-01
    We need here the m6510 port handlers from c64, otherwise c128_common_driver_init
    seems unable to use correctly the timer.
    This will be probably fixed in a future clean up.
*/


WRITE8_DEVICE_HANDLER(c128_m6510_port_write)
{
	c128_state *state = device->machine().driver_data<c128_state>();

	if (state->m_tape_on)
	{
		device->machine().device<cassette_image_device>(CASSETTE_TAG)->output((data & 0x08) ? -(0x5a9e >> 1) : +(0x5a9e >> 1));

		if (!(data & 0x20))
		{
			device->machine().device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
			state->m_datasette_timer->adjust(attotime::zero, 0, attotime::from_hz(44100));
		}
		else
		{
			device->machine().device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_DISABLED ,CASSETTE_MASK_MOTOR);
			state->m_datasette_timer->reset();
		}
	}

	c128_bankswitch_64(device->machine(), 0);

	state->m_memory[0x000] = device->memory().space(AS_PROGRAM)->read_byte(0);
	state->m_memory[0x001] = device->memory().space(AS_PROGRAM)->read_byte(1);

}

READ8_DEVICE_HANDLER(c128_m6510_port_read)
{
	c128_state *state = device->machine().driver_data<c128_state>();
	UINT8 data = 0x07;

	if ((device->machine().device<cassette_image_device>(CASSETTE_TAG)->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED)
		data &= ~0x10;
	else
		data |=  0x10;

	if (state->ioport("SPECIAL")->read() & 0x20)		/* Check Caps Lock */
		data &= ~0x40;
	else
		data |=  0x40;

	return data;
}

static void c128_common_driver_init( running_machine &machine )
{
	c128_state *state = machine.driver_data<c128_state>();
	UINT8 *gfx=machine.root_device().memregion("gfx1")->base();
	UINT8 *ram = state->memregion("maincpu")->base();
	int i;

	state->m_memory = ram;

	state->m_c128_basic = ram + 0x100000;
	state->m_basic = ram + 0x108000;
	state->m_kernal = ram + 0x10a000;
	state->m_editor = ram + 0x10c000;
	state->m_z80 = ram + 0x10d000;
	state->m_c128_kernal = ram + 0x10e000;
	state->m_internal_function = ram + 0x110000;
	state->m_external_function = ram + 0x118000;
	state->m_chargen = ram + 0x120000;
	state->m_c128_chargen = ram + 0x121000;
	state->m_colorram = ram + 0x122000;
	state->m_vdcram = ram + 0x122800;
	state->m_c64_roml = auto_alloc_array(machine, UINT8, 0x2000);
	state->m_c64_romh = auto_alloc_array(machine, UINT8, 0x2000);

	state->m_tape_on = 1;
	state->m_game = 1;
	state->m_exrom = 1;
	state->m_pal = 0;
	state->m_c64mode = 0;
	state->m_vicirq = 0;

	state->m_monitor = -1;
	state->m_cnt1 = 1;
	state->m_sp1 = 1;
	cbm_common_init();
	state->m_keyline[0] = state->m_keyline[1] = state->m_keyline[2] = 0xff;

	for (i = 0; i < 0x100; i++)
		gfx[i] = i;

	if (state->m_tape_on)
		state->m_datasette_timer = machine.scheduler().timer_alloc(FUNC(c64_tape_timer));
}

DRIVER_INIT_MEMBER(c128_state,c128)
{
	//device_t *vic2e = machine().device("vic2e");
	//device_t *vdc8563 = machine().device("vdc8563");

	c128_common_driver_init(machine());

	//vic2_set_rastering(vic2e, 0);
	//vdc8563_set_rastering(vdc8563, 1);
}

DRIVER_INIT_MEMBER(c128_state,c128pal)
{
	//device_t *vic2e = machine().device("vic2e");
	//device_t *vdc8563 = machine().device("vdc8563");

	c128_common_driver_init(machine());
	m_pal = 1;

	//vic2_set_rastering(vic2e, 1);
	//vdc8563_set_rastering(vdc8563, 0);
}

DRIVER_INIT_MEMBER(c128_state,c128d)
{
	DRIVER_INIT_CALL( c128 );
}

DRIVER_INIT_MEMBER(c128_state,c128dpal)
{
	DRIVER_INIT_CALL( c128pal );
}

DRIVER_INIT_MEMBER(c128_state,c128dcr)
{
	DRIVER_INIT_CALL( c128 );
}

DRIVER_INIT_MEMBER(c128_state,c128dcrp)
{
	DRIVER_INIT_CALL( c128pal );
}

DRIVER_INIT_MEMBER(c128_state,c128d81)
{
	DRIVER_INIT_CALL( c128 );
}

MACHINE_START( c128 )
{
	c128_state *state = machine.driver_data<c128_state>();
// This was in MACHINE_START( c64 ), but never called
// TO DO: find its correct use, when fixing c64 mode
	if (state->m_c64mode)
		c128_bankswitch_64(machine, 1);
}

MACHINE_RESET( c128 )
{
	c128_state *state = machine.driver_data<c128_state>();
	state->m_c128_vicaddr = state->m_vicaddr = state->m_memory;
	state->m_c64mode = 0;
	c128_mmu8722_reset(machine);
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, CLEAR_LINE);
	cputag_set_input_line(machine, "m8502", INPUT_LINE_HALT, ASSERT_LINE);
}


INTERRUPT_GEN( c128_frame_interrupt )
{
	c128_state *state = device->machine().driver_data<c128_state>();
	static const char *const c128ports[] = { "KP0", "KP1", "KP2" };
	int i, value;
	//device_t *vic2e = device->machine().device("vic2e");
	//device_t *vdc8563 = device->machine().device("vdc8563");

	c128_nmi(device->machine());

	if ((device->machine().root_device().ioport("SPECIAL")->read() & 0x08) != state->m_monitor)
	{
		if (device->machine().root_device().ioport("SPECIAL")->read() & 0x08)
		{
			//vic2_set_rastering(vic2e, 0);
			//vdc8563_set_rastering(vdc8563, 1);
			device->machine().primary_screen->set_visible_area(0, 655, 0, 215);
		}
		else
		{
			//vic2_set_rastering(vic2e, 1);
			//vdc8563_set_rastering(vdc8563, 0);
			if (state->m_pal)
				device->machine().primary_screen->set_visible_area(0, VIC6569_VISIBLECOLUMNS - 1, 0, VIC6569_VISIBLELINES - 1);
			else
				device->machine().primary_screen->set_visible_area(0, VIC6567_VISIBLECOLUMNS - 1, 0, VIC6567_VISIBLELINES - 1);
		}
		state->m_monitor = device->machine().root_device().ioport("SPECIAL")->read() & 0x08;
	}


	/* common keys input ports */
	cbm_common_interrupt(device);

	/* Fix Me! Currently, neither left Shift nor Shift Lock work in c128, but reading the correspondent input produces a bug!
    Hence, we overwrite the actual reading as it never happens */
	if ((device->machine().root_device().ioport("SPECIAL")->read() & 0x40))	//
		c64_keyline[1] |= 0x80;

	/* c128 specific: keypad input ports */
	for (i = 0; i < 3; i++)
	{
		value = 0xff;
		value &= ~device->machine().root_device().ioport(c128ports[i])->read();
		state->m_keyline[i] = value;
	}
}
