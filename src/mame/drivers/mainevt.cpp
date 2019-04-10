// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  The Main Event, (c) 1988 Konami
  Devastators, (c) 1988 Konami

Emulation by Bryan McPhail, mish@tendril.co.uk

Notes:
- Schematics show a palette/work RAM bank selector, but this doesn't seem
  to be used?

- Devastators: has player-trench collision detection issues, player isn't
  supposed to go through them.

- Devastators: shadows don't work. Bit 7 of the sprite attribute is always 0,
  could there be a global enable flag in the 051960?
  This is particularly evident in level 2 where plane shadows cover other sprites.
  The priority/shadow encoder PROM is quite complex, however bits 5-7 of the sprite
  attribute don't seem to be used, at least not in the first two levels, so the
  PROM just maps to the fixed priority order currently implemented.

- Devastators: sprite zooming for the planes in level 2 is particularly bad.

- Devastators: title screen white backdrop is always supposed to flicker,
  it currently do that only from second/fourth attract cycles (supposed to always
  flicker from PCB video);

Both games run on Konami's PWB351024A PCB
  The Main Event uses a uPD7759 + 640KHz resonator plus sample ROM, for Devastators this area is unpopulated
  Devastators uses a YM2151 + YM3012 and the 051733, for The Main Event these chips are unpopulated

***************************************************************************/

#include "emu.h"
#include "includes/mainevt.h"
#include "includes/konamipt.h"

#include "cpu/z80/z80.h"
#include "cpu/m6809/hd6309.h"
#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/ym2151.h"
#include "emupal.h"
#include "speaker.h"


INTERRUPT_GEN_MEMBER(mainevt_state::mainevt_interrupt)
{
	if (m_k052109->is_irq_enabled())
		irq0_line_hold(device);
}

WRITE8_MEMBER(mainevt_state::dv_nmienable_w)
{
	m_nmi_enable = data;
}

INTERRUPT_GEN_MEMBER(mainevt_state::dv_interrupt)
{
	if (m_nmi_enable)
		nmi_line_pulse(device);
}


WRITE8_MEMBER(mainevt_state::mainevt_bankswitch_w)
{
	/* bit 0-1 ROM bank select */
	m_rombank->set_entry(data & 0x03);

	/* TODO: bit 5 = select work RAM or palette? */
	//palette_selected = data & 0x20;

	/* bit 6 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 7 = NINITSET (unknown) */

	/* other bits unused */
}

WRITE8_MEMBER(mainevt_state::mainevt_coin_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x10);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
	m_leds[0] = BIT(data, 0);
	m_leds[1] = BIT(data, 1);
	m_leds[2] = BIT(data, 2);
	m_leds[3] = BIT(data, 3);
}

WRITE8_MEMBER(mainevt_state::mainevt_sh_irqtrigger_w)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

READ8_MEMBER(mainevt_state::mainevt_sh_busy_r)
{
	return m_upd7759->busy_r();
}

WRITE8_MEMBER(mainevt_state::mainevt_sh_irqcontrol_w)
{
	m_upd7759->reset_w(data & 2);
	m_upd7759->start_w(data & 1);

	m_sound_irq_mask = data & 4;
}

WRITE8_MEMBER(mainevt_state::devstor_sh_irqcontrol_w)
{
	m_sound_irq_mask = data & 4;
}

WRITE8_MEMBER(mainevt_state::mainevt_sh_bankswitch_w)
{
	int bank_A, bank_B;

//logerror("CPU #1 PC: %04x bank switch = %02x\n", m_audiocpu->pc(),data);

	/* bits 0-3 select the 007232 banks */
	bank_A = (data & 0x3);
	bank_B = ((data >> 2) & 0x3);
	m_k007232->set_bank(bank_A, bank_B);

	/* bits 4-5 select the UPD7759 bank */
	m_upd7759->set_rom_bank((data >> 4) & 0x03);
}

WRITE8_MEMBER(mainevt_state::dv_sh_bankswitch_w)
{
	int bank_A, bank_B;

//logerror("CPU #1 PC: %04x bank switch = %02x\n",m_audiocpu->pc(),data);

	/* bits 0-3 select the 007232 banks */
	bank_A = (data & 0x3);
	bank_B = ((data >> 2) & 0x3);
	m_k007232->set_bank(bank_A, bank_B);
}

