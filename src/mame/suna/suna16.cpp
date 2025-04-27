// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -=  SunA 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU:    68000   +  Z80 [Music]  +  Z80 x 2 [4 Bit PCM]
Sound:  YM2151  +  DAC x 4


-------------------------------------------------------------------------------------------
Year + Game                 By      Board      Hardware
-------------------------------------------------------------------------------------------
94  Best Of Best            SunA    KRB-0026   68000 + Z80 x 2 + YM3526 + DAC x 4 + AY-8910
94  Suna Quiz 6000 Academy  SunA    KRB-0027A  68000 + Z80 x 2 + YM2151 + DAC x 2
96  Back Street Soccer      SunA    KRB-0031   68000 + Z80 x 3 + YM2151 + DAC x 4
96  Back Street Soccer      SunA    KRB-0032A  68000 + Z80 x 3 + YM2151 + DAC x 4
96  Ultra Balloon           SunA    KRB-0033A  68000 + Z80 x 2 + YM2151 + DAC x 2
-------------------------------------------------------------------------------------------


***************************************************************************/

#include "emu.h"
#include "suna16.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"
#include "speaker.h"


/***************************************************************************


                                Main CPU


***************************************************************************/

void suna16_state::soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_soundlatch->write(data & 0xff);
	}
	if (data & ~0xff)   logerror("CPU#0 PC %06X - Sound latch unknown bits: %04X\n", m_maincpu->pc(), data);
}


void suna16_state::bssoccer_leds_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_leds[0] = BIT(data, 0);
		m_leds[1] = BIT(data, 1);
		m_leds[2] = BIT(data, 2);
		m_leds[3] = BIT(data, 3);
		machine().bookkeeping().coin_counter_w(0, data & 0x10);
	}
	if (data & ~0x1f)   logerror("CPU#0 PC %06X - Leds unknown bits: %04X\n", m_maincpu->pc(), data);
}


void suna16_state::uballoon_leds_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		m_leds[0] = BIT(data, 1);
		m_leds[1] = BIT(data, 2);
	}
	if (data & ~0x07)   logerror("CPU#0 PC %06X - Leds unknown bits: %04X\n", m_maincpu->pc(), data);
}


void suna16_state::bestbest_coin_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x04);
	}
	if (data & ~0x04)   logerror("CPU#0 PC %06X - Leds unknown bits: %04X\n", m_maincpu->pc(), data);
}


/***************************************************************************
                            Back Street Soccer
***************************************************************************/

void suna16_state::bssoccer_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom(); // ROM
	map(0x200000, 0x203fff).ram(); // RAM
	map(0x400000, 0x4001ff).rw(FUNC(suna16_state::paletteram_r), FUNC(suna16_state::paletteram_w));  // Banked Palette
	map(0x400200, 0x400fff).ram(); //
	map(0x600000, 0x61ffff).ram().share("spriteram");   // Sprites
	map(0xa00000, 0xa00001).portr("P1").w(FUNC(suna16_state::soundlatch_w));   // To Sound CPU
	map(0xa00002, 0xa00003).portr("P2").w(FUNC(suna16_state::flipscreen_w));   // Flip Screen
	map(0xa00004, 0xa00005).portr("P3").w(FUNC(suna16_state::bssoccer_leds_w));   // Leds
	map(0xa00006, 0xa00007).portr("P4").nopw(); // ? IRQ 1 Ack
	map(0xa00008, 0xa00009).portr("DSW1").nopw();   // ? IRQ 2 Ack
	map(0xa0000a, 0xa0000b).portr("DSW2");
}


/***************************************************************************
                                Ultra Balloon
***************************************************************************/

uint8_t suna16_state::uballoon_prot_r(offs_t offset)
{
	uint8_t ret = 0;

	switch (offset)
	{
		case 0x0011/2:
			ret  = ((m_prot & 0x03) == 0x03) ? 2 : 0;
			ret |= ((m_prot & 0x30) == 0x30) ? 1 : 0;
			break;

		case 0x0311/2:
			ret = 0x03;
			break;

		default:
			//logerror("uballoon_prot_r %04X\n", offset);
			break;
	}

	return ret;
}

void suna16_state::uballoon_prot_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x0001/2:
			m_prot = data;
			break;

		default:
			//logerror("uballoon_prot_w %04X=%02X\n", offset, data);
			break;
	}
}

void suna16_state::uballoon_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom(); // ROM
	map(0x800000, 0x803fff).ram(); // RAM
	map(0x200000, 0x2001ff).rw(FUNC(suna16_state::paletteram_r), FUNC(suna16_state::paletteram_w)); // Banked Palette
	map(0x200200, 0x200fff).ram(); //
	map(0x400000, 0x41ffff).mirror(0x1e0000).ram().share("spriteram");   // Sprites
	map(0x600000, 0x600001).portr("P1").w(FUNC(suna16_state::soundlatch_w));   // To Sound CPU
	map(0x600002, 0x600003).portr("P2");
	map(0x600004, 0x600005).portr("DSW1").w(FUNC(suna16_state::flipscreen_w)); // Flip Screen
	map(0x600006, 0x600007).portr("DSW2");
	map(0x600008, 0x600009).w(FUNC(suna16_state::uballoon_leds_w));  // Leds
	map(0x60000c, 0x60000d).nopw();    // ? IRQ 1 Ack
	map(0x600010, 0x600011).nopw();    // ? IRQ 1 Ack
	map(0xa00000, 0xa0ffff).rw(FUNC(suna16_state::uballoon_prot_r), FUNC(suna16_state::uballoon_prot_w)).umask16(0x00ff);    // Protection
}


/***************************************************************************
                            Suna Quiz 6000 Academy
***************************************************************************/

