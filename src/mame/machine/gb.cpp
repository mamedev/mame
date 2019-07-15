// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  gb.c

  Machine file to handle emulation of the Nintendo Game Boy.

Cardridge port pinouts:
Pin  Name     Description
1    VCC      +5 VDC
2    PHI      CPU clock ?
3    /WR      Write
4    /RD      Read
5    /CS      SRAM select
6    A0       Address 0
7    A1       Address 1
8    A2       Address 2
9    A3       Address 3
10   A4       Address 4
11   A5       Address 5
12   A6       Address 6
13   A7       Address 7
14   A8       Address 8
15   A9       Address 9
16   A10      Address 10
17   A11      Address 11
18   A12      Address 12
19   A13      Address 13
20   A14      Address 14
21   A15      Address 15
22   D0       Data 0
23   D1       Data 1
24   D2       Data 2
25   D3       Data 3
26   D4       Data 4
27   D5       Data 5
28   D6       Data 6
29   D7       Data 7
30   /RST     Reset
31   AUDIOIN  Never used ?
32   GND      Ground


TODO:
- YongYong mapper:
  - During start there are 2 writes to 5000 and 5003, it is still unknown what these do.
- Story of La Sa Ma mapper:
  - This should display the Gowin logo on boot on both DMG and CGB (Not implemented yet)
- ATV Racing/Rocket Games mapper:
  - How did this overlay the official Nintendo logo at BIOS check time? (Some Sachen titles use a similar trick)


  Changes:

    13/2/2002       AK - MBC2 and MBC3 support and added NVRAM support.
    23/2/2002       AK - MBC5 support, and MBC2 RAM support.
    13/3/2002       AK - Tidied up the MBC code, window layer now has it's
                         own palette. Tidied init code.
    15/3/2002       AK - More init code tidying with a slight hack to stop
                         sound when the machine starts.
    19/3/2002       AK - Changed NVRAM code to the new battery_* functions.
    24/3/2002       AK - Added MBC1 mode switching, and partial MBC3 RTC support.
    28/3/2002       AK - Improved LCD status timing and interrupts.
                         Free memory when we shutdown instead of leaking.
    31/3/2002       AK - Handle IO memory reading so we return 0xFF for registers
                         that are unsupported.
     7/4/2002       AK - Free memory from battery load/save. General tidying.
    13/4/2002       AK - Ok, don't free memory when we shutdown as that causes
                         a crash on reset.
    28/4/2002       AK - General code tidying.
                         Fixed MBC3's RAM/RTC banking.
                         Added support for games with more than 128 ROM banks.
    12/6/2002       AK - Rewrote the way bg and sprite palettes are handled.
                         The window layer no longer has it's own palette.
                         Added Super Game Boy support.
    13/6/2002       AK - Added Game Boy Color support.

    17/5/2004       WP - Added Megaduck/Cougar Boy support.
    13/6/2005       WP - Added support for bootstrap rom banking.

***************************************************************************/

#include "emu.h"
#include "includes/gb.h"


#define ENABLE_LOGGING 0
#define LOG(x) do { if (ENABLE_LOGGING) logerror x; } while(0)


/* RAM layout defines */
#define CGB_START_RAM_BANKS ( 2 * 8 * 1024 )


#define JOYPAD      m_gb_io[0x00]   /* Joystick: 1.1.P15.P14.P13.P12.P11.P10       */
#define SIODATA     m_gb_io[0x01]   /* Serial IO data buffer                       */
#define SIOCONT     m_gb_io[0x02]   /* Serial IO control register                  */
#define TIMECNT     m_gb_io[0x05]   /* Timer counter. Gen. int. when it overflows  */
#define TIMEMOD     m_gb_io[0x06]   /* New value of TimeCount after it overflows   */
#define TIMEFRQ     m_gb_io[0x07]   /* Timer frequency and start/stop switch       */


//-------------------------
// handle save state
//-------------------------

void gb_state::save_gb_base()
{
	save_item(NAME(m_gb_io));
	save_item(NAME(m_divcount));
	save_item(NAME(m_shift));
	save_item(NAME(m_shift_cycles));
	save_item(NAME(m_triggering_irq));
	save_item(NAME(m_reloading));
	save_item(NAME(m_sio_count));
	save_item(NAME(m_bios_disable));
	if (m_cartslot)
		m_cartslot->save_ram();
}

