/*
    Corvus Concept driver

    Raphael Nabet, Brett Wyer, 2003-2005
*/

#include "emu.h"
#include "includes/concept.h"
#include "machine/6522via.h"
#include "machine/mm58274c.h"	/* mm58274 seems to be compatible with mm58174 */
//#include "machine/6551acia.h"
#include "machine/wd17xx.h"
#include "cpu/m68000/m68000.h"
#include "includes/corvushd.h"
#include "imagedev/flopdrv.h"


#define VERBOSE 1

#define LOG(x)  do { if (VERBOSE > 0) logerror x; } while (0)
#define VLOG(x) do { if (VERBOSE > 1) logerror x; } while (0)


/* interrupt priority encoder */
enum
{
	IOCINT_level = 1,	/* serial lines (CTS, DSR & DCD) and I/O ports */
	SR1INT_level,		/* serial port 1 acia */
	OMINT_level,		/* omninet */
	SR0INT_level,		/* serial port 0 acia */
	TIMINT_level,		/* via */
	KEYINT_level,		/* keyboard acia */
	NMIINT_level			/* reserved */
};

/* Clock interface */

/* Omninet */
/*static int ready;*/			/* ready line from monochip, role unknown */

/* Via */
static DECLARE_READ8_DEVICE_HANDLER(via_in_a);
static DECLARE_WRITE8_DEVICE_HANDLER(via_out_a);
static DECLARE_READ8_DEVICE_HANDLER(via_in_b);
static DECLARE_WRITE8_DEVICE_HANDLER(via_out_b);
static DECLARE_WRITE8_DEVICE_HANDLER(via_out_cb2);
static void via_irq_func(device_t *device, int state);


const via6522_interface concept_via6522_intf =
{	/* main via */
	DEVCB_HANDLER(via_in_a), DEVCB_HANDLER(via_in_b),
	DEVCB_NULL, DEVCB_NULL,
	DEVCB_NULL, DEVCB_NULL,
	DEVCB_HANDLER(via_out_a), DEVCB_HANDLER(via_out_b),
	DEVCB_NULL, DEVCB_NULL,
	DEVCB_NULL, DEVCB_HANDLER(via_out_cb2),
	DEVCB_LINE(via_irq_func)
};

/* keyboard interface */

/* Expansion slots */

static void concept_fdc_init(running_machine &machine, int slot);
static void concept_hdc_init(running_machine &machine, int slot);

void concept_state::machine_start()
{
	/* initialize int state */
	m_pending_interrupts = 0;

	/* initialize clock interface */
	m_clock_enable = 0/*1*/;

	/* clear keyboard interface state */
	m_KeyQueueHead = m_KeyQueueLen = 0;
	memset(m_KeyStateSave, 0, sizeof(m_KeyStateSave));

	/* initialize expansion slots */
	memset(m_expansion_slots, 0, sizeof(m_expansion_slots));

	concept_hdc_init(machine(), 1);	/* Flat cable Hard Disk Controller in Slot 2 */
	concept_fdc_init(machine(), 2);	/* Floppy Disk Controller in Slot 3 */
}

static void install_expansion_slot(running_machine &machine, int slot,
	read8_space_func reg_read, write8_space_func reg_write,
	read8_space_func rom_read, write8_space_func rom_write)
{
	concept_state *state = machine.driver_data<concept_state>();
	state->m_expansion_slots[slot].reg_read = reg_read;
	state->m_expansion_slots[slot].reg_write = reg_write;
	state->m_expansion_slots[slot].rom_read = rom_read;
	state->m_expansion_slots[slot].rom_write = rom_write;
}

void concept_state::video_start()
{
}

