// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/******************************************************************************

    pcw16.c
    system driver

    Kevin Thacker [MESS driver]

  Thankyou to:

    - Cliff Lawson @ Amstrad plc for his documentation (Anne ASIC documentation),
                    and extensive help.
            (web.ukonline.co.uk/cliff.lawson/)
            (www.amstrad.com)
    - John Elliot for his help and tips
            (he's written a CP/M implementation for the PCW16)
            (www.seasip.deomon.co.uk)
    - and others who offered their help (Richard Fairhurst, Richard Wildey)

    Hardware:
        - 2mb dram max,
        - 2mb flash-file memory max (in 2 1mb chips),
        - 16MHz Z80 (core combined in Anne ASIC),
        - Anne ASIC (keyboard interface, video (colours), dram/flash/rom paging,
        real time clock, "glue" logic for Super I/O)
        - Winbond Super I/O chip (PC type hardware - FDC, Serial, LPT, Hard-drive)
        - PC/AT keyboard - some keys are coloured to identify special functions, but
        these are the same as normal PC keys
        - PC Serial Mouse - uses Mouse System Mouse protocol
        - PC 1.44MB Floppy drive

    Primary Purpose:
        - built as a successor to the PCW8526/PCW9512 series
        - wordprocessor system (also contains spreadsheet and other office applications)
        - 16MHz processor used so proportional fonts and enhanced wordprocessing features
          are possible, true WYSIWYG wordprocessing.
        - flash-file can store documents.

    To Do:
        - reduce memory usage so it is more MESSD friendly
        - different configurations
        - implement configuration register
        - extract game-port hardware from pc driver - used in any PCW16 progs?
        - extract hard-drive code from PC driver and use in this driver
        - implement printer
        - .. anything else that requires implementing

     Info:
       - to use this driver you need a OS rescue disc.
       (HINT: This also contains the boot-rom)
      - the OS will be installed from the OS rescue disc into the Flash-ROM

    Uses "MEMCARD" dir to hold flash-file data.
    To use the power button, flick the dip switch off/on

 From comp.sys.amstrad.8bit FAQ:

  "Amstrad made the following PCW systems :

  - 1) PCW8256
  - 2) PCW8512
  - 3) PCW9512
  - 4) PCW9512+
  - 5) PcW10
  - 6) PcW16

  1 had 180K drives, 2 had a 180K A drive and a 720K B drive, 3 had only
  720K drives. All subsequent models had 3.5" disks using CP/M format at
  720K until 6 when it switched to 1.44MB in MS-DOS format. The + of
  model 4 was that it had a "real" parallel interface so could be sold
  with an external printer such as the Canon BJ10. The PcW10 wasn't
  really anything more than 4 in a more modern looking case.

  The PcW16 is a radical digression who's sole "raison d'etre" was to
  make a true WYSIWYG product but this meant a change in the screen and
  processor (to 16MHz) etc. which meant that it could not be kept
  compatible with the previous models (though documents ARE compatible)"


TODO:
- Verfiy uart model.


 ******************************************************************************/
/* PeT 19.October 2000
   added/changed printer support
   not working reliable, seams to expect parallelport in epp/ecp mode
   epp/ecp modes in parallel port not supported yet
   so ui disabled */

#include "emu.h"
#include "includes/pcw16.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

// interrupt counter
/* controls which bank of 2mb address space is paged into memory */

// output of 4-bit port from Anne ASIC
// code defining which INT fdc is connected to
// interrupt bits
// bit 7: ??
// bit 6: fdc
// bit 5: ??
// bit 3,4; Serial
// bit 2: Vsync state
// bit 1: keyboard int
// bit 0: Display ints

// debugging - write ram as seen by cpu
void pcw16_state::pcw16_refresh_ints()
{
	/* any bits set excluding vsync */
	if ((m_system_status & (~0x04))!=0)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(pcw16_state::pcw16_timer_callback)
{
	/* do not increment past 15 */
	if (m_interrupt_counter!=15)
	{
		m_interrupt_counter++;
		/* display int */
		m_system_status |= (1<<0);
	}

	if (m_interrupt_counter!=0)
	{
		pcw16_refresh_ints();
	}
}

void pcw16_state::pcw16_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(pcw16_state::pcw16_mem_r), FUNC(pcw16_state::pcw16_mem_w));
}


