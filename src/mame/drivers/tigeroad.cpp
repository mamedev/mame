// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Bryan McPhail
/***************************************************************************

Tiger Road     (C) 1987 Capcom (licensed to Romstar)
F1 Dream       (C) 1988 Capcom

cloned hardware:
Pushman        (C) 1990 Comad
Bouncing Balls (c) 1991 Comad

Please contact Phil Stroffolino (phil@maya.com) if there are any questions
regarding this driver.

TODO:
- F1 Dream throws an address error if player wins all the races (i.e. when the
  game is supposed to give an ending):
  010C68: 102E 001C      move.b  ($1c,A6), D0       ; reads 0xf from work RAM (misaligned)
  010C6C: 207B 000E      movea.l ($e,PC,D0.w), A0   ; table from 0x10c7c onward
  010C70: 4E90           jsr     (A0)               ; throws address error here
  None of the available 5 vectors seems to fit here, btanb?

**************************************************************************

Memory Overview:
    0xfe0800    sprites
    0xfec000    text
    0xfe4000    input ports,dip switches (read); sound out, video reg (write)
    0xfe4002    protection (F1 Dream only)
    0xfe8000    scroll registers
    0xff8200    palette
    0xffC000    working RAM

**************************************************************************

Pushman Notes

With 'Debug Mode' on button 2 advances a level, button 3 goes back.

The microcontroller mainly controls the animation of the enemy robots,
the communication between the 68000 and MCU is probably not emulated
100% correct but it works.

Emulation by Bryan McPhail, mish@tendril.co.uk

The hardware is actually very similar to F1-Dream and Tiger Road but
with a 68705 for protection.

Pushman is known to be released in a 2 PCB stack as well as a large
single plane board.

***************************************************************************/

#include "emu.h"
#include "includes/tigeroad.h"
#include "screen.h"
#include "speaker.h"



WRITE16_MEMBER(tigeroad_state::tigeroad_soundcmd_w)
{
	if (ACCESSING_BITS_8_15)
		m_soundlatch->write(data >> 8);
}


WRITE8_MEMBER(tigeroad_state::msm5205_w)
{
	m_msm->reset_w(BIT(data, 7));
	m_msm->write_data(data);
	m_msm->vclk_w(1);
	m_msm->vclk_w(0);
}


WRITE8_MEMBER(f1dream_state::out1_w)
{
	m_soundlatch->write(data);
}

WRITE8_MEMBER(f1dream_state::out3_w)
{
	if ((m_old_p3 & 0x20) != (data & 0x20))
	{
		// toggles at the start and end of interrupt
	}

	if ((m_old_p3 & 0x01) != (data & 0x01))
	{
		// toggles at the end of interrupt
		if (!(data & 0x01))
		{
			m_maincpu->resume(SUSPEND_REASON_HALT);
		}
	}

	m_old_p3 = data;
}

WRITE16_MEMBER(f1dream_state::blktiger_to_mcu_w)
{
	m_mcu->set_input_line(MCS51_INT0_LINE, HOLD_LINE);

	/* after triggering this address there are one or two NOPs in the 68k code, then it expects the response to be ready
	   the MCU isn't that fast, so either the CPU is suspended on write, or when bit 0x20 of MCU Port 3 toggles in the
	   MCU interrupt code, however no combination of increasing the clock / boosting interleave etc. allows the MCU code
	   to get there in time before the 68k is already expecting a result */
	m_maincpu->suspend(SUSPEND_REASON_HALT, true);
}

/***************************************************************************/

void tigeroad_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0xfe0800, 0xfe0cff).ram().share("spriteram");
	map(0xfe0d00, 0xfe1807).ram();     /* still part of OBJ RAM */
	map(0xfe4000, 0xfe4001).portr("P1_P2").w(FUNC(tigeroad_state::tigeroad_videoctrl_w));   /* char bank, coin counters, + ? */
	map(0xfe4002, 0xfe4003).portr("SYSTEM").w(FUNC(tigeroad_state::tigeroad_soundcmd_w)); /* AM_WRITE(tigeroad_soundcmd_w) is replaced in init for for f1dream protection */
	map(0xfe4004, 0xfe4005).portr("DSW");
	map(0xfe8000, 0xfe8003).w(FUNC(tigeroad_state::tigeroad_scroll_w));
	map(0xfe800e, 0xfe800f).writeonly();    /* fe800e = watchdog or IRQ acknowledge */
	map(0xfec000, 0xfec7ff).ram().w(FUNC(tigeroad_state::tigeroad_videoram_w)).share("videoram");

	map(0xff8000, 0xff87ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xffc000, 0xffffff).ram().share("ram16");
}


READ8_MEMBER(f1dream_state::mcu_shared_r)
{
	uint8_t ret = m_ram16[(0x3fe0 / 2) + offset];
	return ret;
}

WRITE8_MEMBER(f1dream_state::mcu_shared_w)
{
	m_ram16[(0x3fe0 / 2) + offset] = (m_ram16[(0x3fe0 / 2) + offset] & 0xff00) | data;
}

void f1dream_state::f1dream_map(address_map &map)
{
	main_map(map);
	map(0xfe4002, 0xfe4003).portr("SYSTEM").w(FUNC(f1dream_state::blktiger_to_mcu_w));
}

void f1dream_state::f1dream_mcu_io(address_map &map)
{
	map(0x7f0, 0x7ff).rw(FUNC(f1dream_state::mcu_shared_r), FUNC(f1dream_state::mcu_shared_w));
}


void pushman_state::pushman_map(address_map &map)
{
	main_map(map);

	map(0x060000, 0x060007).r(FUNC(pushman_state::mcu_comm_r));
	map(0x060000, 0x060003).w(FUNC(pushman_state::pushman_mcu_comm_w));
}

void pushman_state::bballs_map(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x00000, 0x3ffff).rom();
	map(0x60000, 0x60007).r(FUNC(pushman_state::mcu_comm_r));
	map(0x60000, 0x60001).w(FUNC(pushman_state::bballs_mcu_comm_w));
	// are these mirror addresses or does this PCB have a different addressing?
	map(0xe0800, 0xe17ff).ram().share("spriteram");
	map(0xe4000, 0xe4001).portr("P1_P2").w(FUNC(pushman_state::tigeroad_videoctrl_w));
	map(0xe4002, 0xe4003).portr("SYSTEM").w(FUNC(pushman_state::tigeroad_soundcmd_w));
	map(0xe4004, 0xe4005).portr("DSW");
	map(0xe8000, 0xe8003).w(FUNC(pushman_state::tigeroad_scroll_w));
	map(0xe800e, 0xe800f).nopw(); /* ? */
	map(0xec000, 0xec7ff).ram().w(FUNC(pushman_state::tigeroad_videoram_w)).share("videoram");

	map(0xf8000, 0xf87ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfc000, 0xfffff).ram().share("ram16");
}

/* Capcom games ONLY */
void tigeroad_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa001).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void tigeroad_state::sound_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x7f, 0x7f).w("soundlatch2", FUNC(generic_latch_8_device::write));
}

/* toramich ONLY */
void tigeroad_state::sample_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void tigeroad_state::sample_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0x01, 0x01).w(FUNC(tigeroad_state::msm5205_w));
}

/* Pushman / Bouncing Balls */
void tigeroad_state::comad_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void tigeroad_state::comad_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ym1", FUNC(ym2203_device::write));
	map(0x80, 0x81).w("ym2", FUNC(ym2203_device::write));
}



