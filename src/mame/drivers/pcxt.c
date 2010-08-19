/******************************************************************************************

PC-XT (c) 1987 IBM

(Actually Arcade games running on more or less modified PC-XT HW)

driver by Angelo Salese & Chris Hardy
original tetriunk.c by David Haywood & Tomasz Slanina

Notes:
- The Korean Tetris is a blantant rip-off of the Mirrorsoft/Andromeda Software Tetris PC
  version;

TODO:
- 02851: tetriskr: Corrupt game graphics after some time of gameplay, caused by a wrong
  reading of the i/o $3c8 bit 1.
- EGA/CGA/VGA emulation uses the bare minimum for these games,there are still a lot of
  features that needs to be added;
- Filetto: Add a proper FDC device,if you enable it will give a boot error,probably because it
  expects that in the floppy drive shouldn't be anything,there's currently a kludge that
  does the trick;
- Filetto: Add sound,"buzzer" PC sound plus the UM5100 sound chip ,might be connected to the
  prototyping card;
- Korean Tetris: Add the aforementioned "buzzer" plus identify if there's any kind of sound
  chip on it;

********************************************************************************************
Filetto HW notes:
The PCB is a un-modified IBM-PC with a CGA adapter & a prototyping card that controls the
interface between the pc and the Jamma connectors.Additionally there's also a UM5100 sound
chip for the sound.
PCB Part Number: S/N 90289764 NOVARXT
PCB Contents:
1x UMC 8923S-UM5100 voice processor (upper board)
1x MMI PAL16L8ACN-940CRK9 (upper board)
1x AMD AMPAL16R8APC-8804DM (upper board)
1x AMD P8088-1 main processor 8.000MHz (lower board)
1x Proton PT8010AF PLCC 28.636MHz (lower board)
1x UMC 8928LP-UM8272A floppy disk controller (lower board)
1x UMC 8935CS-UM82C11 Printer Adapter Interface (lower board)
1x UMC 8936CS-UM8250B Programmable asynchronous communications element (lower board)
There isn't any keyboard found connected to the pcb.
********************************************************************************************
Filetto SW notes:
The software of this game can be extracted with a normal Windows program extractor.
The files names are:
-command.com  (1)
-ibmbio.com   (1)
-ibmdos.com   (1)
-ansi.sys     (1)
-config.sys   (2)
-autoexec.bat (3)
-x.exe        (4)
(1)This is an old Italian version of MS-DOS (v3.30 18th March 1987).
(2)Contains "device=ansi.sys",it's an hook-up for the graphics used by the BIOS.
(3)It has an Echo off (as you can notice from the game itself) and then the loading of the
main program (x.exe).
(4)The main program,done in plain Basic with several Italian comments in it.The date of
the main program is 9th October 1990.

******************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pit8253.h"
#include "machine/8255ppi.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "sound/hc55516.h"
#include "sound/beep.h"

#define SET_VISIBLE_AREA(_x_,_y_) \
	{ \
	rectangle visarea; \
	visarea.min_x = 0; \
	visarea.max_x = _x_-1; \
	visarea.min_y = 0; \
	visarea.max_y = _y_-1; \
	machine->primary_screen->configure(_x_, _y_, visarea, machine->primary_screen->frame_period().attoseconds ); \
	} \


static UINT8 *vga_vram,*work_ram;
static UINT8 video_regs[0x19];
static UINT8 *vga_mode;
static UINT8 hv_blank;
static UINT8 *vga_bg_bank;

static int bank;
static int lastvalue;

/*Add here Video regs defines...*/


#define RES_320x200 0
#define RES_640x200 1

static void cga_alphanumeric_tilemap(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs,UINT8 gfx_num);

static VIDEO_START( filetto )
{
}

static VIDEO_START( tetriskr )
{
}

