// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Bagman memory map

driver by Nicola Salmoria
protection and speech emulation by Jarek Burczynski
protection info Andrew Deschenes

memory map:

0000-5fff ROM
6000-67ff RAM
9000-93ff Video RAM
9800-9bff Color RAM
9800-981f Sprites (hidden portion of color RAM)
9c00-9fff ? (filled with 3f, not used otherwise)
c000-ffff ROM (Super Bagman only)

memory mapped ports:

read:
a000      PAL16r6 output. (RD4 line)
a800      ? (read only in one place, not used) (RD5 line)
b000      DSW (RD6 line)
b800      watchdog reset (RD7 line)

write:
a000      interrupt enable
a001      horizontal flip
a002      vertical flip
a003      video enable, not available on earlier hardware revision(s)
a004      coin counter
a007      ? /SCS line in the schems connected to AY8910 pin A4 or AA (schems are unreadable)

a800-a805 these lines control the state machine driving TMS5110 (only bit 0 matters)
          a800,a801,a802 - speech roms BIT select (000 bit 7, 001 bit 4, 010 bit 2)
          a803 - 0 keeps the state machine in reset state; 1 starts speech
          a804 - connected to speech rom 11 (QS) chip enable
          a805 - connected to speech rom 12 (QT) chip enable
b000      ?
b800      ?


PAL16r6 This chip is custom logic used for guards controlling.
        Inputs are connected to buffered address(!!!) lines AB0,AB1,AB2,AB3,AB4,AB5,AB6
        We simulate this writing a800 to a805 there (which is wrong but works)


I/O ports:

I/O 8  ;AY-3-8910 Control Reg.
I/O 9  ;AY-3-8910 Data Write Reg.
I/O C  ;AY-3-8910 Data Read Reg.
        Port A of the 8910 is connected to IN0
        Port B of the 8910 is connected to IN1

DIP locations verified for:
    - bagman (manual)
    - squaitsa (manual)

***************************************************************************/

#include "emu.h"
#include "bagman.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "screen.h"
#include "speaker.h"


void bagman_state::machine_start()
{
	m_video_enable = true;

	save_item(NAME(m_irq_mask));
	save_item(NAME(m_columnvalue));
	save_item(NAME(m_video_enable));
}

void squaitsa_state::machine_start()
{
	bagman_state::machine_start();

	save_item(NAME(m_res));
	save_item(NAME(m_old_val));
}


void bagman_state::ls259_w(offs_t offset, uint8_t data)
{
	pal16r6_w(offset, data); // This is just a simulation

	if (m_tmslatch.found())
		m_tmslatch->write_bit(offset, data & 1);
}

void bagman_state::tmsprom_bit_w(int state)
{
	m_tmsprom->bit_w(7 - ((m_tmslatch->q0_r()<<2) | (m_tmslatch->q1_r()<<1) | (m_tmslatch->q2_r()<<0)));
}

void bagman_state::tmsprom_csq0_w(int state)
{
	m_tmsprom->rom_csq_w(0, state);
}

void bagman_state::tmsprom_csq1_w(int state)
{
	// HACK: Schematics suggest that this LS259 does in fact respond to the master
	// reset signal, which would pull /OE active low on both 2732s at once. How
	// does that situation manage not to overload the circuitry?
	if (state || m_tmslatch->q4_r())
		m_tmsprom->rom_csq_w(1, state);
}

void bagman_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void bagman_state::irq_mask_w(int state)
{
	m_irq_mask = state;
	if (!state)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void bagman_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram();
	map(0x9000, 0x93ff).ram().w(FUNC(bagman_state::videoram_w)).share("videoram");
	map(0x9800, 0x9bff).ram().w(FUNC(bagman_state::colorram_w)).share("colorram"); // Includes spriteram
	map(0x9c00, 0x9fff).nopw();    // Written to, but unused
	map(0xa000, 0xa000).r(FUNC(bagman_state::pal16r6_r));
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xa800, 0xa807).w(FUNC(bagman_state::ls259_w)); // TMS5110 driving state machine
	map(0xb000, 0xb000).portr("DSW");
	map(0xb800, 0xb800).nopr(); // Looks like watchdog from schematics
	map(0xc000, 0xffff).rom(); // Super Bagman only

#if 0
	map(0xb000, 0xb000).nopw(); // ????
	map(0xb800, 0xb800).nopw(); // ????
#endif
}


void pickin_state::pickin_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x7000, 0x77ff).ram();
	map(0x8800, 0x8bff).ram().w(FUNC(pickin_state::videoram_w)).share("videoram");
	map(0x9800, 0x9bff).ram().w(FUNC(pickin_state::colorram_w)).share("colorram"); // Includes spriteram
	map(0x9c00, 0x9fff).nopw(); // Written to in pickin, but unused
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xa800, 0xa800).portr("DSW");
	map(0xb800, 0xb800).r("watchdog", FUNC(watchdog_timer_device::reset_r));
}

void bagman_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x08, 0x09).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x0c, 0x0c).r("aysnd", FUNC(ay8910_device::data_r));
	//map(0x56, 0x56).nopw();
}

uint8_t pickin_state::aysnd_r()
{
	uint8_t data = 0xff;
	if (!m_mainlatch->q5_r())
		data &= m_aysnd[0]->data_r();
	if (!m_mainlatch->q6_r())
		data &= m_aysnd[1]->data_r();
	return data;
}

void pickin_state::aysnd_w(offs_t offset, uint8_t data)
{
	if (!m_mainlatch->q5_r())
		m_aysnd[0]->address_data_w(offset, data);
	if (!m_mainlatch->q6_r())
		m_aysnd[1]->address_data_w(offset, data);
}

void pickin_state::pickin_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x08, 0x09).w(FUNC(pickin_state::aysnd_w));
	map(0x0c, 0x0c).r(FUNC(pickin_state::aysnd_r));
}



static INPUT_PORTS_START( bagman )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "2C/1C 1C/1C 1C/3C 1C/7C" )
	PORT_DIPSETTING(    0x04, "1C/1C 1C/2C 1C/6C 1C/14C" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Language ) )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( French ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_CONFNAME(0x80, 0x80, DEF_STR( Cabinet ) ) // Cabinet type set through edge connector, not dip switch (verified on real PCB)
	PORT_CONFSETTING(   0x80, DEF_STR( Upright ) )
	PORT_CONFSETTING(   0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bagmans )
	PORT_INCLUDE( bagman )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR ( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR ( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sbagman )
	PORT_INCLUDE( bagman )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Double-function button, start and shoot

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // Double-function button, start and shoot
INPUT_PORTS_END

static INPUT_PORTS_START( pickin )
	PORT_INCLUDE( bagman )

	PORT_MODIFY("P1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_MODIFY ("P2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "2C/1C 1C/1C 1C/3C 1C/7C" )
	PORT_DIPSETTING(    0x01, "1C/1C 1C/2C 1C/6C 1C/14C" )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Language ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x40, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( French ) )
	PORT_CONFNAME(0x80, 0x80, DEF_STR( Cabinet ) ) // Sense line on wiring harness
	PORT_CONFSETTING(   0x80, DEF_STR( Upright ) )
	PORT_CONFSETTING(   0x00, DEF_STR( Cocktail ) )

INPUT_PORTS_END

static INPUT_PORTS_START( botanicf )
	PORT_INCLUDE( bagman )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )         PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coinage ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "1C/1C 1C/2C 1C/6C 1C/14C" )
	PORT_DIPSETTING(    0x04, "2C/1C 1C/2C 1C/3C 1C/7C" )
	PORT_DIPNAME( 0x08, 0x08, "Invulnerability Fruits" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
INPUT_PORTS_END

static INPUT_PORTS_START( botanici )
	PORT_INCLUDE( botanicf )

	PORT_MODIFY("P2") // only seems to have 2 coin slots
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // This must be ACTIVE_HIGH or the game fails after you complete a level, protection?

	PORT_MODIFY("DSW") // dipswitches are a bit messy on this set
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coinage ) )                          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "1C/1C 1C/2C" )
	PORT_DIPSETTING(    0x04, "2C/1C 1C/2C" )
	PORT_DIPNAME( 0x18, 0x18, "Invulnerability Fruits" )                    PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "3 (duplicate 1)" )
	PORT_DIPSETTING(    0x18, "3 (duplicate 2)" )
	PORT_DIPNAME( 0x20, 0x20, "Language / Disable Invulnerability Fruits" ) PORT_DIPLOCATION("SW1:6") // Changing this off, even in game, seems to remove all fruits you have?
	PORT_DIPSETTING(    0x20, "Fruits On, English" )
	PORT_DIPSETTING(    0x00, "Fruits Off, Spanish" )
