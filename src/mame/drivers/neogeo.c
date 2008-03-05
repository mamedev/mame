/***************************************************************************

    Neo-Geo hardware

    main driver file - please do NOT put anything specific to a
    particular game in this file, but use neodrvr.c instead

    Credits:
        * This driver was made possible by the research done by
          Charles MacDonald.  For a detailed description of the Neo-Geo
          hardware, please visit his page at:
          http://cgfm2.emuviews.com/temp/mvstech.txt
        * Presented to you by the Shin Emu Keikaku team.
        * The following people have all spent probably far
          too much time on this:
          AVDB
          Bryan McPhail
          Fuzz
          Ernesto Corvi
          Andrew Prime
          Zsolt Vasvari


    Neogeo Motherboard (info - courtesy of Guru):

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


    Known issues/to-do's:

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

****************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "neogeo.h"
#include "machine/pd4990a.h"
#include "cpu/z80/z80.h"
#include "sound/2610intf.h"

#include "neogeo.lh"


#define LOG_VIDEO_SYSTEM		(0)
#define LOG_CPU_COMM			(0)
#define LOG_MAIN_CPU_BANKING	(0)
#define LOG_AUDIO_CPU_BANKING	(0)



/*************************************
 *
 *  Global variables
 *
 *************************************/

static UINT8 display_position_interrupt_control;
static UINT32 display_counter;
static UINT32 vblank_interrupt_pending;
static UINT32 display_position_interrupt_pending;
static UINT32 irq3_pending;
static emu_timer *display_position_interrupt_timer;
static emu_timer *display_position_vblank_timer;
static emu_timer *vblank_interrupt_timer;

static UINT8 controller_select;

static UINT8 *memcard_data;

static UINT32 main_cpu_bank_address;
static UINT8 main_cpu_vector_table_source;

static UINT8 audio_result;
static UINT8 audio_cpu_banks[4];
static UINT8 audio_cpu_rom_source;
static UINT8 audio_cpu_rom_source_last;

static UINT16 *save_ram;
static UINT8 save_ram_unlocked;

static UINT8 output_data;
static UINT8 output_latch;
static UINT8 el_value;
static UINT8 led1_value;
static UINT8 led2_value;



/*************************************
 *
 *  Forward declerations
 *
 *************************************/

static void calendar_clock(void);
static void set_output_latch(UINT8 data);
static void set_output_data(UINT8 data);



/*************************************
 *
 *  Main CPU interrupt generation
 *
 *************************************/

#define IRQ2CTRL_ENABLE				(0x10)
#define IRQ2CTRL_LOAD_RELATIVE		(0x20)
#define IRQ2CTRL_AUTOLOAD_VBLANK	(0x40)
#define IRQ2CTRL_AUTOLOAD_REPEAT	(0x80)


static void adjust_display_position_interrupt_timer(void)
{
	if ((display_counter + 1) != 0)
	{
		attotime period = attotime_mul(ATTOTIME_IN_HZ(NEOGEO_PIXEL_CLOCK), display_counter + 1);
		if (LOG_VIDEO_SYSTEM) logerror("adjust_display_position_interrupt_timer  current y: %02x  current x: %02x   target y: %x  target x: %x\n", video_screen_get_vpos(0), video_screen_get_hpos(0), (display_counter + 1) / NEOGEO_HTOTAL, (display_counter + 1) % NEOGEO_HTOTAL);

		timer_adjust_oneshot(display_position_interrupt_timer, period, 0);
	}
}


void neogeo_set_display_position_interrupt_control(UINT16 data)
{
	display_position_interrupt_control = data;
}


void neogeo_set_display_counter_msb(UINT16 data)
{
	display_counter = (display_counter & 0x0000ffff) | ((UINT32)data << 16);

	if (LOG_VIDEO_SYSTEM) logerror("PC %06x: set_display_counter %08x\n", activecpu_get_pc(), display_counter);
}


void neogeo_set_display_counter_lsb(UINT16 data)
{
	display_counter = (display_counter & 0xffff0000) | data;

	if (LOG_VIDEO_SYSTEM) logerror("PC %06x: set_display_counter %08x\n", activecpu_get_pc(), display_counter);

	if (display_position_interrupt_control & IRQ2CTRL_LOAD_RELATIVE)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_RELATIVE ");
 		adjust_display_position_interrupt_timer();
	}
}


