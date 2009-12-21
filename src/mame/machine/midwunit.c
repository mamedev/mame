/*************************************************************************

    Driver for Williams/Midway Wolf-unit games.

**************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6809/m6809.h"
#include "audio/dcs.h"
#include "includes/midwunit.h"
#include "midwayic.h"


/* code-related variables */
       UINT8 *	midwunit_decode_memory;

/* CMOS-related variables */
static UINT8	cmos_write_enable;

/* I/O-related variables */
static UINT16	iodata[8];
static UINT8	ioshuffle[16];
static UINT8	midxunit_analog_port;

/* UART-related variables */
static UINT8	uart[8];
static UINT8	security_bits;

/* prototype */
static READ16_HANDLER( midwunit_sound_state_r );
static void midxunit_dcs_output_full(running_machine *machine, int state);



/*************************************
 *
 *  State saving
 *
 *************************************/

static void register_state_saving(running_machine *machine)
{
	state_save_register_global(machine, cmos_write_enable);
	state_save_register_global_array(machine, iodata);
	state_save_register_global_array(machine, ioshuffle);
	state_save_register_global(machine, midxunit_analog_port);
	state_save_register_global_array(machine, uart);
	state_save_register_global(machine, security_bits);
}



/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

WRITE16_HANDLER( midwunit_cmos_enable_w )
{
	cmos_write_enable = 1;
}


WRITE16_HANDLER( midwunit_cmos_w )
{
	if (cmos_write_enable)
	{
		COMBINE_DATA(space->machine->generic.nvram.u16+offset);
		cmos_write_enable = 0;
	}
	else
	{
		logerror("%08X:Unexpected CMOS W @ %05X\n", cpu_get_pc(space->cpu), offset);
		popmessage("Bad CMOS write");
	}
}


WRITE16_HANDLER( midxunit_cmos_w )
{
	COMBINE_DATA(space->machine->generic.nvram.u16+offset);
}


READ16_HANDLER( midwunit_cmos_r )
{
	return space->machine->generic.nvram.u16[offset];
}



/*************************************
 *
 *  General I/O writes
 *
 *************************************/

WRITE16_HANDLER( midwunit_io_w )
{
	int oldword, newword;

	offset %= 8;
	oldword = iodata[offset];
	newword = oldword;
	COMBINE_DATA(&newword);

	switch (offset)
	{
		case 1:
			logerror("%08X:Control W @ %05X = %04X\n", cpu_get_pc(space->cpu), offset, data);

			/* bit 4 reset sound CPU */
			dcs_reset_w(newword & 0x10);

			/* bit 5 (active low) reset security chip */
			midway_serial_pic_reset_w(newword & 0x20);
			break;

		case 3:
			/* watchdog reset */
			/* MK3 resets with this enabled */
/*          watchdog_reset_w(0,0);*/
			break;

		default:
			logerror("%08X:Unknown I/O write to %d = %04X\n", cpu_get_pc(space->cpu), offset, data);
			break;
	}
	iodata[offset] = newword;
}


WRITE16_HANDLER( midxunit_io_w )
{
	int oldword, newword;

	offset = (offset / 2) % 8;
	oldword = iodata[offset];
	newword = oldword;
	COMBINE_DATA(&newword);

	switch (offset)
	{
		case 2:
			/* watchdog reset */
//          watchdog_reset_w(0,0);
			break;

		default:
			logerror("%08X:I/O write to %d = %04X\n", cpu_get_pc(space->cpu), offset, data);
//          logerror("%08X:Unknown I/O write to %d = %04X\n", cpu_get_pc(space->cpu), offset, data);
			break;
	}
	iodata[offset] = newword;
}


WRITE16_HANDLER( midxunit_unknown_w )
{
	int offs = offset / 0x40000;

	if (offs == 1 && ACCESSING_BITS_0_7)
		dcs_reset_w(data & 2);

	if (ACCESSING_BITS_0_7 && offset % 0x40000 == 0)
		logerror("%08X:midxunit_unknown_w @ %d = %02X\n", cpu_get_pc(space->cpu), offs, data & 0xff);
}



/*************************************
 *
 *  General I/O reads
 *
 *************************************/

