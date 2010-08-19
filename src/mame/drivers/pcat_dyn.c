/********************************************************************************************************************

Tournament Solitaire (c) 1995 Dynamo

Unmodified 486 PC-AT HW. Input uses a trackball device that isn't PC standard afaik.

Jet Way Information Co. OP495SLC motherboard
 - AMD Am486-DX40 CPU
 - Trident TVGA9000i video card

preliminary driver by Angelo Salese

TODO:
- Returns CMOS checksum error, can't enter into BIOS setup screens to set that up ... it's certainly a MESS-to-MAME
  conversion bug or a keyboard device issue, since it works fine in MESS.

********************************************************************************************************************/

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
#include "sound/beep.h"

static UINT32 *vga_vram;
static UINT8 vga_regs[0x19];

#define SET_VISIBLE_AREA(_x_,_y_) \
	{ \
	rectangle visarea; \
	visarea.min_x = 0; \
	visarea.max_x = _x_-1; \
	visarea.min_y = 0; \
	visarea.max_y = _y_-1; \
	machine->primary_screen->configure(_x_, _y_, visarea, machine->primary_screen->frame_period().attoseconds ); \
	} \

#define RES_320x200 0
#define RES_640x200 1

static VIDEO_START(pcat_dyn)
{
}



static void cga_alphanumeric_tilemap(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs,UINT8 gfx_num)
{
	static UINT32 offs,x,y,max_x,max_y;
	int tile,color;

	/*define the visible area*/
	switch(size)
	{
		case RES_320x200:
			SET_VISIBLE_AREA(320,200);
			max_x = 40;
			max_y = 25;
			break;
		case RES_640x200:
			SET_VISIBLE_AREA(640,200);
			max_x = 80;
			max_y = 25;
			break;
	}

	offs = map_offs;

	for(y=0;y<max_y;y++)
		for(x=0;x<max_x;x+=2)
		{
			tile =  (vga_vram[offs] & 0x00ff0000)>>16;
			color = (vga_vram[offs] & 0xff000000)>>24;

			drawgfx_opaque(bitmap,cliprect,machine->gfx[gfx_num],
					tile,
					color,
					0,0,
					(x+1)*8,y*8);


			tile =  (vga_vram[offs] & 0x000000ff);
			color = (vga_vram[offs] & 0x0000ff00)>>8;

			drawgfx_opaque(bitmap,cliprect,machine->gfx[gfx_num],
					tile,
					color,
					0,0,
					(x+0)*8,y*8);

			offs++;
		}
}

static VIDEO_UPDATE(pcat_dyn)
{
	cga_alphanumeric_tilemap(screen->machine,bitmap,cliprect,RES_640x200,0x10000/4,0);

	return 0;
}

static struct {
	running_device	*pit8253;
	running_device	*pic8259_1;
	running_device	*pic8259_2;
	running_device	*dma8237_1;
	running_device	*dma8237_2;
} pcat_dyn_devices;

/******************
DMA8237 Controller
******************/

static int dma_channel;
static UINT8 dma_offset[2][4];
static UINT8 at_pages[0x10];


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


static WRITE_LINE_DEVICE_HANDLER( pc_dma_hrq_changed )
{
	cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( device, state );
}


static READ8_HANDLER( pc_dma_read_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	return space->read_byte(page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	space->write_byte(page_offset + offset, data);
}

static void set_dma_channel(running_device *device, int channel, int state)
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
	cputag_set_input_line(device->machine, "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static const struct pic8259_interface pic8259_1_config =
{
	DEVCB_LINE(pic8259_1_set_int_line)
};

static const struct pic8259_interface pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w)
};

static IRQ_CALLBACK(irq_callback)
{
	int r = 0;
	r = pic8259_acknowledge(pcat_dyn_devices.pic8259_2);
	if (r==0)
	{
		r = pic8259_acknowledge(pcat_dyn_devices.pic8259_1);
	}
	return r;
}

