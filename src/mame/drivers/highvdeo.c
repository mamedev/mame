/*************************************************************************************************

High Video Tour 4000

driver by Mirko Buffoni


Memory layout:

000000-0003FF   Interrupt table
000400-003FFF   NVRAM and main ram
040000-04FFFF   VGA 320x200 address space
080000-0BFFFF   Banked ROM
0C0000-0FFFFF   Program ROM


Port layout:

0x0008 R    Input port -> Keyboard
0x000A R    Input port -> Coin and Service
0x000C R    Input port -> Reset

0x0030 R    Read continuously... maybe watchdog?

0x0000 W    Keyboard Lights control port
0x0002 W    \ Hopper or ticket related
0x0004 W    /
0x0006 W    OKI6395 ADPCM command:  need to be latched
0x0010 W    Like 0x3c8 in VGA
0x0014 W    Like 0x3c9 in VGA

0x0030 W    Bankswitch select

----

INT 2 (NMI) called every Vblank

----

Interesting locations.  255 = YES

3E23-24     Valore Moneta   (5)  (1,5,10)
3E25-26     Valore Gettone  (5)  (1-20)
3E27-28     Valore Servizio (10) (5-500)
3E29-2A     Banconote 1     (5)  (5-500)

3E33        Replay          (255) (0,255)
3E34        Double          (0)   (0,255)
3E35        BloccaBanconote (255) (0,255)
3E36        Accumulo        (0)   (0,255)
3E37        Vincita 10      (255) (0,255)
3E38        Numeroni        (255) (0,255)
3E39        Palline         (255) (0,255)
3E3B        Lattine         (255) (0,255)
3E3D        Premio          (10)  (X,10)
3E3E        Bet Max Credit  (20)  (1-50)
3E3F        Bet Max Points  (20)  (1-50)

3E40-41     Blocco Getton.  (100) (10-1000)
3E42        Cambio Carte    (0)   (Veloce=0, Normale=1, Lento=2)
3E45-46     Valore ticket   (100) (1-500)
3E4B        Bet Min Gioco   (1)   (1-10)
3E4C        Bet Min Fever   (1)   (1-10)

3E59        Tickets         (10)  (Tutti=0, 10=F, 1=FF)

----

Initial High Video releases have roms named 'vcf'...
They have low resolution 320x200x256 colors.
Game is V30 based, with rom banking

Next, they released new board with roms named 'ncf'...
Same resolution, but different mapping for memory and input ports, plus a check for vblank (protection?)
Game is V30 based, without banking

Newer boards instead have roms named 'tcf'...
Resolution is higher as 400x300x256 colors, and graphic is fancier.
There is a simple protection check, tied on an input port.
Game is V30 based, with rom banking (2Mb)

*************************************************************************************************/

#include "driver.h"
#include "cpu/nec/nec.h"
#include "sound/okim6376.h"

static UINT16 *blit_ram;


VIDEO_START(tourvisn)
{

}

VIDEO_UPDATE(tourvisn)
{
	int x,y,count;

	count = (0/2);

	for(y=0;y<(video_screen_get_visible_area(screen)->max_y+1);y++)
	{
		for(x=0;x<(video_screen_get_visible_area(screen)->max_x+1)/2;x++)
		{
			UINT32 color;

			color = ((blit_ram[count]) & 0x00ff)>>0;

			if((x*2)<video_screen_get_visible_area(screen)->max_x && ((y)+0)<video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, (x*2)+0) = screen->machine->pens[color];

			color = ((blit_ram[count]) & 0xff00)>>8;

			if(((x*2)+1)<video_screen_get_visible_area(screen)->max_x && ((y)+0)<video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, (x*2)+1) = screen->machine->pens[color];

			count++;
		}
	}

	return 0;
}



static READ16_HANDLER( read1_r )
{
	return input_port_read(space->machine, "IN0");
}

static READ16_HANDLER( read2_r )
{
	return input_port_read(space->machine, "IN1");
}

static READ16_HANDLER( read3_r )
{
	return input_port_read(space->machine, "IN2");
}

static WRITE16_HANDLER( tv_vcf_paletteram_w )
{
	static int pal_offs,r,g,b,internal_pal_offs;

	switch(offset*2)
	{
		case 0:
			pal_offs = 0;
			break;
		case 2:
			internal_pal_offs = 0;
			break;
		case 4:
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
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}

			break;
	}
}