void gb_state::save_gbc_only()
{
	save_item(NAME(m_gbc_rambank));
}

void gb_state::save_sgb_only()
{
	save_item(NAME(m_sgb_packets));
	save_item(NAME(m_sgb_bitcount));
	save_item(NAME(m_sgb_bytecount));
	save_item(NAME(m_sgb_start));
	save_item(NAME(m_sgb_rest));
	save_item(NAME(m_sgb_controller_no));
	save_item(NAME(m_sgb_controller_mode));
	save_item(NAME(m_sgb_data));
}


void gb_state::gb_init_regs()
{
	/* Initialize the registers */
	SIODATA = 0x00;
	SIOCONT = 0x7E;

	gb_io_w(m_maincpu->space(AS_PROGRAM), 0x05, 0x00);       /* TIMECNT */
	gb_io_w(m_maincpu->space(AS_PROGRAM), 0x06, 0x00);       /* TIMEMOD */
}


void gb_state::gb_init()
{
	m_apu->sound_w(0x16, 0x00);       /* Initialize sound hardware */

	m_divcount = 8;
	m_internal_serial_clock = 0;
	m_internal_serial_frequency = 512 / 2;
	m_triggering_irq = 0;
	m_shift = 10; // slowest timer?
	m_shift_cycles = 1 << m_shift;

	/* Set registers to default/startup values */
	m_gb_io[0x00] = 0xCF;
	m_gb_io[0x01] = 0x00;
	m_gb_io[0x02] = 0x7E;
	m_gb_io[0x03] = 0xFF;
	m_gb_io[0x07] = 0xF8;        /* Upper bits of TIMEFRQ register are set to 1 */
}


void gb_state::machine_start()
{
	save_gb_base();
}

MACHINE_START_MEMBER(gb_state,gbc)
{
	for (int i = 0; i < 8; i++)
		m_gbc_rammap[i] = m_ram->pointer() + CGB_START_RAM_BANKS + i * 0x1000;

	save_gb_base();
	save_gbc_only();
}


MACHINE_START_MEMBER(gb_state,sgb)
{
	m_sgb_packets = -1;

	save_gb_base();
	save_sgb_only();
}

void gb_state::machine_reset()
{
	gb_init();

	// Enable BIOS ROM
	m_bios_disable = false;
}

MACHINE_RESET_MEMBER(gb_state,gbc)
{
	gb_init();

	gb_init_regs();

	/* Enable BIOS rom */
	m_bios_disable = false;

	for (auto & elem : m_gbc_rammap)
		memset(elem, 0, 0x1000);
}

MACHINE_RESET_MEMBER(gb_state,sgb)
{
	gb_init();

	gb_init_regs();

	/* Enable BIOS rom */
	m_bios_disable = false;
}


