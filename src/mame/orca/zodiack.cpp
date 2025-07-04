// license:BSD-3-Clause
// copyright-holders: Zsolt Vasvari

/***************************************************************************

Zodiack/Dogfight (c) 1983 Orca

driver by Zsolt Vasvari

Notes:
- Moguchan and The Percussor triggers this code:
  2DD6: D1            pop  de
  2DD7: D1            pop  de
  2DD8: D1            pop  de
  2DD9: D1            pop  de
  2DDA: D1            pop  de
  2DDB: D1            pop  de
  2DDC: 32 00 70      ld   ($7000),a
  2DDF: 0B            dec  bc
  2DE0: 78            ld   a,b
  2DE1: B1            or   c
  2DE2: 20 F2         jr   nz,$2DD6
  2DE4: E9            jp   (hl)
  All it does is scanning the whole 64k z80 space via all those pop opcodes ...
  DE register values are always discarded ... bug in coding or ROM patch?

TODO:
- improve video emulation (especially moguchan colors)
- where do the sound NMIs come from exactly?
- can eventually be merged with orca/espial.cpp

============================================================================

Zodiack
Orca, 1983

PCB Layouts
-----------

Top PCB                             Middle PCB

 |---------------------|             |---------------------|
 |     HM4334     DIP24|             |             OVG30C.6|
|-|    HM4334          |            |-|              DIP24 |
| |            OVG20C.1|            | |    6116      DIP24 |
| |                    |            | |              DIP24 |
| |               Z80  |            | |            OVG30C.3|
| |                    |            | |            OVG30C.2|
|-|                    |            |-|                    |
 |             AY3-8910|             |                     |
 |                     |             |                     |
 |2                    |             |                     |
 |2           DSW1 DSW2|             |                     |
 |W                    |             |            Z80      |
 |A  555 LM324         |             |                     |
 |Y  M51516   VOL      |             |            18.432MHz|
 |---------------------|             |---------------------|
Notes:                              Notes:
      Z80      - clock 18.432/6           Z80   - clock 18.432/6
      AY3-8910 - clock 18.432/12          6116  - 2k x8 SRAM
      HM4334   - 1k x4 SRAM               DIP24 - Empty socket
      DIP24    - Empty socket             ROM 2/3 - 2764
      ROM 1    - 2716                     ROM 6   - 2732
      VSync    - 60.5900Hz
      HSync    - 15.5143kHz


Bottom PCB

 |------------------------------------------|
 |    2114          OVG40C.7                |
|-|   2114                         OVG40C.9 |
| |                   2114                  |
| |                   2114         OVG40C.8 |
| |                                         |
| |        2114                             |
|-|        2114                             |
 |                                          |
 |                                          |
 |                           93422          |
 |         OVG40C.2B                        |
 |         OVG40C.2A                        |
 |------------------------------------------|
Notes:
      2114  - 1k x4 SRAM
      93422 - 256 x4 SRAM
      ALL ROMs 2732
      ALL PROMs MMI 6331

Bounty2:
- First 0x100 bytes of the first ROM contain a screen that appears if the protection fails.
- The PCB uses a large CPU epoxy module marked "CPU PACKII". A battery can be spotted through the epoxy.
- If you copy the first 0x100 bytes from "bounty" then the game works.
- Therefore, it can be surmised that the epoxy blob contains a static ram or similar with
  the first 256 bytes of the real game's ROM, for as long as the battery lasts.

***************************************************************************/

#include "emu.h"

#include "orca40c.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "speaker.h"


namespace {

class zodiack_state : public driver_device
{
public:
	zodiack_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch%u", 0),
		m_aysnd(*this, "aysnd")
	{ }

