/***************************************************************************

    Neo-Geo hardware

    main driver file - please do NOT put anything specific to a
    particular game in this file, but use neodrvr.c instead

    Credits:
        * This driver was made possible by the research done by
          Charles MacDonald.  For a detailed description of the Neo-Geo
          hardware, please visit his page at:
          http://cgfm2.emuviews.com/txt/mvstech.txt
        * Presented to you by the Shin Emu Keikaku team.
        * The following people have all spent probably far
          too much time on this:
          AVDB
          Bryan McPhail
          Fuzz
          Ernesto Corvi
          Andrew Prime
          Zsolt Vasvari


    The Neo-Geo Multi Video System (MVS), is an arcade system board, being the first product in the Neo-Geo family, and released in 1990 by SNK.
    It was known to the coin-op industry, and offered arcade operators the ability to put up to 6 different arcade titles into a single cabinet,
    a key economic consideration for operators with limited floorspace (games for the Neo-Geo are cartridge based and are easily exchangeable).
    It comes in many different cabinets but basically consists of an add on board that can be linked to a standard Jamma system.
    The system was discontinued in 2004.
    Source (modified): http://en.wikipedia.org/wiki/Neo_Geo


    MVS motherboards were produced in 1 / 2 / 4 and 6 Slot versions.

    Known motherboards:
    ===================

    1 Slot:
    NEO-MVH MV1
    NEO-MVH MV1-1
    NEO-MVH MV1A
     . NEO-MVH MV1A CHX ??
    NEO-MVH MV1B (1996.1.19)
     . NEO-MVH MV1B CHX (1996.1.19) ??
    NEO-MVH MV1B1 (1998.6.17)
    NEO-MVH MV1C (1999.4.30)
    NEO-MVH MV1F
    NEO-MVH MV1FS
    NEO-MVH MV1FT
    NEO-MVH MV1FZ
    NEO-MVH MV1FZS

    2 Slot:
    NEO-MVH MV2
    NEO-MVH MV2F
    NEO-MVH MV2F-01

    4 Slot:
    NEO-MVH MV4
    NEO-MVH MV4F
    NEO-MVH MV4FS
    NEO-MVH MV4FT
    NEO-MVH MV4FT2

    6 Slot:
    NEO-MVH MV6
    NEO-MVH MV6F


    Neo-Geo Motherboard (info - courtesy of Guru):

          NEO-MVH MV1
          |---------------------------------------------------------------------|
          |       4558                                                          |
          |                                          HC04  HC32                 |
          |                      SP-S2.SP1  NEO-E0   000-L0.L0   LS244  AS04    |
          |             YM2610                                                  |
          | 4558                                                                |
          |       4558                        5814  HC259   SFIX.SFIX           |
          |                                                             NEO-I0  |
          | HA13001 YM3016                    5814                              |
          --|                                                                   |
            |     4558                                                          |
          --|                                                 SM1.SM1   LS32    |
          |                                                                     |
          |                           LSPC-A0         PRO-C0            LS244   |
          |                                                                     |
          |J              68000                                                 |
          |A                                                                    |
          |M                                                                    |
          |M                                                      NEO-ZMC2      |
          |A                                                                    |
          |   LS273  NEO-G0                          58256  58256     Z80A      |
          |                           58256  58256   58256  58256     6116      |
          |   LS273 5864                                                        |
          --| LS05  5864  PRO-B0                                                |
            |                                                                   |
          --|             LS06   HC32           D4990A    NEO-F0   24.000MHz    |
          |                      DSW1    BATT3.6V 32.768kHz       NEO-D0        |
          |                                           2003  2003                |
          |---------------------------------------------------------------------|


    Known driver issues/to-do's:
    ============================

        * Fatal Fury 3 crashes during the ending - this doesn't occur if
          the language is set to Japanese, maybe the English endings
          are incomplete / buggy?
        * Graphical Glitches caused by incorrect timing?
          - Some raster effects are imperfect (off by a couple of lines)
        * Mult-cart support not implementd - the MVS can take up to 6 carts
          depending on the board being used


    Confirmed non-bugs:

        * Bad zooming in the Kof2003 bootlegs - this is what happens
          if you try and use the normal bios with a pcb set, it
          looks like the bootleggers didn't care.
        * Glitches at the edges of the screen - the real hardware
          can display 320x240 but most of the games seem designed
          to work with a width of 304, some less.
        * Distorted jumping sound in Nightmare in the Dark
        * Ninja Combat sometimes glitches


    Mahjong Panel notes (2009-03 FP):
    =================================

    * In Service Mode menu with mahjong panel active, controls are as
      follows:

        A = select / up (for options)
        B = down (for options)
        C = go to previous menu
        E = up (for menu entries)
        F = down (for menu entries)
        G = left (for options)
        H = right (for options)

    * These only work with Japanese BIOS, but I think it's not a bug: I
      doubt other bios were programmed to be compatible with mahjong panels

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/neogeo.h"
#include "machine/nvram.h"
#include "machine/pd4990a.h"
#include "cpu/z80/z80.h"
#include "sound/2610intf.h"
#include "imagedev/cartslot.h"
#include "neogeo.lh"


#define LOG_VIDEO_SYSTEM         (0)
#define LOG_CPU_COMM             (0)
#define LOG_MAIN_CPU_BANKING     (0)
#define LOG_AUDIO_CPU_BANKING    (0)


/*************************************
 *
 *  Global variables
 *
 *************************************/

static UINT8 *memcard_data;
static UINT16 *save_ram;

static const char *audio_banks[4] =
{
    NEOGEO_BANK_AUDIO_CPU_CART_BANK0, NEOGEO_BANK_AUDIO_CPU_CART_BANK1, NEOGEO_BANK_AUDIO_CPU_CART_BANK2, NEOGEO_BANK_AUDIO_CPU_CART_BANK3
};


/*************************************
 *
 *  Forward declerations
 *
 *************************************/

