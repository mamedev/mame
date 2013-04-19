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
	dynamic_cast<i8237_device*>(device)->i8237_hlda_w( state );
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

IRQ_CALLBACK_MEMBER(pcat_base_state::irq_callback)
{
	return pic8259_acknowledge(machine().device("pic8259_1"));
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
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", i8237_device, i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_DEVREADWRITE8("kbdc", kbdc8042_device, data_r, data_w, 0xffffffff)
	AM_RANGE(0x0070, 0x007f) AM_RAM //AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8_LEGACY(dma_page_select_r,dma_page_select_w, 0xffffffff)//TODO
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8("dma8237_2", i8237_device, i8237_r, i8237_w, 0xffff)
ADDRESS_MAP_END

MACHINE_CONFIG_FRAGMENT(pcat_common)
	MCFG_PIC8259_ADD( "pic8259_1", pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", pic8259_2_config )
	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MCFG_PIT8254_ADD( "pit8254", at_pit8254_config )
//  MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )
MACHINE_CONFIG_END
