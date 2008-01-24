/*====================================================================
                        Lock-On (Tatsumi 1986)
                        ======================

                 Preliminary Driver by Philip J Bennett
                 ======================================

To do
======

Almost everything video related :)

o Scene tilemap scrolling
o Colours
o Objects
o HUD layer
o Ground layer
o Layer mixing

=====================================================================*/

#include "driver.h"
#include "sound/2203intf.h"

static UINT16 *lockon_vram0;
static UINT16 *lockon_vram1;
static UINT16 *lockon_vram2;
static UINT16 *object_ram;
static UINT16 *ground_ram;
static UINT8 *z80_ram;
static UINT16 *v30_gnd;
static UINT16 *v30_obj;
static UINT16 *clut_ram;

static int v30_obj_addr=0;
static int v30_gnd_addr=0;
static int main_inten=0;

static size_t objectram_size;
static size_t lockon_ground_size;

static tilemap *lockon_tilemap0;
static tilemap *lockon_tilemap1;
static tilemap *lockon_tilemap2;


/*
Three sources are mixed at the RGB output PROMs:

* Characters
* HUD
* Objects/scene/ground

A = !( ((VCDDB6 || VCDDB1) && !VCDDB7) || HUDPT0 )
B = !( (!HUDPT0 || !VCDDB7) && (VCDDB6 || VCDDB1) )

A B   Layer                  Colours
0 0   Characters             0-255
0 1   Characters             256-511
1 0   HUD                    512-767
1 1   Objects/scene/ground   768-1023
*/

/* Very preliminary! */
static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
        int offs=0;
        int index_x=0, index_y=0;

        UINT16 *ROM_LUTA = (UINT16 *)memory_region(REGION_USER1);
        UINT8  *ROM_LUTB = (UINT8 *)memory_region(REGION_USER2);
        UINT8  *ROM_LUTC = (UINT8 *)memory_region(REGION_USER2)+0x8000;   // Selected by bit 7...

       	for (offs = 0x0; offs <= (objectram_size/2); offs += 4)
		{
     	  int offs0 = (object_ram[offs+3] & 0x3f) << 8;              // Lower 6-bits go into upper ROM portion
     	  int bit7  = (object_ram[offs+3] >> 7) & 0x1;      // Selects which ROM to use in calculating lookup
          int lut;

		  if(!object_ram[offs+2])
		  	continue;
			if((object_ram[offs+3] >> 8) == 0xfc || (object_ram[offs+3] >> 8) == 0xff)   // ?
				break;

          if(!bit7)
              lut = (ROM_LUTC[offs0 | (index_y<<4) | index_x]) & 0x7f;
          else
              lut = (ROM_LUTB[offs0 | (index_y<<4) | index_x]) & 0x7f;

        // Bits 12,13,14 = bank
        // Bits 0-11 obj
        for(index_y=0; index_y<8; index_y++)
        {
			int index_x;
			for(index_x=0; index_x<8; index_x++)
			{

				/* Index into the object lookup table */
				int ref = object_ram[offs+2] + lut;

				int pix = ROM_LUTA[ref+index_x+(16*index_y)];       // ???

				int flip_x = 0;//(object_ram[offs+1] >> 7) & 0x1;  // Bit 15
				int flip_y = 0;//(object_ram[offs+1] >> 6) & 0x1;  // Bit 14

				int scale_x = 0xffff; //+ object_ram[offs+1] & 0xff; //0x1ffff;
				int scale_y = 0xffff; //+ object_ram[offs+1] >> 8; //0x1ffff;

				int sy = 255-(object_ram[offs] & 0xff)+(index_y*8)*(scale_y/0xffff);
				int sx = (object_ram[offs] >> 8)+(index_x*8)*(scale_x/0xffff);

				int bank = ((pix >> 12) & 0x7);
				int index = pix & 0xfff;

				int color = 1;

				const gfx_element *gfx = machine->gfx[bank];

				drawgfxzoom(bitmap, gfx,
					index,
					color,
					flip_x,flip_y,            // FlipX, flipY
					sx,sy,
					cliprect,TRANSPARENCY_PEN,1,
				                 scale_x,scale_y);
			}
        }
	}
}

/* Characters */
static WRITE16_HANDLER( lockon_vram_0_w )
{
    COMBINE_DATA(&lockon_vram0[offset]);
   	tilemap_mark_tile_dirty(lockon_tilemap0,offset);
}

/* Scene */
static WRITE16_HANDLER( lockon_vram_1_w )
{
	COMBINE_DATA(&lockon_vram1[offset]);
	tilemap_mark_tile_dirty(lockon_tilemap1,offset);
}

