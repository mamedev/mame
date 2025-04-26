// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/upd4992.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

/*****************************************************************************

Name        Board No      Maker         Game name
----------------------------------------------------------------------------
pwrkick     SW931201-1    Sunwise       Power Kick
burgkids    SW931201-1    Sunwise       Burger Kids
pyutakun    SW931201-1    Sunwise       Pyuuta-kun
othldrby    S951060-VGP   Sunwise       Othello Derby


Notes on Power Kick coin inputs:
- The 10 yen input is "Key In" according to the bookkeeping screen, but is
  an otherwise normal coin input with a counter and a lockout (sharing the
  latter with the "medal" coin).
- The 100 yen input never adds any credits except in "Coin Function Check,"
  instead dispensing its value into the hopper immediately.

To reset the NVRAM in Othello Derby, hold P1 Button 1 down while booting.

*****************************************************************************/

namespace {

class sunwise_state : public driver_device
{
public:
	sunwise_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rtc(*this, "rtc")
		, m_hopper(*this, "hopper")
		, m_maincpu(*this, "maincpu")
		, m_vdp(*this, "gp9001")
		, m_oki(*this, "oki")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void othldrby(machine_config &config);
	void pwrkick(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

	void common_mem(address_map &map) ATTR_COLD;
	void pwrkick_68k_mem(address_map &map) ATTR_COLD;
	void othldrby_68k_mem(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void sw_oki_bankswitch_w(u8 data);

private:
	void pwrkick_coin_w(u8 data);
	void pwrkick_coin_lockout_w(u8 data);

	required_device<upd4992_device> m_rtc;
	optional_device<ticket_dispenser_device> m_hopper; // pwrkick only

	required_device<m68000_base_device> m_maincpu;
	required_device<gp9001vdp_device> m_vdp;
	required_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	bitmap_ind8 m_custom_priority_bitmap;
};

void sunwise_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}


u32 sunwise_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);
	return 0;
}

void sunwise_state::screen_vblank(int state)
{
	if (state) // rising edge
	{
		m_vdp->screen_eof();
	}
}

void sunwise_state::sw_oki_bankswitch_w(u8 data)
{
	m_oki->set_rom_bank(data & 1);
}



static INPUT_PORTS_START( pwrkick )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x5c, 0x00, "Payout" ) PORT_DIPLOCATION("SW1:!3,!4,!5,!7")
	PORT_DIPSETTING(    0x00, "110" ) // Service mode displays values as 1-8, ignoring SW1:7
	PORT_DIPSETTING(    0x04, "105" )
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x0c, "95" )
	PORT_DIPSETTING(    0x10, "90" )
	PORT_DIPSETTING(    0x14, "85" )
	PORT_DIPSETTING(    0x18, "80" )
	PORT_DIPSETTING(    0x1c, "75" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x44, "65" )
	PORT_DIPSETTING(    0x48, "60" )
	PORT_DIPSETTING(    0x4c, "55" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x54, "45" )
	PORT_DIPSETTING(    0x58, "40" )
	PORT_DIPSETTING(    0x5c, "35" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Diagnostic" ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Play Credit" ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x00, u8"¥10" )
	PORT_DIPSETTING(    0x01, u8"¥20" )
	PORT_DIPSETTING(    0x02, u8"¥30" )
	PORT_DIPSETTING(    0x03, u8"¥40" )
	PORT_DIPNAME( 0x0c, 0x00, "Coin Exchange" ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x00, "12" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, "Game Mode" ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, "Shot" )
	PORT_DIPSETTING(    0x20, "Auto" )
	PORT_DIPSETTING(    0x30, "S-Manual" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW3:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW3:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW3:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW3:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW3:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW3:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW3:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW3:!8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_NAME("Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_NAME("Center")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_NAME("Right")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME(u8"Coin 2 (¥10)")
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_HIGH )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP4 ) PORT_NAME("Down") // does this button really exist?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MEMORY_RESET )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYS")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME(u8"Coin Exchange (¥100)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_SERVICE ) PORT_NAME("Attendant Key")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1 (Medal)")

	// The specific "Payout" button shown on the test screen and diagnostic menu does not exist.
