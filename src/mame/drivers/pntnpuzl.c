/* paint & puzzle */
/* video is standard VGA */
/*
OK, here's a somewhat complete rundown of the PCB. There was 1 IC I didn't
get a pin count of(oops).

Main PCB
Reb B
Label:
Board Num.
90

The PCB has no silkscreen or reference designators, so the numbers I am
providing are made up.

U1 32 pin IC
27C010A
Label:
Paint N Puzzle
Ver. 1.09
Odd

U2 5 pin audio amp
LM383T

U3 40 pin IC
8926S
UM6522A

U4 28 pin IC
Mosel
MS62256L-85PC
8911 5D

U5 18 pin IC
PIC16C54-HS/P
9344 CGA

U6
P8798
L3372718E
Intel
Label:
MicroTouch
5603670 REV 1.0

U7 28 pin IC
MicroTouch Systems
c 1992
19-507 Rev 2
ICS1578N 9334

U8 28 pin IC
Mosel
MS62256L-85PC
8911 5D

U9 32 pin IC
27C010A
Label:
Paint N Puzzle
Ver. 1.09
Even

U10 64 pin IC
MC68000P12
OB26M8829

X1
16.000MHz -connected to U5

X2
ECS-120
32-1
China -connected to U7
Other side is unreadable

CN1 JAMMA
CN2 ISA? Video card slot
CN3 Touchscreen input (12 pins)
CN4 2 pins, unused

1 blue potentiometer connected to audio amp
There doesnt seem to be any dedicated sound chip, and sounds are just bleeps
really.

-----------------------------------------------
Video card (has proper silk screen and designators)
JAX-8327A

X1
40.000MHz

J1 -open
J2 -closed
J3 -open

U1/2 unpopulated but have sockets

U3 20 pin IC
KM44C256BT-8
22BY
Korea

U4 20 pin IC
KM44C256BT-8
22BY
Korea

U5 160 pin QFP
Trident
TVGA9000i
34'3 Japan

U6 28 pin IC
Quadtel
TVGA9000i BIOS Software
c 1993 Ver D3.51 LH

CN1 standard DB15 VGA connector (15KHz)
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"


static const eeprom_interface eeprom_intf =
{
	6,				/* address bits */
	16,				/* data bits */
	"*110",			/*  read command */
	"*101",			/* write command */
	NULL,			/* erase command */
	"*10000xxxx",	/* lock command */
	"*10011xxxx"	/* unlock command */
};

static UINT16 pntnpuzl_eeprom;

static READ16_DEVICE_HANDLER( pntnpuzl_eeprom_r )
{
	/* bit 11 is EEPROM data */
	return (pntnpuzl_eeprom & 0xf4ff) | (eeprom_read_bit(device)<<11) | (input_port_read(device->machine, "IN1") & 0x0300);
}

static WRITE16_DEVICE_HANDLER( pntnpuzl_eeprom_w )
{
	pntnpuzl_eeprom = data;

	/* bit 12 is data */
	/* bit 13 is clock (active high) */
	/* bit 14 is cs (active high) */

	eeprom_write_bit(device, data & 0x1000);
	eeprom_set_cs_line(device, (data & 0x4000) ? CLEAR_LINE : ASSERT_LINE);
	eeprom_set_clock_line(device, (data & 0x2000) ? ASSERT_LINE : CLEAR_LINE);
}




static UINT16* pntnpuzl_3a0000ram;
static UINT16* pntnpuzl_bank;
/* vid */
static VIDEO_START( pntnpuzl )
{
	pntnpuzl_3a0000ram=auto_alloc_array(machine, UINT16, 0x100000/2);
}

static VIDEO_UPDATE( pntnpuzl )
{
	int x,y;
	int count;
#if 0
	static int xxx=0x18f;
	static int yyy=512;
	static int sss=0xa8;

	if ( input_code_pressed_once(screen->machine, KEYCODE_Q) )
	{
		xxx--;
		mame_printf_debug("xxx %04x\n",xxx);
	}

	if ( input_code_pressed_once(screen->machine, KEYCODE_W) )
	{
		xxx++;
		mame_printf_debug("xxx %04x\n",xxx);
	}

	if ( input_code_pressed_once(screen->machine, KEYCODE_A) )
	{
		yyy--;
		mame_printf_debug("yyy %04x\n",yyy);
	}

	if ( input_code_pressed_once(screen->machine, KEYCODE_S) )
	{
		yyy++;
		mame_printf_debug("yyy %04x\n",yyy);
	}

	if ( input_code_pressed_once(screen->machine, KEYCODE_Z) )
	{
		sss--;
		mame_printf_debug("sss %04x\n",sss);
	}

	if ( input_code_pressed_once(screen->machine, KEYCODE_X) )
	{
		sss++;
		mame_printf_debug("sss %04x\n",sss);
	}
#else
	const int xxx=0x18f;
	const int yyy=512;
	const int sss=0xa8;
#endif


	count=sss;

	for(y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x+=2)
		{
			*BITMAP_ADDR16(bitmap, y, x) = (pntnpuzl_3a0000ram[count]&0x1f00)>>8;
			*BITMAP_ADDR16(bitmap, y, x+1) = (pntnpuzl_3a0000ram[count]&0x001f)>>0;
			count++;
		}
	}
	return 0;
}