INPUT_PORTS_END

static INPUT_PORTS_START( botanici2 )
	PORT_INCLUDE( botanici )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Spanish ) )
INPUT_PORTS_END


static INPUT_PORTS_START( squaitsa )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(squaitsa_state, dial_input_r<0>)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(squaitsa_state, dial_input_r<1>)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME(    0x01, 0x01, DEF_STR( Coinage ) )    PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME(    0x06, 0x06, "Max Points" )          PORT_DIPLOCATION("SW:2,3")
	PORT_DIPSETTING( 0x06, "7" )
	PORT_DIPSETTING( 0x04, "11" )
	PORT_DIPSETTING( 0x02, "15" )
	PORT_DIPSETTING( 0x00, "21" )
	PORT_DIPNAME(    0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW:4,5")
	PORT_DIPSETTING( 0x00, "Level 1" )
	PORT_DIPSETTING( 0x08, "Level 2" )
	PORT_DIPSETTING( 0x10, "Level 3" )
	PORT_DIPSETTING( 0x18, "Level 4" )
	PORT_DIPNAME(    0x20, 0x20, DEF_STR( Language ) )   PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING( 0x20, DEF_STR( Spanish ) )
	PORT_DIPSETTING( 0x00, DEF_STR( English ) )
	PORT_DIPNAME(    0x40, 0x40, "Body Fault" )          PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x80, 0x00, "Protection?" ) // Left empty in the dips scan
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("DIAL_P1")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("DIAL_P2")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_COCKTAIL
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,                                        // 8*8 characters
	512,                                        // 512 characters
	2,                                          // 2 bits per pixel
	{ 0, 512*8*8 },                             // The two bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },                 // Pretty straightforward layout
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                                         // Every char takes 8 consecutive bytes
};
static const gfx_layout spritelayout =
{
	16,16,                                                                    // 16*16 sprites
	128,                                                                      // 128 sprites
	2,                                                                        // 2 bits per pixel
	{ 0, 128*16*16 },                                                         // The two bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7,                                                 // Pretty straightforward layout
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8                                                                       // Every sprite takes 32 consecutive bytes
};



static GFXDECODE_START( gfx_bagman )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 ) // Char set #1
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,   0, 16 ) // Sprites
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,     0, 16 ) // Char set #2
GFXDECODE_END

static GFXDECODE_START( gfx_pickin )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 ) // Char set #1
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,   0, 16 ) // Sprites
	// No gfx2
GFXDECODE_END


/* squaitsa doesn't map the dial directly, instead it polls the results of the dial through an external circuitry.
   I don't know if the following is correct, there can possibly be multiple solutions for the same problem. */
template <unsigned N> ioport_value squaitsa_state::dial_input_r()
{
	uint8_t const dial_val = m_dial[N]->read();

	if(m_res[N] != 0x03)
		m_res[N] = 0x03;
	else if(dial_val > m_old_val[N])
		m_res[N] = 0x02;
	else if(dial_val < m_old_val[N])
		m_res[N] = 0x01;
	else
		m_res[N] = 0x03;

	m_old_val[N] = dial_val;

	return m_res[N];
}

void bagman_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

void bagman_state::bagman_base(machine_config &config)
{
	Z80(config, m_maincpu, BAGMAN_H0);
	m_maincpu->set_addrmap(AS_PROGRAM, &bagman_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &bagman_state::main_portmap);

	LS259(config, m_mainlatch); // 8H
	m_mainlatch->q_out_cb<0>().set(FUNC(bagman_state::irq_mask_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(bagman_state::flip_screen_x_set));
	m_mainlatch->q_out_cb<2>().set(FUNC(bagman_state::flip_screen_y_set));
	// video enable register not available on earlier hardware revision(s)
	// Bagman is supposed to have glitches during screen transitions
	m_mainlatch->q_out_cb<4>().set(FUNC(bagman_state::coin_counter_w));
	m_mainlatch->q_out_cb<4>().set_nop(); // ????

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(BAGMAN_HCLK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(bagman_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(bagman_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_bagman);
	PALETTE(config, m_palette, FUNC(bagman_state::bagman_palette), 64);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", BAGMAN_H0 / 2));
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.40);
}

void bagman_state::bagman(machine_config &config)
{
	bagman_base(config);

	TMSPROM(config, m_tmsprom, 640000 / 2);  // ROM clock
	m_tmsprom->set_region("5110ctrl"); // PROM memory region - sound region is automatically assigned
	m_tmsprom->set_rom_size(0x1000);   // Individual rom_size
	m_tmsprom->set_pdc_bit(1);         // bit # of pdc line
	// virtual bit 8: constant 0, virtual bit 9:constant 1
	m_tmsprom->set_ctl1_bit(8);        // bit # of ctl1 line
	m_tmsprom->set_ctl2_bit(2);        // bit # of ctl2 line
	m_tmsprom->set_ctl4_bit(8);        // bit # of ctl4 line
	m_tmsprom->set_ctl8_bit(2);        // bit # of ctl8 line
	m_tmsprom->set_reset_bit(6);       // bit # of rom reset
	m_tmsprom->set_stop_bit(7);        // bit # of stop
	m_tmsprom->pdc().set("tms", FUNC(tms5110_device::pdc_w)); // tms pdc func
	m_tmsprom->ctl().set("tms", FUNC(tms5110_device::ctl_w)); // tms ctl func

	tms5110a_device &tms(TMS5110A(config, "tms", 640000));
	tms.m0().set("tmsprom", FUNC(tmsprom_device::m0_w));
	tms.data().set("tmsprom", FUNC(tmsprom_device::data_r));
	tms.add_route(ALL_OUTPUTS, "mono", 1.0);

	LS259(config, m_tmslatch); // 7H
	m_tmslatch->q_out_cb<0>().set(FUNC(bagman_state::tmsprom_bit_w));
	m_tmslatch->q_out_cb<1>().set(FUNC(bagman_state::tmsprom_bit_w));
	m_tmslatch->q_out_cb<2>().set(FUNC(bagman_state::tmsprom_bit_w));
	m_tmslatch->q_out_cb<3>().set("tmsprom", FUNC(tmsprom_device::enable_w));
	m_tmslatch->q_out_cb<4>().set(FUNC(bagman_state::tmsprom_csq0_w));
	m_tmslatch->q_out_cb<5>().set(FUNC(bagman_state::tmsprom_csq1_w));
}

void bagman_state::sbagman(machine_config &config)
{
	bagman(config);
	m_mainlatch->q_out_cb<3>().set(FUNC(bagman_state::video_enable_w));
}

void bagman_state::sbagmani(machine_config &config)
{
	bagman_base(config);
	m_mainlatch->q_out_cb<3>().set(FUNC(bagman_state::video_enable_w));
}

void pickin_state::pickin(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, BAGMAN_H0);
	m_maincpu->set_addrmap(AS_PROGRAM, &pickin_state::pickin_map);
	m_maincpu->set_addrmap(AS_IO, &pickin_state::pickin_portmap);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(pickin_state::irq_mask_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(pickin_state::flip_screen_x_set));
	m_mainlatch->q_out_cb<2>().set(FUNC(pickin_state::flip_screen_y_set));
	m_mainlatch->q_out_cb<3>().set(FUNC(pickin_state::video_enable_w));
	m_mainlatch->q_out_cb<4>().set(FUNC(pickin_state::coin_counter_w));
	m_mainlatch->q_out_cb<5>().set_nop(); // ????
	m_mainlatch->q_out_cb<6>().set_nop(); // ????
	m_mainlatch->q_out_cb<7>().set_nop(); // ????

	WATCHDOG_TIMER(config, "watchdog");

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(BAGMAN_HCLK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(pickin_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(pickin_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pickin);
	PALETTE(config, m_palette, FUNC(pickin_state::bagman_palette), 64);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 1500000));
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.40);

	// Maybe
	ay8910_device &ay2(AY8910(config, "ay2", 1500000));
	ay2.port_a_read_callback().set_constant(0xff); // needed to avoid spurious credits on startup
	ay2.port_b_read_callback().set_constant(0xff);
	ay2.add_route(ALL_OUTPUTS, "mono", 0.40);
}

/*

Botanic
Valadon Automation 1983

z80
6116 - work ram
2x 2114 - screen ram
2x 2114
6x 27ls00 - sprite buffer ram

2x ay8910

18.432mhz crystal

*/


