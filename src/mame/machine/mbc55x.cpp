// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith
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

#define DEBUG_NONE          0x0000000
#define DMA_BREAK           0x0000001
#define DECODE_BIOS         0x0000002
#define DECODE_BIOS_RAW     0x0000004
#define DECODE_DOS21        0x0000008

#define LOG_KEYBOARD        1

static void decode_dos21(device_t *device,offs_t pc);
//static void mbc55x_recalculate_ints(running_machine &machine);
static void mbc55x_debug(running_machine &machine, int ref, int params, const char *param[]);
static int instruction_hook(device_t &device, offs_t curpc);
//static void fdc_reset(running_machine &machine);
//static void set_disk_int(running_machine &machine, int state);

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
	floppy_image_device *floppy = nullptr;

	switch (data & 0x03)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	case 2: floppy = m_floppy2->get_device(); break;
	case 3: floppy = m_floppy3->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 2));
	}
}

/* Serial port USART, unimplemented as yet */

READ8_MEMBER( mbc55x_state::mbc55x_usart_r )
{
	return 0;
}

WRITE8_MEMBER( mbc55x_state::mbc55x_usart_w )
{
}

/* PIC 8259 Configuration */

READ8_MEMBER(mbc55x_state::mbcpic8259_r)
{
	return m_pic->read(space, offset>>1);
}

WRITE8_MEMBER(mbc55x_state::mbcpic8259_w)
{
	m_pic->write(space, offset>>1, data);
}

READ8_MEMBER(mbc55x_state::mbcpit8253_r)
{
	return m_pit->read(space, offset >> 1);
}

WRITE8_MEMBER(mbc55x_state::mbcpit8253_w)
{
	m_pit->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( mbc55x_state::pit8253_t2 )
{
	m_kb_uart->write_txc(state);
	m_kb_uart->write_rxc(state);
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
	return m_fdc->read(space, offset>>1);
}

WRITE8_MEMBER(mbc55x_state::mbc55x_disk_w)
{
	m_fdc->write(space, offset>>1, data);
}


/*
    Keyboard emulation

*/

void mbc55x_state::keyboard_reset()
{
	logerror("keyboard_reset()\n");

	memset(m_keyboard.keyrows,0xFF,MBC55X_KEYROWS);
	m_keyboard.key_special=0;

	// Setup timer to scan keyboard.
	m_keyboard.keyscan_timer->adjust(attotime::zero, 0, attotime::from_hz(50));
}

void mbc55x_state::scan_keyboard()
{
	UINT8   keyrow;
	UINT8   row;
	UINT8   bitno;
	UINT8   mask;

	char    key;
	static const char *const keynames[] =
	{
		"KEY0", "KEY1", "KEY2", "KEY3", "KEY4",
		"KEY5", "KEY6", "KEY7", "KEY8", "KEY9",
		"KEY10"
	};

	static const char keyvalues_normal[MBC55X_KEYROWS][8] =
	{
		{ '1',  '2', '3', '4' , '5',  '6',  '7',  '8'  },
		{ '9',  '0', '-', '=',  '\\', 'q',  'w',  'e' },
		{ 'r',  't', 'y', 'u',  'i',  'o',  'p',  '[' },
		{ ']',  'a', 's', 'd',  'f',  'g',  'h',  'j' },
		{ 'k',  'l', ';', '\'', '`',  0x0D,  'z',  'x' },
		{ 'c',  'v', 'b', 'n',  'm',  ',',  '.',  '/', },
		{ ' ',  ' ', ' ', ' ',  ' ',  ' ',  ' ',  ' ', }

	};

	static const char keyvalues_shift[MBC55X_KEYROWS][8] =
	{
		{ '!',  '@', '#', '$' , '%',  '^',  '&',  '*'  },
		{ '(',  ')', '_', '+',  '|',  'Q',  'W',  'E'  },
		{ 'R',  'T', 'Y', 'U',  'I',  'O',  'P',  '{' },
		{ '}',  'A', 'S', 'D',  'F',  'G',  'H',  'J' },
		{ 'K',  'L', ':', '"',  '~',  0x0d, 'Z',  'X' },
		{ 'C',  'V', 'B', 'N',  'M',  ',',  '?',  '/' },
		{ ' ',  ' ', ' ', ' ',  ' ',  ' ',  ' ',  ' ' }
	};

	// First read shift, control and graph

	m_keyboard.key_special = ioport(KEY_SPECIAL_TAG)->read();

	for(row=0; row<MBC55X_KEYROWS; row++)
	{
		keyrow = ioport(keynames[row])->read();

		for(mask=0x80, bitno=7;mask>0;mask=mask>>1, bitno-=1)
		{
			if(!(keyrow & mask) && (m_keyboard.keyrows[row] & mask))
			{
				if(m_keyboard.key_special & (KEY_BIT_LSHIFT | KEY_BIT_RSHIFT))
					key=keyvalues_shift[row][bitno];
				else
					key=keyvalues_normal[row][bitno];

				if (LOG_KEYBOARD) logerror("keypress %c\n",key);
				m_kb_uart->receive_character(key);
			}
		}

		m_keyboard.keyrows[row]=keyrow;
	}
}

TIMER_CALLBACK_MEMBER(mbc55x_state::keyscan_callback)
{
	scan_keyboard();
}

READ8_MEMBER(mbc55x_state::mbc55x_kb_usart_r)
{
	UINT8 result = 0;
	offset>>=1;

	switch (offset)
	{
		case 0: //logerror("%s read kb_uart\n",machine().describe_context());
			result = m_kb_uart->data_r(space,0);
			break;

		case 1:
			result = m_kb_uart->status_r(space,0);
			if (m_keyboard.key_special & KEY_BIT_CTRL)  // Parity error used to flag control down
				result |= i8251_device::I8251_STATUS_PARITY_ERROR;
			break;
	}

	return result;
}

WRITE8_MEMBER(mbc55x_state::mbc55x_kb_usart_w)
{
	offset>>=1;

	switch (offset)
	{
		case 0  : m_kb_uart->data_w(space, 0, data); break;
		case 1  : m_kb_uart->control_w(space, 0, data); break;
	}
}

void mbc55x_state::set_ram_size()
{
	address_space   &space      = m_maincpu->space( AS_PROGRAM );
	int             ramsize     = m_ram->size();
	int             nobanks     = ramsize / RAM_BANK_SIZE;
	char            bank[10];
	int             bankno;
	UINT8           *ram        = &m_ram->pointer()[0];
	UINT8           *map_base;
	int             bank_base;


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
			membank(bank)->set_base(map_base);
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
	membank(RED_PLANE_TAG)->set_base(&m_video_mem[RED_PLANE_OFFSET]);
	space.install_readwrite_bank(RED_PLANE_MEMBASE, RED_PLANE_MEMBASE+(COLOUR_PLANE_SIZE-1), RED_PLANE_TAG);
	membank(BLUE_PLANE_TAG)->set_base(&m_video_mem[BLUE_PLANE_OFFSET]);
	space.install_readwrite_bank(BLUE_PLANE_MEMBASE, BLUE_PLANE_MEMBASE+(COLOUR_PLANE_SIZE-1), BLUE_PLANE_TAG);
}

DRIVER_INIT_MEMBER(mbc55x_state,mbc55x)
{
}

void mbc55x_state::machine_reset()
{
	set_ram_size();
	keyboard_reset();
}

void mbc55x_state::machine_start()
{
	/* init cpu */
//  mbc55x_cpu_init();


	/* setup debug commands */
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_register_command(machine(), "mbc55x_debug", CMDFLAG_NONE, 0, 0, 1, mbc55x_debug);

		/* set up the instruction hook */
		m_maincpu->debug()->set_instruction_hook(instruction_hook);
	}

	m_debug_machine=DEBUG_NONE;

	// Allocate keyscan timer
	m_keyboard.keyscan_timer=machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mbc55x_state::keyscan_callback),this));
}


