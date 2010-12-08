/*************************************************************************************************************

Blitter based gambling games
68000 CPU + 8bit MCUs (68HC7058/87C748), CRT controller, RAMDAC

Preliminary driver by David Haywood & Angelo Salese
Original steaser.c driver by David Haywood & Tomasz Slanina
Funny Fruit emulation by Luca Elia

Games:

Il Pagliaccio (c) 19?? unknown
Strip Teaser  (c) 1993 unknown
Funny Fruit   (c) 1998 Cadillac Jack

Notes:

- ilpag: at start-up a "initialize request" pops up. Press Service Mode and the Service switch, and
  reset with F3 for doing it.
- cjffruit: at start-up a "need coin adjustment" pops up. Press menu, go to page 1 with start, move to
  "price coin #1" with big, and set it with small, then exit with menu.

To Do:

- ilpag: protection not yet checked at all. My guess is it communicates via irq 3 and/or 6;
- steaser: understand the shared ram inputs better and find the coin chutes;
- ilpag, steaser: some minor issues with the blitter emulation;
- ilpag: sound uses a MP7524 8-bit DAC (bottom right, by the edge connector -PJB),  but the MCU controls the sound writes?
- steaser: sound uses an OkiM6295 (controlled by the sub MCU), check if it can be simulated;

*****************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/dac.h"
#include "video/mc6845.h"
#include "machine/nvram.h"
#include "deprecat.h"

class ilpag_state : public driver_device
{
public:
	ilpag_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT16>	m_nvram;
};

/*************************************************************************************************************

    Video

*************************************************************************************************************/

static UINT8 *blit_buffer;

static VIDEO_START(ilpag)
{
	blit_buffer = auto_alloc_array(machine, UINT8, 512*512*4); //just to be sure,number is wrong
}

static VIDEO_UPDATE(ilpag)
{
	int x,y;
	int count;

	count = 0;

	for(y=0;y<512;y++)
	{
		for(x=0;x<512;x++)
		{
			UINT32 color;
			color = (blit_buffer[count] & 0xff);

			if(x<screen->visible_area().max_x && y<screen->visible_area().max_y)
				*BITMAP_ADDR32(bitmap, y, x) = screen->machine->pens[color];

			count++;
		}
	}

	return 0;
}

/*************************************************************************************************************

    Palette

*************************************************************************************************************/

