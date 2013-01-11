/***************************************************************************

    machine/pcshare.c

    Functions to emulate general aspects of the machine
    (RAM, ROM, interrupts, I/O ports)

    The information herein is heavily based on
    'Ralph Browns Interrupt List'
    Release 52, Last Change 20oct96

    TODO:
    clean up (maybe split) the different pieces of hardware
    PIC, PIT, DMA... add support for LPT, COM (almost done)
    emulation of a serial mouse on a COM port (almost done)
    support for Game Controller port at 0x0201
    support for XT harddisk (under the way, see machine/pc_hdc.c)
    whatever 'hardware' comes to mind,
    maybe SoundBlaster? EGA? VGA?

***************************************************************************/

#include "emu.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/8042kbdc.h"
#include "machine/mc146818.h"

#define VERBOSE_DBG 0       /* general debug messages */
#define DBG_LOG(N,M,A) \
	if(VERBOSE_DBG>=N){ if( M )logerror("%11.6f: %-24s",pc_keyb.machine().time().as_double(),(char*)M ); logerror A; }

#define VERBOSE_JOY 0       /* JOY (joystick port) */
#define JOY_LOG(N,M,A) \
	if(VERBOSE_JOY>=N){ if( M )logerror("%11.6f: %-24s",pc_keyb.machine().time().as_double(),(char*)M ); logerror A; }


static TIMER_CALLBACK( pc_keyb_timer );

/*
   keyboard seams to permanently sent data clocked by the mainboard
   clock line low for longer means "resync", keyboard sends 0xaa as answer
   will become automatically 0x00 after a while
*/

static struct {
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	running_machine *m_machine;
	void (*int_cb)(running_machine &, int);
	emu_timer *timer;
	UINT8 data;
	int on;
	int self_test;
} pc_keyb;



void init_pc_common(running_machine &machine, UINT32 flags, void (*set_keyb_int_func)(running_machine &, int))
{
	/* PC-XT keyboard */
	if (flags & PCCOMMON_KEYBOARD_AT)
		at_keyboard_init(machine, AT_KEYBOARD_TYPE_AT);
	else
		at_keyboard_init(machine, AT_KEYBOARD_TYPE_PC);
	at_keyboard_set_scan_code_set(1);

	memset(&pc_keyb, 0, sizeof(pc_keyb));
	pc_keyb.m_machine = &machine;
	pc_keyb.int_cb = set_keyb_int_func;
	pc_keyb.timer = machine.scheduler().timer_alloc(FUNC(pc_keyb_timer));
}

UINT8 pc_keyb_read(void)
{
	return pc_keyb.data;
}



static TIMER_CALLBACK( pc_keyb_timer )
{
	if ( pc_keyb.on ) {
		pc_keyboard();
	} else {
		/* Clock has been low for more than 5 msec, start diagnostic test */
		at_keyboard_reset(machine);
		pc_keyb.self_test = 1;
	}
}



void pc_keyb_set_clock(int on)
{
	on = on ? 1 : 0;

	if (pc_keyb.on != on)
	{
		if (!on)
			pc_keyb.timer->adjust(attotime::from_msec(5));
		else {
			if ( pc_keyb.self_test ) {
				/* The self test of the keyboard takes some time. 2 msec seems to work. */
				/* This still needs to verified against a real keyboard. */
				pc_keyb.timer->adjust(attotime::from_msec( 2 ));
			} else {
				pc_keyb.timer->reset();
				pc_keyb.self_test = 0;
			}
		}

		pc_keyb.on = on;
	}
}

void pc_keyb_clear(void)
{
	pc_keyb.data = 0;
	if ( pc_keyb.int_cb ) {
		pc_keyb.int_cb(pc_keyb.machine(), 0);
	}
}

void pc_keyboard(void)
{
	int data;

	at_keyboard_polling();

	if (pc_keyb.on)
	{
		if ( (data=at_keyboard_read())!=-1) {
			pc_keyb.data = data;
			DBG_LOG(1,"KB_scancode",("$%02x\n", pc_keyb.data));
			if ( pc_keyb.int_cb ) {
				pc_keyb.int_cb(pc_keyb.machine(), 1);
			}
			pc_keyb.self_test = 0;
		}
	}
}

/******************
DMA8237 Controller
******************/

static int dma_channel;
static UINT8 dma_offset[2][4];
static UINT8 at_pages[0x10];

static WRITE_LINE_DEVICE_HANDLER( pc_dma_hrq_changed )
{
	device->machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( device, state );
}


static READ8_HANDLER( pc_dma_read_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	return space.read_byte(page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	space.write_byte(page_offset + offset, data);
}

static READ8_HANDLER(dma_page_select_r)
{
	UINT8 data = at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


static WRITE8_HANDLER(dma_page_select_w)
{
	at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 1:
		dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}

static void set_dma_channel(device_t *device, int channel, int state)
{
	if (!state) dma_channel = channel;
}

static WRITE_LINE_DEVICE_HANDLER( pc_dack0_w ) { set_dma_channel(device, 0, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack1_w ) { set_dma_channel(device, 1, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack2_w ) { set_dma_channel(device, 2, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack3_w ) { set_dma_channel(device, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_LINE(pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_LINE(pc_dack0_w), DEVCB_LINE(pc_dack1_w), DEVCB_LINE(pc_dack2_w), DEVCB_LINE(pc_dack3_w) }
};

static I8237_INTERFACE( dma8237_2_config )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};


/******************
8259 IRQ controller
******************/

static WRITE_LINE_DEVICE_HANDLER( pic8259_1_set_int_line )
{
	device->machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

static READ8_DEVICE_HANDLER( get_slave_ack )
{
	if (offset==2) { // IRQ = 2
		return pic8259_acknowledge(space.machine().device("pic8259_2"));
	}
	return 0x00;
}

static const struct pic8259_interface pic8259_1_config =
{
	DEVCB_LINE(pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_HANDLER(get_slave_ack)
};

static const struct pic8259_interface pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

IRQ_CALLBACK(pcat_irq_callback)
{
	return pic8259_acknowledge(device->machine().device("pic8259_1"));
}

static WRITE_LINE_DEVICE_HANDLER( at_pit8254_out0_changed )
{
	if ( device->machine().device("pic8259_1") )
	{
		pic8259_ir0_w(device->machine().device("pic8259_1"), state);
	}
}


static WRITE_LINE_DEVICE_HANDLER( at_pit8254_out2_changed )
{
	//at_speaker_set_input( state ? 1 : 0 );
}


static const struct pit8253_config at_pit8254_config =
{
	{
		{
			4772720/4,              /* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_LINE(at_pit8254_out0_changed)
		}, {
			4772720/4,              /* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,              /* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_LINE(at_pit8254_out2_changed)
		}
	}
};

ADDRESS_MAP_START( pcat32_io_common, AS_IO, 32, driver_device )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8_LEGACY("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE8_LEGACY(kbdc8042_8_r, kbdc8042_8_w, 0xffffffff)
	AM_RANGE(0x0070, 0x007f) AM_RAM //AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8_LEGACY(dma_page_select_r,dma_page_select_w, 0xffffffff)//TODO
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8_LEGACY("dma8237_2", i8237_r, i8237_w, 0xffff)
ADDRESS_MAP_END

MACHINE_CONFIG_FRAGMENT(pcat_common)
	MCFG_PIC8259_ADD( "pic8259_1", pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", pic8259_2_config )
	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MCFG_PIT8254_ADD( "pit8254", at_pit8254_config )
//  MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )
MACHINE_CONFIG_END