READ16_HANDLER( midwunit_io_r )
{
	static const char *const portnames[] = { "IN0", "IN1", "DSW", "IN2" };

	/* apply I/O shuffling */
	offset = ioshuffle[offset % 16];

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return input_port_read(space->machine, portnames[offset]);

		case 4:
			return (midway_serial_pic_status_r() << 12) | midwunit_sound_state_r(space,0,0xffff);

		default:
			logerror("%08X:Unknown I/O read from %d\n", cpu_get_pc(space->cpu), offset);
			break;
	}
	return ~0;
}


READ16_HANDLER( midxunit_io_r )
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "DSW" };

	offset = (offset / 2) % 8;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return input_port_read(space->machine, portnames[offset]);

		default:
			logerror("%08X:Unknown I/O read from %d\n", cpu_get_pc(space->cpu), offset);
			break;
	}
	return ~0;
}


READ16_HANDLER( midxunit_analog_r )
{
	static const char *const portnames[] = { "AN0", "AN1", "AN2", "AN3", "AN4", "AN5" };

	return input_port_read(space->machine, portnames[midxunit_analog_port]);
}


WRITE16_HANDLER( midxunit_analog_select_w )
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		midxunit_analog_port = data - 8;
}


READ16_HANDLER( midxunit_status_r )
{
	/* low bit indicates whether the ADC is done reading the current input */
	return (midway_serial_pic_status_r() << 1) | 1;
}



/*************************************
 *
 *  Revolution X UART
 *
 *************************************/

static void midxunit_dcs_output_full(running_machine *machine, int state)
{
	/* only signal if not in loopback state */
	if (uart[1] != 0x66)
		cputag_set_input_line(machine, "maincpu", 1, state ? ASSERT_LINE : CLEAR_LINE);
}


READ16_HANDLER( midxunit_uart_r )
{
	int result = 0;

	/* convert to a byte offset */
	if (offset & 1)
		return 0;
	offset /= 2;

	/* switch off the offset */
	switch (offset)
	{
		case 0:	/* register 0 must return 0x13 in order to pass the self test */
			result = 0x13;
			break;

		case 1:	/* register 1 contains the status */

			/* loopback case: data always ready, and always ok to send */
			if (uart[1] == 0x66)
				result |= 5;

			/* non-loopback case: bit 0 means data ready, bit 2 means ok to send */
			else
			{
				int temp = midwunit_sound_state_r(space, 0, 0xffff);
				result |= (temp & 0x800) >> 9;
				result |= (~temp & 0x400) >> 10;
				timer_call_after_resynch(space->machine, NULL, 0, 0);
			}
			break;

		case 3:	/* register 3 contains the data read */

			/* loopback case: feed back last data wrtten */
			if (uart[1] == 0x66)
				result = uart[3];

			/* non-loopback case: read from the DCS system */
			else
				result = midwunit_sound_r(space, 0, 0xffff);
			break;

		case 5:	/* register 5 seems to be like 3, but with in/out swapped */

			/* loopback case: data always ready, and always ok to send */
			if (uart[1] == 0x66)
				result |= 5;

			/* non-loopback case: bit 0 means data ready, bit 2 means ok to send */
			else
			{
				int temp = midwunit_sound_state_r(space, 0, 0xffff);
				result |= (temp & 0x800) >> 11;
				result |= (~temp & 0x400) >> 8;
				timer_call_after_resynch(space->machine, NULL, 0, 0);
			}
			break;

		default: /* everyone else reads themselves */
			result = uart[offset];
			break;
	}

/*  logerror("%08X:UART R @ %X = %02X\n", cpu_get_pc(space->cpu), offset, result);*/
	return result;
}


WRITE16_HANDLER( midxunit_uart_w )
{
	/* convert to a byte offset, ignoring MSB writes */
	if ((offset & 1) || !ACCESSING_BITS_0_7)
		return;
	offset /= 2;
	data &= 0xff;

	/* switch off the offset */
	switch (offset)
	{
		case 3:	/* register 3 contains the data to be sent */

			/* loopback case: don't feed through */
			if (uart[1] == 0x66)
				uart[3] = data;

			/* non-loopback case: send to the DCS system */
			else
				midwunit_sound_w(space, 0, data, mem_mask);
			break;

		case 5:	/* register 5 write seems to reset things */
			dcs_data_r();
			break;

		default: /* everyone else just stores themselves */
			uart[offset] = data;
			break;
	}

/*  logerror("%08X:UART W @ %X = %02X\n", cpu_get_pc(space->cpu), offset, data);*/
}



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