static void update_interrupts(running_machine *machine)
{
	int level = 0;

	/* determine which interrupt is active - order implies priority */
	if (vblank_interrupt_pending) level = 1;
	if (display_position_interrupt_pending) level = 2;
	if (irq3_pending) level = 3;

	/* either set or clear the appropriate lines */
	if (level)
		cpunum_set_input_line(machine, 0, level, ASSERT_LINE);
	else
		cpunum_set_input_line(machine, 0, 7, CLEAR_LINE);
}


void neogeo_acknowledge_interrupt(UINT16 data)
{
	if (data & 0x01) irq3_pending = 0;
	if (data & 0x02) display_position_interrupt_pending = 0;
	if (data & 0x04) vblank_interrupt_pending = 0;

	update_interrupts(Machine);
}


static TIMER_CALLBACK( display_position_interrupt_callback )
{
	if (LOG_VIDEO_SYSTEM) logerror("--- Scanline @ %d,%d\n", video_screen_get_vpos(0), video_screen_get_hpos(0));
	if (display_position_interrupt_control & IRQ2CTRL_ENABLE)
	{
		if (LOG_VIDEO_SYSTEM) logerror("*** Scanline interrupt (IRQ2) ***  y: %02x  x: %02x\n", video_screen_get_vpos(0), video_screen_get_hpos(0));
		display_position_interrupt_pending = 1;

		update_interrupts(machine);
	}

	if (display_position_interrupt_control & IRQ2CTRL_AUTOLOAD_REPEAT)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_REPEAT ");
		adjust_display_position_interrupt_timer();
	}
}


static TIMER_CALLBACK( display_position_vblank_callback )
{
	if (display_position_interrupt_control & IRQ2CTRL_AUTOLOAD_VBLANK)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_VBLANK ");
		adjust_display_position_interrupt_timer();
	}

	/* set timer for next screen */
	timer_adjust_oneshot(display_position_vblank_timer, video_screen_get_time_until_pos(0, NEOGEO_VBSTART, NEOGEO_VBLANK_RELOAD_HPOS), 0);
}


static TIMER_CALLBACK( vblank_interrupt_callback )
{
	if (LOG_VIDEO_SYSTEM) logerror("+++ VBLANK @ %d,%d\n", video_screen_get_vpos(0), video_screen_get_hpos(0));

	/* add a timer tick to the pd4990a */
	calendar_clock();

	vblank_interrupt_pending = 1;

	update_interrupts(machine);

	/* set timer for next screen */
	timer_adjust_oneshot(vblank_interrupt_timer, video_screen_get_time_until_pos(0, NEOGEO_VBSTART, 0), 0);
}


static void create_interrupt_timers(void)
{
	display_position_interrupt_timer = timer_alloc(display_position_interrupt_callback, NULL);
	display_position_vblank_timer = timer_alloc(display_position_vblank_callback, NULL);
	vblank_interrupt_timer = timer_alloc(vblank_interrupt_callback, NULL);
}


static void start_interrupt_timers(void)
{
	timer_adjust_oneshot(vblank_interrupt_timer, video_screen_get_time_until_pos(0, NEOGEO_VBSTART, 0), 0);
	timer_adjust_oneshot(display_position_vblank_timer, video_screen_get_time_until_pos(0, NEOGEO_VBSTART, NEOGEO_VBLANK_RELOAD_HPOS), 0);
}



/*************************************
 *
 *  Audio CPU interrupt generation
 *
 *************************************/

static void audio_cpu_irq(int assert)
{
	cpunum_set_input_line(Machine, 1, 0, assert ? ASSERT_LINE : CLEAR_LINE);
}


static void audio_cpu_assert_nmi(void)
{
	cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, ASSERT_LINE);
}


static WRITE8_HANDLER( audio_cpu_clear_nmi_w )
{
	cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, CLEAR_LINE);
}



/*************************************
 *
 *  Input ports / Controllers
 *
 *************************************/

static void select_controller(UINT8 data)
{
	controller_select = data;
}


static CUSTOM_INPUT( multiplexed_controller_r )
{
	char tag[6];

	sprintf(tag, "IN%s-%d", (const char *)param, controller_select & 0x01);

	return readinputportbytag(tag);
}


static CUSTOM_INPUT( mahjong_controller_r )
{
	UINT32 ret;

/*
cpu #0 (PC=00C18B9A): unmapped memory word write to 00380000 = 0012 & 00FF
cpu #0 (PC=00C18BB6): unmapped memory word write to 00380000 = 001B & 00FF
cpu #0 (PC=00C18D54): unmapped memory word write to 00380000 = 0024 & 00FF
cpu #0 (PC=00C18D6C): unmapped memory word write to 00380000 = 0009 & 00FF
cpu #0 (PC=00C18C40): unmapped memory word write to 00380000 = 0000 & 00FF
*/
	switch (controller_select)
	{
	default:
	case 0x00: ret = 0x0000; break; /* nothing? */
	case 0x09: ret = readinputportbytag("MAHJONG1"); break;
	case 0x12: ret = readinputportbytag("MAHJONG2"); break;
	case 0x1b: ret = readinputportbytag("MAHJONG3"); break; /* player 1 normal inputs? */
	case 0x24: ret = readinputportbytag("MAHJONG4"); break;
	}

	return ret;
}


