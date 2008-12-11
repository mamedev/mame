/******************************************************************************************

Filetto (c) 1990 Novarmatic

driver by Angelo Salese & Chris Hardy

TODO:
- Use the MESS implementation of the CGA video emulation;
- Add a proper FDC device,if you enable it will give a boot error,probably because it
  expects that in the floppy drive shouldn't be anything,there's currently a kludge that
  does the trick;
- Add sound,"buzzer" PC sound plus the UM5100 sound chip,might be connected to the
  prototyping card;

********************************************************************************************
HW notes:
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
SW notes:
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

********************************************************************************************
Vector & irq notes (Mainly a memo for me):

7c69 -> int $13 (FDC check),done
7c6c
16ef9 -> read dsw
159cd []
[2361c]
12eae
int $10 (vector number 0x0040) cmds
AH
0x00 - Set Video mode ($449)
0x01 -
0x02 - Set Cursor position (BH=$00 IP=$ff71b DX=$0600)
0x0e - Teletype output (IP=$7d3c) (Note: This types "Errore di Boot")
******************************************************************************************/

#include "driver.h"
#include "video/generic.h"
#include "machine/pit8253.h"
#include "machine/8255ppi.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "sound/hc55516.h"

#define SET_VISIBLE_AREA(_x_,_y_) \
	{ \
	rectangle visarea = *video_screen_get_visible_area(machine->primary_screen); \
	visarea.min_x = 0; \
	visarea.max_x = _x_-1; \
	visarea.min_y = 0; \
	visarea.max_y = _y_-1; \
	video_screen_configure(machine->primary_screen, _x_, _y_, &visarea, video_screen_get_frame_period(machine->primary_screen).attoseconds ); \
	} \


static UINT8 *vga_vram,*work_ram;
static UINT8 video_regs[0x19];
static UINT8 *vga_mode;
static UINT8 hv_blank;

static int bank;
static int lastvalue;

/*Add here Video regs defines...*/


#define RES_320x200 0
#define RES_640x200 1

static void cga_alphanumeric_tilemap(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs);

static VIDEO_START( filetto )
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
	h = video_screen_get_height(space->machine->primary_screen);
	w = video_screen_get_width(space->machine->primary_screen);

//  popmessage("%d %d",h,w);

	if (video_screen_get_hpos(space->machine->primary_screen) > h)
		res|= 1;

	if (video_screen_get_vpos(space->machine->primary_screen) > w)
		res|= 8;

	return res;
}

//Note: this should be converted to red/green/brown instead of magenta/cyan/white
/*Basic Graphic mode */
static void cga_graphic_bitmap(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs)
{
	static UINT16 x,y,pen = 0;
	static UINT32 offs;

	SET_VISIBLE_AREA(320,200);
	offs = map_offs;
	for(y=0;y<200;y+=2)
		for(x=0;x<320;x+=4)
		{
			switch((vga_vram[offs] & 0xc0) >> 6)
			{
				case 0: pen = 0xf0; break;
				case 1: pen = 0xfb; break;
				case 2: pen = 0xfd; break;
				case 3: pen = 0xff; break;
			}
			*BITMAP_ADDR16(bitmap, y, x) = pen;
			switch((vga_vram[offs] & 0x30) >> 4)
			{
				case 0: pen = 0xf0; break;
				case 1: pen = 0xfb; break;
				case 2: pen = 0xfd; break;
				case 3: pen = 0xff; break;
			}
			*BITMAP_ADDR16(bitmap, y, x+1) = pen;
			switch((vga_vram[offs] & 0x0c) >> 2)
			{
				case 0: pen = 0xf0; break;
				case 1: pen = 0xfb; break;
				case 2: pen = 0xfd; break;
				case 3: pen = 0xff; break;
			}
			*BITMAP_ADDR16(bitmap, y, x+2) = pen;
			switch((vga_vram[offs] & 0x03) >> 0)
			{
				case 0: pen = 0xf0; break;
				case 1: pen = 0xfb; break;
				case 2: pen = 0xfd; break;
				case 3: pen = 0xff; break;
			}
			*BITMAP_ADDR16(bitmap, y, x+3) = pen;
			offs++;
		}

	offs = 0x2000+map_offs;
	for(y=1;y<200;y+=2)
		for(x=0;x<320;x+=4)
		{
			switch((vga_vram[offs] & 0xc0) >> 6)
			{
				case 0: pen = 0xf0; break;
				case 1: pen = 0xfb; break;
				case 2: pen = 0xfd; break;
				case 3: pen = 0xff; break;
			}
			*BITMAP_ADDR16(bitmap, y, x) = pen;
			switch((vga_vram[offs] & 0x30) >> 4)
			{
				case 0: pen = 0xf0; break;
				case 1: pen = 0xfb; break;
				case 2: pen = 0xfd; break;
				case 3: pen = 0xff; break;
			}
			*BITMAP_ADDR16(bitmap, y, x+1) = pen;
			switch((vga_vram[offs] & 0x0c) >> 2)
			{
				case 0: pen = 0xf0; break;
				case 1: pen = 0xfb; break;
				case 2: pen = 0xfd; break;
				case 3: pen = 0xff; break;
			}
			*BITMAP_ADDR16(bitmap, y, x+2) = pen;
			switch((vga_vram[offs] & 0x03) >> 0)
			{
				case 0: pen = 0xf0; break;
				case 1: pen = 0xfb; break;
				case 2: pen = 0xfd; break;
				case 3: pen = 0xff; break;
			}
			*BITMAP_ADDR16(bitmap, y, x+3) = pen;
			offs++;
		}

}



