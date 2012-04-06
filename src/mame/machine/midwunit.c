/*************************************************************************

    Driver for Williams/Midway Wolf-unit games.

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6809/m6809.h"
#include "audio/dcs.h"
#include "includes/midtunit.h"
#include "includes/midwunit.h"
#include "midwayic.h"

/*************************************
 *
 *  State saving
 *
 *************************************/

static void register_state_saving(running_machine &machine)
{
	midwunit_state *state = machine.driver_data<midwunit_state>();
	state_save_register_global(machine, state->m_cmos_write_enable);
	state_save_register_global_array(machine, state->m_iodata);
	state_save_register_global_array(machine, state->m_ioshuffle);
	state_save_register_global_array(machine, state->m_uart);
	state_save_register_global(machine, state->m_security_bits);
}



/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

WRITE16_MEMBER(midwunit_state::midwunit_cmos_enable_w)
{
	m_cmos_write_enable = 1;
}


WRITE16_MEMBER(midwunit_state::midwunit_cmos_w)
{
	if (m_cmos_write_enable)
	{
		COMBINE_DATA(m_nvram+offset);
		m_cmos_write_enable = 0;
	}
	else
	{
		logerror("%08X:Unexpected CMOS W @ %05X\n", cpu_get_pc(&space.device()), offset);
		popmessage("Bad CMOS write");
	}
}



READ16_MEMBER(midwunit_state::midwunit_cmos_r)
{
	return m_nvram[offset];
}



/*************************************
 *
 *  General I/O writes
 *
 *************************************/

WRITE16_MEMBER(midwunit_state::midwunit_io_w)
{
	int oldword, newword;

	offset %= 8;
	oldword = m_iodata[offset];
	newword = oldword;
	COMBINE_DATA(&newword);

	switch (offset)
	{
		case 1:
			logerror("%08X:Control W @ %05X = %04X\n", cpu_get_pc(&space.device()), offset, data);

			/* bit 4 reset sound CPU */
			dcs_reset_w(machine(), newword & 0x10);

			/* bit 5 (active low) reset security chip */
			midway_serial_pic_reset_w(newword & 0x20);
			break;

		case 3:
			/* watchdog reset */
			/* MK3 resets with this enabled */
/*          watchdog_reset_w(0,0);*/
			break;

		default:
			logerror("%08X:Unknown I/O write to %d = %04X\n", cpu_get_pc(&space.device()), offset, data);
			break;
	}
	m_iodata[offset] = newword;
}



/*************************************
 *
 *  General I/O reads
 *
 *************************************/

READ16_MEMBER(midwunit_state::midwunit_io_r)
{
	static const char *const portnames[] = { "IN0", "IN1", "DSW", "IN2" };

	/* apply I/O shuffling */
	offset = m_ioshuffle[offset % 16];

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return input_port_read(machine(), portnames[offset]);

		case 4:
			return (midway_serial_pic_status_r() << 12) | midwunit_sound_state_r(space,0,0xffff);

		default:
			logerror("%08X:Unknown I/O read from %d\n", cpu_get_pc(&space.device()), offset);
			break;
	}
	return ~0;
}



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

static void init_wunit_generic(running_machine &machine)
{
	midwunit_state *state = machine.driver_data<midwunit_state>();
	UINT8 *base;
	int i, j, len;

	/* register for state saving */
	register_state_saving(machine);

	/* load the graphics ROMs -- quadruples */
	midtunit_gfx_rom = base = machine.region("gfx1")->base();
	len = machine.region("gfx1")->bytes();
	for (i = 0; i < len / 0x400000; i++)
	{
		memcpy(state->m_decode_memory, base, 0x400000);
		for (j = 0; j < 0x100000; j++)
		{
			*base++ = state->m_decode_memory[0x000000 + j];
			*base++ = state->m_decode_memory[0x100000 + j];
			*base++ = state->m_decode_memory[0x200000 + j];
			*base++ = state->m_decode_memory[0x300000 + j];
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


WRITE16_MEMBER(midwunit_state::umk3_palette_hack_w)
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
	COMBINE_DATA(&m_umk3_palette[offset]);
	device_adjust_icount(&space.device(), -100);
/*  printf("in=%04X%04X  out=%04X%04X\n", m_umk3_palette[3], m_umk3_palette[2], m_umk3_palette[1], m_umk3_palette[0]); */
}

static void init_mk3_common(running_machine &machine)
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
	midwunit_state *state = machine.driver_data<midwunit_state>();
	init_mk3_common(machine);
	state->m_umk3_palette = machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x0106a060, 0x0106a09f, write16_delegate(FUNC(midwunit_state::umk3_palette_hack_w),state));
}