static WRITE16_HANDLER( io_control_w )
{
	switch (offset)
	{
	case 0x00: select_controller(data & 0x00ff); break;
	case 0x18: set_output_latch(data & 0x00ff); break;
	case 0x20: set_output_data(data & 0x00ff); break;
	case 0x28: pd4990a_control_16_w(machine, 0, data, mem_mask); break;
//  case 0x30: break; // coin counters
//  case 0x31: break; // coin counters
//  case 0x32: break; // coin lockout
//  case 0x33: break; // coui lockout

	default:
		logerror("PC: %x  Unmapped I/O control write.  Offset: %x  Data: %x\n", activecpu_get_pc(), offset, data);
		break;
	}
}



/*************************************
 *
 *  Unmapped memory access
 *
 *************************************/

READ16_HANDLER( neogeo_unmapped_r )
{
	static UINT8 recurse = 0;
	UINT16 ret;

	/* unmapped memory returns the last word on the data bus, which is almost always the opcode
       of the next instruction due to prefetch */

	/* prevent recursion */
	if (recurse)
		ret = 0xffff;
	else
	{
		recurse = 1;
		ret = program_read_word(activecpu_get_pc());
		recurse = 0;
	}

	return ret;
}



/*************************************
 *
 *  uPD4990A calendar chip
 *
 *************************************/

static void calendar_init(running_machine *machine)
{
	/* set the celander IC to 'right now' */
	mame_system_time systime;
	mame_system_tm time;

	mame_get_base_datetime(machine, &systime);
	time = systime.local_time;

	pd4990a.seconds = ((time.second / 10) << 4) + (time.second % 10);
	pd4990a.minutes = ((time.minute / 10) << 4) + (time.minute % 10);
	pd4990a.hours = ((time.hour / 10) <<4 ) + (time.hour % 10);
	pd4990a.days = ((time.mday / 10) << 4) + (time.mday % 10);
	pd4990a.month = time.month + 1;
	pd4990a.year = ((((time.year - 1900) % 100) / 10) << 4) + ((time.year - 1900) % 10);
	pd4990a.weekday = time.weekday;
}


static void calendar_clock(void)
{
	pd4990a_addretrace();
}


static CUSTOM_INPUT( get_calendar_status )
{
	return (pd4990a_databit_r(machine, 0) << 1) | pd4990a_testbit_r(machine, 0);
}



/*************************************
 *
 *  NVRAM (Save RAM)
 *
 *************************************/

static NVRAM_HANDLER( neogeo )
{
	if (read_or_write)
		/* save the SRAM settings */
		mame_fwrite(file, save_ram, 0x2000);
	else
	{
		/* load the SRAM settings */
		if (file)
			mame_fread(file, save_ram, 0x2000);
		else
			memset(save_ram, 0, 0x10000);
	}
}


static void set_save_ram_unlock(UINT8 data)
{
	save_ram_unlocked = data;
}


static WRITE16_HANDLER( save_ram_w )
{
	if (save_ram_unlocked)
		COMBINE_DATA(&save_ram[offset]);
}



/*************************************
 *
 *  Memory card
 *
 *************************************/

#define MEMCARD_SIZE	0x0800


static void memcard_init(void)
{
	memcard_data = auto_malloc(MEMCARD_SIZE);
	memset(memcard_data, 0, MEMCARD_SIZE);
}


static CUSTOM_INPUT( get_memcard_status )
{
	/* D0 and D1 are memcard presence indicators, D2 indicates memcard
       write protect status (we are always write enabled) */
	return (memcard_present() == -1) ? 0x07 : 0x00;
}


static READ16_HANDLER( memcard_r )
{
	UINT16 ret;

	if (memcard_present() != -1)
		ret = memcard_data[offset] | 0xff00;
	else
		ret = 0xffff;

	return ret;
}


static WRITE16_HANDLER( memcard_w )
{
	if (ACCESSING_LSB)
	{
		if (memcard_present() != -1)
			memcard_data[offset] = data;
	}
}