INPUT_PORTS_END

static INPUT_PORTS_START( burgkids )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x5c, 0x00, "Payout" ) PORT_DIPLOCATION("SW1:!3,!4,!5,!7")
	PORT_DIPSETTING(    0x00, "110" ) // Service mode displays values as 1-8, ignoring SW1:7
	PORT_DIPSETTING(    0x04, "105" )
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x0c, "95" )
	PORT_DIPSETTING(    0x10, "90" )
	PORT_DIPSETTING(    0x14, "85" )
	PORT_DIPSETTING(    0x18, "80" )
	PORT_DIPSETTING(    0x1c, "75" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x44, "65" )
	PORT_DIPSETTING(    0x48, "60" )
	PORT_DIPSETTING(    0x4c, "55" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x54, "45" )
	PORT_DIPSETTING(    0x58, "40" )
	PORT_DIPSETTING(    0x5c, "35" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Play Credit" ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x00, u8"¥10" )
	PORT_DIPSETTING(    0x01, u8"¥20" )
	PORT_DIPSETTING(    0x02, u8"¥30" )
	PORT_DIPSETTING(    0x03, u8"¥40" )
	PORT_DIPNAME( 0x1c, 0x00, "Coin Exchange" ) PORT_DIPLOCATION("SW2:!3,!4,!5")
	PORT_DIPSETTING(    0x00, "12" )
	PORT_DIPSETTING(    0x04, "11" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x14, "4" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x1c, DEF_STR ( Off ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW3:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW3:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW3:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW3:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW3:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW3:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW3:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW3:!8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_NAME("3")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME(u8"Coin 2 (¥10)")
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_HIGH )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP4 ) PORT_NAME("Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MEMORY_RESET )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYS")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME(u8"Coin Exchange (¥100)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_SERVICE ) PORT_NAME("Attendant Key")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1 (Medal)")
INPUT_PORTS_END


static INPUT_PORTS_START( pyutakun )
	PORT_INCLUDE( burgkids )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 )
INPUT_PORTS_END

static INPUT_PORTS_START( 2b )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL_LOC(SW1)
	// Coinage on bit mask 0x00f0
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below

	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY_LOC(SW2)
	// Per-game features on bit mask 0x00fc
	PORT_BIT( 0x00fc, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below
INPUT_PORTS_END


static INPUT_PORTS_START( base )
	PORT_INCLUDE( 2b )

	PORT_MODIFY("IN1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_MODIFY("IN2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )
INPUT_PORTS_END


static INPUT_PORTS_START( othldrby )
	PORT_INCLUDE( base )

	PORT_MODIFY("SYS")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


void sunwise_state::pwrkick_coin_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1)); // medal
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3)); // 10 yen
	machine().bookkeeping().coin_counter_w(2, BIT(data, 0)); // 100 yen
	m_hopper->motor_w(BIT(data, 7));
}

void sunwise_state::pwrkick_coin_lockout_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 2));
	machine().bookkeeping().coin_lockout_w(2, BIT(~data, 1));
}


void sunwise_state::common_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share("nvram"); // Only 10022C-10037B is actually saved as NVRAM
	map(0x104000, 0x10ffff).ram();

	map(0x200000, 0x20000f).rw(m_rtc, FUNC(upd4992_device::read), FUNC(upd4992_device::write)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x700000, 0x700001).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
	map(0x700004, 0x700005).portr("DSWA");
	map(0x700008, 0x700009).portr("DSWB");
	map(0x70001c, 0x70001d).portr("SYS");
	map(0x700031, 0x700031).w(FUNC(sunwise_state::sw_oki_bankswitch_w));
}

void sunwise_state::pwrkick_68k_mem(address_map &map)
{
	common_mem(map);

	map(0x70000c, 0x70000d).portr("IN1");
	map(0x700014, 0x700015).portr("IN2");
	map(0x700018, 0x700019).portr("DSWC");
	map(0x700035, 0x700035).w(FUNC(sunwise_state::pwrkick_coin_w));
	map(0x700039, 0x700039).w(FUNC(sunwise_state::pwrkick_coin_lockout_w));
}

void sunwise_state::othldrby_68k_mem(address_map &map)
{
	common_mem(map);

	map(0x70000c, 0x70000d).portr("IN1");
	map(0x700010, 0x700011).portr("IN2");
	map(0x700035, 0x700035).w("coincounter", FUNC(toaplan_coincounter_device::coin_w));
}


void sunwise_state::pwrkick(machine_config &config) // Sunwise SW931201-1 PCB (27.000MHz, 20.000MHz & 16.000MHz OSCs)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &sunwise_state::pwrkick_68k_mem);

	UPD4992(config, m_rtc, 32.768_kHz_XTAL);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(50)); // duration is probably wrong

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(sunwise_state::screen_update));
	m_screen->screen_vblank().set(FUNC(sunwise_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	// empty YM2151 socket
	OKIM6295(config, m_oki, 16_MHz_XTAL/4, okim6295_device::PIN7_LOW); // verified on PCB
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void sunwise_state::othldrby(machine_config &config) // Sunwise S951060-VGP PCB (27.000MHz, 20.000MHz & 16.000MHz OSCs)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2); // assumed same as pwrkick
	m_maincpu->set_addrmap(AS_PROGRAM, &sunwise_state::othldrby_68k_mem);

	TOAPLAN_COINCOUNTER(config, "coincounter", 0);

	UPD4992(config, m_rtc, 32.768_kHz_XTAL);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(sunwise_state::screen_update));
	m_screen->screen_vblank().set(FUNC(sunwise_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 16_MHz_XTAL/4, okim6295_device::PIN7_LOW); // assumed same as pwrkick
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( pwrkick ) // Sunwise SW931201-1 PCB - 8-liner connections
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "1.u61", 0x000000, 0x080000, CRC(118b5899) SHA1(7a1637a63eb17e3892d79aede5730013a1dc00f9) )

	ROM_REGION( 0x100000, "gp9001", ROMREGION_ERASE00 )
	ROM_LOAD( "2.u26", 0x000000, 0x080000, CRC(a190eaea) SHA1(2c7b8c8026873e0f591fbcbc2e72b196ef84e162) )
	ROM_LOAD( "3.u27", 0x080000, 0x080000, CRC(0b81e038) SHA1(8376617ae519a8ef604f20b26e941aa5b8066602) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "4.u33", 0x000000, 0x080000, CRC(3ab742f1) SHA1(ce8ca02ca57fd77872e421ce601afd017d3518a0) )
ROM_END

/*

Burger Kids, 1995 Sunwise

SW931201-1
+--||||||-----------------------------------------+
|         YM3014*  YM2151*                        |
| TDA1519A         M6295   FFK4.U33          BT1  |
|                              U31*               |
|                      16.000MHz 32.768kHz D4992C |
|                                      MB3771     |
|8                                                |
|-                                     U67*       |
|L                                 FFK1.U61 62256 |
|I                                     U68* 62256 |
|N                     GAL16V8B    TMP68HC000P-12 |
|E                                                |
|R                                                |
|                        1635  27.000MHz FFK3.U27 |
|             GAL16V8B   1635  20.000MHz     U12* |
| SW1         GAL16V8B    +---------+    FFK2.U26 |
| SW2                     | L7A0498 |        U13* |
| SW3                     | GP9001  |   4256 4256 |
|     6216                +---------+   4256 4256 |
|     6216                              4256 4256 |
+-------------------------------------------------+

NOTE: This PCB uses an 8-Liner style edge connector

* Denotes unpopulated components

   CPU: Toshiba TMP68HC000P-12
 Sound: OKI M6295
 Video: L7A0498 GP9001 QFP208
   OSC: 27MHz, 20MHz, 16MHz & 32.768kHz
   RAM: MB81C4256A-70P - 256K x 4-bit DRAM
        HM62256BLP-9 - 32K x 8-bit SRAM
        IMS1635P-25 - 8K x 8-bit SRAM
        XRM6216-10 - 2K x 8-bit SRAM
 Other: 8 Position Dipswitch x 3
        NEC D4992 CMOS 8-Bit Parallel I/O Calendar Clock
        MB3771 Voltage monitor
        BT1 - CR2550 3Volt battery

NOTE: Sunwise's S951060-VGP PCB uses identical components to the SW931201 but has a standard JAMMA connector
*/

ROM_START( burgkids ) // Sunwise SW931201-1 PCB - 8-liner connections
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ffk1.u61", 0x000000, 0x080000, CRC(ac96cb0d) SHA1(2ce5c06d61f3ff18b222619e41d09e46d44c5bab) )

	ROM_REGION( 0x100000, "gp9001", ROMREGION_ERASE00 )
	ROM_LOAD( "ffk2.u26", 0x000000, 0x080000, CRC(09f7b0ae) SHA1(f340f27a601ff89f143398263d822b8f340eea6e) )
	ROM_LOAD( "ffk3.u27", 0x080000, 0x080000, CRC(63c761bc) SHA1(f0ee1dc6aaeacff23e55d072102b814c7ef30550) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "ffk4.u33", 0x000000, 0x080000,  CRC(3b032d4f) SHA1(69056bf205aadf6c9fee56ce396b11a5187caa03) )
ROM_END

ROM_START( pyutakun ) // Sunwise SW931201-1 PCB - 8-liner connections
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sunwise_p1.u61", 0x00000, 0x80000, CRC(00907713) SHA1(0822044bcf476b3e8aaba752e503ee79459b34ed) )

	ROM_REGION( 0x100000, "gp9001", 0 )
	ROM_LOAD( "sunwise_p2.u26", 0x00000, 0x80000, CRC(180b8b13) SHA1(4a317bd0825f4e4383293220e775c0f807cdd80f) )
	ROM_LOAD( "sunwise_p3.u27", 0x80000, 0x80000, CRC(a23ccb8e) SHA1(5f2f23fa86817ff491058123d775ffebd7e98dee) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sunwise_p4.u33", 0x00000, 0x40000, CRC(b1c5a8bc) SHA1(aa18165a9214f9ed0969da073e4b6092be3c5c1a) )
ROM_END

ROM_START( othldrby ) // Sunwise S951060-VGP PCB - JAMMA compliant (components identical to Sunwise SW931201-1 PCB)
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sunwise_db0_1.u61", 0x00000, 0x80000, CRC(6b4008d3) SHA1(4cf838c47563ba482be8364b2e115569a4a06c83) )

	ROM_REGION( 0x400000, "gp9001", 0 )
	ROM_LOAD( "db0-r2.u26", 0x000000, 0x200000, CRC(4efff265) SHA1(4cd239ff42f532495946cb52bd1fee412f84e192) ) // mask ROMs
	ROM_LOAD( "db0-r3.u27", 0x200000, 0x200000, CRC(5c142b38) SHA1(5466a8b061a0f2545493de0f96fd4387beea276a) )

	ROM_REGION( 0x080000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "sunwise_db0_4.u33", 0x00000, 0x80000, CRC(a9701868) SHA1(9ee89556666d358e8d3915622573b3ba660048b8) )
ROM_END

} // anonymous namespace

GAME( 1994, pwrkick,     0,        pwrkick,    pwrkick,    sunwise_state,  empty_init,      ROT0,   "Sunwise",  "Power Kick (Japan)",    0 )
GAME( 1995, burgkids,    0,        pwrkick,    burgkids,   sunwise_state,  empty_init,      ROT0,   "Sunwise",  "Burger Kids (Japan)",   0 )
GAME( 1995, pyutakun,    0,        pwrkick,    pyutakun,   sunwise_state,  empty_init,      ROT0,   "Sunwise",  "Pyuuta-kun (Japan)",    0 )
GAME( 1995, othldrby,    0,        othldrby,   othldrby,   sunwise_state,  empty_init,      ROT0,   "Sunwise",  "Othello Derby (Japan)", 0 )
