/********************************************************************************************************************

Street Games (c) 1993 New Image Technologies

A modified 386 PC-AT HW

preliminary driver by Angelo Salese

=====================================================================================================================

These games were made by New Image Technologies. They are touch screen multi games from around 1993.
Guru dumped one of their games a year ago but the pcb was rusted out and I think the roms might
be bad, not sure.

nvram is a potential problem. I have them separated into their own folder for now. I'm not really s
ure if they are needed, but I found this inside a Bonanza rom so they may hold a key:

-------------------------------------------
INVALID KEY
_
Programming DS1204
_
Successfully Programmed DS1204
_
Failed Programming DS1204
_
Verifying DS1204
_
Failed to read secure data
_
Failed to read ID DS1204
_
Verified DS1204
-------------------------------------------

I have them (the real chips) for my Bonanza rom boards, but they are missing on my Street Games boards.
However, the back of one of the manuals mentions game swapping on these boards, and it doesn't mention
this chip as needing to be changed. I included the nvram dump from Guru's Street Games II dump, so maybe
that will work on all the Street Games sets? I don't know. My Dallas chip read good as 2764 and verified
ok. The other one (bq4010yma-150) reads the same crc most reads but won't pass the verify function.
It may still be good. Those are for the Bonanza sets as you'll see in the pics.

I made copies of the manuals for Street Games and Bonanza and included lots of pics.

From the looks of the boards I got, Street Games II looks like it uses the 3 original Street Games roms
and just builds on them. You'll see what I mean in the pics.

I don't know if all 3 vga bioses are needed or not, one is soldered and not dumped, I dumped the other two.
I can get the soldered one dumped in the future if it turns out we need it. When these games got upgraded,
they sometimes came with a new video board. Switching from Street Games to Street Games II probably didn't need
it but switching to Bonanza did. The vga bioses I dumped came off of the 2 different video boards for Bonanza.
I dumped them as 27c256. The one on the Street Games video board is the surface mounted one.

The end numbers on the rom stickers give away the version info. For example, it looks like Guru's old dump
of Street Games II is revision 7C.

The system bios roms I dumped give an error in romcmp but they seem to verify ok so I think they are good.

My 2 rom boards for Bonanza were fortunately 2 different revisions. I ended up with 2 sets of Street Games,
1 set of Street Games II, and 2 different sets of Bonanza. A second set of Street Games II might be able to be
"created" if his dump is good because you will notice in the pics, it looks like Street Games II used 3 roms
from Street Games I. That or one of my rom boards came populated incorrectly.

The ATI keyboard bios is an HT6542. The Jet one I'm not sure. I don't think they can be read. Same for the M5818's.

My set naming logic...

bonanza = Bonanza (Rev 3)
bonanza2 = Bonanza (Rev 2)

streetg = Street Games (Rev 4)
streetg2 = Street Games (Rev 2)

streetgii = Street Games II (Rev 5)

Anything else need to be dumped? More pics needed, etc.? Contact me.


Smitdogg

********************************************************************************************************************/

#include "driver.h"
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

static UINT32 *vga_vram;
static UINT8 vga_regs[0x19];

#define SET_VISIBLE_AREA(_x_,_y_) \
	{ \
	rectangle visarea = *video_screen_get_visible_area(machine->primary_screen); \
	visarea.min_x = 0; \
	visarea.max_x = _x_-1; \
	visarea.min_y = 0; \
	visarea.max_y = _y_-1; \
	video_screen_configure(machine->primary_screen, _x_, _y_, &visarea, video_screen_get_frame_period(machine->primary_screen).attoseconds ); \
	} \

#define RES_320x200 0
#define RES_640x200 1

static VIDEO_START(streetg2)
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

static VIDEO_UPDATE(streetg2)
{
	cga_alphanumeric_tilemap(screen->machine,bitmap,cliprect,RES_640x200,0x18000/4,0);

	return 0;
}