static void set_output_latch(running_machine &machine, UINT8 data);
static void set_output_data(running_machine &machine, UINT8 data);


/*************************************
 *
 *  Main CPU interrupt generation
 *
 *************************************/

#define IRQ2CTRL_ENABLE             (0x10)
#define IRQ2CTRL_LOAD_RELATIVE      (0x20)
#define IRQ2CTRL_AUTOLOAD_VBLANK    (0x40)
#define IRQ2CTRL_AUTOLOAD_REPEAT    (0x80)


static void adjust_display_position_interrupt_timer( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	if ((state->m_display_counter + 1) != 0)
	{
		attotime period = attotime::from_hz(NEOGEO_PIXEL_CLOCK) * (state->m_display_counter + 1);
		if (LOG_VIDEO_SYSTEM) logerror("adjust_display_position_interrupt_timer  current y: %02x  current x: %02x   target y: %x  target x: %x\n", machine.primary_screen->vpos(), machine.primary_screen->hpos(), (state->m_display_counter + 1) / NEOGEO_HTOTAL, (state->m_display_counter + 1) % NEOGEO_HTOTAL);

		state->m_display_position_interrupt_timer->adjust(period);
	}
}


void neogeo_set_display_position_interrupt_control( running_machine &machine, UINT16 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_display_position_interrupt_control = data;
}


void neogeo_set_display_counter_msb( address_space *space, UINT16 data )
{
	neogeo_state *state = space->machine().driver_data<neogeo_state>();

	state->m_display_counter = (state->m_display_counter & 0x0000ffff) | ((UINT32)data << 16);

	if (LOG_VIDEO_SYSTEM) logerror("PC %06x: set_display_counter %08x\n", space->device().safe_pc(), state->m_display_counter);
}


void neogeo_set_display_counter_lsb( address_space *space, UINT16 data )
{
	neogeo_state *state = space->machine().driver_data<neogeo_state>();

	state->m_display_counter = (state->m_display_counter & 0xffff0000) | data;

	if (LOG_VIDEO_SYSTEM) logerror("PC %06x: set_display_counter %08x\n", space->device().safe_pc(), state->m_display_counter);

	if (state->m_display_position_interrupt_control & IRQ2CTRL_LOAD_RELATIVE)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_RELATIVE ");
		adjust_display_position_interrupt_timer(space->machine());
	}
}


static void update_interrupts( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	machine.device("maincpu")->execute().set_input_line(1, state->m_vblank_interrupt_pending ? ASSERT_LINE : CLEAR_LINE);
	machine.device("maincpu")->execute().set_input_line(2, state->m_display_position_interrupt_pending ? ASSERT_LINE : CLEAR_LINE);
	machine.device("maincpu")->execute().set_input_line(3, state->m_irq3_pending ? ASSERT_LINE : CLEAR_LINE);
}


void neogeo_acknowledge_interrupt( running_machine &machine, UINT16 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	if (data & 0x01)
		state->m_irq3_pending = 0;
	if (data & 0x02)
		state->m_display_position_interrupt_pending = 0;
	if (data & 0x04)
		state->m_vblank_interrupt_pending = 0;

	update_interrupts(machine);
}


static TIMER_CALLBACK( display_position_interrupt_callback )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	if (LOG_VIDEO_SYSTEM) logerror("--- Scanline @ %d,%d\n", machine.primary_screen->vpos(), machine.primary_screen->hpos());

	if (state->m_display_position_interrupt_control & IRQ2CTRL_ENABLE)
	{
		if (LOG_VIDEO_SYSTEM) logerror("*** Scanline interrupt (IRQ2) ***  y: %02x  x: %02x\n", machine.primary_screen->vpos(), machine.primary_screen->hpos());
		state->m_display_position_interrupt_pending = 1;

		update_interrupts(machine);
	}

	if (state->m_display_position_interrupt_control & IRQ2CTRL_AUTOLOAD_REPEAT)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_REPEAT ");
		adjust_display_position_interrupt_timer(machine);
	}
}


static TIMER_CALLBACK( display_position_vblank_callback )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	if (state->m_display_position_interrupt_control & IRQ2CTRL_AUTOLOAD_VBLANK)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_VBLANK ");
		adjust_display_position_interrupt_timer(machine);
	}

	/* set timer for next screen */
	state->m_display_position_vblank_timer->adjust(machine.primary_screen->time_until_pos(NEOGEO_VBSTART, NEOGEO_VBLANK_RELOAD_HPOS));
}


static TIMER_CALLBACK( vblank_interrupt_callback )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	if (LOG_VIDEO_SYSTEM) logerror("+++ VBLANK @ %d,%d\n", machine.primary_screen->vpos(), machine.primary_screen->hpos());

	/* add a timer tick to the pd4990a */
	upd4990a_addretrace(state->m_upd4990a);

	state->m_vblank_interrupt_pending = 1;

	update_interrupts(machine);

	/* set timer for next screen */
	state->m_vblank_interrupt_timer->adjust(machine.primary_screen->time_until_pos(NEOGEO_VBSTART));
}


static void create_interrupt_timers( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_display_position_interrupt_timer = machine.scheduler().timer_alloc(FUNC(display_position_interrupt_callback));
	state->m_display_position_vblank_timer = machine.scheduler().timer_alloc(FUNC(display_position_vblank_callback));
	state->m_vblank_interrupt_timer = machine.scheduler().timer_alloc(FUNC(vblank_interrupt_callback));
}


static void start_interrupt_timers( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_vblank_interrupt_timer->adjust(machine.primary_screen->time_until_pos(NEOGEO_VBSTART));
	state->m_display_position_vblank_timer->adjust(machine.primary_screen->time_until_pos(NEOGEO_VBSTART, NEOGEO_VBLANK_RELOAD_HPOS));
}



/*************************************
 *
 *  Audio CPU interrupt generation
 *
 *************************************/