WRITE8_MEMBER(pcw16_state::pcw16_palette_w)
{
	m_colour_palette[offset & 0x0f] = data & 31;
}

uint8_t pcw16_state::read_bank_data(uint8_t type, uint16_t offset)
{
	if(type & 0x80) // DRAM
	{
		return m_ram->pointer()[((type & 0x7f)*0x4000) + offset];
	}
	else  // ROM / Flash
	{
		if(type < 4)
		{
			return m_region_rom->base()[((type & 0x03)*0x4000)+offset+0x10000];
		}
		if(type < 0x40)  // first flash
		{
			return m_flash0->read(((type & 0x3f)*0x4000)+offset);
		}
		else  // second flash
		{
			return m_flash1->read(((type & 0x3f)*0x4000)+offset);
		}
	}
}

void pcw16_state::write_bank_data(uint8_t type, uint16_t offset, uint8_t data)
{
	if(type & 0x80) // DRAM
	{
		m_ram->pointer()[((type & 0x7f)*0x4000) + offset] = data;
	}
	else  // ROM / Flash
	{
		if(type < 4)
			return;  // first four sectors are write protected
		if(type < 0x40)  // first flash
		{
			m_flash0->write(((type & 0x3f)*0x4000)+offset, data);
		}
		else  // second flash
		{
			m_flash1->write(((type & 0x3f)*0x4000)+offset, data);
		}
	}
}

uint8_t pcw16_state::pcw16_read_mem(uint8_t bank, uint16_t offset)
{
	switch(bank)
	{
	case 0:
		return read_bank_data(m_banks[bank],offset);
	case 1:
		return read_bank_data(m_banks[bank],offset);
	case 2:
		return read_bank_data(m_banks[bank],offset);
	case 3:
		return read_bank_data(m_banks[bank],offset);
	}
	return 0xff;
}

void pcw16_state::pcw16_write_mem(uint8_t bank, uint16_t offset, uint8_t data)
{
	switch(bank)
	{
	case 0:
		write_bank_data(m_banks[bank],offset,data);
		break;
	case 1:
		write_bank_data(m_banks[bank],offset,data);
		break;
	case 2:
		write_bank_data(m_banks[bank],offset,data);
		break;
	case 3:
		write_bank_data(m_banks[bank],offset,data);
		break;
	}
}

READ8_MEMBER(pcw16_state::pcw16_mem_r)
{
	if(offset < 0x4000)
		return pcw16_read_mem(0,offset);
	if(offset >= 0x4000 && offset < 0x8000)
		return pcw16_read_mem(1,offset-0x4000);
	if(offset >= 0x8000 && offset < 0xc000)
		return pcw16_read_mem(2,offset-0x8000);
	if(offset >= 0xc000 && offset < 0x10000)
		return pcw16_read_mem(3,offset-0xc000);

	return 0xff;
}

WRITE8_MEMBER(pcw16_state::pcw16_mem_w)
{
	if(offset < 0x4000)
		pcw16_write_mem(0,offset,data);
	if(offset >= 0x4000 && offset < 0x8000)
		pcw16_write_mem(1,offset-0x4000,data);
	if(offset >= 0x8000 && offset < 0xc000)
		pcw16_write_mem(2,offset-0x8000,data);
	if(offset >= 0xc000 && offset < 0x10000)
		pcw16_write_mem(3,offset-0xc000,data);
}

READ8_MEMBER(pcw16_state::pcw16_bankhw_r)
{
//  logerror("bank r: %d \n", offset);

	return m_banks[offset];
}

WRITE8_MEMBER(pcw16_state::pcw16_bankhw_w)
{
	//logerror("bank w: %d block: %02x\n", offset, data);

	m_banks[offset] = data;
}

