/***************************************************************************

    Amiga cartridge emulation

TODO:
- Investigate why the AR2/3 sometimes garble the video on exit from the cart
- Add Nordic Power and similar carts
- Add HRTMon A1200 cart

***************************************************************************/


#include "emu.h"
#include "includes/amiga.h"
#include "cpu/m68000/m68000.h"
#include "machine/6526cia.h"
#include "machine/amigacrt.h"

enum
{
	/* supported cartridges */
	ACTION_REPLAY				=	0,
	ACTION_REPLAY_MKII,
	ACTION_REPLAY_MKIII
};

struct amigacrt_t
{
	int cart_type;
	int ar1_spurious;
	UINT16 ar23_mode;
};
static amigacrt_t amigacrt;

/***************************************************************************

  Utilities

***************************************************************************/

static int check_kickstart_12_13( running_machine &machine, const char *cart_name )
{
	UINT16 * ksmem = (UINT16 *)(*machine.root_device().memregion( "user1" ));

	if ( ksmem[2] == 0x00FC )
		return 1;

	logerror( "%s requires Kickstart version 1.2 or 1.3 - Cart not installed\n", cart_name );

	return 0;
}

/***************************************************************************

  Amiga Action Replay 1

  Whenever you push the button, an NMI interrupt is generated.

  The cartridge protects itself from being removed from memory by
  generating an interrupt whenever either the spurious interrupt
  vector is written at $60, or the nmi interrupt vector is written at
  $7c. If the spurious interrupt vector at $60 is written, then
  a NMI is generated, and the spurious irq vector is restored. If the
  NMI vector at $7c is written, then a spurious interrupt is generated,
  and the NMI vector is restored.

  When breakpoints are set, the target address is replaced by a trap
  instruction, and the original contents of the address are kept in
  the cartridge's RAM. A trap handler is installed pointing to $40
  in RAM, where code that clears memory address $60 followed by two
  nops are written. There seems to be a small delay between the write
  to $60 and until the actual NMI is generated, since the cart expects
  the PC to be at $46 (at the second nop).

***************************************************************************/

static IRQ_CALLBACK(amiga_ar1_irqack)
{
	if ( irqline == 7 && amigacrt.ar1_spurious )
	{
		return M68K_INT_ACK_SPURIOUS;
	}

	return (24+irqline);
}

static TIMER_CALLBACK( amiga_ar1_delayed_nmi )
{
	(void)param;
	machine.device("maincpu")->execute().set_input_line(7, PULSE_LINE);
}

static void amiga_ar1_nmi( running_machine &machine )
{
	amiga_state *state = machine.driver_data<amiga_state>();
	/* get the cart's built-in ram */
	UINT16 *ar_ram = (UINT16 *)machine.device("maincpu")->memory().space(AS_PROGRAM)->get_write_ptr(0x9fc000);

	if ( ar_ram != NULL )
	{
		int i;

		/* copy custom register values */
		for( i = 0; i < 256; i++ )
			ar_ram[0x1800+i] = CUSTOM_REG(i);

		/* trigger NMI irq */
		amigacrt.ar1_spurious = 0;
		machine.scheduler().timer_set(machine.device<cpu_device>("maincpu")->cycles_to_attotime(28), FUNC(amiga_ar1_delayed_nmi));
	}
}

static WRITE16_HANDLER( amiga_ar1_chipmem_w )
{
	amiga_state *state = space->machine().driver_data<amiga_state>();
	int pc = space->device().safe_pc();

	/* see if we're inside the AR1 rom */
	if ( ((pc >> 16) & 0xff ) != 0xf0 )
	{
		/* if we're not, see if either the Spurious IRQ vector
           or the NMI vector are being overwritten */
		if ( offset == (0x60/2) || offset == (0x7c/2) )
		{
			/* trigger an NMI or spurious irq */
			amigacrt.ar1_spurious = (offset == 0x60/2) ? 0 : 1;
			space->machine().scheduler().timer_set(space->machine().device<cpu_device>("maincpu")->cycles_to_attotime(28), FUNC(amiga_ar1_delayed_nmi));
		}
	}

	(*state->m_chip_ram_w)(state,  offset * 2, data );
}

static void amiga_ar1_check_overlay( running_machine &machine )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x000000, 0x00007f, FUNC(amiga_ar1_chipmem_w));
}

static void amiga_ar1_init( running_machine &machine )
{
	amiga_state *state = machine.driver_data<amiga_state>();
	void *ar_ram;

	/* check kickstart version */
	if ( !check_kickstart_12_13( machine, "Amiga Action Replay" ) )
	{
		amigacrt.cart_type = -1;
		return;
	}

	/* setup the cart ram */
	ar_ram = auto_alloc_array(machine, UINT8, 0x4000);
	memset(ar_ram, 0, 0x4000);

	/* Install ROM */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0xf00000, 0xf7ffff, "bank2");
	machine.device("maincpu")->memory().space(AS_PROGRAM)->unmap_write(0xf00000, 0xf7ffff);

	/* Install RAM */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_bank(0x9fc000, 0x9fffff, "bank3");

	/* Configure Banks */
	state->membank("bank2")->set_base(machine.root_device().memregion("user2")->base());
	state->membank("bank3")->set_base(ar_ram);

	amigacrt.ar1_spurious = 0;

	/* Install IRQ ACK callback */
	machine.device("maincpu")->execute().set_irq_acknowledge_callback(amiga_ar1_irqack);
}