/* HUD */
static WRITE16_HANDLER( lockon_vram_2_w )
{
	COMBINE_DATA(&lockon_vram2[offset]);
	tilemap_mark_tile_dirty(lockon_tilemap2,offset);
}

/* 8*8 characters */
static TILE_GET_INFO( get_lockon_tile_info0 )
{
	int tileno, color;
	tileno = lockon_vram0[tile_index];
	color = 0;

	SET_TILE_INFO(4,tileno,color,0);
}

/* Scene tiles */
static TILE_GET_INFO( get_lockon_tile_info1 )
{
  	int tileno, color;
	tileno = lockon_vram1[tile_index];
	color = 0;

	SET_TILE_INFO(5,tileno,color,0);
}

/* HUD tiles */
static TILE_GET_INFO( get_lockon_tile_info2 )
{
  	int tileno, color;
	tileno = lockon_vram2[tile_index];
	color = 0;

	SET_TILE_INFO(6,tileno,color,0);
}


static VIDEO_START( lockon )
{
	lockon_tilemap0 = tilemap_create(get_lockon_tile_info0,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8,8,64,32);
	lockon_tilemap1 = tilemap_create(get_lockon_tile_info1,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8,8,64,32);
	lockon_tilemap2 = tilemap_create(get_lockon_tile_info2,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8,8,16,16); // HUD -> wrong
	tilemap_set_transparent_pen(lockon_tilemap0,0x00);
}

static VIDEO_UPDATE( lockon )
{
	tilemap_draw(bitmap,cliprect,lockon_tilemap1,0,0);       // Scene
	tilemap_draw(bitmap,cliprect,lockon_tilemap0,0,0);       // Characters
	//tilemap_draw(bitmap,cliprect,lockon_tilemap2,0,0);       // HUD
	draw_sprites(machine,bitmap,cliprect);
	return 0;
}

/* Wrong last time I checked */
static INPUT_PORTS_START( lockon )
	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "5"    )
	PORT_DIPSETTING(      0x0001, "4"  )
	PORT_DIPSETTING(      0x0002, "2"    )
	PORT_DIPSETTING(      0x0003, "3" )

	PORT_DIPNAME( 0x0006, 0x0008, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0010, "150K & every 200K"  )
	PORT_DIPSETTING(      0x0000, "200K & every 200K"  )

	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( No )  )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0000, "Coins" )
	PORT_DIPSETTING(      0x0000, "Free" )
	PORT_DIPSETTING(      0x0100, "4"  )
	PORT_DIPSETTING(      0x0200, "2"  )
	PORT_DIPSETTING(      0x0300, "3"  )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )     // Service coin
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1  ) // 'Trigger A'
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2  ) // 'Trigger B'
	PORT_DIPNAME( 0x40, 0x00, "Jumper 1" )
	PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Jumper 0" )
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START_TAG("analog_bank")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START_TAG("analog_pitch")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START_TAG("analog_missile")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_BUTTON3 )         // Digital input read by ADC

	PORT_START_TAG("analog_hover")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_BUTTON4 )         // Digital input read by ADC
INPUT_PORTS_END


static READ8_HANDLER(adc_r)
{
	switch(offset)
	{
		case 0: return readinputportbytag("analog_bank");
		case 1: return readinputportbytag("analog_pitch");
		case 2: return readinputportbytag("analog_missile");
		case 3: return readinputportbytag("analog_hover");
		default: return 0;
	}
}

static READ16_HANDLER(unk_read)
{
	return 1;                    // Main CPU jumps to 18002 otherwise
}


static INTERRUPT_GEN( lockon_irq )
{
        if(main_inten)
        	cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0x60/4);

          // Ground CPU takes an interrupt on BUFEND - this is incorrect.
          cpunum_set_input_line_and_vector(machine, 1, 0, HOLD_LINE, 0x60/4);
}

/*
   Main CPU control register
   -------------------------

   Bit 0 = MGAB16     Ground CPU A16
   Bit 1 = MGAB17     Ground CPU A17
   Bit 2 = MGABREQ    Object CPU Bus Request
   Bit 3 = MOAB16     Object CPU A16

   Bit 4 = MOAB17     Object CPU A17
   Bit 5 = OMLBRQ     Object CPU Bus Request
   Bit 6 = /ZBRQ      Z80 Bus Request
   Bit 7 = /ZCCLR     Clear screen
*/