static MEMCARD_HANDLER( neogeo )
{
	switch (action)
	{
	case MEMCARD_CREATE:
		memset(memcard_data, 0, MEMCARD_SIZE);
		mame_fwrite(file, memcard_data, MEMCARD_SIZE);
		break;

	case MEMCARD_INSERT:
		mame_fread(file, memcard_data, MEMCARD_SIZE);
		break;

	case MEMCARD_EJECT:
		mame_fwrite(file, memcard_data, MEMCARD_SIZE);
		break;
	}
}



/*************************************
 *
 *  Inter-CPU communications
 *
 *************************************/

static WRITE16_HANDLER( audio_command_w )
{
	/* accessing the LSB only is not mapped */
	if (mem_mask != 0xff00)
	{
		soundlatch_w(machine, 0, data >> 8);

		audio_cpu_assert_nmi();

		/* boost the interleave to let the audio CPU read the command */
		cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(50));

		if (LOG_CPU_COMM) logerror("MAIN CPU PC %06x: audio_command_w %04x - %04x\n", activecpu_get_pc(), data, mem_mask);
	}
}


static READ8_HANDLER( audio_command_r )
{
	UINT8 ret = soundlatch_r(machine, 0);

	if (LOG_CPU_COMM) logerror(" AUD CPU PC   %04x: audio_command_r %02x\n", activecpu_get_pc(), ret);

	/* this is a guess */
	audio_cpu_clear_nmi_w(machine, 0, 0);

	return ret;
}


static WRITE8_HANDLER( audio_result_w )
{
	if (LOG_CPU_COMM && (audio_result != data)) logerror(" AUD CPU PC   %04x: audio_result_w %02x\n", activecpu_get_pc(), data);

	audio_result = data;
}


static CUSTOM_INPUT( get_audio_result )
{
	UINT32 ret = audio_result;

//  if (LOG_CPU_COMM && (cpu_getactivecpu() >= 0)) logerror("MAIN CPU PC %06x: audio_result_r %02x\n", activecpu_get_pc(), ret);

	return ret;
}



/*************************************
 *
 *  Main CPU banking
 *
 *************************************/

static void _set_main_cpu_vector_table_source(void)
{
	memory_set_bank(NEOGEO_BANK_VECTORS, main_cpu_vector_table_source);
}


static void set_main_cpu_vector_table_source(UINT8 data)
{
	main_cpu_vector_table_source = data;

	_set_main_cpu_vector_table_source();
}


static void _set_main_cpu_bank_address(void)
{
	memory_set_bankptr(NEOGEO_BANK_CARTRIDGE, &memory_region(NEOGEO_REGION_MAIN_CPU_CARTRIDGE)[main_cpu_bank_address]);
}


void neogeo_set_main_cpu_bank_address(UINT32 bank_address)
{
	if (LOG_MAIN_CPU_BANKING) logerror("MAIN CPU PC %06x: neogeo_set_main_cpu_bank_address %06x\n", safe_activecpu_get_pc(), bank_address);

	main_cpu_bank_address = bank_address;

	_set_main_cpu_bank_address();
}


static WRITE16_HANDLER( main_cpu_bank_select_w )
{
	UINT32 bank_address;

	if ((memory_region_length(NEOGEO_REGION_MAIN_CPU_CARTRIDGE) <= 0x100000) && (data & 0x07))
		logerror("PC %06x: warning: bankswitch to %02x but no banks available\n", activecpu_get_pc(), data);
	else
	{
		bank_address = ((data & 0x07) + 1) * 0x100000;

		if (bank_address >= memory_region_length(NEOGEO_REGION_MAIN_CPU_CARTRIDGE))
		{
			logerror("PC %06x: warning: bankswitch to empty bank %02x\n", activecpu_get_pc(), data);
			bank_address = 0x100000;
		}

		neogeo_set_main_cpu_bank_address(bank_address);
	}
}


static void main_cpu_banking_init(void)
{
	/* create vector banks */
	memory_configure_bank(NEOGEO_BANK_VECTORS, 0, 1, memory_region(NEOGEO_REGION_MAIN_CPU_BIOS), 0);
	memory_configure_bank(NEOGEO_BANK_VECTORS, 1, 1, memory_region(NEOGEO_REGION_MAIN_CPU_CARTRIDGE), 0);

	/* set initial main CPU bank */
	if (memory_region_length(NEOGEO_REGION_MAIN_CPU_CARTRIDGE) > 0x100000)
		neogeo_set_main_cpu_bank_address(0x100000);
	else
		neogeo_set_main_cpu_bank_address(0x000000);
}