/***************************************************************************

  Amiga Action Replay MKII/MKIII

  Whenever you push the button, an overlay of the cartridge appears
  in RAM and an NMI interrupt is generated. Only reads are overlayed.
  Writes go directly to chipmem (so that the CPU can write the stack
  information for the NMI). Once the interrupt is taken, the cart
  disables the ROM overlay and checks it's internal mode register
  to see why it was invoked.

  When a breakpoint is set the cart replaces the target instruction
  with a trap, and adds a trap handler that it locates in the $100-$120
  area. The handler simply has one instruction: TST.B $BFE001. This cia
  access is monitored, and the conditions match (PC < 120 and a previous
  command requested the monitoring of the cia) then the cart is invoked.

***************************************************************************/

static void amiga_ar23_freeze( running_machine &machine );

static READ16_HANDLER( amiga_ar23_cia_r )
{
	int pc = space->device().safe_pc();

	if ( ACCESSING_BITS_0_7 && offset == 2048 && pc >= 0x40 && pc < 0x120 )
	{
		amiga_ar23_freeze(space->machine());
	}

	return amiga_cia_r( space, offset, mem_mask );
}

static WRITE16_HANDLER( amiga_ar23_mode_w )
{
	if ( data & 2 )
	{
		space->install_legacy_read_handler(0xbfd000, 0xbfefff, FUNC(amiga_ar23_cia_r));
	}
	else
	{
		space->install_legacy_read_handler(0xbfd000, 0xbfefff, FUNC(amiga_cia_r));
	}

	amigacrt.ar23_mode = (data&0x3);
	if ( amigacrt.ar23_mode == 0 )
		amigacrt.ar23_mode = 1;

}

static READ16_HANDLER( amiga_ar23_mode_r )
{
	amiga_state *state = space->machine().driver_data<amiga_state>();
	UINT16 *mem = (UINT16 *)(*state->memregion( "user2" ));

	if ( ACCESSING_BITS_0_7 )
	{
		if ( offset < 2 )
			return (mem[offset] | (amigacrt.ar23_mode&3));

		if ( offset == 0x03 ) /* disable cart oberlay on chip mem */
		{
			UINT32 mirror_mask = state->m_chip_ram.bytes();

			state->membank("bank1")->set_entry(0);

			while( (mirror_mask<<1) < 0x100000 )
			{
				mirror_mask |= ( mirror_mask << 1 );
			}

			/* overlay disabled, map RAM on 0x000000 */
			space->install_write_bank(0x000000, state->m_chip_ram.bytes() - 1, 0, mirror_mask, "bank1");
		}
	}

	return mem[offset];
}

static WRITE16_HANDLER( amiga_ar23_chipmem_w )
{
	amiga_state *state = space->machine().driver_data<amiga_state>();
	if ( offset == (0x08/2) )
	{
		if ( amigacrt.ar23_mode & 1 )
			amiga_ar23_freeze(space->machine());
	}

	(*state->m_chip_ram_w)(state,  offset * 2, data );
}

static void amiga_ar23_freeze( running_machine &machine )
{
	amiga_state *state = machine.driver_data<amiga_state>();
	int pc = machine.device("maincpu")->safe_pc();

	/* only freeze if we're not inside the cart's ROM */
	if ( ((pc >> 16) & 0xfe ) != 0x40 )
	{
		/* get the cart's built-in ram */
		UINT16 *ar_ram = (UINT16 *)machine.device("maincpu")->memory().space(AS_PROGRAM)->get_write_ptr(0x440000);

		if ( ar_ram != NULL )
		{
			int		i;

			for( i = 0; i < 0x100; i++ )
				ar_ram[0x7800+i] = CUSTOM_REG(i);
		}

		/* overlay the cart rom's in chipram */
		state->membank("bank1")->set_entry(2);

		/* writes go to chipram */
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x000000, state->m_chip_ram.bytes() - 1, FUNC(amiga_ar23_chipmem_w));

		/* trigger NMI irq */
		machine.device("maincpu")->execute().set_input_line(7, PULSE_LINE);
	}
}

static void amiga_ar23_nmi( running_machine &machine )
{
	amigacrt.ar23_mode = 0;
	amiga_ar23_freeze(machine);
}