static void audio_cpu_irq(device_t *device, int assert)
{
	neogeo_state *state = device->machine().driver_data<neogeo_state>();
	state->m_audiocpu->set_input_line(0, assert ? ASSERT_LINE : CLEAR_LINE);
}


static void audio_cpu_assert_nmi(running_machine &machine)
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


WRITE8_MEMBER(neogeo_state::audio_cpu_clear_nmi_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}



/*************************************
 *
 *  Input ports / Controllers
 *
 *************************************/

static void select_controller( running_machine &machine, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_controller_select = data;
}


CUSTOM_INPUT_MEMBER(neogeo_state::multiplexed_controller_r)
{
	int port = (FPTR)param;

	static const char *const cntrl[2][2] =
		{
			{ "IN0-0", "IN0-1" }, { "IN1-0", "IN1-1" }
		};

	return ioport(cntrl[port][m_controller_select & 0x01])->read_safe(0x00);
}

#if 1 // this needs to be added dynamically somehow
CUSTOM_INPUT_MEMBER(neogeo_state::mahjong_controller_r)
{
	UINT32 ret;

/*
cpu #0 (PC=00C18B9A): unmapped memory word write to 00380000 = 0012 & 00FF
cpu #0 (PC=00C18BB6): unmapped memory word write to 00380000 = 001B & 00FF
cpu #0 (PC=00C18D54): unmapped memory word write to 00380000 = 0024 & 00FF
cpu #0 (PC=00C18D6C): unmapped memory word write to 00380000 = 0009 & 00FF
cpu #0 (PC=00C18C40): unmapped memory word write to 00380000 = 0000 & 00FF
*/
	switch (m_controller_select)
	{
	default:
	case 0x00: ret = 0x0000; break; /* nothing? */
	case 0x09: ret = ioport("MAHJONG1")->read(); break;
	case 0x12: ret = ioport("MAHJONG2")->read(); break;
	case 0x1b: ret = ioport("MAHJONG3")->read(); break; /* player 1 normal inputs? */
	case 0x24: ret = ioport("MAHJONG4")->read(); break;
	}

	return ret;
}
#endif

WRITE16_MEMBER(neogeo_state::io_control_w)
{
	switch (offset)
	{
	case 0x00: select_controller(machine(), data & 0x00ff); break;
	case 0x18: set_output_latch(machine(), data & 0x00ff); break;
	case 0x20: set_output_data(machine(), data & 0x00ff); break;
	case 0x28: upd4990a_control_16_w(m_upd4990a, 0, data, mem_mask); break;
//  case 0x30: break; // coin counters
//  case 0x31: break; // coin counters
//  case 0x32: break; // coin lockout
//  case 0x33: break; // coui lockout

	default:
		logerror("PC: %x  Unmapped I/O control write.  Offset: %x  Data: %x\n", space.device().safe_pc(), offset, data);
		break;
	}
}



/*************************************
 *
 *  Unmapped memory access
 *
 *************************************/

READ16_MEMBER(neogeo_state::neogeo_unmapped_r)
{
	UINT16 ret;

	/* unmapped memory returns the last word on the data bus, which is almost always the opcode
       of the next instruction due to prefetch */

	/* prevent recursion */
	if (m_recurse)
		ret = 0xffff;
	else
	{
		m_recurse = 1;
		ret = space.read_word(space.device().safe_pc());
		m_recurse = 0;
	}

	return ret;
}



/*************************************
 *
 *  uPD4990A calendar chip
 *
 *************************************/

CUSTOM_INPUT_MEMBER(neogeo_state::get_calendar_status)
{
	return (upd4990a_databit_r(m_upd4990a, 0) << 1) | upd4990a_testbit_r(m_upd4990a, 0);
}



/*************************************
 *
 *  NVRAM (Save RAM)
 *
 *************************************/

static void set_save_ram_unlock( running_machine &machine, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_save_ram_unlocked = data;
}


WRITE16_MEMBER(neogeo_state::save_ram_w)
{

	if (m_save_ram_unlocked)
		COMBINE_DATA(&save_ram[offset]);
}



/*************************************
 *
 *  Memory card
 *
 *************************************/

#define MEMCARD_SIZE	0x0800


CUSTOM_INPUT_MEMBER(neogeo_state::get_memcard_status)
{
	/* D0 and D1 are memcard presence indicators, D2 indicates memcard
       write protect status (we are always write enabled) */
	return (memcard_present(machine()) == -1) ? 0x07 : 0x00;
}


READ16_MEMBER(neogeo_state::memcard_r)
{
	UINT16 ret;

	if (memcard_present(machine()) != -1)
		ret = memcard_data[offset] | 0xff00;
	else
		ret = 0xffff;

	return ret;
}


WRITE16_MEMBER(neogeo_state::memcard_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (memcard_present(machine()) != -1)
			memcard_data[offset] = data;
	}
}


static MEMCARD_HANDLER( neogeo )
{
	switch (action)
	{
	case MEMCARD_CREATE:
		memset(memcard_data, 0, MEMCARD_SIZE);
		file.write(memcard_data, MEMCARD_SIZE);
		break;

	case MEMCARD_INSERT:
		file.read(memcard_data, MEMCARD_SIZE);
		break;

	case MEMCARD_EJECT:
		file.write(memcard_data, MEMCARD_SIZE);
		break;
	}
}



/*************************************
 *
 *  Inter-CPU communications
 *
 *************************************/

WRITE16_MEMBER(neogeo_state::audio_command_w)
{
	/* accessing the LSB only is not mapped */
	if (mem_mask != 0x00ff)
	{
		soundlatch_byte_w(space, 0, data >> 8);

		audio_cpu_assert_nmi(machine());

		/* boost the interleave to let the audio CPU read the command */
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50));

		if (LOG_CPU_COMM) logerror("MAIN CPU PC %06x: audio_command_w %04x - %04x\n", space.device().safe_pc(), data, mem_mask);
	}
}


