// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    Street Fighter

    driver by Olivier Galibert

    TODO:
    - is there a third coin input?

    - Is sfj confirmed to have a 68705?, sfua has an i8751 and the actual
      gamecode is closer to sfua than the other sets.  The protection
      appears to be the same on both.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"
#include "includes/sf.h"


/* The protection of the Japanese (and alt US) version */
/* I'd love to see someone dump the 68705 / i8751 roms */

void sf_state::write_dword( address_space &space, offs_t offset, UINT32 data )
{
	space.write_word(offset, data >> 16);
	space.write_word(offset + 2, data);
}

WRITE16_MEMBER(sf_state::protection_w)
{
	static const int maplist[4][10] = {
		{ 1, 0, 3, 2, 4, 5, 6, 7, 8, 9 },
		{ 4, 5, 6, 7, 1, 0, 3, 2, 8, 9 },
		{ 3, 2, 1, 0, 6, 7, 4, 5, 8, 9 },
		{ 6, 7, 4, 5, 3, 2, 1, 0, 8, 9 }
	};
	int map = maplist
		[space.read_byte(0xffc006)]
		[(space.read_byte(0xffc003) << 1) + (space.read_word(0xffc004) >> 8)];

	switch (space.read_byte(0xffc684))
	{
	case 1:
		{
			int base;

			base = 0x1b6e8 + 0x300e * map;

			write_dword(space, 0xffc01c, 0x16bfc + 0x270 * map);
			write_dword(space, 0xffc020, base + 0x80);
			write_dword(space, 0xffc024, base);
			write_dword(space, 0xffc028, base + 0x86);
			write_dword(space, 0xffc02c, base + 0x8e);
			write_dword(space, 0xffc030, base + 0x20e);
			write_dword(space, 0xffc034, base + 0x30e);
			write_dword(space, 0xffc038, base + 0x38e);
			write_dword(space, 0xffc03c, base + 0x40e);
			write_dword(space, 0xffc040, base + 0x80e);
			write_dword(space, 0xffc044, base + 0xc0e);
			write_dword(space, 0xffc048, base + 0x180e);
			write_dword(space, 0xffc04c, base + 0x240e);
			write_dword(space, 0xffc050, 0x19548 + 0x60 * map);
			write_dword(space, 0xffc054, 0x19578 + 0x60 * map);
			break;
		}
	case 2:
		{
			static const int delta1[10] = {
				0x1f80, 0x1c80, 0x2700, 0x2400, 0x2b80, 0x2e80, 0x3300, 0x3600, 0x3a80, 0x3d80
			};
			static const int delta2[10] = {
				0x2180, 0x1800, 0x3480, 0x2b00, 0x3e00, 0x4780, 0x5100, 0x5a80, 0x6400, 0x6d80
			};

			int d1 = delta1[map] + 0xc0;
			int d2 = delta2[map];

			space.write_word(0xffc680, d1);
			space.write_word(0xffc682, d2);
			space.write_word(0xffc00c, 0xc0);
			space.write_word(0xffc00e, 0);

			fg_scroll_w(space, 0, d1, 0xffff);
			bg_scroll_w(space, 0, d2, 0xffff);
			break;
		}
	case 4:
		{
			int pos = space.read_byte(0xffc010);
			pos = (pos + 1) & 3;
			space.write_byte(0xffc010, pos);
			if(!pos)
			{
				int d1 = space.read_word(0xffc682);
				int off = space.read_word(0xffc00e);
				if (off!=512)
				{
					off++;
					d1++;
				}
				else
				{
					off = 0;
					d1 -= 512;
				}
				space.write_word(0xffc682, d1);
				space.write_word(0xffc00e, off);
				bg_scroll_w(space, 0, d1, 0xffff);
			}
			break;
		}
	default:
		{
			logerror("Write protection at %06x (%04x)\n", space.device().safe_pc(), data & 0xffff);
			logerror("*** Unknown protection %d\n", space.read_byte(0xffc684));
			break;
		}
	}
}


