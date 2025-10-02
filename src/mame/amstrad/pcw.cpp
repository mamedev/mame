// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/******************************************************************************

    pcw.cpp
    system driver

    Kevin Thacker [MESS driver]

    Thankyou to Jacob Nevins, Richard Fairhurst and Cliff Lawson,
    for their documentation that I used for the development of this
    driver.

    PCW came in 4 forms.
    PCW8256, PCW8512, PCW9256, PCW9512

    These systems were nicknamed "Joyce", apparently after a secretary who worked at
    Amstrad plc.

    These machines were designed for wordprocessing and other business applications.

    The computer came with Locoscript (wordprocessor by Locomotive Software Ltd),
    and CP/M+ (3.1).

    The original PCW8256 system came with a keyboard, green-screen monitor
    (which had 2 3" 80 track, double sided disc drives mounted vertically in it),
    and a dedicated printer. The other systems had different design but shared the
    same hardware.

    Since it was primarily designed as a wordprocessor, there were not many games
    written.
    Some of the games available:
     - Head Over Heels
     - Infocom adventures (Hitchhikers Guide to the Galaxy etc)

    However, it can use the CP/M OS, there is a large variety of CPM software that will
    run on it!!!!!!!!!!!!!!

    Later systems had:
        - black/white monitor,
        - dedicated printer was removed, and support for any printer was added
        - 3" internal drive replaced by a 3.5" drive

    All the logic for the system, except the FDC was found in a Amstrad designed custom
    chip.

    In the original PCW8256, there was no boot-rom. A boot-program was stored in the printer
    chip, and this was activated when the PCW was first switched on. AFAIK there are no
    dumps of this program, so I have written my own replacement.

    The boot-program performs a simple task: Load sector 0, track 0 to &f000 in ram, and execute
    &f010.

    From here CP/M is booted, and the appropriate programs can be run.

    The hardware:
       - Z80 CPU running at 3.4 MHz
       - UPD765 FDC
       - mono display
       - beep (a fixed Hz tone which can be turned on/off)
       - 720x256 (PAL) bitmapped display, 720x200 (NTSC) bitmapped display
       - Amstrad CPC6128 style keyboard

  If there are special roms for any of the PCW series I would be interested in them
  so I can implement the driver properly.

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
  - Printer hardware emulation (8256 etc)
  - Parallel port emulation (9512, 9512+, 10)
  - emulation of serial hardware
  - emulation of other hardware...?
 ******************************************************************************/
#include "emu.h"
// pcw video hardware
#include "pcw.h"

#include "cpu/z80/z80.h"
#include "machine/i8243.h"
#include "machine/upd765.h"
// pcw/pcw16 beeper
#include "sound/beep.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "render.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "pcw.lh"

#define LOG_PRN      (1U << 1)
#define LOG_STROBE   (1U << 2)
#define LOG_PAR      (1U << 3)
#define LOG_EXP      (1U << 4)
#define LOG_MEM      (1U << 5)
#define LOG_SYS      (1U << 6)
#define LOG_BANK     (1U << 7)
#define LOG_RRAM     (1U << 8)
#define LOG_IRQ      (1U << 9)

//#define VERBOSE (LOG_SYS)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGPRN(...)     LOGMASKED(LOG_PRN,      __VA_ARGS__)
#define LOGSTROBE(...)  LOGMASKED(LOG_STROBE,   __VA_ARGS__)
#define LOGPAR(...)     LOGMASKED(LOG_PAR,      __VA_ARGS__)
#define LOGEXP(...)     LOGMASKED(LOG_EXP,      __VA_ARGS__)
#define LOGMEM(...)     LOGMASKED(LOG_MEM,      __VA_ARGS__)
#define LOGSYS(...)     LOGMASKED(LOG_SYS,      __VA_ARGS__)
#define LOGBANK(...)    LOGMASKED(LOG_BANK,     __VA_ARGS__)
#define LOGRRAM(...)    LOGMASKED(LOG_RRAM,     __VA_ARGS__)
#define LOGIRQ(...)     LOGMASKED(LOG_IRQ,      __VA_ARGS__)

static const uint8_t half_step_table[4] = { 0x01, 0x02, 0x04, 0x08 };
static const uint8_t full_step_table[4] = { 0x03, 0x06, 0x0c, 0x09 };

void pcw_state::pcw_update_interrupt_counter()
{
	/* never increments past 15! */
	if (m_interrupt_counter==0x0f)
		return;

	/* increment count */
	m_interrupt_counter++;
}


// set/reset INT and NMI lines
void pcw_state::pcw_update_irqs()
{
	// set NMI line, remains set until FDC interrupt type is changed
	if(m_nmi_flag != 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	// set IRQ line, timer pulses IRQ line, all other devices hold it as necessary
	if(m_fdc_interrupt_code == 1 && (m_system_status & 0x20))
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
		return;
	}

	if(m_timer_irq_flag != 0)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
		return;
	}

	m_maincpu->set_input_line(0, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(pcw_state::pcw_timer_pulse)
{
	m_timer_irq_flag = 0;
	pcw_update_irqs();
}

/* callback for 1/300ths of a second interrupt */
TIMER_DEVICE_CALLBACK_MEMBER(pcw_state::pcw_timer_interrupt)
{
	pcw_update_interrupt_counter();

	m_timer_irq_flag = 1;
	pcw_update_irqs();
	m_pulse_timer->adjust(attotime::from_usec(100), 0, attotime::from_usec(100));
}

/* PCW uses UPD765 in NON-DMA mode. FDC Ints are connected to /INT or
 * /NMI depending on choice (see system control below)
 * fdc interrupt callback. set/clear fdc int */
void pcw_state::pcw_fdc_interrupt(int state)
{
	if (!state)
		m_system_status &= ~(1<<5);
	else
	{
		m_system_status |= (1<<5);
		if(m_fdc_interrupt_code == 0)  // NMI is held until interrupt type is changed
			m_nmi_flag = 1;
	}
}


/* Memory is banked in 16k blocks.

    The upper 16 bytes of block 3, contains the keyboard
    state. This is updated by the hardware.

    block 3 could be paged into any bank, and this explains the
    setup of the memory below.
*/
void pcw_state::pcw_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr(m_rdbanks[0]).bankw(m_wrbanks[0]);
	map(0x4000, 0x7fff).bankr(m_rdbanks[1]).bankw(m_wrbanks[1]);
	map(0x8000, 0xbfff).bankr(m_rdbanks[2]).bankw(m_wrbanks[2]);
	map(0xc000, 0xffff).bankr(m_rdbanks[3]).bankw(m_wrbanks[3]);
}


/* Keyboard is read by the MCU and sent as serial data to the gate array ASIC */
uint8_t pcw_state::pcw_keyboard_r(offs_t offset)
{
	return m_iptlines[offset]->read();
}

uint8_t pcw_state::pcw_keyboard_data_r(offs_t offset)
{
	return m_mcu_keyboard_data[offset];
}

/* -----------------------------------------------------------------------
 * PCW Banking
 * ----------------------------------------------------------------------- */

void pcw_state::pcw_update_read_memory_block(int block, int bank)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* bank 3? */
	if (bank == 3)
	{
		/* when upper 16 bytes are accessed use keyboard read handler */
		space.install_read_handler( block * 0x04000 + 0x3ff0, block * 0x04000 + 0x3fff, read8sm_delegate(*this, FUNC(pcw_state::pcw_keyboard_data_r)));
		LOGMEM("MEM: read block %i -> bank %i\n", block, bank);
	}
	else
	{
		/* restore bank handler across entire block */
		space.install_read_bank(block * 0x04000 + 0x0000, block * 0x04000 + 0x3fff, m_rdbanks[block].target());
		LOGMEM("MEM: read block %i -> bank %i\n", block, bank);
	}
	m_rdbanks[block]->set_base(m_ram->pointer() + ((bank * 0x4000) % m_ram->size()));
}