READ8_MEMBER(neogeo_state::audio_command_r)
{
	UINT8 ret = soundlatch_byte_r(space, 0);

	if (LOG_CPU_COMM) logerror(" AUD CPU PC   %04x: audio_command_r %02x\n", space.device().safe_pc(), ret);

	/* this is a guess */
	audio_cpu_clear_nmi_w(space, 0, 0);

	return ret;
}


WRITE8_MEMBER(neogeo_state::audio_result_w)
{

	if (LOG_CPU_COMM && (m_audio_result != data)) logerror(" AUD CPU PC   %04x: audio_result_w %02x\n", space.device().safe_pc(), data);

	m_audio_result = data;
}


CUSTOM_INPUT_MEMBER(neogeo_state::get_audio_result)
{
	UINT32 ret = m_audio_result;

//  if (LOG_CPU_COMM) logerror("MAIN CPU PC %06x: audio_result_r %02x\n", machine().device("maincpu")->safe_pc(), ret);

	return ret;
}



/*************************************
 *
 *  Main CPU banking
 *
 *************************************/

static void _set_main_cpu_vector_table_source( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->membank(NEOGEO_BANK_VECTORS)->set_entry(state->m_main_cpu_vector_table_source);
}


static void set_main_cpu_vector_table_source( running_machine &machine, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_main_cpu_vector_table_source = data;

	_set_main_cpu_vector_table_source(machine);
}


static void _set_main_cpu_bank_address( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->membank(NEOGEO_BANK_CARTRIDGE)->set_base(&state->memregion("maincpu")->base()[state->m_main_cpu_bank_address]);
}


void neogeo_set_main_cpu_bank_address( address_space *space, UINT32 bank_address )
{
	neogeo_state *state = space->machine().driver_data<neogeo_state>();

	if (LOG_MAIN_CPU_BANKING) logerror("MAIN CPU PC %06x: neogeo_set_main_cpu_bank_address %06x\n", space->device().safe_pc(), bank_address);

	state->m_main_cpu_bank_address = bank_address;

	_set_main_cpu_bank_address(space->machine());
}


WRITE16_MEMBER(neogeo_state::main_cpu_bank_select_w)
{
	UINT32 bank_address;
	UINT32 len = memregion("maincpu")->bytes();

	if ((len <= 0x100000) && (data & 0x07))
		logerror("PC %06x: warning: bankswitch to %02x but no banks available\n", space.device().safe_pc(), data);
	else
	{
		bank_address = ((data & 0x07) + 1) * 0x100000;

		if (bank_address >= len)
		{
			logerror("PC %06x: warning: bankswitch to empty bank %02x\n", space.device().safe_pc(), data);
			bank_address = 0x100000;
		}

		neogeo_set_main_cpu_bank_address(&space, bank_address);
	}
}


static void main_cpu_banking_init( running_machine &machine )
{
	address_space *mainspace = machine.device("maincpu")->memory().space(AS_PROGRAM);

	/* create vector banks */
	machine.root_device().membank(NEOGEO_BANK_VECTORS)->configure_entry(0, machine.root_device().memregion("mainbios")->base());
	machine.root_device().membank(NEOGEO_BANK_VECTORS)->configure_entry(1, machine.root_device().memregion("maincpu")->base());

	/* set initial main CPU bank */
	if (machine.root_device().memregion("maincpu")->bytes() > 0x100000)
		neogeo_set_main_cpu_bank_address(mainspace, 0x100000);
	else
		neogeo_set_main_cpu_bank_address(mainspace, 0x000000);
}



/*************************************
 *
 *  Audio CPU banking
 *
 *************************************/

static void set_audio_cpu_banking( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	int region;

	for (region = 0; region < 4; region++)
		state->membank(audio_banks[region])->set_entry(state->m_audio_cpu_banks[region]);
}


static void audio_cpu_bank_select( address_space *space, int region, UINT8 bank )
{
	neogeo_state *state = space->machine().driver_data<neogeo_state>();

	if (LOG_AUDIO_CPU_BANKING) logerror("Audio CPU PC %03x: audio_cpu_bank_select: Region: %d   Bank: %02x\n", space->device().safe_pc(), region, bank);

	state->m_audio_cpu_banks[region] = bank;

	set_audio_cpu_banking(space->machine());
}


READ8_MEMBER(neogeo_state::audio_cpu_bank_select_f000_f7ff_r)
{
	audio_cpu_bank_select(&space, 0, offset >> 8);

	return 0;
}


READ8_MEMBER(neogeo_state::audio_cpu_bank_select_e000_efff_r)
{
	audio_cpu_bank_select(&space, 1, offset >> 8);

	return 0;
}


READ8_MEMBER(neogeo_state::audio_cpu_bank_select_c000_dfff_r)
{
	audio_cpu_bank_select(&space, 2, offset >> 8);

	return 0;
}


READ8_MEMBER(neogeo_state::audio_cpu_bank_select_8000_bfff_r)
{
	audio_cpu_bank_select(&space, 3, offset >> 8);

	return 0;
}


static void _set_audio_cpu_rom_source( address_space *space )
{
	neogeo_state *state = space->machine().driver_data<neogeo_state>();

/*  if (!state->memregion("audiobios")->base())   */
		state->m_audio_cpu_rom_source = 1;

	state->membank(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK)->set_entry(state->m_audio_cpu_rom_source);

	/* reset CPU if the source changed -- this is a guess */
	if (state->m_audio_cpu_rom_source != state->m_audio_cpu_rom_source_last)
	{
		state->m_audio_cpu_rom_source_last = state->m_audio_cpu_rom_source;

		space->machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, PULSE_LINE);

		if (LOG_AUDIO_CPU_BANKING) logerror("Audio CPU PC %03x: selectign %s ROM\n", space->device().safe_pc(), state->m_audio_cpu_rom_source ? "CARTRIDGE" : "BIOS");
	}
}


