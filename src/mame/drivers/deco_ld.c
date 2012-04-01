/***************************************************************************

Bega's Battle (c) 1983 Data East Corporation

preliminary driver by Angelo Salese

TODO:
- laserdisc hook-ups;
- video emulation is bare bones;
- i/os

***************************************************************************

There are three hardware versions of Cobra Command (LD):

Data East single PCB, Pioneer LD-V1000
Data East 3-boardset, same hardware as Bega's Battle & Road Blaster, Sony
LDP-1000
MACH3 conversion kit (ROMs, disc, decals), Pioneer PR-8210

There are four versions of the laserdisc.

Pioneer (08359)
Data East (Japan), LDS-301 with an orange label
Data East (USA), LDS-301A with a green label
Sony (a1090731704132a)

The Data East labelled discs were released with the DE 3-boardset version
and MACH3 conversion.
The Pioneer Labelled disc was released with the DE single PCB version.
Not sure what version the Sony disc came from. It was given to me by the
copyright owner of Road Blaster, who also gave me a Road Blaster disc/kit
which has a similar Sony label.

I peeled the Data East labels off an orange and a green labelled disc and
the labels underneath were identical to the Sony labelled disc (Sony Japan,
disc No.a1090731704132a).

Physical appearances aside, the Sony and Pioneer pressed discs have
identical content.

===========================================================================


---------------------------------
Bega's Battle by DATA EAST (1983)
---------------------------------
malcor




Location   Device    File ID    Checksum
----------------------------------------
TB 14F      2764      AN00        E929   [ main program ] [ Rev.1 ]
TB 12F      2764      AN01        7B4D   [ main program ] [ Rev.1 ]
TB 11F      2764      AN02        3390   [ main program ] [ Rev.1 ]
TB 9F       2764      AN03        A9E5   [ main program ] [ Rev.1 ]
TB 8F       2764      AN04        303E   [ main program ] [ Rev.1 ]
TB 6F       2764      AN05        3A89   [ main program ] [ Rev.1 ]

TB 14F      2764      AN00-3      E983   [ main program ] [ Rev.3 ]
TB 11F      2764      AN02-3      46DA   [ main program ] [ Rev.3 ]
TB 9F       2764      AN03-3      B99B   [ main program ] [ Rev.3 ]
TB 8F       2764      AN04-3      3A57   [ main program ] [ Rev.3 ]
TB 6F       2764      AN05-3      3A9D   [ main program ] [ Rev.3 ]

TB 15C      2764      AN06        916B   [ snd  program ]
TB 3A       2764      AN07        944B
TB 4A       2764      AN08        798F
TB 6A       2764      AN09        DF57
TB 12A      2764      AN0A        5B95
TB 14A      2764      AN0B        F2C7
TB 15A      2764      AN0C        1605
LB 2F     82S123     AF-8.bpr     00FC   [ DSP select  ]
LB 14K   PAL10L8     LP1-1.pld    6A1A
LB 7C    PAL10L8     LP1-2.pld    6A16
LB 8C    PAL12L6     LP1-3.pld    76B3
LB 11E   PAL12L6     LP1-4.pld    769F
LB 6C    PAL10L8     LP1-5.pld    6A99
LB 12C   PAL10L8     LP1-5.pld    6A99
TB 10H   PAL10L8     LP2-1.pld    6A36
TB C10   PAL10L8     LP2-4.pld    6A05


Note: TB - Top board      VDO-2 DE-0139-1
      LB - Lower board    VDO-1 DE-0138-1

           Laserdisc video game


Brief hardware overview
-----------------------

Main processor  - 6502
                - EF68B50P   communications interface
                - AM2950DC   I/O port to sound processor

Sound processor - 6502
             2x - AY-3-8910

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "machine/ldv1000.h"


class deco_ld_state : public driver_device
{
public:
	deco_ld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_laserdisc(*this, "laserdisc") { }

	UINT8 *m_videoram;
	UINT8 m_vram_bank;
	required_device<pioneer_ldv1000_device> m_laserdisc;
	UINT8 m_laserdisc_data;
	int m_nmimask;
};



static SCREEN_UPDATE_IND16( rblaster )
{
	deco_ld_state *state = screen.machine().driver_data<deco_ld_state>();
	UINT8 *videoram = state->m_videoram;
	const gfx_element *gfx = screen.machine().gfx[0];
	int count = 0x0000;

	int y,x;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int tile = videoram[count];
			int colour = (state->m_vram_bank & 0x7);
			drawgfx_opaque(bitmap,cliprect,gfx,tile,colour,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

#if 0
static WRITE8_HANDLER( rblaster_sound_w )
{
	soundlatch_w(space,0,data);
	device_set_input_line(space->machine().cpu[1], 0, HOLD_LINE);
}
#endif

static WRITE8_HANDLER( rblaster_vram_bank_w )
{
	deco_ld_state *state = space->machine().driver_data<deco_ld_state>();
	state->m_vram_bank = data;
}

static READ8_HANDLER( laserdisc_r )
{
	deco_ld_state *state = space->machine().driver_data<deco_ld_state>();
	UINT8 result = state->m_laserdisc->status_r();
//  mame_printf_debug("laserdisc_r = %02X\n", result);
	return result;
}


static WRITE8_HANDLER( laserdisc_w )
{
	deco_ld_state *state = space->machine().driver_data<deco_ld_state>();
	state->m_laserdisc_data = data;
}

static READ8_HANDLER( test_r )
{
	return space->machine().rand();
}

static ADDRESS_MAP_START( begas_map, AS_PROGRAM, 8, deco_ld_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
//  AM_RANGE(0x1000, 0x1007) AM_NOP
	AM_RANGE(0x1000, 0x1000) AM_READ_LEGACY(test_r)
	AM_RANGE(0x1001, 0x1001) AM_READ_LEGACY(test_r)
	AM_RANGE(0x1002, 0x1002) AM_READ_LEGACY(test_r)
	AM_RANGE(0x1003, 0x1003) AM_READ_LEGACY(test_r)
	AM_RANGE(0x1001, 0x1001) AM_WRITENOP //???
//  AM_RANGE(0x1003, 0x1003) AM_READ_PORT("IN0")
	AM_RANGE(0x1003, 0x1003) AM_WRITE_LEGACY(rblaster_vram_bank_w) //might be 1001
	AM_RANGE(0x1006, 0x1006) AM_NOP //ld status / command
	AM_RANGE(0x1007, 0x1007) AM_READWRITE_LEGACY(laserdisc_r,laserdisc_w) // ld data
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE_LEGACY(paletteram_RRRGGGBB_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_BASE( m_videoram)
	AM_RANGE(0x3000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cobra_map, AS_PROGRAM, 8, deco_ld_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("IN1")
	AM_RANGE(0x1001, 0x1001) AM_READ_LEGACY(test_r)//_PORT("IN2")
	AM_RANGE(0x1002, 0x1002) AM_READ_LEGACY(test_r)//_PORT("IN3")
	AM_RANGE(0x1003, 0x1003) AM_READ_LEGACY(test_r)//AM_READ_PORT("IN0")
//  AM_RANGE(0x1004, 0x1004) AM_READ_LEGACY(test_r)//_PORT("IN4")
//  AM_RANGE(0x1005, 0x1005) AM_READ_LEGACY(test_r)//_PORT("IN5")
	AM_RANGE(0x1004, 0x1004) AM_WRITE_LEGACY(rblaster_vram_bank_w) //might be 1001
	AM_RANGE(0x1006, 0x1006) AM_NOP //ld status / command
	AM_RANGE(0x1007, 0x1007) AM_READWRITE_LEGACY(laserdisc_r,laserdisc_w) // ld data
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE_LEGACY(paletteram_RRRGGGBB_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x2000, 0x2fff) AM_RAM
	AM_RANGE(0x3000, 0x37ff) AM_RAM //vram attr?
	AM_RANGE(0x3800, 0x3fff) AM_RAM AM_BASE( m_videoram)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( rblaster_map, AS_PROGRAM, 8, deco_ld_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
//  AM_RANGE(0x1000, 0x1007) AM_NOP
	AM_RANGE(0x1001, 0x1001) AM_WRITENOP //???
	AM_RANGE(0x1003, 0x1003) AM_READ_PORT("IN0")
	AM_RANGE(0x1003, 0x1003) AM_WRITE_LEGACY(rblaster_vram_bank_w) //might be 1001
	AM_RANGE(0x1006, 0x1006) AM_NOP //ld status / command
	AM_RANGE(0x1007, 0x1007) AM_READWRITE_LEGACY(laserdisc_r,laserdisc_w) // ld data
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE_LEGACY(paletteram_RRRGGGBB_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_BASE( m_videoram)
	AM_RANGE(0x3000, 0x3fff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* sound arrangement is pratically identical to Zero Target. */

