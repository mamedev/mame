/* Little Robin */

/* driver by
Pierpaolo Prazzoli
David Haywood
*/

/*

strange vdp/memory access chip..
at the moment gfx / palettes etc. get drawn then erased

maybe it needs read commands working so it knows how many sprites are in
the list?

maybe the vdp 'commands' can be used to store and get back
write addresses?

Dip sw.1
--------
             | Coin 1 | Coin 2  |
              1  2  3   4  5  6   7  8   Coin  Play
---------------------------------------------------
 Coins        -  -  -   -  -  -           1     4
              +  -  -   +  -  -           1     3
              -  +  -   -  +  -           1     2
              +  +  -   +  +  -           1     1
              -  -  +   -  -  +           2     1
              +  -  +   +  -  +           3     1
              -  +  +   -  +  +           4     1
              +  +  +   +  +  +           5     1
 Player                           -  -    2
                                  +  -    3
                                  -  +    4
                                  +  +    5


Dip sw.2
--------          1  2  3  4  5  6  7  8
-----------------------------------------------------------
 Demo Sound       -                        Yes
                  +                        No
 Mode                -                     Test Mode
                     +                     Game Mode
 Difficulty             -  -  -            0 (Easy)
                        +  -  -            1
                        -  +  -            2 (Normal)
                        +  +  -            3
                        -  -  +            4
                        +  -  +            5
                        -  +  +            6
                        +  +  +            7 (Hardest)
 Bonus                           -  -      Every 150000
                                 +  -      Every 200000
                                 -  +      Every 300000
                                 +  +      No Bonus


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"


class littlerb_state : public driver_device
{
public:
	littlerb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_vdp_address_low;
	UINT16 m_vdp_address_high;
	UINT16 m_vdp_writemode;
	UINT32 m_write_address;
	UINT16* m_region4;
	UINT8 m_paldac[3][0x80];
	int m_paldac_select;
	int m_paldac_offset;
	int m_type2_writes;
	UINT32 m_lasttype2pc;
};


WRITE16_HANDLER( region4_w )
{
	littlerb_state *state = space->machine().driver_data<littlerb_state>();
	COMBINE_DATA(&state->m_region4[offset]);
}

WRITE16_HANDLER(palette_offset_w)
{
	littlerb_state *state = space->machine().driver_data<littlerb_state>();
	//printf("palette offset set to %04x\n",data);
	state->m_paldac_offset = data;
	state->m_paldac_select = 0;
	state->m_paldac_offset&=0x7f;

}

WRITE16_HANDLER( palette_data_w )
{
	littlerb_state *state = space->machine().driver_data<littlerb_state>();
	//printf("palette write %04x\n",data);

	state->m_paldac[state->m_paldac_select][state->m_paldac_offset] = data;
	state->m_paldac_select++;
	if (state->m_paldac_select==3)
	{
		int r,g,b;

		r = state->m_paldac[0][state->m_paldac_offset];
		g = state->m_paldac[1][state->m_paldac_offset];
		b = state->m_paldac[2][state->m_paldac_offset];

		palette_set_color(space->machine(),state->m_paldac_offset,MAKE_RGB(r,g,b));

		state->m_paldac_select = 0;
		state->m_paldac_offset++;
		state->m_paldac_offset&=0x7f;
	}
}

WRITE16_HANDLER( palette_reset_w )
{
	littlerb_state *state = space->machine().driver_data<littlerb_state>();
//  printf("palette reset write %04x\n",data);

	state->m_paldac_select = 0;
	state->m_paldac_offset = 0;

}

/* this map is wrong because our VDP access is wrong! */
static ADDRESS_MAP_START( littlerb_vdp_map8, AS_0, 16 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM_WRITE(region4_w)

	/* these are definitely written by a non-incrementing access to the VDP */
	AM_RANGE(0x00800000, 0x00800001) AM_WRITE(palette_offset_w)
	AM_RANGE(0x00800002 ,0x00800003) AM_WRITE(palette_data_w)
	AM_RANGE(0x00800004 ,0x00800005) AM_WRITE(palette_reset_w)



	AM_RANGE(0x0ff80000, 0x0fffffff) AM_RAM_WRITE(region4_w)


	AM_RANGE(0x1ff80000, 0x1fffffff)  AM_RAM_WRITE(region4_w) AM_BASE_MEMBER(littlerb_state, m_region4)
ADDRESS_MAP_END



/* VDP device to give us our own memory map */
class littlerb_vdp_device;


class littlerb_vdp_device : public device_t,
						  public device_memory_interface
{
public:
	littlerb_vdp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	virtual void device_start() { }
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		return (spacenum == 0) ? &m_space_config : NULL;
	}

	address_space_config		m_space_config;
};

const device_type LITTLERBVDP = &device_creator<littlerb_vdp_device>;

littlerb_vdp_device::littlerb_vdp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LITTLERBVDP, "LITTLERBVDP", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  m_space_config("littlerb_vdp", ENDIANNESS_LITTLE, 16,32, 0, NULL, *ADDRESS_MAP_NAME(littlerb_vdp_map8))
{
}



/* end VDP device to give us our own memory map */


static void littlerb_recalc_regs(running_machine &machine)
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	state->m_vdp_address_low = state->m_write_address&0xffff;
	state->m_vdp_address_high = (state->m_write_address>>16)&0xffff;
}




static void littlerb_data_write(running_machine &machine, UINT16 data, UINT16 mem_mask)
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	UINT32 addr = state->m_write_address>>4; // is this right? should we shift?
	address_space *vdp_space = machine.device<littlerb_vdp_device>("littlerbvdp")->space();


	vdp_space->write_word(addr*2, data, mem_mask);


	// e000 / 2000 are used for palette writes, which should go to a RAMDAC, so probably mean no auto inc.
	if ((state->m_vdp_writemode!=0xe000) && (state->m_vdp_writemode!=0x2000)) state->m_write_address+=0x10;
	littlerb_recalc_regs(machine);

}




static void littlerb_recalc_address(running_machine &machine)
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	state->m_write_address = state->m_vdp_address_low | state->m_vdp_address_high<<16;
}

static READ16_HANDLER( littlerb_vdp_r )
{
	littlerb_state *state = space->machine().driver_data<littlerb_state>();
	logerror("%06x littlerb_vdp_r offs %04x mask %04x\n", cpu_get_pc(&space->device()), offset, mem_mask);

	switch (offset)
	{
		case 0:
		return state->m_vdp_address_low;

		case 1:
		return state->m_vdp_address_high;

		case 2:
		return 0; // data read? -- startup check expects 0 for something..

		case 3:
		return state->m_vdp_writemode;
	}

	return -1;
}

#define LOG_VDP 0
static WRITE16_HANDLER( littlerb_vdp_w )
{
	littlerb_state *state = space->machine().driver_data<littlerb_state>();

	if (offset!=2)
	{
		if (state->m_type2_writes)
		{
			if (state->m_type2_writes>2)
			{
				if (LOG_VDP) logerror("******************************* BIG WRITE OCCURRED BEFORE THIS!!! ****************************\n");
			}

			if (LOG_VDP) logerror("~%06x previously wrote %08x data bytes\n", state->m_lasttype2pc, state->m_type2_writes*2);
			state->m_type2_writes = 0;
		}

		if (LOG_VDP) logerror("%06x littlerb_vdp_w offs %04x data %04x mask %04x\n", cpu_get_pc(&space->device()), offset, data, mem_mask);
	}
	else
	{
		if (mem_mask==0xffff)
		{
			if (state->m_type2_writes==0)
			{
				if (LOG_VDP) logerror("data write started %06x %04x data %04x mask %04x\n", cpu_get_pc(&space->device()), offset, data, mem_mask);
			}

			state->m_type2_writes++;
			state->m_lasttype2pc = cpu_get_pc(&space->device());
		}
		else
		{
			if (LOG_VDP) logerror("xxx %06x littlerb_vdp_w offs %04x data %04x mask %04x\n", cpu_get_pc(&space->device()), offset, data, mem_mask);
		}
	}


	switch (offset)
	{
		case 0:
		state->m_vdp_address_low = data;
		littlerb_recalc_address(space->machine());
		break;

		case 1:
		state->m_vdp_address_high = data;
		littlerb_recalc_address(space->machine());
		break;


		case 2:
		littlerb_data_write(space->machine(), data, mem_mask);
		break;

		case 3:
		logerror("WRITE MODE CHANGED TO %04x\n",data);
		state->m_vdp_writemode = data;
		break;

	}

}

static ADDRESS_MAP_START( littlerb_main, AS_PROGRAM, 16 )
	AM_RANGE(0x000008, 0x000017) AM_WRITENOP
	AM_RANGE(0x000020, 0x00002f) AM_WRITENOP
	AM_RANGE(0x000070, 0x000073) AM_WRITENOP
	AM_RANGE(0x060004, 0x060007) AM_WRITENOP
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x203fff) AM_RAM // main ram?
	AM_RANGE(0x7c0000, 0x7c0001) AM_READ_PORT("DSW")
	AM_RANGE(0x7e0000, 0x7e0001) AM_READ_PORT("P1")
	AM_RANGE(0x7e0002, 0x7e0003) AM_READ_PORT("P2")
	AM_RANGE(0x700000, 0x700007) AM_READ(littlerb_vdp_r) AM_WRITE(littlerb_vdp_w)
	AM_RANGE(0x780000, 0x780001) AM_WRITENOP
ADDRESS_MAP_END


static INPUT_PORTS_START( littlerb )
	PORT_START("DSW")	/* 16bit */
	PORT_DIPNAME( 0x0003, 0x0001, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x001c, 0x0004, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00e0, 0x0020, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_4C ) )
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
	PORT_DIPNAME( 0x4000, 0x4000, "GAME/TEST??" ) // changes what gets uploaded
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1")	/* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x1000, 0x1000, "???"  )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")	/* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static void draw_sprite(running_machine &machine, bitmap_t *bitmap, int xsize,int ysize, int offset, int xpos, int ypos, int pal )
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	UINT16* spritegfx = state->m_region4;
	int x,y;
	//int pal = 1;

	for (y=0;y<ysize;y++)
	{
		for (x=0;x<xsize;x++)
		{
			int drawxpos, drawypos;
			UINT8 pix1 = spritegfx[offset]&0x0f;
			UINT8 pix2 = (spritegfx[offset]>>8)&0x0f;
			drawxpos = xpos+x*2;
			drawypos = ypos+y;

			pix1+=pal*0x10;
			pix2+=pal*0x10;


			if ((drawxpos < 320) && (drawypos < 256) && (drawxpos >= 0) && (drawypos >=0))
			{
				if(pix1&0xf) *BITMAP_ADDR16(bitmap, drawypos, drawxpos) = pix1;
			}
			drawxpos++;
			if ((drawxpos < 320) && (drawypos < 256) && (drawxpos >= 0) && (drawypos >=0))
			{
				if(pix2&0xf) *BITMAP_ADDR16(bitmap, drawypos, drawxpos) = pix2;
			}

			offset++;

			offset&=0x3ffff;
		}
	}
}

/* sprite format / offset could be completely wrong, this is just based on our (currently incorrect) vram access */
static SCREEN_UPDATE(littlerb)
{
	littlerb_state *state = screen->machine().driver_data<littlerb_state>();
	int x,y,offs, code;
	int xsize,ysize;
	int pal;
	UINT16* spriteregion = &state->m_region4[0x400];
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));
	//printf("frame\n");
	/* the spriteram format is something like this .. */
	for (offs=0x26/2;offs<0xc00;offs+=6) // start at 00x26?
	{
		x = spriteregion[offs+2] & 0x01ff;
		ysize = (spriteregion[offs+5] & 0x007f);
		y = (spriteregion[offs+3] & 0x01ff); // 1?
		xsize = (spriteregion[offs+4] & 0x00ff)/2;

		// the code seems to be the same address as the blitter writes
		// e.g  ffc010000
		code =  (spriteregion[offs+0] & 0xfff0)>>4;
		code |=  (spriteregion[offs+1] & 0x003f)<<12;

		pal = 0;//(spriteregion[offs+4] & 0xf000)>>13; // where is the colour bit?!

		//if (code!=0) printf("%04x %04x %04x %04x %04x %04x\n", spriteregion[offs+0], spriteregion[offs+1], spriteregion[offs+2], spriteregion[offs+3], spriteregion[offs+4], spriteregion[offs+5]);

		draw_sprite(screen->machine(),bitmap,xsize,ysize,code,x-8,y-16, pal);
	}

	return 0;
}

static INTERRUPT_GEN( littlerb )
{
	logerror("IRQ\n");
	device_set_input_line(device, 4, HOLD_LINE);
}

static MACHINE_CONFIG_START( littlerb, littlerb_state )
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(littlerb_main)
	MCFG_CPU_VBLANK_INT("screen", littlerb)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 320-1, 0*8, 256-1)
	MCFG_SCREEN_UPDATE(littlerb)

	MCFG_PALETTE_LENGTH(256)

	MCFG_DEVICE_ADD("littlerbvdp", LITTLERBVDP, 0)

//  MCFG_PALETTE_INIT(littlerb)
MACHINE_CONFIG_END

ROM_START( littlerb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "roma.u53", 0x00001, 0x80000, CRC(172fbc13) SHA1(cd165ca0d0546e2634cf182dc98004cbfb02cf9f) )
	ROM_LOAD16_BYTE( "romb.u29", 0x00000, 0x80000, CRC(b2fb1d61) SHA1(9a9d7176c241928d07af651e5f7f21d4f019701d) )

	ROM_REGION( 0x80000, "samples", 0 ) /* sound samples */
	ROM_LOAD16_BYTE( "romc.u26", 0x00001, 0x40000, CRC(f193c5b6) SHA1(95548a40e2b5064c558b36cabbf507d23678b1b2) )
	ROM_LOAD16_BYTE( "romd.u32", 0x00000, 0x40000, CRC(d6b81583) SHA1(b7a63d18a41ccac4d3db9211de0b0cdbc914317a) )
ROM_END


GAME( 1993, littlerb, 0, littlerb, littlerb, 0, ROT0, "TCH", "Little Robin", GAME_NOT_WORKING|GAME_NO_SOUND )