static void set_audio_cpu_rom_source( address_space *space, UINT8 data )
{
	neogeo_state *state = space->machine().driver_data<neogeo_state>();
	state->m_audio_cpu_rom_source = data;

	_set_audio_cpu_rom_source(space);
}


static void audio_cpu_banking_init( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	int region;
	int bank;
	UINT8 *rgn;
	UINT32 address_mask;

	/* audio bios/cartridge selection */
	if (state->memregion("audiobios")->base())
		state->membank(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK)->configure_entry(0, state->memregion("audiobios")->base());
	state->membank(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK)->configure_entry(1, state->memregion("audiocpu")->base());

	/* audio banking */
	address_mask = state->memregion("audiocpu")->bytes() - 0x10000 - 1;

	rgn = state->memregion("audiocpu")->base();
	for (region = 0; region < 4; region++)
	{
		for (bank = 0; bank < 0x100; bank++)
		{
			UINT32 bank_address = 0x10000 + (((bank << (11 + region)) & 0x3ffff) & address_mask);
			state->membank(audio_banks[region])->configure_entry(bank, &rgn[bank_address]);
		}
	}

	/* set initial audio banks --
       how does this really work, or is it even necessary? */
	state->m_audio_cpu_banks[0] = 0x1e;
	state->m_audio_cpu_banks[1] = 0x0e;
	state->m_audio_cpu_banks[2] = 0x06;
	state->m_audio_cpu_banks[3] = 0x02;

	set_audio_cpu_banking(machine);

	state->m_audio_cpu_rom_source_last = 0;
	set_audio_cpu_rom_source(machine.device("maincpu")->memory().space(AS_PROGRAM), 0);
}



/*************************************
 *
 *  System control register
 *
 *************************************/

WRITE16_MEMBER(neogeo_state::system_control_w)
{
	if (ACCESSING_BITS_0_7)
	{
		UINT8 bit = (offset >> 3) & 0x01;

		switch (offset & 0x07)
		{
		default:
		case 0x00: neogeo_set_screen_dark(machine(), bit); break;
		case 0x01: set_main_cpu_vector_table_source(machine(), bit);
				   set_audio_cpu_rom_source(&space, bit); /* this is a guess */
				   break;
		case 0x05: neogeo_set_fixed_layer_source(machine(), bit); break;
		case 0x06: set_save_ram_unlock(machine(), bit); break;
		case 0x07: neogeo_set_palette_bank(machine(), bit); break;

		case 0x02: /* unknown - HC32 middle pin 1 */
		case 0x03: /* unknown - uPD4990 pin ? */
		case 0x04: /* unknown - HC32 middle pin 10 */
			logerror("PC: %x  Unmapped system control write.  Offset: %x  Data: %x\n", space.device().safe_pc(), offset & 0x07, bit);
			break;
		}

		if (LOG_VIDEO_SYSTEM && ((offset & 0x07) != 0x06)) logerror("PC: %x  System control write.  Offset: %x  Data: %x\n", space.device().safe_pc(), offset & 0x07, bit);
	}
}



/*************************************
 *
 *  Watchdog
 *
 *
 *    - The watchdog timer will reset the system after ~0.13 seconds
 *     On an MV-1F MVS system, the following code was used to test:
 *        000100  203C 0001 4F51             MOVE.L   #0x14F51,D0
 *        000106  13C0 0030 0001             MOVE.B   D0,0x300001
 *        00010C  5380                       SUBQ.L   #1,D0
 *        00010E  64FC                       BCC.S    *-0x2 [0x10C]
 *        000110  13C0 0030 0001             MOVE.B   D0,0x300001
 *        000116  60F8                       BRA.S    *-0x6 [0x110]
 *     This code loops long enough to sometimes cause a reset, sometimes not.
 *     The move takes 16 cycles, subq 8, bcc 10 if taken and 8 if not taken, so:
 *     (0x14F51 * 18 + 14) cycles / 12000000 cycles per second = 0.128762 seconds
 *     Newer games force a reset using the following code (this from kof99):
 *        009CDA  203C 0003 0D40             MOVE.L   #0x30D40,D0
 *        009CE0  5380                       SUBQ.L   #1,D0
 *        009CE2  64FC                       BCC.S    *-0x2 [0x9CE0]
 *     Note however that there is a valid code path after this loop.
 *
 *     The watchdog is used as a form of protecetion on a number of games,
 *     previously this was implemented as a specific hack which locked a single
 *     address of SRAM.
 *
 *     What actually happens is if the game doesn't find valid data in the
 *     backup ram it will initialize it, then sit in a loop.  The watchdog
 *     should then reset the system while it is in this loop.  If the watchdog
 *     fails to reset the system the code will continue and set a value in
 *     backup ram to indiate that the protection check has failed.
 *
 *************************************/

WRITE16_MEMBER(neogeo_state::watchdog_w)
{
	/* only an LSB write resets the watchdog */
	if (ACCESSING_BITS_0_7)
	{
		watchdog_reset16_w(space, offset, data, mem_mask);
	}
}



/*************************************
 *
 *  LEDs
 *
 *************************************/

static void set_outputs( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	static const UINT8 led_map[0x10] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x58,0x4c,0x62,0x69,0x78,0x00 };

	/* EL */
	output_set_digit_value(0, led_map[state->m_el_value]);

	/* LED1 */
	output_set_digit_value(1, led_map[state->m_led1_value >> 4]);
	output_set_digit_value(2, led_map[state->m_led1_value & 0x0f]);

	/* LED2 */
	output_set_digit_value(3, led_map[state->m_led2_value >> 4]);
	output_set_digit_value(4, led_map[state->m_led2_value & 0x0f]);
}


