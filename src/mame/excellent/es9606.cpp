// license:BSD-3-Clause
// copyright-holders:

/*
Excellent System's ES-9606-01 PCB

Main components:
TMP68HC000-P16 CPU
2x CY7C199-15PC RAM (near CPU)
32.000 MHz XTAL (near CPU)
ES9402LA (rebranded TC0090LVC?)
3x CY7C199-15PC RAM (near ES9402LA)
26.666 MHz XTAL (near ES9402LA)
Altera EPM7032LC44-15T CPLD
YMZ280B-F
empty space marked for YM2413
3.579545 MHz XTAL (near empty YM2413 space)
MM1035 Mitsumi System Reset IC with Built-in Watchdog Timer
bank of 8 DIP switches (3 other spaces not populated)
memory reset push-button

TODO
- decode GFX ROMs;
- everything else.
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
//#include "machine/nvram.h"
//#include "machine/ticket.h"
//#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ymz280b.h"

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
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void es9606(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void eeprom_w(uint16_t data);
	void watchdog_w(uint16_t data);

	void program_map(address_map &map) ATTR_COLD;
};


void es9606_state::video_start()
{
}

uint32_t es9606_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	return 0;
}


void es9606_state::eeprom_w(uint16_t data)
{
	if (data & 0xff1f)
		logerror("%s unknown eeprom_w bits written %04x\n", machine().describe_context(), data);

	m_eeprom->cs_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 7));
}

void es9606_state::watchdog_w(uint16_t data)
{
	if (data & 0x7fff)
		logerror("%s unknown watchdog_w bits written %04x\n", machine().describe_context(), data);

	if (BIT(data, 15))
		m_watchdog->reset_w();
}


void es9606_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x171fff).ram(); // tiles?
	map(0x172000, 0x173fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x174000, 0x177fff).ram(); // sprites?
	map(0x178000, 0x17ffff).ram(); // ??
	map(0x1788a3, 0x1788a3).lr8(NAME([this] () -> uint8_t { return m_screen->vblank() ? 0x00df : 0x00ff; })); // TODO: where does this come from?
	map(0x400000, 0x400003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);
	map(0x600000, 0x600001).portr("IN0").w(FUNC(es9606_state::eeprom_w));
	map(0x600002, 0x600003).portr("IN1").w(FUNC(es9606_state::watchdog_w));
	map(0x600004, 0x600005).portr("IN2");
	map(0x600006, 0x600007).portr("IN3");
	map(0xff0000, 0xffffff).ram(); // NVRAM
}


static INPUT_PORTS_START( keirind2 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
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
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
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

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
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

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
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
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("DSW") // TODO: identify where this is read (one of the hooked up IN% ports)
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


static GFXDECODE_START( gfx_es9606 )// TODO
GFXDECODE_END


void es9606_state::es9606(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // divider not verified, but CPU is rated for 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &es9606_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(es9606_state::irq1_line_hold));

	EEPROM_93C46_16BIT(config, m_eeprom); // exact model unknown

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(1000));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // TODO
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(es9606_state::screen_update));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_es9606);

	PALETTE(config, "palette").set_format(palette_device::RRRRGGGGBBBBRGBx, 0x1000); // TODO

	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 32_MHz_XTAL / 2)); // TODO: is this the correct XTAL?
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}


ROM_START( keirind2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.u57", 0x00000, 0x80000, CRC(eecbf885) SHA1(fb46b8f24530c08d0b865aa005640ff54ba74ab0) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "5.u59", 0x00001, 0x80000, CRC(da8dabc0) SHA1(e2c055760fa6d6c6022258de3e2292f3e9d409e6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "gfx", 0 ) // TODO: correct ROM loading
	ROM_LOAD( "d23c16000wcz_r52.1.u9",  0x000000, 0x200000, CRC(4e59db08) SHA1(4b79975cd876013d12bb1447c506613098295496) )
	ROM_LOAD( "d23c16000wcz_r50.2.u18", 0x200000, 0x200000, CRC(3cdb7b33) SHA1(413ca4f711494ff157fa05543d2fe7bd70599983) )
	ROM_LOAD( "d23c16000wcz_r53.3.u3",  0x400000, 0x200000, CRC(2f3900f2) SHA1(513a5b128009973ed1e488295cecf629729b2b0a) )
	ROM_LOAD( "d23c16000wcz_r51.4.u14", 0x600000, 0x200000, CRC(9c1b84ae) SHA1(1540f3c7ace06f00ff25d6c4d9c89235cc7221d2) )

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "d23c16000wcz_r49.u52", 0x000000, 0x200000, CRC(cc16a9ed) SHA1(c5468cb69eedd3b5a8b1ef20d2cfa34043a43373) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u31", 0x00, 0x80, CRC(443d9c87) SHA1(2ad83f1c8384612f981aef48943a319ff0fe23ca) )
ROM_END

} // anonymous namespace


GAME( 1997, keirind2, 0, es9606, keirind2, es9606_state, empty_init, ROT0, "Excellent System", "Keirin Derby 2", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