void suna16_state::sunaq_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom(); // ROM
	map(0x500000, 0x500001).portr("P1").w(FUNC(suna16_state::soundlatch_w));   // To Sound CPU
	map(0x500002, 0x500003).portr("P2").w(FUNC(suna16_state::flipscreen_w));   // Flip Screen
	map(0x500004, 0x500005).portr("DSW1");
	map(0x500006, 0x500007).portr("DSW2");               // (unused?)
	map(0x540000, 0x5401ff).rw(FUNC(suna16_state::paletteram_r), FUNC(suna16_state::paletteram_w));
	map(0x540200, 0x540fff).ram();   // RAM
	map(0x580000, 0x583fff).ram(); // RAM
	map(0x5c0000, 0x5dffff).ram().share("spriteram");   // Sprites
}


/***************************************************************************
                            Best Of Best
***************************************************************************/

uint8_t suna16_state::bestbest_prot_r()
{
	return m_prot;
}

void suna16_state::bestbest_prot_w(uint8_t data)
{
	switch (data)
	{
		case 0x00:  m_prot = m_prot ^ 0x0009;   break;
		case 0x08:  m_prot = m_prot ^ 0x0002;   break;
		case 0x0c:  m_prot = m_prot ^ 0x0003;   break;
		//default:    logerror("CPU#0 PC %06X - Unknown protection value: %04X\n", m_maincpu->pc(), data);
	}
}

void suna16_state::bestbest_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0xc0000);        // ROM
	map(0x200000, 0x2fffff).rom().region("user1", 0);     // ROM
	map(0x500000, 0x500001).portr("P1").w(FUNC(suna16_state::soundlatch_w));     // To Sound CPU
	map(0x500002, 0x500003).portr("P2").w(FUNC(suna16_state::bestbest_flipscreen_w));   // P2 + Coins, Flip Screen
	map(0x500004, 0x500005).portr("DSW").w(FUNC(suna16_state::bestbest_coin_w));        // Coin Counter
	map(0x500009, 0x500009).w(FUNC(suna16_state::bestbest_prot_w));       // Protection
	map(0x500019, 0x500019).r(FUNC(suna16_state::bestbest_prot_r));        // "
	map(0x540000, 0x540fff).rw(FUNC(suna16_state::paletteram_r), FUNC(suna16_state::paletteram_w));  // Banked(?) Palette
	map(0x541000, 0x54ffff).ram();                                                       //
	map(0x580000, 0x58ffff).ram();                           // RAM
	map(0x5c0000, 0x5dffff).ram().share("spriteram"); // Sprites (Chip 1)
	map(0x5e0000, 0x5fffff).ram().share("spriteram2");    // Sprites (Chip 2)
}

MACHINE_START_MEMBER(suna16_state,bestbest)
{
	save_item(NAME(m_prot));
}


/***************************************************************************


                                    Z80 #1

        Plays the music (YM2151) and controls the 2 Z80s in charge
        of playing the PCM samples


***************************************************************************/

/***************************************************************************
                            Back Street Soccer
***************************************************************************/

void suna16_state::bssoccer_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); // ROM
	map(0xf000, 0xf7ff).ram(); // RAM
	map(0xf800, 0xf801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));   // YM2151
	map(0xfc00, 0xfc00).r(m_soundlatch, FUNC(generic_latch_8_device::read)); // From Main CPU
	map(0xfd00, 0xfd00).w("soundlatch2", FUNC(generic_latch_8_device::write));   // To PCM Z80 #1
	map(0xfe00, 0xfe00).w("soundlatch3", FUNC(generic_latch_8_device::write));   // To PCM Z80 #2
}

/***************************************************************************
                                Ultra Balloon
***************************************************************************/

void suna16_state::uballoon_sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom(); // ROM
	map(0xf000, 0xf7ff).ram(); // RAM
	map(0xf800, 0xf801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));   // YM2151
	map(0xfc00, 0xfc00).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w("soundlatch2", FUNC(generic_latch_8_device::write));    // To PCM Z80
}

/***************************************************************************
                            Suna Quiz 6000 Academy
***************************************************************************/

void suna16_state::sunaq_sound_map(address_map &map)
{
	map(0x0000, 0xe82f).rom(); // ROM
	map(0xe830, 0xf7ff).ram(); // RAM (writes to efxx, could be a program bug tho)
	map(0xf800, 0xf801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));   // YM2151
	map(0xfc00, 0xfc00).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w("soundlatch2", FUNC(generic_latch_8_device::write));    // To PCM Z80
}

/***************************************************************************
                            Best Of Best
***************************************************************************/

void suna16_state::bestbest_sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();                                   // ROM
	map(0xc000, 0xc001).w("ymsnd", FUNC(ym3526_device::write));
	map(0xc002, 0xc003).w("aysnd", FUNC(ay8910_device::address_data_w));   // AY8910
	map(0xe000, 0xe7ff).ram();                                   // RAM
	map(0xf000, 0xf000).w("soundlatch2", FUNC(generic_latch_8_device::write));   // To PCM Z80
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));   // From Main CPU
}

/***************************************************************************


                                Z80 #2 & #3

        Dumb PCM samples players (e.g they don't even have RAM!)


***************************************************************************/

/***************************************************************************
                            Back Street Soccer
***************************************************************************/

MACHINE_START_MEMBER(suna16_state, bssoccer)
{
	m_leds.resolve();

	m_bank1->configure_entries(0, 8, memregion("pcm1")->base() + 0x1000, 0x10000);
	m_bank2->configure_entries(0, 8, memregion("pcm2")->base() + 0x1000, 0x10000);
}

MACHINE_START_MEMBER(suna16_state, sunaq)
{
	m_leds.resolve();

	m_bank1->configure_entries(0, 8, memregion("pcm1")->base() + 0x1000, 0x10000);
}

/* Bank Switching */

void suna16_state::bssoccer_pcm_1_bankswitch_w(uint8_t data)
{
	const int bank = data & 7;
	if (bank & ~7)  logerror("CPU#2 PC %06X - ROM bank unknown bits: %02X\n", m_pcm1->pc(), data);
	m_bank1->set_entry(bank);
}