static void set_output_latch( running_machine &machine, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	/* looks like the LEDs are set on the
       falling edge */
	UINT8 falling_bits = state->m_output_latch & ~data;

	if (falling_bits & 0x08)
		state->m_el_value = 16 - (state->m_output_data & 0x0f);

	if (falling_bits & 0x10)
		state->m_led1_value = ~state->m_output_data;

	if (falling_bits & 0x20)
		state->m_led2_value = ~state->m_output_data;

	if (falling_bits & 0xc7)
		logerror("%s  Unmaped LED write.  Data: %x\n", machine.describe_context(), falling_bits);

	state->m_output_latch = data;

	set_outputs(machine);
}


static void set_output_data( running_machine &machine, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_output_data = data;
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

static void neogeo_postload(running_machine &machine)
{
	_set_main_cpu_bank_address(machine);
	_set_main_cpu_vector_table_source(machine);
	set_audio_cpu_banking(machine);
	_set_audio_cpu_rom_source(machine.device("maincpu")->memory().space(AS_PROGRAM));
	set_outputs(machine);
}

static MACHINE_START( neogeo )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	/* configure NVRAM */
	machine.device<nvram_device>("saveram")->set_base(save_ram, 0x10000);

	/* set the BIOS bank */
	state->membank(NEOGEO_BANK_BIOS)->set_base(state->memregion("mainbios")->base());

	/* set the initial main CPU bank */
	main_cpu_banking_init(machine);

	/* set the initial audio CPU ROM banks */
	audio_cpu_banking_init(machine);

	create_interrupt_timers(machine);

	/* initialize the memcard data structure */
	memcard_data = auto_alloc_array_clear(machine, UINT8, MEMCARD_SIZE);

	/* start with an IRQ3 - but NOT on a reset */
	state->m_irq3_pending = 1;

	/* get devices */
	state->m_maincpu = machine.device<cpu_device>("maincpu");
	state->m_audiocpu = machine.device<cpu_device>("audiocpu");
	state->m_upd4990a = machine.device("upd4990a");

	/* register state save */
	state->save_item(NAME(state->m_display_position_interrupt_control));
	state->save_item(NAME(state->m_display_counter));
	state->save_item(NAME(state->m_vblank_interrupt_pending));
	state->save_item(NAME(state->m_display_position_interrupt_pending));
	state->save_item(NAME(state->m_irq3_pending));
	state->save_item(NAME(state->m_audio_result));
	state->save_item(NAME(state->m_controller_select));
	state->save_item(NAME(state->m_main_cpu_bank_address));
	state->save_item(NAME(state->m_main_cpu_vector_table_source));
	state->save_item(NAME(state->m_audio_cpu_banks));
	state->save_item(NAME(state->m_audio_cpu_rom_source));
	state->save_item(NAME(state->m_audio_cpu_rom_source_last));
	state->save_item(NAME(state->m_save_ram_unlocked));
	state_save_register_global_pointer(machine, memcard_data, 0x800);
	state->save_item(NAME(state->m_output_data));
	state->save_item(NAME(state->m_output_latch));
	state->save_item(NAME(state->m_el_value));
	state->save_item(NAME(state->m_led1_value));
	state->save_item(NAME(state->m_led2_value));
	state->save_item(NAME(state->m_recurse));

	machine.save().register_postload(save_prepost_delegate(FUNC(neogeo_postload), &machine));
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( neogeo )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	offs_t offs;
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	/* reset system control registers */
	for (offs = 0; offs < 8; offs++)
		state->system_control_w(*space, offs, 0, 0x00ff);

	machine.device("maincpu")->reset();

	neogeo_reset_rng(machine);

	start_interrupt_timers(machine);

	/* trigger the IRQ3 that was set by MACHINE_START */
	update_interrupts(machine);

	state->m_recurse = 0;

	/* AES apparently always uses the cartridge's fixed bank mode */
	// neogeo_set_fixed_layer_source(machine,1);

}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, neogeo_state )
	AM_RANGE(0x000000, 0x00007f) AM_ROMBANK(NEOGEO_BANK_VECTORS)
	AM_RANGE(0x000080, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x0f0000) AM_RAM
	/* some games have protection devices in the 0x200000 region, it appears to map to cart space, not surprising, the ROM is read here too */
	AM_RANGE(0x200000, 0x2fffff) AM_ROMBANK(NEOGEO_BANK_CARTRIDGE)
	AM_RANGE(0x2ffff0, 0x2fffff) AM_WRITE(main_cpu_bank_select_w)
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ff7e) AM_READ_PORT("IN0")
	AM_RANGE(0x300080, 0x300081) AM_MIRROR(0x01ff7e) AM_READ_PORT("IN4")
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ffe0) AM_READ(neogeo_unmapped_r) AM_WRITE(watchdog_w)
	AM_RANGE(0x320000, 0x320001) AM_MIRROR(0x01fffe) AM_READ_PORT("IN3") AM_WRITE(audio_command_w)
	AM_RANGE(0x340000, 0x340001) AM_MIRROR(0x01fffe) AM_READ_PORT("IN1")
	AM_RANGE(0x360000, 0x37ffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x380000, 0x380001) AM_MIRROR(0x01fffe) AM_READ_PORT("IN2")
	AM_RANGE(0x380000, 0x38007f) AM_MIRROR(0x01ff80) AM_WRITE(io_control_w)
	AM_RANGE(0x3a0000, 0x3a001f) AM_MIRROR(0x01ffe0) AM_READ(neogeo_unmapped_r) AM_WRITE(system_control_w)
	AM_RANGE(0x3c0000, 0x3c0007) AM_MIRROR(0x01fff8) AM_READ(neogeo_video_register_r)
	AM_RANGE(0x3c0000, 0x3c000f) AM_MIRROR(0x01fff0) AM_WRITE(neogeo_video_register_w)
	AM_RANGE(0x3e0000, 0x3fffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x400000, 0x401fff) AM_MIRROR(0x3fe000) AM_READWRITE(neogeo_paletteram_r, neogeo_paletteram_w)
	AM_RANGE(0x800000, 0x800fff) AM_READWRITE(memcard_r, memcard_w)
	AM_RANGE(0xc00000, 0xc1ffff) AM_MIRROR(0x0e0000) AM_ROMBANK(NEOGEO_BANK_BIOS)
	AM_RANGE(0xd00000, 0xd0ffff) AM_MIRROR(0x0f0000) AM_RAM_WRITE(save_ram_w) AM_BASE_LEGACY(&save_ram)
	AM_RANGE(0xe00000, 0xffffff) AM_READ(neogeo_unmapped_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, neogeo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_CART_BANK3)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_CART_BANK2)
	AM_RANGE(0xe000, 0xefff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_CART_BANK1)
	AM_RANGE(0xf000, 0xf7ff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_CART_BANK0)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Audio CPU port handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_io_map, AS_IO, 8, neogeo_state )
  /*AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READWRITE(audio_command_r, audio_cpu_clear_nmi_w);*/  /* may not and NMI clear */
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READ(audio_command_r)
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xff00) AM_DEVREADWRITE_LEGACY("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff00) /* write - NMI enable / acknowledge? (the data written doesn't matter) */
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xfff0) AM_MASK(0xfff0) AM_READ(audio_cpu_bank_select_f000_f7ff_r)
	AM_RANGE(0x09, 0x09) AM_MIRROR(0xfff0) AM_MASK(0xfff0) AM_READ(audio_cpu_bank_select_e000_efff_r)
	AM_RANGE(0x0a, 0x0a) AM_MIRROR(0xfff0) AM_MASK(0xfff0) AM_READ(audio_cpu_bank_select_c000_dfff_r)
	AM_RANGE(0x0b, 0x0b) AM_MIRROR(0xfff0) AM_MASK(0xfff0) AM_READ(audio_cpu_bank_select_8000_bfff_r)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xff00) AM_WRITE(audio_result_w)
	AM_RANGE(0x18, 0x18) AM_MIRROR(0xff00) /* write - NMI disable? (the data written doesn't matter) */