static WRITE16_HANDLER( paletteram_io_w )
{
	static int pal_offs,r,g,b,internal_pal_offs,pal_data;

	switch(offset*2)
	{
		case 0:
			pal_offs = (data & 0xff00) >> 8;
			break;
		case 4:
			internal_pal_offs = 0;
			break;
		case 2:
			switch(internal_pal_offs)
			{
				case 0:
					pal_data = (data & 0xff00) >> 8;
					r = ((pal_data & 0x3f) << 2) | ((pal_data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 1:
					pal_data = (data & 0xff00) >> 8;
					g = ((pal_data & 0x3f) << 2) | ((pal_data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 2:
					pal_data = (data & 0xff00) >> 8;
					b = ((pal_data & 0x3f) << 2) | ((pal_data & 0x30) >> 4);
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}

			break;
	}
}

/*************************************************************************************************************

    Palette (Bt476 RAMDAC)
    To do: merge with above, implement as a device

*************************************************************************************************************/

static UINT8 pal_ram[256*3];
static UINT8 pal_offs, internal_pal_offs, r, g, b;

static WRITE8_HANDLER( paletteram_bt476_w )
{
	switch(offset)
	{
		case 0:
			pal_offs = data;
			internal_pal_offs = 0;
			break;
		case 2:
			pal_offs = data;
			internal_pal_offs = 0;
			break;
		case 1:
			data &= 0x3f;
			pal_ram[pal_offs * 3 + internal_pal_offs] = data;
			switch(internal_pal_offs)
			{
				case 0:
					r = data << 2;
					internal_pal_offs++;
					break;
				case 1:
					g = data << 2;
					internal_pal_offs++;
					break;
				case 2:
					b = data << 2;
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}
			break;
	}
}

static READ8_HANDLER( paletteram_bt476_r )
{
	UINT8 ret = 0xff;

	switch(offset)
	{
		case 0:
			ret = pal_offs;
			break;

		case 1:
			ret = pal_ram[pal_offs * 3 + internal_pal_offs];
			internal_pal_offs++;
			if (internal_pal_offs >= 3)
			{
				internal_pal_offs = 0;
				pal_offs++;
			}
			break;
	}

	return ret;
}

/*************************************************************************************************************

    Blitter (ilpag, steaser)

    To do:
    - register names should be properly renamed
      "transpen" 8/2 is for layer clearance;
      "transpen" 10/2 is the trasparency pen;
      "vregs" are pen selects, they select the proper color to render, and are tied to the first three gfx offsets;
    - shrinking bit? (some pictures in steaser, "00" on top of ilpag)
    - draw direction (card choose in steaser, in service mode)
    - line draw? (for the "Game Over" msg in steaser)
    - "random" pens? (9d0000 read in steaser)
    - anything else?

*************************************************************************************************************/

static UINT16 *blit_romaddr,*blit_attr1_ram,*blit_dst_ram_loword,*blit_attr2_ram,*blit_dst_ram_hiword,*blit_vregs,*blit_transpen;

static WRITE16_HANDLER( blit_copy_w )
{
	UINT8 *blit_rom = memory_region(space->machine, "blitter");
	UINT32 blit_dst_xpos;
	UINT32 blit_dst_ypos;
	int x,y,x_size,y_size;
	UINT32 src;

	logerror("blit copy %04x %04x %04x %04x %04x\n", blit_romaddr[0], blit_attr1_ram[0], blit_dst_ram_loword[0], blit_attr2_ram[0], blit_dst_ram_hiword[0] );
	logerror("blit vregs %04x %04x %04x %04x\n",blit_vregs[0/2],blit_vregs[2/2],blit_vregs[4/2],blit_vregs[6/2]);
	logerror("blit transpen %04x %04x %04x %04x %04x %04x %04x %04x\n",blit_transpen[0/2],blit_transpen[2/2],blit_transpen[4/2],blit_transpen[6/2],
	                                                               blit_transpen[8/2],blit_transpen[10/2],blit_transpen[12/2],blit_transpen[14/2]);

	blit_dst_xpos = (blit_dst_ram_loword[0] & 0x00ff)*2;
	blit_dst_ypos = ((blit_dst_ram_loword[0] & 0xff00)>>8);

	y_size = (0x100-((blit_attr2_ram[0] & 0xff00)>>8));
	x_size = (blit_attr2_ram[0] & 0x00ff)*2;

	/* rounding around for 0 size */
	if(x_size == 0) { x_size = 0x200; }

	/* TODO: used by steaser "Game Over" msg on attract mode*/
//  if(y_size == 1) { y_size = 32; }

	src = blit_romaddr[0] | (blit_attr1_ram[0] & 0x1f00)<<8;
//  src|= (blit_transpen[0xc/2] & 0x0100)<<12;

	for(y=0;y<y_size;y++)
	{
		for(x=0;x<x_size;x++)
		{
			int drawx = (blit_dst_xpos+x)&0x1ff;
			int drawy = (blit_dst_ypos+y)&0x1ff;
			if(blit_transpen[0x8/2] & 0x100)
				blit_buffer[drawy*512+drawx] = ((blit_vregs[0] & 0xf00)>>8);
			else
			{
				UINT8 pen_helper;

				pen_helper = blit_rom[src] & 0xff;
				if(blit_transpen[0xa/2] & 0x100) //pen is opaque register
				{
					if(pen_helper)
						blit_buffer[drawy*512+drawx] = ((pen_helper & 0xff) <= 3) ? ((blit_vregs[pen_helper] & 0xf00)>>8) : blit_rom[src];
				}
				else
					blit_buffer[drawy*512+drawx] = ((pen_helper & 0xff) <= 3) ? ((blit_vregs[pen_helper] & 0xf00)>>8) : blit_rom[src];
			}

			src++;
		}
	}
}

/*************************************************************************************************************

    Blitter (cjffruit)
    To do: merge with above? Implement as 8-bit?

*************************************************************************************************************/

static UINT16 *blitter_regs, *blitter_pens, *blitter_trans;

static WRITE16_HANDLER( blitter_regs_w )
{
	UINT8 *blit_rom  = memory_region(space->machine, "blitter");
	int blit_romsize = memory_region_length(space->machine, "blitter");
	UINT32 blit_dst_xpos;
	UINT32 blit_dst_ypos;
	int x, y, x_size, y_size;
	UINT32 src;

	COMBINE_DATA(&blitter_regs[offset]);

	if ( (offset != 0x7/2) || !ACCESSING_BITS_0_7 )
		return;

	logerror("%s: blit %04x %04x %04x %04x - pens %04x%04x - trans %04x%04x / %04x\n", cpuexec_describe_context(space->machine),
				blitter_regs[0],  blitter_regs[1], blitter_regs[2], blitter_regs[3],
				blitter_pens[0],  blitter_pens[1],
				blitter_trans[0], blitter_trans[1],
				blitter_trans[2]
	);

	y_size = ((blitter_regs[0x0/2] & 0xff00) >> 8);
	x_size = ((blitter_regs[0x1/2] & 0x00ff) >> 0);

	x_size = (x_size + 1) * 2;
	y_size = (y_size + 1);

	blit_dst_ypos = ((blitter_regs[0x2/2] & 0xff00) >> 8);
	blit_dst_xpos = ((blitter_regs[0x3/2] & 0x00ff) >> 0);

	blit_dst_xpos *= 2;

	src = ((blitter_regs[0x4/2] & 0x0f00) << 8) | ((blitter_regs[0x5/2] & 0x00ff) << 8) | ((blitter_regs[0x6/2] & 0xff00) >> 8);
	src *= 2;

	int flipx = ((blitter_trans[0x0/2] & 0xff00) == 0);
	int flipy = ((blitter_trans[0x1/2] & 0x00ff) == 0);

	for (y = 0; y < y_size; y++)
	{
		for (x = 0; x < x_size; x++)
		{
			int drawx = (blit_dst_xpos + (flipx ? -x+1 : x)) & 0x1ff;
			int drawy = (blit_dst_ypos + (flipy ? -y+1 : y)) & 0x1ff;

			src %= blit_romsize;

			UINT8 pen = blit_rom[src];
			if ((blitter_trans[0x2/2] & 0xff00) == 0)
				src++;
			else
				pen = (src/2) & 0xff;

			if (pen || ((blitter_trans[0x3/2] & 0x00ff) == 0))
			{
				if (pen <= 3)
					pen = (blitter_pens[pen/2] >> ((pen & 1) ? 0 : 8)) & 0xf;

				blit_buffer[drawy*512+drawx] = pen;
			}
		}
	}

	// used by cjffruit in service mode (girl select screen)
	blit_dst_xpos += (flipx ?        +1 :      0);
	blit_dst_ypos += (flipy ? -y_size+1 : y_size);
	blitter_regs[0x2/2] =  ((blit_dst_ypos & 0xff) << 8) | ((blit_dst_xpos/2) & 0xff);
}


/*************************************************************************************************************

    Memory Maps

*************************************************************************************************************/

/*bit 0 is the blitter busy flag*/
static READ16_HANDLER( blitter_status_r )
{
	return 0;
}

/*TODO*/
static WRITE16_HANDLER( lamps_w )
{
//  popmessage("%02x",data);
}

static READ16_HANDLER( test_r )
{
	return 0xffff;//mame_rand(space->machine);
}

#if 0
static WRITE16_HANDLER( irq_callback_w )
{
//  popmessage("%02x",data);
	cputag_set_input_line(space->machine, "maincpu", 3, HOLD_LINE );
}

static WRITE16_HANDLER( sound_write_w )
{
	popmessage("%02x",data);
	dac_data_w(0, data & 0x0f);		/* Sound DAC? */
}
#endif

static ADDRESS_MAP_START( ilpag_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1fffff) AM_ROM AM_REGION("blitter", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_SHARE("nvram")

//  AM_RANGE(0x800000, 0x800001) AM_READ(test_r)
//  AM_RANGE(0x880000, 0x880001) AM_READ(test_r)

	AM_RANGE(0x900000, 0x900005) AM_WRITE( paletteram_io_w ) //RAMDAC
	AM_RANGE(0x980000, 0x98000f) AM_RAM AM_BASE(&blit_transpen) //video registers for the blitter write
	AM_RANGE(0x990000, 0x990007) AM_RAM AM_BASE(&blit_vregs) //pens
	AM_RANGE(0x998000, 0x998001) AM_RAM AM_BASE(&blit_romaddr)
	AM_RANGE(0x9a0000, 0x9a0001) AM_RAM AM_BASE(&blit_attr1_ram)
	AM_RANGE(0x9a8000, 0x9a8001) AM_RAM AM_BASE(&blit_dst_ram_loword)
	AM_RANGE(0x9b0000, 0x9b0001) AM_RAM AM_BASE(&blit_attr2_ram)
	AM_RANGE(0x9b8000, 0x9b8001) AM_RAM_WRITE( blit_copy_w ) AM_BASE(&blit_dst_ram_hiword)
	AM_RANGE(0x9e0000, 0x9e0001) AM_READ(blitter_status_r)

	AM_RANGE(0xc00000, 0xc00001) AM_WRITE(lamps_w)
	AM_RANGE(0xc00180, 0xc00181) AM_READ_PORT("IN2")
//  AM_RANGE(0xc00200, 0xc00201) AM_WRITE(sound_write_w)
	AM_RANGE(0xc00380, 0xc00381) AM_READ_PORT("IN3")
//  AM_RANGE(0xc00300, 0xc00301) AM_WRITE(irq_callback_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( steaser_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x1fffff) AM_ROM AM_REGION("blitter", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x800000, 0x800001) AM_READ(test_r)
//  AM_RANGE(0x840000, 0x840001) AM_WRITE(sound_write_w)
	AM_RANGE(0x880000, 0x880001) AM_READ(test_r)
//  AM_RANGE(0x8c0000, 0x8c0001) AM_WRITE(sound_write_w)

	AM_RANGE(0x900000, 0x900005) AM_WRITE( paletteram_io_w ) //RAMDAC
	AM_RANGE(0x940000, 0x940001) AM_WRITENOP //? Seems a dword write for some read, written consecutively
	AM_RANGE(0x980000, 0x98000f) AM_RAM AM_BASE(&blit_transpen)//probably transparency pens
	AM_RANGE(0x990000, 0x990005) AM_RAM AM_BASE(&blit_vregs)
	AM_RANGE(0x998000, 0x998001) AM_RAM AM_BASE(&blit_romaddr)
	AM_RANGE(0x9a0000, 0x9a0001) AM_RAM AM_BASE(&blit_attr1_ram)
	AM_RANGE(0x9a8000, 0x9a8001) AM_RAM AM_BASE(&blit_dst_ram_loword)
	AM_RANGE(0x9b0000, 0x9b0001) AM_RAM AM_BASE(&blit_attr2_ram)
	AM_RANGE(0x9b8000, 0x9b8001) AM_RAM_WRITE( blit_copy_w ) AM_BASE(&blit_dst_ram_hiword)
	AM_RANGE(0x9c0002, 0x9c0003) AM_READNOP //pen control?
	AM_RANGE(0x9d0000, 0x9d0001) AM_READNOP //?
	AM_RANGE(0x9e0000, 0x9e0001) AM_READ(blitter_status_r)
	AM_RANGE(0x9f0000, 0x9f0001) AM_WRITENOP //???

//  AM_RANGE(0xc00000, 0xc00001) AM_WRITE(lamps_w)
	AM_RANGE(0xbd0000, 0xbd0001) AM_READ(test_r)
//  AM_RANGE(0xc00200, 0xc00201) AM_WRITE(sound_write_w)
//  AM_RANGE(0xc00380, 0xc00381) AM_READ_PORT("IN3")
//  AM_RANGE(0xc00300, 0xc00301) AM_WRITE(irq_callback_w)
ADDRESS_MAP_END

/*************************************************************************************************************
    Funny Fruit
*************************************************************************************************************/

// Outputs
UINT16 *cjffruit_leds1, *cjffruit_leds2, *cjffruit_leds3;

static void show_leds()
{
#ifdef MAME_DEBUG
//  popmessage("led %02X %02x %02x", cjffruit_leds1[0]>>8, cjffruit_leds2[0]>>8, cjffruit_leds3[0]>>8);
#endif
}

static WRITE16_HANDLER( cjffruit_leds1_w )
{
	data = COMBINE_DATA(cjffruit_leds1);
	if (ACCESSING_BITS_8_15)
	{
		coin_counter_w(space->machine, 0, data & 0x0100);	// coin in
		set_led_status(space->machine, 0, data & 0x0200);	// win???
//                                     1  data & 0x0400     // win???
		set_led_status(space->machine, 2, data & 0x0800);	// small
		set_led_status(space->machine, 3, data & 0x1000);	// big
		set_led_status(space->machine, 4, data & 0x2000);	// take
		set_led_status(space->machine, 5, data & 0x4000);	// double up
		set_led_status(space->machine, 6, data & 0x8000);	// cancel
		show_leds();
	}
}

static WRITE16_HANDLER( cjffruit_leds2_w )
{
	data = COMBINE_DATA(cjffruit_leds2);
	if (ACCESSING_BITS_8_15)
	{
		set_led_status(space->machine,  7, data & 0x0100);	// start
		set_led_status(space->machine,  8, data & 0x0200);	// bet
		set_led_status(space->machine,  9, data & 0x0400);	// hold 5
		set_led_status(space->machine, 10, data & 0x0800);	// hold 4
		set_led_status(space->machine, 11, data & 0x1000);	// hold 3
		set_led_status(space->machine, 12, data & 0x2000);	// hold 2
		set_led_status(space->machine, 13, data & 0x4000);	// collect
		set_led_status(space->machine, 14, data & 0x8000);	// call attendant
		show_leds();
	}
}

static WRITE16_HANDLER( cjffruit_leds3_w )
{
	data = COMBINE_DATA(cjffruit_leds3);
	if (ACCESSING_BITS_8_15)
	{
		set_led_status(space->machine, 15, data & 0x0100);	// hopper coins?
		set_led_status(space->machine, 16, data & 0x0400);	// coin out?
		show_leds();
	}
}

// CRTC
static READ8_DEVICE_HANDLER( crtc_r )
{
	if (offset & 1)
		return mc6845_register_r(device, 0);
	else
		return mc6845_status_r(device, 0);
}

static WRITE8_DEVICE_HANDLER( crtc_w )
{
	if (offset & 1)
		mc6845_register_w(device, 0, data);
	else
		mc6845_address_w(device, 0, data);
}

static WRITE16_DEVICE_HANDLER( crtc_lpen_w )
{
	// 8fe0006: 0->1
	if (ACCESSING_BITS_8_15 && (data & 0x0100))
		mc6845_assert_light_pen_input(device);
	// 8fe0007: 1->0 (MCU irq?)
}

// MCU simulation (to be done)
static READ16_HANDLER( cjffruit_mcu_r )
{
	UINT8 ret = 0x00;	// mame_rand(space->machine) gives "interesting" results
	logerror("%s: mcu reads %02x\n", cpuexec_describe_context(space->machine), ret);
	return ret << 8;
}

static WRITE16_HANDLER( cjffruit_mcu_w )
{
	logerror("%s: mcu written with %02x\n", cpuexec_describe_context(space->machine),data >> 8);
}

static ADDRESS_MAP_START( cjffruit_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x480000, 0x4807ff) AM_RAM

	AM_RANGE(0x820000, 0x820007) AM_WRITE(blitter_regs_w) AM_BASE(&blitter_regs)

	AM_RANGE(0x850000, 0x850001) AM_READ( cjffruit_mcu_r )

	AM_RANGE(0x870000, 0x870001) AM_READ_PORT("IN0")
	AM_RANGE(0x872000, 0x872001) AM_READ_PORT("IN1")
	AM_RANGE(0x874000, 0x874001) AM_READ_PORT("IN2")
	AM_RANGE(0x876000, 0x876001) AM_READ_PORT("DSW")

	AM_RANGE(0x880000, 0x880003) AM_WRITE8( paletteram_bt476_w, 0xffff ) // RAMDAC
	AM_RANGE(0x880000, 0x880001) AM_READ8 ( paletteram_bt476_r, 0xffff ) // ""

	AM_RANGE(0x8a0000, 0x8a0007) AM_WRITE(blitter_regs_w)
	AM_RANGE(0x8b0000, 0x8b0003) AM_WRITEONLY AM_BASE(&blitter_pens)

	AM_RANGE(0x8e0000, 0x8e0001) AM_READ( cjffruit_mcu_w )

	AM_RANGE(0x8f8000, 0x8f8001) AM_WRITE(cjffruit_leds1_w) AM_BASE(&cjffruit_leds1)
	AM_RANGE(0x8fa000, 0x8fa001) AM_WRITE(cjffruit_leds2_w) AM_BASE(&cjffruit_leds2)
	AM_RANGE(0x8fc000, 0x8fc001) AM_WRITE(cjffruit_leds3_w) AM_BASE(&cjffruit_leds3)

	AM_RANGE(0x8fe000, 0x8fe005) AM_WRITEONLY AM_BASE(&blitter_trans)
	AM_RANGE(0x8fe006, 0x8fe007) AM_DEVWRITE("crtc", crtc_lpen_w)	// 0x8fe006: 0->1, 0x8fe007: 1->0

	AM_RANGE(0xc40000, 0xc40001) AM_DEVREADWRITE8("crtc", crtc_r, crtc_w, 0xffff)
ADDRESS_MAP_END


/*************************************************************************************************************

    Inputs

*************************************************************************************************************/

static INPUT_PORTS_START( cjffruit )
	PORT_START("IN0")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1        ) // coin 1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2        ) // coin 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1     ) PORT_NAME("Recall")
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW     ) // menu
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK  ) // stats
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_POKER_HOLD1  ) // hold 1
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_BET   ) // bet
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL  ) // start / deal

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE   ) // take
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   ) // double up
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_LOW    ) // small (show pay table)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH   ) // big
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_POKER_CANCEL  ) // cancel
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // collect
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) // hold 2
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) // hold 3

	PORT_START("IN2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_POKER_HOLD4 ) // hold 4
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_POKER_HOLD5 ) // hold 5
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SPECIAL     ) // hopper coin
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_GAMBLE_DOOR ) PORT_NAME("Cash Door") PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL     ) // blitter busy
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL     ) // 5] 0 = mcu response ready
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_SPECIAL     ) // 6] 0 = mcu busy
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_SERVICE3    ) PORT_NAME("Call Attendant")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Frames Per Second" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, "60" )
	PORT_DIPSETTING(      0x0000, "50" )
	PORT_DIPNAME( 0x0e00, 0x0800, "Pinout" )				PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0e00, "8L6 (Cherry Master)" )
	PORT_DIPSETTING(      0x0c00, "8L7"     )
	PORT_DIPSETTING(      0x0a00, "8L10"    )
	PORT_DIPSETTING(      0x0800, "L74 (Funny Fruit)"   )
	PORT_DIPSETTING(      0x0600, "CYB"     )
	PORT_DIPSETTING(      0x0400, "PMMG"    )
	PORT_DIPSETTING(      0x0200, "Invalid" )
	PORT_DIPSETTING(      0x0000, "Invalid" )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Logic Door") PORT_CODE(KEYCODE_L) PORT_TOGGLE
	PORT_DIPNAME( 0x2000, 0x2000, "Factory Default" )		PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Main Door") PORT_CODE(KEYCODE_O) PORT_TOGGLE
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( ilpag )
	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Gioco (Bet)")
	PORT_DIPNAME( 0x0020, 0x0020, "IN3" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( steaser )
	PORT_START("MENU")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_IMPULSE(1)

	PORT_START("STAT")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_IMPULSE(1)

	PORT_START("BET_DEAL")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_IMPULSE(1)

	PORT_START("TAKE_DOUBLE")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP ) PORT_NAME("Double Up") PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score") PORT_IMPULSE(1)

	PORT_START("SMALL_BIG")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) PORT_NAME("Big") PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW ) PORT_NAME("Small") PORT_IMPULSE(1)

	PORT_START("CANCEL_HOLD1")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_IMPULSE(1)

	PORT_START("HOLD2_HOLD3")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_IMPULSE(1)

	PORT_START("HOLD4_HOLD5")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_IMPULSE(1)
INPUT_PORTS_END

/*************************************************************************************************************

    Machine Drivers

*************************************************************************************************************/

static MACHINE_CONFIG_START( ilpag, ilpag_state )
	MDRV_CPU_ADD("maincpu", M68000, 11059200 )	// ?
	MDRV_CPU_PROGRAM_MAP(ilpag_map)
	MDRV_CPU_VBLANK_INT("screen",irq4_line_hold) //3 & 6 used, mcu comms?

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MDRV_NVRAM_ADD_0FILL("nvram")

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(ilpag)
	MDRV_VIDEO_UPDATE(ilpag)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

/*
20089f = 1 -> menu
2008a0 = 1 -> stat
2008a1 = 1 -> (unused)
2008a2 = 1 -> bet
2008a3 = 1 -> deal
2008a4 = 1 -> take
2008a5 = 1 -> double
2008a6 = 1 -> small
2008a7 = 1 -> big
2008a8 = 1 -> cancel
2008a9 = 1 -> hold 1
2008aa = 1 -> hold 2
2008ab = 1 -> hold 3
2008ac = 1 -> hold 4
2008ad = 1 -> hold 5
*/

static TIMER_DEVICE_CALLBACK( steaser_mcu_sim )
{
	ilpag_state *state = timer.machine->driver_data<ilpag_state>();
//  static int i;
	/*first off, signal the "MCU is running" flag*/
	state->m_nvram[0x932/2] = 0xffff;
	/*clear the inputs (they are impulsed)*/
//  for(i=0;i<8;i+=2)
//      state->m_nvram[((0x8a0)+i)/2] = 0;
	/*finally, read the inputs*/
	state->m_nvram[0x89e/2] = input_port_read(timer.machine, "MENU") & 0xffff;
	state->m_nvram[0x8a0/2] = input_port_read(timer.machine, "STAT") & 0xffff;
	state->m_nvram[0x8a2/2] = input_port_read(timer.machine, "BET_DEAL") & 0xffff;
	state->m_nvram[0x8a4/2] = input_port_read(timer.machine, "TAKE_DOUBLE") & 0xffff;
	state->m_nvram[0x8a6/2] = input_port_read(timer.machine, "SMALL_BIG") & 0xffff;
	state->m_nvram[0x8a8/2] = input_port_read(timer.machine, "CANCEL_HOLD1") & 0xffff;
	state->m_nvram[0x8aa/2] = input_port_read(timer.machine, "HOLD2_HOLD3") & 0xffff;
	state->m_nvram[0x8ac/2] = input_port_read(timer.machine, "HOLD4_HOLD5") & 0xffff;
}

/* TODO: remove this hack.*/
static INTERRUPT_GEN( steaser_irq )
{
	int num=cpu_getiloops(device)+3;
	cpu_set_input_line(device, num, HOLD_LINE);
}

static MACHINE_CONFIG_DERIVED( steaser, ilpag )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(steaser_map)
	MDRV_CPU_VBLANK_INT_HACK(steaser_irq,4)

	MDRV_TIMER_ADD_PERIODIC("coinsim", steaser_mcu_sim, HZ(10000)) // not real, but for simulating the MCU
MACHINE_CONFIG_END


// R6845AP used for video sync signals only

static MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr)
{
}

static WRITE_LINE_DEVICE_HANDLER(crtc_vsync)
{
	cputag_set_input_line(device->machine, "maincpu", 1, state ? ASSERT_LINE : CLEAR_LINE);
}

const mc6845_interface cjffruit_mc6845_intf =
{
	"screen",	/* screen we are acting on */
	4,			/* number of pixels per video memory address */ /* Horizontal Display programmed to 160 characters */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_LINE(crtc_vsync),	/* VSYNC callback */
	crtc_addr				/* update address callback */
};

static MACHINE_CONFIG_START( cjffruit, ilpag_state )
	MDRV_CPU_ADD("maincpu", M68000, XTAL_22_1184MHz/2)
	MDRV_CPU_PROGRAM_MAP(cjffruit_map)

	// MC68HC705C8P (Sound MCU)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-8-1)
	MDRV_MC6845_ADD("crtc", R6545_1, XTAL_22_1184MHz/8, cjffruit_mc6845_intf)

	MDRV_NVRAM_ADD_0FILL("nvram")

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(ilpag)
	MDRV_VIDEO_UPDATE(ilpag)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