WRITE8_MEMBER(gb_state::gb_io_w)
{
	static const uint8_t timer_shifts[4] = {10, 4, 6, 8};

	switch (offset)
	{
	case 0x00:                      /* JOYP - Joypad */
		JOYPAD = 0xCF | data;
		if (!(data & 0x20))
			JOYPAD &= (m_inputs->read() >> 4) | 0xF0;
		if (!(data & 0x10))
			JOYPAD &= m_inputs->read() | 0xF0;
		return;
	case 0x01:                      /* SB - Serial transfer data */
		break;
	case 0x02:                      /* SC - SIO control */
		switch (data & 0x81)
		{
		case 0x00:
		case 0x01:
			m_sio_count = 0;
		case 0x80:              /* enabled & external clock */
			m_sio_count = 16;
			break;
		case 0x81:              /* enabled & internal clock */
			m_sio_count = 16;
			break;
		}
logerror("SIOCONT write, serial clock is %04x\n", m_internal_serial_clock);
		data |= 0x7E; // unused bits stay high
		break;
	case 0x03:
		return;
	case 0x04:                      /* DIV - Divider register */
		/* Force increment of TIMECNT register when the 'highest' bit is set */
		if ((m_divcount >> (m_shift - 1)) & 1)
		{
			gb_timer_increment();
		}
		LOG(("DIV write\n"));
		m_divcount = 0;
		return;
	case 0x05:                      /* TIMA - Timer counter */
		/* Check if the counter is being reloaded in this cycle */
		if ((TIMEFRQ & 0x04) && TIMECNT == TIMEMOD && (m_divcount & (m_shift_cycles - 1)) == 4)
		{
			data = TIMEMOD;
		}
		break;
	case 0x06:                      /* TMA - Timer module */
		/* Check if the counter is being reloaded in this cycle */
		if ((TIMEFRQ & 0x04) && TIMECNT == TIMEMOD && (m_divcount & (m_shift_cycles - 1)) == 4)
		{
			TIMECNT = data;
		}
		break;
	case 0x07:                      /* TAC - Timer control */
		data |= 0xF8;
		/* Check if timer is just disabled or the timer frequency is changing */
		if ((!(data & 0x04) && (TIMEFRQ & 0x04)) || ((data & 0x04) && (TIMEFRQ & 0x04) && (data & 0x03) != (TIMEFRQ & 0x03)))
		{
			/* Check if TIMECNT should be incremented */
			if ((m_divcount & (m_shift_cycles - 1)) >= (m_shift_cycles >> 1))
			{
				gb_timer_increment();
			}
		}
		m_shift = timer_shifts[data & 0x03];
		m_shift_cycles = 1 << m_shift;
		break;
	case 0x0F:                      /* IF - Interrupt flag */
		m_ppu->update_state();
		LOG(("write if\n"));
		data &= 0x1F;
		m_maincpu->set_if(data);
		break;
	}

	m_gb_io[offset] = data;
}

WRITE8_MEMBER(gb_state::gb_io2_w)
{
	if (offset == 0x10)
	{
		/* disable BIOS ROM */
		m_bios_disable = true;
	}
	else
		m_ppu->video_w(space, offset, data);
}

#ifdef MAME_DEBUG
static const char *const sgbcmds[32] =
{
	/* 0x00 */ "PAL01   ",
	/* 0x01 */ "PAL23   ",
	/* 0x02 */ "PAL03   ",
	/* 0x03 */ "PAL12   ",
	/* 0x04 */ "ATTR_BLK",
	/* 0x05 */ "ATTR_LIN",
	/* 0x06 */ "ATTR_DIV",
	/* 0x07 */ "ATTR_CHR",
	/* 0x08 */ "SOUND   ",
	/* 0x09 */ "SOU_TRN ",
	/* 0x0A */ "PAL_SET ",
	/* 0x0B */ "PAL_TRN ",
	/* 0x0C */ "ATRC_EN ",
	/* 0x0D */ "TEST_EN ",
	/* 0x0E */ "ICON_EN ",
	/* 0x0F */ "DATA_SND",
	/* 0x10 */ "DATA_TRN",
	/* 0x11 */ "MLT_REG ",
	/* 0x12 */ "JUMP    ",
	/* 0x13 */ "CHR_TRN ",
	/* 0x14 */ "PCT_TRN ",
	/* 0x15 */ "ATTR_TRN",
	/* 0x16 */ "ATTR_SET",
	/* 0x17 */ "MASK_EN ",
	/* 0x18 */ "OBJ_TRN ",
	/* 0x19 */ "PAL_PRI ",
	/* 0x1A */ "????????",
	/* 0x1B */ "????????",
	/* 0x1C */ "????????",
	/* 0x1D */ "????????",
	/* 0x1E */ "????????",
	/* 0x1F */ "????????"
};
#endif

