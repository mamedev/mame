/*
    machine/mbc55x.c

    Machine driver for the Sanyo MBC-550 and MBC-555.

    Phill Harvey-Smith
    2011-01-29.
*****************************************************************************/

#include "includes/mbc55x.h"


/*-------------------------------------------------------------------------*/
/* Defines, constants, and global variables                                */
/*-------------------------------------------------------------------------*/

/* Debugging */

#define DEBUG_SET(flags)    ((state->m_debug_machine & (flags))==(flags))

#define DEBUG_NONE			0x0000000
#define DMA_BREAK           0x0000001
#define DECODE_BIOS         0x0000002
#define DECODE_BIOS_RAW     0x0000004
#define DECODE_DOS21		0x0000008

#define LOG_KEYBOARD        1

static void decode_dos21(device_t *device,offs_t pc);
//static void mbc55x_recalculate_ints(running_machine &machine);
static void mbc55x_debug(running_machine &machine, int ref, int params, const char *param[]);
static int instruction_hook(device_t &device, offs_t curpc);
//static void fdc_reset(running_machine &machine);
//static void set_disk_int(running_machine &machine, int state);


/* Floppy drives WD2793 */

const wd17xx_interface mbc55x_wd17xx_interface =
{
	DEVCB_LINE_GND,
	DEVCB_DRIVER_LINE_MEMBER(mbc55x_state, mbc55x_fdc_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(mbc55x_state, mbc55x_fdc_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};


/* 8255 Configuration */

READ8_MEMBER( mbc55x_state::ppi8255_r )
{
	return m_ppi->read(space, offset>>1);
}

WRITE8_MEMBER( mbc55x_state::ppi8255_w )
{
	m_ppi->write(space, offset>>1, data);
}

READ8_MEMBER( mbc55x_state::mbc55x_ppi_porta_r )
{
	return 0xff;
}

READ8_MEMBER( mbc55x_state::mbc55x_ppi_portb_r )
{
	return 0xff;
}

READ8_MEMBER( mbc55x_state::mbc55x_ppi_portc_r )
{
	return 0xff;
}

WRITE8_MEMBER( mbc55x_state::mbc55x_ppi_porta_w )
{
}

WRITE8_MEMBER( mbc55x_state::mbc55x_ppi_portb_w )
{
}

WRITE8_MEMBER( mbc55x_state::mbc55x_ppi_portc_w )
{
	wd17xx_set_drive(m_fdc,(data & 0x03));
	wd17xx_set_side(m_fdc, BIT(data, 2));
}

I8255_INTERFACE( mbc55x_ppi8255_interface )
{
	DEVCB_DRIVER_MEMBER(mbc55x_state, mbc55x_ppi_porta_r),
	DEVCB_DRIVER_MEMBER(mbc55x_state, mbc55x_ppi_porta_w),
	DEVCB_DRIVER_MEMBER(mbc55x_state, mbc55x_ppi_portb_r),
	DEVCB_DRIVER_MEMBER(mbc55x_state, mbc55x_ppi_portb_w),
	DEVCB_DRIVER_MEMBER(mbc55x_state, mbc55x_ppi_portc_r),
	DEVCB_DRIVER_MEMBER(mbc55x_state, mbc55x_ppi_portc_w)
};

/* Serial port USART, unimplemented as yet */

READ8_MEMBER( mbc55x_state::mbc55x_usart_r )
{
	return 0;
}

WRITE8_MEMBER( mbc55x_state::mbc55x_usart_w )
{
}

/* PIC 8259 Configuration */

const struct pic8259_interface mbc55x_pic8259_config =
{
	DEVCB_CPU_INPUT_LINE(MAINCPU_TAG, INPUT_LINE_IRQ0),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};

READ8_MEMBER(mbc55x_state::mbcpic8259_r)
{
	return pic8259_r(m_pic, space, offset>>1);
}

WRITE8_MEMBER(mbc55x_state::mbcpic8259_w)
{
	pic8259_w(m_pic, space, offset>>1, data);
}

static IRQ_CALLBACK(mbc55x_irq_callback)
{
	mbc55x_state *state = device->machine().driver_data<mbc55x_state>();
	return pic8259_acknowledge( state->m_pic );
}

/* PIT8253 Configuration */

const struct pit8253_config mbc55x_pit8253_config =
{
	{
		{
			PIT_C0_CLOCK,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE(PIC8259_TAG, pic8259_ir0_w)
		},
		{
			PIT_C1_CLOCK,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE(PIC8259_TAG, pic8259_ir1_w)
		},
		{
			PIT_C2_CLOCK,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(mbc55x_state, pit8253_t2)
		}
	}
};

READ8_MEMBER(mbc55x_state::mbcpit8253_r)
{
	return pit8253_r(m_pit, space, offset>>1);
}

WRITE8_MEMBER(mbc55x_state::mbcpit8253_w)
{
	pit8253_w(m_pit, space, offset>>1, data);
}

WRITE_LINE_MEMBER( mbc55x_state::pit8253_t2 )
{
	m_kb_uart->transmit_clock();
	m_kb_uart->receive_clock();
}

/* Video ram page register */

READ8_MEMBER( mbc55x_state::vram_page_r )
{
	return m_vram_page;
}

WRITE8_MEMBER( mbc55x_state::vram_page_w )
{
	logerror("%s : set vram page to %02X\n", machine().describe_context(),data);

	m_vram_page=data;
}

READ8_MEMBER(mbc55x_state::mbc55x_disk_r)
{
	return wd17xx_r(m_fdc, space, offset>>1);
}

WRITE8_MEMBER(mbc55x_state::mbc55x_disk_w)
{
	wd17xx_w(m_fdc, space, offset>>1, data);
}

WRITE_LINE_MEMBER( mbc55x_state::mbc55x_fdc_intrq_w )
{
}

WRITE_LINE_MEMBER( mbc55x_state::mbc55x_fdc_drq_w )
{
}

/*
    Keyboard emulation

*/

static void keyboard_reset(running_machine &machine)
{
	mbc55x_state *state = machine.driver_data<mbc55x_state>();
	logerror("keyboard_reset()\n");

	memset(state->m_keyboard.keyrows,0xFF,MBC55X_KEYROWS);
	state->m_keyboard.key_special=0;

	// Setup timer to scan keyboard.
	state->m_keyboard.keyscan_timer->adjust(attotime::zero, 0, attotime::from_hz(50));
}

static void scan_keyboard(running_machine &machine)
{
	mbc55x_state *state = machine.driver_data<mbc55x_state>();
	UINT8   keyrow;
	UINT8   row;
	UINT8   bitno;
	UINT8   mask;

	char	key;
	static const char *const keynames[] =
	{
		"KEY0", "KEY1", "KEY2", "KEY3", "KEY4",
		"KEY5", "KEY6", "KEY7", "KEY8", "KEY9",
		"KEY10"
	};

	static const char keyvalues_normal[MBC55X_KEYROWS][8] =
	{
		{ '1',	'2', '3', '4' , '5',  '6',  '7',  '8'  },
		{ '9',	'0', '-', '=',  '\\', 'q',  'w',  'e' },
		{ 'r',	't', 'y', 'u',  'i',  'o',  'p',  '[' },
		{ ']',	'a', 's', 'd',  'f',  'g',  'h',  'j' },
		{ 'k',	'l', ';', '\'', '`',  0x0D,  'z',  'x' },
		{ 'c',	'v', 'b', 'n',  'm',  ',',  '.',  '/', },
		{ ' ',	' ', ' ', ' ',  ' ',  ' ',  ' ',  ' ', }

	};

	static const char keyvalues_shift[MBC55X_KEYROWS][8] =
	{
		{ '!',  '@', '#', '$' , '%',  '^',  '&',  '*'  },
		{ '(',  ')', '_', '+',  '|',  'Q',  'W',  'E'  },
		{ 'R',	'T', 'Y', 'U',  'I',  'O',  'P',  '{' },
		{ '}',	'A', 'S', 'D',  'F',  'G',  'H',  'J' },
		{ 'K',	'L', ':', '"',  '~',  0x0d, 'Z',  'X' },
		{ 'C',	'V', 'B', 'N',  'M',  ',',  '?',  '/' },
		{ ' ',	' ', ' ', ' ',  ' ',  ' ',  ' ',  ' ' }
	};

	// First read shift, control and graph

	state->m_keyboard.key_special = machine.root_device().ioport(KEY_SPECIAL_TAG)->read();

	for(row=0; row<MBC55X_KEYROWS; row++)
	{
		keyrow = machine.root_device().ioport(keynames[row])->read();

		for(mask=0x80, bitno=7;mask>0;mask=mask>>1, bitno-=1)
		{
			if(!(keyrow & mask) && (state->m_keyboard.keyrows[row] & mask))
			{
				if(state->m_keyboard.key_special & (KEY_BIT_LSHIFT | KEY_BIT_RSHIFT))
					key=keyvalues_shift[row][bitno];
				else
					key=keyvalues_normal[row][bitno];

				if (LOG_KEYBOARD) logerror("keypress %c\n",key);
				state->m_kb_uart->receive_character(key);
			}
		}

		state->m_keyboard.keyrows[row]=keyrow;
	}
}

static TIMER_CALLBACK(keyscan_callback)
{
	scan_keyboard(machine);
}

/* i8251 serial */

const i8251_interface mbc55x_i8251a_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE(PIC8259_TAG, pic8259_ir3_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

READ8_MEMBER(mbc55x_state::mbc55x_kb_usart_r)
{
	UINT8 result = 0;
	offset>>=1;

	switch (offset)
	{
		case 0	: //logerror("%s read kb_uart\n",machine().describe_context());
				result = m_kb_uart->data_r(space,0); break;

		case 1	:	result = m_kb_uart->status_r(space,0);

				if (m_keyboard.key_special & KEY_BIT_CTRL)	// Parity error used to flag control down
					result |= I8251_STATUS_PARITY_ERROR;
				break;
	}

	return result;
}

WRITE8_MEMBER(mbc55x_state::mbc55x_kb_usart_w)
{
	offset>>=1;

	switch (offset)
	{
		case 0	: m_kb_uart->data_w(space, 0, data); break;
		case 1	: m_kb_uart->control_w(space, 0, data); break;
	}
}

static void set_ram_size(running_machine &machine)
{
	mbc55x_state	*state		= machine.driver_data<mbc55x_state>();
	address_space	&space		= *machine.device( MAINCPU_TAG)->memory().space( AS_PROGRAM );
	int 			ramsize 	= state->m_ram->size();
	int 			nobanks		= ramsize / RAM_BANK_SIZE;
	char			bank[10];
	int 			bankno;
	UINT8			*ram		= &state->m_ram->pointer()[0];
	UINT8			*map_base;
	int				bank_base;


	logerror("Ramsize is %d bytes\n",ramsize);
	logerror("RAM_BANK_SIZE=%d, nobanks=%d\n",RAM_BANK_SIZE,nobanks);

	// Main memory mapping

	for(bankno=0; bankno<RAM_BANK_COUNT; bankno++)
	{
		sprintf(bank,"bank%x",bankno);
		bank_base=bankno*RAM_BANK_SIZE;
		map_base=&ram[bank_base];

		if(bankno<nobanks)
		{
			state->membank(bank)->set_base(map_base);
			space.install_readwrite_bank(bank_base, bank_base+(RAM_BANK_SIZE-1), bank);
			logerror("Mapping bank %d at %05X to RAM\n",bankno,bank_base);
		}
		else
		{
			space.nop_readwrite(bank_base, bank_base+(RAM_BANK_SIZE-1));
			logerror("Mapping bank %d at %05X to NOP\n",bankno,bank_base);
		}
	}

	// Graphics red and blue plane memory mapping, green is in main memory
	state->membank(RED_PLANE_TAG)->set_base(&state->m_video_mem[RED_PLANE_OFFSET]);
	space.install_readwrite_bank(RED_PLANE_MEMBASE, RED_PLANE_MEMBASE+(COLOUR_PLANE_SIZE-1), RED_PLANE_TAG);
	state->membank(BLUE_PLANE_TAG)->set_base(&state->m_video_mem[BLUE_PLANE_OFFSET]);
	space.install_readwrite_bank(BLUE_PLANE_MEMBASE, BLUE_PLANE_MEMBASE+(COLOUR_PLANE_SIZE-1), BLUE_PLANE_TAG);
}

DRIVER_INIT_MEMBER(mbc55x_state,mbc55x)
{
}

void mbc55x_state::machine_reset()
{
	set_ram_size(machine());
	keyboard_reset(machine());
	machine().device(MAINCPU_TAG)->execute().set_irq_acknowledge_callback(mbc55x_irq_callback);
}

void mbc55x_state::machine_start()
{
	/* init cpu */
//  mbc55x_cpu_init(machine());


	/* setup debug commands */
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_register_command(machine(), "mbc55x_debug", CMDFLAG_NONE, 0, 0, 1, mbc55x_debug);

		/* set up the instruction hook */
		machine().device(MAINCPU_TAG)->debug()->set_instruction_hook(instruction_hook);
	}

	m_debug_machine=DEBUG_NONE;

	// Allocate keyscan timer
	m_keyboard.keyscan_timer=machine().scheduler().timer_alloc(FUNC(keyscan_callback));
}


static void mbc55x_debug(running_machine &machine, int ref, int params, const char *param[])
{
	mbc55x_state *state = machine.driver_data<mbc55x_state>();
	if(params>0)
	{
		sscanf(param[0],"%d",&state->m_debug_machine);
	}
	else
	{
		debug_console_printf(machine,"Error usage : mbc55x_debug <debuglevel>\n");
		debug_console_printf(machine,"Current debuglevel=%02X\n",state->m_debug_machine);
	}
}

/*-----------------------------------------------
    instruction_hook - per-instruction hook
-----------------------------------------------*/

static int instruction_hook(device_t &device, offs_t curpc)
{
	mbc55x_state	*state = device.machine().driver_data<mbc55x_state>();
	address_space	&space = *device.memory().space(AS_PROGRAM);
	UINT8		   *addr_ptr;

	addr_ptr = (UINT8*)space.get_read_ptr(curpc);

	if ((addr_ptr !=NULL) && (addr_ptr[0]==0xCD))
	{
//      logerror("int %02X called\n",addr_ptr[1]);

		if(DEBUG_SET(DECODE_DOS21) && (addr_ptr[1]==0x21))
			decode_dos21(&device,curpc);
	}

	return 0;
}

static void decode_dos21(device_t *device,offs_t pc)
{
	device_t *cpu = device->machine().device(MAINCPU_TAG);

	UINT16  ax = cpu->state().state_int(I8086_AX);
	UINT16  bx = cpu->state().state_int(I8086_BX);
	UINT16  cx = cpu->state().state_int(I8086_CX);
	UINT16  dx = cpu->state().state_int(I8086_DX);
	UINT16  cs = cpu->state().state_int(I8086_CS);
	UINT16  ds = cpu->state().state_int(I8086_DS);
	UINT16  es = cpu->state().state_int(I8086_ES);
	UINT16  ss = cpu->state().state_int(I8086_SS);

	UINT16  si = cpu->state().state_int(I8086_SI);
	UINT16  di = cpu->state().state_int(I8086_DI);
	UINT16  bp = cpu->state().state_int(I8086_BP);

	logerror("=======================================================================\n");
	logerror("DOS Int 0x21 call at %05X\n",pc);
	logerror("AX=%04X, BX=%04X, CX=%04X, DX=%04X\n",ax,bx,cx,dx);
	logerror("CS=%04X, DS=%04X, ES=%04X, SS=%04X\n",cs,ds,es,ss);
	logerror("SI=%04X, DI=%04X, BP=%04X\n",si,di,bp);
	logerror("=======================================================================\n");

	if((ax & 0xff00)==0x0900)
		debugger_break(device->machine());
}