static READ8_HANDLER( vga_hvretrace_r )
{
	/*
    Read-Only lower 8 bits,TRUSTED
    ---- x--- Vertical Retrace
    ---- ---x Horizontal Retrace
    */
	static UINT8 res;
	static int h,w;
	res = 0;
	h = space->machine->primary_screen->height();
	w = space->machine->primary_screen->width();

//  popmessage("%d %d",h,w);

	if (space->machine->primary_screen->hpos() > h)
		res|= 1;

	if (space->machine->primary_screen->vpos() > w)
		res|= 8;

	return res;
}

/*Basic Graphic mode */
/*TODO: non-black colours should use the bright versions*/
static void cga_graphic_bitmap(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs)
{
	static UINT16 x,y;
	static UINT32 offs;

	SET_VISIBLE_AREA(320,200);
	offs = map_offs;
	for(y=0;y<200;y+=2)
		for(x=0;x<320;x+=4)
		{
			*BITMAP_ADDR16(bitmap, y, x+0) = machine->pens[0x200+(((vga_vram[offs] & 0xc0)>>6)<<1)];
			*BITMAP_ADDR16(bitmap, y, x+1) = machine->pens[0x200+(((vga_vram[offs] & 0x30)>>4)<<1)];
			*BITMAP_ADDR16(bitmap, y, x+2) = machine->pens[0x200+(((vga_vram[offs] & 0x0c)>>2)<<1)];
			*BITMAP_ADDR16(bitmap, y, x+3) = machine->pens[0x200+(((vga_vram[offs] & 0x03)>>0)<<1)];
			offs++;
		}

	offs = 0x2000+map_offs;
	for(y=1;y<200;y+=2)
		for(x=0;x<320;x+=4)
		{
			*BITMAP_ADDR16(bitmap, y, x+0) = machine->pens[0x200+(((vga_vram[offs] & 0xc0)>>6)<<1)];
			*BITMAP_ADDR16(bitmap, y, x+1) = machine->pens[0x200+(((vga_vram[offs] & 0x30)>>4)<<1)];
			*BITMAP_ADDR16(bitmap, y, x+2) = machine->pens[0x200+(((vga_vram[offs] & 0x0c)>>2)<<1)];
			*BITMAP_ADDR16(bitmap, y, x+3) = machine->pens[0x200+(((vga_vram[offs] & 0x03)>>0)<<1)];
			offs++;
		}

}



static void cga_alphanumeric_tilemap(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs,UINT8 gfx_num)
{
	static UINT32 offs,x,y,max_x,max_y;

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
		for(x=0;x<max_x;x++)
		{
			int tile =  vga_vram[offs] & 0xff;
			int color = vga_vram[offs+1] & 0xff;

			drawgfx_transpen(bitmap,cliprect,machine->gfx[gfx_num],
					tile,
					color,
					0,0,
					x*8,y*8,
					((color & 0xf0) != 0) ? -1 : 0);

			offs+=2;
		}
}


static VIDEO_UPDATE( filetto )
{
/*          xx1x xxxx  Attribute bit 7. 0=blink, 1=Intesity
            xxx1 xxxx  640x200 mode
            xxxx 1xxx  Enable video signal
            xxxx x1xx  Select B/W mode
            xxxx xx1x  Select graphics
            xxxx xxx1  80x25 text
            */
	bitmap_fill(bitmap, cliprect, 0);

	if(vga_mode[0] & 8)
	{
		if(vga_mode[0] & 2)
			cga_graphic_bitmap(screen->machine,bitmap,cliprect,0,0x18000);
		else
		{
			switch(vga_mode[0] & 1)
			{
				case 0x00:
					cga_alphanumeric_tilemap(screen->machine,bitmap,cliprect,RES_320x200,0x18000,2);
					break;
				case 0x01:
					cga_alphanumeric_tilemap(screen->machine,bitmap,cliprect,RES_640x200,0x18000,2);
					break;
			}
		}
	}

	return 0;
}

static void vga_bitmap_layer(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int x,y,z;
	UINT8 *region = memory_region(machine, "user1");
	static UINT32 cur_bank;

	/*TODO: might be a different descramble algorythm plus plain bg bank*/
	cur_bank = (((8-vga_bg_bank[0]) & 0x1f)*0x10000);

	for(y=0;y<200;y+=8)
	{
		for(z=0;z<8;z++)
		for(x=0;x<320;x++)
		{
			*BITMAP_ADDR16(bitmap, y+z, x) = 0x200+(region[(y*320/8)+x+z*0x2000+cur_bank+8] & 0xf);
		}
	}
}