ADDRESS_MAP_END



/*************************************
 *
 *  Audio interface
 *
 *************************************/

static const ym2610_interface ym2610_config =
{
	audio_cpu_irq
};



/*************************************
 *
 *  Standard Neo-Geo DIPs and
 *  input port definition
 *
 *************************************/

#define STANDARD_DIPS																		\
	PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" ) PORT_DIPLOCATION("SW:1")					\
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )											\
	PORT_DIPNAME( 0x0002, 0x0002, "Coin Chutes?" ) PORT_DIPLOCATION("SW:2")					\
	PORT_DIPSETTING(	  0x0000, "1?" )													\
	PORT_DIPSETTING(	  0x0002, "2?" )													\
	PORT_DIPNAME( 0x0004, 0x0004, "Autofire (in some games)" ) PORT_DIPLOCATION("SW:3")		\
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )											\
	PORT_DIPNAME( 0x0018, 0x0018, "COMM Setting (Cabinet No.)" ) PORT_DIPLOCATION("SW:4,5")	\
	PORT_DIPSETTING(	  0x0018, "1" )														\
	PORT_DIPSETTING(	  0x0010, "2" )														\
	PORT_DIPSETTING(	  0x0008, "3" )														\
	PORT_DIPSETTING(	  0x0000, "4" )														\
	PORT_DIPNAME( 0x0020, 0x0020, "COMM Setting (Link Enable)" ) PORT_DIPLOCATION("SW:6")	\
	PORT_DIPSETTING(	  0x0020, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )											\
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW:7")			\
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )											\
	PORT_DIPNAME( 0x0080, 0x0080, "Freeze" ) PORT_DIPLOCATION("SW:8")						\
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )


#define STANDARD_IN0														\
	PORT_START("IN0")														\
	STANDARD_DIPS															\
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)		\
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)		\
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)		\
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)	\
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)			\
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)			\
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)			\
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)


#define STANDARD_IN1														\
	PORT_START("IN1")														\
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )							\
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)		\
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)		\
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)		\
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)	\
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)			\
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)			\
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)			\
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)


#define STANDARD_IN2																				\
	PORT_START("IN2")																				\
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )													\
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )   												\
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Next Game") PORT_CODE(KEYCODE_7)		\
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2 )   												\
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Previous Game") PORT_CODE(KEYCODE_8)	\
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, neogeo_state,get_memcard_status, NULL)			\
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* In AES 'mode' nitd, kof2000, sengoku3, matrim and mslug5 check if this is ACTIVE_HIGH */


#define STANDARD_IN3																				\
	PORT_START("IN3")																				\
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )													\
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )													\
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )													\
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* having this ACTIVE_HIGH causes you to start with 2 credits using USA bios roms; if ACTIVE_HIGH + IN4 bit 6 ACTIVE_HIGH = AES 'mode' */	\
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* having this ACTIVE_HIGH causes you to start with 2 credits using USA bios roms; if ACTIVE_HIGH + IN4 bit 6 ACTIVE_HIGH = AES 'mode' */	\
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL ) /* what is this? When ACTIVE_HIGH + IN4 bit 6 ACTIVE_LOW MVS-4 slot is detected */	\
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, neogeo_state,get_calendar_status, NULL)			\
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, neogeo_state,get_audio_result, NULL)


#define STANDARD_IN4																			\
	PORT_START("IN4")																			\
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* what is this? If ACTIVE_LOW, MVS-6 slot detected, when ACTIVE_HIGH MVS-1 slot (AES) detected */	\
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter BIOS") PORT_CODE(KEYCODE_F2)	\
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )


static INPUT_PORTS_START( neogeo )
	STANDARD_IN0

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4
INPUT_PORTS_END