WRITE8_MEMBER(sf_state::coin_w)
{
	machine().bookkeeping().coin_counter_w(0,  data & 0x01);
	machine().bookkeeping().coin_counter_w(1,  data & 0x02);
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x10);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x20);
	machine().bookkeeping().coin_lockout_w(2, ~data & 0x40); /* is there a third coin input? */
}

WRITE8_MEMBER(sf_state::soundcmd_w)
{
	soundlatch_byte_w(space, offset, data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(sf_state::sound2_bank_w)
{
	membank("bank1")->set_entry(data);
}

WRITE8_MEMBER(sf_state::msm1_5205_w)
{
	m_msm1->reset_w(BIT(data, 7));
	/* ?? bit 6?? */
	m_msm1->data_w(data);
	m_msm1->vclk_w(1);
	m_msm1->vclk_w(0);
}

WRITE8_MEMBER(sf_state::msm2_5205_w)
{
	m_msm2->reset_w(BIT(data, 7));
	/* ?? bit 6?? */
	m_msm2->data_w(data);
	m_msm2->vclk_w(1);
	m_msm2->vclk_w(0);
}

static ADDRESS_MAP_START( sfan_map, AS_PROGRAM, 16, sf_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x04ffff) AM_ROM
	AM_RANGE(0x800000, 0x800fff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xb00000, 0xb007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc00000, 0xc00001) AM_READ_PORT("IN0")
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("IN1")
	AM_RANGE(0xc00004, 0xc00005) AM_READ_PORT("PUNCH")
	AM_RANGE(0xc00006, 0xc00007) AM_READ_PORT("KICK")
	AM_RANGE(0xc00008, 0xc00009) AM_READ_PORT("DSW1")
	AM_RANGE(0xc0000a, 0xc0000b) AM_READ_PORT("DSW2")
	AM_RANGE(0xc0000c, 0xc0000d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc0000e, 0xc0000f) AM_READNOP
	AM_RANGE(0xc00010, 0xc00011) AM_WRITE8(coin_w, 0x00ff)
	AM_RANGE(0xc00014, 0xc00015) AM_WRITE(fg_scroll_w)
	AM_RANGE(0xc00018, 0xc00019) AM_WRITE(bg_scroll_w)
	AM_RANGE(0xc0001a, 0xc0001b) AM_WRITE(gfxctrl_w)
	AM_RANGE(0xc0001c, 0xc0001d) AM_WRITE8(soundcmd_w, 0x00ff)
//  AM_RANGE(0xc0001e, 0xc0001f) AM_WRITE(protection_w)
	AM_RANGE(0xff8000, 0xffdfff) AM_RAM
	AM_RANGE(0xffe000, 0xffffff) AM_RAM AM_SHARE("objectram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfus_map, AS_PROGRAM, 16, sf_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x04ffff) AM_ROM
	AM_RANGE(0x800000, 0x800fff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xb00000, 0xb007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc00000, 0xc00001) AM_READ_PORT("IN0")
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("IN1")
	AM_RANGE(0xc00004, 0xc00005) AM_READNOP
	AM_RANGE(0xc00006, 0xc00007) AM_READNOP
	AM_RANGE(0xc00008, 0xc00009) AM_READ_PORT("DSW1")
	AM_RANGE(0xc0000a, 0xc0000b) AM_READ_PORT("DSW2")
	AM_RANGE(0xc0000c, 0xc0000d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc0000e, 0xc0000f) AM_READNOP
	AM_RANGE(0xc00010, 0xc00011) AM_WRITE8(coin_w, 0x00ff)
	AM_RANGE(0xc00014, 0xc00015) AM_WRITE(fg_scroll_w)
	AM_RANGE(0xc00018, 0xc00019) AM_WRITE(bg_scroll_w)
	AM_RANGE(0xc0001a, 0xc0001b) AM_WRITE(gfxctrl_w)
	AM_RANGE(0xc0001c, 0xc0001d) AM_WRITE8(soundcmd_w, 0x00ff)
//  AM_RANGE(0xc0001e, 0xc0001f) AM_WRITE(protection_w)
	AM_RANGE(0xff8000, 0xffdfff) AM_RAM
	AM_RANGE(0xffe000, 0xffffff) AM_RAM AM_SHARE("objectram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfjp_map, AS_PROGRAM, 16, sf_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x04ffff) AM_ROM
	AM_RANGE(0x800000, 0x800fff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xb00000, 0xb007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc00000, 0xc00001) AM_READ_PORT("IN0")
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("IN1")
	AM_RANGE(0xc00004, 0xc00005) AM_READ_PORT("IN2")
	AM_RANGE(0xc00006, 0xc00007) AM_READNOP
	AM_RANGE(0xc00008, 0xc00009) AM_READ_PORT("DSW1")
	AM_RANGE(0xc0000a, 0xc0000b) AM_READ_PORT("DSW2")
	AM_RANGE(0xc0000c, 0xc0000d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc0000e, 0xc0000f) AM_READNOP
	AM_RANGE(0xc00010, 0xc00011) AM_WRITE8(coin_w, 0x00ff)
	AM_RANGE(0xc00014, 0xc00015) AM_WRITE(fg_scroll_w)
	AM_RANGE(0xc00018, 0xc00019) AM_WRITE(bg_scroll_w)
	AM_RANGE(0xc0001a, 0xc0001b) AM_WRITE(gfxctrl_w)
	AM_RANGE(0xc0001c, 0xc0001d) AM_WRITE8(soundcmd_w, 0x00ff)
	AM_RANGE(0xc0001e, 0xc0001f) AM_WRITE(protection_w)
	AM_RANGE(0xff8000, 0xffdfff) AM_RAM
	AM_RANGE(0xffe000, 0xffffff) AM_RAM AM_SHARE("objectram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, sf_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc800) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ymsnd", ym2151_device,read,write)
ADDRESS_MAP_END

/* Yes, _no_ ram */
static ADDRESS_MAP_START( sound2_map, AS_PROGRAM, 8, sf_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
	AM_RANGE(0x0000, 0xffff) AM_WRITENOP /* avoid cluttering up error.log */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound2_io_map, AS_IO, 8, sf_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(msm1_5205_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(msm2_5205_w)
	AM_RANGE(0x01, 0x01) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x02, 0x02) AM_WRITE(sound2_bank_w)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( common )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("DSW1.7E:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("DSW1.7E:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "DSW1.7E:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW1.7E:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("DSW2.13E:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Attract Music" )                 PORT_DIPLOCATION("DSW2.13E:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "DSW2.13E:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "DSW2.13E:4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Speed" )                         PORT_DIPLOCATION("DSW2.13E:5")
	PORT_DIPSETTING(      0x0000, "Slow" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("DSW2.13E:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Freeze" )                        PORT_DIPLOCATION("DSW2.13E:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "DSW2.13E:8" ) // Self-Test Mode

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, "Game Continuation" )             PORT_DIPLOCATION("DSW3.6E:1,2,3")
	PORT_DIPSETTING(      0x0007, "5th Stage Maximum" )
	PORT_DIPSETTING(      0x0006, "4th Stage Maximum" )
	PORT_DIPSETTING(      0x0005, "3rd Stage Maximum" )
	PORT_DIPSETTING(      0x0004, "2nd Stage Maximum" )
	PORT_DIPSETTING(      0x0003, "1st Stage Maximum" )
	PORT_DIPSETTING(      0x0002, DEF_STR( None ) )
	PORT_DIPNAME( 0x0018, 0x0018, "Round Time Count" )              PORT_DIPLOCATION("DSW3.6E:4,5")
	PORT_DIPSETTING(      0x0018, "100" )
	PORT_DIPSETTING(      0x0010, "150" )
	PORT_DIPSETTING(      0x0008, "200" )
	PORT_DIPSETTING(      0x0000, "250" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("DSW3.6E:6,7")
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x0380, 0x0380, "Buy-In Feature" )                PORT_DIPLOCATION("DSW3.6E:8,DSW4.11E:1,2")
	PORT_DIPSETTING(      0x0380, "5th Stage Maximum" )
	PORT_DIPSETTING(      0x0300, "4th Stage Maximum" )
	PORT_DIPSETTING(      0x0280, "3rd Stage Maximum" )
	PORT_DIPSETTING(      0x0200, "2nd Stage Maximum" )
	PORT_DIPSETTING(      0x0180, "1st Stage Maximum" )
	PORT_DIPSETTING(      0x0080, DEF_STR( None ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Number of Countries Selected" )  PORT_DIPLOCATION("DSW4.11E:3")
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "DSW4.11E:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "DSW4.11E:5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "DSW4.11E:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "DSW4.11E:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "DSW4.11E:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Freezes the game ? */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sfan )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "DSW2.13E:1" ) // Flip Screen not available

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0400, 0x0400, "Number of Countries Selected" )  PORT_DIPLOCATION("DSW4.11E:3")
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0000, "2" )

	// 4 pneumatic buttons. When their pressure starts decreasing, the game will latch
	// the highest measured value and respond with a low/mid/strong attack: approx.
	// 0x40 for low, 0xe0 for mid, 0xfe for strong.
	// NOTE: Timing is a matter of tenth-seconds. Tapping the button too lightly/quickly,
	// will not trigger an attack, same as on the original cab. Similarly, holding the
	// button for too long won't register either, analogous to the original cab by pushing
	// the button down slowly instead of hammering it.
	PORT_START("PUNCH")
	PORT_BIT( 0x00ff, 0x0000, IPT_PEDAL1 ) PORT_PLAYER(1) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("P1 Punch")
	PORT_BIT( 0xff00, 0x0000, IPT_PEDAL1 ) PORT_PLAYER(2) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("P2 Punch")

	PORT_START("KICK")
	PORT_BIT( 0x00ff, 0x0000, IPT_PEDAL2 ) PORT_PLAYER(1) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("P1 Kick")
	PORT_BIT( 0xff00, 0x0000, IPT_PEDAL2 ) PORT_PLAYER(2) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("P2 Kick")
INPUT_PORTS_END

static INPUT_PORTS_START( sfus )
	PORT_INCLUDE( common )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( sfjp )
	PORT_INCLUDE( common )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************

  Machine Configs

***************************************************************************/

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+8+0, 16*16+8+1, 16*16+8+2, 16*16+8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};


static GFXDECODE_START( sf )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, sprite_layout, 512, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, char_layout,   768, 16 )
GFXDECODE_END


void sf_state::machine_start()
{
	save_item(NAME(m_active));
	save_item(NAME(m_bgscroll));
	save_item(NAME(m_fgscroll));

	membank("bank1")->configure_entries(0, 256, memregion("audio2")->base() + 0x8000, 0x8000);
}

void sf_state::machine_reset()
{
	m_active = 0;
	m_bgscroll = 0;
	m_fgscroll = 0;
}

static MACHINE_CONFIG_START( sfan, sf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(sfan_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sf_state, irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)  /* ? xtal is 3.579545MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_CPU_ADD("audio2", Z80, XTAL_3_579545MHz)    /* ? xtal is 3.579545MHz */
	MCFG_CPU_PROGRAM_MAP(sound2_map)
	MCFG_CPU_IO_MAP(sound2_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(sf_state, irq0_line_hold, 8000) // ?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(sf_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sf)

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)

	MCFG_SOUND_ADD("msm1", MSM5205, 384000)
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)  /* 8KHz playback ?    */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_SOUND_ADD("msm2", MSM5205, 384000)
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)  /* 8KHz playback ?    */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sfus, sfan )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sfus_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sfjp, sfan )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sfjp_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sfp, sfan )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sf_state, irq6_line_hold)
MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( sf )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sfd-19", 0x00000, 0x10000, CRC(faaf6255) SHA1(f6d0186c6109780839576c141fc6b557c170c182) )
	ROM_LOAD16_BYTE("sfd-22", 0x00001, 0x10000, CRC(e1fe3519) SHA1(5c59343a8acaaa4f36636d8e28a4ca7854110dad) )
	ROM_LOAD16_BYTE("sfd-20", 0x20000, 0x10000, CRC(44b915bd) SHA1(85772fb89712f97bb0489a7e43f8b1a5037c8081) )
	ROM_LOAD16_BYTE("sfd-23", 0x20001, 0x10000, CRC(79c43ff8) SHA1(450fb75b6f36e08788d7a806122e4e1b0a87746c) )
	ROM_LOAD16_BYTE("sfd-21", 0x40000, 0x10000, CRC(e8db799b) SHA1(8443ba6a9b9ad29d5985d434658e685fd46d8f1e) )
	ROM_LOAD16_BYTE("sfd-24", 0x40001, 0x10000, CRC(466a3440) SHA1(689823763bfdbc12ac11ff176acfd22f352e2658) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.bin", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sfu-00",    0x00000, 0x20000, CRC(a7cce903) SHA1(76f521c9a00abd95a3491ab95e8eccd0fc7ea0e5) )
	ROM_LOAD( "sf-01.bin", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.bin", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.bin", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.bin", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.bin", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.bin", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.bin", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.bin", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.bin", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.bin", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.bin", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.bin", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.bin", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.bin", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.bin", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.bin", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.bin", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.bin", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.bin", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.bin", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.bin", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.bin", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.bin", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.bin", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.bin", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.bin", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.bin", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.bin", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "gfx5", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.bin", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.bin", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.bin", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.bin", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "mb7114h.12k",  0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) /* unknown */
	ROM_LOAD( "mb7114h.11h",  0x0100, 0x0100, CRC(c0e56586) SHA1(2abf93aef48af34f869b30f63c130513a97f86a3) ) /* unknown */
	ROM_LOAD( "mb7114h.12j",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END


ROM_START( sfua )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sfs19u.1a", 0x00000, 0x10000, CRC(c8e41c49) SHA1(01f023864a662fa451901b8689341b00a36973c1) )
	ROM_LOAD16_BYTE("sfs22u.1b", 0x00001, 0x10000, CRC(667e9309) SHA1(3d895874cf86470e4f2041d21e751fac3170b4c5) )
	ROM_LOAD16_BYTE("sfs20u.2a", 0x20000, 0x10000, CRC(303065bf) SHA1(152bb707cd71a8614f6d17cf9a145c8a8184ded7) )
	ROM_LOAD16_BYTE("sfs23u.2b", 0x20001, 0x10000, CRC(de6927a3) SHA1(862a62b71fbd2049f05968a238b97344d3b7404e) )
	ROM_LOAD16_BYTE("sfs21u.3a", 0x40000, 0x10000, CRC(004a418b) SHA1(1048afe2e0dbc22969d79a031394f3c8ab4c8901) )
	ROM_LOAD16_BYTE("sfs24u.3b", 0x40001, 0x10000, CRC(2b4545ff) SHA1(19bdae7947d13b861ace25b96e46f199ee9a6eb2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.bin", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sfu-00",    0x00000, 0x20000, CRC(a7cce903) SHA1(76f521c9a00abd95a3491ab95e8eccd0fc7ea0e5) )
	ROM_LOAD( "sf-01.bin", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x0800, "mcu", 0 ) /* i8751 MCU */
	ROM_LOAD( "i8751.bin",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.bin", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.bin", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.bin", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.bin", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.bin", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.bin", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.bin", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.bin", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.bin", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.bin", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.bin", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.bin", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.bin", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.bin", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.bin", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.bin", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.bin", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.bin", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.bin", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.bin", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.bin", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.bin", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.bin", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.bin", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.bin", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.bin", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.bin", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "gfx5", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.bin", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.bin", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.bin", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.bin", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.bin",    0x0000, 0x0100, CRC(864199ad) SHA1(b777df20b19fa7b7536120191df1875101e9d7af) ) /* unknown */
	ROM_LOAD( "sfb00.bin",    0x0100, 0x0100, CRC(bd3f8c5d) SHA1(c31ee9f466f05a21612f5ea29fb8c7c25dc9e011) ) /* unknown */
	ROM_LOAD( "mb7114h.12j",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END


ROM_START( sfj )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sf-19.bin", 0x00000, 0x10000, CRC(116027d7) SHA1(6bcb117ee415aff4d8ea962d4eff4088ca94c251) )
	ROM_LOAD16_BYTE("sf-22.bin", 0x00001, 0x10000, CRC(d3cbd09e) SHA1(7274c603100132102de09e10d2129cfeb6c06369) )
	ROM_LOAD16_BYTE("sf-20.bin", 0x20000, 0x10000, CRC(fe07e83f) SHA1(252dd592c31e594103ac1eabd734d10748655701) )
	ROM_LOAD16_BYTE("sf-23.bin", 0x20001, 0x10000, CRC(1e435d33) SHA1(2022a4368aa63cb036e77cb5739810030db469ff) )
	ROM_LOAD16_BYTE("sf-21.bin", 0x40000, 0x10000, CRC(e086bc4c) SHA1(782134978ff0a7133768d9cc8050bc3b5016580b) )
	ROM_LOAD16_BYTE("sf-24.bin", 0x40001, 0x10000, CRC(13a6696b) SHA1(c01f9b700928e427bc9914c61beeaa6bcbde4546) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.bin", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sf-00.bin", 0x00000, 0x20000, CRC(4b733845) SHA1(f7ff46e02f8ce6682d6e573588271bae2edfa90f) )
	ROM_LOAD( "sf-01.bin", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x0800, "mcu", 0 ) /* 68705 MCU */ // or should it be an i8751 like the above set? the protection is the same!
	ROM_LOAD( "68705.bin",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.bin", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.bin", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.bin", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.bin", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.bin", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.bin", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.bin", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.bin", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.bin", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.bin", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.bin", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.bin", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.bin", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.bin", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.bin", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.bin", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.bin", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.bin", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.bin", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.bin", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.bin", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.bin", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.bin", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.bin", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.bin", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.bin", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.bin", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "gfx5", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.bin", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.bin", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.bin", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.bin", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.bin",    0x0000, 0x0100, CRC(864199ad) SHA1(b777df20b19fa7b7536120191df1875101e9d7af) ) /* unknown */
	ROM_LOAD( "sfb00.bin",    0x0100, 0x0100, CRC(bd3f8c5d) SHA1(c31ee9f466f05a21612f5ea29fb8c7c25dc9e011) ) /* unknown */
	ROM_LOAD( "mb7114h.12j",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END


ROM_START( sfan )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sfe-19", 0x00000, 0x10000, CRC(8346c3ca) SHA1(404e26d210e453ef0f03b092d70c770106eed1d1) )
	ROM_LOAD16_BYTE("sfe-22", 0x00001, 0x10000, CRC(3a4bfaa8) SHA1(6a6fc8d967838eca7d2973de987bb350c25628d5) )
	ROM_LOAD16_BYTE("sfe-20", 0x20000, 0x10000, CRC(b40e67ee) SHA1(394987dc4c306351b1657d10528ecb665700c4db) )
	ROM_LOAD16_BYTE("sfe-23", 0x20001, 0x10000, CRC(477c3d5b) SHA1(6443334b3546550e5d97cf4057b279ec7b3cd758) )
	ROM_LOAD16_BYTE("sfe-21", 0x40000, 0x10000, CRC(2547192b) SHA1(aaf07c613a6c42ec1dc82ffa86d00044b4ea27fc) )
	ROM_LOAD16_BYTE("sfe-24", 0x40001, 0x10000, CRC(79680f4e) SHA1(df596fa5b49a336fe462c2be7b454e695f5382db) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.bin", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sfu-00",    0x00000, 0x20000, CRC(a7cce903) SHA1(76f521c9a00abd95a3491ab95e8eccd0fc7ea0e5) )
	ROM_LOAD( "sf-01.bin", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.bin", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.bin", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.bin", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.bin", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.bin", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.bin", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.bin", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.bin", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.bin", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.bin", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.bin", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.bin", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.bin", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.bin", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.bin", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.bin", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.bin", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.bin", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.bin", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.bin", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.bin", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.bin", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.bin", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.bin", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.bin", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.bin", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.bin", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "gfx5", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.bin", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.bin", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.bin", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.bin", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.mb7114h.12k", 0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) /* MB7114H */
	ROM_LOAD( "sfb10.mb7114h.11h", 0x0100, 0x0100, CRC(c0e56586) SHA1(2abf93aef48af34f869b30f63c130513a97f86a3) ) /* MB7114H */
	ROM_LOAD( "sfb04.mb7114h.12j", 0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* MB7114H */
	ROM_LOAD( "sfb00.mb7051.13h",  0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* MMI-7603 or MB7051 (equiv to 82s123 32x8 TS) */
ROM_END


ROM_START( sfp )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("prg8.2a", 0x00000, 0x20000, CRC(d48d06a3) SHA1(d899771c66c1e7a5caa11f67a1122adb6f0f4d28) )
	ROM_LOAD16_BYTE("prg0.2c", 0x00001, 0x20000, CRC(e8606c1a) SHA1(be94203cba733e337993e6f386ff5ce1e76d8913) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sound.9j", 0x0000, 0x8000, CRC(43cd32ae) SHA1(42e59becde5761eb5d5bc310d2bc690f6f16882a) )

	ROM_REGION( 0x10000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "voice.1g", 0x00000, 0x10000, CRC(3f23c180) SHA1(fb4e3bb835d94a733eacc0b1df9fe19fa1120997) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "bkchr.2k", 0x000000, 0x020000, CRC(e4d47aca) SHA1(597ed03e5c8328ec7209282247080c171eaedf86) ) /* Background b planes 0-1*/
	ROM_LOAD( "bkchr.1k", 0x020000, 0x020000, CRC(5a1cbc1b) SHA1(ad7bf117a7d1c0ef2aa47e133b0889092a009ae5) )
	ROM_LOAD( "bkchr.4k", 0x040000, 0x020000, CRC(c351bd48) SHA1(58131974d378a91f03f8c0bbd2ea384bd4fe501a) ) /* planes 2-3 */
	ROM_LOAD( "bkchr.3k", 0x060000, 0x020000, CRC(6bb2b050) SHA1(d36419dabdc0a90b76e295b746928d9e1e69674a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mchr.1d", 0x000000, 0x020000, CRC(ab06a60b) SHA1(44febaa2ac8f060ed297b69af1fd258164ff565d) ) /* Background m planes 0-1 */
	ROM_LOAD( "mchr.1e", 0x020000, 0x020000, CRC(d221387d) SHA1(012dc8c646a6a4b8bf905d859e3465b4bcaaed67) )
	ROM_LOAD( "mchr.1g", 0x040000, 0x020000, CRC(1e4c1712) SHA1(543b47a865d11dd91331c0236c5578dbe7549881) )
	ROM_LOAD( "mchr.1h", 0x060000, 0x020000, CRC(a381f529) SHA1(7e427894f8440c23c92ce5d1f118b7a1d70b0282) )
	ROM_LOAD( "mchr.2d", 0x080000, 0x020000, CRC(e52303c4) SHA1(1ae4979c53e589d9a5e7c0dbbf33b980d10274ac) ) /* planes 2-3 */
	ROM_LOAD( "mchr.2e", 0x0a0000, 0x020000, CRC(23b9a6a1) SHA1(bf7f67d97cfaa1f4c78f290c7c18e099566709c7) )
	ROM_LOAD( "mchr.2g", 0x0c0000, 0x020000, CRC(1283ac09) SHA1(229a507e0a1c46b451d8879e690e8557d21d588d) )
	ROM_LOAD( "mchr.2h", 0x0e0000, 0x020000, CRC(cc6bf05c) SHA1(4e83dd55c88d5b539ab1dcae5bfd16195bcd2565) )

	/* these graphic roms seem mismatched with this version of the prototype, they don't contain the graphics needed
	   for the bonus round, or have complete tile sets graphic set for 2 of the characters which are used by the prototype
	   (all of Joe is missing, many of Mike's poses are missing) If you use the original ROMs instead the graphics are
	   correct, so the prototype is clearly already referencing the final tile arrangement for them.  The glitches
	   therefore are not emulation bugs, if the PCB contained the same mismatched ROMs it would exhibit the same glitches. */
	ROM_REGION( 0x1c0000, "gfx3", ROMREGION_ERASE00 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "b1m.bin", 0x000000, 0x010000, CRC(64758232) SHA1(20d21677b791a7f96afed54b286ee92adb80456d) )
	ROM_LOAD( "b2m.bin", 0x010000, 0x010000, CRC(d958f5ad) SHA1(0e5c98a24814f5e1e6346dba4cfbd3a3a72ed724) )
	ROM_LOAD( "b1k.bin", 0x020000, 0x010000, CRC(e766f5fe) SHA1(ad48a543507a981d844f0e2d5cceb689775b9ad6) )
	ROM_LOAD( "b2k.bin", 0x030000, 0x010000, CRC(e71572d3) SHA1(752540bbabf56c883208b132e285b485d4b5b4ee) )
	ROM_LOAD( "b1h.bin", 0x040000, 0x010000, CRC(8494f38c) SHA1(8d99ae088bd5b479f10e69b0a960f07d10adc23b) )
	ROM_LOAD( "b2h.bin", 0x050000, 0x010000, CRC(1fc5f049) SHA1(bb6d5622247ec32ad044cde856cf67dddc3c732f) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "b3m.bin", 0x0e0000, 0x010000, CRC(d136802e) SHA1(84c2a6b2a8bad7e9249b6dce9cbf5301526aa6af) )
	ROM_LOAD( "b4m.bin", 0x0f0000, 0x010000, CRC(b4fa85d3) SHA1(c15e36000bf68a838eb34c3872e342acbb9c140a) )
	ROM_LOAD( "b3k.bin", 0x100000, 0x010000, CRC(40e11cc8) SHA1(ed469a8629080da88ce6faeb232633f94e2816c3) )
	ROM_LOAD( "b4k.bin", 0x110000, 0x010000, CRC(5ca9716e) SHA1(87620083aa6a7697f6faf742ac0e47115af3e0f3) )
	ROM_LOAD( "b3h.bin", 0x120000, 0x010000, CRC(8c3d9173) SHA1(08df92d962852f88b42e76dfaf6bb23a80d84657) )
	ROM_LOAD( "b4h.bin", 0x130000, 0x010000, CRC(a2df66f8) SHA1(9349704fdb7b0919813cb48d4deacdbbdebb2fee) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "vram.4d", 0x000000, 0x004000, CRC(bfadfb32) SHA1(8443ad9f02da5fb032017fc0c657b1bdc15e4f27) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "gfx5", 0 )    /* background tilemaps */
	ROM_LOAD( "bks1j10.5h", 0x000000, 0x010000, CRC(4934aacd) SHA1(15274ae8b26799e15c7a66ff89ffd386de1659d3) )
	ROM_LOAD( "bks1j18.3h", 0x010000, 0x010000, CRC(551ffc88) SHA1(4f9213f4e80033f910dd8aae44b2c6d9ba760d61) )
	ROM_LOAD( "ms1j10.3g",  0x020000, 0x010000, CRC(f92958b8) SHA1(da8fa64ea9ad27c737225681c49f7c57cc7afeed) )
	ROM_LOAD( "ms1j18.5g",  0x030000, 0x010000, CRC(89e35dc1) SHA1(368d0cce3bc39b3762d79df0c023242018fbbcb8) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.bin",    0x0000, 0x0100, CRC(864199ad) SHA1(b777df20b19fa7b7536120191df1875101e9d7af) ) /* unknown */
	ROM_LOAD( "sfb00.bin",    0x0100, 0x0100, CRC(bd3f8c5d) SHA1(c31ee9f466f05a21612f5ea29fb8c7c25dc9e011) ) /* unknown */
	ROM_LOAD( "mb7114h.12j",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END



GAME( 1987, sf,   0,  sfus, sfus, driver_device, 0, ROT0, "Capcom", "Street Fighter (US, set 1)", MACHINE_SUPPORTS_SAVE ) // Shows Capcom copyright
GAME( 1987, sfua, sf, sfjp, sfjp, driver_device, 0, ROT0, "Capcom", "Street Fighter (US, set 2) (protected)", MACHINE_SUPPORTS_SAVE ) // Shows Capcom USA copyright
GAME( 1987, sfj,  sf, sfjp, sfjp, driver_device, 0, ROT0, "Capcom", "Street Fighter (Japan) (protected)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sfan, sf, sfan, sfan, driver_device, 0, ROT0, "Capcom", "Street Fighter (World, pneumatic buttons)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sfp,  sf, sfp,  sfan, driver_device, 0, ROT0, "Capcom", "Street Fighter (prototype)", MACHINE_SUPPORTS_SAVE )
