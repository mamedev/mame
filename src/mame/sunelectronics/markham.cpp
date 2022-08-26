// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

    Markham (c) 1983 Sun Electronics
    Strength & Skill (c) 1984 Sun Electronics

    Driver by Uki

    TODO:
    - needs merging with ikki.cpp
    - look up schematics for all games
    - hook up actual SUN 8212 and figure out ROM mode communications

    Notes:
    Banbam has a Fujitsu MB8841 4-Bit MCU for protection labeled SUN 8212.
    Its internal ROM has been imaged, manually typed, and decoded as sun-8212.ic3.
    Pettan Pyuu is a clone of Banbam although with different levels / play fields.

    The MCU controls:
      - general protection startup
      - the time between when enemies spawn
      - graphics selection for playfields

*****************************************************************************/

#include "emu.h"
#include "markham.h"

#define MASTER_CLOCK (20_MHz_XTAL)
#define PIXEL_CLOCK  (MASTER_CLOCK/4) // guess
#define CPU_CLOCK    (8_MHz_XTAL)

/* also a guess */
#define HTOTAL       (320)
#define HBEND        (8)
#define HBSTART      (248)
#define VTOTAL       (262)
#define VBEND        (16)
#define VBSTART      (240)

uint8_t markham_state::markham_e004_r()
{
	return 0;
}

void markham_state::coin_output_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));

	// plain, flat out weird stuff needed to prevent phantom coins
	// likely an activation mechanism to test individual chute behavior?
	// this can't be boolean wise, because banbam triggers this three times
	if (!m_coin2_lock_cnt)
	{
		machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	}
	else if (BIT(data, 1))
	{
		m_coin2_lock_cnt--;
	}
}

void markham_state::flipscreen_w(uint8_t data)
{
	if (flip_screen() != (BIT(data, 0)))
	{
		flip_screen_set(BIT(data, 0));
		machine().tilemap().mark_all_dirty();
	}
}

/****************************************************************************/

uint8_t markham_state::strnskil_d800_r()
{
	// bit0: interrupt type?, bit1: CPU2 busack?
	return (m_irq_source);
}

void markham_state::strnskil_master_output_w(uint8_t data)
{
	m_scroll_ctrl = data >> 5;

	flipscreen_w((data >> 3) & 1);

	// bit 0: master CPU bus request?
}

TIMER_DEVICE_CALLBACK_MEMBER(markham_state::strnskil_scanline)
{
	int scanline = param;

	// same as Ikki, whereas if non-vblank IRQ isn't in-sync with slave CPU irqs then stage 2/3 lacks any sprite whatsoever.
	// TODO: maybe this is just a timer device that dispatches irqs to both CPUs and running at VSync*2. Verify on real HW or schematics.
	if (scanline == m_irq_scanline_end || scanline == m_irq_scanline_start)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);

		m_irq_source = (scanline != m_irq_scanline_end);
	}
}

/****************************************************************************/

uint8_t markham_state::banbam_protection_r()
{
	const uint8_t *prot_rom = (const uint8_t *)memregion("mcu_rom")->base();

	const uint8_t init = m_packet_buffer[0] & 0x0f;
	uint8_t comm = m_packet_buffer[1] & 0xf0;
	uint8_t arg = m_packet_buffer[1] & 0x0f;

	if (m_packet_reset)
	{
		// returning m_packet_buffer[0] breaks demo
		return 0xa5;
	}
	else if (init == 0x08 || init == 0x05)
	{
		switch (comm)
		{
		case 0x30:
			// palette/gfx select
			arg = prot_rom[0x799 + (arg * 4)];
			break;
		case 0x40:
			// palette/gfx select
			arg = prot_rom[0x7C5 + (arg * 4)];
			break;
		case 0x60:
			// enemy wave timer trigger
			// randomized for now
			arg = machine().rand();
			break;
		case 0x70:
			// ??
			arg++;
			break;
		case 0xb0:
			// ??
			arg = arg + 3;
			break;
		default:
			logerror("unknown command %02x, argument is %02x \n", comm, arg);
			arg = 0;
		}
		arg &= 0x0f;
	}
	else
	{
		comm = 0xf0;
		arg = 0x0f;
	}
	return comm | arg;
}