/*S3 Video card,VGA*/
static VIDEO_UPDATE( tetriskr )
{
	bitmap_fill(bitmap, cliprect, 0);

	if(vga_mode[0] & 8)
	{
		if(vga_mode[0] & 2)
			cga_graphic_bitmap(screen->machine,bitmap,cliprect,0,0x18000);
		else
		{
			vga_bitmap_layer(screen->machine,bitmap,cliprect);

			switch(vga_mode[0] & 1)
			{
				case 0x00:
					cga_alphanumeric_tilemap(screen->machine,bitmap,cliprect,RES_320x200,0x18000,0);
					break;
				case 0x01:
					cga_alphanumeric_tilemap(screen->machine,bitmap,cliprect,RES_640x200,0x18000,0);
					break;
			}
		}
	}

	return 0;
}

static READ8_HANDLER( vga_regs_r )
{
	logerror("(PC=%05x) Warning: VGA reg port read\n",cpu_get_pc(space->cpu));
	return 0xff;
}

static WRITE8_HANDLER( vga_regs_w )
{
	static UINT8 video_index;

	if(offset == 0)
	{
		video_index = data;
	}
	if(offset == 1)
	{
		if(video_index <= 0x18)
		{
			video_regs[video_index] = data;
			//logerror("write %02x to video register [%02x]",data,video_index);
		}
		else
			logerror("(PC=%05x) Warning: Undefined VGA reg port write (I=%02x D=%02x)\n",cpu_get_pc(space->cpu),video_index,data);
	}
}

static WRITE8_HANDLER( vga_vram_w )
{
	vga_vram[offset] = data;
}

/*end of Video HW file*/

static struct {
	running_device	*pit8253;
	running_device	*pic8259_1;
	running_device	*pic8259_2;
	running_device	*dma8237_1;
	running_device	*dma8237_2;
} filetto_devices;


static UINT8 disk_data[2];

static READ8_HANDLER( disk_iobank_r )
{
	//printf("Read Prototyping card [%02x] @ PC=%05x\n",offset,cpu_get_pc(space->cpu));
	//if(offset == 0) return input_port_read(space->machine, "DSW");
	if(offset == 1) return input_port_read(space->machine, "IN1");

	return disk_data[offset];
}

static WRITE8_HANDLER( disk_iobank_w )
{
/*
    BIOS does a single out $0310,$F0 on reset

    Then does 2 outs to set the bank..

        X1  X2

        $F0 $F2 = m0
        $F1 $F2 = m1
        $F0 $F3 = m2
        $F1 $F3 = m3

    The sequence of

    out $0310,X1
    out $0310,X2

    sets the selected rom that appears in $C0000-$CFFFF

*/
	int newbank = 0;

//  printf("bank %d set to %02X\n", offset,data);

	if (data == 0xF0)
	{
		newbank = 0;
	}
	else
	{
		if((lastvalue == 0xF0) && (data == 0xF2))
			newbank = 0;
		else if ((lastvalue == 0xF1) && (data == 0xF2))
			newbank = 1;
		else if ((lastvalue == 0xF0) && (data == 0xF3))
			newbank = 2;
		else if ((lastvalue == 0xF1) && (data == 0xF3))
			newbank = 3;
	}

//  printf("newbank = %d\n", newbank);

	if (newbank != bank)
	{
		bank = newbank;
		memory_set_bankptr(space->machine,  "bank1",memory_region(space->machine, "user1") + 0x10000 * bank );
	}

	lastvalue = data;

	disk_data[offset] = data;
}

/*********************************
Pit8253
*********************************/

static const struct pit8253_config pc_pit8253_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir0_w)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};

static UINT8 port_b_data;
static UINT8 wss1_data,wss2_data;

