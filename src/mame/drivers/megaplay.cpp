// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega MegaPlay

  changelog:

  01 Oct  2009 - Converted to use the HazeMD SMS code so that old code
                 can be removed, however this makes the text transparent,
                 which IIRC is incorrect

  22 Sept 2007 - Started updating this to use the new Megadrive code,
                 fixing issues with Mazin Wars + Grand Slam.
                 However I'm still not convinced that the handling of
                 the Megaplay side of things is correct at all, and
                 we're still hanging off the old SMS vdp code and
                 IO code.

*/

/*

About MegaPlay:

Megaplay games are specially designed Genesis games, produced for arcade use.

The code of these games has significant modifications when compared to the Genesis
releases and in many cases the games are cut-down versions of the games that were
released for the home system.  For example, Sonic has less zones, and no special
stages, thus making it impossible to get all the chaos emeralds.  Zones also have a
strict timer.

Coins buy you credits on Megaplay games, meaning if you lose all your lives the game is
over, like a regular Arcade game.

Like the Megatech the Megaplay boards have an additional Z80 and SMS VDP chip when compared
to the standard Genesis hardware.  In this case the additional hardware creates a layer
which is displayed as an overlay to the game screen.  This layer contains basic text
such as Insert Coin, and the Megaplay Logo / Instructions during the attract loop.

Communication between the various CPUs seems to be fairly complex and it is not fully
understood what is shared, where, and how.  One of the BIOS sets doesn't work, maybe for
this reason.

Only a handful of games were released for this system.

Bugs:
 Most of this is guesswork and should be verified on real hw.  Sometimes after inserting
 a coin and pressing start the 'press start' message remains on screen and no credit is
 deducted.  (timing?)


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#include "includes/megadriv.h"

#define MASTER_CLOCK        53693100

#define MP_ROM  1
#define MP_GAME 0


class mplay_state : public md_base_state
{
public:
	mplay_state(const machine_config &mconfig, device_type type, std::string tag)
	: md_base_state(mconfig, type, tag),
	m_ic3_ram(*this, "ic3_ram"),
	m_vdp1(*this, "vdp1"),
	m_bioscpu(*this, "mtbios")
	{ }

	DECLARE_READ16_MEMBER(extra_ram_r);
	DECLARE_WRITE16_MEMBER(extra_ram_w);
	DECLARE_READ8_MEMBER(bios_banksel_r);
	DECLARE_WRITE8_MEMBER(bios_banksel_w);
	DECLARE_READ8_MEMBER(bios_gamesel_r);
	DECLARE_WRITE8_MEMBER(bios_gamesel_w);
	DECLARE_WRITE16_MEMBER(mp_io_write);
	DECLARE_READ16_MEMBER(mp_io_read);
	DECLARE_READ8_MEMBER(bank_r);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(bios_6402_r);
	DECLARE_WRITE8_MEMBER(bios_6402_w);
	DECLARE_READ8_MEMBER(bios_6204_r);
	DECLARE_WRITE8_MEMBER(bios_width_w);
	DECLARE_READ8_MEMBER(bios_6404_r);
	DECLARE_WRITE8_MEMBER(bios_6404_w);
	DECLARE_READ8_MEMBER(bios_6600_r);
	DECLARE_WRITE8_MEMBER(bios_6600_w);
	DECLARE_WRITE8_MEMBER(game_w);
	DECLARE_READ8_MEMBER(vdp1_count_r);
	DECLARE_WRITE_LINE_MEMBER(bios_int_callback);

	DECLARE_DRIVER_INIT(megaplay);
	DECLARE_VIDEO_START(megplay);
	DECLARE_MACHINE_RESET(megaplay);
	UINT32 screen_update_megplay(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:

	UINT32 m_bios_mode;  // determines whether ROM banks or Game data is to read from 0x8000-0xffff

	UINT32 m_bios_bank; // ROM bank selection
	UINT16 m_game_banksel;  // Game bank selection
	UINT32 m_readpos;  // serial bank selection position (9-bit)
	UINT32 m_bios_bank_addr;

	UINT32 m_bios_width;  // determines the way the game info ROM is read
	UINT8 m_bios_ctrl[6];
	UINT8 m_bios_6600;
	UINT8 m_bios_6403;
	UINT8 m_bios_6404;

	std::unique_ptr<UINT16[]> m_ic36_ram;
	std::unique_ptr<UINT8[]> m_ic37_ram;

	required_shared_ptr<UINT8>           m_ic3_ram;
	optional_device<sega315_5124_device> m_vdp1;
	required_device<cpu_device>          m_bioscpu;
};


static INPUT_PORTS_START ( megaplay )
	PORT_INCLUDE( md_common )

	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Select") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 1") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 2") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 3") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 4") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_NAME("0x6400 bit 6") PORT_CODE(KEYCODE_U)
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* Caused 01081:
 *  PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
 *  PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_START2 )
 */

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x0f, "Coin slot 1" ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING( 0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x05, "2 coins/1 credit - 5 coins/3 credits - 6 coins/4 credits" )
	PORT_DIPSETTING( 0x04, "2 coins/1 credit - 4 coins/3 credits" )
	PORT_DIPSETTING( 0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING( 0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING( 0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING( 0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING( 0x03, "1 coin/1 credit - 5 coins/6 credits" )
	PORT_DIPSETTING( 0x02, "1 coin/1 credit - 4 coins/5 credits" )
	PORT_DIPSETTING( 0x01, "1 coin/1 credit - 2 coins/3 credits" )
	PORT_DIPSETTING( 0x00, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0xf0, 0xf0, "Coin slot 2" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING( 0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x50, "2 coins/1 credit - 5 coins/3 credits - 6 coins/4 credits" )
	PORT_DIPSETTING( 0x40, "2 coins/1 credit - 4 coins/3 credits" )
	PORT_DIPSETTING( 0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING( 0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING( 0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING( 0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING( 0x30, "1 coin/1 credit - 5 coins/6 credits" )
	PORT_DIPSETTING( 0x20, "1 coin/1 credit - 4 coins/5 credits" )
	PORT_DIPSETTING( 0x10, "1 coin/1 credit - 2 coins/3 credits" )
	PORT_DIPSETTING( 0x00, " 1 coin/1 credit" )

	PORT_START("DSW1")  /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Off )  )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING( 0x02, DEF_STR( Off )  )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING( 0x04, DEF_STR( Off )  )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING( 0x08, DEF_STR( Off )  )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_sonic )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x03, 0x01, "Initial Players" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x00, DEF_STR( Hardest ) )
	PORT_DIPSETTING( 0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING( 0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR( Normal ) )
	// Who knows...
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 5") PORT_CODE(KEYCODE_H)
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 6") PORT_CODE(KEYCODE_J)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 7") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_gaxe2 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, "Life" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING( 0x02, "1" )
	PORT_DIPSETTING( 0x00, "2" )
	PORT_DIPNAME( 0x04, 0x04, "Initial Players" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING( 0x00, "1" )
	PORT_DIPSETTING( 0x04, "2" )
	PORT_DIPNAME( 0x08, 0x00, "Timer" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING( 0x08, DEF_STR( Off )  )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	// Who knows...
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 5") PORT_CODE(KEYCODE_H)
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 6") PORT_CODE(KEYCODE_J)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 7") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END

#ifdef UNUSED_DEFINITION
static INPUT_PORTS_START ( mp_col3 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING( 0x01, DEF_STR( English ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x02, 0x02, "2P Mode Games" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING( 0x02, "1" )
	PORT_DIPSETTING( 0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x0c, "Speed / Difficulty" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x08, "Slow"  )
	PORT_DIPSETTING( 0x0c, "Middle"  )
	PORT_DIPSETTING( 0x04, "Fast"  )
	PORT_DIPSETTING( 0x00, "Max"  )
	// Who knows...
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 5") PORT_CODE(KEYCODE_H)
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 6") PORT_CODE(KEYCODE_J)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 7") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END
#endif

static INPUT_PORTS_START ( mp_twc )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x01, 0x01, "Time" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x00, "Short" )
	PORT_DIPNAME( 0x0e, 0x08, "Level" ) PORT_DIPLOCATION("SW3:2,3,4")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x02, "0" )
	PORT_DIPSETTING( 0x04, "5" )
	PORT_DIPSETTING( 0x06, "4" )
	PORT_DIPSETTING( 0x08, "3" )
	PORT_DIPSETTING( 0x0a, "2" )
	PORT_DIPSETTING( 0x0c, "1" )
	PORT_DIPSETTING( 0x0e, "0" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_sor2 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPNAME( 0xc, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x00, DEF_STR ( Hardest ) )
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_bio )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x00, "5" )
	PORT_DIPSETTING( 0x01, "4" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "3" )
	PORT_DIPNAME( 0xc, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x00, DEF_STR ( Hardest ) )
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_gslam )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x07, 0x04, DEF_STR ( Game_Time ) ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING( 0x00, "5:00" )
	PORT_DIPSETTING( 0x01, "4:30" )
	PORT_DIPSETTING( 0x02, "4:00" )
	PORT_DIPSETTING( 0x03, "3:30" )
	PORT_DIPSETTING( 0x04, "3:00" )
	PORT_DIPSETTING( 0x05, "2:30" )
	PORT_DIPSETTING( 0x06, "2:00" )
	PORT_DIPSETTING( 0x07, "1:30" )
	PORT_DIPNAME( 0x08, 0x08, "2P-Play Continue" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING( 0x00, "1 Credit" )
	PORT_DIPSETTING( 0x08, "2 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_mazin )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x03, 0x02, "Initial Player" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x00, "2" )
	PORT_DIPSETTING( 0x01, "1" )
	PORT_DIPSETTING( 0x02, "3" )
	PORT_DIPSETTING( 0x03, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x00, DEF_STR ( Normal ) )
	PORT_DIPNAME( 0x08, 0x08, "Title" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING( 0x08, "EUROPE" )
	PORT_DIPSETTING( 0x00, "U.S.A" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_soni2 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x03, 0x01, "Initial Players (Normal mode)" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPNAME( 0x0c, 0x0c, "Initial Players (Dual mode)" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPSETTING( 0x04, "2" )
	PORT_DIPSETTING( 0x08, "1" )
	PORT_DIPSETTING( 0x0c, "3" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_shnb3 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPNAME( 0xc, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x00, "Expert" )
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
INPUT_PORTS_END

/*MEGAPLAY specific*/

READ8_MEMBER(mplay_state::bios_banksel_r)
{
	return m_bios_bank;
}

WRITE8_MEMBER(mplay_state::bios_banksel_w)
{
/*  Multi-slot note:
    Bits 0 and 1 appear to determine the selected game slot.
    It should be possible to multiplex different game ROMs at
    0x000000-0x3fffff based on these bits.
*/
	m_bios_bank = data;
	m_bios_mode = MP_ROM;
//  logerror("BIOS: ROM bank %i selected [0x%02x]\n",bios_bank >> 6, data);
}

READ8_MEMBER(mplay_state::bios_gamesel_r)
{
	return m_bios_6403;
}

WRITE8_MEMBER(mplay_state::bios_gamesel_w)
{
	m_bios_6403 = data;

//  logerror("BIOS: 0x6403 write: 0x%02x\n",data);
	m_bios_mode = BIT(data, 4);
}

WRITE16_MEMBER(mplay_state::mp_io_write)
{
	if (offset == 0x03)
		m_megadrive_io_data_regs[2] = (data & m_megadrive_io_ctrl_regs[2]) | (m_megadrive_io_data_regs[2] & ~m_megadrive_io_ctrl_regs[2]);
	else
		megadriv_68k_io_write(space, offset & 0x1f, data, 0xffff);
}

READ16_MEMBER(mplay_state::mp_io_read)
{
	if (offset == 0x03)
		return m_megadrive_io_data_regs[2];
	else
		return megadriv_68k_io_read(space, offset & 0x1f, 0xffff);
}

READ8_MEMBER(mplay_state::bank_r)
{
	UINT8* bank = memregion("mtbios")->base();
	UINT32 fulladdress = m_bios_bank_addr + offset;

	if (fulladdress <= 0x3fffff) // ROM addresses
	{
		if (m_bios_mode == MP_ROM)
		{
			int sel = (m_bios_bank >> 6) & 0x03;
			return bank[sel * 0x8000 + offset];
		}
		else if (m_bios_width & 0x08)
		{
			if (offset >= 0x2000)
				return m_ic36_ram[offset - 0x2000];
			else
				return m_ic37_ram[(0x2000 * (m_bios_bank & 0x03)) + offset];
		}
		else
		{
			return memregion("maincpu")->base()[fulladdress ^ 1];
		}
	}
	else if (fulladdress >= 0xa10000 && fulladdress <= 0xa1001f) // IO access
	{
		return mp_io_read(space, (offset & 0x1f) / 2, 0xffff);
	}
	else
	{
		printf("bank_r fulladdress %08x\n", fulladdress);
		return 0x00;
	}

}

WRITE8_MEMBER(mplay_state::bank_w)
{
	UINT32 fulladdress = m_bios_bank_addr + offset;

	if (fulladdress <= 0x3fffff && m_bios_width & 0x08) // ROM / Megaplay Custom Addresses
	{
		if (offset >= 0x2000)
			m_ic36_ram[offset - 0x2000] = data;
		else
			m_ic37_ram[(0x2000 * (m_bios_bank & 0x03)) + offset] = data;
	}
	else if (fulladdress >= 0xa10000 && fulladdress <=0xa1001f) // IO Access
	{
		mp_io_write(space, (offset & 0x1f) / 2, data, 0xffff);
	}
	else
	{
		printf("bank_w fulladdress %08x\n", fulladdress);
	}
}


/* Megaplay BIOS handles regs[2] at start in a different way compared to megadrive */
/* other io data/ctrl regs are dealt with exactly like in the console              */

READ8_MEMBER(mplay_state::bios_6402_r)
{
	return m_megadrive_io_data_regs[2];// & 0xfe;
}

WRITE8_MEMBER(mplay_state::bios_6402_w)
{
	m_megadrive_io_data_regs[2] = (m_megadrive_io_data_regs[2] & 0x07) | ((data & 0x70) >> 1);
//  logerror("BIOS: 0x6402 write: 0x%02x\n", data);
}

READ8_MEMBER(mplay_state::bios_6204_r)
{
	return m_megadrive_io_data_regs[2];
//  return (m_bios_width & 0xf8) + (m_bios_6204 & 0x07);
}

WRITE8_MEMBER(mplay_state::bios_width_w)
{
	m_bios_width = data;
	m_megadrive_io_data_regs[2] = (m_megadrive_io_data_regs[2] & 0x07) | ((data & 0xf8));
//  logerror("BIOS: 0x6204 - Width write: %02x\n", data);
}

READ8_MEMBER(mplay_state::bios_6404_r)
{
//  logerror("BIOS: 0x6404 read: returned 0x%02x\n",bios_6404 | (bios_6403 & 0x10) >> 4);
	return (m_bios_6404 & 0xfe) | ((m_bios_6403 & 0x10) >> 4);
//  return m_bios_6404 | (m_bios_6403 & 0x10) >> 4;
}

WRITE8_MEMBER(mplay_state::bios_6404_w)
{
	if(((m_bios_6404 & 0x0c) == 0x00) && ((data & 0x0c) == 0x0c))
		m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	m_bios_6404 = data;

//  logerror("BIOS: 0x6404 write: 0x%02x\n", data);
}

READ8_MEMBER(mplay_state::bios_6600_r)
{
/*  Multi-slot note:
    0x6600 appears to be used to check for extra slots being used.
    Enter the following line in place of the return statement in this
    function to make the BIOS check all 4 slots (3 and 4 will be "not used")
        return (m_bios_6600 & 0xfe) | (m_bios_bank & 0x01);
*/
	return m_bios_6600;// & 0xfe;
}

WRITE8_MEMBER(mplay_state::bios_6600_w)
{
	m_bios_6600 = data;
//  logerror("BIOS: 0x6600 write: 0x%02x\n",data);
}

WRITE8_MEMBER(mplay_state::game_w)
{
	if (m_readpos == 1)
		m_game_banksel = 0;
	m_game_banksel |= (1 << (m_readpos - 1)) * (data & 0x01);

	m_readpos++;

	if (m_readpos > 9)
	{
		m_bios_mode = MP_GAME;
		m_readpos = 1;
//      popmessage("Game bank selected: 0x%03x", m_game_banksel);
		logerror("BIOS [0x%04x]: 68K address space bank selected: 0x%03x\n", space.device().safe_pcbase(), m_game_banksel);
	}

	m_bios_bank_addr = ((m_bios_bank_addr >> 1) | (data << 23)) & 0xff8000;
}

static ADDRESS_MAP_START( megaplay_bios_map, AS_PROGRAM, 8, mplay_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_WRITE(game_w)
	AM_RANGE(0x6200, 0x6200) AM_READ_PORT("DSW0")
	AM_RANGE(0x6201, 0x6201) AM_READ_PORT("DSW1")
	AM_RANGE(0x6203, 0x6203) AM_READWRITE(bios_banksel_r, bios_banksel_w)
	AM_RANGE(0x6204, 0x6204) AM_READWRITE(bios_6204_r, bios_width_w)
	AM_RANGE(0x6400, 0x6400) AM_READ_PORT("TEST")
	AM_RANGE(0x6401, 0x6401) AM_READ_PORT("COIN")
	AM_RANGE(0x6402, 0x6402) AM_READWRITE(bios_6402_r, bios_6402_w)
	AM_RANGE(0x6403, 0x6403) AM_READWRITE(bios_gamesel_r, bios_gamesel_w)
	AM_RANGE(0x6404, 0x6404) AM_READWRITE(bios_6404_r, bios_6404_w)
	AM_RANGE(0x6600, 0x6600) AM_READWRITE(bios_6600_r, bios_6600_w)
	AM_RANGE(0x6001, 0x67ff) AM_WRITEONLY
	AM_RANGE(0x6800, 0x77ff) AM_RAM AM_SHARE("ic3_ram")
	AM_RANGE(0x8000, 0xffff) AM_READWRITE(bank_r, bank_w)
ADDRESS_MAP_END



READ8_MEMBER(mplay_state::vdp1_count_r)
{
	address_space &prg = m_bioscpu->space(AS_PROGRAM);
	if (offset & 0x01)
		return m_vdp1->hcount_read(prg, offset);
	else
		return m_vdp1->vcount_read(prg, offset);
}

static ADDRESS_MAP_START( megaplay_bios_io_map, AS_IO, 8, mplay_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7f, 0x7f) AM_DEVWRITE("sn2", sn76496_device, write)

	AM_RANGE(0x40, 0x41) AM_MIRROR(0x3e) AM_READ(vdp1_count_r)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3e) AM_DEVREADWRITE("vdp1", sega315_5124_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x3e) AM_DEVREADWRITE("vdp1", sega315_5124_device, register_read, register_write)
ADDRESS_MAP_END


UINT32 mplay_state::screen_update_megplay(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//printf("megplay vu\n");
	screen_update_megadriv(screen, bitmap, cliprect);
	//m_vdp1->screen_update(screen, bitmap, cliprect);

	// i'm not sure if the overlay (256 pixels wide) is meant to be stretched over the 320 resolution genesis output, or centered.
	// if it's meant to be stretched we'll have to multiply the entire outut x4 for the Genesis VDP and x5 for the SMS VDP to get a common 1280 pixel wide image

	// overlay, only drawn for pixels != 0
	for (int y = 0; y < 224; y++)
	{
		UINT32* lineptr = &bitmap.pix32(y);
		UINT32* srcptr =  &m_vdp1->get_bitmap().pix32(y + SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT);

		for (int x = 0; x < SEGA315_5124_WIDTH; x++)
		{
			UINT32 src = srcptr[x] & 0xffffff;

			if (src)
			{
				if (x>=16)
					lineptr[x-16] = src;

			}
		}
	}

	return 0;
}

MACHINE_RESET_MEMBER(mplay_state,megaplay)
{
	m_bios_mode = MP_ROM;
	m_bios_bank_addr = 0;
	m_readpos = 1;
	MACHINE_RESET_CALL_MEMBER(megadriv);
}

WRITE_LINE_MEMBER( mplay_state::bios_int_callback )
{
	m_bioscpu->set_input_line(0, state);
}

static MACHINE_CONFIG_START( megaplay, mplay_state )
	/* basic machine hardware */
	MCFG_FRAGMENT_ADD(md_ntsc)

	/* The Megaplay has an extra BIOS cpu which drives an SMS VDP
	   which includes an SN76496 for sound */
	MCFG_CPU_ADD("mtbios", Z80, MASTER_CLOCK / 15) /* ?? */
	MCFG_CPU_PROGRAM_MAP(megaplay_bios_map)
	MCFG_CPU_IO_MAP(megaplay_bios_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_SOUND_ADD("sn2", SN76496, MASTER_CLOCK/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */

	/* New update functions to handle the extra layer */
	MCFG_SCREEN_MODIFY("megadriv")
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_738635MHz/2, \
		SEGA315_5124_WIDTH , SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_UPDATE_DRIVER(mplay_state, screen_update_megplay)

	// Megaplay has an additional SMS VDP as an overlay
	MCFG_DEVICE_ADD("vdp1", SEGA315_5246, 0)
	MCFG_SEGA315_5246_SET_SCREEN("megadriv")
	MCFG_SEGA315_5246_IS_PAL(false)
	MCFG_SEGA315_5246_INT_CB(WRITELINE(mplay_state, bios_int_callback))
MACHINE_CONFIG_END


/* MegaPlay Games - Modified Genesis games */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define MEGAPLAY_BIOS \
	ROM_SYSTEM_BIOS( 0, "ver1",       "Megaplay Bios (Ver. 1)" ) \
	ROM_LOAD_BIOS( 0, "ep15294.ic2",   0x000000, 0x20000, CRC(aa8dc2d8) SHA1(96771ad7b79dc9c83a1594243250d65052d23176) ) \
	ROM_SYSTEM_BIOS( 1, "ver2",       "Megaplay Bios (Ver. 2)" ) /* this one doesn't boot .. dump was verified with another working pcb */ \
	ROM_LOAD_BIOS( 1, "epr-a15294.ic2",0x000000, 0x20000, CRC(f97c68aa) SHA1(bcabc879950bca1ced11c550a484e697ec5706b2) )
ROM_START( megaplay )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

/* The system appears to access the instruction rom at
    0x300000 in the 68k space (rom window from z80 side)

   This probably means the maximum 68k rom size is 0x2fffff for MegaPlay
*/

ROM_START( mp_sonic ) /* Sonic */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ep15177.ic2", 0x000000, 0x040000, CRC(a389b03b) SHA1(8e9e1cf3dd65ddf08757f5a1ce472130c902ea2c) )
	ROM_LOAD16_BYTE( "ep15176.ic1", 0x000001, 0x040000, CRC(d180cc21) SHA1(62805cfaaa80c1da6146dd89fc2b49d819fd4f22) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "ep15175-01.ic3", 0x000000, 0x08000, CRC(99246889) SHA1(184aa3b7fdedcf578c5e34edb7ed44f57f832258) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

/* this cart looks to be a conversion from something else.. sega rom numbers were missing
   but the code looks like it's probably real */
/* pcb  171-5834 */
ROM_START( mp_col3 ) /* Columns 3 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.ic2", 0x000000, 0x040000, CRC(a1602235) SHA1(38751b585849c8966acc3f508714937fe29dcf5c) )
	ROM_LOAD16_BYTE( "2.ic1", 0x000001, 0x040000, CRC(999b2fe6) SHA1(ad967a28e4eebd7b01273e4e04c35a0198ef834a) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "1.ic3", 0x000000, 0x08000,  CRC(dac9bf91) SHA1(0117972a7181f8aaf942a259cc8764b821031253) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_gaxe2 ) /* Golden Axe 2, revision B */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ep15179b.ic2", 0x000000, 0x040000, CRC(00d97b84) SHA1(914bbf566ddf940aab67b92af237d251650ddadf) )
	ROM_LOAD16_BYTE( "ep15178b.ic1", 0x000001, 0x040000, CRC(2ea576db) SHA1(6d96b948243533de1f488b1f80e0d5431a4f1f53) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "ep15175-02b.ic3", 0x000000, 0x08000, CRC(3039b653) SHA1(b19874c74d0fc0cca1169f62e5e74f0e8ca83679) ) // 15175-02b.ic3

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_gaxe2a ) /* Golden Axe 2 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15179.ic2", 0x000000, 0x040000, CRC(d35f1a35) SHA1(3105cd3b55f65337863703db04527fe298fc04e0) )
	ROM_LOAD16_BYTE( "epr-15178.ic1", 0x000001, 0x040000, CRC(2c6b6b76) SHA1(25577f49ecad451c217da9cacbd78ffca9dca24e) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-02.ic3", 0x000000, 0x08000, CRC(cfc87f91) SHA1(110609094aa6d848bec613faa0558db7ad272b77) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_gslam ) /* Grand Slam */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15181.ic2", 0x000000, 0x040000, CRC(642437c1) SHA1(cbf88e196c04b6d886bf9642b69bf165045510fe) )
	ROM_LOAD16_BYTE( "epr-15180.ic1", 0x000001, 0x040000, CRC(73bb48f1) SHA1(981b64f834d5618599352f5fad683bf232390ba3) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-03.ic3", 0x000000, 0x08000, CRC(70ea1aec) SHA1(0d9d82a1f8aa51d02707f7b343e7cfb6591efccd) ) // 15175-02b.ic3

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END


ROM_START( mp_twc ) /* Tecmo World Cup */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ep15183.ic2", 0x000000, 0x040000, CRC(8b79b861) SHA1(c72af72840513b82f2562409eccdf13b031bf3c0) )
	ROM_LOAD16_BYTE( "ep15182.ic1", 0x000001, 0x040000, CRC(eb8325c3) SHA1(bb21ac926c353e14184dd476222bc6a8714606e5) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "ep15175-04.ic3", 0x000000, 0x08000, CRC(faf7c030) SHA1(16ef405335b4d3ecb0b7d97b088dafc4278d4726) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_sor2 ) /* Streets of Rage 2 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-15425.ic1", 0x000000, 0x200000, CRC(cd6376af) SHA1(57ec210975e40505649f152b60ef54f99da31f0e) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-05.ic2", 0x000000, 0x08000, CRC(1df5347c) SHA1(faced2e875e1914392f61577b5256d006eebeef9) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_bio ) /* Bio Hazard Battle */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-15699-f.ic1", 0x000000, 0x100000, CRC(4b193229) SHA1(f8629171ae9b4792f142f6957547d886e5cc6817) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-06.ic2", 0x000000, 0x08000, CRC(1ef64e41) SHA1(13984b714b014ea41963b70de74a5358ed223bc5) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_soni2 ) /* Sonic The Hedgehog 2 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16011.ic1", 0x000000, 0x100000, CRC(3d7bf98a) SHA1(dce0e4e8f2573e0ffe851edaa235e4ed9e61ee2d) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-07.ic1", 0x000000, 0x08000, CRC(bb5f67f0) SHA1(33b7a5d14015a5fcf41976a8f648f8f48ce9bb03) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_mazin ) /* Mazin Wars */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16460.ic1", 0x000000, 0x100000, CRC(e9635a83) SHA1(ab3afa11656f0ae3a50c957dce012fb15d3992e0) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-11.ic2", 0x000000, 0x08000, CRC(bb651120) SHA1(81cb736f2732373e260dde162249c1d29a3489c3) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END

ROM_START( mp_shnb3 ) /* Shinobi 3 */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16197.ic1", 0x000000, 0x100000, CRC(48162361) SHA1(77d544509339b5ddf6d19941377e81d29e9e21dc) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-09.ic2", 0x000000, 0x08000, CRC(6254e45a) SHA1(8667922a6eade03c964ce224f7fa39ba871c60a4) )

	ROM_REGION( 0x20000, "mtbios", 0 ) /* Bios */
	MEGAPLAY_BIOS
ROM_END


READ16_MEMBER(mplay_state::extra_ram_r )
{
	return m_ic36_ram[(offset << 1) ^ 1] | (m_ic36_ram[(offset << 1)] << 8);
}

WRITE16_MEMBER(mplay_state::extra_ram_w )
{
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		m_ic36_ram[(offset << 1)] = (data & 0xff00) >> 8;
	}
	else if (!ACCESSING_BITS_8_15)
	{
		m_ic36_ram[(offset << 1) ^ 1] = (data & 0x00ff);
	}
	else // for WORD access only the MSB is used, LSB is ignored
	{
		m_ic36_ram[(offset << 1)] = (data & 0xff00) >> 8;
	}
}


DRIVER_INIT_MEMBER(mplay_state,megaplay)
{
	// copy game instruction rom to main map. maybe this should just be accessed
	// through a handler instead?
	UINT8 *instruction_rom = memregion("user1")->base();
	UINT8 *game_rom = memregion("maincpu")->base();

	for (int offs = 0; offs < 0x8000; offs++)
	{
		UINT8 dat = instruction_rom[offs];

		game_rom[0x300000 + offs * 2] = dat;
		game_rom[0x300001 + offs * 2] = dat;
	}

	// to support the old code
	m_ic36_ram = std::make_unique<UINT16[]>(0x10000 / 2);
	m_ic37_ram = std::make_unique<UINT8[]>(0x10000);

	DRIVER_INIT_CALL(megadrij);
	m_megadrive_io_read_data_port_ptr = read8_delegate(FUNC(md_base_state::megadrive_io_read_data_port_3button),this);
	m_megadrive_io_write_data_port_ptr = write16_delegate(FUNC(md_base_state::megadrive_io_write_data_port_3button),this);

	// for now ...
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa10000, 0xa1001f, read16_delegate(FUNC(mplay_state::mp_io_read),this), write16_delegate(FUNC(mplay_state::mp_io_write),this));

	// megaplay has ram shared with the bios cpu here
	m_z80snd->space(AS_PROGRAM).install_ram(0x2000, 0x3fff, &m_ic36_ram[0]);

	// instead of a RAM mirror the 68k sees the extra ram of the 2nd z80 too
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa02000, 0xa03fff, read16_delegate(FUNC(mplay_state::extra_ram_r),this), write16_delegate(FUNC(mplay_state::extra_ram_w),this));
}

/*
Sega Mega Play Cartridges
-------------------------

These are cart-based games for use with Sega Mega Play hardware. There are 2 known types of carts. Both carts
are very simple, almost exactly the same as Mega Tech carts. They contain just 2 or 3 ROMs.
PCB 171-6215A has locations for 2 ROMs and is dated 1991.
PCB 171-5834 has locations for 3 ROMs and is dated 1989.

                                                                       |------------------------------- ROMs ------------------------------|
                                                                       |                                                                   |
Game                 PCB #       Sticker on PCB    Sticker on cart      IC1                     IC2                    IC3
-------------------------------------------------------------------------------------------------------------------------------------------
Sonic The Hedgehog                                    -    -01
Golden Axe 2                                          -    -02
Grand Slam           171-5834    837-9165-03       610-0297-03          EPR-15180   (27C020)    EPR-15181    (27C020)  EPR-15175-03 (27256)
Tecmo World Cup                                       -    -04
Columns 3            171-5834                      610-0297-04*         2           (27C020)    3            (27C020)  1            (27256)
Streets Of Rage II   171-6215A   837-9165-05       610-0297-05          MPR-15425   (8316200A)  EPR-15175-05 (27256)   n/a
Bio-Hazard Battle    171-6215A   837-9165-06       610-0298-06          MPR-15699-F (838200)    EPR-15175-06 (27256)   n/a
Sonic The Hedgehog 2 171-6215A   837-9165-07       610-0297-07          MPR-16011   (838200)    EPR-15175-07 (27256)   n/a
Shinobi III          171-6215A   837-9165-09       610-0297-09          MPR-16197   (838200)    EPR-15175-09 (27256)   n/a
Mazin Wars           171-6215A   837-9165-11       610-0297-11          MPR-16460   (838200)    EPR-15175-11 (27256)   n/a

* This is the code for Tecmo World Cup, as the ROMs in the Columns 3 cart
didn't have original Sega part numbers it's probably a converted TWC cart
*/

/* -- */ GAME( 1993, megaplay, 0,        megaplay, megaplay, mplay_state, megaplay, ROT0, "Sega",                  "Mega Play BIOS", MACHINE_IS_BIOS_ROOT )
/* 01 */ GAME( 1993, mp_sonic, megaplay, megaplay, mp_sonic, mplay_state, megaplay, ROT0, "Sega",                  "Sonic The Hedgehog (Mega Play)" , 0 )
/* 02 */ GAME( 1993, mp_gaxe2, megaplay, megaplay, mp_gaxe2, mplay_state, megaplay, ROT0, "Sega",                  "Golden Axe II (Mega Play) (Rev B)" , 0 )
/* 02 */ GAME( 1993, mp_gaxe2a,mp_gaxe2, megaplay, mp_gaxe2, mplay_state, megaplay, ROT0, "Sega",                  "Golden Axe II (Mega Play)" , 0 )
/* 03 */ GAME( 1993, mp_gslam, megaplay, megaplay, mp_gslam, mplay_state, megaplay, ROT0, "Sega",                  "Grand Slam (Mega Play)",0  )
/* 04 */ GAME( 1993, mp_twc,   megaplay, megaplay, mp_twc, mplay_state,   megaplay, ROT0, "Sega",                  "Tecmo World Cup (Mega Play)" , 0 )
/* 05 */ GAME( 1993, mp_sor2,  megaplay, megaplay, mp_sor2, mplay_state,  megaplay, ROT0, "Sega",                  "Streets of Rage II (Mega Play)" , 0 )
/* 06 */ GAME( 1993, mp_bio,   megaplay, megaplay, mp_bio, mplay_state,   megaplay, ROT0, "Sega",                  "Bio-hazard Battle (Mega Play)" , 0 )
/* 07 */ GAME( 1993, mp_soni2, megaplay, megaplay, mp_soni2, mplay_state, megaplay, ROT0, "Sega",                  "Sonic The Hedgehog 2 (Mega Play)" , 0 )
/* 08 */
/* 09 */ GAME( 1993, mp_shnb3, megaplay, megaplay, mp_shnb3, mplay_state, megaplay, ROT0, "Sega",                  "Shinobi III (Mega Play)" , 0 )
/* 10 */
/* 11 */ GAME( 1993, mp_mazin, megaplay, megaplay, mp_mazin, mplay_state, megaplay, ROT0, "Sega",                  "Mazin Wars / Mazin Saga (Mega Play)",0  )

/* ?? */ GAME( 1993, mp_col3,  megaplay, megaplay, megaplay, mplay_state, megaplay, ROT0, "Sega",                  "Columns III (Mega Play)" , 0 )


/* Also confirmed to exist:
Gunstar Heroes

system16.com lists 'Streets of Rage' but this seems unlikely, there are no gaps in
the numbering prior to 'Streets of Rage 2'

*/
