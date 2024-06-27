// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Ernesto Corvi
/*************************************************************************

    Driver for Williams/Midway Wolf-unit games.

**************************************************************************/

#include "emu.h"
#include "midwunit.h"

#define LOG_UNKNOWN (1U << 1)
#define LOG_CMOS    (1U << 2)
#define LOG_IO      (1U << 3)
#define LOG_SOUND   (1U << 4)

#define VERBOSE     (0)
#include "logmacro.h"

/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

void midwunit_state::cmos_enable_w(uint16_t data)
{
	m_cmos_write_enable = 1;
}


void midwunit_state::cmos_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_cmos_write_enable)
	{
		COMBINE_DATA(m_nvram+offset);
		m_cmos_write_enable = 0;
	}
	else
	{
		LOGMASKED(LOG_CMOS | LOG_UNKNOWN, "%s: Unexpected CMOS W @ %05X\n", machine().describe_context(), offset);
		popmessage("Bad CMOS write");
	}
}



uint16_t midwunit_state::cmos_r(offs_t offset)
{
	return m_nvram[offset];
}



/*************************************
 *
 *  General I/O writes
 *
 *************************************/

void midwunit_state::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// apply I/O shuffling
	offset = m_ioshuffle[offset % 16];

	offset %= 8;
	uint16_t const oldword = m_iodata[offset];
	uint16_t newword = oldword;
	COMBINE_DATA(&newword);

	switch (offset)
	{
		case 1:
			LOGMASKED(LOG_IO, "%s: Control W @ %05X = %04X\n", machine().describe_context(), offset, data);

			// bit 4 reset sound CPU
			m_dcs->reset_w(~newword & 0x10);

			// bit 5 (active low) reset security chip
			if (m_midway_serial_pic) m_midway_serial_pic->reset_w(newword & 0x20);
			if (m_midway_serial_pic_emu) m_midway_serial_pic_emu->reset_w(newword & 0x20);
			break;

		case 3:
			// watchdog reset
			// MK3 resets with this enabled
			// watchdog_reset_w(0,0);
			break;

		default:
			LOGMASKED(LOG_IO | LOG_UNKNOWN, "%s: Unknown I/O write to %d = %04X\n", machine().describe_context(), offset, data);
			break;
	}
	m_iodata[offset] = newword;
}



/*************************************
 *
 *  General I/O reads
 *
 *************************************/

uint16_t midwunit_state::io_r(offs_t offset)
{
	// apply I/O shuffling
	offset = m_ioshuffle[offset % 16];

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return m_ports[offset]->read();

		case 4:
		{
			int picret = 0;
			if (m_midway_serial_pic) picret = m_midway_serial_pic->status_r();
			if (m_midway_serial_pic_emu) picret = m_midway_serial_pic_emu->status_r();

			return (picret << 12) | sound_state_r();
		}
		default:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_IO | LOG_UNKNOWN, "%s: Unknown I/O read from %d\n", machine().describe_context(), offset);
			break;
	}
	return ~0;
}



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

void midwunit_state::machine_start()
{
	save_item(NAME(m_cmos_write_enable));
	save_item(NAME(m_iodata));
	save_item(NAME(m_ioshuffle));
	save_item(NAME(m_uart));
	save_item(NAME(m_security_bits));
}




/*************************************
 *
 *  Wolf-unit init (DCS)
 *
 *  music: ADSP2101
 *
 *************************************/

/********************** Mortal Kombat 3 **********************/


void midwunit_state::umk3_palette_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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
	m_maincpu->adjust_icount(-100);
//  printf("in=%04X%04X  out=%04X%04X\n", m_umk3_palette[3], m_umk3_palette[2], m_umk3_palette[1], m_umk3_palette[0]);
}

void midwunit_state::init_mk3_common()
{
	// serial prefixes 439, 528
	//midway_serial_pic_init(machine(), 528);
}

void midwunit_state::init_mk3()
{
	init_mk3_common();
}

void midwunit_state::init_mk3r20()
{
	init_mk3_common();
}

void midwunit_state::init_mk3r10()
{
	init_mk3_common();
}

void midwunit_state::init_umk3()
{
	init_mk3_common();
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0106a060, 0x0106a09f, write16s_delegate(*this, FUNC(midwunit_state::umk3_palette_hack_w)));
	m_umk3_palette = m_mainram + (0x6a060>>4);
}

void midwunit_state::init_umk3r11()
{
	init_mk3_common();
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0106a060, 0x0106a09f, write16s_delegate(*this, FUNC(midwunit_state::umk3_palette_hack_w)));
	m_umk3_palette = m_mainram + (0x6a060>>4);
}


/********************** 2 On 2 Open Ice Challenge **********************/

void midwunit_state::init_openice()
{
	// serial prefixes 438, 528
	//midway_serial_pic_init(machine(), 528);
}


/********************** NBA Hangtime & NBA Maximum Hangtime **********************/

void midwunit_state::init_nbahangt()
{
	// serial prefixes 459, 470, 528
	//midway_serial_pic_init(machine(), 528);
}


/********************** WWF Wrestlemania **********************/

// note: other game's PCBs probably shuffle I/O addresses too, but only WWF game code use/require this.
void midwunit_state::wwfmania_io_0_w(uint16_t data)
{
	// start with the originals
	for (int i = 0; i < 16; i++)
		m_ioshuffle[i] = i % 8;

	// based on the data written, shuffle
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
	LOGMASKED(LOG_IO, "Changed I/O swiching to %d\n", data);
}

void midwunit_state::init_wwfmania()
{
	// enable I/O shuffling
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x01800000, 0x0180000f, write16smo_delegate(*this, FUNC(midwunit_state::wwfmania_io_0_w)));

	// serial prefixes 430, 528
	//midway_serial_pic_init(machine(), 528);
}


/********************** Rampage World Tour **********************/

void midwunit_state::init_rmpgwt()
{
	// serial prefixes 465, 528
	//midway_serial_pic_init(machine(), 528);
}


/*************************************
 *
 *  Machine reset
 *
 *************************************/

void midwunit_state::machine_reset()
{
	// reset sound
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);

	// reset I/O shuffling
	for (int i = 0; i < 16; i++)
		m_ioshuffle[i] = i % 8;
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

uint16_t midwunit_state::security_r()
{
	uint16_t picret = 0;
	if (m_midway_serial_pic) picret = m_midway_serial_pic->read();
	if (m_midway_serial_pic_emu) picret = m_midway_serial_pic_emu->read();
	return picret;
}


void midwunit_state::security_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0 && ACCESSING_BITS_0_7)
	{
		if (m_midway_serial_pic) m_midway_serial_pic->write(data);
		if (m_midway_serial_pic_emu) m_midway_serial_pic_emu->write(data);
	}
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

uint16_t midwunit_state::sound_r()
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_SOUND, "%s: Sound read\n", machine().describe_context());

	return m_dcs->data_r() & 0xff;
}


uint16_t midwunit_state::sound_state_r()
{
	return m_dcs->control_r();
}


void midwunit_state::sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// check for out-of-bounds accesses
	if (offset)
	{
		LOGMASKED(LOG_SOUND | LOG_UNKNOWN, "%s: Unexpected write to sound (hi) = %04X\n", machine().describe_context(), data);
		return;
	}

	// call through based on the sound type
	if (ACCESSING_BITS_0_7)
	{
		LOGMASKED(LOG_SOUND, "%s: Sound write = %04X\n", machine().describe_context(), data);
		m_dcs->data_w(data & 0xff);
	}
}