DRIVER_INIT( umk3r11 )
{
	midwunit_state *state = machine.driver_data<midwunit_state>();
	init_mk3_common(machine);
	state->m_umk3_palette = machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x0106a060, 0x0106a09f,write16_delegate(FUNC(midwunit_state::umk3_palette_hack_w),state));
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

WRITE16_MEMBER(midwunit_state::wwfmania_io_0_w)
{
	int i;

	/* start with the originals */
	for (i = 0; i < 16; i++)
		m_ioshuffle[i] = i % 8;

	/* based on the data written, shuffle */
	switch (data)
	{
		case 0:
			break;

		case 1:
			m_ioshuffle[4] = 0;
			m_ioshuffle[8] = 1;
			m_ioshuffle[1] = 2;
			m_ioshuffle[9] = 3;
			m_ioshuffle[2] = 4;
			break;

		case 2:
			m_ioshuffle[8] = 0;
			m_ioshuffle[2] = 1;
			m_ioshuffle[4] = 2;
			m_ioshuffle[6] = 3;
			m_ioshuffle[1] = 4;
			break;

		case 3:
			m_ioshuffle[1] = 0;
			m_ioshuffle[8] = 1;
			m_ioshuffle[2] = 2;
			m_ioshuffle[10] = 3;
			m_ioshuffle[5] = 4;
			break;

		case 4:
			m_ioshuffle[2] = 0;
			m_ioshuffle[4] = 1;
			m_ioshuffle[1] = 2;
			m_ioshuffle[7] = 3;
			m_ioshuffle[8] = 4;
			break;
	}
	logerror("Changed I/O swiching to %d\n", data);
}

DRIVER_INIT( wwfmania )
{
	/* common init */
	init_wunit_generic(machine);

	/* enable I/O shuffling */
	midwunit_state *state = machine.driver_data<midwunit_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x01800000, 0x0180000f, write16_delegate(FUNC(midwunit_state::wwfmania_io_0_w),state));

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


/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET( midwunit )
{
	midwunit_state *state = machine.driver_data<midwunit_state>();
	int i;

	/* reset sound */
	dcs_reset_w(machine, 1);
	dcs_reset_w(machine, 0);

	/* reset I/O shuffling */
	for (i = 0; i < 16; i++)
		state->m_ioshuffle[i] = i % 8;
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

READ16_MEMBER(midwunit_state::midwunit_security_r)
{
	return midway_serial_pic_r(&space);
}


WRITE16_MEMBER(midwunit_state::midwunit_security_w)
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		midway_serial_pic_w(&space, data);
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

READ16_MEMBER(midwunit_state::midwunit_sound_r)
{
	logerror("%08X:Sound read\n", cpu_get_pc(&space.device()));

	return dcs_data_r(machine()) & 0xff;
}


READ16_MEMBER(midwunit_state::midwunit_sound_state_r)
{
	return dcs_control_r(machine());
}


WRITE16_MEMBER(midwunit_state::midwunit_sound_w)
{
	/* check for out-of-bounds accesses */
	if (offset)
	{
		logerror("%08X:Unexpected write to sound (hi) = %04X\n", cpu_get_pc(&space.device()), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_BITS_0_7)
	{
		logerror("%08X:Sound write = %04X\n", cpu_get_pc(&space.device()), data);
		dcs_data_w(machine(), data & 0xff);
	}
}