static INPUT_PORTS_START( tigeroad )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DIPA:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DIPA:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DIPA:7")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DIPA:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("DIPB:1,2")
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("DIPB:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("DIPB:4,5")
	PORT_DIPSETTING(      0x1800, "20000 70000 70000" )
	PORT_DIPSETTING(      0x1000, "20000 80000 80000" )
	PORT_DIPSETTING(      0x0800, "30000 80000 80000" )
	PORT_DIPSETTING(      0x0000, "30000 90000 90000" )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DIPB:6,7")
	PORT_DIPSETTING(      0x2000, "Very Easy (Level 0)")
	PORT_DIPSETTING(      0x4000, "Easy (Level 10)")
	PORT_DIPSETTING(      0x6000, "Normal (Level 20)")
	PORT_DIPSETTING(      0x0000, "Difficult (Level 30)")
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("DIPB:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( toramich )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ))
	PORT_DIPSETTING(      0x0400, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0800, "20000 70000 70000" )
	PORT_DIPSETTING(      0x0000, "20000 80000 80000" )
	PORT_DIPNAME( 0x1000, 0x1000, "Allow Level Select" )
	PORT_DIPSETTING(      0x1000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( f1dream )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DIPA:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )  PORT_DIPLOCATION("DIPA:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DIPA:7")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DIPA:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("DIPB:1,2") // "Not Used" according to Romstar manual
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("DIPB:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ))
	PORT_DIPSETTING(      0x0400, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x1800, 0x1800, "F1 Up Point" ) PORT_DIPLOCATION("DIPB:4,5")
	PORT_DIPSETTING(      0x1800, "12" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0800, "18" )
	PORT_DIPSETTING(      0x0000, "20" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DIPB:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Version ) ) PORT_DIPLOCATION("DIPB:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Japan ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("DIPB:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( pushman )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") /* not sure, probably wrong */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Debug Mode (Cheat)")         PORT_DIPLOCATION("SW1:1")    /* Listed as "Screen Skip" */
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Pull Option" )           PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, "5" )
	PORT_DIPSETTING(      0x0000, "9" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Level_Select ) )     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0020, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( bballs )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Open/Close Gate")   // Open/Close gate
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Zap")   // Use Zap
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )  // BUTTON3 in "test mode"
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Open/Close Gate")   // Open/Close gate
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Zap")   // Use Zap
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // BUTTON3 in "test mode"

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") /* not sure, probably wrong */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )         // less bubbles before cycling
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )         // more bubbles before cycling
	PORT_DIPNAME( 0x0010, 0x0000, "Music (In-game)" )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Music (Attract Mode)" )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0100, 0x0100, "Zaps" )              PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0200, 0x0000, "Display Next Ball" )         PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6")   // code at 0x0054ac, 0x0054f2, 0x0056fc
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Service_Mode ) )     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( Off ) )
//  PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, "Inputs/Outputs" )
	PORT_DIPSETTING(      0x0000, "Graphics" )
INPUT_PORTS_END

static const gfx_layout text_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tile_layout =
{
	32,32,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{
		0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
		64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+8+0, 64*8+8+1, 64*8+8+2, 64*8+8+3,
		2*64*8+0, 2*64*8+1, 2*64*8+2, 2*64*8+3, 2*64*8+8+0, 2*64*8+8+1, 2*64*8+8+2, 2*64*8+8+3,
		3*64*8+0, 3*64*8+1, 3*64*8+2, 3*64*8+3, 3*64*8+8+0, 3*64*8+8+1, 3*64*8+8+2, 3*64*8+8+3,
	},
	{
		0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16
	},
	256*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};


static GFXDECODE_START( gfx_tigeroad )
	GFXDECODE_ENTRY( "text", 0, text_layout,      0x300, 16 )
	GFXDECODE_ENTRY( "tiles", 0, tile_layout,     0x100, 16 )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout, 0x200, 16 )
GFXDECODE_END

void tigeroad_state::tigeroad(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(10'000'000)); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &tigeroad_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tigeroad_state::irq2_line_hold));

	Z80(config, m_audiocpu, XTAL(3'579'545)); /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &tigeroad_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &tigeroad_state::sound_port_map);

	/* IRQs are triggered by the YM2203 */

	/* video hardware */
	BUFFERED_SPRITERAM16(config, "spriteram");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.08);   /* verified on pcb */
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tigeroad_state::screen_update_tigeroad));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tigeroad);

	TIGEROAD_SPRITE(config, m_spritegen, 0);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, "soundlatch2");

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(3'579'545))); /* verified on pcb */
	ym1.irq_handler().set_inputline("audiocpu", 0);
	ym1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(3'579'545))); /* verified on pcb */
	ym2.add_route(ALL_OUTPUTS, "mono", 0.25);
}


void f1dream_state::f1dream(machine_config &config)
{
	tigeroad(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &f1dream_state::f1dream_map);

	I8751(config, m_mcu, XTAL(10'000'000)); /* ??? */
	m_mcu->set_addrmap(AS_IO, &f1dream_state::f1dream_mcu_io);
	m_mcu->port_out_cb<1>().set(FUNC(f1dream_state::out1_w));
	m_mcu->port_out_cb<3>().set(FUNC(f1dream_state::out3_w));
}

/* same as above but with additional Z80 for samples playback */
void tigeroad_state::toramich(machine_config &config)
{
	tigeroad(config);

	/* basic machine hardware */

	z80_device &sample(Z80(config, "sample", 3579545)); /* ? */
	sample.set_addrmap(AS_PROGRAM, &tigeroad_state::sample_map);
	sample.set_addrmap(AS_IO, &tigeroad_state::sample_port_map);
	sample.set_periodic_int(FUNC(tigeroad_state::irq0_line_hold), attotime::from_hz(4000));  /* ? */

	/* sound hardware */
	MSM5205(config, m_msm, 384000);
	m_msm->set_prescaler_selector(msm5205_device::SEX_4B);  /* 4KHz playback ?  */
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void tigeroad_state::f1dream_comad(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tigeroad_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tigeroad_state::irq2_line_hold));

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &tigeroad_state::comad_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &tigeroad_state::comad_sound_io_map);

	config.m_minimum_quantum = attotime::from_hz(3600);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, "spriteram");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.08);   /* verified on pcb */
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tigeroad_state::screen_update_tigeroad));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tigeroad);

	TIGEROAD_SPRITE(config, m_spritegen, 0);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym1(YM2203(config, "ym1", 2000000));
	ym1.irq_handler().set_inputline("audiocpu", 0);
	ym1.add_route(ALL_OUTPUTS, "mono", 0.40);

	ym2203_device &ym2(YM2203(config, "ym2", 2000000));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.40);
}


void pushman_state::machine_start()
{
	save_item(NAME(m_host_semaphore));
	save_item(NAME(m_mcu_semaphore));
	save_item(NAME(m_host_latch));
	save_item(NAME(m_mcu_latch));
	save_item(NAME(m_mcu_output));
	save_item(NAME(m_mcu_latch_ctl));
}

void pushman_state::pushman(machine_config &config)
{
	f1dream_comad(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pushman_state::pushman_map);

	M68705R3(config, m_mcu, 4000000);    /* No idea */
	m_mcu->porta_w().set(FUNC(pushman_state::mcu_pa_w));
	m_mcu->portb_w().set(FUNC(pushman_state::mcu_pb_w));
	m_mcu->portc_w().set(FUNC(pushman_state::mcu_pc_w));
}

