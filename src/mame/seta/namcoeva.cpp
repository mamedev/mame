// license:BSD-3-Clause
// copyright-holders:

/*
Hammer Champ
Namco 1997
https://www.youtube.com/watch?v=gs1pAe5S0gY

main board:
NAMCO EVA PCB 8826960100 - P0-135B
with
- TMP68301AF-16 (main)
- 3x NEC DX-102
- Hitachi H8/3002 (sound)
- Namco C352
- Namco C416
- MACH111 PLD (KC026)
- 2x banks of 8 DIP switches
- 50 MHz XTAL
- 26.670 MHz XTAL (possibly for the video chips)

I/O board:
NAMCO M136 I/O PCB - hi-pric P41 B - 1423961101 (1423971101)
with
- Motorola MC68HC11K1
- Fuji MB8422 DPRAM
- 8 MHz XTAL

On real hardware, the game runs without IO board to test mode but will not go in game (error 07)


TODO:
- currently starts with 9 credits inserted. After entering and exiting test mode, the game shows 0
  coins and can be coined up normally;
- implement proper controls. The game has a peculiar input setup (see video link above);
- sound system is the same as namco/namcond1.cpp (puts "Quattro Ver.1.2.H8" in H8 RAM). Ir interacts
  with the keycus. Handling is copied over from said driver, but could probably be improved;
- after coining up there's a GFX bug that maybe points to some unimplemented feature in seta2_v.cpp;
- once the video emulation in seta/seta2_v.cpp has been devicified, remove derivation from
  seta/seta2.h and possibly move to namco/ folder.
*/


#include "emu.h"

#include "seta2.h"

#include "cpu/mc68hc11/mc68hc11.h"
#include "cpu/h8/h83002.h"
#include "machine/mb8421.h"
#include "machine/watchdog.h"
#include "sound/c352.h"

#include "speaker.h"


// configurable logging
#define LOG_MAINCPU   (1U << 1)
#define LOG_IOCPU     (1U << 2)
#define LOG_SUBCPU    (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_MAINCPU | LOG_IOCPU | LOG_SUBCPU)

#include "logmacro.h"

#define LOGMAINCPU(...)   LOGMASKED(LOG_MAINCPU,   __VA_ARGS__)
#define LOGIOCPU(...)     LOGMASKED(LOG_IOCPU,     __VA_ARGS__)
#define LOGSUBCPU(...)    LOGMASKED(LOG_SUBCPU,    __VA_ARGS__)


namespace {

class namcoeva_state : public seta2_state
{
public:
	namcoeva_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta2_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu"),
		m_iocpu(*this, "iocpu")
	{ }

	void hammerch(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<h83002_device> m_subcpu;
	required_device<mc68hc11k1_device> m_iocpu;

	void maincpu_map(address_map &map) ATTR_COLD;
	void subcpu_map(address_map &map) ATTR_COLD;
	void iocpu_map(address_map &map) ATTR_COLD;

	uint8_t m_h8_irq5_enabled = 0;

	uint16_t keycus_r(offs_t offset);
	void keycus_w(offs_t offset, uint16_t data);
	INTERRUPT_GEN_MEMBER(mcu_interrupt);
};


void namcoeva_state::machine_start()
{
	save_item(NAME(m_h8_irq5_enabled));
}

void namcoeva_state::machine_reset()
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_h8_irq5_enabled = 0;
}


uint16_t namcoeva_state::keycus_r(offs_t offset)
{
	switch (offset)
	{
		// this address returns a jump vector inside ISR2
		// - if zero then the ISR returns without jumping
		case (0x2e >> 1):
			return 0x0000;
		case (0x30 >> 1):
			return 0x0000;

		default:
			return 0;
	}
}

void namcoeva_state::keycus_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case (0x0a >> 1):
			// this is a kludge
			if ((m_h8_irq5_enabled == 0) && (data != 0x0000))
			{
				m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			m_h8_irq5_enabled = (data != 0x0000);
			break;

		default:
			break;
	}
}

INTERRUPT_GEN_MEMBER(namcoeva_state::mcu_interrupt)
{
	if (m_h8_irq5_enabled)
	{
		device.execute().pulse_input_line(5, device.execute().minimum_quantum_time());
	}
}