WRITE8_MEMBER(pcw16_state::pcw16_video_control_w)
{
	//logerror("video control w: %02x\n", data);

	m_video_control = data;
}

/* PCW16 KEYBOARD */

//unsigned char pcw16_keyboard_status;



#define PCW16_KEYBOARD_PARITY_MASK  (1<<7)
#define PCW16_KEYBOARD_STOP_BIT_MASK (1<<6)
#define PCW16_KEYBOARD_START_BIT_MASK (1<<5)
#define PCW16_KEYBOARD_BUSY_STATUS  (1<<4)
#define PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK (1<<1)
#define PCW16_KEYBOARD_TRANSMIT_MODE (1<<0)

#define PCW16_KEYBOARD_RESET_INTERFACE (1<<2)

#define PCW16_KEYBOARD_DATA (1<<1)
#define PCW16_KEYBOARD_CLOCK (1<<0)

/* parity table. Used to set parity bit in keyboard status register */

void pcw16_state::pcw16_keyboard_init()
{
	int i;
	int b;

	/* if sum of all bits in the byte is even, then the data
	has even parity, otherwise it has odd parity */
	for (i=0; i<256; i++)
	{
		int data;
		int sum;

		sum = 0;
		data = i;

		for (b=0; b<8; b++)
		{
			sum+=data & 0x01;

			data = data>>1;
		}

		m_keyboard_parity_table[i] = sum & 0x01;
	}


	/* clear int */
	pcw16_keyboard_int(0);
	/* reset state */
	m_keyboard_state = 0;
	/* reset ready for transmit */
	pcw16_keyboard_reset();
}

void pcw16_state::pcw16_keyboard_refresh_outputs()
{
	/* generate output bits */
	m_keyboard_bits_output = m_keyboard_bits;

	/* force clock low? */
	if (m_keyboard_state & PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK)
	{
		m_keyboard_bits_output &= ~PCW16_KEYBOARD_CLOCK;
	}
}

void pcw16_state::pcw16_keyboard_set_clock_state(int state)
{
	m_keyboard_bits &= ~PCW16_KEYBOARD_CLOCK;

	if (state)
	{
		m_keyboard_bits |= PCW16_KEYBOARD_CLOCK;
	}

	pcw16_keyboard_refresh_outputs();
}

void pcw16_state::pcw16_keyboard_int(int state)
{
	m_system_status &= ~(1<<1);

	if (state)
	{
		m_system_status |= (1<<1);
	}

	pcw16_refresh_ints();
}

void pcw16_state::pcw16_keyboard_reset()
{
	/* clock set to high */
	pcw16_keyboard_set_clock_state(1);
}

/* interfaces to a pc-at keyboard */
READ8_MEMBER(pcw16_state::pcw16_keyboard_data_shift_r)
{
	//logerror("keyboard data shift r: %02x\n", m_keyboard_data_shift);
	m_keyboard_state &= ~(PCW16_KEYBOARD_BUSY_STATUS);

	pcw16_keyboard_int(0);
	/* reset for reception */
	pcw16_keyboard_reset();

	/* read byte */
	return m_keyboard_data_shift;
}

/* if force keyboard clock is low it is safe to send */
int pcw16_state::pcw16_keyboard_can_transmit()
{
	/* clock is not forced low */
	/* and not busy - i.e. not already sent a char */
	return ((m_keyboard_bits_output & PCW16_KEYBOARD_CLOCK)!=0);
}

#ifdef UNUSED_FUNCTION
/* issue a begin byte transfer */
void ::pcw16_begin_byte_transfer(void)
{
}
#endif

