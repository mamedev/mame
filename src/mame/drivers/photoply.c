/*******************************************************************************************************

Photo Play (c) 199? Funworld

Preliminary driver by Angelo Salese

TODO:
- Puts a FDC error, needs a DASM investigation / work-around.

*******************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "memconv.h"
#include "devconv.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/pci.h"
#include "machine/8042kbdc.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"

class photoply_state : public driver_device
{
public:
	photoply_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT8 m_vga_address;

	device_t	*m_pit8253;
	device_t	*m_pic8259_1;
	device_t	*m_pic8259_2;
	device_t	*m_dma8237_1;
	device_t	*m_dma8237_2;
};



/******************
DMA8237 Controller
******************/


static WRITE_LINE_DEVICE_HANDLER( pc_dma_hrq_changed )
{
	cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( device, state );
}


static READ8_HANDLER( pc_dma_read_byte )
{
	photoply_state *state = space->machine().driver_data<photoply_state>();
	offs_t page_offset = (((offs_t) state->m_dma_offset[0][state->m_dma_channel]) << 16)
		& 0xFF0000;

	return space->read_byte(page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	photoply_state *state = space->machine().driver_data<photoply_state>();
	offs_t page_offset = (((offs_t) state->m_dma_offset[0][state->m_dma_channel]) << 16)
		& 0xFF0000;

	space->write_byte(page_offset + offset, data);
}

static READ8_HANDLER(dma_page_select_r)
{
	photoply_state *state = space->machine().driver_data<photoply_state>();
	UINT8 data = state->m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = state->m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = state->m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = state->m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = state->m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


static WRITE8_HANDLER(dma_page_select_w)
{
	photoply_state *state = space->machine().driver_data<photoply_state>();
	state->m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 1:
		state->m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		state->m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		state->m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		state->m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}

static void set_dma_channel(device_t *device, int channel, int state)
{
	photoply_state *drvstate = device->machine().driver_data<photoply_state>();
	if (!state) drvstate->m_dma_channel = channel;
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
	cputag_set_input_line(device->machine(), "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static READ8_DEVICE_HANDLER( get_slave_ack )
{
	photoply_state *state = device->machine().driver_data<photoply_state>();
	if (offset==2) { // IRQ = 2
		return pic8259_acknowledge(state->m_pic8259_2);
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

static IRQ_CALLBACK(irq_callback)
{
	photoply_state *state = device->machine().driver_data<photoply_state>();
	return pic8259_acknowledge(state->m_pic8259_1);
}

static WRITE_LINE_DEVICE_HANDLER( at_pit8254_out0_changed )
{
	photoply_state *drvstate = device->machine().driver_data<photoply_state>();
	if ( drvstate->m_pic8259_1 )
	{
		pic8259_ir0_w(drvstate->m_pic8259_1, state);
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
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_LINE(at_pit8254_out0_changed)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_LINE(at_pit8254_out2_changed)
		}
	}
};

static ADDRESS_MAP_START( photoply_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM // VGA RAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_RAM AM_REGION("video_bios", 0) //???
	AM_RANGE(0x000c8000, 0x000cffff) AM_RAM AM_REGION("video_bios", 0)
	AM_RANGE(0x000d0000, 0x000dffff) AM_RAM AM_REGION("ex_bios", 0)
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END



static ADDRESS_MAP_START( photoply_io, AS_IO, 32 )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_RAM//DEVREADWRITE8_MODERN("rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(dma_page_select_r,dma_page_select_w, 0xffffffff)//TODO
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8("dma8237_2", i8237_r, i8237_w, 0xffff)
	AM_RANGE(0x0278, 0x027f) AM_RAM //parallel port 2
	AM_RANGE(0x0378, 0x037f) AM_RAM //parallel port
	AM_RANGE(0x03bc, 0x03bf) AM_RAM //parallel port 3
//	AM_RANGE(0x03f4, 0x03f7) AM_READ(kludge_r) // fdc
ADDRESS_MAP_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( photoply )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED ) 	/* unused scancode 0 */
	AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */

	PORT_START("pc_keyboard_1")
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */

	PORT_START("pc_keyboard_2")

	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */

	PORT_START("pc_keyboard_4")

	PORT_START("pc_keyboard_5")

	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",		KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",			KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",		KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",	KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",		KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",		KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",      		    KEYCODE_A           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")
INPUT_PORTS_END


static void photoply_set_keyb_int(running_machine &machine, int state)
{
	photoply_state *drvstate = machine.driver_data<photoply_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}


static MACHINE_START( photoply )
{
	photoply_state *state = machine.driver_data<photoply_state>();
	device_set_irq_callback(machine.device("maincpu"), irq_callback);
	state->m_pit8253 = machine.device( "pit8254" );
	state->m_pic8259_1 = machine.device( "pic8259_1" );
	state->m_pic8259_2 = machine.device( "pic8259_2" );
	state->m_dma8237_1 = machine.device( "dma8237_1" );
	state->m_dma8237_2 = machine.device( "dma8237_2" );

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, photoply_set_keyb_int);
}

static const gfx_layout CGA_charlayout =
{
	8,8,
    256,
    1,
    { 0 },
    { 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
    8*8
};

static GFXDECODE_START( photoply )
	GFXDECODE_ENTRY( "video_bios", 0x6000+0xa5*8+7, CGA_charlayout,              0, 256 )
	//there's also a 8x16 entry (just after the 8x8)
GFXDECODE_END

static READ8_HANDLER( vga_setting ) { return 0xff; } // hard-code to color

static const struct pc_vga_interface vga_interface =
{
	NULL,
	NULL,
	vga_setting,
	AS_PROGRAM,
	0xa0000,
	AS_IO,
	0x0000
};



static MACHINE_CONFIG_START( photoply, photoply_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I486, 75000000)	/* I486DX4, 75 or 100 Mhz */
	MCFG_CPU_PROGRAM_MAP(photoply_map)
	MCFG_CPU_IO_MAP(photoply_io)

	MCFG_GFXDECODE( photoply )

	MCFG_MACHINE_START(photoply)
	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

//  MCFG_FRAGMENT_ADD( at_kbdc8042 )
	MCFG_PIC8259_ADD( "pic8259_1", pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", pic8259_2_config )
	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MCFG_PIT8254_ADD( "pit8254", at_pit8254_config )

	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END


ROM_START(photoply)
	ROM_REGION(0x20000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("award bootblock bios v1.0.bin", 0x000000, 0x20000, CRC(e96d1bbc) SHA1(64d0726c4e9ecee8fddf4cc39d92aecaa8184d5c) )

	ROM_REGION(0x10000, "ex_bios", 0) /* multifunction board with a ESS AudioDrive chip,  M27128A */
	ROM_LOAD("enhanced bios.bin", 0x000000, 0x4000, CRC(a216404e) SHA1(c9067cf87d5c8106de00866bb211eae3a6c02c65) )
	ROM_RELOAD(                   0x004000, 0x4000 )
	ROM_RELOAD(                   0x008000, 0x4000 )
	ROM_RELOAD(                   0x00c000, 0x4000 )

	ROM_REGION(0x8000, "video_bios", 0 )
	ROM_LOAD("vga.bin", 0x000000, 0x8000, CRC(7a859659) SHA1(ff667218261969c48082ec12aa91088a01b0cb2a) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "photoply", 0,NO_DUMP )
ROM_END

static DRIVER_INIT( photoply )
{
	pc_vga_init(machine, &vga_interface, NULL);
}

GAME( 199?, photoply,  0,   photoply, photoply, photoply, ROT0, "Funworld", "PhotoPlay", GAME_NOT_WORKING|GAME_NO_SOUND )