void suna16_state::bssoccer_pcm_2_bankswitch_w(uint8_t data)
{
	const int bank = data & 7;
	if (bank & ~7)  logerror("CPU#3 PC %06X - ROM bank unknown bits: %02X\n", m_pcm2->pc(), data);
	m_bank2->set_entry(bank);
}



/* Memory maps: Yes, *no* RAM */

void suna16_state::bssoccer_pcm_1_map(address_map &map)
{
	map(0x0000, 0x0fff).rom(); // ROM
	map(0x1000, 0xffff).bankr("bank1");    // Banked ROM
}

void suna16_state::bssoccer_pcm_2_map(address_map &map)
{
	map(0x0000, 0x0fff).rom(); // ROM
	map(0x1000, 0xffff).bankr("bank2");    // Banked ROM
}



void suna16_state::bssoccer_pcm_1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch2", FUNC(generic_latch_8_device::read));    // From The Sound Z80
	map(0x00, 0x00).w("ldac", FUNC(dac_byte_interface::data_w));
	map(0x01, 0x01).w("rdac", FUNC(dac_byte_interface::data_w));
	map(0x03, 0x03).w(FUNC(suna16_state::bssoccer_pcm_1_bankswitch_w));  // Rom Bank
}

void suna16_state::bssoccer_pcm_2_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch3", FUNC(generic_latch_8_device::read));    // From The Sound Z80
	map(0x00, 0x00).w("ldac2", FUNC(dac_byte_interface::data_w));
	map(0x01, 0x01).w("rdac2", FUNC(dac_byte_interface::data_w));
	map(0x03, 0x03).w(FUNC(suna16_state::bssoccer_pcm_2_bankswitch_w));  // Rom Bank
}


/***************************************************************************
                                Ultra Balloon
***************************************************************************/

/* Bank Switching */

void suna16_state::uballoon_pcm_1_bankswitch_w(uint8_t data)
{
	const int bank = data & 1;
	if (bank & ~1)  logerror("CPU#2 PC %06X - ROM bank unknown bits: %02X\n", m_pcm1->pc(), data);
	m_bank1->set_entry(bank);
}

/* Memory maps: Yes, *no* RAM */

void suna16_state::uballoon_pcm_1_map(address_map &map)
{
	map(0x0000, 0x03ff).rom(); // ROM
	map(0x0400, 0xffff).bankr("bank1");    // Banked ROM
}

void suna16_state::uballoon_pcm_1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch2", FUNC(generic_latch_8_device::read));    // From The Sound Z80
	map(0x00, 0x00).w("ldac", FUNC(dac_byte_interface::data_w));
	map(0x01, 0x01).w("rdac", FUNC(dac_byte_interface::data_w));
	map(0x03, 0x03).w(FUNC(suna16_state::uballoon_pcm_1_bankswitch_w));  // Rom Bank
}

MACHINE_START_MEMBER(suna16_state,uballoon)
{
	m_bank1->configure_entries(0, 2, memregion("pcm1")->base() + 0x400, 0x10000);

	m_leds.resolve();

	save_item(NAME(m_prot));
}

MACHINE_RESET_MEMBER(suna16_state,uballoon)
{
	uballoon_pcm_1_bankswitch_w(0);
}


/***************************************************************************
                            Best Of Best
***************************************************************************/

void suna16_state::bestbest_pcm_1_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void suna16_state::bestbest_pcm_1_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch2", FUNC(generic_latch_8_device::read));   // From The Sound Z80
	map(0x00, 0x00).w("ldac", FUNC(dac_byte_interface::data_w));
	map(0x01, 0x01).w("rdac", FUNC(dac_byte_interface::data_w));
	map(0x02, 0x02).w("ldac2", FUNC(dac_byte_interface::data_w));
	map(0x03, 0x03).w("rdac2", FUNC(dac_byte_interface::data_w));
}

/***************************************************************************


                                Input Ports


***************************************************************************/

#define JOY(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(_n_)    \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(_n_)  \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(_n_)  \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(_n_)        \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(_n_)        \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(_n_)        \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_ )


/***************************************************************************
                            Back Street Soccer
***************************************************************************/

static INPUT_PORTS_START( bssoccer )
	PORT_START("P1")    /* $a00001.b */
	JOY(1)

	PORT_START("P2")    /* $a00003.b */
	JOY(2)

	PORT_START("P3")    /* $a00005.b */
	JOY(3)

	PORT_START("P4")    /* $a00007.b */
	JOY(4)

	PORT_START("DSW1")  /* $a00008.w */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, "Hardest?" ) // duplicate of "HARD" not shown as supported in manual - but possible to set on PCB
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0300, 0x0300, "Play Time P1" )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "1:30" )
	PORT_DIPSETTING(      0x0200, "1:45" )
	PORT_DIPSETTING(      0x0100, "2:00" )
	PORT_DIPSETTING(      0x0000, "2:15" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Play Time P2" )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "1:30" )
	PORT_DIPSETTING(      0x0800, "1:45" )
	PORT_DIPSETTING(      0x0400, "2:00" )
	PORT_DIPSETTING(      0x0000, "2:15" )
	PORT_DIPNAME( 0x3000, 0x3000, "Play Time P3" )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "1:30" )
	PORT_DIPSETTING(      0x2000, "1:45" )
	PORT_DIPSETTING(      0x1000, "2:00" )
	PORT_DIPSETTING(      0x0000, "2:15" )
	PORT_DIPNAME( 0xc000, 0xc000, "Play Time P4" )  PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0xc000, "1:30" )
	PORT_DIPSETTING(      0x8000, "1:45" )
	PORT_DIPSETTING(      0x4000, "2:00" )
	PORT_DIPSETTING(      0x0000, "2:15" )

	PORT_START("DSW2")  /* $a0000b.b - JP3, JP6 & JP7 and what else?? */
	PORT_DIPNAME( 0x0001, 0x0001, "Copyright" )     PORT_DIPLOCATION("Jumper:1")    // these 4 are shown in test mode
	PORT_DIPSETTING(      0x0001, "Distributer Unico" )
	PORT_DIPSETTING(      0x0000, "All Rights Reserved" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("Jumper:2")    // used!
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("Jumper:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("Jumper:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN4 )
INPUT_PORTS_END


/***************************************************************************
                                Ultra Balloon
***************************************************************************/

static INPUT_PORTS_START( uballoon )
	PORT_START("P1")    /* $600000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")    /* $600002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x3000, 0x3000, "Copyright" ) // Jumpers
	PORT_DIPSETTING(      0x3000, "Distributer Unico" )
	PORT_DIPSETTING(      0x2000, "All Rights Reserved" )
//  PORT_DIPSETTING(      0x1000, "Distributer Unico" )
//  PORT_DIPSETTING(      0x0000, "All Rights Reserved" )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")  /* $600005.b */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0018, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW2")  /* $600007.b */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x001c, "200K" )
	PORT_DIPSETTING(      0x0010, "300K, 1000K" )
	PORT_DIPSETTING(      0x0018, "400K" )
	PORT_DIPSETTING(      0x000c, "500K, 1500K" )
	PORT_DIPSETTING(      0x0008, "500K, 2000K" )
	PORT_DIPSETTING(      0x0004, "500K, 3000K" )
	PORT_DIPSETTING(      0x0014, "600K" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown DSW2-6*" )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown DSW2-7*" )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                            Suna Quiz 6000 Academy
***************************************************************************/

static INPUT_PORTS_START( sunaq )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )


	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Difficulty ) )   /* Should this be Difficulty or Lives ?? */
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) ) /* 5 Hearts */
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )   /* 5 Hearts */
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) ) /* 4 Hearts */
	PORT_DIPSETTING(      0x0018, DEF_STR( Hardest ) )  /* 3 Hearts */
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2") /* Unused? */
INPUT_PORTS_END