void pcw_state::pcw_update_write_memory_block(int block, int bank)
{
	m_wrbanks[block]->set_base(m_ram->pointer() + ((bank * 0x4000) % m_ram->size()));
	LOGMEM("MEM: write block %i -> bank %i\n", block, bank);
}


/* ----------------------------------------------------------------------- */

/* &F4 O  b7-b4: when set, force memory reads to access the same bank as
writes for &C000, &0000, &8000, and &4000 respectively */

void pcw_state::pcw_update_mem(int block, int data)
{
	/* expansion ram select.
	    if block is 0-7, selects internal ram instead for read/write
	    */
	if (data & 0x080)
	{
		int bank;

		/* same bank for reading and writing */
		bank = data & 0x7f;

		pcw_update_read_memory_block(block, bank);
		pcw_update_write_memory_block(block, bank);
	}
	else
	{
		/* specify a different bank for reading and writing */
		int write_bank;
		int read_bank;
		int mask=0;

		switch (block)
		{
			case 0:
			{
				mask = (1<<6);
			}
			break;

			case 1:
			{
				mask = (1<<4);
			}
			break;

			case 2:
			{
				mask = (1<<5);
			}
			break;

			case 3:
			{
				mask = (1<<7);
			}
			break;
		}

		if (m_bank_force & mask)
		{
			read_bank = data & 0x07;
		}
		else
		{
			read_bank = (data>>4) & 0x07;
		}

		pcw_update_read_memory_block(block, read_bank);

		write_bank = data & 0x07;
		pcw_update_write_memory_block(block, write_bank);
	}

	/* if boot is active, page in fake ROM */
/*  if ((m_boot) && (block==0))
    {
        unsigned char *FakeROM;

        FakeROM = &memregion("maincpu")->base()[0x010000];

        m_rdbanks[0]->set_base(FakeROM);
    }*/
}

/* from Jacob Nevins docs */
int pcw_state::pcw_get_sys_status()
{
	return
		m_interrupt_counter
		| (m_screen->vblank() ? 0x40 : 0x00)
		| (ioport("EXTRA")->read() & 0x010)
		| (m_system_status & 0x20);
}

uint8_t pcw_state::pcw_interrupt_counter_r()
{
	int data;

	/* from Jacob Nevins docs */

	/* get data */
	data = pcw_get_sys_status();
	/* clear int counter */
	m_interrupt_counter = 0;
	/* check interrupts */
	pcw_update_irqs();
	/* return data */
	LOGIRQ("IRQ counter read, returning %02x\n", data);
	return data;
}


void pcw_state::pcw_bank_select_w(offs_t offset, uint8_t data)
{
	LOGBANK("BANK: %2x %x\n", offset, data);
	m_banks[offset] = data;

	pcw_update_mem(offset, data);
	LOGBANK("RAM Banks: %02x %02x %02x %02x Lock:%02x",m_banks[0],m_banks[1],m_banks[2],m_banks[3],m_bank_force);
}

void pcw_state::pcw_bank_force_selection_w(uint8_t data)
{
	m_bank_force = data;

	pcw_update_mem(0, m_banks[0]);
	pcw_update_mem(1, m_banks[1]);
	pcw_update_mem(2, m_banks[2]);
	pcw_update_mem(3, m_banks[3]);
}


void pcw_state::pcw_roller_ram_addr_w(uint8_t data)
{
	/*
	Address of roller RAM. b7-5: bank (0-7). b4-1: address / 512. */

	m_roller_ram_addr = (((data>>5) & 0x07)<<14) |
							((data & 0x01f)<<9);
	LOGRRAM("Roller-RAM: Address set to 0x%05x\n", m_roller_ram_addr);
}

void pcw_state::pcw_pointer_table_top_scan_w(uint8_t data)
{
	m_roller_ram_offset = data;
	LOGRRAM("Roller-RAM: offset set to 0x%05x\n", m_roller_ram_offset);
}

void pcw_state::pcw_vdu_video_control_register_w(uint8_t data)
{
	m_vdu_video_control_register = data;
	LOGRRAM("Roller-RAM: control reg set to 0x%02x\n", data);
}