READ8_MEMBER(mainevt_state::k052109_051960_r)
{
	if (m_k052109->get_rmrd_line() == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return m_k051960->k051937_r(space, offset - 0x3800);
		else if (offset < 0x3c00)
			return m_k052109->read(space, offset);
		else
			return m_k051960->k051960_r(space, offset - 0x3c00);
	}
	else
		return m_k052109->read(space, offset);
}

WRITE8_MEMBER(mainevt_state::k052109_051960_w)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(space, offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(space, offset, data);
	else
		m_k051960->k051960_w(space, offset - 0x3c00, data);
}


void mainevt_state::mainevt_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(mainevt_state::k052109_051960_r), FUNC(mainevt_state::k052109_051960_w));

	map(0x1f80, 0x1f80).w(FUNC(mainevt_state::mainevt_bankswitch_w));
	map(0x1f84, 0x1f84).w("soundlatch", FUNC(generic_latch_8_device::write)); /* probably */
	map(0x1f88, 0x1f88).w(FUNC(mainevt_state::mainevt_sh_irqtrigger_w));  /* probably */
	map(0x1f8c, 0x1f8d).nopw();    /* ??? */
	map(0x1f90, 0x1f90).w(FUNC(mainevt_state::mainevt_coin_w));   /* coin counters + lamps */

	map(0x1f94, 0x1f94).portr("SYSTEM");
	map(0x1f95, 0x1f95).portr("P1");
	map(0x1f96, 0x1f96).portr("P2");
	map(0x1f97, 0x1f97).portr("DSW1");
	map(0x1f98, 0x1f98).portr("DSW3");
	map(0x1f99, 0x1f99).portr("P3");
	map(0x1f9a, 0x1f9a).portr("P4");
	map(0x1f9b, 0x1f9b).portr("DSW2");

	map(0x4000, 0x5dff).ram();
	map(0x5e00, 0x5fff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x6000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom();
}


void mainevt_state::devstors_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(mainevt_state::k052109_051960_r), FUNC(mainevt_state::k052109_051960_w));

	map(0x1f80, 0x1f80).w(FUNC(mainevt_state::mainevt_bankswitch_w));
	map(0x1f84, 0x1f84).w("soundlatch", FUNC(generic_latch_8_device::write)); /* probably */
	map(0x1f88, 0x1f88).w(FUNC(mainevt_state::mainevt_sh_irqtrigger_w));  /* probably */
	map(0x1f90, 0x1f90).w(FUNC(mainevt_state::mainevt_coin_w));   /* coin counters + lamps */

	map(0x1f94, 0x1f94).portr("SYSTEM");
	map(0x1f95, 0x1f95).portr("P1");
	map(0x1f96, 0x1f96).portr("P2");
	map(0x1f97, 0x1f97).portr("DSW1");
	map(0x1f98, 0x1f98).portr("DSW3");
	map(0x1f9b, 0x1f9b).portr("DSW2");
	map(0x1fa0, 0x1fbf).rw("k051733", FUNC(k051733_device::read), FUNC(k051733_device::write));
	map(0x1fb2, 0x1fb2).w(FUNC(mainevt_state::dv_nmienable_w));

	map(0x4000, 0x5dff).ram();
	map(0x5e00, 0x5fff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x6000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom();
}


void mainevt_state::mainevt_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram();
	map(0x9000, 0x9000).w(m_upd7759, FUNC(upd7759_device::port_w));
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xd000, 0xd000).r(FUNC(mainevt_state::mainevt_sh_busy_r));
	map(0xe000, 0xe000).w(FUNC(mainevt_state::mainevt_sh_irqcontrol_w));
	map(0xf000, 0xf000).w(FUNC(mainevt_state::mainevt_sh_bankswitch_w));
}

void mainevt_state::devstors_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram();
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe000, 0xe000).w(FUNC(mainevt_state::devstor_sh_irqcontrol_w));
	map(0xf000, 0xf000).w(FUNC(mainevt_state::dv_sh_bankswitch_w));
}


/*****************************************************************************/

static INPUT_PORTS_START( mainevt )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)

	PORT_START("P3")
	KONAMI8_B12_UNK(3)

	PORT_START("P4")
	KONAMI8_B12_UNK(4)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        /* Listed as "Unused" */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x18, 0x10, "Bonus Energy" )      PORT_DIPLOCATION("SW2:4,5") // Typo on US manual "SW2:1,2"
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPSETTING(    0x08, "70" )
	PORT_DIPSETTING(    0x10, "80" )    // factory default
	PORT_DIPSETTING(    0x18, "90" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )   // factory default
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )        /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )        /* Listed as "Unused" */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mainev2p )
	PORT_INCLUDE( mainevt )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* no keys for P3 & P4 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	KONAMI_COINAGE_ALT