/***************************************************************************
                            Best Of Best
***************************************************************************/

static INPUT_PORTS_START( bestbest )
	PORT_START("P1")    /* 500000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")    /* 500002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")   /* 500004.w */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0018, 0x0010, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(      0x0018, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Display Combos" )    PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0040, IP_ACTIVE_LOW, "SWA:7" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0400, "Play Time" )     PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(      0x0600, "1:10" )
	PORT_DIPSETTING(      0x0400, "1:20" )
	PORT_DIPSETTING(      0x0200, "1:30" )
	PORT_DIPSETTING(      0x0000, "1:40" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SWB:8" )
INPUT_PORTS_END

/***************************************************************************


                                Graphics Layouts


***************************************************************************/

/* Tiles are 8x8x4 but the minimum sprite size is 2x2 tiles */

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0,4 },
	{ 3,2,1,0, 19,18,17,16 },
	{ STEP8(0,32) },
	8*8*4
};

static GFXDECODE_START( gfx_suna16 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4, 0, 16*2 ) // [0] Sprites
GFXDECODE_END

// Two sprites chips
static GFXDECODE_START( gfx_bestbest )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4, 0, 256*8/16 ) // [0] Sprites (Chip 1)
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4, 0, 256*8/16 ) // [1] Sprites (Chip 2)
GFXDECODE_END



/***************************************************************************


                                Machine drivers


***************************************************************************/


/***************************************************************************
                            Back Street Soccer
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(suna16_state::bssoccer_interrupt)
{
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(2, HOLD_LINE); // does RAM to sprite buffer copy here
}

void suna16_state::bssoccer(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/4);    /* 8MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &suna16_state::bssoccer_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(suna16_state::bssoccer_interrupt), "screen", 0, 1);

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(14'318'181)/4)); /* Z80B at 3.579545MHz */
	audiocpu.set_addrmap(AS_PROGRAM, &suna16_state::bssoccer_sound_map);

	Z80(config, m_pcm1, XTAL(32'000'000)/6);      /* Z80B at 5.333MHz */
	m_pcm1->set_addrmap(AS_PROGRAM, &suna16_state::bssoccer_pcm_1_map);
	m_pcm1->set_addrmap(AS_IO, &suna16_state::bssoccer_pcm_1_io_map);

	Z80(config, m_pcm2, XTAL(32'000'000)/6);      /* Z80B at 5.333MHz */
	m_pcm2->set_addrmap(AS_PROGRAM, &suna16_state::bssoccer_pcm_2_map);
	m_pcm2->set_addrmap(AS_IO, &suna16_state::bssoccer_pcm_2_io_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	MCFG_MACHINE_START_OVERRIDE(suna16_state,bssoccer)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0+16, 256-16-1);
	m_screen->set_screen_update(FUNC(suna16_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_suna16);
	PALETTE(config, m_palette).set_entries(512);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, "soundlatch2");
	GENERIC_LATCH_8(config, "soundlatch3");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4));  /* 3.579545MHz */
	ymsnd.add_route(0, "speaker", 0.2, 0);
	ymsnd.add_route(1, "speaker", 0.2, 1);

	DAC_4BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "speaker", 0.2, 0); // unknown DAC
	DAC_4BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "speaker", 0.2, 1); // unknown DAC
	DAC_4BIT_R2R(config, "ldac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.2, 0); // unknown DAC
	DAC_4BIT_R2R(config, "rdac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.2, 1); // unknown DAC
}


/***************************************************************************
                                Ultra Balloon
***************************************************************************/

void suna16_state::uballoon(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/4);   /* 8MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &suna16_state::uballoon_map);
	m_maincpu->set_vblank_int("screen", FUNC(suna16_state::irq1_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(14'318'181)/4));   /* Z80B at 3.579545MHz */
	audiocpu.set_addrmap(AS_PROGRAM, &suna16_state::uballoon_sound_map);

	Z80(config, m_pcm1, XTAL(32'000'000)/6); /* Z80B at 5.333MHz */
	m_pcm1->set_addrmap(AS_PROGRAM, &suna16_state::uballoon_pcm_1_map);
	m_pcm1->set_addrmap(AS_IO, &suna16_state::uballoon_pcm_1_io_map);

	/* 2nd PCM Z80 missing */

	config.set_maximum_quantum(attotime::from_hz(6000));

	MCFG_MACHINE_START_OVERRIDE(suna16_state,uballoon)
	MCFG_MACHINE_RESET_OVERRIDE(suna16_state,uballoon)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0+16, 256-16-1);
	m_screen->set_screen_update(FUNC(suna16_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_suna16);
	PALETTE(config, m_palette).set_entries(512);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, "soundlatch2");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4));  /* 3.579545MHz */
	ymsnd.add_route(0, "speaker", 0.50, 0);
	ymsnd.add_route(1, "speaker", 0.50, 1);

	DAC_4BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 0); // unknown DAC
	DAC_4BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 1); // unknown DAC
}