void pcw_state::pcw_system_control_w(uint8_t data)
{
	LOGSYS("SYSTEM CONTROL: %d\n", data);

	switch (data)
	{
		/* end bootstrap */
		case 0:
		{
			m_boot = 0;
			pcw_update_mem(0, m_banks[0]);
		}
		break;

		/* reboot */
		case 1:
		{
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			LOGSYS("SYS: Reboot");
		}
		break;

		/* connect fdc interrupt to nmi */
		case 2:
		{
			int fdc_previous_interrupt_code = m_fdc_interrupt_code;

			m_fdc_interrupt_code = 0;

			/* previously connected to INT? */
			if (fdc_previous_interrupt_code == 1)
			{
				/* yes */

				pcw_update_irqs();
			}

		}
		break;


		/* connect fdc interrupt to interrupt */
		case 3:
		{
			int fdc_previous_interrupt_code = m_fdc_interrupt_code;

			/* connect to INT */
			m_fdc_interrupt_code = 1;

			/* previously connected to NMI? */
			if (fdc_previous_interrupt_code == 0)
			{
				/* yes */

				/* clear nmi interrupt */
				m_nmi_flag = 0;
			}

			/* re-issue interrupt */
			pcw_update_irqs();
		}
		break;


		/* connect fdc interrupt to neither */
		case 4:
		{
			int fdc_previous_interrupt_code = m_fdc_interrupt_code;

			m_fdc_interrupt_code = 2;

			/* previously connected to NMI or INT? */
			if ((fdc_previous_interrupt_code == 0) || (fdc_previous_interrupt_code == 1))
			{
				/* yes */

				/* Clear NMI */
				m_nmi_flag = 0;
			}
			pcw_update_irqs();

		}
		break;

		/* set fdc terminal count */
		case 5:
		{
			m_fdc->tc_w(true);
		}
		break;

		/* clear fdc terminal count */
		case 6:
		{
			m_fdc->tc_w(false);
		}
		break;

		/* screen on */
		case 7:
		{
		}
		break;

		/* screen off */
		case 8:
		{
		}
		break;

		/* disc motor on */
		case 9:
		{
			floppy_image_device *floppy;
			floppy = m_floppy[0]->get_device();
			if(floppy)
				floppy->mon_w(0);
			floppy = m_floppy[1]->get_device();
			if(floppy)
				floppy->mon_w(0);
		}
		break;

		/* disc motor off */
		case 10:
		{
			floppy_image_device *floppy;
			floppy = m_floppy[0]->get_device();
			if(floppy)
				floppy->mon_w(1);
			floppy = m_floppy[1]->get_device();
			if(floppy)
				floppy->mon_w(1);
		}
		break;

		/* beep on */
		case 11:
		{
			m_beeper->set_state(1);
		}
		break;

		/* beep off */
		case 12:
		{
			m_beeper->set_state(0);
		}
		break;

	}
}

uint8_t pcw_state::pcw_system_status_r()
{
	/* from Jacob Nevins docs */
	uint8_t ret = pcw_get_sys_status();

	return ret;
}

/* read from expansion hardware - additional hardware not part of
the PCW custom ASIC */
uint8_t pcw_state::pcw_expansion_r(offs_t offset)
{
	LOGEXP("pcw expansion r: %04x\n", offset+0x080);

	switch (offset+0x080)
	{
		case 0x0e0:
		{
			/* spectravideo joystick */
			if (ioport("EXTRA")->read() & 0x020)
			{
				return ioport("SPECTRAVIDEO")->read();
			}
			else
			{
				return 0x0ff;
			}
		}

		case 0x09f:
		{
			/* kempston joystick */
			return ioport("KEMPSTON")->read();
		}

		case 0x0e1:
		case 0x0e3:
		{
			return 0x7f;
		}

		case 0x085:
		{
			return 0x0fe;
		}

		case 0x087:
		{
			return 0x0ff;
		}

	}

	/* result from floating bus/no peripheral at this port */
	return 0x0ff;
}

/* write to expansion hardware - additional hardware not part of
the PCW custom ASIC */
void pcw_state::pcw_expansion_w(offs_t offset, uint8_t data)
{
	LOGEXP("pcw expansion w: %04x %02x\n", offset+0x080, data);
}

void pcw_state::pcw_printer_fire_pins(uint16_t pins)
{
	int x,line;
	int32_t feed = (m_paper_feed / 2);

	for(x=feed+PCW_PRINTER_HEIGHT-16;x<feed+PCW_PRINTER_HEIGHT-7;x++)
	{
		line = x % PCW_PRINTER_HEIGHT;
		if((pins & 0x01) == 0)
			m_prn_output->pix(line, m_printer_headpos) = (uint16_t)(pins & 0x01);
		pins >>= 1;
	}
//  if(m_printer_headpos < PCW_PRINTER_WIDTH)
//      m_printer_headpos++;
}

// print error type
// should return 0xF8 if there are no errors
// 0 = underrun
// 1 = printer controller RAM fault
// 3 = bad command
// 5 = print error
// anything else = no printer

// printer status
// bit 7 - bail bar in
// bit 6 - not currently executing a command
// bit 5 - printer RAM is full
// bit 4 - print head is not at left margin
// bit 3 - sheet feeder is present
// bit 2 - paper is present
// bit 1 - busy
// bit 0 - controller fault

/* MCU handlers */
/* I/O ports: (likely to be completely wrong...)
 * (write)
 * all are active low
 * P1: pins 0-7
 * P2 bit 0: pin 8
 * P2 bit 1: serial shift/store clock
 * P2 bit 2: serial shift store data
 * P2 bit 3: serial shift/store strobe
 * P2 bit 4: print head motor
 * P2 bit 5: paper feed motor
 * P2 bit 6: fire pins
 * (read)
 * P2 bit 7: bail bar status (0 if out)
 * T0: Paper sensor (?)
 * T1: Print head location (1 if not at left margin)
 */