static void cga_alphanumeric_tilemap(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs)
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
			int color = vga_vram[offs+1] & 0x0f;

			drawgfx(bitmap,machine->gfx[2],
					tile,
					color,
					0,0,
					x*8,y*8,
					cliprect,TRANSPARENCY_NONE,0);

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
					cga_alphanumeric_tilemap(screen->machine,bitmap,cliprect,RES_320x200,0x18000);
					break;
				case 0x01:
					cga_alphanumeric_tilemap(screen->machine,bitmap,cliprect,RES_640x200,0x18000);
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
	const device_config	*pit8253;
	const device_config	*pic8259_1;
	const device_config	*pic8259_2;
	const device_config	*dma8237_1;
	const device_config	*dma8237_2;
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
		memory_set_bankptr(space->machine,  1,memory_region(space->machine, "user1") + 0x10000 * bank );
	}

	lastvalue = data;

	disk_data[offset] = data;
}

/*********************************
Pit8253
*********************************/

static PIT8253_OUTPUT_CHANGED( pc_timer0_w )
{
    pic8259_set_irq_line(filetto_devices.pic8259_1, 0, state);
}

static const struct pit8253_config pc_pit8253_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			pc_timer0_w
		}, {
			4772720/4,				/* dram refresh */
			NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			NULL
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
		//cpu_set_input_line(device->machine->cpu[0],1,PULSE_LINE);
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
static WRITE8_DEVICE_HANDLER( port_b_w )
{
	port_b_data = data;
//  hc55516_digit_w(0, data);
//  popmessage("%02x",data);
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
	cpu_set_input_line(device->machine->cpu[0],INPUT_LINE_RESET,PULSE_LINE);
}


static const ppi8255_interface filetto_ppi8255_intf[2] =
{
	{
		port_a_r,			/* Port A read */
		port_b_r,			/* Port B read */
		port_c_r,  			/* Port C read */
		NULL,   			/* Port A write */
		port_b_w,  		 	/* Port B write */
		NULL				/* Port C write */
	},
	{
		NULL,				/* Port A read */
		NULL,				/* Port B read */
		NULL,				/* Port C read */
		wss_1_w,			/* Port A write */
		wss_2_w,			/* Port B write */
		sys_reset_w			/* Port C write */
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

static UINT8 dma_offset[2][4];
static UINT8 at_pages[0x10];

static DMA8237_MEM_READ( pc_dma_read_byte )
{
	const address_space *space = cpu_get_address_space(device->machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	offs_t page_offset = (((offs_t) dma_offset[0][channel]) << 16)
		& 0xFF0000;

	return memory_read_byte(space, page_offset + offset);
}


static DMA8237_MEM_WRITE( pc_dma_write_byte )
{
	const address_space *space = cpu_get_address_space(device->machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	offs_t page_offset = (((offs_t) dma_offset[0][channel]) << 16)
		& 0xFF0000;

	memory_write_byte(space, page_offset + offset, data);
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


static const struct dma8237_interface dma8237_1_config =
{
	0,
	1.0e-6, // 1us

	pc_dma_read_byte,
	pc_dma_write_byte,

	{ 0, 0, NULL, NULL },
	{ 0, 0, NULL, NULL },
	NULL
};

/******************
8259 IRQ controller
******************/

static PIC8259_SET_INT_LINE( pic8259_1_set_int_line ) {
	cpu_set_input_line(device->machine->cpu[0], 0, interrupt ? HOLD_LINE : CLEAR_LINE);
}

static const struct pic8259_interface pic8259_1_config = {
	pic8259_1_set_int_line
};

static PIC8259_SET_INT_LINE( pic8259_2_set_int_line ) {
	pic8259_set_irq_line( filetto_devices.pic8259_1, 2, interrupt);
}

static const struct pic8259_interface pic8259_2_config = {
	pic8259_2_set_int_line
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
	AM_RANGE(0xa0000, 0xbffff) AM_READWRITE(SMH_RAM,vga_vram_w) AM_BASE(&vga_vram)//VGA RAM
	AM_RANGE(0xc0000, 0xcffff) AM_READ(SMH_BANK1)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( filetto_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE(DMA8237, "dma8237_1", dma8237_r, dma8237_w ) //8237 DMA Controller
	AM_RANGE(0x0020, 0x002f) AM_DEVREADWRITE(PIC8259, "pic8259_1", pic8259_r, pic8259_w ) //8259 Interrupt control
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE(PIT8253, "pit8253", pit8253_r, pit8253_w)    //8253 PIT
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE(PPI8255, "ppi8255_0", ppi8255_r, ppi8255_w)  //PPI 8255
	AM_RANGE(0x0064, 0x0066) AM_DEVREADWRITE(PPI8255, "ppi8255_1", ppi8255_r, ppi8255_w)  //PPI 8255
	AM_RANGE(0x0070, 0x007f) AM_READWRITE(mc146818_port_r,mc146818_port_w)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(dma_page_select_r,dma_page_select_w)
	AM_RANGE(0x00a0, 0x00af) AM_DEVREADWRITE(PIC8259, "pic8259_2", pic8259_r, pic8259_w )
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
	GFXDECODE_ENTRY( "gfx1", 0x0000, dos_chars,    0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, dos_chars2,   0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x1800, dos_chars2,   0, 16 )
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

static PALETTE_INIT(filetto)
{
	/*Note:palette colors are 6bpp...
    xxxx xx--
    */
	int i;

	for(i=0;i<0x100;i++)
		palette_set_color(machine, i,MAKE_RGB(0x00,0x00,0x00));

	palette_set_color(machine, 0+2,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine, 1+2,MAKE_RGB(0x00,0x00,0x9f));
	palette_set_color(machine, 3+2,MAKE_RGB(0x00,0x9f,0x00));
	palette_set_color(machine, 5+2,MAKE_RGB(0x00,0x9f,0x9f));
	palette_set_color(machine, 7+2,MAKE_RGB(0x9f,0x00,0x00));
	palette_set_color(machine, 9+2,MAKE_RGB(0x9f,0x00,0x9f));
	palette_set_color(machine, 11+2,MAKE_RGB(0x9f,0x9f,0x00));
	palette_set_color(machine, 13+2,MAKE_RGB(0x9f,0x9f,0x9f));
	palette_set_color(machine, 15+2,MAKE_RGB(0x3f,0x3f,0x3f));
	palette_set_color(machine, 17+2,MAKE_RGB(0x3f,0x3f,0xff));
	palette_set_color(machine, 19+2,MAKE_RGB(0x3f,0xff,0x3f));
	palette_set_color(machine, 21+2,MAKE_RGB(0x3f,0xff,0xff));
	palette_set_color(machine, 23+2,MAKE_RGB(0xff,0x3f,0x10));
	palette_set_color(machine, 25+2,MAKE_RGB(0xff,0x3f,0xff));
	palette_set_color(machine, 27+2,MAKE_RGB(0xff,0xff,0x3f));
	palette_set_color(machine, 29+2,MAKE_RGB(0xff,0xff,0xff));

	palette_set_color(machine, 0xf0,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine, 0xf1,MAKE_RGB(0x00,0x00,0xaa));
	palette_set_color(machine, 0xf2,MAKE_RGB(0x00,0xaa,0x00));
	palette_set_color(machine, 0xf3,MAKE_RGB(0x00,0xaa,0xaa));
	palette_set_color(machine, 0xf4,MAKE_RGB(0xaa,0x00,0x00));
	palette_set_color(machine, 0xf5,MAKE_RGB(0xaa,0x00,0xaa));
	palette_set_color(machine, 0xf6,MAKE_RGB(0xaa,0xaa,0x00));
	palette_set_color(machine, 0xf7,MAKE_RGB(0xaa,0xaa,0xaa));
	palette_set_color(machine, 0xf8,MAKE_RGB(0x55,0x55,0x55));
	palette_set_color(machine, 0xf9,MAKE_RGB(0x55,0x55,0xff));
	palette_set_color(machine, 0xfa,MAKE_RGB(0x55,0xff,0x55));
	palette_set_color(machine, 0xfb,MAKE_RGB(0x55,0xff,0xff));
	palette_set_color(machine, 0xfc,MAKE_RGB(0xff,0x55,0x55));
	palette_set_color(machine, 0xfd,MAKE_RGB(0xff,0x55,0xff));
	palette_set_color(machine, 0xfe,MAKE_RGB(0xff,0xff,0x55));
	palette_set_color(machine, 0xff,MAKE_RGB(0xff,0xff,0xff));
}

static MACHINE_RESET( filetto )
{
	bank = -1;
	lastvalue = -1;
	hv_blank = 0;
	cpu_set_irq_callback(machine->cpu[0], irq_callback);
	filetto_devices.pit8253 = device_list_find_by_tag( machine->config->devicelist, PIT8253, "pit8253" );
	filetto_devices.pic8259_1 = device_list_find_by_tag( machine->config->devicelist, PIC8259, "pic8259_1" );
	filetto_devices.pic8259_2 = device_list_find_by_tag( machine->config->devicelist, PIC8259, "pic8259_2" );
	filetto_devices.dma8237_1 = device_list_find_by_tag( machine->config->devicelist, DMA8237, "dma8237_1" );
	filetto_devices.dma8237_2 = device_list_find_by_tag( machine->config->devicelist, DMA8237, "dma8237_2" );
}

static MACHINE_DRIVER_START( filetto )
	MDRV_CPU_ADD("main", I8088, 8000000)
	MDRV_CPU_PROGRAM_MAP(filetto_map,0)
	MDRV_CPU_IO_MAP(filetto_io,0)

	MDRV_MACHINE_RESET( filetto )

	MDRV_PIT8253_ADD( "pit8253", pc_pit8253_config )

	MDRV_PPI8255_ADD( "ppi8255_0", filetto_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", filetto_ppi8255_intf[1] )

	MDRV_DMA8237_ADD( "dma8237_1", dma8237_1_config )

	MDRV_PIC8259_ADD( "pic8259_1", pic8259_1_config )

	MDRV_PIC8259_ADD( "pic8259_2", pic8259_2_config )

	MDRV_GFXDECODE(filetto)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 640-1, 0*8, 480-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_PALETTE_INIT(filetto)

	MDRV_VIDEO_START(filetto)
	MDRV_VIDEO_UPDATE(filetto)

	/*Sound Hardware*/
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("voice", HC55516, 8000000/4)//8923S-UM5100 is a HC55536 with ROM hook-up
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

//  PC "buzzer" sound
MACHINE_DRIVER_END


ROM_START( filetto )
	ROM_REGION( 0x100000, "main", 0 )
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

	ROM_REGION( 0x2000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD("u67.bin", 0x0000, 0x2000, CRC(09710122) SHA1(de84bdd9245df287bbd3bb808f0c3531d13a3545) )

	ROM_REGION( 0x40000, "user2", ROMREGION_DISPOSE ) // UM5100 sample roms?
	ROM_LOAD16_BYTE("v1.u15",  0x00000, 0x20000, CRC(613ddd07) SHA1(ebda3d559315879819cb7034b5696f8e7861fe42) )
	ROM_LOAD16_BYTE("v2.u14",  0x00001, 0x20000, CRC(427e012e) SHA1(50514a6307e63078fe7444a96e39d834684db7df) )
ROM_END

static DRIVER_INIT( filetto )
{
	//...
}

GAME( 1990, filetto,    0, filetto,    filetto,   filetto, ROT0,  "Novarmatic", "Filetto (v1.05 901009)",GAME_NO_SOUND | GAME_IMPERFECT_COLORS)