/***************************************************************************
                            Suna Quiz 6000 Academy
***************************************************************************/

void suna16_state::sunaq(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/4);   /* 6MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &suna16_state::sunaq_map);
	m_maincpu->set_vblank_int("screen", FUNC(suna16_state::irq1_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(14'318'181)/4));   /* Z80B at 3.579545MHz */
	audiocpu.set_addrmap(AS_PROGRAM, &suna16_state::sunaq_sound_map);

	Z80(config, m_pcm1, XTAL(24'000'000)/4); /* Z80B at 6MHz */
	m_pcm1->set_addrmap(AS_PROGRAM, &suna16_state::bssoccer_pcm_1_map);
	m_pcm1->set_addrmap(AS_IO, &suna16_state::bssoccer_pcm_1_io_map);

	/* 2nd PCM Z80 missing */

	config.set_maximum_quantum(attotime::from_hz(6000));

	MCFG_MACHINE_START_OVERRIDE(suna16_state, sunaq)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0+16, 256-16-1);
	m_screen->set_screen_update(FUNC(suna16_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_suna16);
	PALETTE(config, m_palette).set_entries(512);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, "soundlatch2");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4));  /* 3.579545MHz */
	ymsnd.add_route(0, "speaker", 0.50, 0);
	ymsnd.add_route(1, "speaker", 0.50, 1);

	DAC_4BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 0); // unknown DAC
	DAC_4BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 1); // unknown DAC
}


/***************************************************************************
                            Best Of Best
***************************************************************************/

void suna16_state::bestbest_ay8910_port_a_w(uint8_t data)
{
	// ?
}