TIMER_CALLBACK_MEMBER(pcw_state::pcw_stepper_callback)
{
	LOGPRN("PRN: P2 bits %s %s %s\nSerial: %02x\nHeadpos: %i",
		   m_printer_p2 & 0x40 ? " " : "6",
		   m_printer_p2 & 0x20 ? " " : "5",
		   m_printer_p2 & 0x10 ? " " : "4",
		   m_printer_shift_output,m_printer_headpos);

	if((m_printer_p2 & 0x10) == 0)  // print head motor active
	{
		uint8_t stepper_state = (m_printer_shift_output >> 4) & 0x0f;
		if(stepper_state == full_step_table[(m_head_motor_state + 1) & 0x03])
		{
			m_printer_headpos += 2;
			m_head_motor_state++;
			LOGPRN("Printer head moved forward by 2 to %i\n", m_printer_headpos);
		}
		if(stepper_state == half_step_table[(m_head_motor_state + 1) & 0x03])
		{
			m_printer_headpos += 1;
			m_head_motor_state++;
			LOGPRN("Printer head moved forward by 1 to %i\n", m_printer_headpos);
		}
		if(stepper_state == full_step_table[(m_head_motor_state - 1) & 0x03])
		{
			m_printer_headpos -= 2;
			m_head_motor_state--;
			LOGPRN("Printer head moved back by 2 to %i\n", m_printer_headpos);
		}
		if(stepper_state == half_step_table[(m_head_motor_state - 1) & 0x03])
		{
			m_printer_headpos -= 1;
			m_head_motor_state--;
			LOGPRN("Printer head moved back by 1 to %i\n", m_printer_headpos);
		}
		if(m_printer_headpos < 0)
			m_printer_headpos = 0;
		if(m_printer_headpos > PCW_PRINTER_WIDTH)
			m_printer_headpos = PCW_PRINTER_WIDTH;
		m_head_motor_state &= 0x03;
		m_printer_p2 |= 0x10;
	}
	if((m_printer_p2 & 0x20) == 0)  // line feed motor active
	{
		uint8_t stepper_state = m_printer_shift_output & 0x0f;
		if(stepper_state == full_step_table[(m_linefeed_motor_state + 1) & 0x03])
		{
			m_paper_feed++;
			if(m_paper_feed > PCW_PRINTER_HEIGHT*2)
				m_paper_feed = 0;
			m_linefeed_motor_state++;
		}
		if(stepper_state == half_step_table[(m_linefeed_motor_state + 1) & 0x03])
		{
			m_paper_feed++;
			if(m_paper_feed > PCW_PRINTER_HEIGHT*2)
				m_paper_feed = 0;
			m_linefeed_motor_state++;
		}
		m_linefeed_motor_state &= 0x03;
		m_printer_p2 |= 0x20;
	}
}

TIMER_CALLBACK_MEMBER(pcw_state::pcw_pins_callback)
{
	pcw_printer_fire_pins(m_printer_pins);
	m_printer_p2 |= 0x40;
}

uint8_t pcw_state::mcu_printer_p1_r()
{
	LOGPRN("PRN: MCU reading data from P1\n");
	return m_printer_pins & 0x00ff;
}

void pcw_state::mcu_printer_p1_w(uint8_t data)
{
	m_printer_pins = (m_printer_pins & 0x0100) | data;
	LOGPRN("PRN: Print head position = %i", m_printer_headpos);
	LOGPRN("PRN: MCU writing %02x to P1 [%03x/%03x]\n", data,m_printer_pins, ~m_printer_pins & 0x1ff);
}

uint8_t pcw_state::mcu_printer_p2_r()
{
	uint8_t ret = 0x00;
	LOGPRN("PRN: MCU reading data from P2\n");
	ret |= 0x80;  // make sure bail bar is in
	ret |= (m_printer_p2 & 0x70);
	ret |= (m_printer_pins & 0x100) ? 0x01 : 0x00;  // ninth pin
	ret |= 0x0e;
	return ret;
}

void pcw_state::mcu_printer_p2_w(uint8_t data)
{
	LOGPRN("PRN: MCU writing %02x to P2\n", data);
	m_printer_p2 = data & 0x70;

	// handle shift/store
	m_printer_serial = data & 0x04;  // data
	if (!BIT(m_printer_p2_prev, 1) && BIT(data, 1))  // only update when clock goes positive
	{
		m_printer_shift <<= 1;
		if(m_printer_serial == 0)
			m_printer_shift &= ~0x01;
		else
			m_printer_shift |= 0x01;
	}
	if((data & 0x08) != 0)  // strobe
	{
		LOGSTROBE("Strobe active [%02x]\n",m_printer_shift);
		m_printer_shift_output = m_printer_shift;
		m_prn_stepper->adjust(PERIOD_OF_555_MONOSTABLE(22000,0.00000001));
	}

	if(data & 0x40)
		m_prn_pins->adjust(PERIOD_OF_555_MONOSTABLE(22000,0.0000000068));

	if(data & 0x01)
		m_printer_pins |= 0x0100;
	else
		m_printer_pins &= ~0x0100;
	m_printer_p2_prev = data;
}

// Paper sensor
int pcw_state::mcu_printer_t1_r()
{
	return 1;
}

// Print head location (0 if at left margin, otherwise 1)
int pcw_state::mcu_printer_t0_r()
{
	if(m_printer_headpos == 0)
		return 0;
	else
		return 1;
}

/*
 *  Keyboard MCU (i8048)
 *  P1 = keyboard scan row select (low 8 bits)
 *  P2 = bits 7-4: keyboard scan row select (high 4 bits)
 *       bit 1: keyboard serial clock
 *       bit 0: keyboard serial data
 */
void pcw_state::mcu_transmit_serial(uint8_t bit)
{
	int seq;

	/* Keyboard data is sent in serial from the MCU through the keyboard port, to the ASIC
	   Sends a string of 12-bit sequences, first 4 bits are the RAM location (from &3ff0),
	   then 8 bits for the data to be written there. */
	seq = m_mcu_transmit_count % 12;
	if(seq < 4)
	{
		if(bit == 0)
			m_mcu_selected &= ~(8 >> seq);
		else
			m_mcu_selected |= (8 >> seq);
	}
	else
	{
		seq -= 4;
		if(bit == 0)
			m_mcu_buffer &= ~(128 >> seq);
		else
			m_mcu_buffer |= (128 >> seq);
	}
	m_mcu_transmit_count++;
	if(m_mcu_transmit_count >= 12)
	{
		m_mcu_keyboard_data[m_mcu_selected] = m_mcu_buffer;
		m_mcu_transmit_count = 0;
	}
}

uint8_t pcw_state::mcu_kb_scan_r()
{
	return m_kb_scan_row & 0xff;
}