WRITE8_MEMBER(gb_state::sgb_io_w)
{
	uint8_t *sgb_data = m_sgb_data;

	switch (offset)
	{
		case 0x00:
			switch (data & 0x30)
			{
			case 0x00:                 /* start condition */
				if (m_sgb_start)
					logerror("SGB: Start condition before end of transfer ??\n");
				m_sgb_bitcount = 0;
				m_sgb_start = 1;
				m_sgb_rest = 0;
				JOYPAD = 0x0F & ((m_inputs->read() >> 4) | m_inputs->read() | 0xF0);
				break;
			case 0x10:                 /* data true */
				if (m_sgb_rest)
				{
					/* We should test for this case , but the code below won't
					   work with the current setup */
#if 0
					if (m_sgb_bytecount == 16)
					{
						logerror("SGB: end of block is not zero!");
						m_sgb_start = 0;
					}
#endif
					sgb_data[m_sgb_bytecount] >>= 1;
					sgb_data[m_sgb_bytecount] |= 0x80;
					m_sgb_bitcount++;
					if (m_sgb_bitcount == 8)
					{
						m_sgb_bitcount = 0;
						m_sgb_bytecount++;
					}
					m_sgb_rest = 0;
				}
				JOYPAD = 0x1F & ((m_inputs->read() >> 4) | 0xF0);
				break;
			case 0x20:              /* data false */
				if (m_sgb_rest)
				{
					if (m_sgb_bytecount == 16 && m_sgb_packets == -1)
					{
#ifdef MAME_DEBUG
						LOG(("SGB: %s (%02X) pkts: %d data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
								sgbcmds[sgb_data[0] >> 3],sgb_data[0] >> 3, sgb_data[0] & 0x07, sgb_data[1], sgb_data[2], sgb_data[3],
								sgb_data[4], sgb_data[5], sgb_data[6], sgb_data[7],
								sgb_data[8], sgb_data[9], sgb_data[10], sgb_data[11],
								sgb_data[12], sgb_data[13], sgb_data[14], sgb_data[15]));
#endif
						m_sgb_packets = sgb_data[0] & 0x07;
						m_sgb_start = 0;
					}
					if (m_sgb_bytecount == (m_sgb_packets << 4))
					{
						switch (sgb_data[0] >> 3)
						{
							case 0x11:  /* MLT_REQ - Multi controller request */
								if (sgb_data[1] == 0x00)
									m_sgb_controller_mode = 0;
								else if (sgb_data[1] == 0x01)
									m_sgb_controller_mode = 2;
								break;
							default:
								dynamic_cast<sgb_ppu_device*>(m_ppu.target())->sgb_io_write_pal(sgb_data[0] >> 3, &sgb_data[0]);
								break;
						}
						m_sgb_start = 0;
						m_sgb_bytecount = 0;
						m_sgb_packets = -1;
					}
					if (m_sgb_start)
					{
						sgb_data[m_sgb_bytecount] >>= 1;
						m_sgb_bitcount++;
						if (m_sgb_bitcount == 8)
						{
							m_sgb_bitcount = 0;
							m_sgb_bytecount++;
						}
					}
					m_sgb_rest = 0;
				}
				JOYPAD = 0x2F & (m_inputs->read() | 0xF0);
				break;
			case 0x30:                 /* rest condition */
				if (m_sgb_start)
					m_sgb_rest = 1;
				if (m_sgb_controller_mode)
				{
					m_sgb_controller_no++;
					if (m_sgb_controller_no == m_sgb_controller_mode)
						m_sgb_controller_no = 0;
					JOYPAD = 0x3F - m_sgb_controller_no;
				}
				else
					JOYPAD = 0x3F;

				/* Hack to let cartridge know it's running on an SGB */
				if ((sgb_data[0] >> 3) == 0x1F)
					JOYPAD = 0x3E;
				break;
			}
			return;
		default:
			/* we didn't handle the write, so pass it to the GB handler */
			gb_io_w(space, offset, data);
			return;
	}

	m_gb_io[offset] = data;
}

/* Interrupt Enable register */
READ8_MEMBER(gb_state::gb_ie_r)
{
	return m_maincpu->get_ie();
}

WRITE8_MEMBER(gb_state::gb_ie_w)
{
	m_maincpu->set_ie(data);
}

/* IO read */
READ8_MEMBER(gb_state::gb_io_r)
{
	switch(offset)
	{
		case 0x04:
			LOG(("read DIV, divcount = %04x\n", m_divcount));
			return (m_divcount >> 8) & 0xFF;
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x05:
		case 0x06:
		case 0x07:
			return m_gb_io[offset];
		case 0x0F:
			/* Make sure the internal states are up to date */
			m_ppu->update_state();
			LOG(("read if\n"));
logerror("IF read, serial clock is %04x\n", m_internal_serial_clock);
			return 0xE0 | m_maincpu->get_if();
		default:
			/* Unsupported registers return 0xFF */
			return 0xFF;
	}
}


