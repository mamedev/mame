// license:GPL-2.0+
// copyright-holders:Raphael Nabet, Brett Wyer
/*
    Corvus Concept driver

    Raphael Nabet, Brett Wyer, 2003-2005
*/

#include "emu.h"
#include "includes/concept.h"


#define VERBOSE 1

#define LOG(x)  do { if (VERBOSE > 0) logerror x; } while (0)
#define VLOG(x) do { if (VERBOSE > 1) logerror x; } while (0)


/* interrupt priority encoder */
enum
{
	IOCINT_level = 1,   /* serial lines (CTS, DSR & DCD) and I/O ports */
	SR1INT_level,       /* serial port 1 acia */
	OMINT_level,        /* omninet */
	SR0INT_level,       /* serial port 0 acia */
	TIMINT_level,       /* via */
	KEYINT_level,       /* keyboard acia */
	NMIINT_level            /* reserved */
};

/* Clock interface */

/* Omninet */
/*static int ready;*/           /* ready line from monochip, role unknown */

/* Via */


void concept_state::machine_start()
{
	/* initialize int state */
	m_pending_interrupts = 0;

	/* initialize clock interface */
	m_clock_enable = FALSE /*TRUE*/;

	save_item(NAME(m_pending_interrupts));
	save_item(NAME(m_clock_enable));
	save_item(NAME(m_clock_address));
}


void concept_state::machine_reset()
{
	// OS will not boot if TDRE is clear on ACIA 0; this fixes that
	m_acia0->write_cts(CLEAR_LINE);
	m_acia1->write_cts(CLEAR_LINE);
}

void concept_state::video_start()
{
}

UINT32 concept_state::screen_update_concept(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* resolution is 720*560 */
	UINT16 *videoram = m_videoram;
	int x, y;
	UINT16 *line;

	for (y = 0; y < 560; y++)
	{
		line = &bitmap.pix16(560-1-y);
		for (x = 0; x < 720; x++)
			line[720-1-x] = (videoram[(x+48+y*768)>>4] & (0x8000 >> ((x+48+y*768) & 0xf))) ? 1 : 0;
	}
	return 0;
}