static void mbc55x_debug(running_machine &machine, int ref, int params, const char *param[])
{
	mbc55x_state *state = machine.driver_data<mbc55x_state>();
	if(params>0)
	{
		int temp;
		sscanf(param[0],"%d",&temp); state->m_debug_machine = temp;
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
	mbc55x_state    *state = device.machine().driver_data<mbc55x_state>();
	address_space   &space = device.memory().space(AS_PROGRAM);
	UINT8          *addr_ptr;

	addr_ptr = (UINT8*)space.get_read_ptr(curpc);

	if ((addr_ptr !=nullptr) && (addr_ptr[0]==0xCD))
	{
//      logerror("int %02X called\n",addr_ptr[1]);

		if(DEBUG_SET(DECODE_DOS21) && (addr_ptr[1]==0x21))
			decode_dos21(&device,curpc);
	}

	return 0;
}

static void decode_dos21(device_t *device,offs_t pc)
{
	mbc55x_state    *state = device->machine().driver_data<mbc55x_state>();

	UINT16  ax = state->m_maincpu->state_int(I8086_AX);
	UINT16  bx = state->m_maincpu->state_int(I8086_BX);
	UINT16  cx = state->m_maincpu->state_int(I8086_CX);
	UINT16  dx = state->m_maincpu->state_int(I8086_DX);
	UINT16  cs = state->m_maincpu->state_int(I8086_CS);
	UINT16  ds = state->m_maincpu->state_int(I8086_DS);
	UINT16  es = state->m_maincpu->state_int(I8086_ES);
	UINT16  ss = state->m_maincpu->state_int(I8086_SS);

	UINT16  si = state->m_maincpu->state_int(I8086_SI);
	UINT16  di = state->m_maincpu->state_int(I8086_DI);
	UINT16  bp = state->m_maincpu->state_int(I8086_BP);

	device->logerror("=======================================================================\n");
	device->logerror("DOS Int 0x21 call at %05X\n",pc);
	device->logerror("AX=%04X, BX=%04X, CX=%04X, DX=%04X\n",ax,bx,cx,dx);
	device->logerror("CS=%04X, DS=%04X, ES=%04X, SS=%04X\n",cs,ds,es,ss);
	device->logerror("SI=%04X, DI=%04X, BP=%04X\n",si,di,bp);
	device->logerror("=======================================================================\n");

	if((ax & 0xff00)==0x0900)
		debugger_break(device->machine());
}