static void init_wunit_generic(running_machine *machine)
{
	UINT8 *base;
	int i, j, len;

	/* register for state saving */
	register_state_saving(machine);

	/* load the graphics ROMs -- quadruples */
	midyunit_gfx_rom = base = memory_region(machine, "gfx1");
	len = memory_region_length(machine, "gfx1");
	for (i = 0; i < len / 0x400000; i++)
	{
		memcpy(midwunit_decode_memory, base, 0x400000);
		for (j = 0; j < 0x100000; j++)
		{
			*base++ = midwunit_decode_memory[0x000000 + j];
			*base++ = midwunit_decode_memory[0x100000 + j];
			*base++ = midwunit_decode_memory[0x200000 + j];
			*base++ = midwunit_decode_memory[0x300000 + j];
		}
	}

	/* init sound */
	dcs_init(machine);
}




/*************************************
 *
 *  Wolf-unit init (DCS)
 *
 *  music: ADSP2101
 *
 *************************************/

/********************** Mortal Kombat 3 **********************/

static UINT16 *umk3_palette;

static WRITE16_HANDLER( umk3_palette_hack_w )
{
	/*
        UMK3 uses a circular buffer to hold pending palette changes; the buffer holds 17 entries
        total, and the buffer is processed/cleared during the video interrupt. Most of the time,
        17 entries is enough. However, when characters are unlocked, or a number of characters are
        being displayed, the circular buffer sometimes wraps, losing the first 17 palette changes.

        This bug manifests itself on a real PCB, but only rarely; whereas in MAME, it manifests
        itself very frequently. This is due to the fact that the instruction timing for the TMS34010
        is optimistic and assumes that the instruction cache is always fully populated. Without
        full cache level emulation of the chip, there is no hope of fixing this issue without a
        hack.

        Thus, the hack. To slow down the CPU when it is adding palette entries to the list, we
        install this write handler on the memory locations where the start/end circular buffer
        pointers live. Each time they are written to, we penalize the main CPU a number of cycles.
        Although not realistic, this is sufficient to reduce the frequency of incorrect colors
        without significantly impacting the rest of the system.
    */
	COMBINE_DATA(&umk3_palette[offset]);
	cpu_adjust_icount(space->cpu, -100);
/*  printf("in=%04X%04X  out=%04X%04X\n", umk3_palette[3], umk3_palette[2], umk3_palette[1], umk3_palette[0]); */
}

static void init_mk3_common(running_machine *machine)
{
	/* common init */
	init_wunit_generic(machine);

	/* serial prefixes 439, 528 */
	midway_serial_pic_init(machine, 528);
}

DRIVER_INIT( mk3 )
{
	init_mk3_common(machine);
}

DRIVER_INIT( mk3r20 )
{
	init_mk3_common(machine);
}

DRIVER_INIT( mk3r10 )
{
	init_mk3_common(machine);
}

DRIVER_INIT( umk3 )
{
	init_mk3_common(machine);
	umk3_palette = memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0106a060, 0x0106a09f, 0, 0, umk3_palette_hack_w);
}

DRIVER_INIT( umk3r11 )
{
	init_mk3_common(machine);
	umk3_palette = memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0106a060, 0x0106a09f, 0, 0, umk3_palette_hack_w);
}


/********************** 2 On 2 Open Ice Challenge **********************/

DRIVER_INIT( openice )
{
	/* common init */
	init_wunit_generic(machine);

	/* serial prefixes 438, 528 */
	midway_serial_pic_init(machine, 528);
}


/********************** NBA Hangtime & NBA Maximum Hangtime **********************/

DRIVER_INIT( nbahangt )
{
	/* common init */
	init_wunit_generic(machine);

	/* serial prefixes 459, 470, 528 */
	midway_serial_pic_init(machine, 528);
}


/********************** WWF Wrestlemania **********************/