static WRITE16_HANDLER( tv_vcf_bankselect_w )
{
	static UINT32 bankaddress;
	UINT8 *ROM = memory_region(space->machine, "user1");

	/* bits 0, 1 select the ROM bank */
	bankaddress = (data & 0x03) * 0x40000;

	memory_set_bankptr(space->machine, 1, &ROM[bankaddress]);
}


static WRITE16_DEVICE_HANDLER( tv_oki6395_w )
{
	static int okidata;
	if (ACCESSING_BITS_0_7 && okidata != data) {
		okidata = data;
		okim6376_w(device, 0, data);
		okim6376_w(device, 0, (1 << 4));
	}
}


static ADDRESS_MAP_START( tv_vcf_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x003ff) AM_RAM /*irq vector area*/
	AM_RANGE(0x00400, 0x03fff) AM_RAM AM_BASE( &generic_nvram16 ) AM_SIZE( &generic_nvram_size )
	AM_RANGE(0x40000, 0x4ffff) AM_RAM AM_BASE(&blit_ram) /*blitter ram*/
	AM_RANGE(0x80000, 0xbffff) AM_ROMBANK(1)
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tv_vcf_io, ADDRESS_SPACE_IO, 16 )
 	AM_RANGE(0x0006, 0x0007) AM_DEVWRITE( SOUND, "oki", tv_oki6395_w )
 	AM_RANGE(0x0008, 0x0009) AM_READ( read1_r )
	AM_RANGE(0x000a, 0x000b) AM_READ( read2_r )
	AM_RANGE(0x000c, 0x000d) AM_READ( read3_r )
 	AM_RANGE(0x0010, 0x0015) AM_WRITE( tv_vcf_paletteram_w )
 	AM_RANGE(0x0030, 0x0031) AM_WRITE( tv_vcf_bankselect_w ) AM_DEVREAD8( SOUND, "oki", okim6376_r, 0x00ff )
ADDRESS_MAP_END


static READ16_HANDLER( tv_ncf_read2_r )
{
	static int resetpulse = 0;

	// Bit 6 of port 1 is connected to clock impulse, as heartbeat.  If impulse cease
	// machine resets itself.
	resetpulse ^= 0x40;

	return (input_port_read(space->machine, "IN1") & 0xbf) | resetpulse;
}

static WRITE16_DEVICE_HANDLER( tv_ncf_oki6395_w )
{
	static int okidata;
	if (ACCESSING_BITS_0_7 && okidata != data) {
		okidata = data;
		okim6376_w(device, 0, data | 0x80);
		okim6376_w(device, 0, (1 << 4));
	}
}
static ADDRESS_MAP_START( tv_ncf_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x003ff) AM_RAM /*irq vector area*/
	AM_RANGE(0x00400, 0x03fff) AM_RAM AM_BASE( &generic_nvram16 ) AM_SIZE( &generic_nvram_size )
	AM_RANGE(0x20000, 0x2ffff) AM_RAM AM_BASE(&blit_ram) /*blitter ram*/
	AM_RANGE(0x40000, 0xbffff) AM_ROM AM_REGION("user1",0x40000)
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tv_ncf_io, ADDRESS_SPACE_IO, 16 )
 	AM_RANGE(0x0008, 0x0009) AM_DEVWRITE( SOUND, "oki", tv_ncf_oki6395_w )
 	AM_RANGE(0x000c, 0x000d) AM_READ( read1_r )
	AM_RANGE(0x0010, 0x0011) AM_READ( tv_ncf_read2_r )
	AM_RANGE(0x0012, 0x0013) AM_READ( read3_r )
 	AM_RANGE(0x0030, 0x0035) AM_WRITE( tv_vcf_paletteram_w )
ADDRESS_MAP_END


static WRITE16_HANDLER( tv_tcf_paletteram_w )
{
	int r, g, b, color;

	COMBINE_DATA(&paletteram16[offset]);

	color = paletteram16[offset];
	r = (color >> 8) & 0xf8;
	g = (color >> 3) & 0xf8;
	b = (color << 3) & 0xf8;

	palette_set_color_rgb(space->machine, offset, r, g, b);
}