void pcw_state::mcu_kb_scan_w(uint8_t data)
{
	m_kb_scan_row = (m_kb_scan_row & 0xff00) | data;
}

uint8_t pcw_state::mcu_kb_scan_high_r()
{
	return (m_kb_scan_row & 0xff00) >> 8;
}

void pcw_state::mcu_kb_scan_high_w(uint8_t data)
{
	if((m_mcu_prev & 0x02) && !(data & 0x02))  // bit is transmitted on high-to-low clock transition
	{
		mcu_transmit_serial(data & 0x01);
		m_mcu_transmit_reset_seq = 0;
	}

	if((m_mcu_prev & 0x01) != (data & 0x01))  // two high->low transitions on the data pin signals the beginning of a new transfer
	{
		m_mcu_transmit_reset_seq++;
		if(m_mcu_transmit_reset_seq > 3)
			m_mcu_transmit_count = 0;
	}

	m_kb_scan_row = (m_kb_scan_row & 0x00ff) | ((data & 0xff) << 8);
	m_mcu_prev = data;
}

uint8_t pcw_state::mcu_kb_data_r()
{
	uint16_t scan_bits = ((m_kb_scan_row & 0xf000) >> 4) | (m_kb_scan_row & 0xff);
	int x;

	for(x=0;x<12;x++)
	{
		if(!(scan_bits & 1))
			return pcw_keyboard_r(x);
		else
			scan_bits >>= 1;
	}
	return 0xff;
}

int pcw_state::mcu_kb_t1_r()
{
	return 1;
}

int pcw_state::mcu_kb_t0_r()
{
	return 0;
}

/* TODO: Implement parallel port! */
uint8_t pcw_state::pcw9512_parallel_r(offs_t offset)
{
	if (offset==1)
	{
		return 0xff^0x020;
	}

	LOGPAR("pcw9512 parallel r: offs: %04x\n", (int) offset);
	return 0x00;
}

/* TODO: Implement parallel port! */
void pcw_state::pcw9512_parallel_w(offs_t offset, uint8_t data)
{
	LOGPAR("pcw9512 parallel w: offs: %04x data: %02x\n", offset, data);
}

void pcw_state::pcw_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x000, 0x001).mirror(0x7e).m(m_fdc, FUNC(upd765a_device::map));
	map(0x080, 0x0ef).rw(FUNC(pcw_state::pcw_expansion_r), FUNC(pcw_state::pcw_expansion_w));
	map(0x0f0, 0x0f3).w(FUNC(pcw_state::pcw_bank_select_w));
	map(0x0f4, 0x0f4).rw(FUNC(pcw_state::pcw_interrupt_counter_r), FUNC(pcw_state::pcw_bank_force_selection_w));
	map(0x0f5, 0x0f5).w(FUNC(pcw_state::pcw_roller_ram_addr_w));
	map(0x0f6, 0x0f6).w(FUNC(pcw_state::pcw_pointer_table_top_scan_w));
	map(0x0f7, 0x0f7).w(FUNC(pcw_state::pcw_vdu_video_control_register_w));
	map(0x0f8, 0x0f8).rw(FUNC(pcw_state::pcw_system_status_r), FUNC(pcw_state::pcw_system_control_w));
	map(0x0fc, 0x0fd).rw(m_printer_mcu, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
}



void pcw_state::pcw9512_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x000, 0x001).mirror(0x7e).m(m_fdc, FUNC(upd765a_device::map));
	map(0x080, 0x0ef).rw(FUNC(pcw_state::pcw_expansion_r), FUNC(pcw_state::pcw_expansion_w));
	map(0x0f0, 0x0f3).w(FUNC(pcw_state::pcw_bank_select_w));
	map(0x0f4, 0x0f4).rw(FUNC(pcw_state::pcw_interrupt_counter_r), FUNC(pcw_state::pcw_bank_force_selection_w));
	map(0x0f5, 0x0f5).w(FUNC(pcw_state::pcw_roller_ram_addr_w));
	map(0x0f6, 0x0f6).w(FUNC(pcw_state::pcw_pointer_table_top_scan_w));
	map(0x0f7, 0x0f7).w(FUNC(pcw_state::pcw_vdu_video_control_register_w));
	map(0x0f8, 0x0f8).rw(FUNC(pcw_state::pcw_system_status_r), FUNC(pcw_state::pcw_system_control_w));
	map(0x0fc, 0x0fd).rw(FUNC(pcw_state::pcw9512_parallel_r), FUNC(pcw_state::pcw9512_parallel_w));
}


TIMER_CALLBACK_MEMBER(pcw_state::setup_beep)
{
	m_beeper->set_state(0);
}


void pcw_state::machine_start()
{
	m_fdc_interrupt_code = 2;
	m_vdu_video_control_register = 0;
	m_nmi_flag = 0;
}

void pcw_state::machine_reset()
{
	uint8_t* code = memregion("printer_mcu")->base();
	int x;
	/* ram paging is actually undefined at power-on */

	m_bank_force = 0xf0;  // banks are locked for CPC-style paging - Batman expects this

	m_banks[0] = 0x80;
	m_banks[1] = 0x81;
	m_banks[2] = 0x82;
	m_banks[3] = 0x83;

	pcw_update_mem(0, m_banks[0]);
	pcw_update_mem(1, m_banks[1]);
	pcw_update_mem(2, m_banks[2]);
	pcw_update_mem(3, m_banks[3]);

	m_boot = 0;   // System starts up in bootstrap mode, disabled until it's possible to emulate it.

	/* copy boot code into RAM - yes, it's skipping a step */
	memset(m_ram->pointer(),0x00,m_ram->size());
	for(x=0;x<256;x++)
		m_ram->pointer()[x+2] = code[x+0x300];

	/* and hack our way past the MCU side of the boot process */
	code[0x01] = 0x40;

	m_printer_headpos = 0x00; // bring printer head to left margin
	m_printer_shift = 0;
	m_printer_shift_output = 0;
	machine().render().first_target()->set_view(0);
}