static WRITE16_HANDLER( wwfmania_io_0_w )
{
	int i;

	/* start with the originals */
	for (i = 0; i < 16; i++)
		ioshuffle[i] = i % 8;

	/* based on the data written, shuffle */
	switch (data)
	{
		case 0:
			break;

		case 1:
			ioshuffle[4] = 0;
			ioshuffle[8] = 1;
			ioshuffle[1] = 2;
			ioshuffle[9] = 3;
			ioshuffle[2] = 4;
			break;

		case 2:
			ioshuffle[8] = 0;
			ioshuffle[2] = 1;
			ioshuffle[4] = 2;
			ioshuffle[6] = 3;
			ioshuffle[1] = 4;
			break;

		case 3:
			ioshuffle[1] = 0;
			ioshuffle[8] = 1;
			ioshuffle[2] = 2;
			ioshuffle[10] = 3;
			ioshuffle[5] = 4;
			break;

		case 4:
			ioshuffle[2] = 0;
			ioshuffle[4] = 1;
			ioshuffle[1] = 2;
			ioshuffle[7] = 3;
			ioshuffle[8] = 4;
			break;
	}
	logerror("Changed I/O swiching to %d\n", data);
}

DRIVER_INIT( wwfmania )
{
	/* common init */
	init_wunit_generic(machine);

	/* enable I/O shuffling */
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x01800000, 0x0180000f, 0, 0, wwfmania_io_0_w);

	/* serial prefixes 430, 528 */
	midway_serial_pic_init(machine, 528);
}


/********************** Rampage World Tour **********************/

DRIVER_INIT( rmpgwt )
{
	/* common init */
	init_wunit_generic(machine);

	/* serial prefixes 465, 528 */
	midway_serial_pic_init(machine, 528);
}


/********************** Revolution X **********************/

DRIVER_INIT( revx )
{
	UINT8 *base;
	int i, j, len;

	/* register for state saving */
	register_state_saving(machine);

	/* load the graphics ROMs -- quadruples */
	midyunit_gfx_rom = base = memory_region(machine, "gfx1");
	len = memory_region_length(machine, "gfx1");
	for (i = 0; i < len / 0x200000; i++)
	{
		memcpy(midwunit_decode_memory, base, 0x200000);
		for (j = 0; j < 0x80000; j++)
		{
			*base++ = midwunit_decode_memory[0x000000 + j];
			*base++ = midwunit_decode_memory[0x080000 + j];
			*base++ = midwunit_decode_memory[0x100000 + j];
			*base++ = midwunit_decode_memory[0x180000 + j];
		}
	}

	/* init sound */
	dcs_init(machine);

	/* serial prefixes 419, 420 */
	midway_serial_pic_init(machine, 419);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET( midwunit )
{
	int i;

	/* reset sound */
	dcs_reset_w(1);
	dcs_reset_w(0);

	/* reset I/O shuffling */
	for (i = 0; i < 16; i++)
		ioshuffle[i] = i % 8;
}


MACHINE_RESET( midxunit )
{
	MACHINE_RESET_CALL(midwunit);
	dcs_set_io_callbacks(midxunit_dcs_output_full, NULL);
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

READ16_HANDLER( midwunit_security_r )
{
	return midway_serial_pic_r(space);
}


WRITE16_HANDLER( midwunit_security_w )
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		midway_serial_pic_w(space, data);
}


WRITE16_HANDLER( midxunit_security_w )
{
	if (ACCESSING_BITS_0_7)
		security_bits = data & 0x0f;
}


WRITE16_HANDLER( midxunit_security_clock_w )
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		midway_serial_pic_w(space, ((~data & 2) << 3) | security_bits);
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

READ16_HANDLER( midwunit_sound_r )
{
	logerror("%08X:Sound read\n", cpu_get_pc(space->cpu));

	return dcs_data_r() & 0xff;
}


READ16_HANDLER( midwunit_sound_state_r )
{
	return dcs_control_r();
}


WRITE16_HANDLER( midwunit_sound_w )
{
	/* check for out-of-bounds accesses */
	if (offset)
	{
		logerror("%08X:Unexpected write to sound (hi) = %04X\n", cpu_get_pc(space->cpu), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_BITS_0_7)
	{
		logerror("%08X:Sound write = %04X\n", cpu_get_pc(space->cpu), data);
		dcs_data_w(data & 0xff);
	}
}