void namcoeva_state::maincpu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x210000, 0x21002f).ram(); // ??
	map(0x300000, 0x3001ff).ram(); // ??
	map(0x400000, 0x40ffff).ram().share("sharedram");
	map(0x600000, 0x600001).portr("IN0");
	map(0x600002, 0x600003).portr("IN1");
	map(0x600004, 0x600005).portr("IN2");
	map(0x600006, 0x600007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0x600200, 0x600201).nopw(); // lamps?
	map(0x600300, 0x600301).portr("DSW1");
	map(0x600302, 0x600303).portr("DSW2");
	map(0x800000, 0x800fff).rw("dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w)).umask16(0x00ff); // EXT IO CHECK: NG if unmapped
	map(0xa00000, 0xa3ffff).ram().share(m_spriteram);
	map(0xa40000, 0xa4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa60000, 0xa6003f).ram().w(FUNC(namcoeva_state::vregs_w)).share(m_vregs);
	map(0xc3ff00, 0xc3ffff).rw(FUNC(namcoeva_state::keycus_r), FUNC(namcoeva_state::keycus_w));
}

void namcoeva_state::subcpu_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x20ffff).ram().share("sharedram");
	map(0xa00000, 0xa07fff).rw("c352", FUNC(c352_device::read), FUNC(c352_device::write));
}

void namcoeva_state::iocpu_map(address_map &map)
{
	map(0x1000, 0x17ff).rw("dpram", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w));
	map(0x8000, 0xffff).rom();
}