static WRITE16_HANDLER(adrst_w)
{
	if (ACCESSING_LSB)
	{
		v30_gnd_addr = (data & 0x3);
		v30_obj_addr = (data & 0x18) >> 3;

		if (data & 0x04)
			cpunum_set_input_line(Machine, 1, INPUT_LINE_HALT, ASSERT_LINE);
		else
			cpunum_set_input_line(Machine, 1, INPUT_LINE_HALT, CLEAR_LINE);

		if (data & 0x20)
			cpunum_set_input_line(Machine, 2, INPUT_LINE_HALT, ASSERT_LINE);
		else
			cpunum_set_input_line(Machine, 2, INPUT_LINE_HALT, CLEAR_LINE);

		/* Suspend the Z80 when writing to shared RAM */
		if (data & 0x40)
			cpunum_set_input_line(Machine, 3, INPUT_LINE_HALT, CLEAR_LINE);
		else
			cpunum_set_input_line(Machine, 3, INPUT_LINE_HALT, ASSERT_LINE);
	}
}

static READ16_HANDLER(z80_shared_r)
{
	return z80_ram[offset] | 0xff00;
}

static WRITE16_HANDLER(z80_shared_w)
{
	if (ACCESSING_LSB)
		z80_ram[offset] = data;
}

static READ16_HANDLER(main_gnd_r)
{
	UINT16 result;
	cpuintrf_push_context(1);
	result = program_read_word((offset * 2) | (v30_gnd_addr << 16));
	cpuintrf_pop_context();
	return result;
}

static WRITE16_HANDLER(main_gnd_w)
{
	cpuintrf_push_context(1);
	if (ACCESSING_LSB)
		program_write_byte((offset * 2 + 0) | (v30_gnd_addr << 16), data);
	if (ACCESSING_MSB)
		program_write_byte((offset * 2 + 1) | (v30_gnd_addr << 16), data >> 8);
	cpuintrf_pop_context();
}

static READ16_HANDLER(main_obj_r)
{
	UINT16 result;
	cpuintrf_push_context(2);
	result = program_read_word((offset * 2) | (v30_obj_addr << 16));
	cpuintrf_pop_context();
	return result;
}

static WRITE16_HANDLER(main_obj_w)
{
	cpuintrf_push_context(2);
	if (ACCESSING_LSB)
		program_write_byte((offset * 2 + 0) | (v30_obj_addr << 16), data);
	if (ACCESSING_MSB)
		program_write_byte((offset * 2 + 1) | (v30_obj_addr << 16), data >> 8);
	cpuintrf_pop_context();
}


static WRITE16_HANDLER(testcs_w)
{
	if (offset < 0x400)
	{
		cpuintrf_push_context(1);
		if (ACCESSING_LSB)
			program_write_byte((offset*2+0) | (v30_gnd_addr << 16), data);
		if (ACCESSING_MSB)
			program_write_byte((offset*2+1) | (v30_gnd_addr << 16), data >> 8);
		cpuintrf_pop_context();

		cpuintrf_push_context(2);
		if (ACCESSING_LSB)
			program_write_byte((offset*2+0) | (v30_obj_addr << 16), data);
		if (ACCESSING_MSB)
			program_write_byte((offset*2+1) | (v30_obj_addr << 16), data >> 8);
		cpuintrf_pop_context();
	}
}

/*
The ground register is as follows:

0 27512x4 A14
1 27512x4 A15
2 Select 27512 A
3 Select 27512 B
4 LO3_04A A12
5 LO3_04A A13
6 LO3_04A A14
7 CS - ROMs
*/
static WRITE16_HANDLER(ground_bank_w)
{
// Nothing here yet
}


#ifdef UNUSED_FUNCTION
/* Allow /CUDISP to interrupt main CPU */
static WRITE8_HANDLER(sound_atten)
{
       mame_printf_debug("%x\n",data);
}
#endif

/* Allow /CUDISP to interrupt main CPU */
static WRITE16_HANDLER(inten_w)
{
	main_inten = TRUE;
}

/* Reset watchdog and mask /CUDISP interrupt */
static WRITE16_HANDLER(emres_w)
{
	// Implement watchdog reset here.
	main_inten = FALSE;
}


