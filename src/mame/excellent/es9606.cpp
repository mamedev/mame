// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Excellent System's ES-9606-01 PCB

TODO:
- alt input schemes (currently hardwired to Slot Game 1);
- Imagetek irqs, has code for bits 0-4 but only bit 0 seems used (i.e. irq enable 0x1e ^ 0xff).
  Also checks bit 5 outside irq routine at PC=11240 (just vblank delay with no irq source?
  Check other Imagetek games);
- Whatever the stealth mode is supposed to do, if anything at all.
  Reads inputs but does nothing with them except throwing a "COIN ERROR" (???)
- Locate "Service B35" pin (for hopper testing) update: seems not really coded?
- Boots with "Cadence Technology" if EEPROM initialized from test mode,
  is it possible to make it init as Excellent System mode?
- Has GFX transition issues when coining in (verify, should be just sloppy coding);
- lamps;

===================================================================================================

Main components:
TMP68HC000-P16 CPU
2x CY7C199-15PC RAM (near CPU)
32.000 MHz XTAL (near CPU)
ES9402LA (same label as the one in lastbank.cpp but actually Imagetek VDP I4220)
3x CY7C199-15PC RAM (near ES9402LA)
26.666 MHz XTAL (near ES9402LA)
Altera EPM7032LC44-15T CPLD
YMZ280B-F
empty space marked for YM2413
3.579545 MHz XTAL (near empty YM2413 space)
AT93C46 EEPROM
MM1035 Mitsumi System Reset IC with Built-in Watchdog Timer
bank of 8 DIP switches (3 other spaces not populated)
memory reset push-button

**************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "sound/ymz280b.h"
#include "video/imagetek_i4100.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class es9606_state : public driver_device
{
public:
	es9606_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_hopper(*this,"hopper"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_vdp2(*this, "vdp2")
	{ }

	void es9606(machine_config &config) ATTR_COLD;

protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ticket_dispenser_device> m_hopper;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	required_device<imagetek_i4220_device> m_vdp2;

	void eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void watchdog_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void program_map(address_map &map) ATTR_COLD;
};

void es9606_state::eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// bit 15: unknown, verbose
	if (data & 0x701f)
		logerror("%s unknown eeprom_w bits written %04x\n", machine().describe_context(), data);

	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->cs_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->clk_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->di_write(BIT(data, 7));
	}

	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data, 8));
		machine().bookkeeping().coin_counter_w(1, BIT(data, 9));
		machine().bookkeeping().coin_counter_w(2, BIT(data, 10));
		machine().bookkeeping().coin_counter_w(3, BIT(data, 11)); // key in
		m_hopper->motor_w(BIT(data, 12));
	}
}

// lamps
// x--- ---- Key Out lamp?
// -x-- ---- Gamble High
// --x- ---- Take Score
// ---x ---- Double Up
// ---- x--- Bet
// ---- -x-- Gamble Low
// ---- --x- Start
// ---- ---x high in test mode, coin lockout?
void es9606_state::watchdog_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (data & 0x7f01)
		logerror("%s unknown watchdog_w bits written %04x\n", machine().describe_context(), data);

	if (ACCESSING_BITS_8_15)
	{
		if (BIT(data, 15))
			m_watchdog->reset_w();
	}
}


void es9606_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x17ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x400000, 0x400003).mirror(0x00000c).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff); // r/w mirror if reset from win screen
	map(0x600000, 0x600001).portr("IN0").w(FUNC(es9606_state::eeprom_w));
	map(0x600002, 0x600003).portr("IN1").w(FUNC(es9606_state::watchdog_w));
	map(0x600004, 0x600005).portr("DSW");
	map(0x600006, 0x600007).portr("IN3");
	map(0xf00000, 0xf0ffff).mirror(0x0f0000).ram().share("nvram"); // dword access at $fef172 on win screen, assume same mirror as vmetal
}


static INPUT_PORTS_START( keirind2 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) // left cursor / red for D_UP sexy
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Take Score")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double Up / Bet All")
	PORT_DIPNAME( 0x20, 0x20, "IN0" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) // right cursor / white for D_UP sexy
	PORT_DIPNAME( 0x0100, 0x0100, "IN0-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_CODE(KEYCODE_8) // Analyzer
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Service A22")
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) // Service B35 according to sound test, however doesn't work for main test menu?

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "IN1-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 ) // doesn't have an assigned SFX sample, why?
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Settings Menu")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Service B22")
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service A35") // adds service coins in-game

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Control Panel") PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "Slot Game 2" )
	PORT_DIPSETTING(    0x01, "Key Matrix" ) // "Matorix" (sic)
	PORT_DIPSETTING(    0x02, "Joystick" )
	PORT_DIPSETTING(    0x03, "Slot Game 1" )
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPNAME( 0x10, 0x10, "Boot in Stealth Game") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: rename label
	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "IN3-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	// enables debug counters during races, first two looks 1st and 2nd position minus 1
	// i.e. (0004 0003 -> 5 wins 4 gets 2nd -> 4-5)
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

void es9606_state::es9606(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // divider not verified, but CPU is rated for 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &es9606_state::program_map);
//  m_maincpu->set_vblank_int("screen", FUNC(es9606_state::irq1_line_hold));

	EEPROM_93C46_16BIT(config, m_eeprom); // exact model unknown

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// hand-tuned timing
	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(100));

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1000));

	I4220(config, m_vdp2, 26.666_MHz_XTAL);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_1);

	m_vdp2->set_vblank_irq_level(0);
	// blitter irq unused by the game, assume same config as vmetal
	m_vdp2->set_blit_irq_level(2);

	m_vdp2->set_tmap_xoffsets(0,0,0);
	m_vdp2->set_tmap_yoffsets(0,0,0);
	m_vdp2->set_tmap_flip_xoffsets(72,72,72);
	m_vdp2->set_tmap_flip_yoffsets(39,39,39);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: copied verbatim from vmetal config, unverified
	m_screen->set_refresh_hz(58.2328); // VSync 58.2328Hz, HSync 15.32kHz
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1500));
	m_screen->set_size(392, 263);
	m_screen->set_visarea(0, 320-1, 0, 224-1);
	m_screen->set_screen_update("vdp2", FUNC(imagetek_i4220_device::screen_update));
	m_screen->screen_vblank().set([this] (int state) {
		if (state)
		{
			m_vdp2->screen_eof(state);
		}
	});

	SPEAKER(config, "speaker", 2).front();

	// Unverified clock, sounds reasonable
	ymz280b_device &ymz(YMZ280B(config, "ymz", 32_MHz_XTAL / 2));
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}


ROM_START( keirind2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.u57", 0x00000, 0x80000, CRC(eecbf885) SHA1(fb46b8f24530c08d0b865aa005640ff54ba74ab0) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "5.u59", 0x00001, 0x80000, CRC(da8dabc0) SHA1(e2c055760fa6d6c6022258de3e2292f3e9d409e6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "vdp2", 0 )
	ROM_LOAD64_WORD( "d23c16000wcz_r52.1.u9",  0x000004, 0x200000, CRC(4e59db08) SHA1(4b79975cd876013d12bb1447c506613098295496) )
	ROM_LOAD64_WORD( "d23c16000wcz_r50.2.u18", 0x000000, 0x200000, CRC(3cdb7b33) SHA1(413ca4f711494ff157fa05543d2fe7bd70599983) )
	ROM_LOAD64_WORD( "d23c16000wcz_r53.3.u3",  0x000006, 0x200000, CRC(2f3900f2) SHA1(513a5b128009973ed1e488295cecf629729b2b0a) )
	ROM_LOAD64_WORD( "d23c16000wcz_r51.4.u14", 0x000002, 0x200000, CRC(9c1b84ae) SHA1(1540f3c7ace06f00ff25d6c4d9c89235cc7221d2) )

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "d23c16000wcz_r49.u52", 0x000000, 0x200000, CRC(cc16a9ed) SHA1(c5468cb69eedd3b5a8b1ef20d2cfa34043a43373) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u31", 0x00, 0x80, CRC(443d9c87) SHA1(2ad83f1c8384612f981aef48943a319ff0fe23ca) )
ROM_END

} // anonymous namespace


GAME( 1997, keirind2, 0, es9606, keirind2, es9606_state, empty_init, ROT0, "Cadence Technology / Excellent System / Woo Lim Electronics", "Keirin Derby II", MACHINE_SUPPORTS_SAVE ) // 1997/06/18