static READ8_DEVICE_HANDLER( port_a_r )
{
	if(!(port_b_data & 0x80))//???
	{
		/*
        x--- ---- Undefined (Always 0)
        -x-- ---- B: Floppy disk drive installed.
        --xx ---- Default Display Mode
        ---- xx-- Undefined (Always 1)
        ---- --x- 8087 NDP installed
        ---- ---x Undefined (Always 1)
        */
		return wss1_data;
	}
	else//keyboard emulation
	{
		//cputag_set_input_line(device->machine, "maincpu", 1, PULSE_LINE);
		return 0x00;//Keyboard is disconnected
		//return 0xaa;//Keyboard code
	}
}

static READ8_DEVICE_HANDLER( port_b_r )
{
	return port_b_data;
}

static READ8_DEVICE_HANDLER( port_c_r )
{
	return wss2_data;//???
}

/*'buzzer' sound routes here*/
/* Filetto uses this for either beep and um5100 sound routing,probably there's a mux somewhere.*/
/* The Korean Tetris uses it as a regular buzzer,probably the sound is all in there...*/
static WRITE8_DEVICE_HANDLER( port_b_w )
{
	port_b_data = data;
// running_device *beep = device->machine->device("beep");
// running_device *cvsd = device->machine->device("cvsd");
//  hc55516_digit_w(cvsd, data);
//  popmessage("%02x\n",data);
//  beep_set_state(beep, 0);
//  beep_set_state(beep, 1);
//  beep_set_frequency(beep, port_b_data);
}

static WRITE8_DEVICE_HANDLER( wss_1_w )
{
	wss1_data = data;
}

static WRITE8_DEVICE_HANDLER( wss_2_w )
{
	wss2_data = data;
}

static WRITE8_DEVICE_HANDLER( sys_reset_w )
{
	cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_RESET, PULSE_LINE);
}


static const ppi8255_interface filetto_ppi8255_intf[2] =
{
	{
		DEVCB_HANDLER(port_a_r),		/* Port A read */
		DEVCB_HANDLER(port_b_r),		/* Port B read */
		DEVCB_HANDLER(port_c_r),		/* Port C read */
		DEVCB_NULL, 					/* Port A write */
		DEVCB_HANDLER(port_b_w),		/* Port B write */
		DEVCB_NULL						/* Port C write */
	},
	{
		DEVCB_NULL,						/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_HANDLER(wss_1_w),			/* Port A write */
		DEVCB_HANDLER(wss_2_w),			/* Port B write */
		DEVCB_HANDLER(sys_reset_w)		/* Port C write */
	}
};

/*Floppy Disk Controller 765 device*/
/*Currently we only emulate it at a point that the BIOS will pass the checks*/
static UINT8 status;

#define FDC_BUSY 0x10
#define FDC_WRITE 0x40
#define FDC_READ 0x00 /*~0x40*/

static READ8_HANDLER( fdc765_status_r )
{
	static UINT8 tmp,clr_status;
//  popmessage("Read FDC status @ PC=%05x",cpu_get_pc(space->cpu));
	tmp = status | 0x80;
	clr_status++;
	if(clr_status == 0x10)
	{
		status = 0;
		clr_status = 0;
	}
	return tmp;
}

static READ8_HANDLER( fdc765_data_r )
{
	status = (FDC_READ);
	return 0xc0;
}

static WRITE8_HANDLER( fdc765_data_w )
{
	status = (FDC_WRITE);
}

static UINT8 drive_data;

static WRITE8_HANDLER( drive_selection_w )
{
	drive_data = data;
	/*write to this area then expects that location [43e] has the bit 7 activated*/
	work_ram[0x3e] = 0x80;
}

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

	return space->read_byte(page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	space->write_byte(page_offset + offset, data);
}

