// license:BSD-3-Clause
// copyright-holders:David Haywood

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

The BIOS reads the Start inputs and (once enough coins have been inserted) passes them to
the 68k rather than using the regular Genesis PAD hookups.

Communication between the various CPUs seems to be fairly complex and it is not fully
understood what is shared, where, and how.  One of the BIOS sets doesn't work, maybe for
this reason.

*/

#include "emu.h"
#include "megadriv.h"

#include "cpu/z80/z80.h"
#include "machine/cxd1095.h"


namespace {

#define MASTER_CLOCK        53693100

#define MP_ROM  1
#define MP_GAME 0


class mplay_state : public md_ctrl_state
{
public:
	mplay_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_ctrl_state(mconfig, type, tag),
		m_ic3_ram(*this, "ic3_ram"),
		m_vdp1(*this, "vdp1"),
		m_bioscpu(*this, "mtbios")
	{ }

	void megaplay(machine_config &config);

	void init_megaplay();

	int start1_r();
	int start2_r();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:

	uint16_t extra_ram_r(offs_t offset);
	void extra_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bios_banksel_w(uint8_t data);
	void bios_gamesel_w(uint8_t data);
	void mp_io_exp_out(uint8_t data, uint8_t mem_mask);
	uint8_t mp_io_exp_in();
	uint8_t bank_r(offs_t offset);
	void bank_w(offs_t offset, uint8_t data);
	uint8_t bios_6402_r();
	void bios_6402_w(uint8_t data);
	uint8_t bios_6204_r();
	void bios_width_w(uint8_t data);
	uint8_t bios_6404_r();
	void bios_6404_w(uint8_t data);
	uint8_t bios_6600_r();
	void bios_6600_w(uint8_t data);
	void game_w(uint8_t data);
	uint8_t vdp1_count_r(offs_t offset);

	uint32_t screen_update_megplay(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void megaplay_bios_io_map(address_map &map) ATTR_COLD;
	void megaplay_bios_map(address_map &map) ATTR_COLD;

	uint32_t m_bios_mode = 0;  // determines whether ROM banks or Game data is to read from 0x8000-0xffff

	uint32_t m_bios_bank = 0; // ROM bank selection
	uint16_t m_game_banksel = 0;  // Game bank selection
	uint32_t m_readpos = 0;  // serial bank selection position (9-bit)
	uint32_t m_bios_bank_addr = 0;

	uint32_t m_bios_width = 0;  // determines the way the game info ROM is read
	uint8_t m_bios_6600 = 0;
	uint8_t m_bios_6403 = 0;
	uint8_t m_bios_6404 = 0;

	uint8_t m_io_exp_data = 0;

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
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(mplay_state::start1_r))

	PORT_MODIFY("PAD2") // P2 Start input processed through BIOS
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(mplay_state::start2_r))

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

	PORT_START("DSW1")  // DSW C  (per game settings)
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

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, "Initial Players" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Hardest ) )
	// Who knows...
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 5") PORT_CODE(KEYCODE_H)
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 6") PORT_CODE(KEYCODE_J)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("0x6201 bit 7") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_gaxe2 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
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

static INPUT_PORTS_START ( mp_col3 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
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

static INPUT_PORTS_START ( mp_twc )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x01, 0x01, "Time" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING( 0x00, "Short" )
	PORT_DIPSETTING( 0x01, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x0e, 0x08, "Level" ) PORT_DIPLOCATION("SW3:2,3,4")
	PORT_DIPSETTING( 0x00, "0 (duplicate 1)" )
	PORT_DIPSETTING( 0x02, "0 (duplicate 2)" )
	PORT_DIPSETTING( 0x0e, "0" )
	PORT_DIPSETTING( 0x0c, "1" )
	PORT_DIPSETTING( 0x0a, "2" )
	PORT_DIPSETTING( 0x08, "3" )
	PORT_DIPSETTING( 0x06, "4" )
	PORT_DIPSETTING( 0x04, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_sor2 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0xc, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x00, DEF_STR ( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_bio )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x03, "3" )
	PORT_DIPSETTING( 0x01, "4" )
	PORT_DIPSETTING( 0x00, "5" )
	PORT_DIPNAME( 0xc, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x00, DEF_STR ( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_gslam )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x07, 0x04, DEF_STR ( Game_Time ) ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING( 0x07, "1:30" )
	PORT_DIPSETTING( 0x06, "2:00" )
	PORT_DIPSETTING( 0x05, "2:30" )
	PORT_DIPSETTING( 0x04, "3:00" )
	PORT_DIPSETTING( 0x03, "3:30" )
	PORT_DIPSETTING( 0x02, "4:00" )
	PORT_DIPSETTING( 0x01, "4:30" )
	PORT_DIPSETTING( 0x00, "5:00" )
	PORT_DIPNAME( 0x08, 0x08, "2P-Play Continue" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING( 0x00, "1 Credit" )
	PORT_DIPSETTING( 0x08, "2 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_mazin )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, "Initial Player" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x02, "1" )
	PORT_DIPSETTING( 0x03, "2" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING( 0x04, DEF_STR ( Normal ) )
	PORT_DIPSETTING( 0x00, DEF_STR ( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, "Title" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING( 0x08, "EUROPE" )
	PORT_DIPSETTING( 0x00, "U.S.A" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_soni2 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, "Initial Players (Normal mode)" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, "Initial Players (Dual mode)" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x08, "1" )
	PORT_DIPSETTING( 0x04, "2" )
	PORT_DIPSETTING( 0x0c, "3" )
	PORT_DIPSETTING( 0x00, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_shnb3 )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0xc, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x00, "Expert" )
INPUT_PORTS_END

static INPUT_PORTS_START ( mp_gunhe )
	PORT_INCLUDE( megaplay )

	PORT_MODIFY("DSW1") // DSW C  (per game settings)
	PORT_DIPNAME( 0x03, 0x01, "Initial Players" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING( 0x03, "1" )
	PORT_DIPSETTING( 0x02, "2" )
	PORT_DIPSETTING( 0x01, "3" )
	PORT_DIPSETTING( 0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING( 0x08, DEF_STR ( Easy ) )
	PORT_DIPSETTING( 0x0c, DEF_STR ( Normal ) )
	PORT_DIPSETTING( 0x04, DEF_STR ( Hard ) )
	PORT_DIPSETTING( 0x00, "Expert" )
INPUT_PORTS_END

// MEGAPLAY specific

int mplay_state::start1_r()
{
	return BIT(m_bios_bank, 4);
}

int mplay_state::start2_r()
{
	return BIT(m_bios_bank, 5);
}

void mplay_state::bios_banksel_w(uint8_t data)
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

void mplay_state::bios_gamesel_w(uint8_t data)
{
	m_bios_6403 = data;

//  logerror("BIOS: 0x6403 write: 0x%02x\n",data);
	m_bios_mode = BIT(data, 4);
}

void mplay_state::mp_io_exp_out(uint8_t data, uint8_t mem_mask)
{
	// TODO: TH (bit 6) is configured as an output and seems to be used for something, too
	m_io_exp_data = (data & 0x07) | (m_io_exp_data & 0xf8);
}

uint8_t mplay_state::mp_io_exp_in()
{
	return m_io_exp_data;
}

uint8_t mplay_state::bank_r(offs_t offset)
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
			return memregion("maincpu")->as_u8(BYTE_XOR_BE(fulladdress));
		}
	}
	else if (fulladdress >= 0xa10000 && fulladdress <= 0xa1001f) // IO access
	{
		return m_maincpu->space(AS_PROGRAM).read_byte(fulladdress);
	}
	else
	{
		printf("bank_r fulladdress %08x\n", fulladdress);
		return 0x00;
	}

}

void mplay_state::bank_w(offs_t offset, uint8_t data)
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
		m_maincpu->space(AS_PROGRAM).write_byte(fulladdress, data);
	}
	else
	{
		printf("bank_w fulladdress %08x\n", fulladdress);
	}
}


/* Megaplay BIOS handles regs[2] at start in a different way compared to MegaDrive
   other I/O data/ctrl regs are dealt with exactly like in the console */

uint8_t mplay_state::bios_6402_r()
{
	return m_io_exp_data;// & 0xfe;
}

void mplay_state::bios_6402_w(uint8_t data)
{
	m_io_exp_data = (m_io_exp_data & 0x07) | ((data & 0x70) >> 1);
//  logerror("BIOS: 0x6402 write: 0x%02x\n", data);
}

uint8_t mplay_state::bios_6204_r()
{
	return m_io_exp_data;
//  return (m_bios_width & 0xf8) + (m_bios_6204 & 0x07);
}

void mplay_state::bios_width_w(uint8_t data)
{
	m_bios_width = data;
	m_io_exp_data = (m_io_exp_data & 0x07) | ((data & 0xf8));
//  logerror("BIOS: 0x6204 - Width write: %02x\n", data);
}

uint8_t mplay_state::bios_6404_r()
{
//  logerror("BIOS: 0x6404 read: returned 0x%02x\n",bios_6404 | (bios_6403 & 0x10) >> 4);
	return ((m_bios_6403 & 0x10) >> 4);
//  return m_bios_6404 | (m_bios_6403 & 0x10) >> 4;
}

void mplay_state::bios_6404_w(uint8_t data)
{
	if(((m_bios_6404 & 0x0c) == 0x00) && ((data & 0x0c) == 0x0c))
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	m_bios_6404 = data;

//  logerror("BIOS: 0x6404 write: 0x%02x\n", data);
}

uint8_t mplay_state::bios_6600_r()
{
/*  Multi-slot note:
    0x6600 appears to be used to check for extra slots being used.
    Enter the following line in place of the return statement in this
    function to make the BIOS check all 4 slots (3 and 4 will be "not used")
        return (m_bios_6600 & 0xfe) | (m_bios_bank & 0x01);
*/
	return m_bios_6600;// & 0xfe;
}

void mplay_state::bios_6600_w(uint8_t data)
{
	m_bios_6600 = data;
//  logerror("BIOS: 0x6600 write: 0x%02x\n",data);
}

void mplay_state::game_w(uint8_t data)
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



uint8_t mplay_state::vdp1_count_r(offs_t offset)
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
	// if it's meant to be stretched we'll have to multiply the entire output x4 for the Genesis VDP and x5 for the SMS VDP to get a common 1280 pixel wide image

	// overlay, only drawn for pixels != 0
	const u32 width = sega315_5124_device::WIDTH - (sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH);
	for (int y = 0; y < 224; y++)
	{
		uint32_t *const lineptr = &bitmap.pix(y);
		uint32_t const *const srcptr =  &m_vdp1->get_bitmap().pix(y + sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH);
		uint8_t const *const y1ptr = &m_vdp1->get_y1_bitmap().pix(y + sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH);

		for (int srcx = 0, xx = 0, dstx = 0; srcx < width; dstx++)
		{
			uint32_t src = srcptr[srcx] & 0xffffff;

			if (y1ptr[srcx])
			{
				lineptr[dstx] = src;
			}
			if (++xx >= 5)
			{
				srcx++;
				xx = 0;
			}
		}
	}

	return 0;
}

void mplay_state::machine_reset()
{
	m_bios_mode = MP_ROM;
	m_bios_bank_addr = 0;
	m_readpos = 1;
	md_ctrl_state::machine_reset();
}

void mplay_state::megaplay(machine_config &config)
{
	// basic machine hardware
	md_ntsc(config);

	// integrated 3-button controllers
	ctrl1_3button(config);
	ctrl2_3button(config);

	// for now ...
	m_ioports[2]->set_in_handler(FUNC(mplay_state::mp_io_exp_in));
	m_ioports[2]->set_out_handler(FUNC(mplay_state::mp_io_exp_out));

	// The Megaplay has an extra BIOS CPU which drives an SMS VDP which includes an SN76496 for sound
	Z80(config, m_bioscpu, MASTER_CLOCK / 15); // ??
	m_bioscpu->set_addrmap(AS_PROGRAM, &mplay_state::megaplay_bios_map);
	m_bioscpu->set_addrmap(AS_IO, &mplay_state::megaplay_bios_io_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	cxd1095_device &io1(CXD1095(config, "io1"));
	io1.in_porta_cb().set_ioport("DSW0");
	io1.in_portb_cb().set_ioport("DSW1");
	io1.out_portd_cb().set(FUNC(mplay_state::bios_banksel_w));
	io1.in_porte_cb().set(FUNC(mplay_state::bios_6204_r));
	io1.out_porte_cb().set(FUNC(mplay_state::bios_width_w));

	cxd1095_device &io2(CXD1095(config, "io2"));
	io2.in_porta_cb().set_ioport("TEST");
	io2.in_portb_cb().set_ioport("COIN");
	io2.in_portc_cb().set(FUNC(mplay_state::bios_6402_r));
	io2.out_portc_cb().set(FUNC(mplay_state::bios_6402_w));
	io2.out_portd_cb().set(FUNC(mplay_state::bios_gamesel_w));
	io2.in_porte_cb().set(FUNC(mplay_state::bios_6404_r));
	io2.out_porte_cb().set(FUNC(mplay_state::bios_6404_w));

	m_vdp->set_lcm_scaling(true);

	// New update functions to handle the extra layer
	subdevice<screen_device>("megadriv")->set_raw((XTAL(10'738'635) * 5)/2,
			sega315_5124_device::WIDTH * 5, (sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH) * 5, (sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256) * 5,
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	subdevice<screen_device>("megadriv")->set_screen_update(FUNC(mplay_state::screen_update_megplay));

	// Megaplay has an additional SMS VDP as an overlay
	SEGA315_5246(config, m_vdp1, MASTER_CLOCK / 5); // ??
	m_vdp1->set_screen("megadriv");
	m_vdp1->set_hcounter_divide(5);
	m_vdp1->set_is_pal(false);
	m_vdp1->n_int().set_inputline(m_bioscpu, 0);
	m_vdp1->add_route(ALL_OUTPUTS, "speaker", 0.25);
	m_vdp1->add_route(ALL_OUTPUTS, "speaker", 0.25);
}


// MegaPlay Games - Modified Genesis games

#define MEGAPLAY_PLDS \
	ROM_REGION( 0x651, "plds", 0) \
	ROM_LOAD( "315-5661.ic7",   0x000, 0x117, BAD_DUMP CRC(d8289e31) SHA1(a0e9134d9e8043a3660a2ce122cfd5d7f76773b9) ) /* GAL16V8, bruteforced but verified */ \
	ROM_LOAD( "315-5653.ic56",  0x117, 0x117, BAD_DUMP CRC(fd5c4fb3) SHA1(6b2ba657836f3031d77602526416200e31d41a6e) ) /* GAL16V8, bruteforced but verified */ \
	ROM_LOAD( "315-5651.ic8",   0x22e, 0x117, BAD_DUMP CRC(55c6cddb) SHA1(e1a968305ca7ea17e9021b31506ca087b84a8ab1) ) /* GAL16V8, bruteforced but verified */ \
	ROM_LOAD( "315-5349a.ic54", 0x345, 0x104, BAD_DUMP CRC(825ea316) SHA1(f49edb6a3f9349330f7ff525ef60517ed276a663) ) /* PAL16L8BCN, bruteforced but verified */ \
	ROM_LOAD( "315-5654.ic33",  0x449, 0x104, NO_DUMP ) /* PAL16L8BCN */ \
	ROM_LOAD( "315-5655.ic34",  0x54d, 0x104, BAD_DUMP CRC(4a2d27d1) SHA1(ba79183e4b522d1b57a46a56fc7d9b85de24df36) ) /* PAL16L8BCN, bruteforced but verified */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

#define MEGAPLAY_BIOS \
	ROM_SYSTEM_BIOS( 0, "ver1",       "Mega Play BIOS (Ver. 1)" ) \
	ROM_LOAD_BIOS( 0, "ep15294.ic2",   0x000000, 0x20000, CRC(aa8dc2d8) SHA1(96771ad7b79dc9c83a1594243250d65052d23176) ) \
	ROM_SYSTEM_BIOS( 1, "ver2",       "Mega Play BIOS (Ver. 2)" ) /* This one doesn't boot... Dump was verified with another working PCB */ \
	ROM_LOAD_BIOS( 1, "epr-a15294.ic2",0x000000, 0x20000, CRC(f97c68aa) SHA1(bcabc879950bca1ced11c550a484e697ec5706b2) )

ROM_START( megaplay )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

/* The system appears to access the instruction rom at
    0x300000 in the 68k space (ROM window from z80 side)

   This probably means the maximum 68k rom size is 0x2fffff for MegaPlay
*/

// PCB  171-5834
ROM_START( mp_sonic ) // Sonic
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ep15177.ic2", 0x000000, 0x040000, CRC(a389b03b) SHA1(8e9e1cf3dd65ddf08757f5a1ce472130c902ea2c) )
	ROM_LOAD16_BYTE( "ep15176.ic1", 0x000001, 0x040000, CRC(d180cc21) SHA1(62805cfaaa80c1da6146dd89fc2b49d819fd4f22) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "ep15175-01.ic3", 0x000000, 0x08000, CRC(99246889) SHA1(184aa3b7fdedcf578c5e34edb7ed44f57f832258) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

/* this cart looks to be a conversion from something else... Sega rom numbers were missing
   but the code looks like it's probably real */
// PCB  171-5834
ROM_START( mp_col3 ) // Columns 3
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.ic2", 0x000000, 0x040000, CRC(a1602235) SHA1(38751b585849c8966acc3f508714937fe29dcf5c) )
	ROM_LOAD16_BYTE( "2.ic1", 0x000001, 0x040000, CRC(999b2fe6) SHA1(ad967a28e4eebd7b01273e4e04c35a0198ef834a) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "1.ic3", 0x000000, 0x08000,  CRC(dac9bf91) SHA1(0117972a7181f8aaf942a259cc8764b821031253) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_gaxe2 ) // Golden Axe 2, revision B
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ep15179b.ic2", 0x000000, 0x040000, CRC(00d97b84) SHA1(914bbf566ddf940aab67b92af237d251650ddadf) )
	ROM_LOAD16_BYTE( "ep15178b.ic1", 0x000001, 0x040000, CRC(2ea576db) SHA1(6d96b948243533de1f488b1f80e0d5431a4f1f53) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game Instructions
	ROM_LOAD( "ep15175-02b.ic3", 0x000000, 0x08000, CRC(3039b653) SHA1(b19874c74d0fc0cca1169f62e5e74f0e8ca83679) ) // 15175-02b.ic3

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_gaxe2a ) // Golden Axe 2
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15179.ic2", 0x000000, 0x040000, CRC(d35f1a35) SHA1(3105cd3b55f65337863703db04527fe298fc04e0) )
	ROM_LOAD16_BYTE( "epr-15178.ic1", 0x000001, 0x040000, CRC(2c6b6b76) SHA1(25577f49ecad451c217da9cacbd78ffca9dca24e) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "epr-15175-02.ic3", 0x000000, 0x08000, CRC(cfc87f91) SHA1(110609094aa6d848bec613faa0558db7ad272b77) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_gslam ) // Grand Slam
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-15181.ic2", 0x000000, 0x040000, CRC(642437c1) SHA1(cbf88e196c04b6d886bf9642b69bf165045510fe) )
	ROM_LOAD16_BYTE( "epr-15180.ic1", 0x000001, 0x040000, CRC(73bb48f1) SHA1(981b64f834d5618599352f5fad683bf232390ba3) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "epr-15175-03.ic3", 0x000000, 0x08000, CRC(70ea1aec) SHA1(0d9d82a1f8aa51d02707f7b343e7cfb6591efccd) ) // 15175-02b.ic3

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END


ROM_START( mp_twcup ) // Tecmo World Cup
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ep15183.ic2", 0x000000, 0x040000, CRC(8b79b861) SHA1(c72af72840513b82f2562409eccdf13b031bf3c0) )
	ROM_LOAD16_BYTE( "ep15182.ic1", 0x000001, 0x040000, CRC(eb8325c3) SHA1(bb21ac926c353e14184dd476222bc6a8714606e5) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "ep15175-04.ic3", 0x000000, 0x08000, CRC(faf7c030) SHA1(16ef405335b4d3ecb0b7d97b088dafc4278d4726) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_sor2 ) // Streets of Rage 2
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-15425.ic1", 0x000000, 0x200000, CRC(cd6376af) SHA1(57ec210975e40505649f152b60ef54f99da31f0e) )
	// Game Instruction rom copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "epr-15175-05.ic2", 0x000000, 0x08000, CRC(1df5347c) SHA1(faced2e875e1914392f61577b5256d006eebeef9) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_bio ) // Bio Hazard Battle
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-15699-f.ic1", 0x000000, 0x100000, CRC(4b193229) SHA1(f8629171ae9b4792f142f6957547d886e5cc6817) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "epr-15175-06.ic2", 0x000000, 0x08000, CRC(1ef64e41) SHA1(13984b714b014ea41963b70de74a5358ed223bc5) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_soni2 ) // Sonic The Hedgehog 2
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16011.ic1", 0x000000, 0x100000, CRC(3d7bf98a) SHA1(dce0e4e8f2573e0ffe851edaa235e4ed9e61ee2d) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "epr-15175-07.ic1", 0x000000, 0x08000, CRC(bb5f67f0) SHA1(33b7a5d14015a5fcf41976a8f648f8f48ce9bb03) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_mazin ) // Mazin Wars
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16460.ic1", 0x000000, 0x100000, CRC(e9635a83) SHA1(ab3afa11656f0ae3a50c957dce012fb15d3992e0) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "epr-15175-11.ic2", 0x000000, 0x08000, CRC(bb651120) SHA1(81cb736f2732373e260dde162249c1d29a3489c3) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_shnb3 ) // Shinobi 3
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16197.ic1", 0x000000, 0x100000, CRC(48162361) SHA1(77d544509339b5ddf6d19941377e81d29e9e21dc) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "epr-15175-09.ic2", 0x000000, 0x08000, CRC(6254e45a) SHA1(8667922a6eade03c964ce224f7fa39ba871c60a4) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

ROM_START( mp_gunhe ) // Gunstar Heroes
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mpr-16390.ic1", 0x000000, 0x100000, CRC(d963a748) SHA1(adf231c5180a9307fd6675fe77fffd4c5bfa3d6a) )
	// Game instruction ROM copied to 0x300000 - 0x310000 (odd / even bytes equal)

	ROM_REGION( 0x8000, "user1", 0 ) // Game instructions
	ROM_LOAD( "epr-15175-10.ic2", 0x000000, 0x08000, CRC(e4f08233) SHA1(b7e0ad3f6ae1c56df6ec76375842050f08afcbef) )

	ROM_REGION( 0x20000, "mtbios", 0 ) // BIOS
	MEGAPLAY_BIOS

	MEGAPLAY_PLDS
ROM_END

uint16_t mplay_state::extra_ram_r(offs_t offset)
{
	return m_ic36_ram[(offset << 1) ^ 1] | (m_ic36_ram[(offset << 1)] << 8);
}

void mplay_state::extra_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

	// use Export NTSC init, as MegaPlay was apparently only intended for export markets.
	init_megadriv();

	// megaplay has ram shared with the bios cpu here
	m_z80snd->space(AS_PROGRAM).install_ram(0x2000, 0x3fff, &m_ic36_ram[0]);

	// instead of a RAM mirror the 68k sees the extra ram of the 2nd z80 too
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa02000, 0xa03fff, read16sm_delegate(*this, FUNC(mplay_state::extra_ram_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa02000, 0xa03fff, write16s_delegate(*this, FUNC(mplay_state::extra_ram_w)));
}

} // anonymous namespace


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
Sonic The Hedgehog   171-5834    610-0298-01       610-0297-01          EPR-15176   (27C2001)   EPR-15177    (27C2001) EPR-15175-01 (27C256)
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

/* -- */ GAME( 1993, megaplay, 0,        megaplay, megaplay, mplay_state, init_megaplay, ROT0, "Sega", "Mega Play BIOS",                      MACHINE_IS_BIOS_ROOT | MACHINE_IMPERFECT_GRAPHICS )
/* 01 */ GAME( 1993, mp_sonic, megaplay, megaplay, mp_sonic, mplay_state, init_megaplay, ROT0, "Sega", "Sonic The Hedgehog (Mega Play)",      MACHINE_IMPERFECT_GRAPHICS )
/* 02 */ GAME( 1993, mp_gaxe2, megaplay, megaplay, mp_gaxe2, mplay_state, init_megaplay, ROT0, "Sega", "Golden Axe II (Mega Play) (Rev B)",   MACHINE_IMPERFECT_GRAPHICS )
/* 02 */ GAME( 1993, mp_gaxe2a,mp_gaxe2, megaplay, mp_gaxe2, mplay_state, init_megaplay, ROT0, "Sega", "Golden Axe II (Mega Play)",           MACHINE_IMPERFECT_GRAPHICS )
/* 03 */ GAME( 1993, mp_gslam, megaplay, megaplay, mp_gslam, mplay_state, init_megaplay, ROT0, "Sega", "Grand Slam (Mega Play)",              MACHINE_IMPERFECT_GRAPHICS )
/* 04 */ GAME( 1993, mp_twcup, megaplay, megaplay, mp_twc,   mplay_state, init_megaplay, ROT0, "Sega", "Tecmo World Cup (Mega Play)",         MACHINE_IMPERFECT_GRAPHICS )
/* 05 */ GAME( 1993, mp_sor2,  megaplay, megaplay, mp_sor2,  mplay_state, init_megaplay, ROT0, "Sega", "Streets of Rage II (Mega Play)",      MACHINE_IMPERFECT_GRAPHICS )
/* 06 */ GAME( 1993, mp_bio,   megaplay, megaplay, mp_bio,   mplay_state, init_megaplay, ROT0, "Sega", "Bio-hazard Battle (Mega Play)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // frequently softlocks after continue, usually on the 2nd stage
/* 07 */ GAME( 1993, mp_soni2, megaplay, megaplay, mp_soni2, mplay_state, init_megaplay, ROT0, "Sega", "Sonic The Hedgehog 2 (Mega Play)",    MACHINE_IMPERFECT_GRAPHICS )
/* 08 - Columns 3? see below */
/* 09 */ GAME( 1993, mp_shnb3, megaplay, megaplay, mp_shnb3, mplay_state, init_megaplay, ROT0, "Sega", "Shinobi III (Mega Play)",             MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // game softlocks if you coin during the intro sequence
/* 10 */ GAME( 1993, mp_gunhe, megaplay, megaplay, mp_gunhe, mplay_state, init_megaplay, ROT0, "Sega", "Gunstar Heroes (Mega Play)",          MACHINE_IMPERFECT_GRAPHICS )
/* 11 */ GAME( 1993, mp_mazin, megaplay, megaplay, mp_mazin, mplay_state, init_megaplay, ROT0, "Sega", "Mazin Wars / Mazin Saga (Mega Play)", MACHINE_IMPERFECT_GRAPHICS )

/* ?? */ GAME( 1993, mp_col3,  megaplay, megaplay, mp_col3,  mplay_state, init_megaplay, ROT0, "Sega", "Columns III (Mega Play)",             MACHINE_IMPERFECT_GRAPHICS )
