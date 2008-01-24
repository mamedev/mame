/******************************************************************************************

Filetto (c) 1990 Novarmatic

driver by Angelo Salese & Chris Hardy

TODO:
- Move the emulation of the VGA/CGA/EGA in a proper file,various old arcade games uses them,
  namely several Mega-Touch type games...
- The current emulation hasn't any PC-specific chip hooked-up,it just have the minimum
  requirements & some kludges to let this boot.
- Can't reset (currently for "Duplicate save state function (0, 0x41b900)"),pit8253 issue
  probably.

********************************************************************************************
HW notes:
The PCB is a (un?)modified IBM-PC with a CGA adapter & a prototyping card that controls the
interface between the pc and the Jamma connectors.
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
16ef9 -> read dsw?
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

#define SET_VISIBLE_AREA(_x_,_y_) \
	{ \
	screen_state *state = &machine->screen[0]; \
	rectangle visarea = state->visarea; \
	visarea.min_x = 0; \
	visarea.max_x = _x_-1; \
	visarea.min_y = 0; \
	visarea.max_y = _y_-1; \
	video_screen_configure(0, _x_, _y_, &visarea, state->refresh ); \
	} \


static UINT8 *vga_vram,*work_ram;
static UINT8 video_regs[0x19];
static UINT8 *vga_mode;
static UINT8 hv_blank;
/*Add here Video regs defines...*/


#define RES_320x200 0
#define RES_640x200 1

static void cga_alphanumeric_tilemap(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs);

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
	//v_blank^=8;
	//h_blank^=1;
	return (hv_blank);
}


/*Basic Graphic mode */
static void cga_graphic_bitmap(running_machine *machine,mame_bitmap *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs)
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



static void cga_alphanumeric_tilemap(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,UINT16 size,UINT32 map_offs)
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
	fillbitmap(bitmap, machine->pens[0], cliprect);

	if(vga_mode[0] & 8)
	{
		if(vga_mode[0] & 2)
			cga_graphic_bitmap(machine,bitmap,cliprect,0,0x18000);
		else
		{
			switch(vga_mode[0] & 1)
			{
				case 0x00:
					cga_alphanumeric_tilemap(machine,bitmap,cliprect,RES_320x200,0x18000);
					break;
				case 0x01:
					cga_alphanumeric_tilemap(machine,bitmap,cliprect,RES_640x200,0x18000);
					break;
			}
		}
	}

	return 0;
}

static READ8_HANDLER( vga_regs_r )
{
	logerror("(PC=%05x) Warning: VGA reg port read\n",activecpu_get_pc());
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
			video_regs[video_index] = data;
		else
			logerror("(PC=%05x) Warning: Undefined VGA reg port write (I=%02x D=%02x)\n",activecpu_get_pc(),video_index,data);
	}
}

static WRITE8_HANDLER( vga_vram_w )
{
	vga_vram[offset] = data;
}

static UINT8 disk_data[2];

static READ8_HANDLER( disk_iobank_r )
{
	printf("Read Prototyping card [%02x] @ PC=%05x\n",offset,activecpu_get_pc());
	if(offset == 1)
		return readinputport(1);

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
	static int bank = -1;
	static int lastvalue = -1;
	int newbank = 0;

	printf("bank %d set to %02X\n", offset,data);

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

	printf("newbank = %d\n", newbank);

	if (newbank != bank)
	{
		bank = newbank;
		memory_set_bankptr( 1,memory_region(REGION_USER1) + 0x10000 * bank );
	}

	lastvalue = data;

	disk_data[offset] = data;
}


/*RTC?*/
static READ8_HANDLER( undefined_r )
{
	logerror("(PC=%05x) Warning: Undefined read\n",activecpu_get_pc());
	return 0x00;
}