void markham_state::banbam_protection_w(uint8_t data)
{
	if (m_packet_write_pos)
	{
		m_packet_reset = false;
	}
	else
	{
		m_packet_reset = true;
	}

	m_packet_buffer[m_packet_write_pos] = data;
	m_packet_write_pos++;

	if (m_packet_write_pos > 1)
	{
		m_packet_write_pos = 0;
	}
	logerror("packet buffer is: %02x %02x, status: %s \n", m_packet_buffer[0], m_packet_buffer[1], m_packet_reset ? "reset" : "active" );
}

void markham_state::mcu_reset_w(uint8_t data)
{
	// clear or assert?
	logerror("reset = %02x \n", data);
}

/****************************************************************************/

void markham_state::base_master_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();

	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(markham_state::videoram_w)).share("videoram");
}

void markham_state::markham_master_map(address_map &map)
{
	base_master_map(map);

	map(0xc800, 0xcfff).ram().share("spriteram");
	map(0xd800, 0xdfff).ram().share("share1");

	map(0xe000, 0xe000).portr("DSW2");
	map(0xe001, 0xe001).portr("DSW1");
	map(0xe002, 0xe002).portr("P1");
	map(0xe003, 0xe003).portr("P2");

	map(0xe004, 0xe004).r(FUNC(markham_state::markham_e004_r)); /* from CPU2 busack */

	map(0xe005, 0xe005).portr("SYSTEM");

	map(0xe008, 0xe008).w(FUNC(markham_state::coin_output_w));
	map(0xe009, 0xe009).nopw(); /* to CPU2 busreq */

	map(0xe00c, 0xe00d).writeonly().share("xscroll");
	map(0xe00e, 0xe00e).w(FUNC(markham_state::flipscreen_w));
}

void markham_state::strnskil_master_map(address_map &map)
{
	base_master_map(map);

	map(0x6000, 0x9fff).rom();

	map(0xc800, 0xcfff).ram().share("share1");

	map(0xd800, 0xd800).r(FUNC(markham_state::strnskil_d800_r));
	map(0xd801, 0xd801).portr("DSW1");
	map(0xd802, 0xd802).portr("DSW2");
	map(0xd803, 0xd803).portr("SYSTEM");
	map(0xd804, 0xd804).portr("P1");
	map(0xd805, 0xd805).portr("P2");

	map(0xd808, 0xd808).w(FUNC(markham_state::strnskil_master_output_w));
	map(0xd809, 0xd809).w(FUNC(markham_state::coin_output_w));
	map(0xd80a, 0xd80b).writeonly().share("xscroll");
}

void markham_state::banbam_master_map(address_map &map)
{
	strnskil_master_map(map);
	map(0xd806, 0xd806).r(FUNC(markham_state::banbam_protection_r)); /* mcu data read */
	map(0xd80d, 0xd80d).w(FUNC(markham_state::banbam_protection_w)); /* mcu data write */
	map(0xd80c, 0xd80c).w(FUNC(markham_state::mcu_reset_w)); /* mcu reset? */
}

void markham_state::markham_slave_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x87ff).ram().share("share1");

	map(0xc000, 0xc000).w("sn1", FUNC(sn76496_device::write));
	map(0xc001, 0xc001).w("sn2", FUNC(sn76496_device::write));

	map(0xc002, 0xc002).nopw(); /* unknown */
	map(0xc003, 0xc003).nopw(); /* unknown */
}

void markham_state::strnskil_slave_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0xc000, 0xc7ff).ram().share("spriteram");
	map(0xc800, 0xcfff).ram().share("share1");

	map(0xd801, 0xd801).w("sn1", FUNC(sn76496_device::write));
	map(0xd802, 0xd802).w("sn2", FUNC(sn76496_device::write));
}

/****************************************************************************/

