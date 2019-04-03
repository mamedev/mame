// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

Driver is marked as NOT WORKING because interaction between BIOS and 68k side is
not fully understood.  The BIOS often doesn't register that a game has been started
and leaves the 'PRESS P1 OR P2 START' message onscreen during gameplay as a result.
If this happens, the games usually then crash when you run out of lives as they end
up in an unknown state.



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

The BIOS reads the Start inputs and (once enough coins have been inserted) passes them to
the 68k rather than using the regular Genesis PAD hookups.

Communication between the various CPUs seems to be fairly complex and it is not fully
understood what is shared, where, and how.  One of the BIOS sets doesn't work, maybe for
this reason.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/cxd1095.h"

#include "includes/megadriv.h"

#define MASTER_CLOCK        53693100

#define MP_ROM  1
#define MP_GAME 0


class mplay_state : public md_base_state
{
public:
	mplay_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_base_state(mconfig, type, tag),
		m_ic3_ram(*this, "ic3_ram"),
		m_vdp1(*this, "vdp1"),
		m_bioscpu(*this, "mtbios")
	{ }

	void megaplay(machine_config &config);

	void init_megaplay();

	DECLARE_READ_LINE_MEMBER(start1_r);
	DECLARE_READ_LINE_MEMBER(start2_r);

private:

	DECLARE_READ16_MEMBER(extra_ram_r);
	DECLARE_WRITE16_MEMBER(extra_ram_w);
	DECLARE_WRITE8_MEMBER(bios_banksel_w);
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

	DECLARE_VIDEO_START(megplay);
	DECLARE_MACHINE_RESET(megaplay);
	uint32_t screen_update_megplay(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void megaplay_bios_io_map(address_map &map);
	void megaplay_bios_map(address_map &map);

	uint32_t m_bios_mode;  // determines whether ROM banks or Game data is to read from 0x8000-0xffff

	uint32_t m_bios_bank; // ROM bank selection
	uint16_t m_game_banksel;  // Game bank selection
	uint32_t m_readpos;  // serial bank selection position (9-bit)
	uint32_t m_bios_bank_addr;

	uint32_t m_bios_width;  // determines the way the game info ROM is read
	uint8_t m_bios_ctrl[6];
	uint8_t m_bios_6600;
	uint8_t m_bios_6403;
	uint8_t m_bios_6404;

	std::unique_ptr<uint16_t[]> m_ic36_ram;
	std::unique_ptr<uint8_t[]> m_ic37_ram;

	required_shared_ptr<uint8_t>           m_ic3_ram;
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("PAD1") // P1 Start input processed through BIOS
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, mplay_state, start1_r)

	PORT_MODIFY("PAD2") // P2 Start input processed through BIOS
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, mplay_state, start2_r)

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

static INPUT_PORTS_START ( mp_gunhe )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") /* DSW C  (per game settings) */
	PORT_DIPNAME( 0x03, 0x01, "Initial Players" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x00, "Expert" )
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
INPUT_PORTS_END

/*MEGAPLAY specific*/

READ_LINE_MEMBER(mplay_state::start1_r)
{
	return BIT(m_bios_bank, 4);
}

READ_LINE_MEMBER(mplay_state::start2_r)
{
	return BIT(m_bios_bank, 5);
}

WRITE8_MEMBER(mplay_state::bios_banksel_w)
{
/*  Multi-slot note:
    Bits 0 and 1 appear to determine the selected game slot.
    It should be possible to multiplex different game ROMs at
    0x000000-0x3fffff based on these bits.
*/
	if (BIT(m_bios_bank ^ data, 4))
		logerror("BIOS: P1 Start %sactive\n", BIT(data, 4) ? "in" : "");
	if (BIT(m_bios_bank ^ data, 5))
		logerror("BIOS: P2 Start %sactive\n", BIT(data, 5) ? "in" : "");
	m_bios_bank = data;
	m_bios_mode = MP_ROM;
//  logerror("BIOS: ROM bank %i selected [0x%02x]\n", m_bios_bank >> 6, data);
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
	uint8_t* bank = memregion("mtbios")->base();
	uint32_t fulladdress = m_bios_bank_addr + offset;

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
	uint32_t fulladdress = m_bios_bank_addr + offset;

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
	return ((m_bios_6403 & 0x10) >> 4);
//  return m_bios_6404 | (m_bios_6403 & 0x10) >> 4;
}

WRITE8_MEMBER(mplay_state::bios_6404_w)
{
	if(((m_bios_6404 & 0x0c) == 0x00) && ((data & 0x0c) == 0x0c))
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
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
		logerror("BIOS [0x%04x]: 68K address space bank selected: 0x%03x\n", m_bioscpu->pcbase(), m_game_banksel);
	}

	m_bios_bank_addr = ((m_bios_bank_addr >> 1) | (data << 23)) & 0xff8000;
	logerror("BIOS bank addr = %X\n", m_bios_bank_addr);
}