void pcw_state::init_pcw()
{
	m_maincpu->set_input_line_vector(0, 0x0ff); // Z80

	/* lower 4 bits are interrupt counter */
	m_system_status = 0x000;
	m_system_status &= ~((1<<6) | (1<<5) | (1<<4));

	m_interrupt_counter = 0;

	m_roller_ram_offset = 0;

	/* timer interrupt */
	m_beep_setup_timer = timer_alloc(FUNC(pcw_state::setup_beep), this);
	m_beep_setup_timer->adjust(attotime::zero);

	m_prn_stepper = timer_alloc(FUNC(pcw_state::pcw_stepper_callback), this);
	m_prn_pins = timer_alloc(FUNC(pcw_state::pcw_pins_callback), this);
	m_pulse_timer = timer_alloc(FUNC(pcw_state::pcw_timer_pulse), this);
}


/*
b7:   k2     k1     [+]    .      ,      space  V      X      Z      del<   alt
b6:   k3     k5     1/2    /      M      N      B      C      lock          k.
b5:   k6     k4     shift  ;      K      J      F      D      A             enter
b4:   k9     k8     k7     ??      L      H      G      S      tab           f8
b3:   paste  copy   #      P      I      Y      T      W      Q             [-]
b2:   f2     cut    return [      O      U      R      E      stop          can
b1:   k0     ptr    ]      -      9      7      5      3      2             extra
b0:   f4     exit   del>   =      0      8      6      4      1             f6
      &3FF0  &3FF1  &3FF2  &3FF3  &3FF4  &3FF5  &3FF6  &3FF7  &3FF8  &3FF9  &3FFA

2008-05 FP:
Small note about Natural keyboard: currently,
- "Paste" is mapped to 'F9'
- "Exit" is mapped to 'F10'
- "Ptr" is mapped to 'Print Screen'
- "Cut" is mapped to 'F11'
- "Copy" is mapped to 'F12'
- "Stop" is mapped to 'Esc'
- [+] is mapped to 'Page Up'
- [-] is mapped to 'Page Down'
- "Extra" is mapped to 'Home'
- "Can" is mapped to 'Insert'
*/

static INPUT_PORTS_START(pcw)
	/* keyboard "ports". These are poked automatically into the PCW address space */

	PORT_START("LINE0")     /* 0x03ff0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F4") PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F2") PORT_CODE(KEYCODE_F1)       PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Paste") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))

	PORT_START("LINE1")     /* 0x03ff1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Exit") PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ptr") PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cut") PORT_CODE(KEYCODE_SLASH_PAD)   PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Copy") PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("LINE2")     /* 0x03ff2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL>") PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)                   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)                        PORT_CHAR('#') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)                        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)                          PORT_CHAR(189) PORT_CHAR('@')   // (½ @) between slash and rshift
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[+]") PORT_CODE(KEYCODE_F2)          PORT_CHAR(UCHAR_MAMEKEY(PGUP))  // 1st key on the left from 'Spacebar'

	PORT_START("LINE3")     /* 0x03ff3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)                       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)                        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)                    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                            PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)                        PORT_CHAR(0xA7) PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)                        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)                     PORT_CHAR('.')

	PORT_START("LINE4")     /* 0x03ff4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)                            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)                            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                            PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                            PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                            PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                            PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                        PORT_CHAR(',')

	PORT_START("LINE5")     /* 0x03ff5 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)                            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)                            PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                            PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                            PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                            PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                            PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                            PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                        PORT_CHAR(' ')

	PORT_START("LINE6")     /* 0x03ff6 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)                            PORT_CHAR('6') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)                            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                            PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                            PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                            PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                            PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                            PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                            PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("LINE7")     /* 0x03ff7 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)                            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)                            PORT_CHAR('3') PORT_CHAR(0xA3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                            PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                            PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                            PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                            PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                            PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                            PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("LINE8")     /* 0x03ff8 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)                            PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)                            PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                            PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)                      PORT_CHAR('\t')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                            PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                            PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("LINE9")     /* 0x03ff9 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5  F6") PORT_CODE(KEYCODE_F5)       PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Extra") PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Can") PORT_CODE(KEYCODE_PGUP)        PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[-]") PORT_CODE(KEYCODE_F4)          PORT_CHAR(UCHAR_MAMEKEY(PGDN))  // 1st key on the right from 'Spacebar'
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7  F8") PORT_CODE(KEYCODE_F7)       PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)                    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)                  PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL<") PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)

	PORT_START("LINE10")    /* 0x03ffa */
	PORT_BIT(0x07f,0x7f, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT)        PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CHAR(UCHAR_MAMEKEY(RALT))

	/* at this point the following reflect the above key combinations but in a incomplete
	way. No details available at this time */
	PORT_START("LINE11")    /* 0x03ffb */
	PORT_BIT(0xff, 0xff,     IPT_UNUSED)

	PORT_START("LINE12")    /* 0x03ffc */
	PORT_BIT(0xff, 0xff,     IPT_UNUSED)

	/* 2008-05  FP: not sure if this key is correct, "Caps Lock" is already mapped above.
	For now, I let it with no default mapping. */
	PORT_START("LINE13")    /* 0x03ffd */
	PORT_BIT(0xff, 0xff, IPT_UNUSED)
//  PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT LOCK") //PORT_CODE(KEYCODE_CAPSLOCK)
//  PORT_BIT(0x80, 0xff, IPT_UNUSED)

	PORT_START("LINE14")    /* 0x03ffe */
	PORT_BIT ( 0xff, 0xff,   IPT_UNUSED )

	PORT_START("LINE15")    /* 0x03fff */
	PORT_BIT ( 0xff, 0xff,   IPT_UNUSED )

	/* from here on are the pretend dipswitches for machine config etc */
	PORT_START("EXTRA")
	/* frame rate option */
	PORT_DIPNAME( 0x10, 0x010, "50/60Hz Frame Rate Option")
	PORT_DIPSETTING(    0x00, "60Hz")
	PORT_DIPSETTING(    0x10, "50Hz" )
	/* spectravideo joystick enabled */
	PORT_DIPNAME( 0x20, 0x020, "Spectravideo Joystick Enabled")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x20, DEF_STR(Yes) )

	/* Spectravideo joystick */
	/* bit 7: 0
	6: 1
	5: 1
	4: 1 if in E position
	3: 1 if N
	2: 1 if W
	1: 1 if fire pressed
	0: 1 if S
	*/

	PORT_START("SPECTRAVIDEO")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x020, 0x020, IPT_UNUSED)
	PORT_BIT(0x040, 0x040, IPT_UNUSED)
	PORT_BIT(0x080, 0x00, IPT_UNUSED)

	/* Kempston joystick */
	PORT_START("KEMPSTON")
	PORT_BIT( 0xff, 0x00,    IPT_UNUSED)