static INPUT_PORTS_START( markham )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Chutes" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPSETTING(    0x08, "Common" )
	PORT_DIPNAME( 0xf0, 0x00, "Coin1 / Coin2" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, "1C 1C / 1C 1C" )
	PORT_DIPSETTING(    0x10, "2C 1C / 2C 1C" )
	PORT_DIPSETTING(    0x20, "2C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x30, "1C 1C / 1C 2C" )
	PORT_DIPSETTING(    0x40, "1C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x50, "1C 1C / 1C 4C" )
	PORT_DIPSETTING(    0x60, "1C 1C / 1C 5C" )
	PORT_DIPSETTING(    0x70, "1C 1C / 1C 6C" )
	PORT_DIPSETTING(    0x80, "1C 2C / 1C 2C" )
	PORT_DIPSETTING(    0x90, "1C 2C / 1C 4C" )
	PORT_DIPSETTING(    0xa0, "1C 2C / 1C 5C" )
	PORT_DIPSETTING(    0xb0, "1C 2C / 1C 10C" )
	PORT_DIPSETTING(    0xc0, "1C 2C / 1C 11C" )
	PORT_DIPSETTING(    0xd0, "1C 2C / 1C 12C" )
	PORT_DIPSETTING(    0xe0, "1C 2C / 1C 6C" )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x02, "20000, Every 50000" )
	PORT_DIPSETTING(    0x03, "20000, Every 80000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" ) /* These next five dips are unused according to the manual */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1") /* e002 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2") /* e003 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SYSTEM") /* e005 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( strnskil )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Unknown 1-2" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Chutes" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPSETTING(    0x08, "Common" )
	PORT_DIPNAME( 0xf0, 0x00, "Coin1 / Coin2" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, "1C 1C / 1C 1C" )
	PORT_DIPSETTING(    0x10, "2C 1C / 2C 1C" )
	PORT_DIPSETTING(    0x20, "2C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x30, "1C 1C / 1C 2C" )
	PORT_DIPSETTING(    0x40, "1C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x50, "1C 1C / 1C 4C" )
	PORT_DIPSETTING(    0x60, "1C 1C / 1C 5C" )
	PORT_DIPSETTING(    0x70, "1C 1C / 1C 6C" )
	PORT_DIPSETTING(    0x80, "1C 2C / 1C 2C" )
	PORT_DIPSETTING(    0x90, "1C 2C / 1C 4C" )
	PORT_DIPSETTING(    0xa0, "1C 2C / 1C 5C" )
	PORT_DIPSETTING(    0xb0, "1C 2C / 1C 10C" )
	PORT_DIPSETTING(    0xc0, "1C 2C / 1C 11C" )
	PORT_DIPSETTING(    0xd0, "1C 2C / 1C 12C" )
	PORT_DIPSETTING(    0xe0, "1C 2C / 1C 6C" )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, "Unknown 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Unknown 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Unknown 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Unknown 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Unknown 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Unknown 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1") /* d804 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2") /* d805 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SYSTEM") /* d803 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( banbam )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME(0x08,  0x00, "Coin Chutes") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Individual")
	PORT_DIPSETTING(    0x08, "Common")
	PORT_DIPNAME( 0xf0, 0x00, "Coin1 / Coin2" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, "1C 1C / 1C 1C" )
	PORT_DIPSETTING(    0x10, "2C 1C / 2C 1C" )
	PORT_DIPSETTING(    0x20, "2C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x30, "1C 1C / 1C 2C" )
	PORT_DIPSETTING(    0x40, "1C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x50, "1C 1C / 1C 4C" )
	PORT_DIPSETTING(    0x60, "1C 1C / 1C 5C" )
	PORT_DIPSETTING(    0x70, "1C 1C / 1C 6C" )
	PORT_DIPSETTING(    0x80, "1C 2C / 1C 2C" )
	PORT_DIPSETTING(    0x90, "1C 2C / 1C 4C" )
	PORT_DIPSETTING(    0xa0, "1C 2C / 1C 5C" )
	PORT_DIPSETTING(    0xb0, "1C 2C / 1C 10C" )
	PORT_DIPSETTING(    0xc0, "1C 2C / 1C 11C" )
	PORT_DIPSETTING(    0xd0, "1C 2C / 1C 12C" )
	PORT_DIPSETTING(    0xe0, "1C 2C / 1C 6C" )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x02, "20000 80000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x06, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, "Second Practice" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" ) /* These four dips are unused according to the manual */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" ) PORT_DIPLOCATION("SW2:8") // game stands in a tight loop at $14-$16 -> $866 if this is putted off
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1") /* d804 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_START("P2") /* d805 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("SYSTEM") /* d803 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	3,      /* 3 bits per pixel */
	{0,8192*8,8192*8*2},
	{7,6,5,4,3,2,1,0},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 characters */
	256,    /* 256 characters */
	3,      /* 3 bits per pixel */
	{8192*8*2,8192*8,0},
	{7,6,5,4,3,2,1,0,
		8*16+7,8*16+6,8*16+5,8*16+4,8*16+3,8*16+2,8*16+1,8*16+0},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7,
		8*8,8*9,8*10,8*11,8*12,8*13,8*14,8*15},
	8*8*4
};

static GFXDECODE_START( gfx_markham )
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout,   512, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0,   64 )
GFXDECODE_END