/* Called when 512 internal cycles are passed */
void gb_state::gb_serial_timer_tick()
{
	if (SIOCONT & SIO_ENABLED)
	{
		if (m_sio_count & 1)
		{
			/* Shift in a received bit */
			SIODATA = (SIODATA << 1) | 0x01;
		}
		/* Decrement number of handled bits */
		m_sio_count--;

		LOG(("%04x - gb_serial_timer_proc: SIODATA = %02x, sio_count = %u\n", m_maincpu->pc(), SIODATA, m_sio_count));
		/* If all bits done, stop timer and trigger interrupt */
		if (m_sio_count == 0)
		{
			SIOCONT &= ~SIO_ENABLED;
			m_maincpu->set_input_line(lr35902_cpu_device::SIO_INT, ASSERT_LINE);
			// Make sure the state is updated during the current timeslice in case it is read.
			m_maincpu->execute_set_input(lr35902_cpu_device::SIO_INT, ASSERT_LINE);
		}
	}
}


void gb_state::gb_timer_check_irq()
{
	m_reloading = 0;
	if (m_triggering_irq)
	{
		m_triggering_irq = 0;
		if (TIMECNT == 0)
		{
			TIMECNT = TIMEMOD;
			m_maincpu->set_input_line(lr35902_cpu_device::TIM_INT, ASSERT_LINE);
			// Make sure the state is updated during the current timeslice in case it is read.
			m_maincpu->execute_set_input(lr35902_cpu_device::TIM_INT, ASSERT_LINE);
			m_reloading = 1;
		}
	}
}

void gb_state::gb_timer_increment()
{
	gb_timer_check_irq();

	LOG(("increment timer\n"));
	TIMECNT += 1;
	if (TIMECNT == 0)
	{
		m_triggering_irq = 1;
	}
}

// This gets called while the cpu is executing instructions to keep the timer state in sync
WRITE8_MEMBER(gb_state::gb_timer_callback)
{
	uint16_t old_gb_divcount = m_divcount;
	uint16_t old_internal_serial_clock = m_internal_serial_clock;
	m_divcount += data;
	m_internal_serial_clock += data;

	if ( (old_gb_divcount >> 8) != (m_divcount >> 8)) {
		//LOG(("DIV became %02x\n", m_divcount >> 8));
	}
	gb_timer_check_irq();

	if (TIMEFRQ & 0x04)
	{
		uint16_t old_count = old_gb_divcount >> m_shift;
		uint16_t new_count = m_divcount >> m_shift;
		if (data > m_shift_cycles)
		{
			gb_timer_increment();
			old_count++;
		}
		if (new_count != old_count)
		{
			gb_timer_increment();
			if (new_count << m_shift < m_divcount)
			{
				gb_timer_check_irq();
			}
		}
	}

	if (((m_internal_serial_clock ^ old_internal_serial_clock) & m_internal_serial_frequency) && (SIOCONT & SIO_INTERNAL_CLOCK))
	{
		gb_serial_timer_tick();
	}
}


WRITE8_MEMBER(gb_state::gbc_io_w)
{
	gb_io_w(space, offset, data);

	// On CGB the internal serial transfer clock is selectable
	if (offset == 0x02)
	{
		m_internal_serial_frequency = ((data & SIO_FAST_CLOCK) ? 16 : 512) / 2;
		SIOCONT = (SIOCONT & ~SIO_FAST_CLOCK) | (data & SIO_FAST_CLOCK);
	}
}


WRITE8_MEMBER(gb_state::gbc_io2_w)
{
	switch (offset)
	{
		case 0x0D:  /* KEY1 - Prepare speed switch */
			m_maincpu->set_speed(data);
			return;
		case 0x10:  /* BFF - Bios disable */
			m_bios_disable = true;
			return;
		case 0x16:  /* RP - Infrared port */
			break;
		case 0x30:  /* SVBK - RAM bank select */
			m_gbc_rambank = data & 0x7;
			if (!m_gbc_rambank)
				m_gbc_rambank = 1;
			m_rambank->set_base(m_gbc_rammap[m_gbc_rambank]);
			break;
		default:
			break;
	}
	m_ppu->video_w(space, offset, data);
}

READ8_MEMBER(gb_state::gbc_io2_r)
{
	switch (offset)
	{
	case 0x0D:  /* KEY1 */
		return m_maincpu->get_speed();
	case 0x16:  /* RP - Infrared port */
		break;
	case 0x30:  /* SVBK - RAM bank select */
		return m_gbc_rambank;
	default:
		break;
	}
	return m_ppu->video_r(space, offset);
}