INPUT_PORTS_END

static void pcw_ssfloppies(device_slot_interface &device)
{
	device.option_add("3ssdd", FLOPPY_3_SSDD);
}

static void pcw_dsfloppies(device_slot_interface &device)
{
	device.option_add("3dsqd", FLOPPY_3_DSQD);
}

static void pcw_35floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

/* PCW8256, PCW8512, PCW9256 */
void pcw_state::pcw(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(32'000'000) / 8); /* clock supplied to chip, but in reality it is 3.4 MHz due to z80 wait cycles*/
	m_maincpu->set_addrmap(AS_PROGRAM, &pcw_state::pcw_map);
	m_maincpu->set_addrmap(AS_IO, &pcw_state::pcw_io);

	I8041AH(config, m_printer_mcu, 11000000);  // 11MHz
	m_printer_mcu->p2_in_cb().set(FUNC(pcw_state::mcu_printer_p2_r));
	m_printer_mcu->p2_out_cb().set(FUNC(pcw_state::mcu_printer_p2_w));
	m_printer_mcu->p1_in_cb().set(FUNC(pcw_state::mcu_printer_p1_r));
	m_printer_mcu->p1_out_cb().set(FUNC(pcw_state::mcu_printer_p1_w));
	m_printer_mcu->t1_in_cb().set(FUNC(pcw_state::mcu_printer_t1_r));
	m_printer_mcu->t0_in_cb().set(FUNC(pcw_state::mcu_printer_t0_r));

	I8048(config, m_keyboard_mcu, 5000000); // 5MHz
	m_keyboard_mcu->p1_in_cb().set(FUNC(pcw_state::mcu_kb_scan_r));
	m_keyboard_mcu->p1_out_cb().set(FUNC(pcw_state::mcu_kb_scan_w));
	m_keyboard_mcu->p2_in_cb().set(FUNC(pcw_state::mcu_kb_scan_high_r));
	m_keyboard_mcu->p2_out_cb().set(FUNC(pcw_state::mcu_kb_scan_high_w));
	m_keyboard_mcu->t1_in_cb().set(FUNC(pcw_state::mcu_kb_t1_r));
	m_keyboard_mcu->t0_in_cb().set(FUNC(pcw_state::mcu_kb_t0_r));
	m_keyboard_mcu->bus_in_cb().set(FUNC(pcw_state::mcu_kb_data_r));

//  config.set_maximum_quantum(attotime::from_hz(50));
	config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(32_MHz_XTAL / 3, 720 + 20, 8, 720 + 8, 256 + 32, 8, 256 + 8); // Hand tuned to get 50Hz, it is all in the Amstrad ASIC, 32MHz in and video out
	m_screen->set_screen_update(FUNC(pcw_state::screen_update_pcw));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(pcw_state::set_9xxx_palette), PCW_NUM_COLOURS);
	PALETTE(config, m_ppalette, FUNC(pcw_state::set_printer_palette), PCW_NUM_COLOURS);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 3750).add_route(ALL_OUTPUTS, "mono", 1.00);

	UPD765A(config, m_fdc, 4'000'000, true, true);
	m_fdc->intrq_wr_callback().set(FUNC(pcw_state::pcw_fdc_interrupt));

	SOFTWARE_LIST(config, "disk_list").set_original("pcw");

	/* internal ram */
	RAM(config, m_ram).set_default_size("256K");

	TIMER(config, "pcw_timer", 0).configure_periodic(FUNC(pcw_state::pcw_timer_interrupt), attotime::from_hz(300));
}