void markham_state::machine_start()
{
	save_item(NAME(m_coin2_lock_cnt));

	/* banbam specific */
	save_item(NAME(m_packet_buffer));
	save_item(NAME(m_packet_reset));
	save_item(NAME(m_packet_write_pos));
}

void markham_state::machine_reset()
{
	/* prevent phantom coins again */
	m_coin2_lock_cnt = 3;

	/* banbam specific */
	m_packet_write_pos = 0;
	m_packet_reset = true;
}

void markham_state::markham(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, CPU_CLOCK/2); /* 4.000MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &markham_state::markham_master_map);
	m_maincpu->set_vblank_int("screen", FUNC(markham_state::irq0_line_hold));

	Z80(config, m_subcpu, CPU_CLOCK/2); /* 4.000MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &markham_state::markham_slave_map);
	m_subcpu->set_vblank_int("screen", FUNC(markham_state::irq0_line_hold));

	config.set_maximum_quantum(attotime::from_hz(CPU_CLOCK/256));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(markham_state::screen_update_markham));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_markham);
	PALETTE(config, m_palette, FUNC(markham_state::markham_palette), 1024, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SN76496(config, m_sn[0], CPU_CLOCK/2).add_route(ALL_OUTPUTS, "mono", 0.75);

	SN76496(config, m_sn[1], CPU_CLOCK/2).add_route(ALL_OUTPUTS, "mono", 0.75);
}

void markham_state::strnskil(machine_config &config)
{
	markham(config);
	/* basic machine hardware */
	Z80(config.replace(), m_maincpu, CPU_CLOCK/2); /* 4.000MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &markham_state::strnskil_master_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(markham_state::strnskil_scanline), "screen", 0, 1);

	Z80(config.replace(), m_subcpu, CPU_CLOCK/2); /* 4.000MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &markham_state::strnskil_slave_map);
	m_subcpu->set_periodic_int(FUNC(markham_state::irq0_line_hold), attotime::from_hz(2*(PIXEL_CLOCK/HTOTAL/VTOTAL)));

	/* video hardware */
	m_screen->set_screen_update(FUNC(markham_state::screen_update_strnskil));

	MCFG_VIDEO_START_OVERRIDE(markham_state, strnskil)

	/* sound hardware */
	m_sn[0]->set_clock(CPU_CLOCK/4);
}

void markham_state::banbam(machine_config &config)
{
	strnskil(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &markham_state::banbam_master_map);

	MB8841(config, m_mcu, CPU_CLOCK/2); /* 4.000MHz */
	// m_mcu->read_k().set(FUNC(markham_state::mcu_portk_r));
	// m_mcu->write_o().set(FUNC(markham_state::mcu_port_o_w));
	// m_mcu->write_p().set(FUNC(markham_state::mcu_port_p_w));
	// m_mcu->read_r<0>().set(FUNC(markham_state::mcu_port_r0_r));
	// m_mcu->write_r<0>().set(FUNC(markham_state::mcu_port_r0_w));
	// m_mcu->read_r<1>().set(FUNC(markham_state::mcu_port_r1_r));
	// m_mcu->write_r<1>().set(FUNC(markham_state::mcu_port_r1_w));
	// m_mcu->read_r<2>().set(FUNC(markham_state::mcu_port_r2_r));
	// m_mcu->write_r<2>().set(FUNC(markham_state::mcu_port_r2_w));
	// m_mcu->read_r<3>().set(FUNC(markham_state::mcu_port_r3_r));
	// m_mcu->write_r<3>().set(FUNC(markham_state::mcu_port_r3_w));
	m_mcu->set_disable();
}

/****************************************************************************/