static WRITE16_HANDLER( tv_tcf_bankselect_w )
{
	static UINT32 bankaddress;
	UINT8 *ROM = memory_region(space->machine, "user1");

	/* bits 0, 1, 2 select the ROM bank */
	bankaddress = (data & 0x07) * 0x40000;

	memory_set_bankptr(space->machine, 1, &ROM[bankaddress]);
}

static ADDRESS_MAP_START( tv_tcf_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x003ff) AM_RAM /*irq vector area*/
	AM_RANGE(0x00400, 0x03fff) AM_RAM AM_BASE( &generic_nvram16 ) AM_SIZE( &generic_nvram_size )
	AM_RANGE(0x40000, 0x5d4bf) AM_RAM AM_BASE(&blit_ram) /*blitter ram*/
	AM_RANGE(0x7fe00, 0x7ffff) AM_RAM_WRITE( tv_tcf_paletteram_w ) AM_BASE(&paletteram16)
	AM_RANGE(0x80000, 0xbffff) AM_ROMBANK(1)
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tv_tcf_io, ADDRESS_SPACE_IO, 16 )
 	AM_RANGE(0x0006, 0x0007) AM_DEVWRITE( SOUND, "oki", tv_oki6395_w )
 	AM_RANGE(0x0008, 0x0009) AM_READ( read1_r )
	AM_RANGE(0x000a, 0x000b) AM_READ( read2_r )
	AM_RANGE(0x0030, 0x0031) AM_READ( read3_r ) AM_WRITE( tv_tcf_bankselect_w )
ADDRESS_MAP_END

static INPUT_PORTS_START( tv_vcf )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Take Button") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Start") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Risk Button") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // Note 1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Ticket")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Diagnostics") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Hopper")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Key") PORT_CODE(KEYCODE_9)
	PORT_DIPNAME( 0x0002, 0x0000, "Reset NVRAM" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tv_ncf )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Take Button") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Start") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Risk Button") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // Note 1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER )		/* connected to the clock signal, to signal heartbeat */
	PORT_DIPNAME( 0x0080, 0x0000, "Reset NVRAM" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Diagnostics") PORT_CODE(KEYCODE_F2)
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END

static INPUT_PORTS_START( tv_tcf )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Take Button") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Start") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Risk Button") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // Note 1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Ticket")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Diagnostics") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Hopper")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("IN2")
	PORT_BIT( 0x0003, 0x0002, IPT_OTHER ) // Protection
	PORT_DIPNAME( 0x0004, 0x0000, "Reset NVRAM" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INTERRUPT_GEN( vblank_irq )
{
	cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x08/4);
}

static MACHINE_DRIVER_START( tv_vcf )
	MDRV_CPU_ADD("main", V30, XTAL_12MHz/2 )	// ?
	MDRV_CPU_PROGRAM_MAP(tv_vcf_map,0)
	MDRV_CPU_IO_MAP(tv_vcf_io,0)
	MDRV_CPU_VBLANK_INT("main", vblank_irq)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(400, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(tourvisn)
	MDRV_VIDEO_UPDATE(tourvisn)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	//OkiM6376
	MDRV_SOUND_ADD("oki", OKIM6376, XTAL_12MHz/2/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tv_ncf )
	MDRV_IMPORT_FROM(tv_vcf)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(tv_ncf_map,0)
	MDRV_CPU_IO_MAP(tv_ncf_io,0)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tv_tcf )
	MDRV_IMPORT_FROM(tv_vcf)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(tv_tcf_map,0)
	MDRV_CPU_IO_MAP(tv_tcf_io,0)

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 300-1)
MACHINE_DRIVER_END