/*************************************
 *
 *  Audio CPU banking
 *
 *************************************/

static void set_audio_cpu_banking(void)
{
	int region;

	for (region = 0; region < 4; region++)
		memory_set_bank(NEOGEO_BANK_AUDIO_CPU_CART_BANK + region, audio_cpu_banks[region]);
}


static void audio_cpu_bank_select(int region, UINT8 bank)
{
	if (LOG_AUDIO_CPU_BANKING) logerror("Audio CPU PC %03x: audio_cpu_bank_select: Region: %d   Bank: %02x\n", safe_activecpu_get_pc(), region, bank);

	audio_cpu_banks[region] = bank;

	set_audio_cpu_banking();
}


static READ8_HANDLER( audio_cpu_bank_select_f000_f7ff_r )
{
	audio_cpu_bank_select(0, offset >> 8);

	return 0;
}


static READ8_HANDLER( audio_cpu_bank_select_e000_efff_r )
{
	audio_cpu_bank_select(1, offset >> 8);

	return 0;
}


static READ8_HANDLER( audio_cpu_bank_select_c000_dfff_r )
{
	audio_cpu_bank_select(2, offset >> 8);

	return 0;
}


static READ8_HANDLER( audio_cpu_bank_select_8000_bfff_r )
{
	audio_cpu_bank_select(3, offset >> 8);

	return 0;
}


static void _set_audio_cpu_rom_source(void)
{
/*  if (!memory_region(NEOGEO_REGION_AUDIO_CPU_BIOS))   */
		audio_cpu_rom_source = 1;

	memory_set_bank(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK, audio_cpu_rom_source);

	/* reset CPU if the source changed -- this is a guess */
	if (audio_cpu_rom_source != audio_cpu_rom_source_last)
	{
		audio_cpu_rom_source_last = audio_cpu_rom_source;

		cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, PULSE_LINE);

		if (LOG_AUDIO_CPU_BANKING) logerror("Audio CPU PC %03x: selectign %s ROM\n", safe_activecpu_get_pc(), audio_cpu_rom_source ? "CARTRIDGE" : "BIOS");
	}
}


static void set_audio_cpu_rom_source(UINT8 data)
{
	audio_cpu_rom_source = data;

	_set_audio_cpu_rom_source();
}


static void audio_cpu_banking_init(void)
{
	int region;
	int bank;
	UINT32 address_mask;

	/* audio bios/cartridge selection */
 	if (memory_region(NEOGEO_REGION_AUDIO_CPU_BIOS))
		memory_configure_bank(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK, 0, 1, memory_region(NEOGEO_REGION_AUDIO_CPU_BIOS), 0);
	memory_configure_bank(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK, 1, 1, memory_region(NEOGEO_REGION_AUDIO_CPU_CARTRIDGE), 0);

	/* audio banking */
	address_mask = memory_region_length(NEOGEO_REGION_AUDIO_CPU_CARTRIDGE) - 0x10000 - 1;

	for (region = 0; region < 4; region++)
	{
		for (bank = 0; bank < 0x100; bank++)
		{
			UINT32 bank_address = 0x10000 + (((bank << (11 + region)) & 0x3ffff) & address_mask);
			memory_configure_bank(NEOGEO_BANK_AUDIO_CPU_CART_BANK + region, bank, 1, &memory_region(NEOGEO_REGION_AUDIO_CPU_CARTRIDGE)[bank_address], 0);
		}
	}

	/* set initial audio banks --
       how does this really work, or is it even neccessary? */
	audio_cpu_banks[0] = 0x1e;
	audio_cpu_banks[1] = 0x0e;
	audio_cpu_banks[2] = 0x06;
	audio_cpu_banks[3] = 0x02;

	set_audio_cpu_banking();

	audio_cpu_rom_source_last = 0;
	set_audio_cpu_rom_source(0);
}



/*************************************
 *
 *  System control register
 *
 *************************************/