ROM_START( markham )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "tv3.9",    0x0000, 0x2000, CRC(59391637) SHA1(e0cfe49a5591d6a6e64c3277319a19235b0ee6ea) )
	ROM_LOAD( "tvg4.10",  0x2000, 0x2000, CRC(1837bcce) SHA1(50e1ae0a4937f09a3dced48bb12f57cee846487a) )
	ROM_LOAD( "tvg5.11",  0x4000, 0x2000, CRC(651da602) SHA1(9f33d6ea0526af9be8ac9210910ea768da825ee5) )

	ROM_REGION( 0x10000, "subcpu", 0 ) /* sub CPU */
	ROM_LOAD( "tvg1.5",   0x0000, 0x2000, CRC(c5299766) SHA1(a6c903088ffd6c5ae0ba7ff50c8509a185f88220) )
	ROM_LOAD( "tvg2.6",   0x4000, 0x2000, CRC(b216300a) SHA1(036fafd0277b3422cf491db77748358da1ecfb43) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "tvg6.84",  0x0000, 0x2000, CRC(ab933ae5) SHA1(d2bdbc35d751480ddf8b89b90063510684b00db2) )
	ROM_LOAD( "tvg7.85",  0x2000, 0x2000, CRC(ce8edda7) SHA1(5312754aec20791398de57f08857d4097a7cfc2c) )
	ROM_LOAD( "tvg8.86",  0x4000, 0x2000, CRC(74d1536a) SHA1(ff2efbbe1420282643558a65bfa5fd278cdaf135) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "tvg9.87",  0x0000, 0x2000, CRC(42168675) SHA1(d2cce79a05ca7fda9347630fe0045a2d8182025d) )
	ROM_LOAD( "tvg10.88", 0x2000, 0x2000, CRC(fa9feb67) SHA1(669c6e1defc33541c36d4deb9667b67254f53a37) )
	ROM_LOAD( "tvg11.89", 0x4000, 0x2000, CRC(71f3dd49) SHA1(8fecb6b76907c592d545dafeaa47cf765513b3fe) )

	ROM_REGION( 0x0700, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "14-3.99",  0x0000, 0x0100, CRC(89d09126) SHA1(1f78f3b3ef8c6ba9c00a58ae89837d9a92e5078f) ) /* R */
	ROM_LOAD( "14-4.100", 0x0100, 0x0100, CRC(e1cafe6c) SHA1(8c37c3829bf1b96690fb853a2436f1b5e8d45e8c) ) /* G */
	ROM_LOAD( "14-5.101", 0x0200, 0x0100, CRC(2d444fa6) SHA1(66b64133ca740686bedd33bafd20a3f9f3df97d4) ) /* B */
	ROM_LOAD( "14-1.61",  0x0300, 0x0200, CRC(3ad8306d) SHA1(877f1d58cb8da9098ec71a7c7aec633dbf9e76e6) ) /* sprite */
	ROM_LOAD( "14-2.115", 0x0500, 0x0200, CRC(12a4f1ff) SHA1(375e37d7162053d45da66eee23d66bd432303c1c) ) /* bg */
ROM_END

ROM_START( strnskil )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "tvg3.7",    0x0000, 0x2000, CRC(31fd793a) SHA1(b86efe8ea60edf414a23fb6abc09db691c085fe9) )
	ROM_CONTINUE(          0x8000, 0x2000 )
	ROM_LOAD( "tvg4.8",    0x2000, 0x2000, CRC(c58315b5) SHA1(2039cd89ef59d05f353f6c367fa851c0f60cdc4a) )
	ROM_LOAD( "tvg5.9",    0x4000, 0x2000, CRC(29e7ded5) SHA1(6eae5988139f22c3ff166192e4fda77db38a79bc) )
	ROM_LOAD( "tvg6.10",   0x6000, 0x2000, CRC(8b126a4b) SHA1(68b617c5dc120c777e152919cba9daeaf3ceac5f) )

	ROM_REGION( 0x10000, "subcpu", 0 ) /* sub CPU */
	ROM_LOAD( "tvg1.2",    0x0000, 0x2000, CRC(b586b753) SHA1(7c9891fb279b1323c059ffdcf7c009bf971037be) )
	ROM_LOAD( "tvg2.3",    0x2000, 0x2000, CRC(8bd71bb6) SHA1(cc35e1e4cbb893ab04f1b6ceef0a050243e3b462) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "tvg7.90",   0x0000, 0x2000, CRC(ee3bd593) SHA1(398e426e53695cc184d5a2750fd32a1c2c68bf30) )
	ROM_LOAD( "tvg8.92",   0x2000, 0x2000, CRC(1b265360) SHA1(fbc64c504639106c1813bf91bd31bda1ce4c7ffe) )
	ROM_LOAD( "tvg9.94",   0x4000, 0x2000, CRC(776c7ca6) SHA1(23fd1ac15395822b318db4435e48dd4e0e3e61de) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "tvg12.102", 0x0000, 0x2000, CRC(68b9d888) SHA1(7a4071fe882c1949979f97a020d7c6e95643ef42) )
	ROM_LOAD( "tvg11.101", 0x2000, 0x2000, CRC(7f2179ff) SHA1(24fab1f4430ae883bc1f477d3df7643e06c67349) )
	ROM_LOAD( "tvg10.100", 0x4000, 0x2000, CRC(321ad963) SHA1(9b50fbf0c3b4ce7ce3c68339b99a2ccadef4646f) )

	ROM_REGION( 0x0800, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "15-3.prm",  0x0000, 0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) /* R */
	ROM_LOAD( "15-4.prm",  0x0100, 0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) /* G */
	ROM_LOAD( "15-5.prm",  0x0200, 0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) /* B */
	ROM_LOAD( "15-1.prm",  0x0300, 0x0200, CRC(d4f5b3d7) SHA1(9a244c77a752df655ff756e063d56c2c767e37d9) ) /* sprite */
	ROM_LOAD( "15-2.prm",  0x0500, 0x0200, CRC(cdffede9) SHA1(3ecdf91e3f78eb6cdd3a6f58d1a89d448a676c52) ) /* bg */

	ROM_REGION( 0x0100, "scroll_prom", 0 ) /* scroll control PROM */
	ROM_LOAD( "15-6.prm",  0x0000, 0x0100, CRC(ec4faf5b) SHA1(7ebbf50807d04105ebadec91bded069408e399ba) )
