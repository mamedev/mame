/*************************************************************************

    Driver for Williams/Midway Wolf-unit games.

**************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6809/m6809.h"
#include "audio/dcs.h"
#include "midwunit.h"
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
static void midxunit_dcs_output_full(int state);



/*************************************
 *
 *  State saving
 *
 *************************************/

static void register_state_saving(void)
{
	state_save_register_global(cmos_write_enable);
	state_save_register_global_array(iodata);
	state_save_register_global_array(ioshuffle);
	state_save_register_global(midxunit_analog_port);
	state_save_register_global_array(uart);
	state_save_register_global(security_bits);
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
		COMBINE_DATA(generic_nvram16+offset);
		cmos_write_enable = 0;
	}
	else
	{
		logerror("%08X:Unexpected CMOS W @ %05X\n", activecpu_get_pc(), offset);
		popmessage("Bad CMOS write");
	}
}


WRITE16_HANDLER( midxunit_cmos_w )
{
	COMBINE_DATA(generic_nvram16+offset);
}


READ16_HANDLER( midwunit_cmos_r )
{
	return generic_nvram16[offset];
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
			logerror("%08X:Control W @ %05X = %04X\n", activecpu_get_pc(), offset, data);

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
			logerror("%08X:Unknown I/O write to %d = %04X\n", activecpu_get_pc(), offset, data);
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
			logerror("%08X:I/O write to %d = %04X\n", activecpu_get_pc(), offset, data);
//          logerror("%08X:Unknown I/O write to %d = %04X\n", activecpu_get_pc(), offset, data);
			break;
	}
	iodata[offset] = newword;
}


WRITE16_HANDLER( midxunit_unknown_w )
{
	int offs = offset / 0x40000;
	if (ACCESSING_LSB && offset % 0x40000 == 0)
		logerror("%08X:midxunit_unknown_w @ %d = %02X\n", activecpu_get_pc(), offs, data & 0xff);
}


/*************************************
 *
 *  General I/O reads
 *
 *************************************/

READ16_HANDLER( midwunit_io_r )
{
	/* apply I/O shuffling */
	offset = ioshuffle[offset % 16];

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return readinputport(offset);

		case 4:
			return (midway_serial_pic_status_r() << 12) | midwunit_sound_state_r(0,0);

		default:
			logerror("%08X:Unknown I/O read from %d\n", activecpu_get_pc(), offset);
			break;
	}
	return ~0;
}


READ16_HANDLER( midxunit_io_r )
{
	offset = (offset / 2) % 8;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return readinputport(offset);

		default:
			logerror("%08X:Unknown I/O read from %d\n", activecpu_get_pc(), offset);
			break;
	}
	return ~0;
}


READ16_HANDLER( midxunit_analog_r )
{
	return readinputport(midxunit_analog_port);
}


WRITE16_HANDLER( midxunit_analog_select_w )
{
	if (offset == 0 && ACCESSING_LSB)
		midxunit_analog_port = data - 8 + 4;
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

void midxunit_dcs_output_full(int state)
{
	/* only signal if not in loopback state */
	if (uart[1] != 0x66)
		cpunum_set_input_line(1, 1, state ? ASSERT_LINE : CLEAR_LINE);
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
				int temp = midwunit_sound_state_r(0, 0);
				result |= (temp & 0x800) >> 9;
				result |= (~temp & 0x400) >> 10;
				timer_call_after_resynch(0, 0);
			}
			break;

		case 3:	/* register 3 contains the data read */

			/* loopback case: feed back last data wrtten */
			if (uart[1] == 0x66)
				result = uart[3];

			/* non-loopback case: read from the DCS system */
			else
				result = midwunit_sound_r(0, 0);
			break;

		case 5:	/* register 5 seems to be like 3, but with in/out swapped */

			/* loopback case: data always ready, and always ok to send */
			if (uart[1] == 0x66)
				result |= 5;

			/* non-loopback case: bit 0 means data ready, bit 2 means ok to send */
			else
			{
				int temp = midwunit_sound_state_r(0, 0);
				result |= (temp & 0x800) >> 11;
				result |= (~temp & 0x400) >> 8;
				timer_call_after_resynch(0, 0);
			}
			break;

		default: /* everyone else reads themselves */
			result = uart[offset];
			break;
	}