ROM_START( tour4000 )
	ROM_REGION( 0x100000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "vcfi18.bin", 0x00001, 0x80000, CRC(8c83cd34) SHA1(a94bdfdb74d047ac3851f2aef295a37c93b091f2) )
	ROM_LOAD16_BYTE( "vcfi17.bin", 0x00000, 0x80000, CRC(bcae57ed) SHA1(13c02cae59ed5cc0847a7827a315902066b03190) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x0c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever40 )
	ROM_REGION( 0x100000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "vcfi48.bin", 0x00001, 0x80000, CRC(5a86a642) SHA1(fd927bc393242ff0aca87a0e3c2127f6f1df09cd) )
	ROM_LOAD16_BYTE( "vcfi47.bin", 0x00000, 0x80000, CRC(e7adc4d8) SHA1(862041c2c5d260727e525ab85fde18994484db16) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x0c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever50 )
	ROM_REGION( 0x100000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "vcfi58.bin", 0x00001, 0x80000, CRC(c3464934) SHA1(1672c34d9ca250769973f7bc739137f153552eb9) )
	ROM_LOAD16_BYTE( "vcfi57.bin", 0x00000, 0x80000, CRC(2b789acb) SHA1(782ad3a6e0eacbf9adec4afd20a309215913e505) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x0c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( tour4010 )
	ROM_REGION( 0x100000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "ncfi18.bin", 0x00001, 0x80000, CRC(294929d9) SHA1(712926cf1f78197fce838a4e76d70082182214eb) )
	ROM_LOAD16_BYTE( "ncfi17.bin", 0x00000, 0x80000, CRC(4a8ac279) SHA1(41b0de4444466700ef2de2e926c8fa6f0bda280d) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x0c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever51 )
	ROM_REGION( 0x100000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "ncfi58.bin", 0x00001, 0x80000, CRC(cdf9c2f0) SHA1(94f9cf6b1856becd74971022ded6db5ae927fb54) )
	ROM_LOAD16_BYTE( "ncfi57.bin", 0x00000, 0x80000, CRC(5005cf2b) SHA1(468ccd27fcb8bdb7d6ccf423542e1d4773930b88) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x0c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever61 )
	ROM_REGION( 0x100000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "ncfi68.bin", 0x00001, 0x80000, CRC(51fe839f) SHA1(e4d9bce4a995cb407faaf36b2c1e10409a2e94da) )
	ROM_LOAD16_BYTE( "ncfi67.bin", 0x00000, 0x80000, CRC(d889d6b6) SHA1(791d9b9fc2d0a128ab07a9ae18a32f2838a5ea3f) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x0c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever1k )
	ROM_REGION( 0x200000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "tcfi28.bin", 0x00001, 0x100000, CRC(e38d115a) SHA1(7fec94ddcdb07e483ed2f0d7d667c35ceb7a1f44) )
	ROM_LOAD16_BYTE( "tcfi27.bin", 0x00000, 0x100000, CRC(32f884e6) SHA1(cc74a4c6313654bbd363a89fe7757a05c74de45b) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x1C0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( girotutt )
	ROM_REGION( 0x200000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "tcfi18.bin", 0x00001, 0x100000, CRC(822ab6a1) SHA1(04f4254da46cf67ea17587fde4a0fdd39c658b3b) )
	ROM_LOAD16_BYTE( "tcfi17.bin", 0x00000, 0x100000, CRC(b326a0ee) SHA1(c96b7578c112a97ba1d8de4d3d0ae68fef846cad) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x1C0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "t41.bin", 0x00000, 0x80000, CRC(6f694406) SHA1(ec8b8baba0ee1bfe8986ce978412ee4de06f1906) )
ROM_END

GAME( 2000, tour4000,  0,      tv_vcf,   tv_vcf,   0,       ROT0,  "High Video", "Tour 4000",         0 )
GAME( 2000, cfever40,  0,      tv_vcf,   tv_vcf,   0,       ROT0,  "High Video", "Casino Fever 4.0",  0 )
GAME( 2000, cfever50,  0,      tv_vcf,   tv_vcf,   0,       ROT0,  "High Video", "Casino Fever 5.0",  0 )
GAME( 2000, tour4010,  0,      tv_ncf,   tv_ncf,   0,       ROT0,  "High Video", "Tour 4010",         0 )
GAME( 2000, cfever51,  0,      tv_ncf,   tv_ncf,   0,       ROT0,  "High Video", "Casino Fever 5.1",  0 )
GAME( 2000, cfever61,  0,      tv_ncf,   tv_ncf,   0,       ROT0,  "High Video", "Casino Fever 6.1",  0 )
GAME( 2000, cfever1k,  0,      tv_tcf,   tv_tcf,   0,       ROT0,  "High Video", "Casino Fever 1k",   0 )
GAME( 2000, girotutt,  0,      tv_tcf,   tv_tcf,   0,       ROT0,  "High Video", "GiroTutto",         0 )