static struct {
	const device_config	*pit8253;
	const device_config	*pic8259_1;
	const device_config	*pic8259_2;
	const device_config	*dma8237_1;
	const device_config	*dma8237_2;
} streetg2_devices;

/******************
DMA8237 Controller
******************/

static int dma_channel;
static UINT8 dma_offset[2][4];
static UINT8 at_pages[0x10];

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

	return memory_read_byte(space, page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	memory_write_byte(space, page_offset + offset, data);
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

static void set_dma_channel(const device_config *device, int channel, int state)
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

static PIC8259_SET_INT_LINE( pic8259_1_set_int_line )
{
	cputag_set_input_line(device->machine, "maincpu", 0, interrupt ? HOLD_LINE : CLEAR_LINE);
}

static const struct pic8259_interface pic8259_1_config = {
	pic8259_1_set_int_line
};

static PIC8259_SET_INT_LINE( pic8259_2_set_int_line )
{
	pic8259_set_irq_line( streetg2_devices.pic8259_1, 2, interrupt);
}

static const struct pic8259_interface pic8259_2_config = {
	pic8259_2_set_int_line
};

static IRQ_CALLBACK(irq_callback)
{
	int r = 0;
	r = pic8259_acknowledge(streetg2_devices.pic8259_2);
	if (r==0)
	{
		r = pic8259_acknowledge(streetg2_devices.pic8259_1);
	}
	return r;
}

static PIT8253_OUTPUT_CHANGED( at_pit8254_out0_changed )
{
	if ( streetg2_devices.pic8259_1 )
	{
		pic8259_set_irq_line(streetg2_devices.pic8259_1, 0, state);
	}
}


static PIT8253_OUTPUT_CHANGED( at_pit8254_out2_changed )
{
	//at_speaker_set_input( state ? 1 : 0 );
}


static const struct pit8253_config at_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			at_pit8254_out0_changed
		}, {
			4772720/4,				/* dram refresh */
			NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			at_pit8254_out2_changed
		}
	}
};

//ce9b8
/* TODO: understand the proper ROM loading.*/
static ADDRESS_MAP_START( pcat_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM AM_BASE(&vga_vram)
	AM_RANGE(0x000c0000, 0x000c7fff) AM_RAM
	AM_RANGE(0x000c8000, 0x000cffff) AM_RAM AM_REGION("video_bios", 0)
	AM_RANGE(0x000d0000, 0x000d7fff) AM_RAM AM_REGION("disk_bios", 0)
	AM_RANGE(0x000d8000, 0x000dffff) AM_RAM AM_REGION("disk_bios", 0)
	AM_RANGE(0x000e0000, 0x000effff) AM_ROM AM_REGION("game_prg", 0)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROM AM_REGION("bios", 0 )
	AM_RANGE(0x00100000, 0x0027ffff) AM_ROM AM_REGION("game_prg", 0)
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
	AM_RANGE(0x0070, 0x007f) AM_RAM//READWRITE(mc146818_port32le_r,     mc146818_port32le_w)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(dma_page_select_r,dma_page_select_w, 0xffffffff)//TODO
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8("dma8237_2", i8237_r, i8237_w, 0xffff)
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
	AM_RANGE(0x03d8, 0x03db) AM_RAM
	AM_RANGE(0x03bc, 0x03bf) AM_RAM //parallel port 3
ADDRESS_MAP_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( pcat_nit )
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
	AT_KEYB_HELPER( 0x4000, "Del",       		    KEYCODE_A           ) /* Delete                      6f  ef */

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

static void streetg2_set_keyb_int(running_machine *machine, int state)
{
	pic8259_set_irq_line(streetg2_devices.pic8259_1, 1, state);
}


static MACHINE_START( streetg2 )
{
//  bank = -1;
//  lastvalue = -1;
//  hv_blank = 0;
	cpu_set_irq_callback(cputag_get_cpu(machine, "maincpu"), irq_callback);
	streetg2_devices.pit8253 = devtag_get_device( machine, "pit8254" );
	streetg2_devices.pic8259_1 = devtag_get_device( machine, "pic8259_1" );
	streetg2_devices.pic8259_2 = devtag_get_device( machine, "pic8259_2" );
	streetg2_devices.dma8237_1 = devtag_get_device( machine, "dma8237_1" );
	streetg2_devices.dma8237_2 = devtag_get_device( machine, "dma8237_2" );

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, streetg2_set_keyb_int);
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

static GFXDECODE_START( streetg2 )
	GFXDECODE_ENTRY( "video_bios", 0x2da2, CGA_charlayout,              0, 256 )
	//there's also a 8x16 entry (just after the 8x8)
GFXDECODE_END

static GFXDECODE_START( bonanza )
	GFXDECODE_ENTRY( "video_bios", 0x56f3, CGA_charlayout,              0, 256 )
	//there's also a 8x16 entry (just after the 8x8)
GFXDECODE_END

static MACHINE_DRIVER_START( pcat_nit )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I386, 14318180*2)	/* I386 ?? Mhz */
	MDRV_CPU_PROGRAM_MAP(pcat_map)
	MDRV_CPU_IO_MAP(pcat_io)

 	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MDRV_GFXDECODE( streetg2 )

	MDRV_MACHINE_START(streetg2)
	MDRV_NVRAM_HANDLER( mc146818 )