static READ8_HANDLER(dma_page_select_r)
{
	UINT8 data = at_pages[offset % 0x10];

	switch(offset % 8) {
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

	switch(offset % 8) {
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
	r = pic8259_acknowledge(filetto_devices.pic8259_2);
	if (r==0)
	{
		r = pic8259_acknowledge(filetto_devices.pic8259_1);
	}
	return r;
}

static ADDRESS_MAP_START( filetto_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x003ff) AM_RAM //irq vectors
	AM_RANGE(0x00400, 0x007ff) AM_RAM AM_BASE(&work_ram)
	AM_RANGE(0x00800, 0x9ffff) AM_RAM //work RAM 640KB
	AM_RANGE(0xa0000, 0xbffff) AM_RAM_WRITE(vga_vram_w) AM_BASE(&vga_vram)//VGA RAM
	AM_RANGE(0xc0000, 0xcffff) AM_ROMBANK("bank1")
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( filetto_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237_1", i8237_r, i8237_w ) //8237 DMA Controller
	AM_RANGE(0x0020, 0x002f) AM_DEVREADWRITE("pic8259_1", pic8259_r, pic8259_w ) //8259 Interrupt control
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_r, pit8253_w)    //8253 PIT
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)  //PPI 8255
	AM_RANGE(0x0064, 0x0066) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)  //PPI 8255
	AM_RANGE(0x0070, 0x007f) AM_READWRITE(mc146818_port_r,mc146818_port_w)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(dma_page_select_r,dma_page_select_w)
	AM_RANGE(0x00a0, 0x00af) AM_DEVREADWRITE("pic8259_2", pic8259_r, pic8259_w )
//  AM_RANGE(0x0200, 0x020f) AM_RAM //game port
	AM_RANGE(0x0201, 0x0201) AM_READ_PORT("COIN") //game port
	AM_RANGE(0x0278, 0x027f) AM_RAM //printer (parallel) port latch
	AM_RANGE(0x02f8, 0x02ff) AM_RAM //Modem port
	AM_RANGE(0x0310, 0x0311) AM_READWRITE(disk_iobank_r,disk_iobank_w) //Prototyping card
	AM_RANGE(0x0312, 0x0312) AM_READ_PORT("IN0") //Prototyping card,read only
	AM_RANGE(0x0378, 0x037f) AM_RAM //printer (parallel) port
	AM_RANGE(0x03bc, 0x03bf) AM_RAM //printer port
	AM_RANGE(0x03b4, 0x03b5) AM_READWRITE(vga_regs_r,vga_regs_w) //various VGA/CGA/EGA regs
	AM_RANGE(0x03d4, 0x03d5) AM_READWRITE(vga_regs_r,vga_regs_w) //mirror of above
	AM_RANGE(0x03d8, 0x03d9) AM_RAM AM_BASE(&vga_mode)
	AM_RANGE(0x03ba, 0x03bb) AM_READ(vga_hvretrace_r)//Controls H-Blank/V-Blank
	AM_RANGE(0x03da, 0x03db) AM_READ(vga_hvretrace_r)//mirror of above
	AM_RANGE(0x03f2, 0x03f2) AM_WRITE(drive_selection_w)
	AM_RANGE(0x03f4, 0x03f4) AM_READ(fdc765_status_r) //765 Floppy Disk Controller (FDC) Status
	AM_RANGE(0x03f5, 0x03f5) AM_READWRITE(fdc765_data_r,fdc765_data_w)//FDC Data
	AM_RANGE(0x03f8, 0x03ff) AM_RAM //rs232c (serial) port
ADDRESS_MAP_END

static ADDRESS_MAP_START( tetriskr_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237_1", i8237_r, i8237_w ) //8237 DMA Controller
	AM_RANGE(0x0020, 0x002f) AM_DEVREADWRITE("pic8259_1", pic8259_r, pic8259_w ) //8259 Interrupt control
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_r, pit8253_w)    //8253 PIT
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)  //PPI 8255
	AM_RANGE(0x0064, 0x0066) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)  //PPI 8255
	AM_RANGE(0x0070, 0x007f) AM_READWRITE(mc146818_port_r,mc146818_port_w)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(dma_page_select_r,dma_page_select_w)
	AM_RANGE(0x00a0, 0x00af) AM_DEVREADWRITE("pic8259_2", pic8259_r, pic8259_w )
	AM_RANGE(0x0200, 0x020f) AM_RAM //game port