#if 0
static WRITE16_HANDLER( amiga_ar23_custom_w )
{
	int pc = space->device().safe_pc();

	/* see if we're inside the AR2 rom */
	if ( ((pc >> 16) & 0xfe ) != 0x40 )
	{
		/* get the cart's built-in ram */
		UINT16 *ar_ram = (UINT16 *)memory_get_write_ptr(0, AS_PROGRAM, 0x440000);

		if ( ar_ram != NULL )
		{
			ar_ram[0x7800+offset] = data;
		}
	}

	amiga_custom_w( offset, data, mem_mask );
}

static READ16_HANDLER( amiga_ar23_custom_r )
{
	UINT16 data = amiga_custom_r( offset, mem_mask );

	int pc = space->device().safe_pc();

	/* see if we're inside the AR2 rom */
	if ( ((pc >> 16) & 0xfe ) != 0x40 )
	{
		/* get the cart's built-in ram */
		UINT16 *ar_ram = (UINT16 *)memory_get_write_ptr(0, AS_PROGRAM, 0x440000);

		if ( ar_ram != NULL )
		{
			ar_ram[0x7800+offset] = data;
		}
	}

	return data;
}
#endif

static void amiga_ar23_check_overlay( running_machine &machine )
{
	amigacrt.ar23_mode = 3;
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x000000, 0x00000f, FUNC(amiga_ar23_chipmem_w));
}

static void amiga_ar23_init( running_machine &machine, int ar3 )
{
	amiga_state *state = machine.driver_data<amiga_state>();
	UINT32 mirror = 0x20000, size = 0x1ffff;
	void *ar_ram;

	/* check kickstart version */
	if ( !check_kickstart_12_13( machine, "Action Replay MKII or MKIII" ) )
	{
		amigacrt.cart_type = -1;
		return;
	}

	/* setup the cart ram */
	ar_ram = auto_alloc_array_clear(machine, UINT8,0x10000);

	if ( ar3 )
	{
		mirror = 0;
		size = 0x3ffff;
	}

	/* Install ROM */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x400000, 0x400000+size, 0, mirror, "bank2");
	machine.device("maincpu")->memory().space(AS_PROGRAM)->unmap_write(0x400000, 0x400000+size, 0, mirror);

	/* Install RAM */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x440000, 0x44ffff, "bank3");
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_bank(0x440000, 0x44ffff, "bank3");

	/* Install Custom chip monitor */
//  machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xdff000, 0xdff1ff, FUNC(amiga_ar23_custom_r));
//  machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xdff000, 0xdff1ff, FUNC(amiga_ar23_custom_w));

	/* Install status/mode handlers */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x400007, 0, mirror, FUNC(amiga_ar23_mode_r));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x400000, 0x400003, 0, mirror, FUNC(amiga_ar23_mode_w));

	/* Configure Banks */
	state->membank("bank2")->set_base(machine.root_device().memregion("user2")->base());
	state->membank("bank3")->set_base(ar_ram);

	state->membank("bank1")->configure_entries(0, 2, state->m_chip_ram, 0);
	state->membank("bank1")->configure_entries(1, 2, machine.root_device().memregion("user1")->base(), 0);
	state->membank("bank1")->configure_entries(2, 2, machine.root_device().memregion("user2")->base(), 0);

	amigacrt.ar23_mode = 3;
}

/***************************************************************************

    MAME/MESS hooks

***************************************************************************/

void amiga_cart_init( running_machine &machine )
{
	/* see what is there */
	UINT16 *mem = (UINT16 *)(*machine.root_device().memregion( "user2" ));

	amigacrt.cart_type = -1;

	if ( mem != NULL )
	{
		if ( mem[0x00] == 0x1111 )
		{
			amigacrt.cart_type = ACTION_REPLAY;
			amiga_ar1_init(machine);
		}
		else if ( mem[0x0C] == 0x4D6B )
		{
			amigacrt.cart_type = ACTION_REPLAY_MKII;
			amiga_ar23_init(machine, 0);
		}
		else if ( mem[0x0C] == 0x4D4B )
		{
			amigacrt.cart_type = ACTION_REPLAY_MKIII;
			amiga_ar23_init(machine, 1);
		}
	}
}

void amiga_cart_check_overlay( running_machine &machine )
{
	if ( amigacrt.cart_type < 0 )
		return;

	switch( amigacrt.cart_type )
	{
		case ACTION_REPLAY:
			amiga_ar1_check_overlay(machine);
		break;

		case ACTION_REPLAY_MKII:
		case ACTION_REPLAY_MKIII:
			amiga_ar23_check_overlay(machine);
		break;
	}
}

void amiga_cart_nmi( running_machine &machine )
{
	if ( amigacrt.cart_type < 0 )
		return;

	switch( amigacrt.cart_type )
	{
		case ACTION_REPLAY:
			amiga_ar1_nmi(machine);
		break;

		case ACTION_REPLAY_MKII:
		case ACTION_REPLAY_MKIII:
			amiga_ar23_nmi(machine);
		break;
	}
}