//  MDRV_IMPORT_FROM( at_kbdc8042 )
	MDRV_PIC8259_ADD( "pic8259_1", pic8259_1_config )
	MDRV_PIC8259_ADD( "pic8259_2", pic8259_2_config )
	MDRV_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MDRV_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MDRV_PIT8254_ADD( "pit8254", at_pit8254_config )

	MDRV_PALETTE_INIT(pcat_286)

	MDRV_PALETTE_LENGTH(0x300)

	MDRV_VIDEO_START(streetg2)
	MDRV_VIDEO_UPDATE(streetg2)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( pcat_nitB )
	MDRV_IMPORT_FROM( pcat_nit )
	MDRV_GFXDECODE( bonanza )
MACHINE_DRIVER_END

/***************************************
*
* ROM definitions
*
***************************************/

ROM_START(streetg)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("system-bios-10-0004-01.u6", 0x00000, 0x10000, CRC(e4d6511f) SHA1(d432743f549fa6ecc04bc5bf94999253f86af08c) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x180000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-0003-04.u11", 0x000000,0x80000, CRC(1471a728) SHA1(5e12a9230f8130282a1be9a10118a3556bafbc37) )
	ROM_LOAD("10-0003-04.u12", 0x080000,0x80000, CRC(5a50f519) SHA1(c07a583b4220d4d5506824def91774fede760e65) )
	ROM_LOAD("10-0003-04.u13", 0x100000,0x80000, CRC(8a609145) SHA1(18fcb58b461aa9149a163b85dd8267dec90da3cd) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("disk-bios-10-0001-04.u10",     0x00000, 0x08000, CRC(1b4ce068) SHA1(8570b36acf3eb29f1c59e56a4dad6d38c218748f) )

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("8k_nvram.u9",     0x00000, 0x02000, CRC(44be0b89) SHA1(81666dd369d1d85269833293136d61ffe80e940a))
ROM_END

ROM_START(streetgr3)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("system-bios-10-0004-01.u6", 0x00000, 0x10000, CRC(e4d6511f) SHA1(d432743f549fa6ecc04bc5bf94999253f86af08c) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x180000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-00003-03.u11", 0x000000,0x80000, CRC(2fbcb12b) SHA1(f6413565fc1289ba32b411de877cdf6526f1fa9d) )
	ROM_LOAD("10-00003-03.u12", 0x080000,0x80000, CRC(b37c7dff) SHA1(cf6318bfeca0bd272734f45c7589a0224863b0f1) )
	ROM_LOAD("10-00003-03.u13", 0x100000,0x80000, CRC(6a9d0771) SHA1(6cd9a56a2413416d0928e5cf9340c94bc0c87c46) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("disk-bios-10-0001-04.u10",     0x00000, 0x08000, CRC(1b4ce068) SHA1(8570b36acf3eb29f1c59e56a4dad6d38c218748f) )

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("8k_nvram.u9",     0x00000, 0x02000, CRC(44be0b89) SHA1(81666dd369d1d85269833293136d61ffe80e940a))
ROM_END

ROM_START(bonanza)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("system-bios-sx-10-0004-02.u6", 0x00000, 0x10000, CRC(fa545ba8) SHA1(db64548bd87262cd2e82175a1b66f168b5ae072d) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("techyosd-isa-bios-v1.2.u8",     0x00000, 0x04000, CRC(6adf7e71) SHA1(2b07d964cc7c2c0aa560625b7c12f38d4537d652) )
	ROM_CONTINUE(                                    0x00001, 0x04000 )

	ROM_REGION(0x300000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-0018-03-090894.u11", 0x000000,0x80000, CRC(32b6c8bc) SHA1(7f4097990dca268915842d4253d4257654de2cfc) )
	ROM_LOAD("10-0018-03-090894.u12", 0x080000,0x80000, CRC(d7cb191d) SHA1(2047f3668b0e41ad5347107f4e3446c0374c5bb7) )
	ROM_LOAD("10-0018-03-090894.u13", 0x100000,0x80000, CRC(1d3ddeaa) SHA1(8e73fe535882f6d634668733e550281e727fbdbc) )
	ROM_LOAD("10-0018-03-090894.u15", 0x180000,0x80000, CRC(b9b3f442) SHA1(6ea5ce3eb007b95ad3350fdb634625b151ae7bdb) )
	ROM_LOAD("10-0018-03-090894.u16", 0x200000,0x80000, CRC(5b0dd6f5) SHA1(8172118185179ecb7d3f958480186bf9c906785f) )
	ROM_LOAD("10-0018-03-090894.u17", 0x280000,0x80000, CRC(b637eb58) SHA1(7c4615f58118d9b82575d816ef916fccbb1be0f9) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("disk-bios-10-0001-04.u10",     0x00000, 0x08000, CRC(1b4ce068) SHA1(8570b36acf3eb29f1c59e56a4dad6d38c218748f) )

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("bq4010yma-150.u9",     0x00000, 0x02000, CRC(f4ca28ee) SHA1(17b852028568fb814df62f5870b91a8303302b55))
ROM_END

ROM_START(bonanzar2)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("system-bios-sx-10-0004-02.u6", 0x00000, 0x10000, CRC(fa545ba8) SHA1(db64548bd87262cd2e82175a1b66f168b5ae072d) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("techyosd-isa-bios-v1.2.u8",     0x00000, 0x04000, CRC(6adf7e71) SHA1(2b07d964cc7c2c0aa560625b7c12f38d4537d652) )
	ROM_CONTINUE(                                    0x00001, 0x04000 )

	ROM_REGION(0x300000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-0018-02-081794.u11", 0x000000,0x80000, CRC(f87fa935) SHA1(b06144496406231aa63149ae12a048ffab8f77d0) )
	ROM_LOAD("10-0018-02-081794.u12", 0x080000,0x80000, CRC(bd892e3e) SHA1(1b9174fe2a6eaa7687b543798099b86b9039c049) )
	ROM_LOAD("10-0018-02-081794.u13", 0x100000,0x80000, CRC(626d999e) SHA1(5c27e3b064b0235c0d6e0be8d8f78538a11647a2) )
	ROM_LOAD("10-0018-02-081794.u15", 0x180000,0x80000, CRC(3b28f582) SHA1(3da61fbd92e6cc60e00eaa21d8fb04aa78cce663) )
	ROM_LOAD("10-0018-02-081794.u16", 0x200000,0x80000, CRC(fe29ad76) SHA1(64aaae639f024c50c09fe920bc92e6d45ced5648) )
	ROM_LOAD("10-0018-02-081794.u17", 0x280000,0x80000, CRC(066108fe) SHA1(ef837422a2a81f5ac3375b6ed68f20143ac6caec) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("disk-bios-10-0001-04.u10",     0x00000, 0x08000, CRC(1b4ce068) SHA1(8570b36acf3eb29f1c59e56a4dad6d38c218748f) )

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("bq4010yma-150.u9",     0x00000, 0x02000, CRC(f4ca28ee) SHA1(17b852028568fb814df62f5870b91a8303302b55))
ROM_END

ROM_START(streetg2)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("10-0004-01_mb-bios.bin", 0x00000, 0x10000, CRC(e4d6511f) SHA1(d432743f549fa6ecc04bc5bf94999253f86af08c) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x180000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-0007-07c_083194_rom4.u11", 0x000000,0x80000, CRC(244c2bfa) SHA1(4f2f0fb6923b4e3f1ab4e607e29a27fb15b39fac) )
	ROM_LOAD("10-0007-07c_083194_rom5.u12", 0x080000,0x80000, CRC(c89d5dca) SHA1(212bcbf7a39243f4524b4a855fbedabd387d17f2) )
	ROM_LOAD("10-0007-07c_083194_rom6.u13", 0x100000,0x80000, CRC(6264f65f) SHA1(919a8e5d9861dc642ac0f0885faed544bbafa321) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("10-0001-03_disk_bios.u10",     0x00000, 0x08000, CRC(d6ba8b37) SHA1(1d1d984bc15fd154fc07dcfa2132bd44636d7bf1))

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("8k_nvram.u9",     0x00000, 0x02000, CRC(44be0b89) SHA1(81666dd369d1d85269833293136d61ffe80e940a))
ROM_END

ROM_START(streetg2r5)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("10-0004-01_mb-bios.bin", 0x00000, 0x10000, CRC(e4d6511f) SHA1(d432743f549fa6ecc04bc5bf94999253f86af08c) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x180000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-00007-05-032194.u15", 0x000000,0x80000, CRC(cefa230f) SHA1(91fd30a3def381974fae0edb4d42d452acda19bb) )
	ROM_LOAD("10-00007-05-032194.u16", 0x080000,0x80000, CRC(0be5dd19) SHA1(d0474ff5156e1fa8b4edb502c49b7e1a2b3f6169) )
	ROM_LOAD("10-00007-05-032194.u17", 0x100000,0x80000, CRC(f6c996b9) SHA1(871a8d093b856511a0e2b03334ef5c66a2482622) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("10-0001-03_disk_bios.u10",     0x00000, 0x08000, CRC(d6ba8b37) SHA1(1d1d984bc15fd154fc07dcfa2132bd44636d7bf1))

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("8k_nvram.u9",     0x00000, 0x02000, CRC(44be0b89) SHA1(81666dd369d1d85269833293136d61ffe80e940a))
ROM_END

GAME( 1993, bonanza,    0,		   pcat_nitB, pcat_nit, 0, ROT0, "New Image Technologies",  "Bonanza (Revision 3)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, bonanzar2,  bonanza,   pcat_nitB, pcat_nit, 0, ROT0, "New Image Technologies",  "Bonanza (Revision 2)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, streetg,    0,		   pcat_nit,  pcat_nit, 0, ROT0, "New Image Technologies",  "Street Games (Revision 4)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, streetgr3,  streetg,   pcat_nit,  pcat_nit, 0, ROT0, "New Image Technologies",  "Street Games (Revision 3)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, streetg2,   0,		   pcat_nit,  pcat_nit, 0, ROT0, "New Image Technologies",  "Street Games II (Revision 7C)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, streetg2r5, streetg2,  pcat_nit,  pcat_nit, 0, ROT0, "New Image Technologies",  "Street Games II (Revision 5)", GAME_NOT_WORKING|GAME_NO_SOUND )