/* signal a code has been received */
void pcw16_state::pcw16_keyboard_signal_byte_received(int data)
{
	/* clear clock */
	pcw16_keyboard_set_clock_state(0);

	/* set code in shift register */
	m_keyboard_data_shift = data;
	/* busy */
	m_keyboard_state |= PCW16_KEYBOARD_BUSY_STATUS;

	/* initialise start, stop and parity bits */
	m_keyboard_state &= ~PCW16_KEYBOARD_START_BIT_MASK;
	m_keyboard_state |=PCW16_KEYBOARD_STOP_BIT_MASK;

	/* "Keyboard data has odd parity, so the parity bit in the
	status register should only be set when the shift register
	data itself has even parity. */

	m_keyboard_state &= ~PCW16_KEYBOARD_PARITY_MASK;

	/* if data has even parity, set parity bit */
	if ((m_keyboard_parity_table[data])==0)
		m_keyboard_state |= PCW16_KEYBOARD_PARITY_MASK;

	pcw16_keyboard_int(1);
}


WRITE8_MEMBER(pcw16_state::pcw16_keyboard_data_shift_w)
{
	//logerror("Keyboard Data Shift: %02x\n", data);
	/* writing to shift register clears parity */
	/* writing to shift register clears start bit */
	m_keyboard_state &= ~(
		PCW16_KEYBOARD_PARITY_MASK |
		PCW16_KEYBOARD_START_BIT_MASK);

	/* writing to shift register sets stop bit */
	m_keyboard_state |= PCW16_KEYBOARD_STOP_BIT_MASK;

	m_keyboard_data_shift = data;

}

READ8_MEMBER(pcw16_state::pcw16_keyboard_status_r)
{
	/* bit 2,3 are bits 8 and 9 of vdu pointer */
	return (m_keyboard_state &
		(PCW16_KEYBOARD_PARITY_MASK |
			PCW16_KEYBOARD_STOP_BIT_MASK |
			PCW16_KEYBOARD_START_BIT_MASK |
			PCW16_KEYBOARD_BUSY_STATUS |
			PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK |
			PCW16_KEYBOARD_TRANSMIT_MODE));
}

WRITE8_MEMBER(pcw16_state::pcw16_keyboard_control_w)
{
	//logerror("Keyboard control w: %02x\n",data);

	m_keyboard_previous_state = m_keyboard_state;

	/* if set, set parity */
	if (data & 0x080)
	{
		m_keyboard_state |= PCW16_KEYBOARD_PARITY_MASK;
	}

	/* clear read/write bits */
	m_keyboard_state &=
		~(PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK |
			PCW16_KEYBOARD_TRANSMIT_MODE);
	/* set read/write bits from data */
	m_keyboard_state |= (data & 0x03);

	if (data & PCW16_KEYBOARD_RESET_INTERFACE)
	{
		pcw16_keyboard_reset();
	}

	if (data & PCW16_KEYBOARD_TRANSMIT_MODE)
	{
		/* force clock changed */
		if (((m_keyboard_state^m_keyboard_previous_state) & PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK)!=0)
		{
			/* just cleared? */
			if ((m_keyboard_state & PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK)==0)
			{
				/* write */
				/* busy */
				m_keyboard_state |= PCW16_KEYBOARD_BUSY_STATUS;
				/* keyboard takes data */
				m_keyboard->write(space, 0, m_keyboard_data_shift);
				/* set clock low - no furthur transmissions */
				pcw16_keyboard_set_clock_state(0);
				/* set int */
				pcw16_keyboard_int(1);
			}
		}


	}

	if (((m_keyboard_state^m_keyboard_previous_state) & PCW16_KEYBOARD_TRANSMIT_MODE)!=0)
	{
		if ((m_keyboard_state & PCW16_KEYBOARD_TRANSMIT_MODE)==0)
		{
			if ((m_system_status & (1<<1))!=0)
			{
				pcw16_keyboard_int(0);
			}
		}
	}

	pcw16_keyboard_refresh_outputs();
}


WRITE_LINE_MEMBER(pcw16_state::pcw16_keyboard_callback)
{
	if(!state)
		return;

	if (pcw16_keyboard_can_transmit())
	{
		int data;

		data = m_keyboard->read(machine().dummy_space(), 0);

		if (data)
		{
//          if (data==4)
//          {
//              pcw16_dump_cpu_ram();
//          }

			pcw16_keyboard_signal_byte_received(data);
		}
	}
}