//  AM_RANGE(0x0201, 0x0201) AM_READ_PORT("IN1") //game port
	AM_RANGE(0x0278, 0x027f) AM_RAM //printer (parallel) port latch
	AM_RANGE(0x02f8, 0x02ff) AM_RAM //Modem port
//  AM_RANGE(0x0310, 0x0311) AM_READWRITE(disk_iobank_r,disk_iobank_w) //Prototyping card
//  AM_RANGE(0x0312, 0x0312) AM_READ_PORT("IN0") //Prototyping card,read only
	AM_RANGE(0x0378, 0x037f) AM_RAM //printer (parallel) port
	AM_RANGE(0x03c0, 0x03c0) AM_RAM AM_BASE(&vga_bg_bank)
	AM_RANGE(0x03c8, 0x03c8) AM_READ_PORT("IN0")
	AM_RANGE(0x03c9, 0x03c9) AM_READ_PORT("IN1")
//  AM_RANGE(0x03ce, 0x03ce) AM_READ_PORT("IN1")
	AM_RANGE(0x03bc, 0x03bf) AM_RAM //printer port
	AM_RANGE(0x03b4, 0x03b5) AM_READWRITE(vga_regs_r,vga_regs_w) //various VGA/CGA/EGA regs
	AM_RANGE(0x03d4, 0x03d5) AM_READWRITE(vga_regs_r,vga_regs_w) //mirror of above
	AM_RANGE(0x03d8, 0x03d9) AM_RAM AM_BASE(&vga_mode)
	AM_RANGE(0x03ba, 0x03bb) AM_READ(vga_hvretrace_r)//Controls H-Blank/V-Blank
	AM_RANGE(0x03da, 0x03db) AM_READ(vga_hvretrace_r)//mirror of above
	AM_RANGE(0x03f2, 0x03f2) AM_WRITE(drive_selection_w)
	AM_RANGE(0x03f4, 0x03f4) AM_READ(fdc765_status_r) //765 Floppy Disk Controller (FDC) Status
	AM_RANGE(0x03f5, 0x03f5) AM_READWRITE(fdc765_data_r,fdc765_data_w)//FDC Data
	AM_RANGE(0x03f8, 0x03ff) AM_RAM //rs232c (serial) port
ADDRESS_MAP_END

static INPUT_PORTS_START( filetto )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Extra Play" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, "Play at 6th match reached" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tetriskr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) //probably unused
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_START("IN1") //dip-switches?
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout dos_chars =
{
	8,16,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	(0x4000)+(0*8),(0x4000)+(1*8),(0x4000)+(2*8),(0x4000)+(3*8),(0x4000)+(4*8),(0x4000)+(5*8),(0x4000)+(6*8),(0x4000)+(7*8) },
	8*8
};

static const gfx_layout dos_chars2 =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( filetto )
	GFXDECODE_ENTRY( "gfx1", 0x0000, dos_chars,    0, 0x100 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, dos_chars2,   0, 0x100 )
	GFXDECODE_ENTRY( "gfx1", 0x1800, dos_chars2,   0, 0x100 )
GFXDECODE_END

static GFXDECODE_START( tetriskr )
	GFXDECODE_ENTRY( "gfx1", 0x0000, dos_chars2,   0, 0x100 )
GFXDECODE_END


/*
BLACK = 0
BLUE = 1
GREEN = 2
CYAN = 3
RED = 4
MAGENTA = 5
BROWN = 6
LIGHT GRAY = 7
*/

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

static PALETTE_INIT(filetto)
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

static MACHINE_RESET( filetto )
{
	bank = -1;
	lastvalue = -1;
	hv_blank = 0;
	cpu_set_irq_callback(machine->device("maincpu"), irq_callback);
	filetto_devices.pit8253 = machine->device( "pit8253" );
	filetto_devices.pic8259_1 = machine->device( "pic8259_1" );
	filetto_devices.pic8259_2 = machine->device( "pic8259_2" );
	filetto_devices.dma8237_1 = machine->device( "dma8237_1" );
	filetto_devices.dma8237_2 = machine->device( "dma8237_2" );
}