static WRITE16_HANDLER( system_control_w )
{
	if ((mem_mask & 0x00ff) != 0x00ff)
	{
		UINT8 bit = (offset >> 3) & 0x01;

		switch (offset & 0x07)
		{
		default:
		case 0x00: neogeo_set_screen_dark(bit); break;
		case 0x01: set_main_cpu_vector_table_source(bit);
				   set_audio_cpu_rom_source(bit); /* this is a guess */
				   break;
		case 0x05: neogeo_set_fixed_layer_source(bit); break;
		case 0x06: set_save_ram_unlock(bit); break;
		case 0x07: neogeo_set_palette_bank(bit); break;

		case 0x02: /* unknown - HC32 middle pin 1 */
		case 0x03: /* unknown - uPD4990 pin ? */
		case 0x04: /* unknown - HC32 middle pin 10 */
			logerror("PC: %x  Unmapped system control write.  Offset: %x  Data: %x\n", safe_activecpu_get_pc(), offset & 0x07, bit);
			break;
		}

		if (LOG_VIDEO_SYSTEM && ((offset & 0x07) != 0x06)) logerror("PC: %x  System control write.  Offset: %x  Data: %x\n", safe_activecpu_get_pc(), offset & 0x07, bit);
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

static WRITE16_HANDLER( watchdog_w )
{
	/* only an LSB write resets the watchdog */
	if (ACCESSING_LSB)
	{
		watchdog_reset16_w(machine, offset, data, mem_mask);
	}
}



/*************************************
 *
 *  LEDs
 *
 *************************************/

static void set_outputs(void)
{
	static const UINT8 led_map[0x10] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x58,0x4c,0x62,0x69,0x78,0x00 };

	/* EL */
	output_set_digit_value(0, led_map[el_value]);

	/* LED1 */
	output_set_digit_value(1, led_map[led1_value >> 4]);
	output_set_digit_value(2, led_map[led1_value & 0x0f]);

	/* LED2 */
	output_set_digit_value(3, led_map[led2_value >> 4]);
	output_set_digit_value(4, led_map[led2_value & 0x0f]);
}


static void set_output_latch(UINT8 data)
{
	/* looks like the LEDs are set on the
       falling edge */
    UINT8 falling_bits = output_latch & ~data;

    if (falling_bits & 0x08)
    	el_value = 16 - (output_data & 0x0f);

    if (falling_bits & 0x10)
    	led1_value = ~output_data;

    if (falling_bits & 0x20)
    	led2_value = ~output_data;

  	if (falling_bits & 0xc7)
		logerror("PC: %x  Unmaped LED write.  Data: %x\n", activecpu_get_pc(), falling_bits);

	output_latch = data;

	set_outputs();
}


static void set_output_data(UINT8 data)
{
	output_data = data;
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

static MACHINE_START( neogeo )
{
	/* set the BIOS bank */
	memory_set_bankptr(NEOGEO_BANK_BIOS, memory_region(NEOGEO_REGION_MAIN_CPU_BIOS));

	/* set the initial main CPU bank */
	main_cpu_banking_init();

	/* set the initial audio CPU ROM banks */
	audio_cpu_banking_init();

	create_interrupt_timers();

	/* initialize the celander IC to 'right now' */
	calendar_init(machine);

	/* initialize the memcard data structure */
	memcard_init();

	/* start with an IRQ3 - but NOT on a reset */
	irq3_pending = 1;

	/* register state save */
	state_save_register_global(display_position_interrupt_control);
	state_save_register_global(display_counter);
	state_save_register_global(vblank_interrupt_pending);
	state_save_register_global(display_position_interrupt_pending);
	state_save_register_global(irq3_pending);
	state_save_register_global(audio_result);
	state_save_register_global(controller_select);
	state_save_register_global(main_cpu_bank_address);
	state_save_register_global(main_cpu_vector_table_source);
	state_save_register_global_array(audio_cpu_banks);
	state_save_register_global(audio_cpu_rom_source);
	state_save_register_global(audio_cpu_rom_source_last);
	state_save_register_global(save_ram_unlocked);
	state_save_register_global_pointer(memcard_data, 0x800);
	state_save_register_global(output_data);
	state_save_register_global(output_latch);
	state_save_register_global(el_value);
	state_save_register_global(led1_value);
	state_save_register_global(led2_value);

	state_save_register_func_postload(_set_main_cpu_bank_address);
	state_save_register_func_postload(_set_main_cpu_vector_table_source);
	state_save_register_func_postload(set_audio_cpu_banking);
	state_save_register_func_postload(_set_audio_cpu_rom_source);
	state_save_register_func_postload(set_outputs);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( neogeo )
{
	offs_t offs;

	/* reset system control registers */
	for (offs = 0; offs < 8; offs++)
		system_control_w(machine, offs, 0, 0xff00);

	neogeo_reset_rng();

	start_interrupt_timers();

	/* trigger the IRQ3 that was set by MACHINE_START */
	update_interrupts(machine);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00007f) AM_ROMBANK(NEOGEO_BANK_VECTORS)
	AM_RANGE(0x000080, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x0f0000) AM_RAM
	/* some games have protection devices in the 0x200000 region, it appears to map to cart space, not surprising, the ROM is read here too */
	AM_RANGE(0x200000, 0x2fffff) AM_ROMBANK(NEOGEO_BANK_CARTRIDGE)
	AM_RANGE(0x2ffff0, 0x2fffff) AM_WRITE(main_cpu_bank_select_w)
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ff7e) AM_READ_PORT("IN0")
	AM_RANGE(0x300080, 0x300081) AM_MIRROR(0x01ff7e) AM_READ_PORT("IN4")
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ffe0) AM_READWRITE(neogeo_unmapped_r, watchdog_w)
	AM_RANGE(0x320000, 0x320001) AM_MIRROR(0x01fffe) AM_READ_PORT("IN3") AM_WRITE(audio_command_w)
	AM_RANGE(0x340000, 0x340001) AM_MIRROR(0x01fffe) AM_READ_PORT("IN1")
	AM_RANGE(0x360000, 0x37ffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x380000, 0x380001) AM_MIRROR(0x01fffe) AM_READ_PORT("IN2")
	AM_RANGE(0x380000, 0x38007f) AM_MIRROR(0x01ff80) AM_WRITE(io_control_w)
	AM_RANGE(0x3a0000, 0x3a001f) AM_MIRROR(0x01ffe0) AM_READWRITE(neogeo_unmapped_r, system_control_w)
	AM_RANGE(0x3c0000, 0x3c0007) AM_MIRROR(0x01fff8) AM_READ(neogeo_video_register_r)
	AM_RANGE(0x3c0000, 0x3c000f) AM_MIRROR(0x01fff0) AM_WRITE(neogeo_video_register_w)
	AM_RANGE(0x3e0000, 0x3fffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x400000, 0x401fff) AM_MIRROR(0x3fe000) AM_READWRITE(neogeo_paletteram_r, neogeo_paletteram_w)
	AM_RANGE(0x800000, 0x800fff) AM_READWRITE(memcard_r, memcard_w)
	AM_RANGE(0xc00000, 0xc1ffff) AM_MIRROR(0x0e0000) AM_ROMBANK(NEOGEO_BANK_BIOS)
	AM_RANGE(0xd00000, 0xd0ffff) AM_MIRROR(0x0f0000) AM_READWRITE(MRA16_RAM, save_ram_w) AM_BASE(&save_ram)
	AM_RANGE(0xe00000, 0xffffff) AM_READ(neogeo_unmapped_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_MAIN_BANK)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_CART_BANK + 3)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_CART_BANK + 2)
	AM_RANGE(0xe000, 0xefff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_CART_BANK + 1)
	AM_RANGE(0xf000, 0xf7ff) AM_ROMBANK(NEOGEO_BANK_AUDIO_CPU_CART_BANK + 0)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Audio CPU port handlers
 *
 *************************************/