void pcw_state::pcw8256(machine_config &config)
{
	pcw(config);
	m_palette->set_init(FUNC(pcw_state::set_8xxx_palette));

	FLOPPY_CONNECTOR(config, "upd765:0", pcw_ssfloppies, "3ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pcw_dsfloppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	screen_device &printer(SCREEN(config, "printer", SCREEN_TYPE_RASTER));
	printer.set_refresh_hz(50);
	printer.set_size(PCW_PRINTER_WIDTH, PCW_PRINTER_HEIGHT);
	printer.set_visarea(0, PCW_PRINTER_WIDTH-1, 0, PCW_PRINTER_HEIGHT-1);
	printer.set_screen_update(FUNC(pcw_state::screen_update_pcw_printer));
	printer.set_palette(m_ppalette);

	config.set_default_layout(layout_pcw);
}

void pcw_state::pcw8512(machine_config &config)
{
	pcw(config);
	m_palette->set_init(FUNC(pcw_state::set_8xxx_palette));

	FLOPPY_CONNECTOR(config, "upd765:0", pcw_ssfloppies, "3ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pcw_dsfloppies, "3dsqd", floppy_image_device::default_mfm_floppy_formats);

	screen_device &printer(SCREEN(config, "printer", SCREEN_TYPE_RASTER));
	printer.set_refresh_hz(50);
	printer.set_size(PCW_PRINTER_WIDTH, PCW_PRINTER_HEIGHT);
	printer.set_visarea(0, PCW_PRINTER_WIDTH-1, 0, PCW_PRINTER_HEIGHT-1);
	printer.set_screen_update(FUNC(pcw_state::screen_update_pcw_printer));
	printer.set_palette(m_ppalette);

	config.set_default_layout(layout_pcw);

	/* internal ram */
	m_ram->set_default_size("512K");
}

/* PCW9512 */
void pcw_state::pcw9512(machine_config &config)
{
	pcw(config);
	m_palette->set_init(FUNC(pcw_state::set_9xxx_palette));

	FLOPPY_CONNECTOR(config, "upd765:0", pcw_dsfloppies, "3dsqd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pcw_dsfloppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	m_maincpu->set_addrmap(AS_IO, &pcw_state::pcw9512_io);

	/* internal ram */
	m_ram->set_default_size("512K");
}

/* PCW9256 */
void pcw_state::pcw9256(machine_config &config)
{
	pcw(config);
	m_palette->set_init(FUNC(pcw_state::set_9xxx_palette));

	FLOPPY_CONNECTOR(config, "upd765:0", pcw_35floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pcw_35floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	m_maincpu->set_addrmap(AS_IO, &pcw_state::pcw9512_io);
}

/* PCW9512+ */
void pcw_state::pcw9512p(machine_config &config)
{
	pcw(config);
	m_palette->set_init(FUNC(pcw_state::set_9xxx_palette));

	FLOPPY_CONNECTOR(config, "upd765:0", pcw_35floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pcw_35floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	m_maincpu->set_addrmap(AS_IO, &pcw_state::pcw9512_io);

	/* internal ram */
	m_ram->set_default_size("512K");
}

/* PCW10 - essentially the same as pcw9512+ */
void pcw_state::pcw10(machine_config &config)
{
	pcw(config);
	m_palette->set_init(FUNC(pcw_state::set_9xxx_palette));

	FLOPPY_CONNECTOR(config, "upd765:0", pcw_35floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pcw_35floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	m_maincpu->set_addrmap(AS_IO, &pcw_state::pcw9512_io);

	/* internal ram */
	m_ram->set_default_size("512K");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(pcw8256)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_FILL(0x0000,0x10000,0x00)
	ROM_REGION(0x400,"printer_mcu",0)  // i8041 9-pin dot-matrix
	ROM_LOAD("40026.ic701", 0, 0x400, CRC(ee8890ae) SHA1(91679cc5e07464ac55ef9a10f7095b2438223332))
	ROM_REGION(0x400,"keyboard_mcu",0) // i8048
	ROM_LOAD("40027.ic801", 0, 0x400, CRC(25260958) SHA1(210e7e25228c79d2920679f217d68e4f14055825))
ROM_END

ROM_START(pcw8512)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_FILL(0x0000,0x10000,0x00)
	ROM_REGION(0x400,"printer_mcu",0)  // i8041 9-pin dot-matrix
	ROM_LOAD("40026.ic701", 0, 0x400, CRC(ee8890ae) SHA1(91679cc5e07464ac55ef9a10f7095b2438223332))
	ROM_REGION(0x400,"keyboard_mcu",0) // i8048
	ROM_LOAD("40027.ic801", 0, 0x400, CRC(25260958) SHA1(210e7e25228c79d2920679f217d68e4f14055825))
ROM_END

ROM_START(pcw9256)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_FILL(0x0000,0x10000,0x00)
	ROM_REGION(0x2000,"printer_mcu",0) // i8041 9-pin dot-matrix
	ROM_LOAD("40026.ic701", 0, 0x400, CRC(ee8890ae) SHA1(91679cc5e07464ac55ef9a10f7095b2438223332))
	ROM_REGION(0x400,"keyboard_mcu",0) // i8048
	ROM_LOAD("40027.ic801", 0, 0x400, CRC(25260958) SHA1(210e7e25228c79d2920679f217d68e4f14055825))
ROM_END

ROM_START(pcw9512)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_FILL(0x0000,0x10000,0x00)
	ROM_REGION(0x2000,"printer_mcu",0) // i8041 daisywheel (schematics say i8039?)
	ROM_LOAD("40103.ic109", 0, 0x2000, CRC(a64d450a) SHA1(ebbf0ef19d39912c1c127c748514dd299915f88b))
	ROM_REGION(0x400,"keyboard_mcu",0) // i8048
	ROM_LOAD("40027.ic801", 0, 0x400, CRC(25260958) SHA1(210e7e25228c79d2920679f217d68e4f14055825))
ROM_END

ROM_START(pcw9512p)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_FILL(0x0000,0x10000,0x00)
	ROM_REGION(0x2000,"printer_mcu",0) // i8041 daisywheel (schematics say i8039?)
	ROM_LOAD("40103.ic109", 0, 0x2000, CRC(a64d450a) SHA1(ebbf0ef19d39912c1c127c748514dd299915f88b))
	ROM_REGION(0x400,"keyboard_mcu",0) // i8048
	ROM_LOAD("40027.ic801", 0, 0x400, CRC(25260958) SHA1(210e7e25228c79d2920679f217d68e4f14055825))
ROM_END

ROM_START(pcw10)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_FILL(0x0000,0x10000,0x00)
	ROM_REGION(0x2000,"printer_mcu",0) // i8041 9-pin dot matrix
	ROM_LOAD("40026.ic701", 0, 0x400, CRC(ee8890ae) SHA1(91679cc5e07464ac55ef9a10f7095b2438223332))
	ROM_REGION(0x400,"keyboard_mcu",0) // i8048
	ROM_LOAD("40027.ic801", 0, 0x400, CRC(25260958) SHA1(210e7e25228c79d2920679f217d68e4f14055825))
ROM_END


/* these are all variants on the pcw design */
/* major difference is memory configuration and drive type */
/*    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT CLASS      INIT      COMPANY        FULLNAME */
COMP( 1985, pcw8256,  0,       0,      pcw8256,  pcw,  pcw_state, init_pcw, "Amstrad plc", "PCW8256",       MACHINE_NOT_WORKING)
COMP( 1985, pcw8512,  pcw8256, 0,      pcw8512,  pcw,  pcw_state, init_pcw, "Amstrad plc", "PCW8512",       MACHINE_NOT_WORKING)
COMP( 1987, pcw9512,  pcw8256, 0,      pcw9512,  pcw,  pcw_state, init_pcw, "Amstrad plc", "PCW9512",       MACHINE_NOT_WORKING)
COMP( 1991, pcw9256,  pcw8256, 0,      pcw9256,  pcw,  pcw_state, init_pcw, "Amstrad plc", "PCW9256",       MACHINE_NOT_WORKING)
COMP( 1991, pcw9512p, pcw8256, 0,      pcw9512p, pcw,  pcw_state, init_pcw, "Amstrad plc", "PCW9512 (+)",   MACHINE_NOT_WORKING)
COMP( 1993, pcw10,    pcw8256, 0,      pcw10,    pcw,  pcw_state, init_pcw, "Amstrad plc", "PCW10",         MACHINE_NOT_WORKING)