/****************************************************************************

  Megaduck routines

 ****************************************************************************/

void megaduck_state::machine_start()
{
	gb_state::machine_start();
}

void megaduck_state::machine_reset()
{
	// We may have to add some more stuff here, if not then it can be merged back into gb
	gb_state::machine_reset();

	m_bios_disable = true;
}

/*
 Map megaduck video related area on to regular Game Boy video area

 Different locations of the video registers:
 Register      Game Boy   MegaDuck
 LCDC          FF40       FF10  (See different bit order below)
 STAT          FF41       FF11
 SCY           FF42       FF12
 SCX           FF43       FF13
 LY            FF44       FF18
 LYC           FF45       FF19
 DMA           FF46       FF1A
 BGP           FF47       FF1B
 OBP0          FF48       FF14
 OBP1          FF49       FF15
 WY            FF4A       FF16
 WX            FF4B       FF17
 Unused        FF4C       FF4C (?)
 Unused        FF4D       FF4D (?)
 Unused        FF4E       FF4E (?)
 Unused        FF4F       FF4F (?)

 Different LCDC register

 Game Boy       MegaDuck
 0                      6       - BG & Window Display : 0 - Off, 1 - On
 1                      0       - OBJ Display: 0 - Off, 1 - On
 2                      1       - OBJ Size: 0 - 8x8, 1 - 8x16
 3                      2       - BG Tile Map Display: 0 - 9800, 1 - 9C00
 4                      4       - BG & Window Tile Data Select: 0 - 8800, 1 - 8000
 5                      5       - Window Display: 0 - Off, 1 - On
 6                      3       - Window Tile Map Display Select: 0 - 9800, 1 - 9C00
 7                      7       - LCD Operation

 **************/

READ8_MEMBER(megaduck_state::megaduck_video_r)
{
	uint8_t data;

	if ((offset & 0x0C) && ((offset & 0x0C) ^ 0x0C))
	{
		offset ^= 0x0C;
	}
	data = m_ppu->video_r(space, offset);
	if (offset)
		return data;
	return bitswap<8>(data,7,0,5,4,6,3,2,1);
}

WRITE8_MEMBER(megaduck_state::megaduck_video_w)
{
	if (!offset)
	{
		data = bitswap<8>(data,7,3,5,4,2,1,0,6);
	}
	if ((offset & 0x0C) && ((offset & 0x0C) ^ 0x0C))
	{
		offset ^= 0x0C;
	}
	m_ppu->video_w(space, offset, data);
}

/* Map megaduck audio offset to game boy audio offsets */
/* Envelope and LFSR register nibbles are reversed relative to the game boy */

static const uint8_t megaduck_sound_offsets[16] = { 0, 2, 1, 3, 4, 6, 5, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

WRITE8_MEMBER(megaduck_state::megaduck_sound_w1)
{
	if ((offset == 0x01) || (offset == 0x07))
		m_apu->sound_w(megaduck_sound_offsets[offset], ((data & 0x0f)<<4) | ((data & 0xf0)>>4));
	else
		m_apu->sound_w(megaduck_sound_offsets[offset], data);
}

READ8_MEMBER(megaduck_state::megaduck_sound_r1)
{
	uint8_t data = m_apu->sound_r(megaduck_sound_offsets[offset]);
	if ((offset == 0x01) || (offset == 0x07))
		return ((data & 0x0f)<<4) | ((data & 0xf0)>>4);
	else
		return data;
}

WRITE8_MEMBER(megaduck_state::megaduck_sound_w2)
{
	if ((offset == 0x01) || (offset == 0x02))
		m_apu->sound_w(0x10 + megaduck_sound_offsets[offset], ((data & 0x0f)<<4) | ((data & 0xf0)>>4));
	else
		m_apu->sound_w(0x10 + megaduck_sound_offsets[offset], data);
}

READ8_MEMBER(megaduck_state::megaduck_sound_r2)
{
	uint8_t data = m_apu->sound_r(0x10 + megaduck_sound_offsets[offset]);
	if ((offset == 0x01) || (offset == 0x02))
		return ((data & 0x0f)<<4) | ((data & 0xf0)>>4);
	else
		return data;
}
