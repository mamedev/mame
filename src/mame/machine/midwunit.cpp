// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Ernesto Corvi
/*************************************************************************

    Driver for Williams/Midway Wolf-unit games.

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6809/m6809.h"
#include "audio/dcs.h"
#include "includes/midtunit.h"
#include "includes/midwunit.h"

/*************************************
 *
 *  State saving
 *
 *************************************/

void midwunit_state::register_state_saving()
{
	save_item(NAME(m_cmos_write_enable));
	save_item(NAME(m_iodata));
	save_item(NAME(m_ioshuffle));
	save_item(NAME(m_uart));
	save_item(NAME(m_security_bits));
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
		logerror("%08X:Unexpected CMOS W @ %05X\n", space.device().safe_pc(), offset);
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
			logerror("%08X:Control W @ %05X = %04X\n", space.device().safe_pc(), offset, data);

			/* bit 4 reset sound CPU */
			m_dcs->reset_w(newword & 0x10);

			/* bit 5 (active low) reset security chip */
			m_midway_serial_pic->reset_w(newword & 0x20);
			break;

		case 3:
			/* watchdog reset */
			/* MK3 resets with this enabled */
/*          watchdog_reset_w(0,0);*/
			break;

		default:
			logerror("%08X:Unknown I/O write to %d = %04X\n", space.device().safe_pc(), offset, data);
			break;
	}
	m_iodata[offset] = newword;
}



/*************************************
 *
 *  General I/O reads
 *
 *************************************/

IOPORT_ARRAY_MEMBER(midwunit_state::wunit_ports) { "IN0", "IN1", "DSW", "IN2" };

READ16_MEMBER(midwunit_state::midwunit_io_r)
{
	/* apply I/O shuffling */
	offset = m_ioshuffle[offset % 16];

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return m_ports[offset]->read();

		case 4:
			return (m_midway_serial_pic->status_r(space,0) << 12) | midwunit_sound_state_r(space,0,0xffff);

		default:
			logerror("%08X:Unknown I/O read from %d\n", space.device().safe_pc(), offset);
			break;
	}
	return ~0;
}



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

void midwunit_state::init_wunit_generic()
{
	/* register for state saving */
	register_state_saving();
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
	space.device().execute().adjust_icount(-100);
/*  printf("in=%04X%04X  out=%04X%04X\n", m_umk3_palette[3], m_umk3_palette[2], m_umk3_palette[1], m_umk3_palette[0]); */
}

void midwunit_state::init_mk3_common()
{
	/* common init */
	init_wunit_generic();

	/* serial prefixes 439, 528 */
	//midway_serial_pic_init(machine(), 528);
}

DRIVER_INIT_MEMBER(midwunit_state,mk3)
{
	init_mk3_common();
}

DRIVER_INIT_MEMBER(midwunit_state,mk3r20)
{
	init_mk3_common();
}

DRIVER_INIT_MEMBER(midwunit_state,mk3r10)
{
	init_mk3_common();
}

DRIVER_INIT_MEMBER(midwunit_state,umk3)
{
	init_mk3_common();
	m_umk3_palette = m_maincpu->space(AS_PROGRAM).install_write_handler(0x0106a060, 0x0106a09f, write16_delegate(FUNC(midwunit_state::umk3_palette_hack_w),this));
}

DRIVER_INIT_MEMBER(midwunit_state,umk3r11)
{
	init_mk3_common();
	m_umk3_palette = m_maincpu->space(AS_PROGRAM).install_write_handler(0x0106a060, 0x0106a09f,write16_delegate(FUNC(midwunit_state::umk3_palette_hack_w),this));
}


/********************** 2 On 2 Open Ice Challenge **********************/

DRIVER_INIT_MEMBER(midwunit_state,openice)
{
	/* common init */
	init_wunit_generic();

	/* serial prefixes 438, 528 */
	//midway_serial_pic_init(machine(), 528);
}


/********************** NBA Hangtime & NBA Maximum Hangtime **********************/

DRIVER_INIT_MEMBER(midwunit_state,nbahangt)
{
	/* common init */
	init_wunit_generic();

	/* serial prefixes 459, 470, 528 */
	//midway_serial_pic_init(machine(), 528);
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

DRIVER_INIT_MEMBER(midwunit_state,wwfmania)
{
	/* common init */
	init_wunit_generic();

	/* enable I/O shuffling */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x01800000, 0x0180000f, write16_delegate(FUNC(midwunit_state::wwfmania_io_0_w),this));

	/* serial prefixes 430, 528 */
	//midway_serial_pic_init(machine(), 528);
}


/********************** Rampage World Tour **********************/

DRIVER_INIT_MEMBER(midwunit_state,rmpgwt)
{
	/* common init */
	init_wunit_generic();

	/* serial prefixes 465, 528 */
	//midway_serial_pic_init(machine(), 528);
}


/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET_MEMBER(midwunit_state,midwunit)
{
	int i;

	/* reset sound */
	m_dcs->reset_w(1);
	m_dcs->reset_w(0);

	/* reset I/O shuffling */
	for (i = 0; i < 16; i++)
		m_ioshuffle[i] = i % 8;
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

READ16_MEMBER(midwunit_state::midwunit_security_r)
{
	return m_midway_serial_pic->read(space,0);
}


WRITE16_MEMBER(midwunit_state::midwunit_security_w)
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		m_midway_serial_pic->write(space, 0, data);
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

READ16_MEMBER(midwunit_state::midwunit_sound_r)
{
	logerror("%08X:Sound read\n", space.device().safe_pc());

	return m_dcs->data_r() & 0xff;
}


READ16_MEMBER(midwunit_state::midwunit_sound_state_r)
{
	return m_dcs->control_r();
}


WRITE16_MEMBER(midwunit_state::midwunit_sound_w)
{
	/* check for out-of-bounds accesses */
	if (offset)
	{
		logerror("%08X:Unexpected write to sound (hi) = %04X\n", space.device().safe_pc(), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_BITS_0_7)
	{
		logerror("%08X:Sound write = %04X\n", space.device().safe_pc(), data);
		m_dcs->data_w(data & 0xff);
	}
}