static WRITE8_HANDLER( nmi_enable_w )
{
	//if(data & 0x80)
		//cpunum_set_input_line(Machine, 0, INPUT_LINE_NMI,ASSERT_LINE);
	//if(!(data & 0x80))
		//cpunum_set_input_line(Machine, 0, INPUT_LINE_NMI,CLEAR_LINE);
}

static READ8_HANDLER( kludge_r )
{
	return mame_rand(Machine);
}

static void pc_timer0_w(int state)
{
	//if (state)
	//  pic8259_0_issue_irq(0);
}

static const struct pit8253_config pc_pit8253_config =
{
	TYPE8253,
	{
		{
			4772720/4,				/* heartbeat IRQ */
			pc_timer0_w,
			NULL
		}, {
			4772720/4,				/* dram refresh */
			NULL,
			NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			NULL,
			NULL//pc_sh_speaker_change_clock
		}
	}
};

static UINT8 drive_data;

static WRITE8_HANDLER( drive_selection_w )
{
	drive_data = data;
	/*Write to this area then expects that location [43e] has the bit 7 activated*/
	work_ram[0x3e] = 0x80;
}

static UINT8 port_b_data;
static UINT8 wss1_data,wss2_data;

static READ8_HANDLER( port_a_r )
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
		cpunum_set_input_line(Machine, 0,1,PULSE_LINE);
		return 0x00;//Keyboard is disconnected
		//return 0xaa;//Keyboard code
	}
}

static READ8_HANDLER( port_b_r )
{
	return port_b_data;
}

static READ8_HANDLER( port_c_r )
{
	return wss2_data;//???
}

static WRITE8_HANDLER( port_b_w )
{
	port_b_data = data;
}

static WRITE8_HANDLER( wss_1_w )
{
	wss1_data = data;
}

static WRITE8_HANDLER( wss_2_w )
{
	wss2_data = data;
}

static WRITE8_HANDLER( sys_reset_w )
{
	cpunum_set_input_line(Machine, 0,INPUT_LINE_RESET,PULSE_LINE);
}


static const ppi8255_interface filetto_ppi8255_intf =
{
	2, 							/* 2 chips */
	{ port_a_r,NULL },			/* Port A read */
	{ port_b_r,NULL },			/* Port B read */
	{ port_c_r,NULL },  		/* Port C read */
	{ NULL	  ,wss_1_w },   	/* Port A write */
	{ port_b_w,wss_2_w },   	/* Port B write */
	{ NULL	  ,sys_reset_w },	/* Port C write */
};

static UINT8 irq_data[2];

static READ8_HANDLER( irq_ctrl_8259_r )
{
	return irq_data[offset];
}

static WRITE8_HANDLER( irq_ctrl_8259_w )
{
//  usrintf_showmessage("Write to irq device %02x %02x\n",offset,data);
	irq_data[offset] = data;
}

/*Floppy Disk Controller 765 device*/
/*Currently we only emulate it at a point that the BIOS will pass the checks*/
static UINT8 status;

#define FDC_BUSY 0x10
#define FDC_WRITE 0x40
#define FDC_READ 0x00 /*~0x40*/