static const int rtc_days_in_each_month[]=
{
	31,/* jan */
	28, /* feb */
	31, /* march */
	30, /* april */
	31, /* may */
	30, /* june */
	31, /* july */
	31, /* august */
	30, /* september */
	31, /* october */
	30, /* november */
	31  /* december */
};

static const int rtc_days_in_february[] =
{
	29, 28, 28, 28
};

void pcw16_state::rtc_setup_max_days()
{
	/* february? */
	if (m_rtc_months == 2)
	{
		/* low two bits of year select number of days in february */
		m_rtc_days_max = rtc_days_in_february[m_rtc_years & 0x03];
	}
	else
	{
		m_rtc_days_max = (unsigned char)rtc_days_in_each_month[m_rtc_months];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pcw16_state::rtc_timer_callback)
{
	int fraction_of_second;

	/* halt counter? */
	if ((m_rtc_control & 0x01)!=0)
	{
		/* no */

		/* increment 256th's of a second register */
		fraction_of_second = m_rtc_256ths_seconds+1;
		/* add bit 8 = overflow */
		m_rtc_seconds+=(fraction_of_second>>8);
		/* ensure counter is in range 0-255 */
		m_rtc_256ths_seconds = fraction_of_second & 0x0ff;
	}

	if (m_rtc_seconds>59)
	{
		m_rtc_seconds = 0;

		m_rtc_minutes++;

		if (m_rtc_minutes>59)
		{
			m_rtc_minutes = 0;

			m_rtc_hours++;

			if (m_rtc_hours>23)
			{
				m_rtc_hours = 0;

				m_rtc_days++;

				if (m_rtc_days > m_rtc_days_max)
				{
					m_rtc_days = 1;

					m_rtc_months++;

					if (m_rtc_months>12)
					{
						m_rtc_months = 1;

						/* 7 bit year counter */
						m_rtc_years = (m_rtc_years + 1) & 0x07f;

					}

					rtc_setup_max_days();
				}

			}


		}
	}
}

READ8_MEMBER(pcw16_state::rtc_year_invalid_r)
{
	/* year in lower 7 bits. RTC Invalid status is m_rtc_control bit 0
	inverted */
	return (m_rtc_years & 0x07f) | (((m_rtc_control & 0x01)<<7)^0x080);
}

READ8_MEMBER(pcw16_state::rtc_month_r)
{
	return m_rtc_months;
}

READ8_MEMBER(pcw16_state::rtc_days_r)
{
	return m_rtc_days;
}

READ8_MEMBER(pcw16_state::rtc_hours_r)
{
	return m_rtc_hours;
}

READ8_MEMBER(pcw16_state::rtc_minutes_r)
{
	return m_rtc_minutes;
}

READ8_MEMBER(pcw16_state::rtc_seconds_r)
{
	return m_rtc_seconds;
}

READ8_MEMBER(pcw16_state::rtc_256ths_seconds_r)
{
	return m_rtc_256ths_seconds;
}

WRITE8_MEMBER(pcw16_state::rtc_control_w)
{
	/* write control */
	m_rtc_control = data;
}

WRITE8_MEMBER(pcw16_state::rtc_seconds_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_seconds = data;
}

WRITE8_MEMBER(pcw16_state::rtc_minutes_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_minutes = data;
}

WRITE8_MEMBER(pcw16_state::rtc_hours_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_hours = data;
}

WRITE8_MEMBER(pcw16_state::rtc_days_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_days = data;
}

WRITE8_MEMBER(pcw16_state::rtc_month_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_months = data;

	rtc_setup_max_days();
}


WRITE8_MEMBER(pcw16_state::rtc_year_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_hours = data;

	rtc_setup_max_days();
}


void pcw16_state::trigger_fdc_int()
{
	int state;

	state = m_system_status & (1<<6);

	switch (m_fdc_int_code)
	{
		/* nmi */
		case 0:
		{
			/* I'm assuming that the nmi is edge triggered */
			/* a interrupt from the fdc will cause a change in line state, and
			the nmi will be triggered, but when the state changes because the int
			is cleared this will not cause another nmi */
			/* I'll emulate it like this to be sure */

			if (state!=m_previous_fdc_int_state)
			{
				if (state)
				{
					/* I'll pulse it because if I used hold-line I'm not sure
					it would clear - to be checked */
					m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
				}
			}
		}
		break;

		/* attach fdc to int */
		case 1:
		{
			pcw16_refresh_ints();
		}
		break;

		/* do not interrupt */
		default:
			break;
	}

	m_previous_fdc_int_state = state;
}

READ8_MEMBER(pcw16_state::pcw16_system_status_r)
{
//  logerror("system status r: \n");

	return m_system_status | (m_io_extra->read() & 0x04);
}

READ8_MEMBER(pcw16_state::pcw16_timer_interrupt_counter_r)
{
	int data;

	data = m_interrupt_counter;

	m_interrupt_counter = 0;
	/* clear display int */
	m_system_status &= ~(1<<0);

	pcw16_refresh_ints();

	return data;
}


WRITE8_MEMBER(pcw16_state::pcw16_system_control_w)
{
	//logerror("0x0f8: function: %d\n",data);

	/* lower 4 bits define function code */
	switch (data & 0x0f)
	{
		/* no effect */
		case 0x00:
		case 0x09:
		case 0x0a:
		case 0x0d:
		case 0x0e:
			break;

		/* system reset */
		case 0x01:
			break;

		/* connect IRQ6 input to /NMI */
		case 0x02:
		{
			m_fdc_int_code = 0;
		}
		break;

		/* connect IRQ6 input to /INT */
		case 0x03:
		{
			m_fdc_int_code = 1;
		}
		break;

		/* dis-connect IRQ6 input from /NMI and /INT */
		case 0x04:
		{
			m_fdc_int_code = 2;
		}
		break;

		/* set terminal count */
		case 0x05:
		{
			m_fdc->tc_w(true);
		}
		break;

		/* clear terminal count */
		case 0x06:
		{
			m_fdc->tc_w(false);
		}
		break;

		/* bleeper on */
		case 0x0b:
		{
			m_beeper->set_state(1);
		}
		break;

		/* bleeper off */
		case 0x0c:
		{
			m_beeper->set_state(0);
		}
		break;

		/* drive video outputs */
		case 0x07:
		{
		}
		break;

		/* float video outputs */
		case 0x08:
		{
		}
		break;

		/* set 4-bit output port to value X */
		case 0x0f:
		{
			/* bit 7 - ?? */
			/* bit 6 - ?? */
			/* bit 5 - green/red led (1==green)*/
			/* bit 4 - monitor on/off (1==on) */

			m_4_bit_port = data>>4;


		}
		break;
	}
}

WRITE_LINE_MEMBER( pcw16_state::fdc_interrupt )
{
	/* IRQ6 */
	/* bit 6 of PCW16 system status indicates floppy ints */
	if (state)
		m_system_status |= (1<<6);
	else
		m_system_status &= ~(1<<6);

	trigger_fdc_int();
}


WRITE_LINE_MEMBER(pcw16_state::pcw16_com_interrupt_1)
{
	m_system_status &= ~(1 << 4);

	if ( state ) {
		m_system_status |= (1 << 4);
	}

	pcw16_refresh_ints();
}


WRITE_LINE_MEMBER(pcw16_state::pcw16_com_interrupt_2)
{
	m_system_status &= ~(1 << 3);

	if ( state ) {
		m_system_status |= (1 << 3);
	}

	pcw16_refresh_ints();
}

FLOPPY_FORMATS_MEMBER( pcw16_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void pcw16_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}


void pcw16_state::pcw16_io(address_map &map)
{
	map.global_mask(0xff);
	/* super i/o chip */
	map(0x018, 0x01f).m(m_fdc, FUNC(pc_fdc_superio_device::map));
	map(0x020, 0x027).rw("ns16550_1", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x028, 0x02f).rw(m_uart2, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x038, 0x03a).rw("lpt", FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write));
	/* anne asic */
	map(0x0e0, 0x0ef).w(FUNC(pcw16_state::pcw16_palette_w));
	map(0x0f0, 0x0f3).rw(FUNC(pcw16_state::pcw16_bankhw_r), FUNC(pcw16_state::pcw16_bankhw_w));
	map(0x0f4, 0x0f4).rw(FUNC(pcw16_state::pcw16_keyboard_data_shift_r), FUNC(pcw16_state::pcw16_keyboard_data_shift_w));
	map(0x0f5, 0x0f5).rw(FUNC(pcw16_state::pcw16_keyboard_status_r), FUNC(pcw16_state::pcw16_keyboard_control_w));
	map(0x0f7, 0x0f7).rw(FUNC(pcw16_state::pcw16_timer_interrupt_counter_r), FUNC(pcw16_state::pcw16_video_control_w));
	map(0x0f8, 0x0f8).rw(FUNC(pcw16_state::pcw16_system_status_r), FUNC(pcw16_state::pcw16_system_control_w));
	map(0x0f9, 0x0f9).rw(FUNC(pcw16_state::rtc_256ths_seconds_r), FUNC(pcw16_state::rtc_control_w));
	map(0x0fa, 0x0fa).rw(FUNC(pcw16_state::rtc_seconds_r), FUNC(pcw16_state::rtc_seconds_w));
	map(0x0fb, 0x0fb).rw(FUNC(pcw16_state::rtc_minutes_r), FUNC(pcw16_state::rtc_minutes_w));
	map(0x0fc, 0x0fc).rw(FUNC(pcw16_state::rtc_hours_r), FUNC(pcw16_state::rtc_hours_w));
	map(0x0fd, 0x0fd).rw(FUNC(pcw16_state::rtc_days_r), FUNC(pcw16_state::rtc_days_w));
	map(0x0fe, 0x0fe).rw(FUNC(pcw16_state::rtc_month_r), FUNC(pcw16_state::rtc_month_w));
	map(0x0ff, 0x0ff).rw(FUNC(pcw16_state::rtc_year_invalid_r), FUNC(pcw16_state::rtc_year_w));
}