static INPUT_PORTS_START( hammerch )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) // Photo Sensor - High (Game) in test mode
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) // "
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) // "
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) // "
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
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) // Photo Sensor - Low (Game) in test mode (also used to move and select in test mode)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 ) // "
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) // "
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) // "
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
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5) // Coin Switch 1 in test mode
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5) // Coin Switch 2 in test mode, doesn't have any effect in game
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Service Switch in test mode
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW ) // Test Switch in test mode
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

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC(         0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME(        0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(             0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(             0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(             0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(             0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(             0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(             0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0080, 0x0080, "Freeze Screen" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(             0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_BIT(                    0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPNAME(        0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(             0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(             0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(             0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(             0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(             0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(             0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(             0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_DIPNAME(        0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(             0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(             0x0000, DEF_STR( On ) )
	PORT_BIT(                    0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IO_IN0") // TODO: Photo Sensor - Low (I/O) and High (I/O)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(7*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};


/*  Tiles are 8bpp, but the hardware is additionally able to discard
    some bitplanes and use the low 4 bits only, or the high 4 bits only */
static GFXDECODE_START( gfx_dx_10x )
	GFXDECODE_ENTRY( "sprites", 0, tile_layout, 0, 0x8000 / 16 )   // 8bpp, but 4bpp color granularity
GFXDECODE_END


void namcoeva_state::hammerch(machine_config &config)
{
	tmp68301_device &maincpu(TMP68301(config, m_maincpu, 50_MHz_XTAL / 4)); // TODO: clock and divider not verified
	maincpu.set_addrmap(AS_PROGRAM, &namcoeva_state::maincpu_map);
	maincpu.parallel_r_cb().set([this] () { LOGMAINCPU("%s: P4 read\n", machine().describe_context()); return uint16_t(0); });
	maincpu.parallel_w_cb().set([this] (uint8_t data) { LOGMAINCPU("%s: P4 write %04x\n", machine().describe_context(), data); });
	maincpu.tx0_handler().set([this] (int state) { LOGMAINCPU("%s: tx0 line write %d\n", machine().describe_context(), state); });
	maincpu.tx1_handler().set([this] (int state) { LOGMAINCPU("%s: tx1 line write %d\n", machine().describe_context(), state); });
	maincpu.tx2_handler().set([this] (int state) { LOGMAINCPU("%s: tx2 line write %d\n", machine().describe_context(), state); });

	WATCHDOG_TIMER(config, "watchdog");

	H83002(config, m_subcpu, 50_MHz_XTAL / 3); // TODO: clock and divider not verified
	m_subcpu->set_addrmap(AS_PROGRAM, &namcoeva_state::subcpu_map);
	m_subcpu->set_vblank_int("screen", FUNC(namcoeva_state::mcu_interrupt));
	// seems to only use P4 read at start up
	m_subcpu->read_port4().set([this] () { LOGSUBCPU("%s: P4 read\n", machine().describe_context()); return u8(0); });
	m_subcpu->write_port4().set([this] (u8 data) { LOGSUBCPU("%s: P4 write %02x\n", machine().describe_context(), data); });
	m_subcpu->read_port6().set([this] () { LOGSUBCPU("%s: P6 read\n", machine().describe_context()); return u8(0); });
	m_subcpu->write_port6().set([this] (u8 data) { LOGSUBCPU("%s: P6 write %02x\n", machine().describe_context(), data); });
	m_subcpu->read_port7().set([this] () { LOGSUBCPU("%s: P7 read\n", machine().describe_context()); return u8(0); });
	m_subcpu->read_port8().set([this] () { LOGSUBCPU("%s: P8 read\n", machine().describe_context()); return u8(0); });
	m_subcpu->write_port8().set([this] (u8 data) { LOGSUBCPU("%s: P8 write %02x\n", machine().describe_context(), data); });
	m_subcpu->read_port9().set([this] () { LOGSUBCPU("%s: P9 read\n", machine().describe_context()); return u8(0); });
	m_subcpu->write_port9().set([this] (u8 data) { LOGSUBCPU("%s: P9 write %02x\n", machine().describe_context(), data); });
	m_subcpu->read_porta().set([this] () { LOGSUBCPU("%s: PA read\n", machine().describe_context()); return u8(0); });
	m_subcpu->write_porta().set([this] (u8 data) { LOGSUBCPU("%s: PA write %02x\n", machine().describe_context(), data); });
	m_subcpu->read_portb().set([this] () { LOGSUBCPU("%s: PB read\n", machine().describe_context()); return u8(0); });
	m_subcpu->write_portb().set([this] (u8 data) { LOGSUBCPU("%s: PB write %02x\n", machine().describe_context(), data); });
	m_subcpu->tend0().set([this] (int state) { LOGSUBCPU("%s: tend0 line write %d\n", machine().describe_context(), state); });
	m_subcpu->tend1().set([this] (int state) { LOGSUBCPU("%s: tend1 line write %d\n", machine().describe_context(), state); });

	MC68HC11K1(config, m_iocpu, 8_MHz_XTAL);
	m_iocpu->set_addrmap(AS_PROGRAM, &namcoeva_state::iocpu_map);
	m_iocpu->in_pa_callback().set([this] () { LOGIOCPU("%s: PA read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_pb_callback().set([this] () { LOGIOCPU("%s: PB read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_pc_callback().set([this] () { LOGIOCPU("%s: PC read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_pd_callback().set([this] () { LOGIOCPU("%s: PD read\n", machine().describe_context()); return u8(0); }); // read often
	m_iocpu->in_pe_callback().set([this] () { LOGIOCPU("%s: PE read\n", machine().describe_context()); return u8(0); }); // read often, Photo Sensor - Low (I/O) and High (I/O) in test mode react from here, but not correctly right now
	m_iocpu->in_pf_callback().set([this] () { LOGIOCPU("%s: PF read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_pg_callback().set([this] () { LOGIOCPU("%s: PG read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_ph_callback().set([this] () { LOGIOCPU("%s: PH read\n", machine().describe_context()); return u8(0); }); // read often
	m_iocpu->out_pa_callback().set([this] (u8 data) { LOGIOCPU("%s: PA write %02x\n", machine().describe_context(), data); });
	m_iocpu->out_pb_callback().set([this] (u8 data) { LOGIOCPU("%s: PB write %02x\n", machine().describe_context(), data); });
	m_iocpu->out_pc_callback().set([this] (u8 data) { LOGIOCPU("%s: PC write %02x\n", machine().describe_context(), data); });
	m_iocpu->out_pd_callback().set([this] (u8 data) { LOGIOCPU("%s: PD write %02x\n", machine().describe_context(), data); }); // very rarely written (bit 2)
	m_iocpu->out_pe_callback().set([this] (u8 data) { LOGIOCPU("%s: PE write %02x\n", machine().describe_context(), data); });
	m_iocpu->out_pf_callback().set([this] (u8 data) { LOGIOCPU("%s: PF write %02x\n", machine().describe_context(), data); });
	m_iocpu->out_pg_callback().set([this] (u8 data) { LOGIOCPU("%s: PG write %02x\n", machine().describe_context(), data); }); // bits 0, 4, 5 and 6 written often
	m_iocpu->out_ph_callback().set([this] (u8 data) { LOGIOCPU("%s: PH write %02x\n", machine().describe_context(), data); }); // writes 0xa0 at start up
	m_iocpu->in_an0_callback().set([this] () { LOGIOCPU("%s: AN0 read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_an1_callback().set([this] () { LOGIOCPU("%s: AN1 read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_an2_callback().set([this] () { LOGIOCPU("%s: AN2 read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_an3_callback().set([this] () { LOGIOCPU("%s: AN3 read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_an4_callback().set([this] () { LOGIOCPU("%s: AN4 read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_an5_callback().set([this] () { LOGIOCPU("%s: AN5 read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_an6_callback().set([this] () { LOGIOCPU("%s: AN6 read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_an7_callback().set([this] () { LOGIOCPU("%s: AN7 read\n", machine().describe_context()); return u8(0); });
	m_iocpu->in_spi2_data_callback().set([this] () { LOGIOCPU("%s: SPI2 data read\n", machine().describe_context()); return u8(0); });
	m_iocpu->out_spi2_data_callback().set([this] (u8 data) { LOGIOCPU("%s: SPI2 data write %02x\n", machine().describe_context(), data); });

	MB8421(config, "dpram"); // actually MB8422

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(0x200, 0x100);
	m_screen->set_visarea(0x00, 0x140-1, 0x00, 0xe0-1);
	m_screen->set_screen_update(FUNC(namcoeva_state::screen_update));
	m_screen->screen_vblank().set(FUNC(namcoeva_state::screen_vblank));
	m_screen->screen_vblank().append_inputline(m_maincpu, 0);
	m_screen->set_palette(m_palette);
	//m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dx_10x);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x8000+0xf0);    // extra 0xf0 because we might draw 256-color object with 16-color granularity

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	c352_device &c352(C352(config, "c352", 50_MHz_XTAL / 2, 288)); // TODO: clock and divider not verified
	c352.add_route(0, "lspeaker", 1.00);
	c352.add_route(1, "rspeaker", 1.00);
	c352.add_route(2, "lspeaker", 1.00);
	c352.add_route(3, "rspeaker", 1.00);
}


ROM_START( hammerch )
	ROM_REGION( 0x200000, "maincpu", 0 ) // TMP68301
	ROM_LOAD16_WORD( "hc1_main0.u02", 0x00000, 0x80000, CRC(150164bb) SHA1(c99f03718fd1002386bfbf8695b7010ec5dad168) )

	ROM_REGION( 0x80000, "subcpu", 0 ) // H8/3002
	ROM_LOAD( "hc1_sub0.u47", 0x00000, 0x80000, CRC(4762451a) SHA1(b46bf1eaeac317264eb80c2e3f50d2821791569f) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "iocpu", 0 ) // MC68HC11K1
	ROM_LOAD( "hc1prg0_ioboard.ic2", 0x00000, 0x10000, CRC(606fad7e) SHA1(cf975e09038f84eb75d30130bfaf31f4c74986b2) )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD64_WORD( "hc1_cg3.u29", 0x000000, 0x400000, CRC(d9cc519b) SHA1(43670b357fadc5faf5ee229176ae97e8c29b6874) )
	ROM_LOAD64_WORD( "hc1_cg1.u28", 0x000002, 0x400000, CRC(386c129e) SHA1(21325b44d96b184df09118cd0c68733afde6db03) )
	ROM_LOAD64_WORD( "hc1_cg2.u31", 0x000004, 0x400000, CRC(3aea6bf0) SHA1(a8e7a7fae0ab83b08b80ff28461c9e97afe85cf7) )
	ROM_LOAD64_WORD( "hc1_cg0.u30", 0x000006, 0x400000, CRC(f1685224) SHA1(94f491d92e91cc0040b67a82849f2832629459b0) )

	ROM_REGION( 0x400000, "c352", 0 )
	ROM_LOAD( "hc1_wave0.u51", 0x000000, 0x400000, CRC(a1546af4) SHA1(983a18e61696ed906d2ad1594f01527097742f98) )
ROM_END

} // anonymous namespace


GAME( 1997, hammerch, 0, hammerch, hammerch, namcoeva_state, empty_init, ROT0, "Namco", "Hammer Champ (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