static MACHINE_DRIVER_START( filetto )
	MDRV_CPU_ADD("maincpu", I8088, 8000000) //or regular PC-XT 14318180/3 clock?
	MDRV_CPU_PROGRAM_MAP(filetto_map)
	MDRV_CPU_IO_MAP(filetto_io)

	MDRV_MACHINE_RESET( filetto )

	MDRV_PIT8253_ADD( "pit8253", pc_pit8253_config )

	MDRV_PPI8255_ADD( "ppi8255_0", filetto_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", filetto_ppi8255_intf[1] )

	MDRV_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )

	MDRV_PIC8259_ADD( "pic8259_1", pic8259_1_config )

	MDRV_PIC8259_ADD( "pic8259_2", pic8259_2_config )

	MDRV_GFXDECODE(filetto)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 640-1, 0*8, 480-1)

	MDRV_PALETTE_LENGTH(0x300)

	MDRV_PALETTE_INIT(filetto)

	MDRV_VIDEO_START(filetto)
	MDRV_VIDEO_UPDATE(filetto)

	/*Sound Hardware*/
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("voice", HC55516, 8000000/4)//8923S-UM5100 is a HC55536 with ROM hook-up
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

//  PC "buzzer" sound
	MDRV_SOUND_ADD("beep", BEEP, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tetriskr )
	MDRV_CPU_ADD("maincpu", I8088, 14318180/3)
	MDRV_CPU_PROGRAM_MAP(filetto_map)
	MDRV_CPU_IO_MAP(tetriskr_io)

	MDRV_MACHINE_RESET( filetto )

	MDRV_PIT8253_ADD( "pit8253", pc_pit8253_config )

	MDRV_PPI8255_ADD( "ppi8255_0", filetto_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", filetto_ppi8255_intf[1] )

	MDRV_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )

	MDRV_PIC8259_ADD( "pic8259_1", pic8259_1_config )

	MDRV_PIC8259_ADD( "pic8259_2", pic8259_2_config )

	MDRV_GFXDECODE(tetriskr)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 640-1, 0*8, 480-1)

	MDRV_PALETTE_LENGTH(0x300)

	MDRV_PALETTE_INIT(filetto)

	MDRV_VIDEO_START(tetriskr)
	MDRV_VIDEO_UPDATE(tetriskr)

	/*Sound Hardware*/
	MDRV_SPEAKER_STANDARD_MONO("mono")