void pcw16_state::machine_reset()
{
	/* initialise defaults */
	m_fdc_int_code = 2;
	/* clear terminal count */
	m_fdc->tc_w(false);
	/* select first rom page */
	m_banks[0] = 0;
//  pcw16_update_memory(machine);

	/* temp rtc setup */
	m_rtc_seconds = 0;
	m_rtc_minutes = 0;
	m_rtc_hours = 0;
	m_rtc_days_max = 0;
	m_rtc_days = 1;
	m_rtc_months = 1;
	m_rtc_years = 0;
	m_rtc_control = 1;
	m_rtc_256ths_seconds = 0;

	pcw16_keyboard_init();
	m_uart2->ri_w(0);
}


void pcw16_state::machine_start()
{
	m_system_status = 0;
	m_interrupt_counter = 0;

	m_beeper->set_state(0);
}

static INPUT_PORTS_START(pcw16)
	PORT_START("EXTRA")
	/* vblank */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")
	/* power switch - default is on */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Power Switch/Suspend") PORT_WRITE_LINE_DEVICE_MEMBER("ns16550_2", ins8250_uart_device, ri_w) PORT_TOGGLE

	PORT_INCLUDE( at_keyboard )     /* IN4 - IN11 */
INPUT_PORTS_END

static void pcw16_com(device_slot_interface &device)
{
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
}