#ifdef UNUSED_FUNCTION
static WRITE8_HANDLER( nmimask_w )
{
	deco_ld_state *state = space->machine().driver_data<deco_ld_state>();
	state->m_nmimask = data & 0x80;
}
#endif

static INTERRUPT_GEN ( sound_interrupt )
{
	deco_ld_state *state = device->machine().driver_data<deco_ld_state>();
	if (!state->m_nmimask) device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}


static ADDRESS_MAP_START( rblaster_sound_map, AS_PROGRAM, 8, deco_ld_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_DEVWRITE_LEGACY("ay1", ay8910_data_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE_LEGACY("ay1", ay8910_address_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE_LEGACY("ay2", ay8910_data_w)
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE_LEGACY("ay2", ay8910_address_w)
	AM_RANGE(0xa000, 0xa000) AM_READ_LEGACY(soundlatch_r)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( cobra )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "SYS0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "SYS1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "SYS2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x00, "SYS3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x00, "SYS4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x00, "SYS5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( rblaster )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "SYS0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "SYS1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "SYS2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};

static GFXDECODE_START( rblaster )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )
GFXDECODE_END

static MACHINE_START( rblaster )
{
}

static MACHINE_CONFIG_START( rblaster, deco_ld_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,8000000/2)
	MCFG_CPU_PROGRAM_MAP(rblaster_map)
//  MCFG_CPU_VBLANK_INT("screen",irq0_line_hold)
	MCFG_CPU_VBLANK_INT("screen",nmi_line_pulse)

	MCFG_CPU_ADD("audiocpu",M6502,8000000/2)
	MCFG_CPU_PROGRAM_MAP(rblaster_sound_map)
//  MCFG_CPU_VBLANK_INT("screen",irq0_line_hold) //test
	MCFG_CPU_PERIODIC_INT(sound_interrupt, 640)

	MCFG_LASERDISC_LDV1000_ADD("laserdisc") //Sony LDP-1000A, is it truly compatible with the Pioneer?
	MCFG_LASERDISC_OVERLAY_STATIC(256, 256, rblaster)

	/* video hardware */
	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")
	MCFG_GFXDECODE(rblaster)
	MCFG_PALETTE_LENGTH(512)
	MCFG_MACHINE_START(rblaster)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_MODIFY("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( begas, rblaster )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(begas_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cobra, rblaster )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(cobra_map)

	MCFG_DEVICE_REMOVE("audiocpu")
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( begas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "an05-3",   0x4000, 0x2000, CRC(c917a283) SHA1(b91f8cd18b8cc1189e4b69d6932d6f01d4ccfb81) )
	ROM_LOAD( "an04-3",   0x6000, 0x2000, CRC(935b2b0a) SHA1(e7c09960607569bd88e9af396aa70661f4352efb) )
	ROM_LOAD( "an03-3",   0x8000, 0x2000, CRC(79438d80) SHA1(e641336f23c6b84d84313ef3e94871ac9aa8b612) )
	ROM_LOAD( "an02-3",   0xa000, 0x2000, CRC(98ce4ca0) SHA1(e7db66b1f0f06b0a21e7450962ba70f460a24847) )
	ROM_LOAD( "an01",     0xc000, 0x2000, CRC(15f8921d) SHA1(32f945bee8f30e5896da38ac6184a11c0a8194bb) ) //ok?
	ROM_LOAD( "an00-3",   0xe000, 0x2000, CRC(124a3a36) SHA1(e2f7110196cb46fcda429c613388285b46ec1a9e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "an06",   0xe000, 0x2000, CRC(cbbcd730) SHA1(2f2e78fcf2eba71044bec60d27d8756d9b5af551) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "an0a",   0x0000, 0x2000, CRC(e429305d) SHA1(9a05ab7916235d028b6b05270703516581825660) )
	ROM_LOAD( "an0b",   0x4000, 0x2000, CRC(09e4b780) SHA1(0735420b8529017e507feecf8f74fecd80fbf7d5) )
	ROM_LOAD( "an0c",   0x8000, 0x2000, CRC(0c127207) SHA1(b8372b2fa20ffe5ac278f558c07fd761c86e514b) )

	ROM_LOAD( "an07",   0x2000, 0x2000, CRC(6b8ad735) SHA1(a703523202d40e409e2345a6626b9e29b7a59cd3) )
	ROM_LOAD( "an08",   0x6000, 0x2000, CRC(b5518391) SHA1(57f6407491cff075f76a8b459cc33e8b9a91e7de) )
	ROM_LOAD( "an09",   0xa000, 0x2000, CRC(b7375fd7) SHA1(93a59e99e375bdba77199a705b5e304ece221617) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "af-8.bpr",    0x00, 0x20, CRC(20006a72) SHA1(6d0e1c6de45079f9e128186478a7e0ed3fd471d0) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "lp1-1.pld",   0x0000, 44, CRC(cc84cb09) SHA1(61620ef30dfd6c81cc517f95ab6358c619ca3298) )
	ROM_LOAD( "lp1-2.pld",   0x0100, 44, CRC(60e16fc4) SHA1(1df735f393ed0fcf1272fceada9764084ff11e07) )
	ROM_LOAD( "lp1-3.pld",   0x0200, 52, CRC(976a7c57) SHA1(202c55a236799fb44a977c074c231ed54c71a872) )
	ROM_LOAD( "lp1-4.pld",   0x0300, 52, CRC(cc9a442f) SHA1(5d08873b204b15f888d02d79e049119e05e41b45) )
	ROM_LOAD( "lp1-5.pld",   0x0400, 44, CRC(2d9f3118) SHA1(02e40a99f131bb47562d5b90fdfb11ca8cd90da6) )
	ROM_LOAD( "lp2-1.pld",   0x0500, 44, CRC(dbb05313) SHA1(fc37db24f12c4f5170945c9ec9a333e4583c1712) )
	ROM_LOAD( "lp2-4.pld",   0x0600, 44, CRC(4c72736c) SHA1(6f7521284a5d960ff05c4361095c3e89a79f7475) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "begas", 0, NO_DUMP )
ROM_END

ROM_START( begas1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "an05",   0x4000, 0x2000, CRC(91a05549) SHA1(425668ee0dcf44bc011ee3649aa82df6ad3180eb) )
	ROM_LOAD( "an04",   0x6000, 0x2000, CRC(670966fe) SHA1(c179e3045ed0e46c5829fce5297ada475141e662) )
	ROM_LOAD( "an03",   0x8000, 0x2000, CRC(d2d85cdf) SHA1(da557ce5c3252297d2c073a0242e1989b0b7388b) )
	ROM_LOAD( "an02",   0xa000, 0x2000, CRC(84d13c20) SHA1(6474d90b84bca88c35cdb1d4c117ce431d6addf7) )
	ROM_LOAD( "an01",   0xc000, 0x2000, CRC(15f8921d) SHA1(32f945bee8f30e5896da38ac6184a11c0a8194bb) )
	ROM_LOAD( "an00",   0xe000, 0x2000, CRC(184297f3) SHA1(6813f076fde3eb583929506b2e65d9cd988b1b75) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "an06",   0xe000, 0x2000, CRC(cbbcd730) SHA1(2f2e78fcf2eba71044bec60d27d8756d9b5af551) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "an0a",   0x0000, 0x2000, CRC(e429305d) SHA1(9a05ab7916235d028b6b05270703516581825660) )
	ROM_LOAD( "an0b",   0x4000, 0x2000, CRC(09e4b780) SHA1(0735420b8529017e507feecf8f74fecd80fbf7d5) )
	ROM_LOAD( "an0c",   0x8000, 0x2000, CRC(0c127207) SHA1(b8372b2fa20ffe5ac278f558c07fd761c86e514b) )

	ROM_LOAD( "an07",   0x2000, 0x2000, CRC(6b8ad735) SHA1(a703523202d40e409e2345a6626b9e29b7a59cd3) )
	ROM_LOAD( "an08",   0x6000, 0x2000, CRC(b5518391) SHA1(57f6407491cff075f76a8b459cc33e8b9a91e7de) )
	ROM_LOAD( "an09",   0xa000, 0x2000, CRC(b7375fd7) SHA1(93a59e99e375bdba77199a705b5e304ece221617) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "af-8.bpr",    0x00, 0x20, CRC(20006a72) SHA1(6d0e1c6de45079f9e128186478a7e0ed3fd471d0) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "lp1-1.pld",   0x0000, 44, CRC(cc84cb09) SHA1(61620ef30dfd6c81cc517f95ab6358c619ca3298) )
	ROM_LOAD( "lp1-2.pld",   0x0100, 44, CRC(60e16fc4) SHA1(1df735f393ed0fcf1272fceada9764084ff11e07) )
	ROM_LOAD( "lp1-3.pld",   0x0200, 52, CRC(976a7c57) SHA1(202c55a236799fb44a977c074c231ed54c71a872) )
	ROM_LOAD( "lp1-4.pld",   0x0300, 52, CRC(cc9a442f) SHA1(5d08873b204b15f888d02d79e049119e05e41b45) )
	ROM_LOAD( "lp1-5.pld",   0x0400, 44, CRC(2d9f3118) SHA1(02e40a99f131bb47562d5b90fdfb11ca8cd90da6) )
	ROM_LOAD( "lp2-1.pld",   0x0500, 44, CRC(dbb05313) SHA1(fc37db24f12c4f5170945c9ec9a333e4583c1712) )
	ROM_LOAD( "lp2-4.pld",   0x0600, 44, CRC(4c72736c) SHA1(6f7521284a5d960ff05c4361095c3e89a79f7475) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "begas", 0, NO_DUMP )
ROM_END

ROM_START( rblaster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "01.bin",   0xc000, 0x2000, CRC(e4733c49) SHA1(357f46a80273f8a365d16cddf5e2caaeeacaf4ad) )
	ROM_LOAD( "00.bin",   0xe000, 0x2000, CRC(084d6ae2) SHA1(f49eb2d53bad5af88a12535ba628c9decce690ff) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "02.bin",   0xe000, 0x2000, CRC(6c20335d) SHA1(b28e80f112553af8e3fba9ebbfc10d1f56396ac1) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "03.bin",   0x0000, 0x2000, CRC(d1ff5ffb) SHA1(29df207e225e3b0477d5566d256198310d6ae526) )
	ROM_LOAD( "06.bin",   0x2000, 0x2000, CRC(d1ff5ffb) SHA1(29df207e225e3b0477d5566d256198310d6ae526) )
	ROM_LOAD( "04.bin",   0x4000, 0x2000, CRC(da2c84d9) SHA1(3452b0e2a45fa771e226c3a3668afbf3ceb0ec11) )
	ROM_LOAD( "07.bin",   0x6000, 0x2000, CRC(da2c84d9) SHA1(3452b0e2a45fa771e226c3a3668afbf3ceb0ec11) )
	ROM_LOAD( "05.bin",   0x8000, 0x2000, CRC(4608b516) SHA1(44af4be84a0b807ea0813ce86376a4b6fd927e5a) )
	ROM_LOAD( "08.bin",   0xa000, 0x2000, CRC(4608b516) SHA1(44af4be84a0b807ea0813ce86376a4b6fd927e5a) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "rblaster", 0, NO_DUMP )
ROM_END

ROM_START( cobra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "au03-2",   0x8000, 0x2000, CRC(8f0a8fba) SHA1(8e11d2bd665a5ca6b3bb11aa2b707458c1534327) )
	ROM_LOAD( "au02-2",   0xa000, 0x2000, CRC(7db11acf) SHA1(1eebae0741f5735bc8966f3c31a9c07dac2e3916) )
	ROM_LOAD( "au01-2",   0xc000, 0x2000, CRC(523dd8f6) SHA1(47bd4c9b2272e9a710e6e97f2505075df68101ed) )
	ROM_LOAD( "au00-2",   0xe000, 0x2000, CRC(6c0f1f16) SHA1(ed05d3eaa24e84b1dfb4e1eb5f69b23e4a1494ba) )
	ROM_COPY( "maincpu",  0x8000, 0x4000, 0x4000 )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "au0a",   0x0000, 0x2000, CRC(6aaedcf3) SHA1(52dc913eecf8a159784d500217cffd7a6d8eb45c) )
	ROM_LOAD( "au0b",   0x4000, 0x2000, CRC(92247877) SHA1(f9bb0c20212ab13caabfb5beb9b6afc807bc9555) )
	ROM_LOAD( "au0c",   0x8000, 0x2000, CRC(d00a2762) SHA1(84d4329b39b9fd30682b7efa5cb2744934c5ee5c) )

	ROM_LOAD( "au07",   0x2000, 0x2000, CRC(d4bf12a5) SHA1(e172f69ae02ac2670b70af0cfcf3887dd99c2761) )
	ROM_LOAD( "au08",   0x6000, 0x2000, CRC(63158274) SHA1(c728e8ba0a11ea67cf508877ad74a3aab9ef26fc) )
	ROM_LOAD( "au09",   0xa000, 0x2000, CRC(74e93394) SHA1(7a1470cf2008b1bef8d950939b758707297b3655) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "cobra", 0, SHA1(8390498294aca97a5d1769032e7b115d1a42f5d3) )
ROM_END

GAME( 1983, begas,  0,       begas,  cobra,  0, ROT0,    "Data East", "Bega's Battle (Revision 3)", GAME_NOT_WORKING )
GAME( 1983, begas1, begas,   rblaster,  cobra,  0, ROT0, "Data East", "Bega's Battle (Revision 1)", GAME_NOT_WORKING )
GAME( 1984, cobra,  0,       cobra,     cobra,  0, ROT0, "Data East", "Cobra Command (Data East LD)", GAME_NOT_WORKING )
// Thunder Storm (Cobra Command Japanese version)
GAME( 1985, rblaster,  0,    rblaster,  rblaster,  0, ROT0, "Data East", "Road Blaster (Data East LD)", GAME_NOT_WORKING )