void pushman_state::bballs(machine_config &config)
{
	pushman(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pushman_state::bballs_map);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tigeroad ) /* ECT program roms */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "tre_02.6j", 0x00000, 0x20000, CRC(c394add0) SHA1(f71cceca92ed7d2211f508df9ddfa97e0dd28d11) ) /* Blue ink underline */
	ROM_LOAD16_BYTE( "tre_04.6k", 0x00001, 0x20000, CRC(73bfbf4a) SHA1(821af477953f7a64f4f1b09e8978fb2bce4138ff) ) /* Blue ink underline */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "tru_05.12k", 0x0000, 0x8000, CRC(f9a7c9bf) SHA1(4d37c71aa6523ac21c6e8b23f9957e75ec4304bf) ) /* Red ink underline */

	/* no samples player in the English version */

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "tr_01.10d", 0x00000, 0x08000, CRC(74a9f08c) SHA1(458958c8d9a2af5df88bb24c9c5bcbd37d6856bc) ) /* 8x8 text */

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "tr-01a.3f", 0x00000, 0x20000, CRC(a8aa2e59) SHA1(792f50d688a4ffb574e41257816bc304d41f0458) ) /* tiles */
	ROM_LOAD( "tr-04a.3h", 0x20000, 0x20000, CRC(8863a63c) SHA1(11bfce5b09c5b8a781c658f035d5658c3710d189) )
	ROM_LOAD( "tr-02a.3j", 0x40000, 0x20000, CRC(1a2c5f89) SHA1(2a2aa2f1e2a0cdd4bbdb25236e49c7cc573db9e9) )
	ROM_LOAD( "tr-05.3l",  0x60000, 0x20000, CRC(5bf453b3) SHA1(5eef151974c6b818a17756549d24a702e1f3a859) )
	ROM_LOAD( "tr-03a.2f", 0x80000, 0x20000, CRC(1e0537ea) SHA1(bc65f7104d5f7728b68b3dcb45151c41fc30aa0d) )
	ROM_LOAD( "tr-06a.2h", 0xa0000, 0x20000, CRC(b636c23a) SHA1(417e289745996bd00114df6ade591e702265d3a5) )
	ROM_LOAD( "tr-07a.2j", 0xc0000, 0x20000, CRC(5f907d4d) SHA1(1820c5c6e0b078db9c64655c7983ea115ad81036) )
	ROM_LOAD( "tr_08.2l",  0xe0000, 0x20000, CRC(adee35e2) SHA1(6707cf43a697eb9465449a144ae4508afe2e6496) ) /* EPROM */

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "tr-09a.3b", 0x00000, 0x20000, CRC(3d98ad1e) SHA1(f12cdf50e1708ddae092b9784d4319a7d5f092bc) ) /* sprites */
	ROM_LOAD( "tr-10a.2b", 0x20000, 0x20000, CRC(8f6f03d7) SHA1(08a02cfb373040ea5ffbf5604f68df92a1338bb0) )
	ROM_LOAD( "tr-11a.3d", 0x40000, 0x20000, CRC(cd9152e5) SHA1(6df3c43c0c41289890296c2b2aeca915dfdae3b0) )
	ROM_LOAD( "tr-12a.2d", 0x60000, 0x20000, CRC(7d8a99d0) SHA1(af8221cfd2ce9aa3bf296981fb7fddd1e9ef4599) )

	ROM_REGION( 0x08000, "bgmap", 0 )    /* background tilemaps */
	ROM_LOAD( "tr_13.7l", 0x0000, 0x8000, CRC(a79be1eb) SHA1(4191ccd48f7650930f9a4c2be0790239d7420bb1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tr.9e", 0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) - N82S129A or compatible */
ROM_END

ROM_START( tigeroadu ) /* US ROMSTAR program roms */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "tru_02.6j", 0x00000, 0x20000, CRC(8d283a95) SHA1(eb6c9225f79f62c22ae1e8980a557d896f598947) ) /* Red ink underline */
	ROM_LOAD16_BYTE( "tru_04.6k", 0x00001, 0x20000, CRC(72e2ef20) SHA1(57ab7df2050042690ccfb1f2d170840f926dcf46) ) /* Red ink underline */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "tru_05.12k", 0x0000, 0x8000, CRC(f9a7c9bf) SHA1(4d37c71aa6523ac21c6e8b23f9957e75ec4304bf) ) /* Red ink underline */

	/* no samples player in the English version */

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "tr_01.10d", 0x00000, 0x08000, CRC(74a9f08c) SHA1(458958c8d9a2af5df88bb24c9c5bcbd37d6856bc) ) /* 8x8 text */

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "tr-01a.3f", 0x00000, 0x20000, CRC(a8aa2e59) SHA1(792f50d688a4ffb574e41257816bc304d41f0458) ) /* tiles */
	ROM_LOAD( "tr-04a.3h", 0x20000, 0x20000, CRC(8863a63c) SHA1(11bfce5b09c5b8a781c658f035d5658c3710d189) )
	ROM_LOAD( "tr-02a.3j", 0x40000, 0x20000, CRC(1a2c5f89) SHA1(2a2aa2f1e2a0cdd4bbdb25236e49c7cc573db9e9) )
	ROM_LOAD( "tr-05.3l",  0x60000, 0x20000, CRC(5bf453b3) SHA1(5eef151974c6b818a17756549d24a702e1f3a859) )
	ROM_LOAD( "tr-03a.2f", 0x80000, 0x20000, CRC(1e0537ea) SHA1(bc65f7104d5f7728b68b3dcb45151c41fc30aa0d) )
	ROM_LOAD( "tr-06a.2h", 0xa0000, 0x20000, CRC(b636c23a) SHA1(417e289745996bd00114df6ade591e702265d3a5) )
	ROM_LOAD( "tr-07a.2j", 0xc0000, 0x20000, CRC(5f907d4d) SHA1(1820c5c6e0b078db9c64655c7983ea115ad81036) )
	ROM_LOAD( "tr_08.2l",  0xe0000, 0x20000, CRC(adee35e2) SHA1(6707cf43a697eb9465449a144ae4508afe2e6496) ) /* EPROM */

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "tr-09a.3b", 0x00000, 0x20000, CRC(3d98ad1e) SHA1(f12cdf50e1708ddae092b9784d4319a7d5f092bc) ) /* sprites */
	ROM_LOAD( "tr-10a.2b", 0x20000, 0x20000, CRC(8f6f03d7) SHA1(08a02cfb373040ea5ffbf5604f68df92a1338bb0) )
	ROM_LOAD( "tr-11a.3d", 0x40000, 0x20000, CRC(cd9152e5) SHA1(6df3c43c0c41289890296c2b2aeca915dfdae3b0) )
	ROM_LOAD( "tr-12a.2d", 0x60000, 0x20000, CRC(7d8a99d0) SHA1(af8221cfd2ce9aa3bf296981fb7fddd1e9ef4599) )

	ROM_REGION( 0x08000, "bgmap", 0 )    /* background tilemaps */
	ROM_LOAD( "tr_13.7l", 0x0000, 0x8000, CRC(a79be1eb) SHA1(4191ccd48f7650930f9a4c2be0790239d7420bb1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tr.9e", 0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) - N82S129A or compatible */
ROM_END

ROM_START( toramich )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "tr_02.6j", 0x00000, 0x20000, CRC(b54723b1) SHA1(dfad82e96dff072c967dd59e3db71fb3b43b6dcb) )
	ROM_LOAD16_BYTE( "tr_04.6k", 0x00001, 0x20000, CRC(ab432479) SHA1(b8ec547f7bab67107a7c83931c7ed89142a7af69) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "tr_05.12k", 0x0000, 0x8000, CRC(3ebe6e62) SHA1(6f5708b6ff8c91bc706f73300e0785f15999d570) )

	ROM_REGION( 0x10000, "sample", 0 ) /* samples player */
	ROM_LOAD( "tr_03.11j", 0x0000, 0x10000, CRC(ea1807ef) SHA1(f856e7b592c6df81586821284ea2220468c5ea9d) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "tr_01.10d", 0x00000, 0x08000, CRC(74a9f08c) SHA1(458958c8d9a2af5df88bb24c9c5bcbd37d6856bc) ) /* 8x8 text */

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "tr-01a.3f", 0x00000, 0x20000, CRC(a8aa2e59) SHA1(792f50d688a4ffb574e41257816bc304d41f0458) ) /* tiles */
	ROM_LOAD( "tr-04a.3h", 0x20000, 0x20000, CRC(8863a63c) SHA1(11bfce5b09c5b8a781c658f035d5658c3710d189) )
	ROM_LOAD( "tr-02a.3j", 0x40000, 0x20000, CRC(1a2c5f89) SHA1(2a2aa2f1e2a0cdd4bbdb25236e49c7cc573db9e9) )
	ROM_LOAD( "tr-05.3l",  0x60000, 0x20000, CRC(5bf453b3) SHA1(5eef151974c6b818a17756549d24a702e1f3a859) )
	ROM_LOAD( "tr-03a.2f", 0x80000, 0x20000, CRC(1e0537ea) SHA1(bc65f7104d5f7728b68b3dcb45151c41fc30aa0d) )
	ROM_LOAD( "tr-06a.2h", 0xa0000, 0x20000, CRC(b636c23a) SHA1(417e289745996bd00114df6ade591e702265d3a5) )
	ROM_LOAD( "tr-07a.2j", 0xc0000, 0x20000, CRC(5f907d4d) SHA1(1820c5c6e0b078db9c64655c7983ea115ad81036) )
	ROM_LOAD( "tr_08.2l",  0xe0000, 0x20000, CRC(adee35e2) SHA1(6707cf43a697eb9465449a144ae4508afe2e6496) ) /* EPROM */

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "tr-09a.3b", 0x00000, 0x20000, CRC(3d98ad1e) SHA1(f12cdf50e1708ddae092b9784d4319a7d5f092bc) ) /* sprites */
	ROM_LOAD( "tr-10a.2b", 0x20000, 0x20000, CRC(8f6f03d7) SHA1(08a02cfb373040ea5ffbf5604f68df92a1338bb0) )
	ROM_LOAD( "tr-11a.3d", 0x40000, 0x20000, CRC(cd9152e5) SHA1(6df3c43c0c41289890296c2b2aeca915dfdae3b0) )
	ROM_LOAD( "tr-12a.2d", 0x60000, 0x20000, CRC(7d8a99d0) SHA1(af8221cfd2ce9aa3bf296981fb7fddd1e9ef4599) )

	ROM_REGION( 0x08000, "bgmap", 0 )    /* background tilemaps */
	ROM_LOAD( "tr_13.7l", 0x0000, 0x8000, CRC(a79be1eb) SHA1(4191ccd48f7650930f9a4c2be0790239d7420bb1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tr.9e", 0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) - N82S129A or compatible */
ROM_END

ROM_START( tigeroadb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "tgrroad.3",    0x00000, 0x10000, CRC(14c87e07) SHA1(31363b56dd9d387f3ebd7ca1c209148c389ec1aa) )
	ROM_LOAD16_BYTE( "tgrroad.5",    0x00001, 0x10000, CRC(0904254c) SHA1(9ce7b8a699bc21618032db9b0c5494242ad77a6b) )
	ROM_LOAD16_BYTE( "tgrroad.2",    0x20000, 0x10000, CRC(cedb1f46) SHA1(bc2d5730ff809fb0f38327d72485d472ab9da54d) )
	ROM_LOAD16_BYTE( "tgrroad.4",    0x20001, 0x10000, CRC(e117f0b1) SHA1(ed0050247789bedaeb213c3d7c2d2cdb239bb4b4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "tru05.bin",    0x0000, 0x8000, CRC(f9a7c9bf) SHA1(4d37c71aa6523ac21c6e8b23f9957e75ec4304bf) )

	/* no samples player in the English version */

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "tr01.bin",     0x00000, 0x08000, CRC(74a9f08c) SHA1(458958c8d9a2af5df88bb24c9c5bcbd37d6856bc) ) /* 8x8 text */

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "tr-01a.bin",   0x00000, 0x20000, CRC(a8aa2e59) SHA1(792f50d688a4ffb574e41257816bc304d41f0458) ) /* tiles */
	ROM_LOAD( "tr-04a.bin",   0x20000, 0x20000, CRC(8863a63c) SHA1(11bfce5b09c5b8a781c658f035d5658c3710d189) )
	ROM_LOAD( "tr-02a.bin",   0x40000, 0x20000, CRC(1a2c5f89) SHA1(2a2aa2f1e2a0cdd4bbdb25236e49c7cc573db9e9) )
	ROM_LOAD( "tr05.bin",     0x60000, 0x20000, CRC(5bf453b3) SHA1(5eef151974c6b818a17756549d24a702e1f3a859) )
	ROM_LOAD( "tr-03a.bin",   0x80000, 0x20000, CRC(1e0537ea) SHA1(bc65f7104d5f7728b68b3dcb45151c41fc30aa0d) )
	ROM_LOAD( "tr-06a.bin",   0xa0000, 0x20000, CRC(b636c23a) SHA1(417e289745996bd00114df6ade591e702265d3a5) )
	ROM_LOAD( "tr-07a.bin",   0xc0000, 0x20000, CRC(5f907d4d) SHA1(1820c5c6e0b078db9c64655c7983ea115ad81036) )
	ROM_LOAD( "tgrroad.17",   0xe0000, 0x10000, CRC(3f7539cc) SHA1(ca3ef1fabcb0c7abd7bc211ba128d2433e3dbf26) )
	ROM_LOAD( "tgrroad.18",   0xf0000, 0x10000, CRC(e2e053cb) SHA1(eb9432140fc167dec5d3273112933201be2be1b3) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "tr-09a.bin",   0x00000, 0x20000, CRC(3d98ad1e) SHA1(f12cdf50e1708ddae092b9784d4319a7d5f092bc) ) /* sprites */
	ROM_LOAD( "tr-10a.bin",   0x20000, 0x20000, CRC(8f6f03d7) SHA1(08a02cfb373040ea5ffbf5604f68df92a1338bb0) )
	ROM_LOAD( "tr-11a.bin",   0x40000, 0x20000, CRC(cd9152e5) SHA1(6df3c43c0c41289890296c2b2aeca915dfdae3b0) )
	ROM_LOAD( "tr-12a.bin",   0x60000, 0x20000, CRC(7d8a99d0) SHA1(af8221cfd2ce9aa3bf296981fb7fddd1e9ef4599) )

	ROM_REGION( 0x08000, "bgmap", 0 )    /* background tilemaps */
	ROM_LOAD( "tr13.bin",     0x0000, 0x8000, CRC(a79be1eb) SHA1(4191ccd48f7650930f9a4c2be0790239d7420bb1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "trprom.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) */
ROM_END

ROM_START( f1dream )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "06j_02.bin",   0x00000, 0x20000, CRC(3c2ec697) SHA1(bccb431ad92455484420f91770e91db6d69b09ec) )
	ROM_LOAD16_BYTE( "06k_03.bin",   0x00001, 0x20000, CRC(85ebad91) SHA1(000f5c617417ff20ee9b378166776fecfacdff95) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "12k_04.bin",   0x0000, 0x8000, CRC(4b9a7524) SHA1(19004958c19ac0af35f2c97790b0082ee2c15bc4) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "8751.mcu",     0x0000, 0x1000, CRC(c8e6075c) SHA1(d98bd358d30d22a8009cd2728dde1871a8140c23) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "10d_01.bin",   0x00000, 0x08000, CRC(361caf00) SHA1(8a109e4e116d0c5eea86f9c57c05359754daa5b9) ) /* 8x8 text */

	ROM_REGION( 0x060000, "tiles", 0 )
	ROM_LOAD( "03f_12.bin",   0x00000, 0x10000, CRC(bc13e43c) SHA1(f9528839858d7a45395062a43b71d80400c73173) ) /* tiles */
	ROM_LOAD( "01f_10.bin",   0x10000, 0x10000, CRC(f7617ad9) SHA1(746a0ec433d5246ac4dbae17d6498e3d154e2df1) )
	ROM_LOAD( "03h_14.bin",   0x20000, 0x10000, CRC(e33cd438) SHA1(89a6faea19e8a01b38ba45413609603e559877e9) )
	ROM_LOAD( "02f_11.bin",   0x30000, 0x10000, CRC(4aa49cd7) SHA1(b7052d51a3cb570299f4db1492a1293c4d8b067f) )
	ROM_LOAD( "17f_09.bin",   0x40000, 0x10000, CRC(ca622155) SHA1(00ae4a8e9cad2c42a10b410b594b0e414ada6cfe) )
	ROM_LOAD( "02h_13.bin",   0x50000, 0x10000, CRC(2a63961e) SHA1(a35e9bf0408716f460487a8d2ae336572a98d2fb) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "03b_06.bin",   0x00000, 0x10000, CRC(5e54e391) SHA1(475c968bfeb41b0448e621f59724c7b70d184d36) ) /* sprites */
	ROM_LOAD( "02b_05.bin",   0x10000, 0x10000, CRC(cdd119fd) SHA1(e279ada53f5a1e2ada0195b93399731af213f518) )
	ROM_LOAD( "03d_08.bin",   0x20000, 0x10000, CRC(811f2e22) SHA1(cca7e8cc43408c2c3067a731a98a8a6418a000aa) )
	ROM_LOAD( "02d_07.bin",   0x30000, 0x10000, CRC(aa9a1233) SHA1(c2079ad81d67b54483ea5f69ac2edf276ad58ca9) )

	ROM_REGION( 0x08000, "bgmap", 0 )    /* background tilemaps */
	ROM_LOAD( "07l_15.bin",   0x0000, 0x8000, CRC(978758b7) SHA1(ebd415d70e2f1af3b1bd51f40e7d60f22369638c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "09e_tr.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) */
ROM_END

ROM_START( f1dreamb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "f1d_04.bin",   0x00000, 0x10000, CRC(903febad) SHA1(73726b220ce45e1f13798e50fb6455671f1150f3) )
	ROM_LOAD16_BYTE( "f1d_05.bin",   0x00001, 0x10000, CRC(666fa2a7) SHA1(f38e71293368ddc586f437c38ced1d8ce91527ea) )
	ROM_LOAD16_BYTE( "f1d_02.bin",   0x20000, 0x10000, CRC(98973c4c) SHA1(a73d396a1c3e43e6250d9e0ab1902d6f754d1ed9) )
	ROM_LOAD16_BYTE( "f1d_03.bin",   0x20001, 0x10000, CRC(3d21c78a) SHA1(edee180131a5b4d507ce0490fd3890bdd03ce62f) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "12k_04.bin",   0x0000, 0x8000, CRC(4b9a7524) SHA1(19004958c19ac0af35f2c97790b0082ee2c15bc4) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "10d_01.bin",   0x00000, 0x08000, CRC(361caf00) SHA1(8a109e4e116d0c5eea86f9c57c05359754daa5b9) ) /* 8x8 text */

	ROM_REGION( 0x060000, "tiles", 0 )
	ROM_LOAD( "03f_12.bin",   0x00000, 0x10000, CRC(bc13e43c) SHA1(f9528839858d7a45395062a43b71d80400c73173) ) /* tiles */
	ROM_LOAD( "01f_10.bin",   0x10000, 0x10000, CRC(f7617ad9) SHA1(746a0ec433d5246ac4dbae17d6498e3d154e2df1) )
	ROM_LOAD( "03h_14.bin",   0x20000, 0x10000, CRC(e33cd438) SHA1(89a6faea19e8a01b38ba45413609603e559877e9) )
	ROM_LOAD( "02f_11.bin",   0x30000, 0x10000, CRC(4aa49cd7) SHA1(b7052d51a3cb570299f4db1492a1293c4d8b067f) )
	ROM_LOAD( "17f_09.bin",   0x40000, 0x10000, CRC(ca622155) SHA1(00ae4a8e9cad2c42a10b410b594b0e414ada6cfe) )
	ROM_LOAD( "02h_13.bin",   0x50000, 0x10000, CRC(2a63961e) SHA1(a35e9bf0408716f460487a8d2ae336572a98d2fb) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "03b_06.bin",   0x00000, 0x10000, CRC(5e54e391) SHA1(475c968bfeb41b0448e621f59724c7b70d184d36) ) /* sprites */
	ROM_LOAD( "02b_05.bin",   0x10000, 0x10000, CRC(cdd119fd) SHA1(e279ada53f5a1e2ada0195b93399731af213f518) )
	ROM_LOAD( "03d_08.bin",   0x20000, 0x10000, CRC(811f2e22) SHA1(cca7e8cc43408c2c3067a731a98a8a6418a000aa) )
	ROM_LOAD( "02d_07.bin",   0x30000, 0x10000, CRC(aa9a1233) SHA1(c2079ad81d67b54483ea5f69ac2edf276ad58ca9) )

	ROM_REGION( 0x08000, "bgmap", 0 )    /* background tilemaps */
	ROM_LOAD( "07l_15.bin",   0x0000, 0x8000, CRC(978758b7) SHA1(ebd415d70e2f1af3b1bd51f40e7d60f22369638c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "09e_tr.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) */
ROM_END

ROM_START( f1dreamba )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "3.bin",   0x00000, 0x10000, CRC(bdfbbbec) SHA1(08e058f0e612463a2975c4283b7210a3247a90ad) )
	ROM_LOAD16_BYTE( "5.bin",   0x00001, 0x10000, CRC(cc47cfb2) SHA1(2a6c66f4e7e81550af2d94e4a219a0c03173039e) )
	ROM_LOAD16_BYTE( "2.bin",   0x20000, 0x10000, CRC(a34f63fb) SHA1(db1ce7ff3a2496649d8357c3999c1ea1a06ba043) )
	ROM_LOAD16_BYTE( "4.bin",   0x20001, 0x10000, CRC(f98db083) SHA1(07e3e611eed1a77b7cd99c231e401c18465445ce) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "12k_04.bin",   0x0000, 0x8000, CRC(4b9a7524) SHA1(19004958c19ac0af35f2c97790b0082ee2c15bc4) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "10d_01.bin",   0x00000, 0x08000, CRC(361caf00) SHA1(8a109e4e116d0c5eea86f9c57c05359754daa5b9) ) /* 8x8 text */

	ROM_REGION( 0x060000, "tiles", 0 )
	ROM_LOAD( "03f_12.bin",   0x00000, 0x10000, CRC(bc13e43c) SHA1(f9528839858d7a45395062a43b71d80400c73173) ) /* tiles */
	ROM_LOAD( "01f_10.bin",   0x10000, 0x10000, CRC(f7617ad9) SHA1(746a0ec433d5246ac4dbae17d6498e3d154e2df1) )
	ROM_LOAD( "03h_14.bin",   0x20000, 0x10000, CRC(e33cd438) SHA1(89a6faea19e8a01b38ba45413609603e559877e9) )
	ROM_LOAD( "02f_11.bin",   0x30000, 0x10000, CRC(4aa49cd7) SHA1(b7052d51a3cb570299f4db1492a1293c4d8b067f) )
	ROM_LOAD( "17f_09.bin",   0x40000, 0x10000, CRC(ca622155) SHA1(00ae4a8e9cad2c42a10b410b594b0e414ada6cfe) )
	ROM_LOAD( "02h_13.bin",   0x50000, 0x10000, CRC(2a63961e) SHA1(a35e9bf0408716f460487a8d2ae336572a98d2fb) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "03b_06.bin",   0x00000, 0x10000, CRC(5e54e391) SHA1(475c968bfeb41b0448e621f59724c7b70d184d36) ) /* sprites */
	ROM_LOAD( "02b_05.bin",   0x10000, 0x10000, CRC(cdd119fd) SHA1(e279ada53f5a1e2ada0195b93399731af213f518) )
	ROM_LOAD( "03d_08.bin",   0x20000, 0x10000, CRC(811f2e22) SHA1(cca7e8cc43408c2c3067a731a98a8a6418a000aa) )
	ROM_LOAD( "02d_07.bin",   0x30000, 0x10000, CRC(aa9a1233) SHA1(c2079ad81d67b54483ea5f69ac2edf276ad58ca9) )

	ROM_REGION( 0x08000, "bgmap", 0 )    /* background tilemaps */
	ROM_LOAD( "07l_15.bin",   0x0000, 0x8000, CRC(978758b7) SHA1(ebd415d70e2f1af3b1bd51f40e7d60f22369638c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "09e_tr.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) */
ROM_END


ROM_START( pushman )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pushman.012", 0x000000, 0x10000, CRC(330762bc) SHA1(c769b68da40183e6eb84212636bfd1265e5ed2d8) )
	ROM_LOAD16_BYTE( "pushman.011", 0x000001, 0x10000, CRC(62636796) SHA1(1a205c1b0efff4158439bc9a21cfe3cd8834aef9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pushman.013", 0x00000, 0x08000,  CRC(adfe66c1) SHA1(fa4ed13d655c664b06e9b91292d2c0a88cb5a569) )

	ROM_REGION( 0x01000, "mcu", 0 ) /* Verified same for all 4 currently dumped versions */
	ROM_LOAD( "pushman68705r3p.ic23",  0x00000, 0x01000, CRC(d7916657) SHA1(89c14c6044f082fffe2a8f86d0a82336f4a110a2) )

	ROM_REGION( 0x10000, "text", 0 )
	ROM_LOAD( "pushman.001",  0x00000, 0x08000, CRC(626e5865) SHA1(4ab96c8512f439d18390094d71a898f5c576399c) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "pushman.004", 0x30000, 0x10000, CRC(87aafa70) SHA1(560661b23ddac106a3d2762fc32da666b31e7424) )
	ROM_LOAD( "pushman.005", 0x20000, 0x10000, CRC(7fd1200c) SHA1(15d6781a2d7e3ec2e8f85f8585b1e3fd9fe4fd1d) )
	ROM_LOAD( "pushman.002", 0x10000, 0x10000, CRC(0a094ab0) SHA1(2ff5dcf0d9439eeadd61601170c9767f4d81f022) )
	ROM_LOAD( "pushman.003", 0x00000, 0x10000, CRC(73d1f29d) SHA1(0a87fe02b1efd04c540f016b2626d32da70219db) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "pushman.006", 0x20000, 0x10000, CRC(48ef3da6) SHA1(407d50c2030584bb17a4d4a1bb45e0b04e1a95a4) )
	ROM_LOAD( "pushman.008", 0x30000, 0x10000, CRC(4b6a3e88) SHA1(c57d0528e942dd77a13e5a4bf39053f52915d44c) )
	ROM_LOAD( "pushman.007", 0x00000, 0x10000, CRC(b70020bd) SHA1(218ca4a08b87b7dc5c1eed99960f4098c4fc7e0c) )
	ROM_LOAD( "pushman.009", 0x10000, 0x10000, CRC(cc555667) SHA1(6c79e14fc18d1d836392044779cb3219494a3447) )

	ROM_REGION( 0x10000, "bgmap", 0 )    /* bg tilemaps */
	ROM_LOAD( "pushman.010", 0x00000, 0x08000, CRC(a500132d) SHA1(26b02c9fea69b51c5f7dc1b43b838cd336ebf862) )

	ROM_REGION( 0x0100, "proms", 0 ) /* this is the same as tiger road / f1-dream */
	ROM_LOAD( "n82s129an.ic82",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) ) /* priority (not used) */
ROM_END

ROM_START( pushmana )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pushmana.212", 0x000000, 0x10000, CRC(871d0858) SHA1(690ca554c8c6f19c0f26ccd8d948e3aa6e1b23c0) )
	ROM_LOAD16_BYTE( "pushmana.011", 0x000001, 0x10000, CRC(ae57761e) SHA1(63467d61c967f38e0fbb8130f08e09e03cdcce6c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pushman.013", 0x00000, 0x08000,  CRC(adfe66c1) SHA1(fa4ed13d655c664b06e9b91292d2c0a88cb5a569) ) // missing from this set?

	ROM_REGION( 0x01000, "mcu", 0 ) /* Verified same for all 4 currently dumped versions */
	ROM_LOAD( "pushman68705r3p.ic23",  0x00000, 0x01000, CRC(d7916657) SHA1(89c14c6044f082fffe2a8f86d0a82336f4a110a2) )

	ROM_REGION( 0x10000, "text", 0 )
	ROM_LOAD( "pushmana.130",  0x00000, 0x10000, CRC(f83f92e7) SHA1(37f337d7b496f8d81eed247c80390a6aabcf4b95) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "pushman.004", 0x30000, 0x10000, CRC(87aafa70) SHA1(560661b23ddac106a3d2762fc32da666b31e7424) ) // .58
	ROM_LOAD( "pushman.005", 0x20000, 0x10000, CRC(7fd1200c) SHA1(15d6781a2d7e3ec2e8f85f8585b1e3fd9fe4fd1d) ) // .59
	ROM_LOAD( "pushman.002", 0x10000, 0x10000, CRC(0a094ab0) SHA1(2ff5dcf0d9439eeadd61601170c9767f4d81f022) ) // .56
	ROM_LOAD( "pushman.003", 0x00000, 0x10000, CRC(73d1f29d) SHA1(0a87fe02b1efd04c540f016b2626d32da70219db) ) // .57

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "pushman.006", 0x20000, 0x10000, CRC(48ef3da6) SHA1(407d50c2030584bb17a4d4a1bb45e0b04e1a95a4) ) // .131
	ROM_LOAD( "pushman.008", 0x30000, 0x10000, CRC(4b6a3e88) SHA1(c57d0528e942dd77a13e5a4bf39053f52915d44c) ) // .148
	ROM_LOAD( "pushman.007", 0x00000, 0x10000, CRC(b70020bd) SHA1(218ca4a08b87b7dc5c1eed99960f4098c4fc7e0c) ) // .132
	ROM_LOAD( "pushman.009", 0x10000, 0x10000, CRC(cc555667) SHA1(6c79e14fc18d1d836392044779cb3219494a3447) ) // .149

	ROM_REGION( 0x10000, "bgmap", 0 )    /* bg tilemaps */
	ROM_LOAD( "pushmana.189", 0x00000, 0x10000, CRC(59f25598) SHA1(ace33afd6e6d07376ed01048db99b13bcec790d7) )

	ROM_REGION( 0x0100, "proms", 0 ) /* this is the same as tiger road / f1-dream */
	ROM_LOAD( "n82s129an.ic82",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) ) /* priority (not used) */
ROM_END

ROM_START( pushmans )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pman-12.ic212", 0x000000, 0x10000, CRC(4251109d) SHA1(d4b020e4ecc2005b3a4c1b34d88de82b09bf5a6b) )
	ROM_LOAD16_BYTE( "pman-11.ic197", 0x000001, 0x10000, CRC(1167ed9f) SHA1(ca0296950a75ef15ff6f9d3a776b02180b941d61) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pman-13.ic216", 0x00000, 0x08000, CRC(bc03827a) SHA1(b4d6ae164bbb7ba19e4934392fe2ba29575f28b9) )

	ROM_REGION( 0x01000, "mcu", 0 ) /* Verified same for all 4 currently dumped versions */
	ROM_LOAD( "pushman68705r3p.ic23",  0x00000, 0x01000, CRC(d7916657) SHA1(89c14c6044f082fffe2a8f86d0a82336f4a110a2) )

	ROM_REGION( 0x10000, "text", 0 )
	ROM_LOAD( "pman-1.ic130",  0x00000, 0x08000, CRC(14497754) SHA1(a47d03c56add18c5d9aed221990550b18589ff43) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "pman-4.ic58", 0x30000, 0x10000, CRC(16e5ce6b) SHA1(cb9c6094a853abc550eae29c35083f26a0d1de94) )
	ROM_LOAD( "pman-5.ic59", 0x20000, 0x10000, CRC(b82140b8) SHA1(0a16b904eb2739bfa22a87d03266d3ff2b750b67) )
	ROM_LOAD( "pman-2.56", 0x10000, 0x10000, CRC(2cb2ac29) SHA1(165447ad7eb8593c0d4346096ec13ac386e905c9) )
	ROM_LOAD( "pman-3.57", 0x00000, 0x10000, CRC(8ab957c8) SHA1(143501819920c521353930f83e49f1e19fbba34f) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "pman-6.ic131", 0x20000, 0x10000, CRC(bd0f9025) SHA1(7262410d4631f1b051c605d5cea5b91e9f68327e) )
	ROM_LOAD( "pman-8.ic148", 0x30000, 0x10000, CRC(591bd5c0) SHA1(6e0e18e0912fa38e113420ac31c7f36853b830ec) )
	ROM_LOAD( "pman-7.ic132", 0x00000, 0x10000, CRC(208cb197) SHA1(161633b6b0acf25447a5c0b3c6fbf18adc6e2243) )
	ROM_LOAD( "pman-9.ic149", 0x10000, 0x10000, CRC(77ee8577) SHA1(63d13683dd097d8e7cb71ad3abe04e11f2a58bd3) )

	ROM_REGION( 0x10000, "bgmap", 0 )    /* bg tilemaps */
	ROM_LOAD( "pman-10.ic189", 0x00000, 0x08000, CRC(5f9ae9a1) SHA1(87619918c28c942780f6dbd3818d4cc69932eefc) )

	ROM_REGION( 0x0100, "proms", 0 ) /* this is the same as tiger road / f1-dream */
	ROM_LOAD( "n82s129an.ic82",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) ) /* priority (not used) */
ROM_END

ROM_START( pushmant ) /* Single plane PCB */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "12.ic212", 0x000000, 0x10000, CRC(f5c77d86) SHA1(66d2d5dc9f4662efc5a865c9cc1bba653e86a674) )
	ROM_LOAD16_BYTE( "11.ic197", 0x000001, 0x10000, CRC(2e09ff08) SHA1(9bb05c51c985c3a12fb8d6fd915276e304651eb1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "13.ic216", 0x00000, 0x08000, CRC(adfe66c1) SHA1(fa4ed13d655c664b06e9b91292d2c0a88cb5a569) ) /* Same as the Comad set */

	ROM_REGION( 0x01000, "mcu", 0 ) /* Verified same for all 4 currently dumped versions */
	ROM_LOAD( "pushman68705r3p.ic23",  0x00000, 0x01000, CRC(d7916657) SHA1(89c14c6044f082fffe2a8f86d0a82336f4a110a2) )

	ROM_REGION( 0x10000, "text", 0 )
	ROM_LOAD( "1.ic130",  0x00000, 0x08000, CRC(14497754) SHA1(a47d03c56add18c5d9aed221990550b18589ff43) ) /* Same as the Sammy set */

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "4.ic58", 0x30000, 0x10000, CRC(69209214) SHA1(c5b527234aefbdfb39864806e2b1784fdf2dd49c) )
	ROM_LOAD( "5.ic59", 0x20000, 0x10000, CRC(75fc0ac4) SHA1(aa3a19573b96d89b94bfddda5b404d19cbb47335) )
	ROM_LOAD( "2.ic56", 0x10000, 0x10000, CRC(2bb8093f) SHA1(e86048d676b06796a1d2ed325ea8ea9cada3c4b6) )
	ROM_LOAD( "3.ic57", 0x00000, 0x10000, CRC(5f1c4e7a) SHA1(751caf2365eccbab6d7de5434c4656ac5bd7f13b) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "6.ic131", 0x20000, 0x10000, CRC(bd0f9025) SHA1(7262410d4631f1b051c605d5cea5b91e9f68327e) ) /* These 4 are the same as the Sammy set */
	ROM_LOAD( "8.ic148", 0x30000, 0x10000, CRC(591bd5c0) SHA1(6e0e18e0912fa38e113420ac31c7f36853b830ec) )
	ROM_LOAD( "7.ic132", 0x00000, 0x10000, CRC(208cb197) SHA1(161633b6b0acf25447a5c0b3c6fbf18adc6e2243) )
	ROM_LOAD( "9.ic149", 0x10000, 0x10000, CRC(77ee8577) SHA1(63d13683dd097d8e7cb71ad3abe04e11f2a58bd3) )

	ROM_REGION( 0x10000, "bgmap", 0 )    /* bg tilemaps */
	ROM_LOAD( "10.ic189", 0x00000, 0x08000, CRC(5f9ae9a1) SHA1(87619918c28c942780f6dbd3818d4cc69932eefc) ) /* Same as the Sammy set */

	ROM_REGION( 0x0100, "proms", 0 ) /* this is the same as tiger road / f1-dream */
	ROM_LOAD( "n82s129an.ic82",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) ) /* priority (not used) */
ROM_END

ROM_START( bballs )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb12.m17", 0x000000, 0x10000, CRC(4501c245) SHA1(b03ace135b077e8c226dd3be04fa8e86ad096770) )
	ROM_LOAD16_BYTE( "bb11.l17", 0x000001, 0x10000, CRC(55e45b60) SHA1(103d848ae74b59ac2f5a5c5300323bbf8b109752) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb13.n4", 0x00000, 0x08000, CRC(1ef78175) SHA1(2e7dcbab3a572c2a6bb67a36ba283a5faeb14a88) )

	ROM_REGION( 0x01000, "mcu", 0 ) /* using dump from bballsa set */
	ROM_LOAD( "mc68705r3.bin",  0x00000, 0x01000, CRC(4b37b853) SHA1(c95b7b1dcc6f4730fd08535001e2f02b34ea14c2) BAD_DUMP )

	ROM_REGION( 0x10000, "text", 0 )
	ROM_LOAD( "bb1.g20",  0x00000, 0x08000, CRC(b62dbcb8) SHA1(121613f6d2bcd226e71d4ae71830b9b0d15c2331) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "bb4.d1", 0x30000, 0x10000, CRC(b77de5f8) SHA1(e966f982d712109c4402ca3a8cd2c19640d52bdb) )
	ROM_LOAD( "bb5.d2", 0x20000, 0x10000, CRC(ffffccbf) SHA1(3ac85c06c3dca1de8839fca73f5de3982a3baca0) )
	ROM_LOAD( "bb2.b1", 0x10000, 0x10000, CRC(a5b13236) SHA1(e2d21fa3c878b328238ba8b400f3ab00b0763f6b) )
	ROM_LOAD( "bb3.b2", 0x00000, 0x10000, CRC(e35b383d) SHA1(5312e80d786dc2ffe0f7b1038a64f8ec6e590e0c) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "bb6.h1", 0x20000, 0x10000, CRC(0cada9ce) SHA1(5f2e85baf5f04e874e0857451946c8b1e1c8d209) )
	ROM_LOAD( "bb8.j1", 0x30000, 0x10000, CRC(d55fe7c1) SHA1(de5ba87c0f905e6f1abadde3af63884a8a130806) )
	ROM_LOAD( "bb7.h2", 0x00000, 0x10000, CRC(a352d53b) SHA1(c71e976b7c28630d7af11fffe0d1cfd7d611ee8b) )
	ROM_LOAD( "bb9.j2", 0x10000, 0x10000, CRC(78d185ac) SHA1(6ed6e1f5eeb93129eeeab6bae22b640c9782f7fc) )

	ROM_REGION( 0x10000, "bgmap", 0 )    /* bg tilemaps */
	ROM_LOAD( "bb10.l6", 0x00000, 0x08000, CRC(d06498f9) SHA1(9f33bbc40ebe11c03aec29289f76f1c3ca5bf009) )

	ROM_REGION( 0x0100, "proms", 0 ) /* this is the same as tiger road / f1-dream */
	ROM_LOAD( "bb_prom.e9",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) N82S129 BPROM */
ROM_END

ROM_START( bballsa )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "12.ic212", 0x000000, 0x10000, CRC(8917aedd) SHA1(ded983ba753699c97b02e991eb7195c0122b4065) )
	ROM_LOAD16_BYTE( "11.ic197", 0x000001, 0x10000, CRC(430fca1b) SHA1(70ed70396f5346a3120f7a046a111fa114c997bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "13.ic216", 0x00000, 0x08000, CRC(1ef78175) SHA1(2e7dcbab3a572c2a6bb67a36ba283a5faeb14a88) )

	ROM_REGION( 0x01000, "mcu", 0 )
	ROM_LOAD( "mc68705r3.bin",  0x00000, 0x01000, CRC(4b37b853) SHA1(c95b7b1dcc6f4730fd08535001e2f02b34ea14c2) )

	ROM_REGION( 0x10000, "text", 0 )
	ROM_LOAD( "1.ic130",  0x00000, 0x08000, CRC(67672444) SHA1(f1d4681999d44e8d3cbf26b8a9c05f50573e0df6) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "4.ic58", 0x30000, 0x10000, CRC(144ca816) SHA1(c166e3f42f12c14df977adad363575e786b2be51) )
	ROM_LOAD( "5.ic59", 0x20000, 0x10000, CRC(486c8385) SHA1(4421160573a2435bb0f330a04fef486d14a1293b) )
	ROM_LOAD( "2.ic56", 0x10000, 0x10000, CRC(1d464915) SHA1(3396e919da7e3d08a1a9c76db5d0d214ddf7adcd) )
	ROM_LOAD( "3.ic57", 0x00000, 0x10000, CRC(595439ec) SHA1(6e1a79e5583960f700fd8a63cd48c7e222a315dc) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "6.ic131", 0x20000, 0x10000, CRC(15d4975b) SHA1(71dbb63e70a52ec12fdfc20047a48beb73fac8a0) )
	ROM_LOAD( "8.ic148", 0x30000, 0x10000, CRC(c1a21c75) SHA1(34b877ac85274e50b2ce65945b9761e70e80b852) )
	ROM_LOAD( "7.ic132", 0x00000, 0x10000, CRC(2289393a) SHA1(e1370925a92f7d9f96c9431cf1b8dd262c41017e) )
	ROM_LOAD( "9.ic149", 0x10000, 0x10000, CRC(1fe3d172) SHA1(f7415e8633507a507ec1cd68de224722a726a473) )

	ROM_REGION( 0x10000, "bgmap", 0 )    /* bg tilemaps */
	ROM_LOAD( "10.ic189", 0x00000, 0x08000, CRC(52e4ab27) SHA1(c9ae15f970b4bf120a4bbee9adcf0e5e4de001e7) )

	ROM_REGION( 0x0100, "proms", 0 ) /* this is the same as tiger road / f1-dream */
	ROM_LOAD( "bb_prom.e9",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )    /* priority (not used) N82S129 BPROM */
ROM_END



/***************************************************************************/


GAME( 1987, tigeroad, 0,        tigeroad, tigeroad, tigeroad_state, empty_init, ROT0, "Capcom", "Tiger Road (US)", 0 )
GAME( 1987, tigeroadu,tigeroad, tigeroad, tigeroad, tigeroad_state, empty_init, ROT0, "Capcom (Romstar license)", "Tiger Road (US, Romstar license)", 0 )
GAME( 1987, toramich, tigeroad, toramich, toramich, tigeroad_state, empty_init, ROT0, "Capcom", "Tora e no Michi (Japan)", 0 )
GAME( 1987, tigeroadb,tigeroad, tigeroad, tigeroad, tigeroad_state, empty_init, ROT0, "bootleg", "Tiger Road (US bootleg)", 0 )

/* F1 Dream has an Intel 8751 microcontroller for protection */
GAME( 1988, f1dream,  0,        f1dream,      f1dream,  f1dream_state,  empty_init, ROT0, "Capcom (Romstar license)", "F-1 Dream", 0 )
GAME( 1988, f1dreamb, f1dream,  tigeroad,     f1dream,  tigeroad_state, empty_init, ROT0, "bootleg", "F-1 Dream (bootleg, set 1)", 0 )
GAME( 1988, f1dreamba,f1dream,  tigeroad,     f1dream,  tigeroad_state, empty_init, ROT0, "bootleg", "F-1 Dream (bootleg, set 2)", 0 )

/* This Comad hardware is based around the F1 Dream design */
GAME( 1990, pushman,  0,        pushman, pushman,   pushman_state, empty_init, ROT0, "Comad", "Pushman (Korea, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pushmana, pushman,  pushman, pushman,   pushman_state, empty_init, ROT0, "Comad", "Pushman (Korea, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pushmans, pushman,  pushman, pushman,   pushman_state, empty_init, ROT0, "Comad (American Sammy license)", "Pushman (American Sammy license)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pushmant, pushman,  pushman, pushman,   pushman_state, empty_init, ROT0, "Comad (Top Tronic license)", "Pushman (Top Tronic license)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, bballs,   0,        bballs,  bballs,    pushman_state, empty_init, ROT0, "Comad", "Bouncing Balls", MACHINE_SUPPORTS_SAVE )
GAME( 1991, bballsa,  bballs,   bballs,  bballs,    pushman_state, empty_init, ROT0, "Comad", "Bouncing Balls (Adult)", MACHINE_SUPPORTS_SAVE )