static ADDRESS_MAP_START( auido_io_map, ADDRESS_SPACE_IO, 8 )
  /*AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READWRITE(audio_command_r, audio_cpu_clear_nmi_w);*/  /* may not and NMI clear */
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READ(audio_command_r)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0xff00) AM_READWRITE(YM2610_status_port_0_A_r, YM2610_control_port_0_A_w)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0xff00) AM_READWRITE(YM2610_read_port_0_r, YM2610_data_port_0_A_w)
	AM_RANGE(0x06, 0x06) AM_MIRROR(0xff00) AM_READWRITE(YM2610_status_port_0_B_r, YM2610_control_port_0_B_w)
	AM_RANGE(0x07, 0x07) AM_MIRROR(0xff00) AM_WRITE(YM2610_data_port_0_B_w)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff00) /* write - NMI enable / acknowledge? (the data written doesn't matter) */
	AM_SPACE(0x08, 0x0f) AM_READ(audio_cpu_bank_select_f000_f7ff_r)
	AM_SPACE(0x09, 0x0f) AM_READ(audio_cpu_bank_select_e000_efff_r)
	AM_SPACE(0x0a, 0x0f) AM_READ(audio_cpu_bank_select_c000_dfff_r)
	AM_SPACE(0x0b, 0x0f) AM_READ(audio_cpu_bank_select_8000_bfff_r)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xff00) AM_WRITE(audio_result_w)
	AM_RANGE(0x18, 0x18) AM_MIRROR(0xff00) /* write - NMI disable? (the data written doesn't matter) */
ADDRESS_MAP_END



/*************************************
 *
 *  Audio interface
 *
 *************************************/

static struct YM2610interface ym2610_interface =
{
	audio_cpu_irq,
	-1,	 /* this will be set by DRIVER_INIT */
	NEOGEO_REGION_AUDIO_DATA_1
};