void suna16_state::bestbest(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/4); /* 6MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &suna16_state::bestbest_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(suna16_state::bssoccer_interrupt), "screen", 0, 1);

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(24'000'000)/4)); /* 6MHz */
	audiocpu.set_addrmap(AS_PROGRAM, &suna16_state::bestbest_sound_map);

	Z80(config, m_pcm1, XTAL(24'000'000)/4); /* 6MHz */
	m_pcm1->set_addrmap(AS_PROGRAM, &suna16_state::bestbest_pcm_1_map);
	m_pcm1->set_addrmap(AS_IO, &suna16_state::bestbest_pcm_1_iomap);

	/* 2nd PCM Z80 missing */

	config.set_maximum_quantum(attotime::from_hz(6000));

	MCFG_MACHINE_START_OVERRIDE(suna16_state, bestbest)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.1734);    // measured on pcb (15.6218kHz HSync)
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0+16, 256-16-1);
	m_screen->set_screen_update(FUNC(suna16_state::screen_update_bestbest));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bestbest);
	PALETTE(config, m_palette).set_entries(256*8);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, "soundlatch2");

	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(24'000'000)/16));  /* 1.5MHz */
	aysnd.port_a_write_callback().set(FUNC(suna16_state::bestbest_ay8910_port_a_w));
	aysnd.add_route(0, "speaker", 1.0, 0);
	aysnd.add_route(1, "speaker", 1.0, 1);

	ym3526_device &ymsnd(YM3526(config, "ymsnd", XTAL(24'000'000)/8));   /* 3MHz */
	ymsnd.irq_handler().set_inputline("audiocpu", INPUT_LINE_IRQ0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	DAC_4BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "speaker", 0.2, 0); // unknown DAC
	DAC_4BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "speaker", 0.2, 1); // unknown DAC
	DAC_4BIT_R2R(config, "ldac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.2, 0); // unknown DAC
	DAC_4BIT_R2R(config, "rdac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.2, 1); // unknown DAC
}

/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

                            [ Back Street Soccer ]

KRB-0031
+------------------------------------------+
|  6116-45  6116-45          10   09       |
|  6116-45                   08   07       |
|           6116-45          05   06  62256|
|      SW-1                           62256|
|J JP7 SW-2                 ACTEL     62256|
|A JP6     Z80B    Z80B     A1020B         |
|M          13      11 JP3                 |
|M                 6116                    |
|A                 YM2151         6264 6264|
|  YM3012  Z80B                     04  03 |
|3P         12                      02  01 |
|4P                          32MHz         |
|  VOL                   14.318MHz 68000-10|
+------------------------------------------+


KRB-0032A
+------------------------------------------+
|  6116-45  6116-45          UC08003       |
|  6116-45                                 |
|           6116-45          UC16002  62256|
|      SW-1                           62256|
|J JP7 SW-2                 ACTEL     62256|
|A JP6     Z80B    Z80B     A1020B         |
|M         UC04005 UNICO5 JP3              |
|M                 6116                    |
|A                 YM2151         6264 6264|
|  YM3012  Z80B                            |
|3P        UC04004                 UC16001 |
|4P                          32MHz         |
|  VOL                   14.318MHz 68000-8 |
+------------------------------------------+


  CPU: MC68HC000P8 (or MC68HC000P10) @ 8Mhz
Video: Actel A1020B PL84C
Sound: Z0840006PSC Z80B x 3
       YM2151 & YM3012 (rebadged as CY5002)
  OSC: 32MHz & 14.31818MHz
Other: 8 position dipswitch bank x 2
       Misc JP3, JP6 & JP7 jumper pads
       CON-2 12 pin connector for Player 3 (3P)
       CON-2 12 pin connector for Player 4 (4P)
       VOL volume pot

The data is 100% identical between sets / PCB version, just in different rom types / sizes.

13 and 6 files

11                      unico5                  IDENTICAL
12                      uc04004                 IDENTICAL
13                      uc04005                 IDENTICAL

01                      uc16001      [even 1/2] IDENTICAL
02                      uc16001      [odd 1/2]  IDENTICAL
03                      uc16001      [even 2/2] IDENTICAL
04                      uc16001      [odd 2/2]  IDENTICAL

05                      uc16002      [even 1/2] IDENTICAL
06                      uc16002      [odd 1/2]  IDENTICAL
07                      uc16002      [even 2/2] IDENTICAL
08                      uc16002      [odd 2/2]  IDENTICAL

09                      uc08003      [even]     IDENTICAL
10                      uc08003      [odd]      IDENTICAL

***************************************************************************/

ROM_START( bssoccer ) /* KRB-0031 PCB */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 Code */
	ROM_LOAD16_BYTE( "02", 0x000000, 0x080000, CRC(32871005) SHA1(b094ee3f4fc24c0521915d565f6e203d51e51f6d) )
	ROM_LOAD16_BYTE( "01", 0x000001, 0x080000, CRC(ace00db6) SHA1(6bd146f9b44c97be77578b4f0ffa28cbf66283c2) )
	ROM_LOAD16_BYTE( "04", 0x100000, 0x080000, CRC(25ee404d) SHA1(1ab7cb1b4836caa05be73ea441deed80f1e1ba81) )
	ROM_LOAD16_BYTE( "03", 0x100001, 0x080000, CRC(1a131014) SHA1(4d21264da3ee9b9912d1205999a555657ba33bd7) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   /* Z80 #1 - Music */
	ROM_LOAD( "11", 0x000000, 0x010000, CRC(df7ae9bc) SHA1(86660e723b0712c131dc57645b6a659d5100e962) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "pcm1", 0 )   /* Z80 #2 - PCM */
	ROM_LOAD( "13", 0x000000, 0x080000, CRC(2b273dca) SHA1(86e1bac9d1e39457c565390b9053986453db95ab) )

	ROM_REGION( 0x080000, "pcm2", 0 )   /* Z80 #3 - PCM */
	ROM_LOAD( "12", 0x000000, 0x080000, CRC(6b73b87b) SHA1(52c7dc7da6c21eb7e0dad13deadb1faa94a87bb3) )

	ROM_REGION( 0x300000, "gfx1", ROMREGION_INVERT )    /* Sprites */
	ROM_LOAD16_BYTE( "05", 0x000000, 0x080000, CRC(a5245bd4) SHA1(d46a8db437e49158c020661536eb0be8a6e2e8b0) )
	ROM_LOAD16_BYTE( "06", 0x000001, 0x080000, CRC(d42ce84b) SHA1(3a3d07d571793ecf4c936d3af244c63b9e4b4bb9) )
	ROM_LOAD16_BYTE( "07", 0x100000, 0x080000, CRC(fdb765c2) SHA1(f9852fd3734d10e18c91cd572ca62e66d74ccb72) )
	ROM_LOAD16_BYTE( "08", 0x100001, 0x080000, CRC(96cd2136) SHA1(1241859d6c5e64de73898763f0358171ea4aeae3) )
	ROM_LOAD16_BYTE( "09", 0x200000, 0x080000, CRC(0e82277f) SHA1(4bdfd0ff310bf8326806a83767a6c98905debbd0) )
	ROM_LOAD16_BYTE( "10", 0x200001, 0x080000, CRC(1ca94d21) SHA1(23d892b840e37064a175584f955f25f990d9179d) )
ROM_END

ROM_START( bssoccera ) /* KRB-0032A PCB */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "uc16001", 0x000000, 0x200000, CRC(82fa613a) SHA1(451789190017b58b964e676b8e43f3638b4e56ef) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   /* Z80 #1 - Music */
	ROM_LOAD( "unico5", 0x000000, 0x010000, CRC(df7ae9bc) SHA1(86660e723b0712c131dc57645b6a659d5100e962) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "pcm1", 0 )   /* Z80 #2 - PCM */
	ROM_LOAD( "uc04005", 0x000000, 0x080000, CRC(2b273dca) SHA1(86e1bac9d1e39457c565390b9053986453db95ab) )

	ROM_REGION( 0x080000, "pcm2", 0 )   /* Z80 #3 - PCM */
	ROM_LOAD( "uc04004", 0x000000, 0x080000, CRC(6b73b87b) SHA1(52c7dc7da6c21eb7e0dad13deadb1faa94a87bb3) )

	ROM_REGION( 0x300000, "gfx1", ROMREGION_INVERT )    /* Sprites */
	ROM_LOAD( "uc16002", 0x000000, 0x200000, CRC(884f3ecf) SHA1(56306bb20433bf77697eb9d71ba561daec7feedb) )
	ROM_LOAD( "uc08003", 0x200000, 0x100000, CRC(d17c23f5) SHA1(12bba57f911ae58b2d3ea330e2ade4cdd1379181) )
ROM_END

/***************************************************************************

                            [ Ultra Balloon ]
KRB-0033A
+------------------------------------+
| VOL    Z80B    Z80B 14.318MHz 32MHz|
|        ROM 8   ROM 7               |
|        YM3012  6116               6|
|                                   8|
|  JP2 SW1       YM2151             0|
|J JP3 SW2                          0|
|A                                  0|
|M               ACTEL               |
|M               A1020B   ROM 2 ROM 1|
|A                        6264  6264 |
|                                    |
|                              62256 |
|         6116                 62256 |
|    6116       ROM 6  ROM 5   62256 |
|    6116 6116  ROM 4  ROM 3         |
+------------------------------------+

  CPU: MC68HC000P8
Video: Actel A1020B PL84C
Sound: Z0840006PSC Z80B x 3
       YM2151 & YM3012
  OSC: 32MHz & 14.31818MHz
Other: 8 position dipswitch bank x 2
       JP2 & JP3 jumper pads
       VOL volume pot

ROMs had no labels and were assigned names by the original dumper.

prg1.rom1      27c040
prg2.rom2      27c040
gfx3.rom3      27c040
gfx4.rom4      27c040
gfx5.rom5      27c040
gfx6.rom6      27c040
audio1.rom7    27c512
audio2.rom8    27c010

***************************************************************************/

ROM_START( uballoon )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 Code */
	ROM_LOAD16_BYTE( "prg2.rom2", 0x000000, 0x080000, CRC(72ab80ea) SHA1(b755940877cf286559208106dd5e6933aeb72242) )
	ROM_LOAD16_BYTE( "prg1.rom1", 0x000001, 0x080000, CRC(27a04f55) SHA1(a530294b000654db8d84efe4835b72e0dca62819) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   /* Z80 #1 - Music */
	ROM_LOAD( "audio1.rom7", 0x000000, 0x010000, CRC(c771f2b4) SHA1(6da4c526c0ea3be5d5bb055a31bf1171a6ddb51d) )

	ROM_REGION( 0x020000, "pcm1", 0 )   /* Z80 #2 - PCM */
	ROM_LOAD( "audio2.rom8", 0x000000, 0x020000, CRC(c7f75347) SHA1(5bbbd39285c593441c6da6a12f3632d60b103216) )

	/* There's no Z80 #3 - PCM */

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT )    /* Sprites */
	ROM_LOAD16_BYTE( "gfx3.rom3", 0x000000, 0x080000, CRC(fd2ec297) SHA1(885834d9b58ccfd9a32ecaa51c45e70fbbe935db) )
	ROM_LOAD16_BYTE( "gfx4.rom4", 0x000001, 0x080000, CRC(718f3150) SHA1(5971f006203f86743ebc825e4ab1ed1f811e3165) )
	ROM_LOAD16_BYTE( "gfx5.rom5", 0x100000, 0x080000, CRC(6307aa60) SHA1(00406eba98ec368e72ee53c08b9111dec4f2552f) )
	ROM_LOAD16_BYTE( "gfx6.rom6", 0x100001, 0x080000, CRC(af7e057e) SHA1(67a03b54ffa1483c8ed044f27287b7f3f1150455) )
ROM_END

/***************************************************************************
                            Suna Quiz 6000 Academy

  KRB-0027A mainboard

  68000 6 MHz
  Z80B x 2
  Actel A1020B
  OSC: 24.000 MHz, 14.318 MHz
  YM2151, YM3012
  RAM:
  62256 x 3
  6264 x 2
  6116 x 5
  Single 8 switch DSW

***************************************************************************/

ROM_START( sunaq )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 Code */
	ROM_LOAD16_BYTE( "prog2.bin", 0x000000, 0x080000, CRC(a92bce45) SHA1(258b2a21c27effa1d3380e4c08558542b1d05175) )
	ROM_LOAD16_BYTE( "prog1.bin", 0x000001, 0x080000, CRC(ff690e7e) SHA1(43b9c67f8d8d791be922966632613a077807b755) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   /* Z80 #1 - Music */
	ROM_LOAD( "audio1.bin", 0x000000, 0x010000, CRC(3df42f82) SHA1(91c1037c9d5d1ec82ed4cdfb35de5a6d626ecb3b) )

	ROM_REGION( 0x080000, "pcm1", 0 )   /* Z80 #2 - PCM */
	ROM_LOAD( "audio2.bin", 0x000000, 0x080000, CRC(cac85ba9) SHA1(e5fbe813022c17d9eaf2a57184341666e2af365a) )

	/* There's no Z80 #3 - PCM */

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT )    /* Sprites */
	ROM_LOAD16_BYTE( "gfx1.bin", 0x000000, 0x080000, CRC(0bde5acf) SHA1(a9befb5f9a663bf48537471313f606853ea1f274) )
	ROM_LOAD16_BYTE( "gfx2.bin", 0x000001, 0x080000, CRC(24b74826) SHA1(cb3f665d1b1f5c9d385a3a3193866c9cae6c7002) )
ROM_END


/***************************************************************************

Best Of Best
Suna, 1994

PCB Layout
----------

KRB-0026
SUNA ELECTRONICS IND CO., LTD.
|------------------------------------------------------------|
|VOL     AY3-8910   Z80B     5.BIN                           |
|UPD1242H    6.BIN  Z80B     6116            68000           |
| LM324              24MHz   YM3526                          |
| LM324                                                      |
|    YM3014                82S129.5     4.BIN       2.BIN    |
|                  82S129.6|-------|                         |
|      62256               |UNKNOWN|    3.BIN       1.BIN    |
|                          |PLCC84 |                         |
|      62256               |       |    62256       62256    |
|                          |-------|                         |
|J                                                           |
|A                         |-------|                         |
|M     DSW2(8)             |ACTEL  |                         |
|M                         |A1020B |                         |
|A                         |PLCC84 |                         |
|      DSW1(8) 62256       |-------|           62256         |
|                                                            |
|        6116  62256                           62256         |
|        6116                                                |
|        6116  62256                           62256         |
|        6116                |-------------------------------|
|        6116                | 18.BIN  17.BIN          10.BIN|
|        6116                |         16.BIN  13.BIN   9.BIN|
|CN1     6116                |         15.BIN  12.BIN   8.BIN|
|        6116                |         14.BIN  11.BIN   7.BIN|
|----------------------------|-------------------------------|
Notes:
      68000    - clock 6.000MHz [24/4]
      Z80B     - clock 6.000MHz [24/4] for both
      YM3526   - clock 3.000MHz [24/8]
      AY3-8910 - clock 1.500MHz [24/16]
      6116     - 2kx8 SRAM
      62256    - 32kx8 SRAM
      CN1      - Connector for extra controls
      ROMs 7-18 located on a plug-in daughterboard
      Both PROMs are identical

      Measurements
      ------------
      XTAL  - 23.99463MHz
      VSync - 59.1734Hz
      HSync - 15.6218kHz

***************************************************************************/

ROM_START( bestbest )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 Code */
	// V13.0 1993,3,25-11,29 KIM.H.T M=1:KDS=9
	ROM_LOAD16_BYTE( "4.bin", 0x00000, 0x20000, CRC(06741994) SHA1(e872e9e9d02360dda9c9b6df8e6424b0f3e18c1f) )  // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "2.bin", 0x00001, 0x20000, CRC(42843dec) SHA1(3705661a9740b3499297424e340da9a3606873fb) )  // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_BE( 0x100000, "user1", 0 )     /* 68000 Data */
	ROM_LOAD16_BYTE( "3.bin", 0x00000, 0x80000, CRC(e2bb8f26) SHA1(d73bbe034718c77774dede61e751a9ae2d29118a) )
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x80000, CRC(d365e20a) SHA1(29706d6e422e71c7dad51a3369683a6539f72b54) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 #1 - Music */
	ROM_LOAD( "5.bin", 0x00000, 0x10000, CRC(bb9265e6) SHA1(424eceac4fd48c9a99653ece2f3fcbc8b37569cf) ) // BEST OF BEST V10 XILINX PROGRAM 3020 1994,1,17

	ROM_REGION( 0x10000, "pcm1", 0 )    /* Z80 #2 - PCM */
	ROM_LOAD( "6.bin", 0x00000, 0x10000, CRC(dd445f6b) SHA1(658417d72c003f25db273e3c731838317ed1876c) )

	/* There's no Z80 #3 - PCM */

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT )    /* Sprites (Chip 1) */
	ROM_LOAD16_BYTE( "9.bin",  0x000000, 0x80000, CRC(b11994ea) SHA1(4ff2250a9dbb2e575982e2ffcad7686347368b5b) )
	ROM_LOAD16_BYTE( "7.bin",  0x000001, 0x80000, CRC(16188b73) SHA1(1e67f9b100614466e2ff1169f25c90e34a2e7db9) )
	ROM_LOAD16_BYTE( "10.bin", 0x100000, 0x80000, CRC(37b41ef5) SHA1(dd4500663537ffad369ee9415c56df90221bed23) )
	ROM_LOAD16_BYTE( "8.bin",  0x100001, 0x80000, CRC(765ce06b) SHA1(6cc6d7c27b49eedd58104c50e4887f86bff9357c) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_INVERT )    /* Sprites (Chip 2) */
	ROM_LOAD16_BYTE( "16.bin", 0x000000, 0x80000, CRC(dc46cdea) SHA1(d601f5464894223ce8459093ae53006155a3e680) )
	ROM_LOAD16_BYTE( "14.bin", 0x000001, 0x80000, CRC(c210fb53) SHA1(3d5a763bffaef922a77c95131b1e41f0f90629a5) )
	ROM_LOAD16_BYTE( "17.bin", 0x100000, 0x80000, CRC(c6fadd57) SHA1(ce9bc4d7a288feebdd19de09d00bec8489346878) )
	ROM_LOAD16_BYTE( "15.bin", 0x100001, 0x80000, CRC(3b1166c7) SHA1(7f2a0c9131fcf39dd67047b6e697c4076ca37b19) )
	ROM_LOAD16_BYTE( "13.bin", 0x200000, 0x80000, CRC(23283ac4) SHA1(f7aa00f203b17b590f1c43990f3f1c4aba7ba0ad) )
	ROM_LOAD16_BYTE( "11.bin", 0x200001, 0x80000, CRC(323eebc3) SHA1(0e82b583273c9ba5252f7a108538ae58edf39a03) )
	ROM_LOAD16_BYTE( "18.bin", 0x300000, 0x80000, CRC(674c4609) SHA1(f1de78c01d26dfb1174203415ccf3c771398d163) )
	ROM_LOAD16_BYTE( "12.bin", 0x300001, 0x80000, CRC(ca7c8176) SHA1(1ec99db3e0840b4647d6ccdf6fda118fa9ad4f42) )

	ROM_REGION( 0x200, "proms", 0 ) // ?
	ROM_LOAD( "82s129.5", 0x000, 0x100, CRC(10bfcebb) SHA1(ae8708db7d3a8984f16e876867ecdbb4445e3378) )  // FIXED BITS (0000xx0x0000xxxx), same as the one for starfigh in suna8.cpp
	ROM_LOAD( "82s129.6", 0x100, 0x100, CRC(10bfcebb) SHA1(ae8708db7d3a8984f16e876867ecdbb4445e3378) )  // identical to 82s129.5
ROM_END


/***************************************************************************


                                Games Drivers


***************************************************************************/

GAME( 1994, bestbest,  0,        bestbest, bestbest, suna16_state, empty_init, ROT0, "SunA",                 "Best Of Best",                       MACHINE_SUPPORTS_SAVE )
GAME( 1994, sunaq,     0,        sunaq,    sunaq,    suna16_state, empty_init, ROT0, "SunA",                 "SunA Quiz 6000 Academy (940620-6)",  MACHINE_SUPPORTS_SAVE )   // Date/Version on-screen is 940620-6, but in the program rom it's  1994,6,30  K.H.T  V6.00
GAME( 1996, bssoccer,  0,        bssoccer, bssoccer, suna16_state, empty_init, ROT0, "SunA (Unico license)", "Back Street Soccer (KRB-0031 PCB)",  MACHINE_SUPPORTS_SAVE )
GAME( 1996, bssoccera, bssoccer, bssoccer, bssoccer, suna16_state, empty_init, ROT0, "SunA (Unico license)", "Back Street Soccer (KRB-0032A PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, uballoon,  0,        uballoon, uballoon, suna16_state, empty_init, ROT0, "SunA (Unico license)", "Ultra Balloon",                      MACHINE_SUPPORTS_SAVE )