ROM_END

ROM_START( guiness )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "tvg3.15",   0x0000, 0x2000, CRC(3a605ad8) SHA1(f6e2dd4989fdb68bc55857f5a8f06601416139d5) )
	ROM_CONTINUE(          0x8000, 0x2000 )
	ROM_LOAD( "tvg4.8",    0x2000, 0x2000, CRC(c58315b5) SHA1(2039cd89ef59d05f353f6c367fa851c0f60cdc4a) )
	ROM_LOAD( "tvg5.9",    0x4000, 0x2000, CRC(29e7ded5) SHA1(6eae5988139f22c3ff166192e4fda77db38a79bc) )
	ROM_LOAD( "tvg6.10",   0x6000, 0x2000, CRC(8b126a4b) SHA1(68b617c5dc120c777e152919cba9daeaf3ceac5f) )

	ROM_REGION( 0x10000, "subcpu", 0 ) /* sub CPU */
	ROM_LOAD( "tvg1.2",    0x0000, 0x2000, CRC(b586b753) SHA1(7c9891fb279b1323c059ffdcf7c009bf971037be) )
	ROM_LOAD( "tvg2.3",    0x2000, 0x2000, CRC(8bd71bb6) SHA1(cc35e1e4cbb893ab04f1b6ceef0a050243e3b462) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "tvg7.90",   0x0000, 0x2000, CRC(ee3bd593) SHA1(398e426e53695cc184d5a2750fd32a1c2c68bf30) )
	ROM_LOAD( "tvg8.92",   0x2000, 0x2000, CRC(1b265360) SHA1(fbc64c504639106c1813bf91bd31bda1ce4c7ffe) )
	ROM_LOAD( "tvg9.94",   0x4000, 0x2000, CRC(776c7ca6) SHA1(23fd1ac15395822b318db4435e48dd4e0e3e61de) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "=tvg12.15", 0x0000, 0x2000, CRC(a82c923d) SHA1(2bd2b028d782fac18f2fe9c9ef73ce0af67db347) )
	ROM_LOAD( "tvg11.15",  0x2000, 0x2000, CRC(d432c96f) SHA1(0d4b3af778dbd40bc26bad4c673a9ce1ef537c04) )
	ROM_LOAD( "tvg10.15",  0x4000, 0x2000, CRC(a53959d6) SHA1(cdf7acf1a75d83b259948c482f06543624a695a3) )

	ROM_REGION( 0x0800, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "15-3.prm",  0x0000, 0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) /* R */
	ROM_LOAD( "15-4.prm",  0x0100, 0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) /* G */
	ROM_LOAD( "15-5.prm",  0x0200, 0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) /* B */
	ROM_LOAD( "15-1.prm",  0x0300, 0x0200, CRC(d4f5b3d7) SHA1(9a244c77a752df655ff756e063d56c2c767e37d9) ) /* sprite */
	ROM_LOAD( "15-2.prm",  0x0500, 0x0200, CRC(cdffede9) SHA1(3ecdf91e3f78eb6cdd3a6f58d1a89d448a676c52) ) /* bg */

	ROM_REGION( 0x0100, "scroll_prom", 0 ) /* scroll control PROM */
	ROM_LOAD( "15-6.prm",  0x0000, 0x0100, CRC(ec4faf5b) SHA1(7ebbf50807d04105ebadec91bded069408e399ba) )
ROM_END