SCREEN_UPDATE_IND16(concept)
{
	/* resolution is 720*560 */
	concept_state *state = screen.machine().driver_data<concept_state>();
	UINT16 *videoram = state->m_videoram;
	int x, y;
	UINT16 *line;

	for (y = 0; y < 560; y++)
	{
		line = &bitmap.pix16(560-1-y);
		for (x = 0; x < 720; x++)
			line[720-1-x] = (videoram[(x+48+y*768)>>4] & (0x8000 >> ((x+48+y*768) & 0xf))) ? 0 : 1;
	}
	return 0;
}

static void concept_set_interrupt(running_machine &machine, int level, int state)
{
	concept_state *drvstate = machine.driver_data<concept_state>();
	int interrupt_mask;
	int final_level;

	if (state)
		drvstate->m_pending_interrupts |= 1 << level;
	else
		drvstate->m_pending_interrupts &= ~ (1 << level);

	for (final_level = 7, interrupt_mask = drvstate->m_pending_interrupts; (final_level > 0) && ! (interrupt_mask & 0x80); final_level--, interrupt_mask <<= 1)
		;

	if (final_level)
		/* assert interrupt */
		machine.device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_1 + final_level - 1, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	else
		/* clear all interrupts */
		machine.device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_1, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
}

INLINE void post_in_KeyQueue(concept_state *state, int keycode)
{
	state->m_KeyQueue[(state->m_KeyQueueHead+state->m_KeyQueueLen) % KeyQueueSize] = keycode;
	state->m_KeyQueueLen++;
}

static void poll_keyboard(running_machine &machine)
{
	concept_state *state = machine.driver_data<concept_state>();
	UINT32 keystate;
	UINT32 key_transitions;
	int i, j;
	int keycode;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	for(i = 0; (i < /*4*/3) && (state->m_KeyQueueLen <= (KeyQueueSize-MaxKeyMessageLen)); i++)
	{
		keystate = machine.root_device().ioport(keynames[2*i])->read() | (machine.root_device().ioport(keynames[2*i + 1])->read() << 16);
		key_transitions = keystate ^ state->m_KeyStateSave[i];
		if(key_transitions)
		{
			for(j = 0; (j < 32) && (state->m_KeyQueueLen <= (KeyQueueSize-MaxKeyMessageLen)); j++)
			{
				if((key_transitions >> j) & 1)
				{
					keycode = (i << 5) | j;

					if (((keystate >> j) & 1))
					{
						/* key is pressed */
						state->m_KeyStateSave[i] |= (1 << j);
						keycode |= 0x80;
					}
					else
						/* key is released */
						state->m_KeyStateSave[i] &= ~ (1 << j);

					post_in_KeyQueue(state, keycode);
					concept_set_interrupt(machine, KEYINT_level, 1);
				}
			}
		}
	}
}