static READ8_HANDLER( fdc765_status_r )
{
	static UINT8 tmp,clr_status;
	popmessage("Read FDC status @ PC=%05x",activecpu_get_pc());
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

static ADDRESS_MAP_START( filetto_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x003ff) AM_RAM //irq vectors
	AM_RANGE(0x00400, 0x007ff) AM_RAM AM_BASE(&work_ram)
	AM_RANGE(0x00800, 0x9ffff) AM_RAM //work RAM 640KB
//  AM_RANGE(0xa0000, 0xb7fff) AM_RAM //VGA RAM
	AM_RANGE(0xa0000, 0xbffff) AM_READWRITE(MRA8_RAM,vga_vram_w) AM_BASE(&vga_vram)//VGA RAM
	AM_RANGE(0xc0000, 0xcffff) AM_READ(MRA8_BANK1)

	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( filetto_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x000f) AM_RAM //AM_READWRITE(dma8237_0_r,dma8237_0_w) //8237 DMA Controller
	AM_RANGE(0x0020, 0x0021) AM_READWRITE(irq_ctrl_8259_r,irq_ctrl_8259_w)//AM_READWRITE(pic8259_0_r,pic8259_0_w) //8259 Interrupt control
	AM_RANGE(0x0040, 0x0043) AM_READ(kludge_r) //AM_READWRITE(pit8253_0_r,pit8253_0_w) //8253 PIT
	AM_RANGE(0x0060, 0x0063) AM_READWRITE(ppi8255_0_r,ppi8255_0_w) //PPI 8255
	AM_RANGE(0x0064, 0x0066) AM_READWRITE(ppi8255_1_r,ppi8255_1_w) //PPI 8255
	AM_RANGE(0x0073, 0x0073) AM_READ(undefined_r)
	AM_RANGE(0x0080, 0x0087) AM_RAM //AM_READWRITE(dma_page_select_r,dma_page_select_w)
	AM_RANGE(0x00a0, 0x00a0) AM_WRITE(nmi_enable_w)
	//AM_RANGE(0x0200, 0x020f) AM_RAM //game port
	AM_RANGE(0x0278, 0x027f) AM_RAM //printer (parallel) port latch
	AM_RANGE(0x02f8, 0x02ff) AM_RAM //Modem port

	AM_RANGE(0x0310, 0x0311) AM_READWRITE(disk_iobank_r,disk_iobank_w) //Prototyping card (???)
	AM_RANGE(0x0312, 0x0312) AM_READ(input_port_0_r)

//  AM_RANGE(0x0300, 0x031f) AM_RAM //Prototyping card (???)
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
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //START1
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) //START2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	/*
    DSW1
    bit 0   = coinage (0=1co/1cr, 1= 2co/1cr)
    bit 1   = demo_sounds (0=no,1=yes)
    bit 2   = extra play (0=no extra play,1=play at 6th match reached)
    bit 3   = difficulty (0=normal,1=hard)
    bit 4-7 = <unused>
    DSW2
    <unused>
    */
	PORT_START
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, dos_chars,    0, 16 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x1000, dos_chars2,   0, 16 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x1800, dos_chars2,   0, 16 )
GFXDECODE_END


static MACHINE_RESET(filetto)
{
	ppi8255_init(&filetto_ppi8255_intf);
	pit8253_init(1, &pc_pit8253_config);
}

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
	palette_set_color(machine, 0xf1,MAKE_RGB(0x00,0x00,0x9f));
	palette_set_color(machine, 0xf2,MAKE_RGB(0x00,0x9f,0x00));
	palette_set_color(machine, 0xf3,MAKE_RGB(0x00,0x9f,0x9f));
	palette_set_color(machine, 0xf4,MAKE_RGB(0x9f,0x00,0x00));
	palette_set_color(machine, 0xf5,MAKE_RGB(0x9f,0x00,0x9f));
	palette_set_color(machine, 0xf6,MAKE_RGB(0x9f,0x9f,0x00));
	palette_set_color(machine, 0xf7,MAKE_RGB(0x9f,0x9f,0x9f));
	palette_set_color(machine, 0xf8,MAKE_RGB(0x3f,0x3f,0x3f));
	palette_set_color(machine, 0xf9,MAKE_RGB(0x3f,0x3f,0xff));
	palette_set_color(machine, 0xfa,MAKE_RGB(0x3f,0xff,0x3f));
	palette_set_color(machine, 0xfb,MAKE_RGB(0x3f,0xff,0xff));
	palette_set_color(machine, 0xfc,MAKE_RGB(0xff,0x3f,0x10));
	palette_set_color(machine, 0xfd,MAKE_RGB(0xff,0x3f,0xff));
	palette_set_color(machine, 0xfe,MAKE_RGB(0xff,0xff,0x3f));
	palette_set_color(machine, 0xff,MAKE_RGB(0xff,0xff,0xff));
}

#define HBLANK_1 if(!(hv_blank & 1)) hv_blank^= 1;
#define HBLANK_0 if(hv_blank & 1) hv_blank^= 1;

#define VBLANK_1 if(!(hv_blank & 8)) hv_blank^= 8;
#define VBLANK_0 if(hv_blank & 8) hv_blank^= 8;

static INTERRUPT_GEN( filetto_irq )
{
	/*TODO: Timings are guessed*/
	/*H-Blank*/
	if((cpu_getiloops() % 8) == 0){ HBLANK_1; }
	else						  { HBLANK_0; }

	/*V-Blank*/
	if(cpu_getiloops() >= 180) 	  { VBLANK_1; }
	else						  { VBLANK_0; }
}


static MACHINE_DRIVER_START( filetto )
	MDRV_CPU_ADD_TAG("main", I8088, 8000000)
	MDRV_CPU_PROGRAM_MAP(filetto_map,0)
	MDRV_CPU_IO_MAP(filetto_io,0)
	MDRV_CPU_VBLANK_INT(filetto_irq,200)

	MDRV_GFXDECODE(filetto)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 640-1, 0*8, 480-1)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_MACHINE_RESET(filetto)
	MDRV_PALETTE_INIT(filetto)

	MDRV_VIDEO_START(filetto)
	MDRV_VIDEO_UPDATE(filetto)
MACHINE_DRIVER_END


ROM_START( filetto )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )

	ROM_LOAD("u49.bin", 0xfc000, 0x2000, CRC(1be6948a) SHA1(9c433f63d347c211ee4663f133e8417221bc4bf0))
	ROM_RELOAD(         0xf8000, 0x2000 )
	ROM_RELOAD(         0xf4000, 0x2000 )
	ROM_RELOAD(         0xf0000, 0x2000 )
	ROM_LOAD("u55.bin", 0xfe000, 0x2000, CRC(1e455ed7) SHA1(786d18ce0ab1af45fc538a2300853e497488f0d4) )
	ROM_RELOAD(         0xfa000, 0x2000 )
	ROM_RELOAD(         0xf6000, 0x2000 )
	ROM_RELOAD(         0xf2000, 0x2000 )


	ROM_REGION( 0x40000, REGION_USER1, 0 ) // program data
	ROM_LOAD( "m0.u1", 0x00000, 0x10000, CRC(2408289d) SHA1(eafc144a557a79b58bcb48545cb9c9778e61fcd3) )
	ROM_LOAD( "m1.u2", 0x10000, 0x10000, CRC(5b623114) SHA1(0d9a14e6b7f57ce4fa09762343b610a973910f58) )
	ROM_LOAD( "m2.u3", 0x20000, 0x10000, CRC(abc64869) SHA1(564fc9d90d241a7b7776160b3fd036fb08037355) )
	ROM_LOAD( "m3.u4", 0x30000, 0x10000, CRC(0c1e8a67) SHA1(f1b9280c65fcfcb5ec481cae48eb6f52d6cdbc9d) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD("u67.bin", 0x0000, 0x2000, CRC(09710122) SHA1(de84bdd9245df287bbd3bb808f0c3531d13a3545) )

	ROM_REGION( 0x40000, REGION_USER2, ROMREGION_DISPOSE ) // unknown
	ROM_LOAD16_BYTE("v1.u15",  0x00000, 0x20000, CRC(613ddd07) SHA1(ebda3d559315879819cb7034b5696f8e7861fe42) )
	ROM_LOAD16_BYTE("v2.u14",  0x00001, 0x20000, CRC(427e012e) SHA1(50514a6307e63078fe7444a96e39d834684db7df) )
ROM_END

static DRIVER_INIT( filetto )
{
	//...
}

GAME( 1990, filetto,    0, filetto,    filetto,   filetto, ROT0,  "Novarmatic", "Filetto (v1.05 901009)",GAME_NOT_WORKING|GAME_NO_SOUND )