/*************************************
 *
 *  Standard Neo-Geo DIPs and
 *  input port definition
 *
 *************************************/

#define STANDARD_DIPS 																	\
	PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" ) PORT_DIPLOCATION("SW:1")				\
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )										\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )										\
	PORT_DIPNAME( 0x0002, 0x0002, "Coin Chutes?" ) PORT_DIPLOCATION("SW:2")				\
	PORT_DIPSETTING(	  0x0000, "1?" )												\
	PORT_DIPSETTING(	  0x0002, "2?" )												\
	PORT_DIPNAME( 0x0004, 0x0004, "Autofire (in some games)" ) PORT_DIPLOCATION("SW:3")	\
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )										\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )										\
	PORT_DIPNAME( 0x0038, 0x0038, "COMM Setting" ) PORT_DIPLOCATION("SW:4,5,6")			\
	PORT_DIPSETTING(	  0x0038, DEF_STR( Off ) )										\
	PORT_DIPSETTING(	  0x0030, "1" )													\
	PORT_DIPSETTING(	  0x0020, "2" )													\
	PORT_DIPSETTING(	  0x0010, "3" )													\
	PORT_DIPSETTING(	  0x0000, "4" )													\
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW:7")		\
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )										\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )										\
	PORT_DIPNAME( 0x0080, 0x0080, "Freeze" ) PORT_DIPLOCATION("SW:8")					\
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )										\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )


#define STANDARD_IN0														\
	PORT_START_TAG("IN0")													\
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
	PORT_START_TAG("IN1")													\
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
	PORT_START_TAG("IN2")																			\
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )													\
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )   												\
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Next Game") PORT_CODE(KEYCODE_7)		\
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2 )   												\
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Previous Game") PORT_CODE(KEYCODE_8)	\
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_memcard_status, 0)				\
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )


#define STANDARD_IN3																				\
	PORT_START_TAG("IN3")																			\
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )													\
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )													\
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )													\
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* having this ACTIVE_HIGH causes you to start with 2 credits using USA bios roms */	\
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* having this ACTIVE_HIGH causes you to start with 2 credits using USA bios roms */	\
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL ) /* what is this? */ 								\
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_calendar_status, 0)				\
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_audio_result, 0)


#define STANDARD_IN4																			\
	PORT_START_TAG("IN4")																		\
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL ) /* what is this? */							\
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter BIOS") PORT_CODE(KEYCODE_F2)	\
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )


static INPUT_PORTS_START( neogeo )
	STANDARD_IN0

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( neogeo )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M68000, NEOGEO_MAIN_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(main_map,0)

	MDRV_CPU_ADD(Z80, NEOGEO_AUDIO_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(audio_map,0)
	MDRV_CPU_IO_MAP(auido_io_map,0)

	MDRV_WATCHDOG_TIME_INIT(UINT64_ATTOTIME_IN_USEC(128762))

	MDRV_MACHINE_START(neogeo)
	MDRV_MACHINE_RESET(neogeo)
	MDRV_NVRAM_HANDLER(neogeo)
	MDRV_MEMCARD_HANDLER(neogeo)

	/* video hardware */
	MDRV_VIDEO_START(neogeo)
	MDRV_VIDEO_RESET(neogeo)
	MDRV_VIDEO_UPDATE(neogeo)
	MDRV_DEFAULT_LAYOUT(layout_neogeo)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(NEOGEO_PIXEL_CLOCK, NEOGEO_HTOTAL, NEOGEO_HBEND, NEOGEO_HBSTART, NEOGEO_VTOTAL, NEOGEO_VBEND, NEOGEO_VBSTART)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2610, NEOGEO_YM2610_CLOCK)
	MDRV_SOUND_CONFIG(ym2610_interface)
	MDRV_SOUND_ROUTE(0, "left",  0.60)
	MDRV_SOUND_ROUTE(0, "right", 0.60)
	MDRV_SOUND_ROUTE(1, "left",  1.0)
	MDRV_SOUND_ROUTE(2, "right", 1.0)
MACHINE_DRIVER_END

/*************************************
 *
 *  Driver initalization
 *
 *************************************/

static DRIVER_INIT( neogeo )
{
	/* set the Delta-T sample region */
	ym2610_interface.pcmromb = memory_region(NEOGEO_REGION_AUDIO_DATA_2) ? NEOGEO_REGION_AUDIO_DATA_2 : NEOGEO_REGION_AUDIO_DATA_1;
}


#include "neodrvr.c"