INTERRUPT_GEN( concept_interrupt )
{
	poll_keyboard(device->machine());
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
static  READ8_DEVICE_HANDLER(via_in_a)
{
	LOG(("via_in_a: VIA port A (Omninet and COMM port status) read\n"));
	return 1;		/* omninet ready always 1 */
}

static WRITE8_DEVICE_HANDLER(via_out_a)
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
static READ8_DEVICE_HANDLER(via_in_b)
{
	UINT8 status;

	status = ((device->machine().root_device().ioport("DSW0")->read() & 0x80) >> 1) | ((device->machine().root_device().ioport("DSW0")->read() & 0x40) << 1);
	LOG(("via_in_b: VIA port B (DIP switches, Video, Comm Rate) - status: 0x%2.2x\n", status));
	return status;
}

static WRITE8_DEVICE_HANDLER(via_out_b)
{
	VLOG(("via_out_b: VIA port B (Video Control and COMM rate select) written: data=0x%2.2x\n", data));
}

/*
    VIA CB2: used as sound output
*/
static WRITE8_DEVICE_HANDLER(via_out_cb2)
{
	LOG(("via_out_cb2: Sound control written: data=0x%2.2x\n", data));
}

/*
    VIA irq -> 68k level 5
*/
static void via_irq_func(device_t *device, int state)
{
	concept_set_interrupt(device->machine(), TIMINT_level, state);
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
		case 1:
			/* IO1 registers */
		case 2:
			/* IO2 registers */
		case 3:
			/* IO3 registers */
		case 4:
			/* IO4 registers */
			{
				int slot = ((offset >> 4) & 7) - 1;
				if (m_expansion_slots[slot].reg_read)
					return m_expansion_slots[slot].reg_read(space, offset & 0xf);
			}
			break;

		default:
			/* ??? */
			logerror("concept_io_r: Slot I/O memory accessed for unknown purpose at address 0x03%4.4x\n", offset << 1);
			break;
		}
		break;

	case 1:
		/* IO1 ROM */
	case 2:
		/* IO2 ROM */
	case 3:
		/* IO3 ROM */
	case 4:
		/* IO4 ROM */
		{
			int slot = ((offset >> 8) & 7) - 1;
			LOG(("concept_io_r: Slot ROM memory accessed for slot %d at address 0x03%4.4x\n", slot, offset << 1));
			if (m_expansion_slots[slot].rom_read)
				return m_expansion_slots[slot].rom_read(space, offset & 0xff);
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
			return mm58274c_r(machine().device("mm58274c"), space, m_clock_address);
		break;

	case 7:
		/* I/O ports */
		switch ((offset >> 4) & 7)
		{
		case 0:
			/* NKBP keyboard */
			switch (offset & 0xf)
			{
				int reply;

			case 0:
				/* data */
				reply = 0;

				if (m_KeyQueueLen)
				{
					reply = m_KeyQueue[m_KeyQueueHead];
					m_KeyQueueHead = (m_KeyQueueHead + 1) % KeyQueueSize;
					m_KeyQueueLen--;
				}

				if (!m_KeyQueueLen)
					concept_set_interrupt(machine(), KEYINT_level, 0);

				return reply;

			case 1:
				/* always tell transmit is empty */
				reply = m_KeyQueueLen ? 0x98 : 0x10;
				break;
			}
			break;
		case 1:
			/* NSR0 data comm port 0 */
		case 2:
			/* NSR1 data comm port 1 */
			LOG(("concept_io_r: Data comm port read at address 0x03%4.4x\n", offset << 1));
			if ((offset & 0xf) == 1)
				return 0x10;
			break;

		case 3:
			/* NVIA versatile system interface */
			LOG(("concept_io_r: VIA read at address 0x03%4.4x\n", offset << 1));
			{
				via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
				return via_0->read(space, offset & 0xf);
			}
			break;

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
		case 1:
			/* IO1 registers */
		case 2:
			/* IO2 registers */
		case 3:
			/* IO3 registers */
		case 4:
			/* IO4 registers */
			{
				int slot = ((offset >> 4) & 7) - 1;
				LOG(("concept_io_w: Slot I/O register written for slot %d at address 0x03%4.4x, data: 0x%4.4x\n",
					slot, offset << 1, data));
				if (m_expansion_slots[slot].reg_write)
					m_expansion_slots[slot].reg_write(space, offset & 0xf, data);
			}
			break;

		default:
			/* ??? */
			logerror("concept_io_w: Slot I/O memory written for unknown purpose at address 0x03%4.4x, data: 0x%4.4x\n", offset << 1, data);
			break;
		}
		break;

	case 1:
		/* IO1 ROM */
	case 2:
		/* IO2 ROM */
	case 3:
		/* IO3 ROM */
	case 4:
		/* IO4 ROM */
		{
			int slot = ((offset >> 8) & 7) - 1;
			LOG(("concept_io_w: Slot ROM memory written to for slot %d at address 0x03%4.4x, data: 0x%4.4x\n", slot, offset << 1, data));
			if (m_expansion_slots[slot].rom_write)
				m_expansion_slots[slot].rom_write(space, offset & 0xff, data);
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
			mm58274c_w(machine().device("mm58274c"), space, m_clock_address, data & 0xf);
		break;

	case 7:
		/* I/O ports */
		switch ((offset >> 4) & 7)
		{
		case 0:
			/* NKBP keyboard */
		case 1:
			/* NSR0 data comm port 0 */
		case 2:
			/* NSR1 data comm port 1 */
			/*acia_6551_w((offset >> 4) & 7, offset & 0x3, data);*/
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
				m_clock_enable = (data & 0x10) != 0;
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

/*
    Concept fdc controller
*/

enum
{
	LS_DRQ_bit		= 0,	// DRQ
	LS_INT_bit		= 1,	// INT
	LS_SS_bit		= 4,	// 1 if single-sided (floppy or drive?)
	LS_8IN_bit		= 5,	// 1 if 8" floppy drive?
	LS_DSKCHG_bit	= 6,	// 0 if disk changed, 1 if not
	LS_SD_bit		= 7,	// 1 if single density

	LS_DRQ_mask		= (1 << LS_DRQ_bit),
	LS_INT_mask		= (1 << LS_INT_bit),
	LS_SS_mask		= (1 << LS_SS_bit),
	LS_8IN_mask		= (1 << LS_8IN_bit),
	LS_DSKCHG_mask	= (1 << LS_DSKCHG_bit),
	LS_SD_mask		= (1 << LS_SD_bit)
};
enum
{
	LC_FLPSD1_bit	= 0,	// 0 if side 0 , 1 if side 1
	LC_DE0_bit		= 1,	// drive select bit 0
	LC_DE1_bit		= 4,	// drive select bit 1
	LC_MOTOROF_bit	= 5,	// 1 if motor to be turned off
	LC_FLP8IN_bit	= 6,	// 1 to select 8", 0 for 5"1/4 (which I knew what it means)
	LC_FMMFM_bit	= 7,	// 1 to select single density, 0 for double

	LC_FLPSD1_mask	= (1 << LC_FLPSD1_bit),
	LC_DE0_mask		= (1 << LC_DE0_bit),
	LC_DE1_mask		= (1 << LC_DE1_bit),
	LC_MOTOROF_mask	= (1 << LC_MOTOROF_bit),
	LC_FLP8IN_mask	= (1 << LC_FLP8IN_bit),
	LC_FMMFM_mask	= (1 << LC_FMMFM_bit)
};


static  READ8_HANDLER(concept_fdc_reg_r);
static WRITE8_HANDLER(concept_fdc_reg_w);
static  READ8_HANDLER(concept_fdc_rom_r);

static void concept_fdc_init(running_machine &machine, int slot)
{
	concept_state *state = machine.driver_data<concept_state>();
	state->m_fdc_local_status = 0;
	state->m_fdc_local_command = 0;

	install_expansion_slot(machine, slot, concept_fdc_reg_r, concept_fdc_reg_w, concept_fdc_rom_r, NULL);
}

static WRITE_LINE_DEVICE_HANDLER( concept_fdc_intrq_w )
{
	concept_state *drvstate = device->machine().driver_data<concept_state>();
	if (state)
		drvstate->m_fdc_local_status |= LS_INT_mask;
	else
		drvstate->m_fdc_local_status &= ~LS_INT_mask;
}

static WRITE_LINE_DEVICE_HANDLER( concept_fdc_drq_w )
{
	concept_state *drvstate = device->machine().driver_data<concept_state>();
	if (state)
		drvstate->m_fdc_local_status |= LS_DRQ_mask;
	else
		drvstate->m_fdc_local_status &= ~LS_DRQ_mask;
}

const wd17xx_interface concept_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_LINE(concept_fdc_intrq_w),
	DEVCB_LINE(concept_fdc_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

static  READ8_HANDLER(concept_fdc_reg_r)
{
	concept_state *state = space.machine().driver_data<concept_state>();
	device_t *fdc = space.machine().device("wd179x");
	switch (offset)
	{
	case 0:
		/* local Status reg */
		return state->m_fdc_local_status;

	case 8:
		/* FDC STATUS REG */
		return wd17xx_status_r(fdc, space, offset);

	case 9:
		/* FDC TRACK REG */
		return wd17xx_track_r(fdc, space, offset);

	case 10:
		/* FDC SECTOR REG */
		return wd17xx_sector_r(fdc, space, offset);

	case 11:
		/* FDC DATA REG */
		return wd17xx_data_r(fdc, space, offset);
	}

	return 0;
}

static WRITE8_HANDLER(concept_fdc_reg_w)
{
	concept_state *state = space.machine().driver_data<concept_state>();
	int current_drive;
	device_t *fdc = space.machine().device("wd179x");
	switch (offset)
	{
	case 0:
		/* local command reg */
		state->m_fdc_local_command = data;

		wd17xx_set_side(fdc,(data & LC_FLPSD1_mask) != 0);
		current_drive = ((data >> LC_DE0_bit) & 1) | ((data >> (LC_DE1_bit-1)) & 2);
		wd17xx_set_drive(fdc, current_drive);
		/*motor_on = (data & LC_MOTOROF_mask) == 0;*/
		// floppy_drive_set_motor_state(floppy_get_device(machine,  current_drive), (data & LC_MOTOROF_mask) == 0 ? 1 : 0);
		/*flp_8in = (data & LC_FLP8IN_mask) != 0;*/
		wd17xx_dden_w(fdc, BIT(data, 7));
		floppy_drive_set_ready_state(floppy_get_device(space.machine(), current_drive), 1, 0);
		break;

	case 8:
		/* FDC COMMAMD REG */
		wd17xx_command_w(fdc, space, offset, data);
		break;

	case 9:
		/* FDC TRACK REG */
		wd17xx_track_w(fdc, space, offset, data);
		break;

	case 10:
		/* FDC SECTOR REG */
		wd17xx_sector_w(fdc, space, offset, data);
		break;

	case 11:
		/* FDC DATA REG */
		wd17xx_data_w(fdc, space, offset, data);
		break;
	}
}

static  READ8_HANDLER(concept_fdc_rom_r)
{
	static const UINT8 data[] = "CORVUS01";
	return (offset < 8) ? data[offset] : 0;
}

/*
 *  Concept Hard Disk Controller (hdc)
 */

static  READ8_HANDLER(concept_hdc_reg_r);
static WRITE8_HANDLER(concept_hdc_reg_w);
static  READ8_HANDLER(concept_hdc_rom_r);

/*
 *  Hook up the Register and ROM R/W routines into the Slot I/O Space
 */

static void concept_hdc_init(running_machine &machine, int slot)
{
	if(corvus_hdc_init(machine))
		install_expansion_slot(machine, slot, concept_hdc_reg_r, concept_hdc_reg_w, concept_hdc_rom_r, NULL);
}

/*
 *  Handle reads against the Hard Disk Controller's onboard registers
 */
static READ8_HANDLER(concept_hdc_reg_r)
{
	switch (offset)
	{
	case 0:
		/* HDC Data Register */
		return corvus_hdc_data_r(space, offset);

	case 1:
		/* HDC Status Register */
		return corvus_hdc_status_r(space, offset);
	}

	return 0;
}

/*
 *  Handle writes against the Hard Disk Controller's onboard registers
 */
static WRITE8_HANDLER(concept_hdc_reg_w)
{
	switch (offset)
	{
	case 0:
		/* HDC Data Register */
		corvus_hdc_data_w(space, offset, data);
		break;
	}
}

/*
 *  Handle reads agsint the Hard Disk Controller's onboard ROM
 */
static  READ8_HANDLER(concept_hdc_rom_r)
{
	static const UINT8 data[8] = { 0xa9, 0x20, 0xa9, 0x00, 0xa9, 0x03, 0xa9, 0x3c };			/* Same as Apple II */
	return (offset < 8) ? data[offset] : 0;
}