	void zodiack(machine_config &config);
	void percuss(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void nmi_mask_w(uint8_t data);
	void irq_mask_w(uint8_t data);
	void sound_nmi_enable_w(uint8_t data);
	void control_w(uint8_t data);
	void porta_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// devices
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_device<ay8910_device> m_aysnd;

	// state
	uint8_t m_main_nmi_enabled = 0;
	uint8_t m_main_irq_enabled = 0;
	uint8_t m_sound_nmi_enabled = 0;
	uint8_t m_sound_nmi_freq = 0;
	emu_timer *m_sound_nmi_timer;

	TIMER_CALLBACK_MEMBER(sound_nmi_gen);
	void vblank(int state);

	void io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void zodiack_state::machine_start()
{
	save_item(NAME(m_main_nmi_enabled));
	save_item(NAME(m_main_irq_enabled));
	save_item(NAME(m_sound_nmi_enabled));
	save_item(NAME(m_sound_nmi_freq));

	m_sound_nmi_timer = timer_alloc(FUNC(zodiack_state::sound_nmi_gen), this);
}

void zodiack_state::machine_reset()
{
	m_main_nmi_enabled = 0;
	m_main_irq_enabled = 0;
	m_sound_nmi_enabled = 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void zodiack_state::nmi_mask_w(uint8_t data)
{
	m_main_nmi_enabled = (data & 1) ^ 1;
}

void zodiack_state::irq_mask_w(uint8_t data)
{
	m_main_irq_enabled = data & 1;
}

void zodiack_state::sound_nmi_enable_w(uint8_t data)
{
	m_sound_nmi_enabled = data & 1;
}

void zodiack_state::vblank(int state)
{
	if (state && m_main_nmi_enabled)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	if (state && m_main_irq_enabled)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

TIMER_CALLBACK_MEMBER(zodiack_state::sound_nmi_gen)
{
	if (m_sound_nmi_enabled)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void zodiack_state::control_w(uint8_t data)
{
	// Bit 0-1 - coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x01);
	// Bit 2 - ????
}

void zodiack_state::porta_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (mem_mask && data != m_sound_nmi_freq)
	{
		// sound NMI frequency
		m_sound_nmi_freq = data;
		attotime period;

		// dogfight writes 0xc0 and expects 4 NMIs per frame
		// others write 0xe0 and expect 8 NMIs per frame
		switch (data)
		{
			case 0:
				period = attotime::never;
				break;

			case 0xc0:
				period = m_screen->frame_period() / 4;
				break;

			case 0xe0:
				period = m_screen->frame_period() / 8;
				break;

			default:
				logerror("%s: unknown sound NMI frequency %02X", machine().describe_context(), data);
				return;
		}

		m_sound_nmi_timer->adjust(period, 0, period);
	}
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

void zodiack_state::main_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0x5800, 0x5fff).ram();
	map(0x6081, 0x6081).portr("DSW0").w(FUNC(zodiack_state::control_w));
	map(0x6082, 0x6082).portr("DSW1");
	map(0x6083, 0x6083).portr("IN0");
	map(0x6084, 0x6084).portr("IN1");
	map(0x6090, 0x6090).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
	map(0x7000, 0x7000).rw("watchdog", FUNC(watchdog_timer_device::reset_r), FUNC(watchdog_timer_device::reset_w));
	map(0x7100, 0x7100).w(FUNC(zodiack_state::nmi_mask_w));
	map(0x7200, 0x7200).w("videopcb", FUNC(orca_ovg_40c_device::flipscreen_w));
	map(0x8000, 0x8000).w(FUNC(zodiack_state::irq_mask_w));
	map(0x9000, 0x903f).ram().w("videopcb", FUNC(orca_ovg_40c_device::attributes_w)).share("videopcb:attributeram");
	map(0x9040, 0x905f).ram().share("videopcb:spriteram");
	map(0x9060, 0x907f).ram().share("videopcb:bulletsram");
	map(0x9080, 0x93ff).ram();
	map(0xa000, 0xa3ff).ram().w("videopcb", FUNC(orca_ovg_40c_device::videoram_w)).share("videopcb:videoram");
	map(0xb000, 0xb3ff).ram().w("videopcb", FUNC(orca_ovg_40c_device::videoram2_w)).share("videopcb:videoram_2");
	map(0xc000, 0xcfff).rom();
}

void zodiack_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x4000, 0x4000).w(FUNC(zodiack_state::sound_nmi_enable_w));
	map(0x6000, 0x6000).r(m_soundlatch[0], FUNC(generic_latch_8_device::read)).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
}

void zodiack_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w(m_aysnd, FUNC(ay8910_device::address_data_w));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( zodiack )
	PORT_START("DSW0") // never read in this game
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x14, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, "2 Coins/1 Credit  3 Coins/2 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x20, "40000 70000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8") // Manual shows this one as Service Mode
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( dogfight )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) // most likely unused
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) // most likely unused
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x20, "40000 70000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( moguchan )
	PORT_START("DSW0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, "2 Coins/1 Credit  3 Coins/2 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x20, "40000 70000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL      // these are read, but are they
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )                   // ever used?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( percuss )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 100000" )
	PORT_DIPSETTING(    0x04, "20000 200000" )
	PORT_DIPSETTING(    0x08, "40000 100000" )
	PORT_DIPSETTING(    0x0c, "40000 200000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x03, "7" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
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

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( bounty )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20k 100k" )
	PORT_DIPSETTING(    0x10, "40k 100k" )
	PORT_DIPSETTING(    0x20, "20k 200k" )
	PORT_DIPSETTING(    0x30, "40k 200k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
INPUT_PORTS_END



/*************************************
 *
 *  Machine configs
 *
 *************************************/

void zodiack_state::zodiack(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &zodiack_state::main_map);

	Z80(config, m_audiocpu, 18.432_MHz_XTAL / 6);
	m_audiocpu->set_addrmap(AS_PROGRAM, &zodiack_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &zodiack_state::io_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER).screen_vblank().set(FUNC(zodiack_state::vblank));

	orca_ovg_40c_device &videopcb(ORCA_OVG_40C(config, "videopcb", 0));
	videopcb.set_screen("screen");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_audiocpu, 0);

	GENERIC_LATCH_8(config, m_soundlatch[1]);

	AY8910(config, m_aysnd, 18.432_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.50);
	m_aysnd->port_a_write_callback().set(FUNC(zodiack_state::porta_w));
}