static WRITE_LINE_DEVICE_HANDLER( at_pit8254_out0_changed )
{
	if ( pcat_dyn_devices.pic8259_1 )
	{
		pic8259_ir0_w(pcat_dyn_devices.pic8259_1, state);
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

//ce9b8
/* TODO: understand the proper ROM loading.*/
static ADDRESS_MAP_START( pcat_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM AM_BASE(&vga_vram)
	AM_RANGE(0x000c0000, 0x000c7fff) AM_RAM AM_REGION("video_bios", 0)
	AM_RANGE(0x000c8000, 0x000cffff) AM_RAM
//  AM_RANGE(0x000d0000, 0x000d7fff) AM_RAM AM_REGION("disk_bios", 0)
//  AM_RANGE(0x000d8000, 0x000dffff) AM_RAM AM_REGION("disk_bios", 0)
//  AM_RANGE(0x000e0000, 0x000effff) AM_ROM AM_REGION("game_prg", 0)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROM AM_REGION("bios", 0 )
	AM_RANGE(0x00100000, 0x001fffff) AM_RAM //AM_REGION("game_prg", 0)
	AM_RANGE(0x00200000, 0x00ffffff) AM_RAM
	AM_RANGE(0x01000000, 0x01ffffff) AM_RAM
	AM_RANGE(0xffff0000, 0xffffffff) AM_ROM AM_REGION("bios", 0 )
ADDRESS_MAP_END


static READ32_HANDLER( kludge_r )
{
	return mame_rand(space->machine);
}

/* 3c8-3c9 -> ramdac*/
static WRITE32_HANDLER( vga_ramdac_w )
{
	static int pal_offs,r,g,b,internal_pal_offs;

	if (ACCESSING_BITS_0_7)
	{
		//printf("%02x X\n",data);
		pal_offs = internal_pal_offs = data;
	}
	if (ACCESSING_BITS_8_15)
	{
		//printf("%02x\n",data);
		data>>=8;
		switch(internal_pal_offs)
		{
			case 0:
				r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				internal_pal_offs++;
				break;
			case 1:
				g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				internal_pal_offs++;
				break;
			case 2:
				b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				palette_set_color(space->machine, 0x200+pal_offs, MAKE_RGB(r, g, b));
				internal_pal_offs = 0;
				pal_offs++;
				break;
		}
	}
}

static WRITE32_HANDLER( vga_regs_w )
{
	static UINT8 vga_address;

	if (ACCESSING_BITS_0_7)
		vga_address = data;
	if (ACCESSING_BITS_8_15)
	{
		if(vga_address < 0x19)
		{
			vga_regs[vga_address] = data>>8;
			logerror("VGA reg %02x with data %02x\n",vga_address,vga_regs[vga_address]);
		}
		else
			logerror("Warning: used undefined VGA reg %02x with data %02x\n",vga_address,data>>8);
	}
}

static ADDRESS_MAP_START( pcat_io, ADDRESS_SPACE_IO, 32 )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_READWRITE(mc146818_port32le_r,     mc146818_port32le_w)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(dma_page_select_r,dma_page_select_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8("dma8237_2", i8237_r, i8237_w, 0xffff)
	AM_RANGE(0x0220, 0x022f) AM_RAM //Sound card, according to ROMs
	AM_RANGE(0x0278, 0x027f) AM_RAM //parallel port 2
	AM_RANGE(0x0378, 0x037f) AM_RAM //parallel port
	AM_RANGE(0x03c0, 0x03c3) AM_RAM
	AM_RANGE(0x03cc, 0x03cf) AM_RAM
	AM_RANGE(0x03b4, 0x03b7) AM_WRITE(vga_regs_w)
	AM_RANGE(0x03c4, 0x03c7) AM_RAM //vga regs
	AM_RANGE(0x03c8, 0x03cb) AM_WRITE(vga_ramdac_w)
	AM_RANGE(0x03b8, 0x03bb) AM_READ(kludge_r) //hv_retrace
	AM_RANGE(0x03c8, 0x03cb) AM_READ(kludge_r) //hv_retrace
	AM_RANGE(0x03d4, 0x03d7) AM_WRITE(vga_regs_w)
	AM_RANGE(0x03d8, 0x03db) AM_READ(kludge_r)
	AM_RANGE(0x03bc, 0x03bf) AM_RAM //parallel port 3
ADDRESS_MAP_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( pcat_dyn )
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

static const rgb_t defcolors[]=
{
	MAKE_RGB(0x00,0x00,0x00),
	MAKE_RGB(0x00,0x00,0xaa),
	MAKE_RGB(0x00,0xaa,0x00),
	MAKE_RGB(0x00,0xaa,0xaa),
	MAKE_RGB(0xaa,0x00,0x00),
	MAKE_RGB(0xaa,0x00,0xaa),
	MAKE_RGB(0xaa,0xaa,0x00),
	MAKE_RGB(0xaa,0xaa,0xaa),
	MAKE_RGB(0x55,0x55,0x55),
	MAKE_RGB(0x55,0x55,0xff),
	MAKE_RGB(0x55,0xff,0x55),
	MAKE_RGB(0x55,0xff,0xff),
	MAKE_RGB(0xff,0x55,0x55),
	MAKE_RGB(0xff,0x55,0xff),
	MAKE_RGB(0xff,0xff,0x55),
	MAKE_RGB(0xff,0xff,0xff)
};

static PALETTE_INIT(pcat_286)
{
	/*Note:palette colors are 6bpp...
    xxxx xx--
    */
	int ix,iy;

	for(ix=0;ix<0x300;ix++)
		palette_set_color(machine, ix,MAKE_RGB(0x00,0x00,0x00));

	//regular colors
	for(iy=0;iy<0x10;iy++)
	{
		for(ix=0;ix<0x10;ix++)
		{
			palette_set_color(machine,(ix*2)+1+(iy*0x20),defcolors[ix]);
			palette_set_color(machine,(ix*2)+0+(iy*0x20),defcolors[iy]);
		}
	}

	//bitmap mode
	for(ix=0;ix<0x10;ix++)
		palette_set_color(machine, 0x200+ix,defcolors[ix]);
	//todo: 256 colors
}

static void pcat_dyn_set_keyb_int(running_machine *machine, int state)
{
	pic8259_ir1_w(pcat_dyn_devices.pic8259_1, state);
}


static MACHINE_START( pcat_dyn )
{
//  bank = -1;
//  lastvalue = -1;
//  hv_blank = 0;
	cpu_set_irq_callback(machine->device("maincpu"), irq_callback);
	pcat_dyn_devices.pit8253 = machine->device( "pit8254" );
	pcat_dyn_devices.pic8259_1 = machine->device( "pic8259_1" );
	pcat_dyn_devices.pic8259_2 = machine->device( "pic8259_2" );
	pcat_dyn_devices.dma8237_1 = machine->device( "dma8237_1" );
	pcat_dyn_devices.dma8237_2 = machine->device( "dma8237_2" );

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, pcat_dyn_set_keyb_int);
	mc146818_init(machine, MC146818_STANDARD);
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

static GFXDECODE_START( pcat_dyn )
	GFXDECODE_ENTRY( "video_bios", 0x09cb*8, CGA_charlayout,              0, 256 )
	// there's a 8x16 charset just after the 8x8 one
GFXDECODE_END

static MACHINE_DRIVER_START( pcat_dyn )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I486, 40000000)	/* Am486 DX-40 */
	MDRV_CPU_PROGRAM_MAP(pcat_map)
	MDRV_CPU_IO_MAP(pcat_io)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MDRV_GFXDECODE( pcat_dyn )

	MDRV_MACHINE_START(pcat_dyn)
	MDRV_NVRAM_HANDLER( mc146818 )

//  MDRV_IMPORT_FROM( at_kbdc8042 )
	MDRV_PIC8259_ADD( "pic8259_1", pic8259_1_config )
	MDRV_PIC8259_ADD( "pic8259_2", pic8259_2_config )
	MDRV_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MDRV_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MDRV_PIT8254_ADD( "pit8254", at_pit8254_config )

	MDRV_PALETTE_INIT(pcat_286)

	MDRV_PALETTE_LENGTH(0x300)

	MDRV_VIDEO_START(pcat_dyn)
	MDRV_VIDEO_UPDATE(pcat_dyn)
MACHINE_DRIVER_END

/***************************************
*
* ROM definitions
*
***************************************/

ROM_START(toursol)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* Motherboard BIOS */
	ROM_LOAD("prom.mb", 0x000000, 0x10000, CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	ROM_REGION(0x20000, "video_bios", 0)	/* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(				0x00001, 0x04000 )

	ROM_REGION32_LE(0x100000, "game_prg", 0)	/* PromStor 32, mapping unknown */
	ROM_LOAD("sol.u21", 0x00000, 0x40000, CRC(e97724d9) SHA1(995b89d129c371b815c6b498093bd1bbf9fd8755))
	ROM_LOAD("sol.u22", 0x40000, 0x40000, CRC(69d42f50) SHA1(737fe62f3827b00b4f6f3b72ef6c7b6740947e95))
	ROM_LOAD("sol.u23", 0x80000, 0x40000, CRC(d1e39bd4) SHA1(39c7ee43cddb53fba0f7c0572ddc40289c4edd07))
	ROM_LOAD("sol.u24", 0xa0000, 0x40000, CRC(555341e0) SHA1(81fee576728855e234ff7aae06f54ae9705c3ab5))
	ROM_LOAD("sol.u28", 0xe0000, 0x02000, CRC(c9374d50) SHA1(49173bc69f70bb2a7e8af9d03e2538b34aa881d8))
ROM_END


ROM_START(toursol1)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* Motherboard BIOS */
	ROM_LOAD("prom.mb", 0x000000, 0x10000, CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	ROM_REGION(0x20000, "video_bios", 0)	/* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(				0x00001, 0x04000 )

	ROM_REGION32_LE(0x100000, "game_prg", 0)	/* PromStor 32, mapping unknown */
	ROM_LOAD("prom.0", 0x00000, 0x40000, CRC(f26ce73f) SHA1(5516c31aa18716a47f46e412fc273ae8784d2061))
	ROM_LOAD("prom.1", 0x40000, 0x40000, CRC(8f96e2a8) SHA1(bc3ce8b99e6ff40e355df2c3f797f1fe88b3b219))
	ROM_LOAD("prom.2", 0x80000, 0x40000, CRC(8b0ac5cf) SHA1(1c2b6a53c9ff4d18a5227d899facbbc719f40205))
	ROM_LOAD("prom.3", 0xa0000, 0x40000, CRC(9352e965) SHA1(2bfb647ec27c60a8c821fdf7483199e1a444cea8))
	ROM_LOAD("prom.7", 0xe0000, 0x02000, CRC(154c8092) SHA1(4439ee82f36d5d5c334494ba7bb4848e839213a7))
ROM_END

GAME( 1995, toursol,  0,       pcat_dyn, pcat_dyn, 0, ROT0, "Dynamo", "Tournament Solitaire (V1.06, 08/03/95)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, toursol1, toursol, pcat_dyn, pcat_dyn, 0, ROT0, "Dynamo", "Tournament Solitaire (V1.04, 06/22/95)", GAME_NOT_WORKING|GAME_NO_SOUND )