INPUT_PORTS_END

static INPUT_PORTS_START( devstors )
	PORT_INCLUDE( mainev2p )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* no service2 */

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* no 3rd button */

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* no 3rd button */

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")     /* like mainevt, but different 0x00 settings */
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )     // factory default
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "150 and every 200" )
	PORT_DIPSETTING(    0x10, "150 and every 250" ) // factory default
	PORT_DIPSETTING(    0x08, "150 Only" )
	PORT_DIPSETTING(    0x00, "200 Only" )
INPUT_PORTS_END

/* Same as 'devstors', but additional "Cocktail" Dip Switch (even if I don't see the use) */
static INPUT_PORTS_START( devstors_ct )
	PORT_INCLUDE( devstors )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
INPUT_PORTS_END

/*****************************************************************************/

WRITE8_MEMBER(mainevt_state::volume_callback)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void mainevt_state::machine_start()
{
	m_leds.resolve();
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x2000);

	save_item(NAME(m_nmi_enable));
}

void mainevt_state::machine_reset()
{
	m_nmi_enable = 0;
}

INTERRUPT_GEN_MEMBER(mainevt_state::mainevt_sound_timer_irq)
{
	if(m_sound_irq_mask)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INTERRUPT_GEN_MEMBER(mainevt_state::devstors_sound_timer_irq)
{
	if(m_sound_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

void mainevt_state::mainevt(machine_config &config)
{
	/* basic machine hardware */
	HD6309E(config, m_maincpu, 24_MHz_XTAL / 8); // E & Q generated by 052109
	m_maincpu->set_addrmap(AS_PROGRAM, &mainevt_state::mainevt_map);
	m_maincpu->set_vblank_int("screen", FUNC(mainevt_state::mainevt_interrupt));

	Z80(config, m_audiocpu, 3.579545_MHz_XTAL);  /* 3.579545 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &mainevt_state::mainevt_sound_map);
	m_audiocpu->set_periodic_int(FUNC(mainevt_state::mainevt_sound_timer_irq), attotime::from_hz(8*60));  /* ??? */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
//  screen.set_size(64*8, 32*8);
//  screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_raw(XTAL(24'000'000)/3, 528, 14*8, (64-14)*8, 256, 16, 240); // same hardware as Devastators so assume 59.17
	screen.set_screen_update(FUNC(mainevt_state::screen_update_mainevt));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 256).enable_shadows();

	K052109(config, m_k052109, 24_MHz_XTAL);
	m_k052109->set_palette("palette");
	m_k052109->set_tile_callback(FUNC(mainevt_state::mainevt_tile_callback), this);

	K051960(config, m_k051960, 24_MHz_XTAL);
	m_k051960->set_palette("palette");
	m_k051960->set_screen_tag("screen");
	m_k051960->set_sprite_callback(FUNC(mainevt_state::mainevt_sprite_callback), this);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	K007232(config, m_k007232, 3.579545_MHz_XTAL);
	m_k007232->port_write().set(FUNC(mainevt_state::volume_callback));
	m_k007232->add_route(0, "mono", 0.20);
	m_k007232->add_route(1, "mono", 0.20);

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.50);
}


void mainevt_state::devstors(machine_config &config)
{
	/* basic machine hardware */
	HD6309E(config, m_maincpu, 24_MHz_XTAL / 8); // E & Q generated by 052109
	m_maincpu->set_addrmap(AS_PROGRAM, &mainevt_state::devstors_map);
	m_maincpu->set_vblank_int("screen", FUNC(mainevt_state::dv_interrupt));

	Z80(config, m_audiocpu, 3.579545_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &mainevt_state::devstors_sound_map);
	m_audiocpu->set_periodic_int(FUNC(mainevt_state::devstors_sound_timer_irq), attotime::from_hz(4*60)); /* ??? */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
//  screen.set_size(64*8, 32*8);
//  screen.set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1);
	screen.set_raw(XTAL(24'000'000)/3, 528, 13*8, (64-13)*8, 256, 16, 240); // measured 59.17
	screen.set_screen_update(FUNC(mainevt_state::screen_update_dv));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 256).enable_shadows();

	K052109(config, m_k052109, 24_MHz_XTAL);
	m_k052109->set_palette("palette");
	m_k052109->set_tile_callback(FUNC(mainevt_state::dv_tile_callback), this);

	K051960(config, m_k051960, 24_MHz_XTAL);
	m_k051960->set_palette("palette");
	m_k051960->set_screen_tag("screen");
	m_k051960->set_sprite_callback(FUNC(mainevt_state::dv_sprite_callback), this);

	K051733(config, "k051733", 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "mono", 0.30).add_route(1, "mono", 0.30);

	K007232(config, m_k007232, 3.579545_MHz_XTAL);
	m_k007232->port_write().set(FUNC(mainevt_state::volume_callback));
	m_k007232->add_route(0, "mono", 0.20);
	m_k007232->add_route(1, "mono", 0.20);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( mainevt )    /* 4 players - English title screen - No "Warning" message in the ROM */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "799y02.k11",   0x00000, 0x10000, CRC(e2e7dbd5) SHA1(80314cd42a9f47f7bb82a2160fb5ef2ddc6dff30) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "799c01.f7",    0x00000, 0x08000, CRC(447c4c5c) SHA1(86e42132793c59cc6feece143516f7ecd4ed14e8) )

	ROM_REGION( 0x20000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "799c06.f22",   0x00000, 0x08000, CRC(f839cb58) SHA1(b36202ca2b68b6249c3f972ad09501e28a0162f7) )
	ROM_LOAD32_BYTE( "799c07.h22",   0x00001, 0x08000, CRC(176df538) SHA1(379e1de81afb85b1559de170cd2ab9f4af2b137e) )
	ROM_LOAD32_BYTE( "799c08.j22",   0x00002, 0x08000, CRC(d01e0078) SHA1(7ac242eb24271ac2783ec4d9e97ae051f1f3363a) )
	ROM_LOAD32_BYTE( "799c09.k22",   0x00003, 0x08000, CRC(9baec75e) SHA1(a8f6102c8fd46f18678f336bc44be31458ca9256) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "799b04.h4",    0x00000, 0x80000, CRC(323e0c2b) SHA1(c108d656b6ceff13c910739e4ca760acbb640de3) )
	ROM_LOAD32_WORD( "799b05.k4",    0x00002, 0x80000, CRC(571c5831) SHA1(2a18f0bcf6946ada6e0bde7edbd11afd4db1c170) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14",  0x0000, 0x0100, CRC(61f6c8d1) SHA1(c70f1f8e434aaaffb89e30e2230a08374ef324ad) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "799b03.d4",    0x00000, 0x80000, CRC(f1cfd342) SHA1(079afc5c631de7f5b652d0ce6fd44b3aacd14a1b) )

	ROM_REGION( 0x80000, "upd", 0 ) /* 512k for the UPD7759C samples */
	ROM_LOAD( "799b06.c22",   0x00000, 0x80000, CRC(2c8c47d7) SHA1(18a899767177ddfd870df9ed156d8bbc04b58a19) )
ROM_END

ROM_START( mainevto )   /* 4 players - English title screen - No "Warning" message in the ROM */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "799f02.k11",   0x00000, 0x10000, CRC(c143596b) SHA1(5da7efaf0f7c7a493cc242eae115f278bc9c134b) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "799c01.f7",    0x00000, 0x08000, CRC(447c4c5c) SHA1(86e42132793c59cc6feece143516f7ecd4ed14e8) )

	ROM_REGION( 0x20000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "799c06.f22",   0x00000, 0x08000, CRC(f839cb58) SHA1(b36202ca2b68b6249c3f972ad09501e28a0162f7) )
	ROM_LOAD32_BYTE( "799c07.h22",   0x00001, 0x08000, CRC(176df538) SHA1(379e1de81afb85b1559de170cd2ab9f4af2b137e) )
	ROM_LOAD32_BYTE( "799c08.j22",   0x00002, 0x08000, CRC(d01e0078) SHA1(7ac242eb24271ac2783ec4d9e97ae051f1f3363a) )
	ROM_LOAD32_BYTE( "799c09.k22",   0x00003, 0x08000, CRC(9baec75e) SHA1(a8f6102c8fd46f18678f336bc44be31458ca9256) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "799b04.h4",    0x00000, 0x80000, CRC(323e0c2b) SHA1(c108d656b6ceff13c910739e4ca760acbb640de3) )
	ROM_LOAD32_WORD( "799b05.k4",    0x00002, 0x80000, CRC(571c5831) SHA1(2a18f0bcf6946ada6e0bde7edbd11afd4db1c170) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14",  0x0000, 0x0100, CRC(61f6c8d1) SHA1(c70f1f8e434aaaffb89e30e2230a08374ef324ad) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "799b03.d4",    0x00000, 0x80000, CRC(f1cfd342) SHA1(079afc5c631de7f5b652d0ce6fd44b3aacd14a1b) )

	ROM_REGION( 0x80000, "upd", 0 ) /* 512k for the UPD7759C samples */
	ROM_LOAD( "799b06.c22",   0x00000, 0x80000, CRC(2c8c47d7) SHA1(18a899767177ddfd870df9ed156d8bbc04b58a19) )
ROM_END

ROM_START( mainevt2p )  /* 2 players - English title screen - "Warning" message in the ROM (not displayed) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "799x02.k11",   0x00000, 0x10000, CRC(42cfc650) SHA1(2d1918ebc0d93a2356ad995a6854dbde7c3b8daf) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "799c01.f7",    0x00000, 0x08000, CRC(447c4c5c) SHA1(86e42132793c59cc6feece143516f7ecd4ed14e8) )

	ROM_REGION( 0x20000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "799c06.f22",   0x00000, 0x08000, CRC(f839cb58) SHA1(b36202ca2b68b6249c3f972ad09501e28a0162f7) )
	ROM_LOAD32_BYTE( "799c07.h22",   0x00001, 0x08000, CRC(176df538) SHA1(379e1de81afb85b1559de170cd2ab9f4af2b137e) )
	ROM_LOAD32_BYTE( "799c08.j22",   0x00002, 0x08000, CRC(d01e0078) SHA1(7ac242eb24271ac2783ec4d9e97ae051f1f3363a) )
	ROM_LOAD32_BYTE( "799c09.k22",   0x00003, 0x08000, CRC(9baec75e) SHA1(a8f6102c8fd46f18678f336bc44be31458ca9256) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "799b04.h4",    0x00000, 0x80000, CRC(323e0c2b) SHA1(c108d656b6ceff13c910739e4ca760acbb640de3) )
	ROM_LOAD32_WORD( "799b05.k4",    0x00002, 0x80000, CRC(571c5831) SHA1(2a18f0bcf6946ada6e0bde7edbd11afd4db1c170) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14",  0x0000, 0x0100, CRC(61f6c8d1) SHA1(c70f1f8e434aaaffb89e30e2230a08374ef324ad) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "799b03.d4",    0x00000, 0x80000, CRC(f1cfd342) SHA1(079afc5c631de7f5b652d0ce6fd44b3aacd14a1b) )

	ROM_REGION( 0x80000, "upd", 0 ) /* 512k for the UPD7759C samples */
	ROM_LOAD( "799b06.c22",   0x00000, 0x80000, CRC(2c8c47d7) SHA1(18a899767177ddfd870df9ed156d8bbc04b58a19) )
ROM_END

ROM_START( ringohja )   /* 2 players - Japan title screen - "Warning" message in the ROM (displayed) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "799n02.k11",   0x00000, 0x10000, CRC(f9305dd0) SHA1(7135053be9d46ac9c09ab63eca1eb71825a71a13) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "799c01.f7",    0x00000, 0x08000, CRC(447c4c5c) SHA1(86e42132793c59cc6feece143516f7ecd4ed14e8) )

	ROM_REGION( 0x20000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "799c06.f22",   0x00000, 0x08000, CRC(f839cb58) SHA1(b36202ca2b68b6249c3f972ad09501e28a0162f7) )
	ROM_LOAD32_BYTE( "799c07.h22",   0x00001, 0x08000, CRC(176df538) SHA1(379e1de81afb85b1559de170cd2ab9f4af2b137e) )
	ROM_LOAD32_BYTE( "799c08.j22",   0x00002, 0x08000, CRC(d01e0078) SHA1(7ac242eb24271ac2783ec4d9e97ae051f1f3363a) )
	ROM_LOAD32_BYTE( "799c09.k22",   0x00003, 0x08000, CRC(9baec75e) SHA1(a8f6102c8fd46f18678f336bc44be31458ca9256) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "799b04.h4",    0x00000, 0x80000, CRC(323e0c2b) SHA1(c108d656b6ceff13c910739e4ca760acbb640de3) )
	ROM_LOAD32_WORD( "799b05.k4",    0x00002, 0x80000, CRC(571c5831) SHA1(2a18f0bcf6946ada6e0bde7edbd11afd4db1c170) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14",  0x0000, 0x0100, CRC(61f6c8d1) SHA1(c70f1f8e434aaaffb89e30e2230a08374ef324ad) )    /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "799b03.d4",    0x00000, 0x80000, CRC(f1cfd342) SHA1(079afc5c631de7f5b652d0ce6fd44b3aacd14a1b) )

	ROM_REGION( 0x80000, "upd", 0 ) /* 512k for the UPD7759C samples */
	ROM_LOAD( "799b06.c22",   0x00000, 0x80000, CRC(2c8c47d7) SHA1(18a899767177ddfd870df9ed156d8bbc04b58a19) )
ROM_END


ROM_START( devstors )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "890z02.k11",  0x00000, 0x10000, CRC(ebeb306f) SHA1(838fcfe95dfedd61f21f34301d48e337db765ab2) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD32_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD32_BYTE( "890f08.j22",  0x00002, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD32_BYTE( "890f09.k22",  0x00003, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD32_WORD( "890f05.k4",  0x00002, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) ) /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END

ROM_START( devstorsx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "890x02.k11",  0x00000, 0x10000, CRC(e58ebb35) SHA1(4253b6a7128534cc0866bc910a271d91ac8b40fd) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD32_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD32_BYTE( "890f08.j22",  0x00002, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD32_BYTE( "890f09.k22",  0x00003, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD32_WORD( "890f05.k4",  0x00002, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) ) /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END

ROM_START( devstorsv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "890v02.k11",   0x00000, 0x10000, CRC(52f4ccdd) SHA1(074e526ed170a5f2083c8c0808734291a2ea7403) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD32_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD32_BYTE( "890f08.j22",  0x00002, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD32_BYTE( "890f09.k22",  0x00003, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD32_WORD( "890f05.k4",  0x00002, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) ) /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END

ROM_START( devstors2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "890_202.k11",   0x00000, 0x10000, CRC(afbf0951) SHA1(6eb5d7e1de58058bd50d184d4c7e8dbea9cce13d) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD32_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD32_BYTE( "890f08.j22",  0x00002, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD32_BYTE( "890f09.k22",  0x00003, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD32_WORD( "890f05.k4",  0x00002, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) ) /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END

ROM_START( garuka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "890w02.k11",   0x00000, 0x10000, CRC(b2f6f538) SHA1(95dad3258a2e4c5648d0fc22c06fa3e2da3b5ed1) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD32_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD32_BYTE( "890f08.j22",  0x00002, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD32_BYTE( "890f09.k22",  0x00003, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* sprites */
	ROM_LOAD32_WORD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD32_WORD( "890f05.k4",  0x00002, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) ) /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END



GAME( 1988, mainevt,   0,        mainevt,  mainevt,     mainevt_state, empty_init, ROT0,  "Konami", "The Main Event (4 Players ver. Y)",     MACHINE_SUPPORTS_SAVE )
GAME( 1988, mainevto,  mainevt,  mainevt,  mainevt,     mainevt_state, empty_init, ROT0,  "Konami", "The Main Event (4 Players ver. F)",     MACHINE_SUPPORTS_SAVE )
GAME( 1988, mainevt2p, mainevt,  mainevt,  mainev2p,    mainevt_state, empty_init, ROT0,  "Konami", "The Main Event (2 Players ver. X)",     MACHINE_SUPPORTS_SAVE )
GAME( 1988, ringohja,  mainevt,  mainevt,  mainev2p,    mainevt_state, empty_init, ROT0,  "Konami", "Ring no Ohja (Japan 2 Players ver. N)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, devstors,  0,        devstors, devstors,    mainevt_state, empty_init, ROT90, "Konami", "Devastators (ver. Z)",                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 1988, devstorsx, devstors, devstors, devstors_ct, mainevt_state, empty_init, ROT90, "Konami", "Devastators (ver. X)",                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 1988, devstorsv, devstors, devstors, devstors,    mainevt_state, empty_init, ROT90, "Konami", "Devastators (ver. V)",                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 1988, devstors2, devstors, devstors, devstors_ct, mainevt_state, empty_init, ROT90, "Konami", "Devastators (ver. 2)",                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 1988, garuka,    devstors, devstors, devstors_ct, mainevt_state, empty_init, ROT90, "Konami", "Garuka (Japan ver. W)",                 MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