void zodiack_state::percuss(machine_config &config)
{
	zodiack(config);

	orca_ovg_40c_device &videopcb(*subdevice<orca_ovg_40c_device>("videopcb"));
	videopcb.set_percuss_hardware(true);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( zodiack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ovg30c.2",   0x0000, 0x2000, CRC(a2125e99) SHA1(00ae4ed2c7b6895d2dc58aa2fc51c25b6428e4ba) )
	ROM_LOAD( "ovg30c.3",   0x2000, 0x2000, CRC(aee2b77f) SHA1(2581b7a75d38663cc5ebc91a77385ca7eb9b4aba) )
	ROM_LOAD( "ovg30c.6",   0x4000, 0x0800, CRC(1debb278) SHA1(98c9f09c5f3125ba8a1392be62fd469aa0ba4d98) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ovg20c.1",   0x0000, 0x1000, CRC(2d3c3baf) SHA1(e32937b947e591cba45da2dd456e4f655e62ddfd) )

	ROM_REGION( 0x2800, "videopcb", 0 )
	ROM_LOAD( "ovg40c.7",   0x0000, 0x0800, CRC(ed9d3be7) SHA1(80d5906afef8b6d68fb13d41992aea208d9e3690) )
	ROM_LOAD( "orca40c.8",  0x0800, 0x1000, CRC(88269c94) SHA1(acc27be0d27b33c2242ecf0563fe986e8dffb264) )
	ROM_LOAD( "orca40c.9",  0x1800, 0x1000, CRC(a3bd40c9) SHA1(dcf8cbb73c081a3af85da135e8278c54e9e0de7c) )

	ROM_REGION( 0x0040, "videopcb:proms", 0 )
	ROM_LOAD( "ovg40c.2a",  0x0000, 0x0020, CRC(703821b8) SHA1(33dcc9b0bea5e110eb4ffd3b8b8763e32e927b22) )
	ROM_LOAD( "ovg40c.2b",  0x0020, 0x0020, CRC(21f77ec7) SHA1(b1019afc4361aca98b7120b21743bfeb5ea2ff63) )
ROM_END

ROM_START( dogfight )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "df-2.4f",    0x0000, 0x2000, CRC(ad24b28b) SHA1(5bfc24c9d176a987525c5ad3eff3308f679d4d44) )
	ROM_LOAD( "df-3.4h",    0x2000, 0x2000, CRC(cd172707) SHA1(9d7a494006db13cbe9c895875a18d9423a0128bc) )
	ROM_LOAD( "df-5.4n",    0x4000, 0x1000, CRC(874dc6bf) SHA1(2c34fdfb2838c41f239171bc9a14a5cc7a94a170) )
	ROM_LOAD( "df-4.4m",    0xc000, 0x1000, CRC(d8aa3d6d) SHA1(0863d566ff4a181ae8a8d552f9768dd028254605) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "df-1.4n",    0x0000, 0x1000, CRC(dcbb1c5b) SHA1(51fa51ff64982455b00484d55f6a8cf89fc786f1) )

	ROM_REGION( 0x2800, "videopcb", 0 )
	ROM_LOAD( "df-6.3s",    0x0000, 0x0800, CRC(3059b515) SHA1(849e99a04ddcdfcf097cc3ac17e9edf12b51cd69) )
	// the ROM at 3s has been found also as a 0x1000 ROM with the second half filled with possibly leftover Z80 code (the dogfightp set has it preserved)
	ROM_LOAD( "df-7.7n",    0x0800, 0x1000, CRC(ffe05fee) SHA1(70b9d0808defd936e2c3567f8e6996a19753de81) )
	ROM_LOAD( "df-8.7r",    0x1800, 0x1000, CRC(2cb51793) SHA1(d90177ef28730774202a04a0846281537a1883df) )

	ROM_REGION( 0x0040, "videopcb:proms", 0 )
	ROM_LOAD( "mmi6331.2a", 0x0000, 0x0020, CRC(69a35aa5) SHA1(fe494ed1ff642f95834dfca92e9c4494e04f7b81) )
	ROM_LOAD( "mmi6331.2b", 0x0020, 0x0020, CRC(596ae457) SHA1(1c1a3130d88c5fd5c66ce9f91d97a09c0a0b535f) )
ROM_END

ROM_START( moguchan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.5r",        0x0000, 0x1000, CRC(85d0cb7e) SHA1(20066f71d80161dff556bc86edf40fcc2ac3b993) )
	ROM_LOAD( "4.5m",        0x1000, 0x1000, CRC(359ef951) SHA1(93e80fcd371e8d2026919d0e046b636b7c19002e) )
	ROM_LOAD( "3.5np",       0x2000, 0x1000, CRC(c8776f77) SHA1(edc28a6a804b9573307faa25b6fdd096b7093593) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.7hj",       0x0000, 0x1000, CRC(1a88d35f) SHA1(d9347723c0eb6508739a6c0de4984b8244b197cf) )

	ROM_REGION( 0x2800, "videopcb", 0 )
	ROM_LOAD( "5.4r",        0x0000, 0x0800, CRC(1b7febd8) SHA1(4f9ce99f05e6e207bb91f831b6bee7cf72b3d05b) )
	ROM_LOAD( "6.7p",        0x0800, 0x1000, CRC(c8060ffe) SHA1(f1f975c2638e6cdf00af4a96591529c7b6684742) )
	ROM_LOAD( "7.7m",        0x1800, 0x1000, CRC(bfca00f4) SHA1(797b07bab2467fe00cd85cd4477db2367a3e5a40) )

	ROM_REGION( 0x0040, "videopcb:proms", 0 )
	ROM_LOAD( "moguchan.2a", 0x0000, 0x0020, CRC(e83daab3) SHA1(58b38091dfbc3f3b4ddf6c6febd98c909be89063) )
	ROM_LOAD( "moguchan.2b", 0x0020, 0x0020, CRC(9abfdf40) SHA1(44c4dcdd3d79af2c4a897cc003b5287dece0313e) )
ROM_END

ROM_START( percuss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "percuss.1",    0x0000, 0x1000, CRC(ff0364f7) SHA1(048963d70e513068fdb591b4bc152473fe4cc2c3) )
	ROM_LOAD( "percuss.3",    0x1000, 0x1000, CRC(7f646c59) SHA1(976210f1fed11c03e0a159c8189630a1fec63fc9) )
	ROM_LOAD( "percuss.2",    0x2000, 0x1000, CRC(6bf72dd2) SHA1(447109a508a571650501d3dd3fa018513b4e4558) )
	ROM_LOAD( "percuss.4",    0x3000, 0x1000, CRC(fb1b15ba) SHA1(778853a0fbef7dd80d1046e2b04adcd9d1241f83) )
	ROM_LOAD( "percuss.5",    0x4000, 0x1000, CRC(8e5a9692) SHA1(17db644c3cd0435c380b2080bda02c6a6d45eaf6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "percuss.8",    0x0000, 0x0800, CRC(d63f56f3) SHA1(2cf9cca55fb612c9efae06e62bd6d33fdd38de57) )
	ROM_LOAD( "percuss.9",    0x0800, 0x0800, CRC(e08fef2f) SHA1(af41763f200c4c660ca7230eace4f443e57e809a) )

	ROM_REGION( 0x2800, "videopcb", 0 )
	ROM_LOAD( "percuss.10",   0x0000, 0x0800, CRC(797598aa) SHA1(6d26ed5e964b7f1b2ce651c7bf0168a730bee02f) )
	ROM_LOAD( "percuss.6",    0x0800, 0x1000, CRC(5285a580) SHA1(cd7ba64706458c67166934d0fa08894fb577810b) )
	ROM_LOAD( "percuss.7",    0x1800, 0x1000, CRC(8fc4175d) SHA1(0249f6324ae44142d7da2306674ed86afcccf5f2) )

	ROM_REGION( 0x0040, "videopcb:proms", 0 )
	ROM_LOAD( "percus2a.prm", 0x0000, 0x0020, CRC(e2ee9637) SHA1(e4ca064793ae1dc36b2d852448162d062a2f26f8) )
	ROM_LOAD( "percus2b.prm", 0x0020, 0x0020, CRC(e561b029) SHA1(7d21a3492a179f5ce541911d19e4816960547089) )
ROM_END

ROM_START( bounty )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.4f",      0x0000, 0x1000, CRC(f495b19d) SHA1(df2de0869b10da1ee1d98d48615c9e1dce798c26) )
	ROM_LOAD( "3.4k",      0x1000, 0x1000, CRC(fa3086c3) SHA1(b8bab26a4e68e6d2e5b899e900c9affd297c22de) )
	ROM_LOAD( "2.4h",      0x2000, 0x1000, CRC(52ab5314) SHA1(ba399f8b86cc64d1dd585dde79d8036d24296475) )
	ROM_LOAD( "4.4m",      0x3000, 0x1000, CRC(5c9d3f07) SHA1(340460f38c5b23c591f92d386c0df10ed75fa16f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "7.4n",      0x0000, 0x1000, CRC(45e369b8) SHA1(9799b2ece5e3b92da435255e1b49f5097d3f7972) )
	ROM_LOAD( "8.4r",      0x1000, 0x1000, CRC(4f52c87d) SHA1(1738d798c341a54d293c70da7b6e4a3dfb00de38) )

	ROM_REGION( 0x2800, "videopcb", 0 )
	ROM_LOAD( "9.4r",      0x0000, 0x0800, CRC(4b4acde5) SHA1(5ed60fe50a9ab0b8d433ee9ae5787b936ddbfbdd) )
	ROM_LOAD( "5.7m",      0x0800, 0x1000, CRC(a5ce2a24) SHA1(b5469d31bda52a61cdc46349c139a7eb339ac8a7) )
	ROM_LOAD( "6.7p",      0x1800, 0x1000, CRC(43183301) SHA1(9df89479396d7847ee3325649d7264e75d413add) )

	ROM_REGION( 0x0040, "videopcb:proms", 0 )
	ROM_LOAD( "mb7051.2a", 0x0000, 0x0020, CRC(0de11a46) SHA1(3bc81571832dd78b29654e86479815ee5f97a4d3) )
	ROM_LOAD( "mb7051.2b", 0x0020, 0x0020, CRC(465e31d4) SHA1(d47a4aa0e8931dcd8f85017ef04c2f6ad79f5725) )
ROM_END

ROM_START( bounty2 ) // The PCB uses a large CPU epoxy module marked "CPU PACKII". A battery can be spotted through the epoxy.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.4f",      0x0000, 0x1000, CRC(edc26413) SHA1(c5ac7362a0e0d98eca53b3cd76d28a705c7d6917) )
	ROM_LOAD( "3.4k",      0x1000, 0x1000, CRC(fa3086c3) SHA1(b8bab26a4e68e6d2e5b899e900c9affd297c22de) )
	ROM_LOAD( "2.4h",      0x2000, 0x1000, CRC(52ab5314) SHA1(ba399f8b86cc64d1dd585dde79d8036d24296475) )
	ROM_LOAD( "4.4m",      0x3000, 0x1000, CRC(c0afb93c) SHA1(f1bbcdf849fce464c897fbe3d96a77cb5fcb51fd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "7.4n",      0x0000, 0x1000, CRC(45e369b8) SHA1(9799b2ece5e3b92da435255e1b49f5097d3f7972) )
	ROM_LOAD( "8.4r",      0x1000, 0x1000, CRC(4f52c87d) SHA1(1738d798c341a54d293c70da7b6e4a3dfb00de38) )

	ROM_REGION( 0x2800, "videopcb", 0 )
	ROM_LOAD( "9.4r",      0x0000, 0x0800, CRC(4b4acde5) SHA1(5ed60fe50a9ab0b8d433ee9ae5787b936ddbfbdd) )
	ROM_LOAD( "5.7m",      0x0800, 0x1000, CRC(a5ce2a24) SHA1(b5469d31bda52a61cdc46349c139a7eb339ac8a7) )
	ROM_LOAD( "6.7p",      0x1800, 0x1000, CRC(43183301) SHA1(9df89479396d7847ee3325649d7264e75d413add) )

	ROM_REGION( 0x0040, "videopcb:proms", 0 )
	ROM_LOAD( "mb7051.2a", 0x0000, 0x0020, CRC(0de11a46) SHA1(3bc81571832dd78b29654e86479815ee5f97a4d3) )
	ROM_LOAD( "mb7051.2b", 0x0020, 0x0020, CRC(465e31d4) SHA1(d47a4aa0e8931dcd8f85017ef04c2f6ad79f5725) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, zodiack,  0,      zodiack, zodiack,  zodiack_state, empty_init, ROT270, "Orca (Esco Trading Co., Inc. license)", "Zodiack",                 MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE ) // bullet color needs to be verified
GAME( 1983, dogfight, 0,      zodiack, dogfight, zodiack_state, empty_init, ROT270, "Orca / Thunderbolt",                    "Dog Fight (Thunderbolt)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, moguchan, 0,      percuss, moguchan, zodiack_state, empty_init, ROT270, "Orca (Eastern Commerce Inc. license)",  "Mogu Chan (bootleg?)",    MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // license copyright taken from ROM string at $0b5c
GAME( 1981, percuss,  0,      percuss, percuss,  zodiack_state, empty_init, ROT270, "Orca",                                  "The Percussor",           MACHINE_SUPPORTS_SAVE )
GAME( 1982, bounty,   0,      percuss, bounty,   zodiack_state, empty_init, ROT180, "Orca",                                  "The Bounty (set 1)",      MACHINE_SUPPORTS_SAVE )
GAME( 1982, bounty2,  bounty, percuss, bounty,   zodiack_state, empty_init, ROT180, "Orca",                                  "The Bounty (set 2)",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // seems to use a different memory map