/*  logerror("%08X:UART R @ %X = %02X\n", activecpu_get_pc(), offset, result);*/
	return result;
}


WRITE16_HANDLER( midxunit_uart_w )
{
	/* convert to a byte offset, ignoring MSB writes */
	if ((offset & 1) || !ACCESSING_LSB)
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
				midwunit_sound_w(0, data, mem_mask);
			break;

		case 5:	/* register 5 write seems to reset things */
			dcs_data_r();
			break;

		default: /* everyone else just stores themselves */
			uart[offset] = data;
			break;
	}

/*  logerror("%08X:UART W @ %X = %02X\n", activecpu_get_pc(), offset, data);*/
}



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

static void init_wunit_generic(void)
{
	UINT8 *base;
	int i, j;

	/* register for state saving */
	register_state_saving();

	/* load the graphics ROMs -- quadruples */
	midyunit_gfx_rom = base = memory_region(REGION_GFX1);
	for (i = 0; i < memory_region_length(REGION_GFX1) / 0x400000; i++)
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
	dcs_init();
}




/*************************************
 *
 *  Wolf-unit init (DCS)
 *
 *  music: ADSP2101
 *
 *************************************/

/********************** Mortal Kombat 3 **********************/

static void init_mk3_common(void)
{
	/* common init */
	init_wunit_generic();

	/* serial prefixes 439, 528 */
	midway_serial_pic_init(528);
}

DRIVER_INIT( mk3 )
{
	init_mk3_common();
}

DRIVER_INIT( mk3r20 )
{
	init_mk3_common();
}

DRIVER_INIT( mk3r10 )
{
	init_mk3_common();
}

DRIVER_INIT( umk3 )
{
	init_mk3_common();
}

DRIVER_INIT( umk3r11 )
{
	init_mk3_common();
}


/********************** 2 On 2 Open Ice Challenge **********************/

DRIVER_INIT( openice )
{
	/* common init */
	init_wunit_generic();

	/* serial prefixes 438, 528 */
	midway_serial_pic_init(528);
}


/********************** NBA Hangtime & NBA Maximum Hangtime **********************/

DRIVER_INIT( nbahangt )
{
	/* common init */
	init_wunit_generic();

	/* serial prefixes 459, 470, 528 */
	midway_serial_pic_init(528);
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
	init_wunit_generic();

	/* enable I/O shuffling */
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x01800000, 0x0180000f, 0, 0, wwfmania_io_0_w);

	/* serial prefixes 430, 528 */
	midway_serial_pic_init(528);
}


/********************** Rampage World Tour **********************/

DRIVER_INIT( rmpgwt )
{
	/* common init */
	init_wunit_generic();

	/* serial prefixes 465, 528 */
	midway_serial_pic_init(528);
}


/********************** Revolution X **********************/

DRIVER_INIT( revx )
{
	UINT8 *base;
	int i, j;

	/* register for state saving */
	register_state_saving();

	/* load the graphics ROMs -- quadruples */
	midyunit_gfx_rom = base = memory_region(REGION_GFX1);
	for (i = 0; i < memory_region_length(REGION_GFX1) / 0x200000; i++)
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
	dcs_init();

	/* serial prefixes 419, 420 */
	midway_serial_pic_init(419);
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
	machine_reset_midwunit(machine);
	dcs_set_io_callbacks(midxunit_dcs_output_full, NULL);
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

READ16_HANDLER( midwunit_security_r )
{
	return midway_serial_pic_r();
}


WRITE16_HANDLER( midwunit_security_w )
{
	if (offset == 0 && ACCESSING_LSB)
		midway_serial_pic_w(data);
}


WRITE16_HANDLER( midxunit_security_w )
{
	if (ACCESSING_LSB)
		security_bits = data & 0x0f;
}


WRITE16_HANDLER( midxunit_security_clock_w )
{
	if (offset == 0 && ACCESSING_LSB)
		midway_serial_pic_w(((~data & 2) << 3) | security_bits);
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

READ16_HANDLER( midwunit_sound_r )
{
	logerror("%08X:Sound read\n", activecpu_get_pc());

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
		logerror("%08X:Unexpected write to sound (hi) = %04X\n", activecpu_get_pc(), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_LSB)
	{
		logerror("%08X:Sound write = %04X\n", activecpu_get_pc(), data);
		dcs_data_w(data & 0xff);
	}
}