void pickin_state::botanic(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, BAGMAN_H0);
	m_maincpu->set_addrmap(AS_PROGRAM, &pickin_state::pickin_map);
	m_maincpu->set_addrmap(AS_IO, &pickin_state::pickin_portmap);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(pickin_state::irq_mask_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(pickin_state::flip_screen_x_set));
	m_mainlatch->q_out_cb<2>().set(FUNC(pickin_state::flip_screen_y_set));
	m_mainlatch->q_out_cb<3>().set(FUNC(pickin_state::video_enable_w));
	m_mainlatch->q_out_cb<4>().set(FUNC(pickin_state::coin_counter_w));
	m_mainlatch->q_out_cb<5>().set_nop();    // ????
	m_mainlatch->q_out_cb<6>().set_nop();    // ????
	m_mainlatch->q_out_cb<7>().set_nop();    // ????

	WATCHDOG_TIMER(config, "watchdog");

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(BAGMAN_HCLK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(pickin_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(pickin_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bagman);
	PALETTE(config, m_palette, FUNC(pickin_state::bagman_palette), 64);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 1500000));
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.40);

	AY8910(config, "ay2", 1500000).add_route(ALL_OUTPUTS, "mono", 0.40);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bagman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e9_b05.bin",   0x0000, 0x1000, CRC(e0156191) SHA1(bb5f16d49fbe48f3bac118acd1fea51ec4bc5355) )
	ROM_LOAD( "f9_b06.bin",   0x1000, 0x1000, CRC(7b758982) SHA1(c8460023b43fed4aca9c6b987faea334832c5e30) )
	ROM_LOAD( "f9_b07.bin",   0x2000, 0x1000, CRC(302a077b) SHA1(916c4a6ea1e631cc72bdb91ff9d263dcbaf08bb2) )
	ROM_LOAD( "k9_b08.bin",   0x3000, 0x1000, CRC(f04293cb) SHA1(ce6b0ae4088ce28c75d414f506fad2cf2b6920c2) )
	ROM_LOAD( "m9_b09s.bin",  0x4000, 0x1000, CRC(68e83e4f) SHA1(9454564885a1003cee7107db18bedb387b85e9ab) )
	ROM_LOAD( "n9_b10.bin",   0x5000, 0x1000, CRC(1d6579f7) SHA1(3ab54329f516156b1c9f68efbe59c95d5240bc8c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "e1_b02.bin",   0x0000, 0x1000, CRC(4a0a6b55) SHA1(955f8bd4bd9b0fc3c6c359c25ba543ba26c04cbd) )
	ROM_LOAD( "j1_b04.bin",   0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "c1_b01.bin",   0x0000, 0x1000, CRC(705193b2) SHA1(ca9cfd05f9195c2a38e8854012de51b6ee6bb403) )
	ROM_LOAD( "f1_b03s.bin",  0x1000, 0x1000, CRC(dba1eda7) SHA1(26d877028b3a31dd671f9e667316c8a14780ca73) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "r9_b11.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "t9_b12.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagnard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e9_b05.bin",   0x0000, 0x1000, CRC(e0156191) SHA1(bb5f16d49fbe48f3bac118acd1fea51ec4bc5355) )
	ROM_LOAD( "f9_b06.bin",   0x1000, 0x1000, CRC(7b758982) SHA1(c8460023b43fed4aca9c6b987faea334832c5e30) )
	ROM_LOAD( "f9_b07.bin",   0x2000, 0x1000, CRC(302a077b) SHA1(916c4a6ea1e631cc72bdb91ff9d263dcbaf08bb2) )
	ROM_LOAD( "k9_b08.bin",   0x3000, 0x1000, CRC(f04293cb) SHA1(ce6b0ae4088ce28c75d414f506fad2cf2b6920c2) )
	ROM_LOAD( "bagnard.009",  0x4000, 0x1000, CRC(4f0088ab) SHA1(a8009f5b8517ba4d84fbc483b199f2514f24eae8) )
	ROM_LOAD( "bagnard.010",  0x5000, 0x1000, CRC(cd2cac01) SHA1(76749161feb9af2b3e928408a21b93d143915b57) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "e1_b02.bin",   0x0000, 0x1000, CRC(4a0a6b55) SHA1(955f8bd4bd9b0fc3c6c359c25ba543ba26c04cbd) )
	ROM_LOAD( "j1_b04.bin",   0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bagnard.001",  0x0000, 0x1000, CRC(060b044c) SHA1(3121f07adb661663a2303085eea1b662968f8f98) )
	ROM_LOAD( "bagnard.003",  0x1000, 0x1000, CRC(8043bc1a) SHA1(bd2f3dfe26cf8d987d9ecaa41eac4bdc4e16a692) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "r9_b11.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "t9_b12.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagnarda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bagman.005",   0x0000, 0x1000, CRC(98fca49c) SHA1(60bf15d700cf4174ac531c11febf21d69ec02db5) )
	ROM_LOAD( "bagman.006",   0x1000, 0x1000, CRC(8f447432) SHA1(71fee4feb92cdd35dcd3ad9e95ea9f186cb25e25) )
	ROM_LOAD( "bagman.007",   0x2000, 0x1000, CRC(236203a6) SHA1(3d661c135a5036adeaf5fed2be38c97bbc72cd0a) )
	ROM_LOAD( "bagman.008",   0x3000, 0x1000, CRC(8bd8c6cb) SHA1(3d34333b20d8ef189425334985285e0634c5ee23) )
	ROM_LOAD( "bagman.009",   0x4000, 0x1000, CRC(6211ba82) SHA1(6d43e16cc99159b188f93bed7f9afef81c1b7fb3) )
	ROM_LOAD( "bagman.010",   0x5000, 0x1000, CRC(08ed1247) SHA1(172fb0d1b919fb80f5603ebb52779664122f8e94) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bagman.002",   0x0000, 0x1000, CRC(7dc57abc) SHA1(73ae325ac1077936741833d33095ad6375353c31) )
	ROM_LOAD( "bagman.004",   0x1000, 0x1000, CRC(1e21577e) SHA1(fc849c2fbaf7353a44a9f2743ccf6ac1adb8dc62) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bagman.001",   0x0000, 0x1000, CRC(1eb56acd) SHA1(f75f6709006e78417999d423d2078ed80eae73a2) )
	ROM_LOAD( "bagman.003",   0x1000, 0x1000, CRC(0ad82a39) SHA1(30ac0ff5bc63934c3eb572c7c13df324757e5e44) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "r9_b11.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "t9_b12.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagnardi ) // 1983
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lebag_itisa_5.e9",      0x0000, 0x1000, CRC(e0156191) SHA1(bb5f16d49fbe48f3bac118acd1fea51ec4bc5355) )
	ROM_LOAD( "lebag_itisa_6.f9",      0x1000, 0x1000, CRC(edf765e4) SHA1(8fb03297b4e854f8b051cb3b105257ccece6dcff) )
	ROM_LOAD( "lebag_itisa_7.j9",      0x2000, 0x1000, CRC(ca2e2845) SHA1(1751c091cd00d0b559174d68ba23bf810d792852) )
	ROM_LOAD( "lebag_itisa_8.k9",      0x3000, 0x1000, CRC(f212e287) SHA1(8ed4b8e555239862eec2a2e7496054a9eda341ad) )
	ROM_LOAD( "lebag_itisa_9.m9",      0x4000, 0x1000, CRC(5daf3426) SHA1(14f5eaa01353b418a6f90cc07f6e52f910a169c3) )
	ROM_LOAD( "lebag_itisa_10.n9",     0x5000, 0x1000, CRC(423c54be) SHA1(f3ad41142441eb73bd17ea7cbdb7070f02c18cb8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "lebag_itisa_2.e1",      0x0000, 0x1000, CRC(4a0a6b55) SHA1(955f8bd4bd9b0fc3c6c359c25ba543ba26c04cbd) )
	ROM_LOAD( "lebag_itisa_4.j1",      0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "lebag_itisa_1.c1",      0x0000, 0x1000, CRC(14ac1735) SHA1(a0a5d492d9690333cabf2cb21e121934216fc194) )
	ROM_LOAD( "lebag_itisa_3.f1",      0x1000, 0x1000, CRC(8043bc1a) SHA1(bd2f3dfe26cf8d987d9ecaa41eac4bdc4e16a692) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "lebag_itisa_82s123.p3", 0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "lebag_itisa_82s123.r3", 0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "lebag_itisa_82s123.r6", 0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "lebag_itisa_11.r9",     0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "lebag_itisa_12.t9",     0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )

	ROM_REGION (0x104, "plds", 0)
	ROM_LOAD( "lebag_itisa_pal16r6cn.p6", 0x000, 0x104, CRC(13f14bbf) SHA1(b8c4ddf61609465f3a3699dd42796f15a7b17979) )
ROM_END

ROM_START( bagnardio ) // 1982, based on bagnard set with mods for license text
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bagnardi_05.e9",   0x0000, 0x1000, CRC(e0156191) SHA1(bb5f16d49fbe48f3bac118acd1fea51ec4bc5355) ) // == e9_b05.bin
	ROM_LOAD( "bagnardi_06.f9",   0x1000, 0x1000, CRC(2e98c072) SHA1(d1f2341fc0c04f48615cb21a44736c83b7ded3ee) )
	ROM_LOAD( "bagnardi_07.j9",   0x2000, 0x1000, CRC(698f17b3) SHA1(619498e9e06fcde0a1db67f4347e06c4fc669e6c) )
	ROM_LOAD( "bagnardi_08.k9",   0x3000, 0x1000, CRC(f212e287) SHA1(8ed4b8e555239862eec2a2e7496054a9eda341ad) )
	ROM_LOAD( "bagnardi_09.m9",   0x4000, 0x1000, CRC(4f0088ab) SHA1(a8009f5b8517ba4d84fbc483b199f2514f24eae8) ) // == bagnard.009
	ROM_LOAD( "bagnardi_10.n9",   0x5000, 0x1000, CRC(423c54be) SHA1(f3ad41142441eb73bd17ea7cbdb7070f02c18cb8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bagnardi_02.e1",   0x0000, 0x1000, CRC(4a0a6b55) SHA1(955f8bd4bd9b0fc3c6c359c25ba543ba26c04cbd) ) // == e1_b02.bin
	ROM_LOAD( "bagnardi_04.j1",   0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) ) // == j1_b04.bin

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bagnardi_01.c1",  0x0000, 0x1000, CRC(060b044c) SHA1(3121f07adb661663a2303085eea1b662968f8f98) ) // == bagnard.001
	ROM_LOAD( "bagnardi_03.f1",  0x1000, 0x1000, CRC(8043bc1a) SHA1(bd2f3dfe26cf8d987d9ecaa41eac4bdc4e16a692) ) // == bagnard.003

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "bagnardi_11.r9",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) ) // == r9_b11.bin
	ROM_LOAD( "bagnardi_12.t9",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) ) // == t9_b12.bin
ROM_END

/*
Stern Bagman ROM labels follow this format:

BAGMAN      (c)
A5      9F       <-- Revision level and PCB location
1983      STERN
*/
ROM_START( bagmans )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bagman_a4_9e.9e", 0x0000, 0x1000, CRC(5fb0a1a3) SHA1(849cd60b58de9585a78a1c4c1747f666a4a4fcc3) )
	ROM_LOAD( "bagman_a5_9f.9f", 0x1000, 0x1000, CRC(2ddf6bb9) SHA1(151068dddc55163bb6f925f68e5d04e347ded6a5) )
	ROM_LOAD( "bagman_a4_9j.9j", 0x2000, 0x1000, CRC(b2da8b77) SHA1(ea36cd6be42c5548a9a91054aeebb4b985ba15c9) )
	ROM_LOAD( "bagman_a5_9k.9k", 0x3000, 0x1000, CRC(f91d617b) SHA1(a3323b51277e08747701cc4e2d3a9c466e96d4c1) )
	ROM_LOAD( "bagman_a4_9m.9m", 0x4000, 0x1000, CRC(b8e75eb6) SHA1(433fd736512f10bc0879b15821eb55cc41d58d33) ) // == bagman_a2_9m.9m
	ROM_LOAD( "bagman_a5_9n.9n", 0x5000, 0x1000, CRC(68e4b64d) SHA1(55950d7c07c621cafa001d5d3bfec6bbc02712e2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bagman_a2_1e.1e", 0x0000, 0x1000, CRC(f217ac09) SHA1(a9716674401dff27344a01df8121b6b648688680) )
	ROM_LOAD( "bagman_a2_1j.1j", 0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bagman_a2_1c.1c", 0x0000, 0x1000, CRC(f3e11bd7) SHA1(43ee00ff777008c89f619eb183e7c5e63f6c7694) )
	ROM_LOAD( "bagman_a2_1f.1f", 0x1000, 0x1000, CRC(d0f7105b) SHA1(fb382703850a4ded567706e02ebb7f3e22531b7c) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bagman_color_3pa2.3p", 0x0000, 0x0020, CRC(47504204) SHA1(7524ed766cc6d9a158327717d2cf53346ace2392) ) // MMI 6331 BPROM
	ROM_LOAD( "bagman_color_3ra1.3r", 0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) ) // MMI 6331 BPROM

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "bagman_sound_6ra2.6r", 0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110 - MMI 6331 BPROM

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "bagman_a1_9r.9r", 0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "bagman_a1_9t.9t", 0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagmans4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bagman_a4_9e.9e", 0x0000, 0x1000, CRC(5fb0a1a3) SHA1(849cd60b58de9585a78a1c4c1747f666a4a4fcc3) )
	ROM_LOAD( "bagman_a4_9f.9f", 0x1000, 0x1000, CRC(7871206e) SHA1(14d9b7a0779d59a870e0d4b911797dff5435a16c) )
	ROM_LOAD( "bagman_a4_9j.9j", 0x2000, 0x1000, CRC(b2da8b77) SHA1(ea36cd6be42c5548a9a91054aeebb4b985ba15c9) )
	ROM_LOAD( "bagman_a4_9k.9k", 0x3000, 0x1000, CRC(36b6a944) SHA1(270dd2566b36129366adcbdd5a8db396bec7631f) )
	ROM_LOAD( "bagman_a4_9m.9m", 0x4000, 0x1000, CRC(b8e75eb6) SHA1(433fd736512f10bc0879b15821eb55cc41d58d33) ) // == bagman_a2_9m.9m
	ROM_LOAD( "bagman_a4_9n.9n", 0x5000, 0x1000, CRC(83fccb1c) SHA1(7225d738b64a2cdaaec8860017de4229f2852ed2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bagman_a2_1e.1e", 0x0000, 0x1000, CRC(f217ac09) SHA1(a9716674401dff27344a01df8121b6b648688680) )
	ROM_LOAD( "bagman_a2_1j.1j", 0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bagman_a2_1c.1c", 0x0000, 0x1000, CRC(f3e11bd7) SHA1(43ee00ff777008c89f619eb183e7c5e63f6c7694) )
	ROM_LOAD( "bagman_a2_1f.1f", 0x1000, 0x1000, CRC(d0f7105b) SHA1(fb382703850a4ded567706e02ebb7f3e22531b7c) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bagman_color_3pa2.3p", 0x0000, 0x0020, CRC(47504204) SHA1(7524ed766cc6d9a158327717d2cf53346ace2392) ) // MMI 6331 BPROM
	ROM_LOAD( "bagman_color_3ra1.3r", 0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) ) // MMI 6331 BPROM

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "bagman_sound_6ra2.6r", 0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110 - MMI 6331 BPROM

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "bagman_a1_9r.9r", 0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "bagman_a1_9t.9t", 0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagmans3 ) // not compatible with the PAL16R6 emulator in bagman_m.cpp??
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bagman_a2_9e.9e", 0x0000, 0x1000, CRC(5f04d805) SHA1(84bcdfd25634879438429d2b41c491e092388add) )
	ROM_LOAD( "bagman_a3_9f.9f", 0x1000, 0x1000, CRC(136a78aa) SHA1(14e6a556e00b6ebe718f2fe119b372dc7bfa78d9) )
	ROM_LOAD( "bagman_a2_9j.9j", 0x2000, 0x1000, CRC(f94f5626) SHA1(0ec41c4957833e84e9e498fb4269dfd701a2e5b3) )
	ROM_LOAD( "bagman_a2_9k.9k", 0x3000, 0x1000, CRC(31788fc1) SHA1(959af99490b96c390a43c76dc4e09b35ebda3a24) )
	ROM_LOAD( "bagman_a2_9m.9m", 0x4000, 0x1000, CRC(b8e75eb6) SHA1(433fd736512f10bc0879b15821eb55cc41d58d33) )
	ROM_LOAD( "bagman_a3_9n.9n", 0x5000, 0x1000, CRC(ab66d4c1) SHA1(b90ffc7a8e16abcb88bb5d4705622cfafdf08c81) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bagman_a2_1e.1e", 0x0000, 0x1000, CRC(f217ac09) SHA1(a9716674401dff27344a01df8121b6b648688680) )
	ROM_LOAD( "bagman_a2_1j.1j", 0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bagman_a2_1c.1c", 0x0000, 0x1000, CRC(f3e11bd7) SHA1(43ee00ff777008c89f619eb183e7c5e63f6c7694) )
	ROM_LOAD( "bagman_a2_1f.1f", 0x1000, 0x1000, CRC(d0f7105b) SHA1(fb382703850a4ded567706e02ebb7f3e22531b7c) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bagman_color_3pa2.3p", 0x0000, 0x0020, CRC(47504204) SHA1(7524ed766cc6d9a158327717d2cf53346ace2392) ) // MMI 6331 BPROM
	ROM_LOAD( "bagman_color_3ra1.3r", 0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) ) // MMI 6331 BPROM

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "bagman_sound_6ra2.6r", 0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110 - MMI 6331 BPROM

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "bagman_a1_9r.9r", 0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "bagman_a1_9t.9t", 0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagmanj ) // based on Stern's Bagman revision A4 set (bagmans4)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bf8_06.e9", 0x0000, 0x1000, CRC(5fb0a1a3) SHA1(849cd60b58de9585a78a1c4c1747f666a4a4fcc3) ) // 2732  == bagman_a4_9e.9e
	ROM_LOAD( "bf8_07.f9", 0x1000, 0x1000, CRC(7871206e) SHA1(14d9b7a0779d59a870e0d4b911797dff5435a16c) ) // 2732  == bagman_a4_9f.9f
	ROM_LOAD( "bf8_08.j9", 0x2000, 0x1000, CRC(ae037d0a) SHA1(57d287b3968a4e7fdee2a98014dbdf4fae93d157) ) // 2732
	ROM_LOAD( "bf8_09.k9", 0x3000, 0x1000, CRC(36b6a944) SHA1(270dd2566b36129366adcbdd5a8db396bec7631f) ) // 2732  == bagman_a4_9k.9k
	ROM_LOAD( "bf8_10.m9", 0x4000, 0x1000, CRC(b8e75eb6) SHA1(433fd736512f10bc0879b15821eb55cc41d58d33) ) // 2732  == bagman_a2_9m.9m
	ROM_LOAD( "bf8_11.n9", 0x5000, 0x1000, CRC(83fccb1c) SHA1(7225d738b64a2cdaaec8860017de4229f2852ed2) ) // 2732  == bagman_a4_9n.9n

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bf8_03.e1", 0x0000, 0x1000, CRC(f217ac09) SHA1(a9716674401dff27344a01df8121b6b648688680) ) // 2732  == bagman_a2_1e.1e
	ROM_LOAD( "bf8_05.j1", 0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) ) // 2732  == bagman_a2_1j.1j

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bf8_02.c1",    0x0000, 0x1000, CRC(404283ed) SHA1(18613670cf23181089812c02429e222db0340a60) ) // 2732
	ROM_LOAD( "bf8_04-1.f1",  0x1000, 0x1000, CRC(3f5c991e) SHA1(853c629ba0b4739dcb1af669fd600a3d83fb2072) ) // 2732

	ROM_REGION( 0x0040, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "bagman_color_3pa2.3p", 0x0000, 0x0020, BAD_DUMP CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "bagman_color_3ra1.3r", 0x0020, 0x0020, BAD_DUMP CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0020, "5110ctrl", 0) // not dumped for this set
	ROM_LOAD( "bagman_sound_6ra2.6r", 0x0000, 0x0020, BAD_DUMP CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "bf8_12.r9", 0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) ) // 2732  == bagman_a1_9r.9r
	ROM_LOAD( "bf8_13.t9", 0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) ) // 2732  == bagman_a1_9t.9t
ROM_END

ROM_START( botanic2 ) // PCB has Valadon logo with 'bajo licencia Itisa (Palamos)'.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.9e",    0x0000, 0x1000, CRC(c5170449) SHA1(3f85f254f1a318a0b4d6e12d4df756c880751327) ) // 2732
	ROM_LOAD( "6.9f",    0x1000, 0x1000, CRC(33b2df44) SHA1(9d3697bdf0d906b27374a1460cdac715ef10565d) ) // 2732
	ROM_LOAD( "7.9j",    0x2000, 0x1000, CRC(95bade4c) SHA1(09feb3ecba7d8a0b14af66c89423a22dd6efc90d) ) // 2732
	ROM_LOAD( "8.9k",    0x3000, 0x1000, CRC(1c1a184b) SHA1(0abee2934bd6b00944f25ba5da23c81f2874fa65) ) // 2732
	ROM_LOAD( "9.9m",    0x4000, 0x1000, CRC(728a59a4) SHA1(c833e66ce61f1db72dc094274213b7569dd32569) ) // 2732
	ROM_LOAD( "10.9n",   0x5000, 0x1000, CRC(9e43d32b) SHA1(4f28ddbbe2684aa65f04512eb75f4aac825951a5) ) // 2732

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2.1e",   0x0000, 0x1000, CRC(bea449a6) SHA1(fe06208996d15a4d50753fb62a3020063a0a6620) )
	ROM_LOAD( "4.1j",   0x1000, 0x1000, CRC(a5deb8ed) SHA1(b6b38daffdda263a366656168a6d094ad2b1458f) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "1.1c",    0x0000, 0x1000, CRC(a1148d89) SHA1(b1424693cebc410749216457d07bae54b903bc07) )
	ROM_LOAD( "3.1f",    0x1000, 0x1000, CRC(70be5565) SHA1(a7eab667a82d3e7321f393073f29c6e5e865ec6b) )

	ROM_REGION( 0x0040, "proms", 0 ) // the PCB incorrectly had a bagman PROM left in place, we're using the one from the other Botanic sets instead but marked as BAD_DUMP
	ROM_LOAD( "bota_3p.3p",      0x0000, 0x0020, BAD_DUMP CRC(a8a2ddd2) SHA1(fc2da863d13e92f7682f393a08bc9357841ae7ea) )
	ROM_LOAD( "b-tbp18s030.3r",  0x0020, 0x0020, CRC(edf88f34) SHA1(b9c342d51303d552f87df2543a34e38c30acd07c) )

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "82s123.3p",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "11.9r",    0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) ) // 2732
	ROM_LOAD( "12.9t",    0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) ) // 2732

	ROM_REGION( 0x200, "pld", 0 ) // protection related?
	ROM_LOAD( "pal16r6.6p", 0x000, 0x200, NO_DUMP ) // protected, USA handwritten on PAL
ROM_END

ROM_START( sbagman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sb5v5.9e",     0x0000, 0x1000, CRC(b61029ea) SHA1(2166ca62eb1e237b56252b83d0199e58ec1c8dac) )
	ROM_LOAD( "sb6v4.9f",     0x1000, 0x1000, CRC(bb6a6883) SHA1(a65ac9aeac9c6c85bd4e7eca385d875def92b954) )
	ROM_LOAD( "sb7v4.9j",     0x2000, 0x1000, CRC(a62b6b77) SHA1(ae9cb9c148e293519e391cb88eed9d137d80ea57) )
	ROM_LOAD( "sb8v3.9k",     0x3000, 0x1000, CRC(b94fbb73) SHA1(5d676c5d1d864d70d98f0137c4072062a781b3a0) )
	ROM_LOAD( "sb9v3.9m",     0x4000, 0x1000, CRC(601f34ba) SHA1(1b7ee61a341b9a87abe4fe10b0c647a9b0b97d38) )
	ROM_LOAD( "sb10v3.9n",    0x5000, 0x1000, CRC(5f750918) SHA1(3dc44f259e88999dbb95b4d4376281cc81c1ab87) )
	ROM_LOAD( "sb13v5.8d",    0xc000, 0x0e00, CRC(e0e920f6) SHA1(fbdb36e2d3f4c8dd1f27b3f39a4c025fa47df234) )
	ROM_CONTINUE(             0xfe00, 0x0200 )
	ROM_LOAD( "sb14v3.8f",    0xd000, 0x0400, CRC(83b10139) SHA1(8a1880c6ab8a345676fe30465351d69cc1b416b2) )
	ROM_CONTINUE(             0xe400, 0x0200 )
	ROM_CONTINUE(             0xd600, 0x0a00 )
	ROM_LOAD( "sb15v3.8j",    0xe000, 0x0400, CRC(fe924879) SHA1(b80cbf9cba91e553f7685aef348854c02f0619c7) )
	ROM_CONTINUE(             0xd400, 0x0200 )
	ROM_CONTINUE(             0xe600, 0x0a00 )
	ROM_LOAD( "sb16v3.8k",    0xf000, 0x0e00, CRC(b77eb1f5) SHA1(ef94c1b449e3fa230491052fc3bd4db3f1239263) )
	ROM_CONTINUE(             0xce00, 0x0200 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sb2v3.1e", 0x0000, 0x1000, CRC(f4d3d4e6) SHA1(167ad0259578966fe86384df844e69cf2cc77443) )
	ROM_LOAD( "sb4v3.1j", 0x1000, 0x1000, CRC(2c6a510d) SHA1(304064f11e80f4ec471174823b8aaf59844061ac) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sb1v3.1c", 0x0000, 0x1000, CRC(a046ff44) SHA1(af319cfb74e5efe435c26e971de13bd390f4b378) )
	ROM_LOAD( "sb3v3.1f", 0x1000, 0x1000, CRC(a4422da4) SHA1(3aa55ca8c99566c1c9eb097b6d645c4216e09dfb) )

	ROM_REGION( 0x0040, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "p3.bin", 0x0000, 0x0020, BAD_DUMP CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin", 0x0020, 0x0020, BAD_DUMP CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0020, "5110ctrl", 0) // not dumped for this set
	ROM_LOAD( "r6.bin", 0x0000, 0x0020, BAD_DUMP CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "b11v3.9r", 0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "b12v3.9t", 0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( sbagman2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.9e",         0x0000, 0x1000, CRC(1b1d6b0a) SHA1(549161f6adc88fa16339815e05af33ca57815660) )
	ROM_LOAD( "6.9f",         0x1000, 0x1000, CRC(ac49cb82) SHA1(5affa0c03bedf2c9d5368c7f075818e1760c12ae) )
	ROM_LOAD( "7.9j",         0x2000, 0x1000, CRC(9a1c778d) SHA1(a655e25dc9efdf60cc5b34e42c93c4acaa4a7922) )
	ROM_LOAD( "sb8v3.9k",     0x3000, 0x1000, CRC(b94fbb73) SHA1(5d676c5d1d864d70d98f0137c4072062a781b3a0) )
	ROM_LOAD( "sb9v3.9m",     0x4000, 0x1000, CRC(601f34ba) SHA1(1b7ee61a341b9a87abe4fe10b0c647a9b0b97d38) )
	ROM_LOAD( "sb10v3.9n",    0x5000, 0x1000, CRC(5f750918) SHA1(3dc44f259e88999dbb95b4d4376281cc81c1ab87) )
	ROM_LOAD( "13.8d",        0xc000, 0x0e00, CRC(944a4453) SHA1(cd64d9267d2c5cea39464ba9308752c690e7fd24) )
	ROM_CONTINUE(             0xfe00, 0x0200 )
	ROM_LOAD( "sb14v3.8f",    0xd000, 0x0400, CRC(83b10139) SHA1(8a1880c6ab8a345676fe30465351d69cc1b416b2) )
	ROM_CONTINUE(             0xe400, 0x0200 )
	ROM_CONTINUE(             0xd600, 0x0a00 )
	ROM_LOAD( "sb15v3.8j",    0xe000, 0x0400, CRC(fe924879) SHA1(b80cbf9cba91e553f7685aef348854c02f0619c7) )
	ROM_CONTINUE(             0xd400, 0x0200 )
	ROM_CONTINUE(             0xe600, 0x0a00 )
	ROM_LOAD( "sb16v3.8k",    0xf000, 0x0e00, CRC(b77eb1f5) SHA1(ef94c1b449e3fa230491052fc3bd4db3f1239263) )
	ROM_CONTINUE(             0xce00, 0x0200 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sb2v3.1e", 0x0000, 0x1000, CRC(f4d3d4e6) SHA1(167ad0259578966fe86384df844e69cf2cc77443) )
	ROM_LOAD( "sb4v3.1j", 0x1000, 0x1000, CRC(2c6a510d) SHA1(304064f11e80f4ec471174823b8aaf59844061ac) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sb1v3.1c", 0x0000, 0x1000, CRC(a046ff44) SHA1(af319cfb74e5efe435c26e971de13bd390f4b378) )
	ROM_LOAD( "sb3v3.1f", 0x1000, 0x1000, CRC(a4422da4) SHA1(3aa55ca8c99566c1c9eb097b6d645c4216e09dfb) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "p3.bin", 0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin", 0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "r6.bin", 0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "b11v3.9r", 0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "b12v3.9t", 0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

/*
Stern Super Bagman ROM labels follow this format:

S. BAGMAN   (c)
A1      1F       <-- Revision level and PCB location
1984      STERN

Most examples/photos of the PCB show several hand written labels.

*/
ROM_START( sbagmans ) // known to come in the form of a Bagman to Super Bagman conversion kit
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbag_9e.bin",  0x0000, 0x1000, CRC(c19696f2) SHA1(3a40202a97201a123033358f7afcb06f8ac15063) )
	ROM_LOAD( "6.9f",         0x1000, 0x1000, CRC(ac49cb82) SHA1(5affa0c03bedf2c9d5368c7f075818e1760c12ae) )
	ROM_LOAD( "7.9j",         0x2000, 0x1000, CRC(9a1c778d) SHA1(a655e25dc9efdf60cc5b34e42c93c4acaa4a7922) )
	ROM_LOAD( "8.9k",         0x3000, 0x1000, CRC(b94fbb73) SHA1(5d676c5d1d864d70d98f0137c4072062a781b3a0) )
	ROM_LOAD( "sbag_9m.bin",  0x4000, 0x1000, CRC(b21e246e) SHA1(39d2e93ac5240bb45e76c30c535d12e302690dde) )
	ROM_LOAD( "10.9n",        0x5000, 0x1000, CRC(5f750918) SHA1(3dc44f259e88999dbb95b4d4376281cc81c1ab87) )
	ROM_LOAD( "13.8d",        0xc000, 0x0e00, CRC(944a4453) SHA1(cd64d9267d2c5cea39464ba9308752c690e7fd24) )
	ROM_CONTINUE(             0xfe00, 0x0200 )
	ROM_LOAD( "sbag_f8.bin",  0xd000, 0x0400, CRC(0f3e6de4) SHA1(a7e50d210630b500e534d626d76110dee4aeb18d) )
	ROM_CONTINUE(             0xe400, 0x0200 )
	ROM_CONTINUE(             0xd600, 0x0a00 )
	ROM_LOAD( "15.8j",        0xe000, 0x0400, CRC(fe924879) SHA1(b80cbf9cba91e553f7685aef348854c02f0619c7) )
	ROM_CONTINUE(             0xd400, 0x0200 )
	ROM_CONTINUE(             0xe600, 0x0a00 )
	ROM_LOAD( "16.8k",        0xf000, 0x0e00, CRC(b77eb1f5) SHA1(ef94c1b449e3fa230491052fc3bd4db3f1239263) )
	ROM_CONTINUE(             0xce00, 0x0200 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sb2v3.1e", 0x0000, 0x1000, CRC(f4d3d4e6) SHA1(167ad0259578966fe86384df844e69cf2cc77443) ) // hand written:  SB #3  Ver3
	ROM_LOAD( "sb4v3.1j", 0x1000, 0x1000, CRC(2c6a510d) SHA1(304064f11e80f4ec471174823b8aaf59844061ac) ) // hand written:  SB #4  Ver3

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "s._bagman_a1_1c.1c", 0x0000, 0x1000, CRC(262f870a) SHA1(90877b869a7e927cfa4f9729ec3d6eac3a95dc8f) )
	ROM_LOAD( "s._bagman_a1_1f.1f", 0x1000, 0x1000, CRC(350ed0fb) SHA1(c7804e9618ebc88a1e3684a92a98d9a181441a1f) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bagman_color_3pa2.3p", 0x0000, 0x0020, CRC(47504204) SHA1(7524ed766cc6d9a158327717d2cf53346ace2392) ) // MMI 6331 BPROM
	ROM_LOAD( "bagman_color_3ra1.3r", 0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) ) // MMI 6331 BPROM

	ROM_REGION( 0x0020, "5110ctrl", 0)
	ROM_LOAD( "bagman_sound_6ra2.6r", 0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) // State machine driving TMS5110 - MMI 6331 BPROM

	ROM_REGION( 0x2000, "tmsprom", 0 ) // Data for the TMS5110 speech chip
	ROM_LOAD( "bagman_a1_9r.9r", 0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "bagman_a1_9t.9t", 0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

/*
1x Nec D780C on sub PCB
1x AY-3-8910 on main PCB 3s
1x LM3900 on main PCB 1t
1x LM380 on main PCB 0u
1x oscillator 18.432 MHz on main PCB 5a

sub PCB is marked: "10-27 P1"
sound section is heavily modified with jumper wires
*/

ROM_START( sbagmani )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sb1.5d",         0x0000, 0x1000, CRC(5e24f90f) SHA1(a3ab7ddf26b9fd7bf97720febbbda5773d5ef8d9) )
	ROM_LOAD( "sb2.5f",         0x1000, 0x1000, CRC(746ed840) SHA1(ce23c90c04e8ad0c5ec98ffb18dcc2691639a503) )
	ROM_LOAD( "sb3.5h",         0x2000, 0x1000, CRC(fdfc22ce) SHA1(2f6b9056706c3c5d139e66682572cafa9a361c9f) )
	ROM_LOAD( "sb4.5k",         0x3000, 0x1000, CRC(b94fbb73) SHA1(5d676c5d1d864d70d98f0137c4072062a781b3a0) )
	ROM_LOAD( "sb5.5l",         0x4000, 0x1000, CRC(98067a20) SHA1(f0858fd340584336dbb88f97c6423f11c5d83c25) )
	ROM_LOAD( "sb6.5n",         0x5000, 0x1000, CRC(4726e997) SHA1(d1cc118272fc12f6df3398699b76480fb47a3abf) )
	ROM_LOAD( "sb-a.bin",       0xc000, 0x1000, CRC(0d29a52d) SHA1(40c34114ff40c679b8ee414dd157ec0d77becc06) )
	ROM_LOAD( "sb-b.bin",       0xd000, 0x1000, CRC(f48091c4) SHA1(871f5512d671c871328dcd89680cb8b4add0c867) )
	ROM_LOAD( "sb-c.bin",       0xe000, 0x1000, CRC(7648a042) SHA1(ae387ec26edeb6c214f3026c296992b6b43119b7) )
	ROM_LOAD( "sb-d.bin",       0xf000, 0x1000, CRC(ba82bf0c) SHA1(0a8b6eca476bf6c614da60c3e3a70bd8caf7fb65) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sb8.11k",          0x0000, 0x1000, CRC(189d9bd6) SHA1(9f540a8803298a9849811b40eee62ffc6cd403d9) )
	ROM_LOAD( "sb10.11n",         0x1000, 0x1000, CRC(2c6a510d) SHA1(304064f11e80f4ec471174823b8aaf59844061ac) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sb7.11h",         0x0000, 0x1000, CRC(a046ff44) SHA1(af319cfb74e5efe435c26e971de13bd390f4b378) )
	ROM_LOAD( "sb9.11l",         0x1000, 0x1000, CRC(a4422da4) SHA1(3aa55ca8c99566c1c9eb097b6d645c4216e09dfb) )

	ROM_REGION( 0x0060, "proms", 0 ) // the two 6331 are the same as some Crazy Climber bootlegs
	ROM_LOAD( "am27s19dc.6v",    0x0000, 0x0020, CRC(b3fc1505) SHA1(5b94adde0428a26b815c7eb9b3f3716470d349c7) )
	ROM_LOAD( "6331-1n.6u",      0x0020, 0x0020, CRC(b4e827a5) SHA1(31a5a5ad54417a474d22bb16c473415d99a2b6f1) )
	ROM_LOAD( "6331-1n.6t",      0x0040, 0x0020, CRC(ab1940fa) SHA1(8d98e05cbaa6f55770c12e0a9a8ed9c73cc54423) )

	ROM_REGION( 0x0040, "maincpu_proms", 0 )
	ROM_LOAD( "6331-1n.1",      0x0000, 0x0020, CRC(4d222e6f) SHA1(32133a44569a34fd82d56ca696dc6fd7b0b72436) )
	ROM_LOAD( "6331-1n.2",      0x0020, 0x0020, CRC(ecd06ffb) SHA1(6a9073ed371d86f4def4f18cdd50457441aa2abc) )
ROM_END

ROM_START( pickin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9e",           0x0000, 0x1000, CRC(efd0bd43) SHA1(b70a471a809c08286a82934046357fb46556f641) )
	ROM_LOAD( "9f",           0x1000, 0x1000, CRC(b5785a23) SHA1(9eddda5695981cb0470dfea68d5e2e8e220382b1) )
	ROM_LOAD( "9j",           0x2000, 0x1000, CRC(65ee9fd4) SHA1(2efa40c19a7b0644ef4f4b2ce6a025b2b880239d) )
	ROM_LOAD( "9k",           0x3000, 0x1000, CRC(7b23350e) SHA1(dff19602a0e46ca0bcdbdf2a1d61fd2c80ac70e7) )
	ROM_LOAD( "9m",           0x4000, 0x1000, CRC(935a7248) SHA1(d9af4405d51ce1ff6c4b84709dc85c0db88b1d54) )
	ROM_LOAD( "9n",           0x5000, 0x1000, CRC(52485d1d) SHA1(c309eec506f978388463f20d56d958e6639c31e8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "1f",           0x0000, 0x1000, CRC(c5e96ac6) SHA1(b2d740b6d07c765e8eb2dce31fe285a15a9fe597) )
	ROM_LOAD( "1j",           0x1000, 0x1000, CRC(41c4ac1c) SHA1(aac58a9d675a9b70140d82341231bcf6c77c7b41) )

	// No gfx2

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "6331-1.3p",    0x0000, 0x0020, CRC(fac81668) SHA1(5fa369a5c0ad3a2fc068305336e24772b8e84b62) )
	ROM_LOAD( "6331-1.3r",    0x0020, 0x0020, CRC(14ee1603) SHA1(f3c071399606b727ae7dd0bfc21e1c6ca2d43c7c) )
ROM_END

ROM_START( botanic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.9e",     0x0000, 0x1000, CRC(907f01c7) SHA1(156b6b6bbc2176998fb0c18ad453fc42185ae490) )
	ROM_LOAD( "06.9f",    0x1000, 0x1000, CRC(ff2533fb) SHA1(808a1555c16470b87fca0aea73e0291dbe0b9355) )
	ROM_LOAD( "07.9j",    0x2000, 0x1000, CRC(b7c544ef) SHA1(75b5224c313e97c2c02ca7e9bc3f682278cb7a5c) )
	ROM_LOAD( "08.9k",    0x3000, 0x1000, CRC(2df22793) SHA1(d1f27c915e7563abba4d14ec3fd6757a4d6137be) )
	ROM_LOAD( "09.9m",    0x4000, 0x1000, CRC(f7d908ec) SHA1(ee5827f84505c1f37bebf48181d3e7759421fada) )
	ROM_LOAD( "10.9n",    0x5000, 0x1000, CRC(7ce9fbc8) SHA1(cd2ba01470964640fad9ccf6ff23cbd76c0c2aeb) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2.1e",   0x0000, 0x1000, CRC(bea449a6) SHA1(fe06208996d15a4d50753fb62a3020063a0a6620) )
	ROM_LOAD( "4.1j",   0x1000, 0x1000, CRC(a5deb8ed) SHA1(b6b38daffdda263a366656168a6d094ad2b1458f) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "1.1c",    0x0000, 0x1000, CRC(a1148d89) SHA1(b1424693cebc410749216457d07bae54b903bc07) )
	ROM_LOAD( "3.1f",    0x1000, 0x1000, CRC(70be5565) SHA1(a7eab667a82d3e7321f393073f29c6e5e865ec6b) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "prom.3p",    0x0000, 0x0020, CRC(a8a2ddd2) SHA1(fc2da863d13e92f7682f393a08bc9357841ae7ea) )
	ROM_LOAD( "prom.3r",    0x0020, 0x0020, CRC(edf88f34) SHA1(b9c342d51303d552f87df2543a34e38c30acd07c) )
ROM_END

ROM_START( botanicf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bota_05.9e",    0x0000, 0x1000, CRC(cc66e6f8) SHA1(251481b16f8925a11f02f49e5a79f6524460aa6c) )
	ROM_LOAD( "bota_06.9f",    0x1000, 0x1000, CRC(59892f41) SHA1(eb01601a9163679560b878366aaf7cc0fb54a3e9) )
	ROM_LOAD( "bota_07.9j",    0x2000, 0x1000, CRC(b7c544ef) SHA1(75b5224c313e97c2c02ca7e9bc3f682278cb7a5c) )
	ROM_LOAD( "bota_08.9k",    0x3000, 0x1000, CRC(0afea479) SHA1(d69b2263b4ed09d8f4e40f379aa4a64187a75a52) )
	ROM_LOAD( "bota_09.9m",    0x4000, 0x1000, CRC(2da36120) SHA1(359d7747d8b7c7b4ce876fed722f19dc20e58b89) )
	ROM_LOAD( "bota_10.9n",    0x5000, 0x1000, CRC(7ce9fbc8) SHA1(cd2ba01470964640fad9ccf6ff23cbd76c0c2aeb) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bota_02.1e",   0x0000, 0x1000, CRC(bea449a6) SHA1(fe06208996d15a4d50753fb62a3020063a0a6620) )
	ROM_LOAD( "bota_04.1j",   0x1000, 0x1000, CRC(a5deb8ed) SHA1(b6b38daffdda263a366656168a6d094ad2b1458f) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bota_01.1c",    0x0000, 0x1000, CRC(a1148d89) SHA1(b1424693cebc410749216457d07bae54b903bc07) )
	ROM_LOAD( "bota_03.1f",    0x1000, 0x1000, CRC(70be5565) SHA1(a7eab667a82d3e7321f393073f29c6e5e865ec6b) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bota_3p.3p",    0x0000, 0x0020, CRC(a8a2ddd2) SHA1(fc2da863d13e92f7682f393a08bc9357841ae7ea) )
	ROM_LOAD( "bota_3a.3a",    0x0020, 0x0020, CRC(edf88f34) SHA1(b9c342d51303d552f87df2543a34e38c30acd07c) )
ROM_END

/*

Squash (Itisa)

Anno    1984
Produttore  Itisa-Valadon-gecas

CPU

1x SGS Z8400AB1-Z80ACPU (main)
2x AY-3-8910 (sound)
1x LM380 (sound)
1x oscillator 18432
ROMs

7x 2732
2x MMI6331
Note

1x 22x2 edge connector
1x trimmer (volume)
1x 8 switches dip

This is a strange thing: the PCB is marked "Valadon Automation (C) 1983" and "Fabrique
sous license par GECAS/MILANO" (manufactured under license from GECAS/MILANO)

But if you look in rom 7 with an hex editor you can see the following: "(C) 1984 ITISA"
and "UN BONJOUR A JACQUES DE PEPE PETIT ET HENK" (a good morning to Jacques from Pepe
Petit and Henk). These are the programmers in ITISA, Henk Spits, Josep M. Petit, Josep
Morillas, the very same 3 persons working on BOTANIC (1984)(ITISA).

Game writings in the eprom are in English and Spanish.

So we have an English/Spanish game with a French easter egg on a French PCB manufactured
under license from an Italian company! Let's call it melting pot!

*/

ROM_START( squaitsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sq5.3.9e",    0x0000, 0x1000,CRC(04128d92) SHA1(ca7b7c4be5f40bcefc92b231ce3bba859c9967ee) )
	ROM_LOAD( "sq6.4.9f",    0x1000, 0x1000,CRC(4ff7dd56) SHA1(1955675a9ee3ad7b9185cd027bc42284e15c7451) )
	ROM_LOAD( "sq7.5.9j",    0x2000, 0x1000,CRC(e46ecda6) SHA1(25cd94b6c9602cc00fe3459b524639fd3beb72be) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sq2.1.1e",   0x0000, 0x1000,CRC(0eb6ecad) SHA1(da2facbfa5f2fe233ea09777e9880b4f1d3c1079) )
	ROM_LOAD( "sq4.2.1j",   0x1000, 0x1000,CRC(8d875b0e) SHA1(f949da71167aa81c1cfaefc6f3d88b57792b6191) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sq1.1c",    0x0000, 0x1000,CRC(b6d563e5) SHA1(90a89fd8e892a612c74bd2c7e38acb08c22c6046) )
	ROM_LOAD( "sq3.1f",    0x1000, 0x1000,CRC(0d9d87e6) SHA1(881039d3b8805bb1a546e28abda3273e79714033) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "mmi6331.3p",    0x0000, 0x0020,CRC(06eab7ce) SHA1(d0bafedb340bf12d81446cc672307bb01e5d3026) )
	ROM_LOAD( "mmi6331.3r",    0x0020, 0x0020,CRC(86c1e7db) SHA1(5c974b51d770a555ddab5c23f03a666c6f286cbf) )
ROM_END


void bagman_state::init_bagmans3()
{
	// this earlier version has extra code at 0x5f98 - 0x5fa5 that reads a value from $ed01. Returning 0x01 allows starting a game and gives correct music tempo. TODO: What happens here? Fix this workaround
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xed01, 0xed01, read8smo_delegate(*this, []() { return 0x01; }, "hack_r"));
}

void bagman_state::init_botanic2()
{
	// the protection PAL here must have been changed, the code checks for a fixed value of 0x0b
	// if this isn't returned the title screen bank doesn't get set correctly, there is a garbage enemy
	// tile at the top left corner of the 2nd screen, and the player moves very slowly on the 2nd stage
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa000, 0xa000, read8smo_delegate(*this, []() { return 0x0b; }, "prot_r"));
}

GAME( 1982, bagman,    0,       bagman,   bagman,    bagman_state,   empty_init,    ROT270, "Valadon Automation",                             "Bagman",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1982, bagnard,   bagman,  bagman,   bagman,    bagman_state,   empty_init,    ROT270, "Valadon Automation",                             "Le Bagnard (set 1)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1982, bagnarda,  bagman,  bagman,   bagman,    bagman_state,   empty_init,    ROT270, "Valadon Automation",                             "Le Bagnard (set 2)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1983, bagnardi,  bagman,  bagman,   bagman,    bagman_state,   empty_init,    ROT90,  "Valadon Automation (Itisa license)",             "Le Bagnard (Itisa, Spain)",               MACHINE_SUPPORTS_SAVE )
GAME( 1982, bagnardio, bagman,  bagman,   bagman,    bagman_state,   empty_init,    ROT90,  "Valadon Automation (Itisa license)",             "Le Bagnard (Itisa, Spain, older)",        MACHINE_SUPPORTS_SAVE )
GAME( 1982, bagmans,   bagman,  bagman,   bagmans,   bagman_state,   empty_init,    ROT270, "Valadon Automation (Stern Electronics license)", "Bagman (Stern Electronics, revision A5)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, bagmans4,  bagman,  bagman,   bagman,    bagman_state,   empty_init,    ROT270, "Valadon Automation (Stern Electronics license)", "Bagman (Stern Electronics, revision A4)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, bagmans3,  bagman,  bagman,   bagman,    bagman_state,   init_bagmans3, ROT270, "Valadon Automation (Stern Electronics license)", "Bagman (Stern Electronics, revision A3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) // see init_bagmans3(). Not sure it's actually protection
GAME( 1982, bagmanj,   bagman,  bagman,   bagman,    bagman_state,   empty_init,    ROT270, "Valadon Automation (Taito license)",             "Bagman (Taito)",                          MACHINE_SUPPORTS_SAVE ) // Title screen actually doesn't mention Valadon, only Stern and Taito

GAME( 1984, sbagman,   0,       sbagman,  sbagman,   bagman_state,   empty_init,    ROT270, "Valadon Automation",                             "Super Bagman (version 5)",                MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbagman2,  sbagman, sbagman,  sbagman,   bagman_state,   empty_init,    ROT270, "Valadon Automation",                             "Super Bagman (version 3?)",               MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbagmani,  sbagman, sbagmani, sbagman,   bagman_state,   empty_init,    ROT90,  "Valadon Automation (Itisa license)",             "Super Bagman (Itisa, Spain)",             MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // Different color PROMs, needs correct decoding
GAME( 1984, sbagmans,  sbagman, sbagman,  sbagman,   bagman_state,   empty_init,    ROT270, "Valadon Automation (Stern Electronics license)", "Super Bagman (Stern Electronics)",        MACHINE_SUPPORTS_SAVE )

GAME( 1983, pickin,    0,       pickin,   pickin,    pickin_state,   empty_init,    ROT270, "Valadon Automation",                             "Pickin'",                                 MACHINE_SUPPORTS_SAVE )

GAME( 1983, botanic,   0,       botanic,  botanici,  pickin_state,   empty_init,    ROT90,  "Itisa",                                          "Botanic (English / Spanish)",             MACHINE_SUPPORTS_SAVE )
GAME( 1983, botanic2,  botanic, bagman,   botanici2, bagman_state,   init_botanic2, ROT90,  "Itisa",                                          "Botanic (English / Spanish, Bagman conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, botanicf,  botanic, botanic,  botanicf,  pickin_state,   empty_init,    ROT270, "Itisa (Valadon Automation license)",             "Botanic (French)",                        MACHINE_SUPPORTS_SAVE )

GAME( 1984, squaitsa,  0,       botanic,  squaitsa,  squaitsa_state, empty_init,    ROT0,   "Itisa",                                          "Squash (Itisa)",                          MACHINE_SUPPORTS_SAVE )
