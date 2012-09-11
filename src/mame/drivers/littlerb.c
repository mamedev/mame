/* Little Robin */

/* driver by
Pierpaolo Prazzoli
David Haywood
*/


#define littlerb_printf logerror
#define littlerb_alt_printf logerror

/*

Notes:

VDP (Blitter) handling is not 100% correct
A brief original video (recorded by Dox) can be seen at https://www.youtube.com/watch?v=8THpeogarUk

Overall addressing / auto-increment etc. of the VDP device is not fully understood, but for now
appears to be good enough for the game.

How we distinguish between mode setting (clear, copy, cliprect etc.) VDP commands and actual sprite
commands is not yet understood.  All 'sprite' sections of the blit list seem to be terminated with
a 0x0000 word, but it isn't clear how the blocks are started, the current method relies on some bits
of the sprite data offset to determine if we're sprite data, or a command.  Maybe this is just a
quirk of the hardware, and you can't have sprites at those offsets?

Copy / Scroll are not yet implemented, see the Smileys between scenes in the original video.
 (Clipping is implemented, but might be per layer, so you do see the sprites vanish, but no
   smileys are drawn)

How big are the actual framebuffers?  Are both also double buffered?

Sound pitch is directly correlated with irqs, scanline timings and pixel clock,
so it's surely not 100% correct. Sound sample playbacks looks fine at current time tho.

------



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
#include "sound/dac.h"

static void littlerb_draw_sprites(running_machine &machine);

class littlerb_state : public driver_device
{
public:
	littlerb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_dacl(*this, "dacl"),
	      m_dacr(*this, "dacr"),
		m_region4(*this, "region4")
	{
		m_1ff80804 = -1;
	}

	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dacl;
	required_device<dac_device> m_dacr;
	required_shared_ptr<UINT16> m_region4;
	UINT16 m_vdp_address_low;
	UINT16 m_vdp_address_high;
	UINT16 m_vdp_writemode;
	UINT32 m_write_address;

	UINT32 m_write_address_laststart;
	UINT32 m_write_address_lastend;

	UINT8 m_paldac[3][0x100];
	int m_paldac_select;
	int m_paldac_offset;
	int m_type2_writes;
	UINT32 m_lasttype2pc;
	UINT8 m_sound_index_l,m_sound_index_r;
	UINT16 m_sound_pointer_l,m_sound_pointer_r;

	bitmap_ind16 *m_temp_bitmap_sprites;
	bitmap_ind16 *m_temp_bitmap_sprites_back;


	DECLARE_WRITE16_MEMBER(spritelist_w);
	DECLARE_WRITE16_MEMBER(region4_w);
	DECLARE_READ16_MEMBER(buffer_status_r);
	DECLARE_READ16_MEMBER(littlerb_vdp_r);
	DECLARE_WRITE16_MEMBER(littlerb_vdp_w);
	DECLARE_WRITE16_MEMBER(littlerb_l_sound_w);
	DECLARE_WRITE16_MEMBER(littlerb_r_sound_w);
	DECLARE_CUSTOM_INPUT_MEMBER(littlerb_frame_step_r);

	int m_1ff80804;
	int m_listoffset;
	UINT16* m_spritelist;

	DECLARE_READ16_MEMBER( littlerb_1ff80800_r )
	{
		littlerb_printf("littlerb_1ff80800_r\n");
		return 0x0000;
	}

	DECLARE_WRITE16_MEMBER( littlerb_1ff80800_w )
	{
		littlerb_printf("littlerb_1ff80800_w %04x\n", data);
	}

	DECLARE_READ16_MEMBER( littlerb_1ff80802_r )
	{
		littlerb_printf("littlerb_1ff80802_r\n");
		return 0x0000;
	}

	DECLARE_WRITE16_MEMBER( littlerb_1ff80802_w )
	{
		littlerb_printf("littlerb_1ff80802_w %04x\n", data);
	}

	DECLARE_WRITE16_MEMBER( littlerb_1ff80804_w )
	{
		littlerb_printf("littlerb_1ff80804_w %04x\n", data);

		if ((!(m_spritelist[2] & 0x1000)) && (!(m_spritelist[1] & 0x1000)))
		{

		}
		else
		{
			if (!(m_spritelist[2] & 0x1000))
				m_temp_bitmap_sprites_back->fill(0, m_temp_bitmap_sprites_back->cliprect());

		}

		littlerb_draw_sprites(space.machine());


		m_listoffset = 0;
		memset(m_spritelist, 0x00, 0x20000); // clear out the list.. I think it's commands anyway?

	}

	DECLARE_WRITE16_MEMBER( littlerb_18000012_w )
	{
		littlerb_printf("littlerb_18000012_w %04x\n", data);
		// edfc / fffc alternating (select double buffering?)
	}

	DECLARE_WRITE16_MEMBER( littlerb_18000014_w )
	{
		littlerb_printf("littlerb_18000014_w %04x\n", data);
		// dosen't seem to be the draw trigger, or you get broken platforms..


	}

	DECLARE_WRITE16_MEMBER( littlerb_1800003c_w )
	{
		littlerb_printf("littlerb_1800003c_w %04x\n", data);
		// edfc / fffc alternating (display double buffering?)
	}
};


WRITE16_MEMBER(littlerb_state::region4_w)
{
	COMBINE_DATA(&m_region4[offset]);
}

READ16_MEMBER(littlerb_state::buffer_status_r)
{
	return 0;
}


WRITE16_MEMBER(littlerb_state::spritelist_w)
{
	littlerb_printf("spritelist_w %04x\n",  data);

	COMBINE_DATA(&m_spritelist[m_listoffset]);
	m_listoffset++;
}



/* this map is wrong because our VDP access is wrong! */
static ADDRESS_MAP_START( littlerb_vdp_map8, AS_0, 16, littlerb_state )
	// it ends up writing some gfx here (the bubbles when you shoot an enemy)
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM
	AM_RANGE(0x00080000, 0x003fffff) AM_RAM // temp so it doesn't fill the log

	/* these are definitely written by a non-incrementing access to the VDP */
	AM_RANGE(0x00800000, 0x00800001) AM_DEVWRITE8("^ramdac", ramdac_device, index_w, 0x00ff)
	AM_RANGE(0x00800002 ,0x00800003) AM_DEVWRITE8("^ramdac", ramdac_device, pal_w,   0x00ff)
	AM_RANGE(0x00800004 ,0x00800005) AM_DEVWRITE8("^ramdac", ramdac_device, mask_w,  0x00ff)
	AM_RANGE(0x18000012, 0x18000013) AM_DEVWRITE("^", littlerb_state, littlerb_18000012_w)
	AM_RANGE(0x18000014, 0x18000015) AM_DEVWRITE("^", littlerb_state, littlerb_18000014_w)
	AM_RANGE(0x1800003c, 0x1800003d) AM_DEVWRITE("^", littlerb_state, littlerb_1800003c_w)
	AM_RANGE(0x1ff80800, 0x1ff80801) AM_DEVREADWRITE("^", littlerb_state, littlerb_1ff80800_r, littlerb_1ff80800_w)
	AM_RANGE(0x1ff80802, 0x1ff80803) AM_DEVREADWRITE("^", littlerb_state, littlerb_1ff80802_r, littlerb_1ff80802_w)
	AM_RANGE(0x1ff80804, 0x1ff80805) AM_DEVREADWRITE("^", littlerb_state, buffer_status_r, littlerb_1ff80804_w)
	AM_RANGE(0x1ff80806, 0x1ff80807) AM_RAM AM_DEVWRITE("^", littlerb_state, spritelist_w)

	// most gfx end up here including the sprite list
	AM_RANGE(0x1ff80000, 0x1fffffff) AM_RAM AM_DEVWRITE("^", littlerb_state, region4_w)  AM_SHARE("region4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( ramdac_map, AS_0, 8, littlerb_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb888_w)
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


static UINT16 littlerb_data_read(running_machine &machine, UINT16 mem_mask)
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	UINT32 addr = state->m_write_address >> 3; // almost surely raw addresses are actually shifted by 3
	address_space *vdp_space = machine.device<littlerb_vdp_device>("littlerbvdp")->space();

	return vdp_space->read_word(addr, mem_mask);
}

static void littlerb_data_write(running_machine &machine, UINT16 data, UINT16 mem_mask)
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	UINT32 addr = state->m_write_address >> 3; // almost surely raw addresses are actually shifted by 3
	address_space *vdp_space = machine.device<littlerb_vdp_device>("littlerbvdp")->space();
	int mode = state->m_vdp_writemode;


	logerror("mode %04x, data %04x, mem_mask %04x (address %08x)\n", mode,  data, mem_mask, state->m_write_address >> 3);

	if ((mode!=0x3800) && (mode !=0x2000) && (mode != 0xe000) && (mode != 0xf800))
	{
	}
	else
	{
		vdp_space->write_word(addr, data, mem_mask);

		// 2000 is used for palette writes which appears to be a RAMDAC, no auto-inc.
		//  1ff80806 is our 'spritelist'
		if (mode!=0x2000 && mode != 0xe000 && addr != 0x1ff80806) state->m_write_address+=0x10;
		littlerb_recalc_regs(machine);
	}




}




static void littlerb_recalc_address(running_machine &machine)
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	state->m_write_address = state->m_vdp_address_low | state->m_vdp_address_high<<16;
}

READ16_MEMBER(littlerb_state::littlerb_vdp_r)
{
	logerror("%06x littlerb_vdp_r offs %04x mask %04x (address %08x)\n", space.device().safe_pc(), offset, mem_mask, m_write_address);
	UINT16 res = 0;

	switch (offset & 3)
	{
		case 0: res = m_vdp_address_low; break;
		case 1: res = m_vdp_address_high; break;
		case 2: res = littlerb_data_read(machine(), mem_mask); break;
		case 3: res = m_vdp_writemode; break;
	}

	return res;
}

#define LOG_VDP 1
WRITE16_MEMBER(littlerb_state::littlerb_vdp_w)
{

	if (offset!=2)
	{
		if (m_type2_writes)
		{
			if (m_type2_writes>2)
			{
				if (LOG_VDP) logerror("******************************* BIG WRITE OCCURRED BEFORE THIS!!! ****************************\n");
			}

			littlerb_printf("~%06x big write occured with start %08x end %08x (size %04x bytes)\n", m_lasttype2pc, m_write_address_laststart >> 3, m_write_address_lastend >> 3, m_type2_writes*2);

			if (LOG_VDP) logerror("~%06x previously wrote %08x data bytes\n", m_lasttype2pc, m_type2_writes*2);
			m_type2_writes = 0;
		}

		if (LOG_VDP) logerror("%06x littlerb_vdp_w offs %04x data %04x mask %04x\n", space.device().safe_pc(), offset, data, mem_mask);
	}
	else
	{
		if (mem_mask==0xffff)
		{
			if (m_type2_writes==0)
			{
				if (LOG_VDP) logerror("data write started %06x %04x data %04x mask %04x\n", space.device().safe_pc(), offset, data, mem_mask);
			}
			if (m_type2_writes==0) m_write_address_laststart = m_write_address;
			m_write_address_lastend = m_write_address;
			m_type2_writes++;
			m_lasttype2pc = space.device().safe_pc();
		}
		else
		{
			if (LOG_VDP) logerror("xxx %06x littlerb_vdp_w offs %04x data %04x mask %04x\n", space.device().safe_pc(), offset, data, mem_mask);
		}
	}


	switch (offset)
	{
		case 0:
			COMBINE_DATA(&m_vdp_address_low);
			littlerb_recalc_address(machine());
		break;

		case 1:
			COMBINE_DATA(&m_vdp_address_high);
			littlerb_recalc_address(machine());
		break;


		case 2:
		littlerb_data_write(machine(), data, mem_mask);
		break;

		case 3:
			COMBINE_DATA(&m_vdp_writemode);
			int mode = m_vdp_writemode;
			if ((mode!=0x3800) && (mode !=0x2000)) logerror("WRITE MODE CHANGED TO %04x\n",mode);
		break;

	}

}

/* could be slightly different (timing wise, directly related to the irqs), but certainly they smoked some bad pot for this messy way ... */
static UINT8 sound_data_shift(running_machine &machine)
{
	return ((machine.primary_screen->frame_number() % 16) == 0) ? 8 : 0;
}

/* l is SFX, r is BGM (they doesn't seem to share the same data ROM) */
WRITE16_MEMBER(littlerb_state::littlerb_l_sound_w)
{
	m_sound_index_l = (data >> sound_data_shift(machine())) & 0xff;
	m_sound_pointer_l = 0;
	//popmessage("%04x %04x",m_sound_index_l,m_sound_index_r);
}

WRITE16_MEMBER(littlerb_state::littlerb_r_sound_w)
{
	m_sound_index_r = (data >> sound_data_shift(machine())) & 0xff;
	m_sound_pointer_r = 0;
	//popmessage("%04x %04x",m_sound_index_l,m_sound_index_r);
}

static ADDRESS_MAP_START( littlerb_main, AS_PROGRAM, 16, littlerb_state )
	AM_RANGE(0x000008, 0x000017) AM_WRITENOP
	AM_RANGE(0x000020, 0x00002f) AM_WRITENOP
	AM_RANGE(0x000070, 0x000073) AM_WRITENOP
	AM_RANGE(0x060004, 0x060007) AM_WRITENOP
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x203fff) AM_RAM // main ram?
	AM_RANGE(0x700000, 0x700007) AM_READ(littlerb_vdp_r) AM_WRITE(littlerb_vdp_w)
	AM_RANGE(0x740000, 0x740001) AM_WRITE(littlerb_l_sound_w)
	AM_RANGE(0x760000, 0x760001) AM_WRITE(littlerb_r_sound_w)
	AM_RANGE(0x780000, 0x780001) AM_WRITENOP // generic outputs
	AM_RANGE(0x7c0000, 0x7c0001) AM_READ_PORT("DSW")
	AM_RANGE(0x7e0000, 0x7e0001) AM_READ_PORT("P1")
	AM_RANGE(0x7e0002, 0x7e0003) AM_READ_PORT("P2")
ADDRESS_MAP_END

/* guess according to DASM code and checking the gameplay speed, could be different */
CUSTOM_INPUT_MEMBER(littlerb_state::littlerb_frame_step_r)
{
	UINT32 ret = machine().primary_screen->frame_number();

	return (ret) & 7;
}

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
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
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
	PORT_BIT( 0xe000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, littlerb_state,littlerb_frame_step_r, NULL)

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



/* sprite format / offset could be completely wrong, this is just based on our (currently incorrect) vram access */
static SCREEN_UPDATE_IND16(littlerb)
{
	littlerb_state *state = screen.machine().driver_data<littlerb_state>();
	bitmap.fill(0, cliprect);

	copybitmap_trans(bitmap, *state->m_temp_bitmap_sprites_back, 0, 0, 0, 0, cliprect, 0);
	copybitmap_trans(bitmap, *state->m_temp_bitmap_sprites, 0, 0, 0, 0, cliprect, 0);

	return 0;
}

static TIMER_DEVICE_CALLBACK( littlerb_scanline )
{
	littlerb_state *state = timer.machine().driver_data<littlerb_state>();
	int scanline = param;

	if((scanline % 2) == 0)
	{
		UINT8 res;
		UINT8 *sample_rom = state->memregion("samples")->base();

		res = sample_rom[state->m_sound_pointer_l|(state->m_sound_index_l<<10)|0x40000];
		state->m_dacl->write_signed8(res);
		res = sample_rom[state->m_sound_pointer_r|(state->m_sound_index_r<<10)|0x00000];
		state->m_dacr->write_signed8(res);
		state->m_sound_pointer_l++;
		state->m_sound_pointer_l&=0x3ff;
		state->m_sound_pointer_r++;
		state->m_sound_pointer_r&=0x3ff;
	}

//  logerror("IRQ\n");
	if(scanline == 288)
	{
		device_set_input_line(state->m_maincpu, 4, HOLD_LINE);
	}
}

VIDEO_START( littlerb )
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
//	machine.primary_screen->register_screen_bitmap(state->m_temp_bitmap_sprites_back);
//	machine.primary_screen->register_screen_bitmap(state->m_temp_bitmap_sprites);

	state->m_temp_bitmap_sprites_back = auto_bitmap_ind16_alloc(machine,512,512);
	state->m_temp_bitmap_sprites = auto_bitmap_ind16_alloc(machine,512,512);


	state->m_spritelist = (UINT16*)auto_alloc_array_clear(machine, UINT16, 0x20000);

}




static void draw_sprite(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int xsize,int ysize, UINT32 fulloffs, int xpos, int ypos )
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

			// odd clipping behavior, count is only increased for visible lines?
			// I suspect the sprite corruption if you're near the top of the screen
			// between level changes is related to this too, data offset values are
			// probably being adjusted as layer is scrolled (and the cliprect is changed)
			// even if they don't move.
			if(drawypos>=cliprect.min_y)
			{
				fulloffs++;
			}

			if(cliprect.contains(drawxpos, drawypos))
			{
				if(pix&0xff) bitmap.pix16(drawypos, drawxpos) = pix;
			}

			drawxpos++;
		}
	}
}

static void littlerb_draw_sprites(running_machine &machine)
{
	littlerb_state *state = machine.driver_data<littlerb_state>();
	int x,y,offs;
	int xsize,ysize;
	UINT16* spriteregion = state->m_spritelist;
	//littlerb_printf("frame\n");

	int layer = 0;
	int yoffset = 0;

	littlerb_printf("m_listoffset %04x\n", state->m_listoffset );

	littlerb_printf("%04x %04x %04x %04x\n", spriteregion[0], spriteregion[1], spriteregion[2], spriteregion[3]);

	littlerb_alt_printf("start\n");

	int minx = 0 , maxx = 0x14f;
	int miny = 0 , maxy = 0x11f;

	for (offs=0;offs<(state->m_listoffset);)
	{


		UINT32 read_dword = ((spriteregion[offs+1])<<16)+ (spriteregion[offs+0]);

		littlerb_printf("read %08x\n", read_dword);

		// some kind of control words?? (layer selection, layer scrolls?, layer clears?)
		// but the low bits are the sprite offset??? how can this work
		if (spriteregion[offs+0] == 0x0060)
		{
			offs += 6;

		}
		else if (spriteregion[offs+0] == 0x0040)
		{
			littlerb_alt_printf("Control Word %04x %04x %04x %04x %04x %04x ---- ---- ---- ----\n", spriteregion[offs+0], spriteregion[offs+1], spriteregion[offs+2], spriteregion[offs+3], spriteregion[offs+4], spriteregion[offs+5]);

			// some scroll stuff is here (title -> high score transition)
			// maybe also copy area operations?


			// this isn't understood ... note, we still have a hack before draw_sprites is called too

			// usual start of frame
			// Control Word 0040 0090 6000 0026 0000 0012 ---- ---- ---- ----

			// happens in the middle of list during boss death, causes sprites to vanish atm
			// Control Word 0040 ffd6 00aa 00f8 0088 0018 ---- ---- ---- ----
			if (spriteregion[offs+4]==0x6000)
			{
				if (spriteregion[offs+3] & 0x1000)
					state->m_temp_bitmap_sprites_back->fill(0, state->m_temp_bitmap_sprites_back->cliprect());
			}
			else
			{
				if (spriteregion[offs+1] != 0xffd6) state->m_temp_bitmap_sprites->fill(0, state->m_temp_bitmap_sprites->cliprect());
			}



			// this is some kind of scroll or copy area..
			// also some of the other values change
			// this is set AFTER the graphics which need to be scrolled are sent and causes the credit text to bounce up and down instead of
			// anything scrolling
			//yoffset = spriteregion[offs+1] - 0x90;


			offs += 6;
		}
		else if (read_dword == 0x00e40020)
		{
			littlerb_alt_printf("Control Word %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n", spriteregion[offs+0], spriteregion[offs+1], spriteregion[offs+2], spriteregion[offs+3], spriteregion[offs+4], spriteregion[offs+5], spriteregion[offs+6], spriteregion[offs+7], spriteregion[offs+8], spriteregion[offs+9]);

			if (spriteregion[offs+4]==0x6000)
				layer = 1;
			else
				layer = 0;


			minx = spriteregion[offs+6];
			maxx = spriteregion[offs+8];

			miny = spriteregion[offs+7];
			maxy = spriteregion[offs+9];


			offs += 10;
		}
		else if (read_dword == 0x00e40000)
		{
			// same as above?
			littlerb_alt_printf("Control Word %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n", spriteregion[offs+0], spriteregion[offs+1], spriteregion[offs+2], spriteregion[offs+3], spriteregion[offs+4], spriteregion[offs+5], spriteregion[offs+6], spriteregion[offs+7], spriteregion[offs+8], spriteregion[offs+9]);

			if (spriteregion[offs+4]==0x6000)
				layer = 1;
			else
				layer = 0;

			minx = spriteregion[offs+6];
			maxx = spriteregion[offs+8];

			miny = spriteregion[offs+7];
			maxy = spriteregion[offs+9];


			offs += 10;
		}
		else if (read_dword == 0x00000000)
		{
			offs += 1;
		}
		else
		{
			UINT32 fullcode = ((spriteregion[offs+1])<<16)+ (spriteregion[offs+0]);

			x = spriteregion[offs+2] & 0x07ff;
			y = (spriteregion[offs+3] & 0x03ff); // 1?

			if (x&0x400) x-=0x800;
			if (y&0x200) y-=0x400;

			y+= yoffset;

			xsize = (spriteregion[offs+4] & 0x01ff); // background gfx for many places at 0x1ff wide
			ysize = (spriteregion[offs+5] & 0x00ff); // player1/player2 texts in service mode sides are 0xff high


			rectangle clip;

			clip.min_x = minx;
			clip.max_x = maxx;

			clip.min_y = miny;
			clip.max_y = maxy;

			// used between levels, and on boss death to clip sprite at the ground for sinking effect


			if (clip.min_x > state->m_temp_bitmap_sprites->cliprect().max_x)
				clip.min_x =  state->m_temp_bitmap_sprites->cliprect().max_x;

			if (clip.min_x < state->m_temp_bitmap_sprites->cliprect().min_x)
				clip.min_x =  state->m_temp_bitmap_sprites->cliprect().min_x;

			if (clip.max_x > state->m_temp_bitmap_sprites->cliprect().max_x)
				clip.max_x =  state->m_temp_bitmap_sprites->cliprect().max_x;

			if (clip.max_x < state->m_temp_bitmap_sprites->cliprect().min_x)
				clip.max_x =  state->m_temp_bitmap_sprites->cliprect().min_x;


			if (clip.min_y > state->m_temp_bitmap_sprites->cliprect().max_y)
				clip.min_y =  state->m_temp_bitmap_sprites->cliprect().max_y;

			if (clip.min_y < state->m_temp_bitmap_sprites->cliprect().min_y)
				clip.min_y =  state->m_temp_bitmap_sprites->cliprect().min_y;

			if (clip.max_y > state->m_temp_bitmap_sprites->cliprect().max_y)
				clip.max_y =  state->m_temp_bitmap_sprites->cliprect().max_y;

			if (clip.max_y < state->m_temp_bitmap_sprites->cliprect().min_y)
				clip.max_y =  state->m_temp_bitmap_sprites->cliprect().min_y;


			littlerb_alt_printf("%04x %04x %04x %04x %04x %04x\n", spriteregion[offs+0], spriteregion[offs+1], spriteregion[offs+2], spriteregion[offs+3], spriteregion[offs+4], spriteregion[offs+5]);

			if (layer==0) draw_sprite(machine, *state->m_temp_bitmap_sprites, clip,xsize,ysize,fullcode,x,y);
			else draw_sprite(machine, *state->m_temp_bitmap_sprites_back, clip,xsize,ysize,fullcode,x,y);


			offs += 6;
		}
	}
}



static MACHINE_CONFIG_START( littlerb, littlerb_state )
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(littlerb_main)
	MCFG_TIMER_ADD_SCANLINE("scantimer", littlerb_scanline, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50) // guess based on high vertical resolution
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(512+22, 312)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 336-1, 0*8, 288-1)
	MCFG_SCREEN_UPDATE_STATIC(littlerb)

	MCFG_VIDEO_START(littlerb)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_DEVICE_ADD("littlerbvdp", LITTLERBVDP, 0)
	MCFG_RAMDAC_ADD("ramdac", ramdac_intf, ramdac_map)

//  MCFG_PALETTE_INIT(littlerb)
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")

	MCFG_DAC_ADD("dacl")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MCFG_DAC_ADD("dacr")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

ROM_START( littlerb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "roma.u53", 0x00001, 0x80000, CRC(172fbc13) SHA1(cd165ca0d0546e2634cf182dc98004cbfb02cf9f) )
	ROM_LOAD16_BYTE( "romb.u29", 0x00000, 0x80000, CRC(b2fb1d61) SHA1(9a9d7176c241928d07af651e5f7f21d4f019701d) )

	ROM_REGION( 0x80000, "samples", 0 ) /* sound samples */
	ROM_LOAD( "romc.u26", 0x40000, 0x40000, CRC(f193c5b6) SHA1(95548a40e2b5064c558b36cabbf507d23678b1b2) )
	ROM_LOAD( "romd.u32", 0x00000, 0x40000, CRC(d6b81583) SHA1(b7a63d18a41ccac4d3db9211de0b0cdbc914317a) )
ROM_END


GAME( 1994, littlerb, 0, littlerb, littlerb, driver_device, 0, ROT0, "TCH", "Little Robin", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