MACHINE_CONFIG_START(pcw16_state::pcw16)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, 16000000)
	MCFG_DEVICE_PROGRAM_MAP(pcw16_map)
	MCFG_DEVICE_IO_MAP(pcw16_io)
	config.m_minimum_quantum = attotime::from_hz(60);

	ns16550_device &uart1(NS16550(config, "ns16550_1", XTAL(1'843'200)));     /* TODO: Verify uart model */
	uart1.out_tx_callback().set("serport1", FUNC(rs232_port_device::write_txd));
	uart1.out_dtr_callback().set("serport1", FUNC(rs232_port_device::write_dtr));
	uart1.out_rts_callback().set("serport1", FUNC(rs232_port_device::write_rts));
	uart1.out_int_callback().set(FUNC(pcw16_state::pcw16_com_interrupt_1));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", pcw16_com, "msystems_mouse"));
	serport1.rxd_handler().set(uart1, FUNC(ins8250_uart_device::rx_w));
	serport1.dcd_handler().set(uart1, FUNC(ins8250_uart_device::dcd_w));
	serport1.dsr_handler().set(uart1, FUNC(ins8250_uart_device::dsr_w));
	serport1.ri_handler().set(uart1, FUNC(ins8250_uart_device::ri_w));
	serport1.cts_handler().set(uart1, FUNC(ins8250_uart_device::cts_w));

	NS16550(config, m_uart2, XTAL(1'843'200));     /* TODO: Verify uart model */
	m_uart2->out_tx_callback().set("serport2", FUNC(rs232_port_device::write_txd));
	m_uart2->out_dtr_callback().set("serport2", FUNC(rs232_port_device::write_dtr));
	m_uart2->out_rts_callback().set("serport2", FUNC(rs232_port_device::write_rts));
	m_uart2->out_int_callback().set(FUNC(pcw16_state::pcw16_com_interrupt_2));

	rs232_port_device &serport2(RS232_PORT(config, "serport2", pcw16_com, nullptr));
	serport2.rxd_handler().set(m_uart2, FUNC(ins8250_uart_device::rx_w));
	serport2.dcd_handler().set(m_uart2, FUNC(ins8250_uart_device::dcd_w));
	serport2.dsr_handler().set(m_uart2, FUNC(ins8250_uart_device::dsr_w));
	serport2.cts_handler().set(m_uart2, FUNC(ins8250_uart_device::cts_w));

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(PCW16_SCREEN_WIDTH, PCW16_SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, PCW16_SCREEN_WIDTH-1, 0, PCW16_SCREEN_HEIGHT-1)
	MCFG_SCREEN_UPDATE_DRIVER(pcw16_state, screen_update_pcw16)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", FUNC(pcw16_state::pcw16_colours), PCW16_NUM_COLOURS);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 3750).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* printer */
	pc_lpt_device &lpt(PC_LPT(config, "lpt"));
	lpt.irq_handler().set_inputline(m_maincpu, 0);

	PC_FDC_SUPERIO(config, m_fdc, 48_MHz_XTAL / 2);
	m_fdc->intrq_wr_callback().set(FUNC(pcw16_state::fdc_interrupt));
	FLOPPY_CONNECTOR(config, "fdc:0", pcw16_floppies, "35hd", pcw16_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pcw16_floppies, "35hd", pcw16_state::floppy_formats);

	SOFTWARE_LIST(config, "disk_list").set_original("pcw16");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("2M");

	INTEL_E28F008SA(config, "flash0");
	INTEL_E28F008SA(config, "flash1");

	AT_KEYB(config, m_keyboard, pc_keyboard_device::KEYBOARD_TYPE::AT, 3);
	m_keyboard->keypress().set(FUNC(pcw16_state::pcw16_keyboard_callback));

	/* video ints */
	TIMER(config, "video_timer").configure_periodic(FUNC(pcw16_state::pcw16_timer_callback), attotime::from_usec(5830));
	/* rtc timer */
	TIMER(config, "rtc_timer").configure_periodic(FUNC(pcw16_state::rtc_timer_callback), attotime::from_hz(256));
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/* the lower 64k of the flash-file memory is write protected. This contains the boot
    rom. The boot rom is also on the OS rescue disc. Handy! */
ROM_START(pcw16)
	ROM_REGION((0x010000+524288), "maincpu",0)
	ROM_LOAD("pcw045.sys",0x10000, 524288, CRC(c642f498) SHA1(8a5c05de92e7b2c5acdfb038217503ad363285b5))
ROM_END


/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY        FULLNAME */
COMP( 1995, pcw16, 0,      0,      pcw16,   pcw16, pcw16_state, empty_init, "Amstrad plc", "PcW16", MACHINE_NOT_WORKING )