void mplay_state::megaplay_bios_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4fff).ram();
	map(0x5000, 0x5fff).ram();
	map(0x6000, 0x6000).w(FUNC(mplay_state::game_w));
	map(0x6200, 0x6207).rw("io1", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
	map(0x6400, 0x6407).rw("io2", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
	map(0x6600, 0x6600).rw(FUNC(mplay_state::bios_6600_r), FUNC(mplay_state::bios_6600_w));
	map(0x6800, 0x77ff).ram().share("ic3_ram");
	map(0x8000, 0xffff).rw(FUNC(mplay_state::bank_r), FUNC(mplay_state::bank_w));
}



READ8_MEMBER(mplay_state::vdp1_count_r)
{
	if (offset & 0x01)
		return m_vdp1->hcount_read();
	else
		return m_vdp1->vcount_read();
}

void mplay_state::megaplay_bios_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x7f, 0x7f).w(m_vdp1, FUNC(sega315_5124_device::psg_w));

	map(0x40, 0x41).mirror(0x3e).r(FUNC(mplay_state::vdp1_count_r));
	map(0x80, 0x80).mirror(0x3e).rw(m_vdp1, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0x81, 0x81).mirror(0x3e).rw(m_vdp1, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
}


uint32_t mplay_state::screen_update_megplay(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//printf("megplay vu\n");
	screen_update_megadriv(screen, bitmap, cliprect);
	//m_vdp1->screen_update(screen, bitmap, cliprect);

	// TODO : the overlay (256 pixels wide) is actually stretched over the 320 resolution genesis output, reference is https://youtu.be/Oir1Wp6yOq0.
	// if it's meant to be stretched we'll have to multiply the entire outut x4 for the Genesis VDP and x5 for the SMS VDP to get a common 1280 pixel wide image

	// overlay, only drawn for pixels != 0
	for (int y = 0; y < 224; y++)
	{
		uint32_t* lineptr = &bitmap.pix32(y);
		uint32_t* srcptr =  &m_vdp1->get_bitmap().pix32(y + sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT);

		for (int x = 0; x < sega315_5124_device::WIDTH; x++)
		{
			uint32_t src = srcptr[x] & 0xffffff;

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

void mplay_state::megaplay(machine_config &config)
{
	/* basic machine hardware */
	md_ntsc(config);

	/* The Megaplay has an extra BIOS cpu which drives an SMS VDP
	   which includes an SN76496 for sound */
	Z80(config, m_bioscpu, MASTER_CLOCK / 15); /* ?? */
	m_bioscpu->set_addrmap(AS_PROGRAM, &mplay_state::megaplay_bios_map);
	m_bioscpu->set_addrmap(AS_IO, &mplay_state::megaplay_bios_io_map);

	config.m_minimum_quantum = attotime::from_hz(6000);

	cxd1095_device &io1(CXD1095(config, "io1", 0));
	io1.in_porta_cb().set_ioport("DSW0");
	io1.in_portb_cb().set_ioport("DSW1");
	io1.out_portd_cb().set(FUNC(mplay_state::bios_banksel_w));
	io1.in_porte_cb().set(FUNC(mplay_state::bios_6204_r));
	io1.out_porte_cb().set(FUNC(mplay_state::bios_width_w));

	cxd1095_device &io2(CXD1095(config, "io2", 0));
	io2.in_porta_cb().set_ioport("TEST");
	io2.in_portb_cb().set_ioport("COIN");
	io2.in_portc_cb().set(FUNC(mplay_state::bios_6402_r));
	io2.out_portc_cb().set(FUNC(mplay_state::bios_6402_w));
	io2.out_portd_cb().set(FUNC(mplay_state::bios_gamesel_w));
	io2.in_porte_cb().set(FUNC(mplay_state::bios_6404_r));
	io2.out_porte_cb().set(FUNC(mplay_state::bios_6404_w));

	/* New update functions to handle the extra layer */
	subdevice<screen_device>("megadriv")->set_raw(XTAL(10'738'635)/2, \
			sega315_5124_device::WIDTH, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256, \
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	subdevice<screen_device>("megadriv")->set_screen_update(FUNC(mplay_state::screen_update_megplay));

	// Megaplay has an additional SMS VDP as an overlay
	SEGA315_5246(config, m_vdp1, MASTER_CLOCK / 5); /* ?? */
	m_vdp1->set_screen("megadriv");
	m_vdp1->set_is_pal(false);
	m_vdp1->irq().set_inputline(m_bioscpu, 0);
	m_vdp1->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_vdp1->add_route(ALL_OUTPUTS, "rspeaker", 0.25);
}


/* MegaPlay Games - Modified Genesis games */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

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


ROM_START( mp_twcup ) /* Tecmo World Cup */
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

ROM_START( mp_gunhe ) /* Gunstar Heroes */
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16390.ic1", 0x000000, 0x100000, CRC(d963a748) SHA1(adf231c5180a9307fd6675fe77fffd4c5bfa3d6a) )
	/* Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal) */

	ROM_REGION( 0x8000, "user1", 0 ) /* Game Instructions */
	ROM_LOAD( "epr-15175-10.ic2", 0x000000, 0x08000, CRC(e4f08233) SHA1(b7e0ad3f6ae1c56df6ec76375842050f08afcbef) )

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


void mplay_state::init_megaplay()
{
	// copy game instruction rom to main map. maybe this should just be accessed
	// through a handler instead?
	uint8_t *instruction_rom = memregion("user1")->base();
	uint8_t *game_rom = memregion("maincpu")->base();

	for (int offs = 0; offs < 0x8000; offs++)
	{
		uint8_t dat = instruction_rom[offs];

		game_rom[0x300000 + offs * 2] = dat;
		game_rom[0x300001 + offs * 2] = dat;
	}

	// to support the old code
	m_ic36_ram = std::make_unique<uint16_t[]>(0x10000 / 2);
	m_ic37_ram = std::make_unique<uint8_t[]>(0x10000);

	init_megadrij();
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
Gunstar Heroes       171-6215A   837-9165-10       610-0297-09**        MPR-16390   (838200B)   EPR-15175-10 (27256)   n/a
Mazin Wars           171-6215A   837-9165-11       610-0297-11          MPR-16460   (838200)    EPR-15175-11 (27256)   n/a

* This is the code for Tecmo World Cup, as the ROMs in the Columns 3 cart
didn't have original Sega part numbers it's probably a converted TWC cart
** Probably reused cart case
*/

/* -- */ GAME( 1993, megaplay, 0,        megaplay, megaplay, mplay_state, init_megaplay, ROT0, "Sega",                  "Mega Play BIOS", MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 01 */ GAME( 1993, mp_sonic, megaplay, megaplay, mp_sonic, mplay_state, init_megaplay, ROT0, "Sega",                  "Sonic The Hedgehog (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 02 */ GAME( 1993, mp_gaxe2, megaplay, megaplay, mp_gaxe2, mplay_state, init_megaplay, ROT0, "Sega",                  "Golden Axe II (Mega Play) (Rev B)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 02 */ GAME( 1993, mp_gaxe2a,mp_gaxe2, megaplay, mp_gaxe2, mplay_state, init_megaplay, ROT0, "Sega",                  "Golden Axe II (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 03 */ GAME( 1993, mp_gslam, megaplay, megaplay, mp_gslam, mplay_state, init_megaplay, ROT0, "Sega",                  "Grand Slam (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 04 */ GAME( 1993, mp_twcup, megaplay, megaplay, mp_twc,   mplay_state, init_megaplay, ROT0, "Sega",                  "Tecmo World Cup (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 05 */ GAME( 1993, mp_sor2,  megaplay, megaplay, mp_sor2,  mplay_state, init_megaplay, ROT0, "Sega",                  "Streets of Rage II (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 06 */ GAME( 1993, mp_bio,   megaplay, megaplay, mp_bio,   mplay_state, init_megaplay, ROT0, "Sega",                  "Bio-hazard Battle (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 07 */ GAME( 1993, mp_soni2, megaplay, megaplay, mp_soni2, mplay_state, init_megaplay, ROT0, "Sega",                  "Sonic The Hedgehog 2 (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 08 - Columns 3? see below */
/* 09 */ GAME( 1993, mp_shnb3, megaplay, megaplay, mp_shnb3, mplay_state, init_megaplay, ROT0, "Sega",                  "Shinobi III (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 10 */ GAME( 1993, mp_gunhe, megaplay, megaplay, mp_gunhe, mplay_state, init_megaplay, ROT0, "Sega",                  "Gunstar Heroes (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
/* 11 */ GAME( 1993, mp_mazin, megaplay, megaplay, mp_mazin, mplay_state, init_megaplay, ROT0, "Sega",                  "Mazin Wars / Mazin Saga (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

/* ?? */ GAME( 1993, mp_col3,  megaplay, megaplay, megaplay, mplay_state, init_megaplay, ROT0, "Sega",                  "Columns III (Mega Play)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )


/* Not confirmed to exist:

system16.com lists 'Streets of Rage' but this seems unlikely, there are no gaps in
the numbering prior to 'Streets of Rage 2'

*/