static ADDRESS_MAP_START( main_v30, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_RAM              // IC69/IC70
	AM_RANGE(0x04000, 0x04003) AM_NOP              // CRT Controller
	AM_RANGE(0x06000, 0x06001) AM_READ_PORT("DSW")
	AM_RANGE(0x08000, 0x081ff) AM_READWRITE(MRA16_RAM,lockon_vram_2_w) AM_BASE(&lockon_vram2)  // HUD RAM (IC30/IC31)
	AM_RANGE(0x09000, 0x09fff) AM_READWRITE(MRA16_RAM,lockon_vram_0_w) AM_BASE(&lockon_vram0)  // Character RAM
	AM_RANGE(0x0a000, 0x0a001) AM_WRITE(adrst_w)            // /ADRST: CPU access register
	AM_RANGE(0x0b000, 0x0bfff) AM_RAM                       // /BKBCS - ground control registers
	AM_RANGE(0x0c000, 0x0cfff) AM_RAM AM_BASE(&clut_ram)    // /CCRS - 8kB palette RAM - write only
	AM_RANGE(0x0e000, 0x0e001) AM_WRITE(inten_w)            // /INTEN: /CUDISP interrupt enable
	AM_RANGE(0x0f000, 0x0f001) AM_WRITE(emres_w)            // /EMRES: Watchdog reset and interrupt mask
	AM_RANGE(0x10000, 0x1ffff) AM_WRITE(testcs_w)           // /TESTCS: Write to both object and ground CPU RAM
	AM_RANGE(0x18000, 0x18001) AM_READ(unk_read)            // Not sure what this is
	AM_RANGE(0x20000, 0x2dfff) AM_ROM                                        // Z80 ROM
	AM_RANGE(0x2f000, 0x2ffff) AM_READWRITE(z80_shared_r, z80_shared_w)      // Z80 shared RAM
	AM_RANGE(0x30000, 0x3ffff) AM_READWRITE(main_gnd_r, main_gnd_w)          // Ground CPU memory access
	AM_RANGE(0x40000, 0x4ffff) AM_READWRITE(main_obj_r, main_obj_w)          // Object CPU memory access
	AM_RANGE(0x50000, 0x5ffff) AM_ROM              // IC76/IC77
	AM_RANGE(0x60000, 0x6ffff) AM_ROM              // IC88/IC89
	AM_RANGE(0xf0000, 0xfffff) AM_ROM              // IC95/IC96
ADDRESS_MAP_END

static ADDRESS_MAP_START( ground_v30, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x00000, 0x03fff) AM_RAM AM_BASE(&v30_gnd)        // IC31/IC34)
	AM_RANGE(0x04000, 0x04fff) AM_READWRITE(MRA16_RAM,lockon_vram_1_w) AM_BASE(&lockon_vram1)   // Scene RAM (IC112/IC113)
	AM_RANGE(0x08000, 0x08fff) AM_RAM AM_BASE(&ground_ram) AM_SIZE(&lockon_ground_size)        // Ground RAM (IC98/IC99)
	AM_RANGE(0x0c000, 0x0c001) AM_RAM                                          // /SHNH CS: Horizontal Scroll
	AM_RANGE(0x0c002, 0x0c003) AM_RAM                                          // /SHNV CS: Vertical Scroll
	AM_RANGE(0x0c004, 0x0c005) AM_RAM AM_WRITE(ground_bank_w)                  // /UNB CS:  Ground ROM Bank select
	AM_RANGE(0x20000, 0x2ffff) AM_ROM                                          // Ground ROM (IC30/IC33)
	AM_RANGE(0x30000, 0x3ffff) AM_ROM                                          // Program ROM mirror (use macros?)
	AM_RANGE(0xe0000, 0xeffff) AM_ROM                                          // Ground ROM mirror
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( object_v30, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_RAM AM_BASE(&v30_obj)          // Work RAM (IC29, IC44)
	AM_RANGE(0x04000, 0x04001) AM_RAM              // Read = assert CPU /POLL input (halt on WAIT instruction)
	AM_RANGE(0x08000, 0x081ff) AM_RAM              // TZA112 sprite chip write?
	AM_RANGE(0x0c000, 0x0c1ff) AM_RAM AM_BASE(&object_ram) AM_SIZE(&objectram_size)     // Object RAM (IC39/IC54)
	AM_RANGE(0x20000, 0x2ffff) AM_ROM             // Program ROM mirror
	AM_RANGE(0xf0000, 0xfffff) AM_ROM             // Program ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xf810, 0xf810) AM_WRITE(MWA8_NOP)           // /TS
	AM_RANGE(0x7000, 0x7000) AM_WRITE(MWA8_NOP)           // Attenuate YM3014 output (not used?)
	AM_RANGE(0x7400, 0x7403) AM_READWRITE(adc_r,MWA8_NOP) // M58990 ADC
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_BASE(&z80_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_READWRITE(YM2203_status_port_0_r, YM2203_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(YM2203_read_port_0_r, YM2203_write_port_0_w)
ADDRESS_MAP_END


static const gfx_layout char_layout =
{
	8,8,
	1024,
	2,
	{ 0, 8*8*1024 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout object_layout =
{
	8,8,
	4096,
	8,
	{ 0, 0x8000*8, 0x8000*2*8, 0x8000*3*8, 0x8000*4*8, 0x8000*5*8, 0x8000*6*8, 0x8000*7*8 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout scene_layout =
{
	8,8,
	4096,
	3,
	{ 0, 0x10000*8, 0x20000*8 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout hud_layout =
{
	8,8,
	1024,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout ground_layout =
{
	8,8,
	8192,
	3,
	{ 0, 0x10000*8, 0x20000*8 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( lockon )
	GFXDECODE_ENTRY( REGION_GFX1, 0, object_layout,  0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, object_layout,  0, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, object_layout,  0, 16 )
	GFXDECODE_ENTRY( REGION_GFX4, 0, object_layout,  0, 16 )
	GFXDECODE_ENTRY( REGION_GFX5, 0, char_layout,    0, 16 )
	GFXDECODE_ENTRY( REGION_GFX6, 0, scene_layout,   0, 16 )
	GFXDECODE_ENTRY( REGION_GFX7, 0, hud_layout,     0, 16 )
	GFXDECODE_ENTRY( REGION_GFX8, 0, ground_layout,  0, 16 )
GFXDECODE_END

static void YM2203_irq(int irq)
{
	cpunum_set_input_line(Machine, 3, 0, irq ? ASSERT_LINE : CLEAR_LINE );
}

static WRITE8_HANDLER(YM2203_out_b)
{
        coin_counter_w(0,data & 0x80);
        coin_counter_w(1,data & 0x40);
        coin_counter_w(2,data & 0x20);
        set_led_status(1,!(data & 0x10));            // 'LOCK-ON' lamp
}

static const struct YM2203interface ym2203_interface =
{
	input_port_2_r,
	0,
	0,
	YM2203_out_b,
	YM2203_irq
};

static MACHINE_DRIVER_START( lockon )
	MDRV_CPU_ADD(V30,8000000)
	MDRV_CPU_PROGRAM_MAP(main_v30,0)

	MDRV_CPU_ADD(V30,8000000)
	MDRV_CPU_PROGRAM_MAP(ground_v30,0)

	MDRV_CPU_ADD(V30,8000000)
	MDRV_CPU_PROGRAM_MAP(object_v30,0)

	MDRV_CPU_ADD(Z80,4000000)
	MDRV_CPU_PROGRAM_MAP(sound_prg,0)
	MDRV_CPU_IO_MAP(sound_io,0)

	MDRV_CPU_VBLANK_INT(lockon_irq,1)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(100)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MDRV_GFXDECODE(lockon)

	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(lockon)
	MDRV_VIDEO_UPDATE(lockon)

	MDRV_SPEAKER_STANDARD_STEREO("left","right")

	MDRV_SOUND_ADD(YM2203, 4000000)
	MDRV_SOUND_CONFIG(ym2203_interface)
	MDRV_SOUND_ROUTE(0, "left",  0.2)
	MDRV_SOUND_ROUTE(1, "left",  0.4)
	MDRV_SOUND_ROUTE(2, "left",  0.4)
	MDRV_SOUND_ROUTE(3, "left",  0.4)
	MDRV_SOUND_ROUTE(0, "right",  0.2)
	MDRV_SOUND_ROUTE(1, "right",  0.4)
	MDRV_SOUND_ROUTE(2, "right",  0.4)
	MDRV_SOUND_ROUTE(3, "right",  0.4)
MACHINE_DRIVER_END


ROM_START( lockon )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "lo1_02c", 0x50000, 0x8000, CRC(bbf17263) SHA1(96821a0ecd6efe6764380fef094f87c1d6e1d299) )
	ROM_LOAD16_BYTE( "lo1_03c", 0x50001, 0x8000, CRC(fa58fd36) SHA1(16af24027610bf6d3fdc4c3df3bf6d94c6776420) )

	ROM_LOAD16_BYTE( "lo1_04c", 0x60000, 0x8000, CRC(4a88576e) SHA1(80a8bd89cedebf080b2c08a6e81d3c2754024d8a) )
	ROM_LOAD16_BYTE( "lo1_05c", 0x60001, 0x8000, CRC(5a171b02) SHA1(f41f641136574e6af67c2245eb5a84799984474a) )

	ROM_LOAD16_BYTE( "lo1_00c", 0xf0000, 0x8000, CRC(e2db493b) SHA1(7491c634b698973ea54c25612d1e79c7efea8a45) )
	ROM_LOAD16_BYTE( "lo1_01c", 0xf0001, 0x8000, CRC(3e6065e0) SHA1(d870f5b466fab90d5c51dd27ecc807e7b38b5f79))

	// Z80
	ROM_LOAD16_BYTE( "lo1_08b", 0x20000, 0x8000, CRC(73860ec9) SHA1(a94afa274321b9f9ac2184e133132f9829fb9485) )

	// TF013 V30 (Ground)
	ROM_REGION( 0x100000, REGION_CPU2, 0 )
	ROM_LOAD16_BYTE( "lo3_01a", 0x20000, 0x8000, CRC(3eacdb6b) SHA1(7934c36dac9253dec4d8910954f6f2ae85951fe9) )
	ROM_RELOAD(                 0xe0000, 0x8000 )
	ROM_LOAD16_BYTE( "lo3_03a", 0x20001, 0x8000, CRC(4ce96d71) SHA1(cedbc33e86a93d11d5e11c2ef18bcf6390790a88) )
	ROM_RELOAD(                 0xe0001, 0x8000 )

	ROM_LOAD16_BYTE( "lo3_00b", 0xf0000, 0x8000, CRC(1835dccb) SHA1(8dfb0fea61a3e61f4da3b7f0da02cd19df2e68be) )
	ROM_RELOAD(                 0x30000, 0x8000 )
	ROM_LOAD16_BYTE( "lo3_02b", 0xf0001, 0x8000, CRC(2b8931d3) SHA1(f6f40b7857f3d47da8626450b1c1d3c46a1072ab) )
	ROM_RELOAD(                 0x30001, 0x8000 )

	// TF014 V30 (Object)
	ROM_REGION( 0x100000, REGION_CPU3, 0 )
	ROM_LOAD16_BYTE( "lo4_00b", 0xf0000, 0x8000, CRC(5f6b5a50) SHA1(daf82cafcae86d05587c191b0ff194ca7950e130) )
	ROM_RELOAD(                 0x30000, 0x8000 )

	ROM_LOAD16_BYTE( "lo4_01b", 0xf0001, 0x8000, CRC(7e88bcf2) SHA1(d541458ba6178ec3bce0e9b872b9fa1d8edb107c) )
	ROM_RELOAD(                 0x30001, 0x8000 )

	// TF014 Z80 (Sound)
	ROM_REGION( 0x10000, REGION_CPU4, 0 )
	ROM_LOAD( "lo1_08b", 0x00000, 0x8000, CRC(73860ec9) SHA1(a94afa274321b9f9ac2184e133132f9829fb9485) )

	// 8x8x8 object chunks
	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "lo5_00a", 0x00000, 0x8000, CRC(e9f23ce6) SHA1(4030384a0e8f47e8eea9483482ed1be264aec992) )
	ROM_LOAD( "lo5_04a", 0x08000, 0x8000, CRC(099323bc) SHA1(001d30f4c3c27277fadac89dcf616ff89eb0ea1c) )
	ROM_LOAD( "lo5_08a", 0x10000, 0x8000, CRC(9dedeff5) SHA1(53b0917a4fde4053182d38ea7f99f66e52543c10) )
	ROM_LOAD( "lo5_12a", 0x18000, 0x8000, CRC(2f5164ab) SHA1(df775b9e1c3c605a85d44404e4db42e33e80e664) )
	ROM_LOAD( "lo5_16a", 0x20000, 0x8000, CRC(c4500159) SHA1(e695e31e363cc954aab449f9d3dbc027e27fe7bf) )
	ROM_LOAD( "lo5_20a", 0x28000, 0x8000, CRC(3c1a67b5) SHA1(399935830b32a457ad0de243dd3eb4d368d5c6a6) )
	ROM_LOAD( "lo5_24a", 0x30000, 0x8000, CRC(1892d083) SHA1(8ee92be93ac222ecc2d9f4fcda3099b1db67516c) )
	ROM_LOAD( "lo5_28a", 0x38000, 0x8000, CRC(1186f9b4) SHA1(55598552dafa8cccfb423fc3b65a7fa15831d75b) )

	// 8x8x8 object chunks
	ROM_REGION( 0x400000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "lo5_01a", 0x00000, 0x8000, CRC(528d1395) SHA1(0221f81900757c10f288807f5c9549b9fdf5390f) )
	ROM_LOAD( "lo5_06a", 0x08000, 0x8000, CRC(be539b01) SHA1(1eebcbc592c51a676409b5be6c5d6609cd7118c9) )
	ROM_LOAD( "lo5_10a", 0x10000, 0x8000, CRC(23eeec5a) SHA1(08edd997d773684d329ef554776bc7acff1ac4ce) )
	ROM_LOAD( "lo5_14a", 0x18000, 0x8000, CRC(e63cd59e) SHA1(0518461acdc6c65dca8f21ca29bf528197e7cabe) )
	ROM_LOAD( "lo5_17a", 0x20000, 0x8000, CRC(ccf138d3) SHA1(971e7abe5b4d1a9dc8fc71b1ded5d2b81bcffaf2) )
	ROM_LOAD( "lo5_21a", 0x28000, 0x8000, CRC(39ce2000) SHA1(05f8e6f364ad714232fcea5b535ed5e181febd1e) )
	ROM_LOAD( "lo5_25a", 0x30000, 0x8000, CRC(7f3418bd) SHA1(5e595500f996b71aa73c637a6fddced30d78e222) )
	ROM_LOAD( "lo5_29a", 0x38000, 0x8000, CRC(45353d8d) SHA1(45cacd36700d24ae9f6eeaebed2fc860ef2d2978) )

	// 8x8x8 object chunks
	ROM_REGION( 0x400000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "lo5_02a", 0x00000, 0x8000, CRC(07aa32a1) SHA1(712b1983747acdd754d3abe934642cbc02ee13f2) )
	ROM_LOAD( "lo5_05a", 0x08000, 0x8000, CRC(f6b775a2) SHA1(e0146866f2e89675181c5d9d5aba23116daac420) )
	ROM_LOAD( "lo5_09a", 0x10000, 0x8000, CRC(953289bc) SHA1(197066af45c1193c36cd59b4b72b14f1c3bdd33e) )
	ROM_LOAD( "lo5_13a", 0x18000, 0x8000, CRC(67fbb061) SHA1(78b071cd54642ee7b6d7b9f6b759a1412bb9eef5) )
	ROM_LOAD( "lo5_18a", 0x20000, 0x8000, CRC(89884b24) SHA1(23c1fcc97f3a1abcaad413f4448db26f7c55fd5e) )
	ROM_LOAD( "lo5_22a", 0x28000, 0x8000, CRC(b1ed0361) SHA1(4bdf439026a858fdd929d5a7baac7d76f51550c5) )
	ROM_LOAD( "lo5_26a", 0x30000, 0x8000, CRC(a0b5c040) SHA1(ef63f89a368bc73eb77fc02d83b499a0231c1989) )
	ROM_LOAD( "lo5_30a", 0x38000, 0x8000, CRC(7d3993c5) SHA1(fb18daffcfc46bc1e1cfdee928eea861494af221) )

	// 8x8x8 object chunks
	ROM_REGION( 0x400000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "lo5_03a", 0x00000, 0x8000, CRC(7aca5d83) SHA1(95456b6c5adc5b776fbd33fd95cc62d4a83c34b6) )
	ROM_LOAD( "lo5_07a", 0x08000, 0x8000, CRC(313f127f) SHA1(0782b8dd5f3a3384c3e7bc9cacaadf6804e06a38) )
	ROM_LOAD( "lo5_11a", 0x10000, 0x8000, CRC(9df0b287) SHA1(6ea3b32a7826186c854cc079711ddea4ebf2ab7c) )
	ROM_LOAD( "lo5_15a", 0x18000, 0x8000, CRC(66f9c5db) SHA1(cc68da9312ee0a3441b62d14107e1b7de9b04de3) )
	ROM_LOAD( "lo5_19a", 0x20000, 0x8000, CRC(5aaa6a53) SHA1(f8ff547979883ac9a969e76d90d028ec4286ec4c) )
	ROM_LOAD( "lo5_23a", 0x28000, 0x8000, CRC(1487895b) SHA1(9d617f37932ca17d902307a97d16cf3b4bb5bc4e) )
	ROM_LOAD( "lo5_27a", 0x30000, 0x8000, CRC(119ff70a) SHA1(e64d41bc7822c9e99fd025b771551a6c511d13f2) )
	ROM_LOAD( "lo5_31a", 0x38000, 0x8000, CRC(d3595292) SHA1(9c45be919296626796b07f70b871fba5d444dbb3) )

	// 8x8x2 characters
	ROM_REGION( 0x20000, REGION_GFX5, ROMREGION_DISPOSE )
	ROM_LOAD( "lo1_06a", 0x00000, 0x2000, CRC(c8205913) SHA1(c791ff14418873ce68b502440c3d7ccc1f9cc00e) )
	ROM_LOAD( "lo1_07a", 0x02000, 0x2000, CRC(73673b79) SHA1(246b80f0c465cefb7ce1c87dc90a58f0f0ea3e0d) )

	// 8x8x3 'scene' tiles
	ROM_REGION( 0x40000, REGION_GFX6, ROMREGION_DISPOSE )
	ROM_LOAD( "lo3_10a", 0x00000, 0x10000, CRC(d5f4a8f3) SHA1(fcfaef46ef89c4b97970418a75d110271e94d55f) )
	ROM_LOAD( "lo3_11a", 0x10000, 0x10000, CRC(018efa36) SHA1(99eec3f06146627c7f7177b854424e7162ab7c8e) )
	ROM_LOAD( "lo3_12a", 0x20000, 0x10000, CRC(a34262a7) SHA1(08204a4474ab1b07b9114da8af03442737922d3b) )

	// 8x8x1 HUD tiles
	ROM_REGION( 0x40000, REGION_GFX7, ROMREGION_DISPOSE )
	ROM_LOAD( "lo2_00", 0x00000, 0x2000, CRC(8affea15) SHA1(b7bcf0abde9c933e3f2c75c1f5e2ca3417d50ca1) )

	// 8x8 ground GFX
	ROM_REGION( 0x400000, REGION_GFX8, ROMREGION_DISPOSE )
	ROM_LOAD( "lo3_05a", 0x00000, 0x10000, CRC(5b6f4c8e) SHA1(fc8b2c929c60fb0177ed3e407e3f0aacc5df8401) )
	ROM_LOAD( "lo3_06a", 0x10000, 0x10000, CRC(f6b6ebdd) SHA1(30e92da3bf83c4bb30faf00cbf01664b993f137c) )
	ROM_LOAD( "lo3_07a", 0x20000, 0x10000, CRC(cebc50e1) SHA1(f8b06ce576c3d41b0a8e2cc3ac60d3515d434812) )

	// Object chunk LUTs
	ROM_REGION( 0x30000, REGION_USER1, 0 )
	ROM_LOAD16_BYTE( "lo4_04a", 0x00000, 0x10000, CRC(098f4151) SHA1(cf38e3c5f3442fbfa97870d25f7c89c465f847a9) )
	ROM_LOAD16_BYTE( "lo4_05a", 0x00001, 0x10000, CRC(3b21667c) SHA1(b8337f733ede35145602ee3f0de25c2d4db1b2a5) )

	// Object LUTs
	ROM_REGION( 0x30000, REGION_USER2, 0 )
	ROM_LOAD( "lo4_02",  0x0000, 0x8000, CRC(0832edde) SHA1(77f9efbe029773417dbc3836a36687e37b5bee4b) )
	ROM_LOAD( "lo4_03",  0x8000, 0x8000, CRC(1efac891) SHA1(faf305a30cab1c6bf8a9d6e2682b2c3745aec956) )

	// Ground LUTs
	ROM_REGION( 0x30000, REGION_USER3, 0 )
	ROM_LOAD( "lo3_08",  0x00000, 0x10000, CRC(f418cecd) SHA1(6cf2d13c9df86bad9c24609cb8387e817b5d4281) )
	ROM_LOAD( "lo3_09",  0x10000, 0x10000, CRC(3c245568) SHA1(9ff6a23d83627f55c9d4f68e0bd89927bfe10664) )
	ROM_LOAD( "lo3_04a", 0x00000, 0x10000, CRC(80b67ba9) SHA1(fdbef463b26cd13c43596310f585432c6e0896d0) )

	// Scene tiles CLUT
	ROM_REGION( 0x10000, REGION_USER4, 0 )
	ROM_LOAD( "lo3_13a", 0x0000, 0x10000, CRC(e44774a7) SHA1(010d95ea497690ddd2406b8fef1b0aee375a165e) )

	// Colour PROMs
	ROM_REGION( 0x1000, REGION_PROMS, 0 )
	ROM_LOAD( "lo1a", 0x400,  0x400, CRC(82391f30) SHA1(d7153c1f3a3e54de4d4d6f432fbcd66449b96b6e) )
	ROM_LOAD( "lo2a", 0x000,  0x400, CRC(2bfc6288) SHA1(03d293ddc0c614b606be823826a4375b3d35901f) )
ROM_END

GAME( 1986, lockon, 0, lockon, lockon, 0, ROT0, "Tatsumi", "Lock-On", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )

