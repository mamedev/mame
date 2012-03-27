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

most graphics are stored as packed 4bpp in RAM, expanded before
writing to the vdp device.  actual gfx are 8bpp.


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
#include "video/ramdac.h"

class littlerb_state : public driver_device
{
public:
	littlerb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_vdp_address_low;
	UINT16 m_vdp_address_high;
	UINT16 m_vdp_writemode;
	UINT32 m_write_address;

	UINT32 m_write_address_laststart;
	UINT32 m_write_address_lastend;

	UINT16* m_region4;
	UINT8 m_paldac[3][0x100];
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

/* this map is wrong because our VDP access is wrong! */
static ADDRESS_MAP_START( littlerb_vdp_map8, AS_0, 16 )
	// it ends up writing some gfx here (the bubbles when you shoot an enemy)
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM 

	/* these are definitely written by a non-incrementing access to the VDP */
	AM_RANGE(0x00800000, 0x00800001) AM_DEVWRITE8_MODERN("^ramdac", ramdac_device, index_w, 0x00ff)
	AM_RANGE(0x00800002 ,0x00800003) AM_DEVWRITE8_MODERN("^ramdac", ramdac_device, pal_w,   0x00ff)
	AM_RANGE(0x00800004 ,0x00800005) AM_DEVWRITE8_MODERN("^ramdac", ramdac_device, mask_w,  0x00ff)
               
	// most gfx end up here including the sprite list
	AM_RANGE(0x1ff80000, 0x1fffffff) AM_RAM_WRITE(region4_w)  AM_BASE_MEMBER(littlerb_state, m_region4) 
ADDRESS_MAP_END

static ADDRESS_MAP_START( ramdac_map, AS_0, 8 )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE_MODERN("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb888_w)
ADDRESS_MAP_END

static RAMDAC_INTERFACE( ramdac_intf )
{
	0
};

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

	int mode = state->m_vdp_writemode;
	if ((mode!=0x3800) && (mode !=0x2000))
	{
		printf("mode %04x, data %04x, mem_mask %04x (address %08x)\n", mode,  data, mem_mask, state->m_write_address);
	}
	else
	{
		vdp_space->write_word(addr*2, data, mem_mask);

		// 2000 is used for palette writes which appears to be a RAMDAC, no auto-inc.
		if (mode!=0x2000) state->m_write_address+=0x10;
		littlerb_recalc_regs(machine);
	}




}




static void littlerb_recalc_address(running_machine &machine)
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	state->m_write_address = state->m_vdp_address_low | state->m_vdp_address_high<<16;
}

static READ16_HANDLER( littlerb_vdp_r )
{
	littlerb_state *state = space->machine().driver_data<littlerb_state>();
	logerror("%06x littlerb_vdp_r offs %04x mask %04x (address %08x)\n", cpu_get_pc(&space->device()), offset, mem_mask, state->m_write_address);

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
				printf("big write occured with start %08x end %08x\n", state->m_write_address_laststart, state->m_write_address_lastend);
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
			if (state->m_type2_writes==0) state->m_write_address_laststart = state->m_write_address;
			state->m_write_address_lastend = state->m_write_address;
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
			COMBINE_DATA(&state->m_vdp_address_low);
			littlerb_recalc_address(space->machine());
		break;

		case 1:
			COMBINE_DATA(&state->m_vdp_address_high);
			littlerb_recalc_address(space->machine());
		break;


		case 2:
		littlerb_data_write(space->machine(), data, mem_mask);
		break;

		case 3:
			COMBINE_DATA(&state->m_vdp_writemode);
			int mode = state->m_vdp_writemode;
			if ((mode!=0x3800) && (mode !=0x2000)) printf("WRITE MODE CHANGED TO %04x\n",mode);
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


static void draw_sprite(running_machine &machine, bitmap_ind16 &bitmap, int xsize,int ysize, UINT32 fulloffs, int xpos, int ypos )
{
	int x,y;
	fulloffs >>= 3;
	address_space *vdp_space = machine.device<littlerb_vdp_device>("littlerbvdp")->space();

	for (y=0;y<ysize;y++)
	{
		for (x=0;x<xsize;x++)
		{
			int drawxpos, drawypos;
			// the addresses provided are the same as the offsets as the vdp writes
			UINT16 pix = vdp_space->read_byte(fulloffs);

			drawxpos = xpos+x;
			drawypos = ypos+y;

			if ((drawxpos < 320) && (drawypos < 256) && (drawxpos >= 0) && (drawypos >=0))
			{
				if(pix&0xff) bitmap.pix16(drawypos, drawxpos) = pix;
			}
			drawxpos++;

			fulloffs++;

		}
	}
}

/* sprite format / offset could be completely wrong, this is just based on our (currently incorrect) vram access */
static SCREEN_UPDATE_IND16(littlerb)
{
	littlerb_state *state = screen.machine().driver_data<littlerb_state>();
	int x,y,offs;
	int xsize,ysize;
	UINT16* spriteregion = &state->m_region4[0x400];
	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	//printf("frame\n");
	/* the spriteram format is something like this .. */
	for (offs=0x26/2;offs<0xc00;offs+=6) // start at 00x26?
	{
		x = spriteregion[offs+2] & 0x01ff;
		ysize = (spriteregion[offs+5] & 0x007f);
		y = (spriteregion[offs+3] & 0x01ff); // 1?
		xsize = (spriteregion[offs+4] & 0x00ff);

		UINT32 fullcode = ((spriteregion[offs+1])<<16)+ (spriteregion[offs+0]);

		//printf( "sprite %08x\n", fullcode);

		//if (code!=0) printf("%04x %04x %04x %04x %04x %04x\n", spriteregion[offs+0], spriteregion[offs+1], spriteregion[offs+2], spriteregion[offs+3], spriteregion[offs+4], spriteregion[offs+5]);

		draw_sprite(screen.machine(),bitmap,xsize,ysize,fullcode,x-8,y-16);
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
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 320-1, 0*8, 256-1)
	MCFG_SCREEN_UPDATE_STATIC(littlerb)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_DEVICE_ADD("littlerbvdp", LITTLERBVDP, 0)
	MCFG_RAMDAC_ADD("ramdac", ramdac_intf, ramdac_map)

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