/*************************************************************************************************************

    ROMs Loading

*************************************************************************************************************/

/*************************************************************************************************************

Funny Fruit
(C) 1997-1998 Cadillac Jack

Board: CJ-8L REV-D

CPUs:
  MC68EC000FN12 (Main CPU)
  Osc. 22.1184 MHz
  MC68HC705C8P (Sound MCU, internal ROM not dumped)
  Osc. 4 MHz

Video:
  Xilinx chip (CRT Controller)
  Bt476KPJ35 (RAMDAC)

ROMs:
  6 x 27C040 (JP5-9 configure the rom sizes for program, sound and graphics)
  1 x GAL16V8d-15LP (read protected)

Other:
  8 x DSW
  RS-232C interface (4-pin)
  Lithium battery 3.6V
  36/72 pin edge connector
  10/20 pin edge connector

Note:
  Game supports a Centronics iDP-3541 printer (4-pin connector) and a Deltronics DL-1275 ticket dispenser.
  It is also available on board CJ-8L REV-B LEV-1, wih a R6845AP as CRT controller.

*************************************************************************************************************/

ROM_START( cjffruit )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD( "cjfunfruit-cj_1.13-a.u65", 0x00000, 0x80000, CRC(3a74d769) SHA1(fc8804d49cc31dadf10027ed1e2458cae96d6355) )

	ROM_REGION( 0x2000, "mcu", 0 )	// 68HC705C8P code
	ROM_LOAD( "cjfunfruit_2.3.c8", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x200000, "blitter", 0 ) // data for the blitter
	ROM_LOAD16_BYTE( "cjfunfruit-cj_1.13-d.u68", 0x000000, 0x80000, CRC(33ccdc3f) SHA1(8d81e25c5a38f280c6fe5710937c876dcb679e61) )
	ROM_LOAD16_BYTE( "cjfunfruit-cj_1.13-c.u75", 0x000001, 0x80000, CRC(93854506) SHA1(09fd85d60ab723883d28a12f56dbb0cb2b03907f) )
	ROM_LOAD16_BYTE( "cjfunfruit-cj_1.13-f.u51", 0x100000, 0x80000, CRC(f5de1072) SHA1(943a82899ca6a07991fa4031d2ff96f625c9d6f5) )
	ROM_LOAD16_BYTE( "cjfunfruit-cj_1.13-e.u61", 0x100001, 0x80000, CRC(7acaef9d) SHA1(5031dc22e787dc4d8dffe67382068b9926c24bef) )

	ROM_REGION( 0x80000, "samples", 0 )	// 8 bit unsigned
	ROM_LOAD( "cjfunfruit-cj_1.13-g.u50", 0x00000, 0x80000, CRC(5fb53d3e) SHA1(f4a37b00a9417440685d198f1375b615848e7fb6) )

    ROM_REGION( 0x117, "plds", 0 )
    ROM_LOAD( "gal16v8d_vdp.u15", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Il Pagliaccio (unknown manufacturer)

CPU

1x MC68HC000FN12 (main)
1x P87C748EBPN (80C51 family 8-bit microcontroller)
1x oscillator 11.0592MHz
1x blue resonator CSB400P

VIDEO

1x XC95144XL
2x XC9572XL
1x ADV471KP50

ROMs

4x W27E040
1x P87C748EBPN (read protected)

Note

1x 28x2 edge connector (not JAMMA)
1x trimmer (volume)
1x battery
2x pushbutton (MANAGEMENT,STATISTIC)

*************************************************************************************************************/

ROM_START( ilpag )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.7c-35.u32", 0x000000, 0x80000, CRC(ed99c884) SHA1(b3d2c9fb7765e3c8ff1e0de9c8edb6628e1c79ef) )
	ROM_LOAD16_BYTE( "2.7c-36.u31", 0x000001, 0x80000, CRC(4cd41688) SHA1(a1a15b06aa738cd4154d3c3479a7bf2da0e48426) )

	ROM_REGION( 0x800, "mcu", 0 ) // 87C748 code
	ROM_LOAD( "87c748.u132", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x100000, "blitter", 0 ) // data for the blitter
	ROM_LOAD( "graf2.u51",   0x080000, 0x80000, CRC(2d64d3b5) SHA1(8fdb943d0aedf12706ce0a772c8f5155fa03e8c7) )
	ROM_LOAD( "graf1.u46",   0x000000, 0x80000, CRC(cf745964) SHA1(7af4a6c0b8d01c0d1b71bc5330a257d2fa712611) )
ROM_END

/*************************************************************************************************************

Strip Teaser (unknown manufacturer)

lower board (Dk-B)

TS68000CP12 (main cpu)
osc. 11.0592MHz
MC68HC705C8P (MCU)
UM6845RA (CRT controller-Supports alphanumeric and graphics modes.Addresses up to 16 KB of video memory-2 MHz)
Lithium battery 3,6V


upper board (8L74) (soundboard?)

MC68HC705C8P (MCU)
osc. 4.0000MHz
non JAMMA connector
1x dipswitch (4 switch)


ROMs
1x AT27c010 (u31.1)(program)
1x AM27C010 (u32.6)(program)
4x M27C4001 (u46.2 - u51.3 - u61.4 - u66.5)(GFX)
1x M27C4001 (u18.7)(sound)

*************************************************************************************************************/

ROM_START( steaser )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "u31.1", 0x00001, 0x20000, CRC(7963e960) SHA1(2a1c68265e0a3909ccd097ea784e3e179f528844) )
	ROM_LOAD16_BYTE( "u32.6", 0x00000, 0x20000, CRC(c0ab5fb1) SHA1(15b3dbf0242e885b7009c21479544a821d4e5a7d) )

	ROM_REGION( 0x1000, "mcu", 0 ) // 68705
	ROM_LOAD( "mc68hc705c8p_main.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "mcu2", 0 ) // 68705
	ROM_LOAD( "mc68hc705c8p_sub.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "blitter", 0 ) // data for the blitter
	ROM_LOAD( "u46.2", 0x000000, 0x80000, CRC(c4a5e47b) SHA1(9f3d3124c76c0bdf8cdca849e1d921a335e433b6) )
	ROM_LOAD( "u51.3", 0x080000, 0x80000, CRC(4dc57435) SHA1(7dfa6f9e35986dd48869786abbe70103f336bcb1) )
	ROM_LOAD( "u61.4", 0x100000, 0x80000, CRC(d8d8dc6f) SHA1(5a76b1fd1a3a532e5ff2de127286ace7d3567c58) )
	ROM_LOAD( "u66.5", 0x180000, 0x80000, CRC(da309671) SHA1(66baf8a83024547c471da39748ff99a9a9013ea4) )

	ROM_REGION( 0x80000, "oki", 0 ) // Sound Samples
	ROM_LOAD( "u18.7", 0x00000, 0x80000, CRC(ee942232) SHA1(b9c1fc73c6006bcad0dd177e0f30a96f1063a993) )
ROM_END


static DRIVER_INIT( cjffruit )
{
	UINT16 *ROM = (UINT16 *)memory_region(machine, "maincpu");

	// WRONG C8 #1
	ROM[0xf564/2] = 0x6028;

	// ERROR CHECKSUM ROM PROGRAM
	ROM[0x1e7b8/2] = 0x6050;
}


GAME( 199?, ilpag,    0, ilpag,    ilpag,    0,        ROT0,  "<unknown>",     "Il Pagliaccio (Italy, Version 2.7C)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1993, steaser,  0, steaser,  steaser,  0,        ROT0,  "<unknown>",     "Strip Teaser (Italy, Version 1.22)",  GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS ) // In-game strings are in Italian but service mode is half English / half French?
GAME( 1998, cjffruit, 0, cjffruit, cjffruit, cjffruit, ROT0,  "Cadillac Jack", "Funny Fruit (Version 1.13)",          GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND )
