/* Little Robin */

/* driver by
Pierpaolo Prazzoli
David Haywood
*/

// this should use a TMS.. the test mode being the same as MegaPhoenix was a givewaway.. maybe also made by Dinamic / Inder for TCH?
#define USE_TMS 1

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
#if USE_TMS
#include "cpu/tms34010/tms34010.h"
#endif

class littlerb_state : public driver_device
{
public:
	littlerb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_screen(*this, "screen"),
			m_dacl(*this, "dacl"),
			m_dacr(*this, "dacr"),

			m_vram(*this, "vram"),
			m_palette(*this, "palette"),
			m_shiftfull(0)

	{
	}




	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<dac_device> m_dacl;
	required_device<dac_device> m_dacr;
	UINT8 m_sound_index_l,m_sound_index_r;
	UINT16 m_sound_pointer_l,m_sound_pointer_r;
	DECLARE_CUSTOM_INPUT_MEMBER(littlerb_frame_step_r);
	DECLARE_WRITE16_MEMBER(littlerb_l_sound_w);
	DECLARE_WRITE16_MEMBER(littlerb_r_sound_w);
	UINT8 sound_data_shift();
	TIMER_DEVICE_CALLBACK_MEMBER(littlerb_scanline);


	required_shared_ptr<UINT16> m_vram;
	required_device<palette_device> m_palette;
	int m_shiftfull; // this might be a driver specific hack for a TMS bug.

};



static ADDRESS_MAP_START( ramdac_map, AS_0, 8, littlerb_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb888_w)
ADDRESS_MAP_END

static RAMDAC_INTERFACE( ramdac_intf )
{
	0
};



/* could be slightly different (timing wise, directly related to the irqs), but certainly they smoked some bad pot for this messy way ... */
UINT8 littlerb_state::sound_data_shift()
{
	return ((m_screen->frame_number() % 16) == 0) ? 8 : 0;
}

/* l is SFX, r is BGM (they doesn't seem to share the same data ROM) */
WRITE16_MEMBER(littlerb_state::littlerb_l_sound_w)
{
	m_sound_index_l = (data >> sound_data_shift()) & 0xff;
	m_sound_pointer_l = 0;
	//popmessage("%04x %04x",m_sound_index_l,m_sound_index_r);
}

WRITE16_MEMBER(littlerb_state::littlerb_r_sound_w)
{
	m_sound_index_r = (data >> sound_data_shift()) & 0xff;
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

	AM_RANGE(0x700000, 0x700007) AM_DEVREADWRITE("tms", tms34010_device, host_r, host_w)

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
	UINT32 ret = m_screen->frame_number();

	return (ret) & 7;
}

static INPUT_PORTS_START( littlerb )
	PORT_START("DSW")   /* 16bit */
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

	PORT_START("P1")    /* 16bit */
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

	PORT_START("P2")    /* 16bit */
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

TIMER_DEVICE_CALLBACK_MEMBER(littlerb_state::littlerb_scanline)
{
	int scanline = param;

	if((scanline % 2) == 0)
	{
		UINT8 res;
		UINT8 *sample_rom = memregion("samples")->base();

		res = sample_rom[m_sound_pointer_l|(m_sound_index_l<<10)|0x40000];
		m_dacl->write_signed8(res);
		res = sample_rom[m_sound_pointer_r|(m_sound_index_r<<10)|0x00000];
		m_dacr->write_signed8(res);
		m_sound_pointer_l++;
		m_sound_pointer_l&=0x3ff;
		m_sound_pointer_r++;
		m_sound_pointer_r&=0x3ff;
	}


	// the TMS generates the main interrupt

}



static ADDRESS_MAP_START( littlerb_tms_map, AS_PROGRAM, 16, littlerb_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x04000000, 0x0400000f) AM_DEVWRITE8("ramdac",ramdac_device,index_w,0x00ff)
	AM_RANGE(0x04000010, 0x0400001f) AM_DEVREADWRITE8("ramdac",ramdac_device,pal_r,pal_w,0x00ff)
	AM_RANGE(0x04000030, 0x0400003f) AM_DEVWRITE8("ramdac",ramdac_device,index_r_w,0x00ff)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("tms", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_RAM
ADDRESS_MAP_END


static void littlerb_scanline(screen_device &screen, bitmap_rgb32 &bitmap, int scanline, const tms34010_display_params *params)
{
	littlerb_state *state = screen.machine().driver_data<littlerb_state>();

	UINT16 *vram = &state->m_vram[(((params->rowaddr << 8)) & 0x3ff00) ];
	UINT32 *dest = &bitmap.pix32(scanline);

	const pen_t *paldata = state->m_palette->pens();

	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = paldata[pixels & 0xff];
		dest[x + 1] = paldata[pixels >> 8];
	}

}

static void littlerb_to_shiftreg(address_space &space, UINT32 address, UINT16 *shiftreg)
{
	littlerb_state *state = space.machine().driver_data<littlerb_state>();

	if (state->m_shiftfull == 0)
	{
		//printf("read to shift regs address %08x (%08x)\n", address, TOWORD(address) * 2);
		memcpy(shiftreg, &state->m_vram[TOWORD(address)/* & ~TOWORD(0x1fff)*/], TOBYTE(0x2000));
		state->m_shiftfull = 1;
	}
}

static void littlerb_from_shiftreg(address_space &space, UINT32 address, UINT16 *shiftreg)
{
	littlerb_state *state = space.machine().driver_data<littlerb_state>();
	memcpy(&state->m_vram[TOWORD(address)/* & ~TOWORD(0x1fff)*/], shiftreg, TOBYTE(0x2000));

	state->m_shiftfull = 0;
}




static void m68k_gen_int(device_t *device, int state)
{
	littlerb_state *drvstate = device->machine().driver_data<littlerb_state>();
	if (state) drvstate->m_maincpu->set_input_line(4, ASSERT_LINE);
	else drvstate->m_maincpu->set_input_line(4, CLEAR_LINE);
}


static const tms34010_config tms_config_littlerb =
{
	TRUE,                          /* halt on reset */
	"screen",                       /* the screen operated on */
	XTAL_40MHz/12,                   /* pixel clock */
	2,                              /* pixels per clock */
	NULL,                           /* scanline callback (indexed16) */
	littlerb_scanline,              /* scanline callback (rgb32) */
	m68k_gen_int,                   /* generate interrupt */
	littlerb_to_shiftreg,           /* write to shiftreg function */
	littlerb_from_shiftreg          /* read from shiftreg function */
};




static MACHINE_CONFIG_START( littlerb, littlerb_state )
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(littlerb_main)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", littlerb_state, littlerb_scanline, "screen", 0, 1)


	MCFG_CPU_ADD("tms", TMS34010, XTAL_40MHz)
	MCFG_CPU_CONFIG(tms_config_littlerb)
	MCFG_CPU_PROGRAM_MAP(littlerb_tms_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_40MHz/12, 424, 0, 338-1, 262, 0, 246-1)
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms34010_device, tms340x0_rgb32)


	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_RAMDAC_ADD("ramdac", ramdac_intf, ramdac_map, "palette")

//  MCFG_PALETTE_INIT_OWNER(littlerb_state,littlerb)
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