static WRITE16_HANDLER( pntnpuzl_palette_w )
{
	static int indx,sub,rgb[3];

	if (ACCESSING_BITS_8_15)
	{
		indx = data >> 8;
		sub = 0;
	}
	if (ACCESSING_BITS_0_7)
	{
		rgb[sub++] = data & 0xff;
		if (sub == 3)
		{
			palette_set_color_rgb(space->machine,indx++,pal6bit(rgb[0]),pal6bit(rgb[1]),pal6bit(rgb[2]));
			sub = 0;
			if (indx == 256) indx = 0;
		}
	}
}



#ifdef UNUSED_FUNCTION
READ16_HANDLER ( pntnpuzl_random_r )
{
	return mame_rand(space->machine);
}
#endif

static READ16_HANDLER( pntnpuzl_vid_r )
{
//  logerror("read_videoram: pc = %06x : offset %04x reg %04x\n",cpu_get_pc(space->cpu),offset*2, pntnpuzl_bank[0]);
	return pntnpuzl_3a0000ram[offset+ (pntnpuzl_bank[0]&0x0001)*0x8000 ];
}

static WRITE16_HANDLER( pntnpuzl_vid_w )
{
//  logerror("write_to_videoram: pc = %06x : offset %04x data %04x reg %04x\n",cpu_get_pc(space->cpu),offset*2, data, pntnpuzl_bank[0]);
	COMBINE_DATA(&pntnpuzl_3a0000ram[offset+ (pntnpuzl_bank[0]&0x0001)*0x8000 ]);
}

static READ16_HANDLER( pntnpuzl_vblank_r )
{
	return (input_port_read(space->machine, "IN0") & 1) << 11;
}



/*
reading works this way:
280016 = 00
28001A = 08
wait for bit 3 of 28001A to be 1 (after a timeout, fail)
280010 = 3d
280012 = 00
280016 = 04
read 280014 (throw away result)
wait for bit 2 of 28001A to be 1
read data from 280014

during startup it expects this series:
write                                     read
01 52 0d <pause> 01 50 4e 38 31 0d   ---> 80 0c
01 4d 51 0d                          ---> 80 0c
01 46 54 0d                          ---> 80 0c
01 46 4e 30 38 0d                    ---> 80 0c
01 53 45 32 0d                       ---> 80 0c
01 03 46 31 38 0d                    ---> 80 0c
*/
static UINT16 pntpzl_200000, serial, serial_out,read_count;

static WRITE16_HANDLER( pntnpuzl_200000_w )
{
// logerror("200000: %04x\n",data);
	// bit 12: set to 1 when going to serial output to 280018
	if ((pntpzl_200000 & 0x1000) && !(data & 0x1000))
	{
		serial_out = (serial>>1) & 0xff;
		read_count = 0;
		logerror("serial out: %02x\n",serial_out);
	}

	pntpzl_200000 = data;
}

static WRITE16_HANDLER( pntnpuzl_280018_w )
{
// logerror("%04x: 280018: %04x\n",cpu_get_pc(space->cpu),data);
	serial >>= 1;
	if (data & 0x2000)
		serial |= 0x400;
}

static READ16_HANDLER( pntnpuzl_280014_r )
{
	static const int startup[3] = { 0x80, 0x0c, 0x00 };
	int res;

	if (serial_out == 0x11)
	{
		static int touchscr[5];
		if (input_port_read(space->machine, "IN0") & 0x10)
		{
			touchscr[0] = 0x1b;
			touchscr[2] = BITSWAP8(input_port_read(space->machine, "TOUCHX"),0,1,2,3,4,5,6,7);
			touchscr[4] = BITSWAP8(input_port_read(space->machine, "TOUCHY"),0,1,2,3,4,5,6,7);
		}
		else
			touchscr[0] = 0;

		if (read_count >= 10) read_count = 0;
		res = touchscr[read_count/2];
		read_count++;
	}
	else
	{
		if (read_count >= 6) read_count = 0;
		res = startup[read_count/2];
		read_count++;
	}
	logerror("read 280014: %02x\n",res);
	return res << 8;
}

static READ16_HANDLER( pntnpuzl_28001a_r )
{
	return 0x4c00;
}



static ADDRESS_MAP_START( pntnpuzl_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READNOP //|
	AM_RANGE(0x100000, 0x100001) AM_READNOP	//| irq lines clear
	AM_RANGE(0x180000, 0x180001) AM_READNOP //|
	AM_RANGE(0x200000, 0x200001) AM_WRITE(pntnpuzl_200000_w)
	AM_RANGE(0x280000, 0x280001) AM_DEVREAD("eeprom", pntnpuzl_eeprom_r)
	AM_RANGE(0x280002, 0x280003) AM_READ_PORT("IN2")
	AM_RANGE(0x280000, 0x280001) AM_DEVWRITE("eeprom", pntnpuzl_eeprom_w)
	AM_RANGE(0x280008, 0x280009) AM_WRITENOP
	AM_RANGE(0x28000a, 0x28000b) AM_WRITENOP
	AM_RANGE(0x280010, 0x280011) AM_WRITENOP
	AM_RANGE(0x280012, 0x280013) AM_WRITENOP
	AM_RANGE(0x280014, 0x280015) AM_READ(pntnpuzl_280014_r)
	AM_RANGE(0x280016, 0x280017) AM_WRITENOP
	AM_RANGE(0x280018, 0x280019) AM_WRITE(pntnpuzl_280018_w)
	AM_RANGE(0x28001a, 0x28001b) AM_READ(pntnpuzl_28001a_r)
	AM_RANGE(0x28001a, 0x28001b) AM_WRITENOP

	/* standard VGA */
	AM_RANGE(0x3a0000, 0x3affff) AM_READWRITE(pntnpuzl_vid_r, pntnpuzl_vid_w)
	AM_RANGE(0x3c03c4, 0x3c03c5) AM_RAM AM_BASE(&pntnpuzl_bank)//??
	AM_RANGE(0x3c03c8, 0x3c03c9) AM_WRITE(pntnpuzl_palette_w)
	AM_RANGE(0x3c03da, 0x3c03db) AM_READ(pntnpuzl_vblank_r)

	AM_RANGE(0x400000, 0x407fff) AM_RAM
ADDRESS_MAP_END


static INTERRUPT_GEN( pntnpuzl_irq )
{
	if (input_port_read(device->machine, "IN0") & 0x02)	/* coin */
		generic_pulse_irq_line(device, 1);
	else if (input_port_read(device->machine, "IN0") & 0x04)	/* service */
		generic_pulse_irq_line(device, 2);
	else if (input_port_read(device->machine, "IN0") & 0x08)	/* coin */
		generic_pulse_irq_line(device, 4);
}

static INPUT_PORTS_START( pntnpuzl )
	PORT_START("IN0")	/* fake inputs */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_HIGH ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	/* game uses a touch screen */
	PORT_START("TOUCHX")
	PORT_BIT( 0x7f, 0x40, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,0x7f) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCHY")
	PORT_BIT( 0x7f, 0x40, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_MINMAX(0,0x7f) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)

	PORT_START("IN2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
INPUT_PORTS_END



static MACHINE_DRIVER_START( pntnpuzl )
	MDRV_CPU_ADD("maincpu", M68000, 12000000)//??
	MDRV_CPU_PROGRAM_MAP(pntnpuzl_map)
	MDRV_CPU_VBLANK_INT("screen", pntnpuzl_irq)	// irq1 = coin irq2 = service irq4 = coin

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 50*8-1, 0*8, 30*8-1)

	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(pntnpuzl)
	MDRV_VIDEO_UPDATE(pntnpuzl)
MACHINE_DRIVER_END

ROM_START( pntnpuzl )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "pntnpuzl.u2", 0x00001, 0x40000, CRC(dfda3f73) SHA1(cca8ccdd501a26cba07365b1238d7b434559bbc6) )
	ROM_LOAD16_BYTE( "pntnpuzl.u3", 0x00000, 0x40000, CRC(4173f250) SHA1(516fe6f91b925f71c36b97532608b82e63bda436) )
ROM_END


static DRIVER_INIT(pip)
{
//  UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");
//  rom[0x2696/2] = 0x4e71;
//  rom[0x26a0/2] = 0x4e71;
}

GAME( 199?, pntnpuzl,    0, pntnpuzl,    pntnpuzl,    pip, ROT90,  "Century?", "Paint & Puzzle",GAME_NO_SOUND|GAME_NOT_WORKING )