ROM_START( banbam )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "ban-rom2.ic7",    0x0000, 0x2000, CRC(a5aeef6e) SHA1(92c1ad6ccb96b723e122899150e3c1855c5017b8) )
	ROM_CONTINUE(                0x8000, 0x2000 )
	ROM_LOAD( "ban-rom3.ic8",    0x2000, 0x2000, CRC(f91472bf) SHA1(94988b7bf2be3d3704a802db544070251d6c6a9c) )
	ROM_LOAD( "ban-rom4.ic9",    0x4000, 0x2000, CRC(436a09ef) SHA1(7287f741fbf3ac43d5c46b2f43ec4439cb3d0d56) )
	ROM_LOAD( "ban-rom5.ic10",   0x6000, 0x2000, CRC(45205f86) SHA1(a11d10beb21519f797c902b8a18775c8e2aa0fae) )

	ROM_REGION( 0x10000, "subcpu", 0 ) /* sub CPU */
	ROM_LOAD( "ban-rom1.ic2",    0x0000, 0x2000, CRC(e36009f6) SHA1(72c485e8c19fbfc9c850094cfd87f1055154c0c5) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "ban-rom6.ic90",   0x0000, 0x2000, CRC(41fc44df) SHA1(1c4f21cdc423078fab58370d5245a13292bf7fe6) )
	ROM_LOAD( "ban-rom7.ic92",   0x2000, 0x2000, CRC(8b429c5b) SHA1(505796eac2c8dd84f9ed29a6227b3243f81ec072) )
	ROM_LOAD( "ban-rom8.ic94",   0x4000, 0x2000, CRC(76c02d6b) SHA1(9ef7585da5376cdd604afee281594e24c69addf2) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "ban-rom11.ic102", 0x0000, 0x2000, CRC(aa827c57) SHA1(87d0c5e7df6ce40b0b2fc8f4b9dd43d4b7b0bc2e) )
	ROM_LOAD( "ban-rom10.ic101", 0x2000, 0x2000, CRC(51bd1c5c) SHA1(f714a3168d59cb4f8512440f7a2e27b6e0726a39) )
	ROM_LOAD( "ban-rom9.ic100",  0x4000, 0x2000, CRC(c0a5a4c8) SHA1(7f0978669218982d379a5d72c6198a33a8213ab5) )

	ROM_REGION( 0x0700, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "16-3.66",         0x0000, 0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) /* R      Prom type 24s10 */
	ROM_LOAD( "16-4.67",         0x0100, 0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) /* G      Prom type 24s10 */
	ROM_LOAD( "16-5.68",         0x0200, 0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) /* B      Prom type 24s10 */
	ROM_LOAD( "16-1.148",        0x0300, 0x0200, CRC(777e2770) SHA1(7f4ef42ab4e0546c2932d498cf573bd4f4296db7) ) /* sprite Prom type mb7124h */
	ROM_LOAD( "16-2.97",         0x0500, 0x0200, CRC(7f95d4b2) SHA1(68dc311739a4d5d72f4cfbace27f3a82f05316ff) ) /* bg     Prom type mb7124h */

	ROM_REGION( 0x0100, "scroll_prom", 0 ) /* scroll control PROM */
	ROM_LOAD( "16-6.59",         0x0000, 0x0100, CRC(ec4faf5b) SHA1(7ebbf50807d04105ebadec91bded069408e399ba) ) /* Prom type 24s10 */

	ROM_REGION( 0x2000, "mcu_rom", 0 ) /* protection, data used with Fujitsu MB8841 4-Bit MCU */
	ROM_LOAD( "ban-rom12.ic2",   0x0000, 0x2000, CRC(044bb2f6) SHA1(829b2152740061e0506c7504885d8404fb8fe360) )

	ROM_REGION(0x800, "mcu", 0) /* Fujitsu MB8841 4-Bit MCU internal ROM */
	ROM_LOAD( "sun-8212.ic3",    0x000,  0x800,  CRC(8869611e) SHA1(c6443f3bcb0cdb4d7b1b19afcbfe339c300f36aa) )
ROM_END