//  PC "buzzer" sound
	MDRV_SOUND_ADD("beep", BEEP, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_DRIVER_END

ROM_START( filetto )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("u49.bin", 0xfc000, 0x2000, CRC(1be6948a) SHA1(9c433f63d347c211ee4663f133e8417221bc4bf0))
	ROM_RELOAD(         0xf8000, 0x2000 )
	ROM_RELOAD(         0xf4000, 0x2000 )
	ROM_RELOAD(         0xf0000, 0x2000 )
	ROM_LOAD("u55.bin", 0xfe000, 0x2000, CRC(1e455ed7) SHA1(786d18ce0ab1af45fc538a2300853e497488f0d4) )
	ROM_RELOAD(         0xfa000, 0x2000 )
	ROM_RELOAD(         0xf6000, 0x2000 )
	ROM_RELOAD(         0xf2000, 0x2000 )

	ROM_REGION( 0x40000, "user1", 0 ) // program data
	ROM_LOAD( "m0.u1", 0x00000, 0x10000, CRC(2408289d) SHA1(eafc144a557a79b58bcb48545cb9c9778e61fcd3) )
	ROM_LOAD( "m1.u2", 0x10000, 0x10000, CRC(5b623114) SHA1(0d9a14e6b7f57ce4fa09762343b610a973910f58) )
	ROM_LOAD( "m2.u3", 0x20000, 0x10000, CRC(abc64869) SHA1(564fc9d90d241a7b7776160b3fd036fb08037355) )
	ROM_LOAD( "m3.u4", 0x30000, 0x10000, CRC(0c1e8a67) SHA1(f1b9280c65fcfcb5ec481cae48eb6f52d6cdbc9d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD("u67.bin", 0x0000, 0x2000, CRC(09710122) SHA1(de84bdd9245df287bbd3bb808f0c3531d13a3545) )

	ROM_REGION( 0x40000, "user2", 0 ) // UM5100 sample roms?
	ROM_LOAD16_BYTE("v1.u15",  0x00000, 0x20000, CRC(613ddd07) SHA1(ebda3d559315879819cb7034b5696f8e7861fe42) )
	ROM_LOAD16_BYTE("v2.u14",  0x00001, 0x20000, CRC(427e012e) SHA1(50514a6307e63078fe7444a96e39d834684db7df) )
ROM_END

ROM_START( tetriskr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* code */
	ROM_LOAD( "b-10.u10", 0xf0000, 0x10000, CRC(efc2a0f6) SHA1(5f0f1e90237bee9b78184035a32055b059a91eb3) )

	ROM_REGION( 0x10000, "gfx1",0 ) /* gfx - 1bpp font*/
	ROM_LOAD( "b-3.u36", 0x00000, 0x2000, CRC(1a636f9a) SHA1(a356cc57914d0c9b9127670b55d1f340e64b1ac9) )

	ROM_REGION( 0x80000, "gfx2",ROMREGION_INVERT )
	ROM_LOAD( "b-1.u59", 0x00000, 0x10000, CRC(4719d986) SHA1(6e0499944b968d96fbbfa3ead6237d69c769d634))
	ROM_LOAD( "b-2.u58", 0x10000, 0x10000, CRC(599e1154) SHA1(14d99f90b4fedeab0ac24ffa9b1fd9ad0f0ba699))
	ROM_LOAD( "b-4.u54", 0x20000, 0x10000, CRC(e112c450) SHA1(dfdecfc6bd617ec520b7563b7caf44b79d498bd3))
	ROM_LOAD( "b-5.u53", 0x30000, 0x10000, CRC(050b7650) SHA1(5981dda4ed43b6e81fbe48bfba90a8775d5ecddf))
	ROM_LOAD( "b-6.u49", 0x40000, 0x10000, CRC(d596ceb0) SHA1(8c82fb638688971ef11159a6b240253e63f0949d))
	ROM_LOAD( "b-7.u48", 0x50000, 0x10000, CRC(79336b6c) SHA1(7a95875f3071bdc3ee25c0e6a5a3c00ef02dc977))
	ROM_LOAD( "b-8.u44", 0x60000, 0x10000, CRC(1f82121a) SHA1(106da0f39f1260d0761217ed0a24c1611bfd7f05))
	ROM_LOAD( "b-9.u43", 0x70000, 0x10000, CRC(4ea22349) SHA1(14dfd3dbd51f8bd6f3290293b8ea1c165e8cf7fd))

	ROM_REGION( 0x180000, "user1", ROMREGION_ERASEFF )
	// copy for the gfx2,to be made with the DRIVER_INIT
ROM_END

static DRIVER_INIT( filetto )
{
	//...
}

/*Descramble the background gfx data.*/
static DRIVER_INIT( tetriskr )
{
	int i,j,k;
	int index=0;
	UINT8 *region = memory_region(machine, "user1");
	UINT8 *gfx = memory_region(machine, "gfx2");

	for(i=0;i<0x20000;i++)
	{
		//8 pixels/byte
		for(j=0;j<8;j++)
		{
			int mask=(1<<(7-j));
			int pixel=0;
			for(k=0;k<4;k++)
			{
				if(gfx[k*0x20000+i]&mask)
				{
					pixel|=(1<<k);
				}
			}
			region[index++]=pixel;
		}
	}
}

GAME( 1990, filetto,  0, filetto,  filetto,  filetto,  ROT0,  "Novarmatic", "Filetto (v1.05 901009)",GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1988?,tetriskr, 0, tetriskr, tetriskr, tetriskr, ROT0,  "bootleg",    "Tetris (bootleg of Mirrorsoft PC-XT Tetris version)", GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