void concept_state::concept_set_interrupt(int level, int state)
{
	int interrupt_mask;
	int final_level;

	if (state)
		m_pending_interrupts |= 1 << level;
	else
		m_pending_interrupts &= ~ (1 << level);

	for (final_level = 7, interrupt_mask = m_pending_interrupts; (final_level > 0) && ! (interrupt_mask & 0x80); final_level--, interrupt_mask <<= 1)
		;

	if (final_level)
		/* assert interrupt */
		m_maincpu->set_input_line_and_vector(M68K_IRQ_1 + final_level - 1, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	else
	{
		/* clear all interrupts */
		m_maincpu->set_input_line_and_vector(M68K_IRQ_1 + level - 1, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
	}
}

/*
    VIA port A

    0: omninet ready (I)
    1: CTS0 (I)
    2: CTS1 (I)
    3: DSR0 (I)
    4: DSR1 (I)
    5: DCD0 (I)
    6: DCD1 (I)
    7: IOX (O)
*/
READ8_MEMBER(concept_state::via_in_a)
{
	LOG(("via_in_a: VIA port A (Omninet and COMM port status) read\n"));
	return 1;       /* omninet ready always 1 */
}

WRITE8_MEMBER(concept_state::via_out_a)
{
	LOG(("via_out_a: VIA port A status written: data=0x%2.2x\n", data));
	/*iox = (data & 0x80) != 0;*/
}

/*
    VIA port B

    0: video off (O)
    1: video address 17 (O)
    2: video address 18 (O)
    3: monitor orientation (I)
    4: CH rate select DC0 (serial port line) (O)
    5: CH rate select DC1 (serial port line) (O)
    6: boot switch 0 (I)
    7: boot switch 1 (I)
*/
READ8_MEMBER(concept_state::via_in_b)
{
	UINT8 status;

	status = ((ioport("DSW0")->read() & 0x80) >> 1) | ((ioport("DSW0")->read() & 0x40) << 1);
	LOG(("via_in_b: VIA port B (DIP switches, Video, Comm Rate) - status: 0x%2.2x\n", status));
	return status;
}

WRITE8_MEMBER(concept_state::via_out_b)
{
	VLOG(("via_out_b: VIA port B (Video Control and COMM rate select) written: data=0x%2.2x\n", data));
}

/*
    VIA CB2: used as sound output
*/

WRITE_LINE_MEMBER(concept_state::via_out_cb2)
{
//  LOG(("via_out_cb2: Sound control written: data=0x%2.2x\n", state));
	m_speaker->level_w(state);
}

/*
    VIA irq -> 68k level 5
*/
WRITE_LINE_MEMBER(concept_state::via_irq_func)
{
	concept_set_interrupt(TIMINT_level, state);
}

READ16_MEMBER(concept_state::concept_io_r)
{
	if (! ACCESSING_BITS_0_7)
		return 0;

	switch ((offset >> 8) & 7)
	{
	case 0:
		/* I/O slot regs */
		switch ((offset >> 4) & 7)
		{
			case 1: // IO1 registers
			case 2: // IO2 registers
			case 3: // IO3 registers
			case 4: // IO4 registers
				{
					int slot = ((offset >> 4) & 7);
					device_a2bus_card_interface *card = m_a2bus->get_a2bus_card(slot);

					if (card)
					{
						return card->read_c0nx(space, offset & 0x0f);
					}

					return 0xff;
				}
				break;

			default: // ???
				logerror("concept_io_r: Slot I/O memory accessed for unknown purpose at address 0x03%4.4x\n", offset << 1);
				break;
		}
		break;

	case 1: // IO1 ROM
	case 2: // IO2 ROM
	case 3: // IO3 ROM
	case 4: // IO4 ROM
		{
			int slot = (offset >> 8) & 7;
			device_a2bus_card_interface *card = m_a2bus->get_a2bus_card(slot);

			if (card)
			{
				return card->read_cnxx(space, offset & 0xff);
			}
		}
		break;

	case 5:
		/* slot status */
		LOG(("concept_io_r: Slot status read at address 0x03%4.4x\n", offset << 1));
		break;

	case 6:
		/* calendar R/W */
		VLOG(("concept_io_r: Calendar read at address 0x03%4.4x\n", offset << 1));
		if (!m_clock_enable)
			return m_mm58274->read(space, m_clock_address);
		break;

	case 7:
		/* I/O ports */
		switch ((offset >> 4) & 7)
		{
		case 0:
			/* NKBP keyboard */
			return m_kbdacia->read(space, (offset & 3));

		case 1:
			/* NSR0 data comm port 0 */
			return m_acia0->read(space, (offset & 3));

		case 2:
			/* NSR1 data comm port 1 */
			return m_acia1->read(space, (offset & 3));

		case 3:
			/* NVIA versatile system interface */
//  LOG(("concept_io_r: VIA read at address 0x03%4.4x\n", offset << 1));
			{
				via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
				return via_0->read(space, offset & 0xf);
			}

		case 4:
			/* NCALM clock calendar address and strobe register */
			/* write-only? */
			LOG(("concept_io_r: NCALM clock/calendar read at address 0x03%4.4x\n", offset << 1));
			break;

		case 5:
			/* NOMNI omninet strobe */
			logerror("concept_io_r: NOMNI Omninet Transporter register read at address 0x03%4.4x\n", offset << 1);
			break;

		case 6:
			/* NOMOFF reset omninet interrupt flip-flop */
			logerror("concept_io_r: NOMOFF Omninet interrupt flip-flop read at address 0x03%4.4x\n", offset << 1);
			break;

		case 7:
			/* NIOSTRB external I/O ROM strobe (disables interface RAM) */
			logerror("concept_io_r: NIOSTRB External I/O ROM strobe read at address 0x03%4.4x\n", offset << 1);
			break;
		}
		break;
	}

	return 0;
}

WRITE16_MEMBER(concept_state::concept_io_w)
{
	if (! ACCESSING_BITS_0_7)
		return;

	data &= 0xff;

	switch ((offset >> 8) & 7)
	{
	case 0:
		/* I/O slot regs */
		switch ((offset >> 4) & 7)
		{
			case 1: // IO1 registers
			case 2: // IO2 registers
			case 3: // IO3 registers
			case 4: // IO4 registers
				{
					int slot = (offset >> 4) & 7;
					device_a2bus_card_interface *card = m_a2bus->get_a2bus_card(slot);

					if (card)
					{
						return card->write_c0nx(space, offset & 0x0f, data);
					}
				}
				break;

			default:    // ???
				logerror("concept_io_w: Slot I/O memory written for unknown purpose at address 0x03%4.4x, data: 0x%4.4x\n", offset << 1, data);
				break;
		}
		break;

	case 1: // IO1 ROM
	case 2: // IO2 ROM
	case 3: // IO3 ROM
	case 4: // IO4 ROM
		{
			int slot = (offset >> 8) & 7;
			device_a2bus_card_interface *card = m_a2bus->get_a2bus_card(slot);

			if (card)
			{
				return card->write_cnxx(space, offset & 0xff, data);
			}
		}
		break;

	case 5:
		/* slot status */
		logerror("concept_io_w: Slot status written at address 0x03%4.4x, data: 0x%4.4x\n", offset << 1, data);
		break;

	case 6:
		/* calendar R/W */
		LOG(("concept_io_w: Calendar written to at address 0x03%4.4x, data: 0x%4.4x\n", offset << 1, data));
		if (!m_clock_enable)
			m_mm58274->write(space, m_clock_address, data & 0xf);
		break;

	case 7:
		/* I/O ports */
		switch ((offset >> 4) & 7)
		{
		case 0:
			/* NKBP keyboard */
			m_kbdacia->write(space, (offset & 3), data);
			break;

		case 1:
			/* NSR0 data comm port 0 */
			m_acia0->write(space, (offset & 3), data);
			break;

		case 2:
			/* NSR1 data comm port 1 */
			m_acia1->write(space, (offset & 3), data);
			break;

		case 3:
			/* NVIA versatile system interface */
			{
				via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
				via_0->write(space, offset & 0xf, data);
			}
			break;

		case 4:
			/* NCALM clock calendar address and strobe register */
			if (m_clock_enable != ((data & 0x10) != 0))
			{
				m_clock_enable = ((data & 0x10) != 0);
				if (! m_clock_enable)
					/* latch address when enable goes low */
					m_clock_address = data & 0x0f;
			}
			/*volume_control = (data & 0x20) != 0;*/
			/*alt_map = (data & 0x40) != 0;*/
			break;

		case 5:
			/* NOMNI omninet strobe */
			logerror("concept_io_w: NOMNI Omninet Transporter register written at address 0x03%4.4x, data: 0x%4.4x\n", offset << 1, data);
			break;

		case 6:
			/* NOMOFF reset omninet interrupt flip-flop */
			logerror("concept_io_w: NOMOFF Omninet flip-flop reset at address 0x03%4.4x, data: 0x%4.4x\n", offset << 1, data);
			break;

		case 7:
			/* NIOSTRB external I/O ROM strobe */
			logerror("concept_io_w: NIOSTRB External I/O ROM strobe written at address 0x03%4.4x, data: 0x%4.4x\n", offset << 1, data);
			break;
		}
		break;
	}
}