ROM_START( pettanp )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "tvg2-16a.7",   0x0000, 0x2000, CRC(4cbbbd01) SHA1(3905cf9e9d324bb23688ab29c98d71529d3dbf0c) )
	ROM_CONTINUE(             0x8000, 0x2000 )
	ROM_LOAD( "tvg3-16a.8",   0x2000, 0x2000, CRC(aaa0420f) SHA1(aa7ead51002f8b1bbefd07ff23b9064804fc31b3) )
	ROM_LOAD( "tvg4-16a.9",   0x4000, 0x2000, CRC(43306369) SHA1(1eadebd3d962da49fd204eff8692f1e1a1e3cc98) )
	ROM_LOAD( "tvg5-16a.10",  0x6000, 0x2000, CRC(da9c635f) SHA1(3c084ad159dbabfd02a9772489c3193852d135b7) )

	ROM_REGION( 0x10000, "subcpu", 0 ) /* sub CPU */
	ROM_LOAD( "tvg1-16.2",    0x0000, 0x2000, CRC(e36009f6) SHA1(72c485e8c19fbfc9c850094cfd87f1055154c0c5) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "tvg6-16.90",   0x0000, 0x2000, CRC(6905d9d5) SHA1(586bf72bab5ab6e3e319c925decc16d7f3711af1) )
	ROM_LOAD( "tvg7-16.92",   0x2000, 0x2000, CRC(40d02bfd) SHA1(2f6ca8197048318f7900b56169aba4c9fdf48693) )
	ROM_LOAD( "tvg8-16.94",   0x4000, 0x2000, CRC(b18a2244) SHA1(168061e050530e6a5bc78c14a64e635370256dfd) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "tvg11-16.102", 0x0000, 0x2000, CRC(327b7a29) SHA1(4b8d57607c4a1e84c630c38eba3fa90b5496dcde) )
	ROM_LOAD( "tvg10-16.101", 0x2000, 0x2000, CRC(624ac061) SHA1(9d479a8a256a8ff37c00bc7449b11357f9fe6cdc) )
	ROM_LOAD( "tvg9-16.100",  0x4000, 0x2000, CRC(c477e74c) SHA1(864eddcd9c817aeecb09423071f87d3b39eb5fc4) )

	ROM_REGION( 0x0700, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "16-3.66",      0x0000, 0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) /* R      Prom type 24s10 */
	ROM_LOAD( "16-4.67",      0x0100, 0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) /* G      Prom type 24s10 */
	ROM_LOAD( "16-5.68",      0x0200, 0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) /* B      Prom type 24s10 */
	ROM_LOAD( "16-1.148",     0x0300, 0x0200, CRC(777e2770) SHA1(7f4ef42ab4e0546c2932d498cf573bd4f4296db7) ) /* sprite Prom type mb7124h */
	ROM_LOAD( "16-2.97",      0x0500, 0x0200, CRC(7f95d4b2) SHA1(68dc311739a4d5d72f4cfbace27f3a82f05316ff) ) /* bg     Prom type mb7124h */

	ROM_REGION( 0x0100, "scroll_prom", 0 ) /* scroll control PROM */
	ROM_LOAD( "16-6.59",      0x0000, 0x0100, CRC(ec4faf5b) SHA1(7ebbf50807d04105ebadec91bded069408e399ba) ) /* Prom type 24s10 */

	ROM_REGION( 0x1000, "mcu_rom", 0 ) /* protection data used with Fujitsu MB8841 4-Bit MCU */
	ROM_LOAD( "tvg12-16.2",   0x0000, 0x1000, CRC(3abc6ba8) SHA1(15e0b0f9d068f6094e2be4f4f1dea0ff6e85686b) )

	ROM_REGION(0x800, "mcu", 0) /* Fujitsu MB8841 4-Bit MCU internal ROM */
	ROM_LOAD( "sun-8212.ic3", 0x000,  0x800,  NO_DUMP ) // very much likely to be same as banbam and arabian
ROM_END

/* Markham hardware */
GAME( 1983, markham,  0,        markham,  markham,  markham_state, empty_init, ROT0, "Sun Electronics", "Markham", MACHINE_SUPPORTS_SAVE )

/* Strength & Skill hardware */
GAME( 1984, strnskil, 0,        strnskil, strnskil, markham_state, empty_init, ROT0, "Sun Electronics", "Strength & Skill", MACHINE_SUPPORTS_SAVE)
GAME( 1984, guiness,  strnskil, strnskil, strnskil, markham_state, empty_init, ROT0, "Sun Electronics", "The Guiness (Japan)", MACHINE_SUPPORTS_SAVE)

/* Strength & Skill hardware with SUN 8212 MCU */
GAME( 1984, banbam,   0,        banbam,   banbam,   markham_state, empty_init, ROT0, "Sun Electronics", "BanBam", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE)
GAME( 1984, pettanp,  banbam,   banbam,   banbam,   markham_state, empty_init, ROT0, "Sun Electronics", "Pettan Pyuu (Japan)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE)