DEVICE_IMAGE_LOAD( neo_cartridge )
{
	UINT32 size;
	device_t* ym = image.device().machine().device("ymsnd");

	// first check software list
	if(image.software_entry() != NULL)
	{
		// create memory regions
		size = image.get_software_region_length("maincpu");
		image.device().machine().memory().region_free(":maincpu");
		image.device().machine().memory().region_alloc(":maincpu",size, 2, ENDIANNESS_BIG);
		memcpy(image.device().machine().root_device().memregion("maincpu")->base(),image.get_software_region("maincpu"),size);

		// for whatever reason (intentional, or design flaw) software loaded via software lists is swapped in endianess vs. the standard ROM loading, regardless of the above.  Swap it to keep consistency
		for (int i=0; i<size/2;i++)
		{
			UINT16* ROM = (UINT16*)image.device().machine().root_device().memregion("maincpu")->base();
			ROM[i] = ((ROM[i]&0xff00)>>8) | ((ROM[i]&0x00ff)<<8);
		}

		size = image.get_software_region_length("fixed");
		image.device().machine().memory().region_free(":fixed");
		image.device().machine().memory().region_alloc(":fixed",size,1, ENDIANNESS_LITTLE);
		memcpy(image.device().machine().root_device().memregion("fixed")->base(),image.get_software_region("fixed"),size);

		if(image.get_software_region("audiocpu") != NULL)
		{
			size = image.get_software_region_length("audiocpu");
			image.device().machine().memory().region_free(":audiocpu");
			image.device().machine().memory().region_alloc(":audiocpu",size+0x10000,1, ENDIANNESS_LITTLE);
			memcpy(image.device().machine().root_device().memregion("audiocpu")->base(),image.get_software_region("audiocpu"),size);
			memcpy(image.device().machine().root_device().memregion("audiocpu")->base()+0x10000,image.get_software_region("audiocpu"),size); // avoid reloading in XML, should just improve banking instead tho?
		}


		size = image.get_software_region_length("ymsnd");
		image.device().machine().memory().region_free(":ymsnd");
		image.device().machine().memory().region_alloc(":ymsnd",size,1, ENDIANNESS_LITTLE);
		memcpy(image.device().machine().root_device().memregion("ymsnd")->base(),image.get_software_region("ymsnd"),size);
		if(image.get_software_region("ymsnd.deltat") != NULL)
		{
			size = image.get_software_region_length("ymsnd.deltat");
			image.device().machine().memory().region_free(":ymsnd.deltat");
			image.device().machine().memory().region_alloc(":ymsnd.deltat",size,1, ENDIANNESS_LITTLE);
			memcpy(image.device().machine().root_device().memregion("ymsnd.deltat")->base(),image.get_software_region("ymsnd.deltat"),size);
		}
		else
			image.device().machine().memory().region_free(":ymsnd.deltat");  // removing the region will fix sound glitches in non-Delta-T games
		ym->reset();
		size = image.get_software_region_length("sprites");
		image.device().machine().memory().region_free(":sprites");
		image.device().machine().memory().region_alloc(":sprites",size,1, ENDIANNESS_LITTLE);
		memcpy(image.device().machine().root_device().memregion("sprites")->base(),image.get_software_region("sprites"),size);
		if(image.get_software_region("audiocrypt") != NULL)  // encrypted Z80 code
		{
			size = image.get_software_region_length("audiocrypt");
			image.device().machine().memory().region_alloc(":audiocrypt",size,1, ENDIANNESS_LITTLE);
			memcpy(image.device().machine().root_device().memregion("audiocrypt")->base(),image.get_software_region("audiocrypt"),size);
			// allocate the audiocpu region to decrypt data into
			image.device().machine().memory().region_free(":audiocpu");
			image.device().machine().memory().region_alloc(":audiocpu",size+0x10000,1, ENDIANNESS_LITTLE);
		}

		// setup cartridge ROM area
		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x000080,0x0fffff,"cart_rom");
		image.device().machine().root_device().membank("cart_rom")->set_base(&image.device().machine().root_device().memregion("maincpu")->base()[0x80]);

		// handle possible protection
		neogeo_state *state = image.device().machine().driver_data<neogeo_state>();
		state->mvs_install_protection(image);

		return IMAGE_INIT_PASS;
	}
	return IMAGE_INIT_FAIL;
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( neogeo, neogeo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, NEOGEO_MAIN_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("audiocpu", Z80, NEOGEO_AUDIO_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_IO_MAP(audio_io_map)

	MCFG_WATCHDOG_TIME_INIT(attotime::from_usec(128762))

	MCFG_MACHINE_START(neogeo)
	MCFG_MACHINE_RESET(neogeo)
	MCFG_NVRAM_ADD_0FILL("saveram")
	MCFG_MEMCARD_HANDLER(neogeo)

	/* video hardware */
	MCFG_VIDEO_START(neogeo)
	MCFG_VIDEO_RESET(neogeo)
	MCFG_DEFAULT_LAYOUT(layout_neogeo)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(NEOGEO_PIXEL_CLOCK, NEOGEO_HTOTAL, NEOGEO_HBEND, NEOGEO_HBSTART, NEOGEO_VTOTAL, NEOGEO_VBEND, NEOGEO_VBSTART)
	MCFG_SCREEN_UPDATE_STATIC(neogeo)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, NEOGEO_YM2610_CLOCK)
	MCFG_SOUND_CONFIG(ym2610_config)
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.60)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)

	/* NEC uPD4990A RTC */
	MCFG_UPD4990A_ADD("upd4990a")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mvs, neogeo )

	MCFG_MEMCARD_HANDLER(neogeo)

    MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_LOAD(neo_cartridge)
	MCFG_CARTSLOT_INTERFACE("neo_cart")

	MCFG_SOFTWARE_LIST_ADD("cart_list","neogeo")
MACHINE_CONFIG_END


/*************************************
 *
 *  Driver initalization
 *
 *************************************/

DRIVER_INIT_MEMBER(neogeo_state,neogeo)
{
	m_fixed_layer_bank_type = 0;
}


#include "neodrvr.c"
