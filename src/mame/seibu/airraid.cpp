// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Angelo Salese, hap
/* Air Raid (aka Cross Shooter) (c) 1987 Seibu

  this driver is for the single board version on the S-0087-011A-0 PCB
  for the version using a S-0086-002-B0 base PCB and separate video board see stfight.cpp

  Video system is very similar to darkmist.cpp

  Custom Modules note:

  The Air Raid / Cross Shooter PCB contains 3 custom modules.
  These modules have not been dumped.

  2 of these are responsible for background layer generation.
  the 3rd is responsible for sprites.

  Both the tile gfx data, and tilemap layout data are
  contained within the background modules, none of this is
  accessible by the CPU (the CPU simply provides a scroll
  position)

  The sprite modules contain the sprite gfx data.

  The modules also contain the CLUT PROM data for their layer
  which controls colours and transparency.  It is likely
  each module contains the equivalent logic of a SEI0010BU
  in order to handle this.  This means there is a possibility
  that the raw data from the ROMs is not exposed on the edge
  of the modules, only post-lookup data.

  The above information is based off a development board
  which was unfortunately stripped of all ROMs.

-----------------------------------------------------------------------

  TODO:
  - cshooter/airraid gfx, obviously

-----------------------------------------------------------------------

Stephh's notes (based on the game Z80 code and some tests) :

  - Memory notes (100% guess) :

      * There are still some writes to the ROM area, but I think it's
        related to wrong interrupts and/or incomplete memory mapping.
      * Reads from 0xb0?? seem to be related to sound
      * Write to 0xc500 happens LOTS of time - related to scanlines ?
      * Write to 0xc600 might be used to disable the interrupts and
        the possible communication between CPUs (if they are 2)
      * Write to 0xc700 seems to be done when a coin is inserted
        (also done once during P.O.S.T. - unknown purpose here).
      * Write to 0xc801 might be sort of watchdog as it "pollutes"
        the error.log file.


  - Inputs notes :

      * COINx don't work correctly.
      * In game, bits 3 and 4 of 0xc202 ("START") are tested,
        while bits 4 and 5 are tested in the "test mode".
      * Pressing STARTx while in game adds lives (depending on the
        "Lives" Dip Switch) for player x.


  - Other notes :

      * 0x0006 contains the "region" stuff (value in 'cshooter' = 0xc4) :
          . bits 2 and 3 determine the manufacturer :
              0x00 : "J.K.H. Corp."         (no logo)
              0x04 : "Taito Corporation."   (+ logo)
              0x08 : "International Games"  (+ logo)
              0x0c : "Seibu Kaihatsu,Inc."  (+ logo)
          . bits 6 and 7 determine the title screen :
              0x00 : unknown - scrambled GFX *probably air raid, haze
              0x40 : unknown - scrambled GFX (alternate entry) *probably air raid, haze
              0x80 : "Cross Shooter"
              0xc0 : "Cross Shooter" (same as 0x80)


  - Addresses :

      * 0xe222 : contents of DSW1 (0xc204)
      * 0xe223 : contents of DSW2 (0xc203)
      * 0xe228 : difficulty (from DSW2)
      * 0xe229 : bonus life (from DSW2 - table at 0x6264)
      * 0xe22b : lives      (from DSW2 - table at 0x7546)
      * 0xe234 : credits (0x00-0xff, even if display is limited to 9)
          . if 1 coin slot , total credits
          . if 2 coin slots, credits for player 1
      * 0xe235 : credits (0x00-0xff, even if display is limited to 9)
          . if 1 coin slot , always 0x00 !
          . if 2 coin slots, credits for player 2
      * 0xe237 : lives for player 1
      * 0xe238 : lives for player 2

-----------------------------------------------------------------------

    Custom SIMM module placement

    The Air Raid PCB has 3 custom modules shown
    in the rough diagram of the PCB below.

    The modules do not appear to be 100%
    identical based on external shapes visible.

    The data they contain is as follows
    MODULE1
    Background tilemap graphic data
    Background tilemap layout data

    MODULE2
    Foreground tilemap graphic data
    Foreground tilemap layout data

    MODULE3
    Sprite graphic data

    it's also possible each module contains a
    0x100 colour look up table for that layer.


   |-------------------------------------------------|
   |    A   B   C   D   E   F   G   H   I   J   K    |
 20|            ##MOD1#######                        |
   | #                            Z80                |
   | M                                              -|
   | O                                              |
   | D                                              |=J
   | 2                                              |=A
   | #                                              |=M
 11|                                                |=M
   |                                                |=A
   |                                                |
   |                                                -|
   |                                  Z80            |
   |                                                 |
   |        ##MOD3#######                            |
   |-------------------------------------------------|

*/


#include "emu.h"
#include "airraid_dev.h"

#include "seibusound.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class airraid_state : public driver_device
{
public:
	airraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_seibu_sound(*this, "seibu_sound")
		, m_mainram(*this, "mainram")
		, m_palette(*this, "palette")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_airraid_video(*this,"airraid_vid")
		, m_mainbank(*this, "mainbank")
	{ }

	void airraid(machine_config &config);
	void airraide(machine_config &config);

	void init_airraid();
	void init_airraide();

private:
	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_shared_ptr<uint8_t> m_mainram;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_device<airraid_video_device> m_airraid_video;
	required_memory_bank m_mainbank;

	void c500_w(uint8_t data);
	void c700_w(uint8_t data);
	void bank_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void main_map(address_map &map) ATTR_COLD;
	void sound_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
};



// main CPU

TIMER_DEVICE_CALLBACK_MEMBER(airraid_state::scanline)
{
	int scanline = param;

	if (scanline == 240) // updates scroll registers
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h

	if (scanline == 0) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80 - RST 08h
}

void airraid_state::c500_w(uint8_t data)
{
}

void airraid_state::c700_w(uint8_t data)
{
}

void airraid_state::bank_w(uint8_t data)
{
	// format of this address is TTBB tbfs

	// TT = bg tilemap upper scroll bits (probably bg tilemap bank select?)
	// BB = prg ROM bank select
	// t = text layer enable
	// b = bg layer disable
	// f = fg layer disable
	// s = sprite layer enable

	m_mainbank->set_entry((data >> 4) & 3);

	m_airraid_video->layer_enable_w(data & 0xcf);
}



void airraid_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank).nopw(); // rld result write-back
	map(0xc000, 0xc000).portr("IN0");
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
	map(0xc003, 0xc003).portr("DSW2");
	map(0xc004, 0xc004).portr("DSW1");
	map(0xc500, 0xc500).w(FUNC(airraid_state::c500_w));
//  map(0xc600, 0xc600).w(FUNC(airraid_state::c600_w));            // see notes
	map(0xc700, 0xc700).w(FUNC(airraid_state::c700_w));
//  map(0xc801, 0xc801).w(FUNC(airraid_state::c801_w));            // see notes
	map(0xd000, 0xd7ff).ram().w(m_airraid_video, FUNC(airraid_video_device::txram_w)).share("txram");
	map(0xd800, 0xd8ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xda00, 0xdaff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xdc00, 0xdc0f).ram().w(m_airraid_video, FUNC(airraid_video_device::vregs_w)).share("vregs");
//  map(0xdc10, 0xdc10).ram();
	map(0xdc11, 0xdc11).w(FUNC(airraid_state::bank_w));
//  map(0xdc19, 0xdc19).ram();
//  map(0xdc1e, 0xdc1e).ram();
//  map(0xdc1f, 0xdc1f).ram();
	map(0xde00, 0xde0f).rw(m_seibu_sound, FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w));
	map(0xe000, 0xfdff).ram().share(m_mainram);
	map(0xfe00, 0xffff).ram().share("sprite_ram");
}

void airraid_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
}

void airraid_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).r("sei80bu", FUNC(sei80bu_device::data_r));
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).w(m_seibu_sound, FUNC(seibu_sound_device::pending_w));
	map(0x4001, 0x4001).w(m_seibu_sound, FUNC(seibu_sound_device::irq_clear_w));
	map(0x4002, 0x4002).w(m_seibu_sound, FUNC(seibu_sound_device::rst10_ack_w));
	map(0x4003, 0x4003).w(m_seibu_sound, FUNC(seibu_sound_device::rst18_ack_w));
	map(0x4007, 0x4007).w(m_seibu_sound, FUNC(seibu_sound_device::bank_w));
	map(0x4008, 0x4009).rw(m_seibu_sound, FUNC(seibu_sound_device::ym_r), FUNC(seibu_sound_device::ym_w));
	map(0x4010, 0x4011).r(m_seibu_sound, FUNC(seibu_sound_device::soundlatch_r));
	map(0x4012, 0x4012).r(m_seibu_sound, FUNC(seibu_sound_device::main_data_pending_r));
	map(0x4013, 0x4013).portr("COIN");
	map(0x4018, 0x4019).w(m_seibu_sound, FUNC(seibu_sound_device::main_data_w));
	map(0x401b, 0x401b).w(m_seibu_sound, FUNC(seibu_sound_device::coin_w));
	map(0x8000, 0xffff).rom();
}

void airraid_state::sound_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x1fff).r("sei80bu", FUNC(sei80bu_device::opcode_r));
	map(0x8000, 0xffff).rom().region("audiocpu", 0x8000);
}


static INPUT_PORTS_START( airraid )
	PORT_START("IN0")   // IN0  (0xc200)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   // IN1  (0xc201)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   // START    (0xc202)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // DSW2 (0xc203)
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "2k 10k 20k" )
	PORT_DIPSETTING(    0x08, "5k 20k 40k" )
	PORT_DIPSETTING(    0x04, "6k 30k 60k" )
	PORT_DIPSETTING(    0x00, "7k 40k 80k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW1")  // DSW1 (0xc204)
	PORT_DIPNAME( 0x01, 0x01, "Coin Slots" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_SERVICE_DIPLOC(  0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("COIN")  // COIN (0xc205)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( failgate )
	PORT_INCLUDE( airraid )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Energy Consumption" )  PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )     PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )
INPUT_PORTS_END



void airraid_state::airraid(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 2);        // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &airraid_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(airraid_state::scanline), "airraid_vid:screen", 0, 1);

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(14'318'181) / 4));      // verified on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &airraid_state::sound_map);
	audiocpu.set_addrmap(AS_OPCODES, &airraid_state::sound_decrypted_opcodes_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	config.set_perfect_quantum(m_maincpu);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x100);

	AIRRAID_VIDEO(config, m_airraid_video, 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181) / 4));
	ymsnd.irq_handler().set(m_seibu_sound, FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline("audiocpu", 0);
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));

	SEI80BU(config, "sei80bu", 0).set_device_rom_tag("audiocpu");
}

void airraid_state::airraide(machine_config &config)
{
	airraid(config);
	m_maincpu->set_addrmap(AS_OPCODES, &airraid_state::decrypted_opcodes_map);
}


/*

Air Raid (Seibu 1987)
S-0087-011A-0

            82S129        TMM2015      Z80B  2.19J
SEI0020BU                 TMM2015            1.18J
SEI0020BU         63S281                    TMM2063
SEI0020BU         TMM2015
63S281
SEI0050BU                 3.13F

SEI0040BU                                   TMM2015          on
                                     4.7F    YM2151          x x
TMM2015                TMM2015       5.6F    Z80         sw2  x xxxxx
TMM2015               TMM2015                                  x
SEI0030BU          SEI0060BU                             sw1 xx xxxxx
                                   SEI80BU
                                   SEI0100BU(YM3931) YM3012
*/

ROM_START( airraid )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Main CPU
	ROM_LOAD( "1.16j",   0x00000, 0x08000, CRC(7ac2cedf) SHA1(272831f51a2731e067b5aec6dba6bddd3c5350c9) )

	ROM_REGION( 0x10000, "maindata", 0 ) // cpu data
	ROM_LOAD( "2.19j",   0x00000, 0x10000, CRC(842ae6c2) SHA1(0468445e4ab6f42bac786f9a258df3972fd1fde9) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sub/Sound CPU
	ROM_LOAD( "5.6f",    0x00000, 0x02000, CRC(30be398c) SHA1(6c61200ee8888d6270c8cec50423b3b5602c2027) )
	ROM_LOAD( "4.7f",    0x08000, 0x08000, CRC(3cd715b4) SHA1(da735fb5d262908ddf7ed7dacdea68899f1723ff) )

	ROM_REGION( 0x0200, "proms", 0 ) // this PCB type has different PROMs when compared to the cshootert hardware PCB where they were dumped
	ROM_LOAD( "pr.c19",   0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "6308.a13", 0x0100, 0x0100, NO_DUMP )

	ROM_REGION( 0x02000, "airraid_vid:tx_gfx", 0 ) // TX Layer
	ROM_LOAD( "3.13e",   0x00000, 0x02000, CRC(672ec0e8) SHA1(a11cd90d6494251ceee3bc7c72f4e7b1580b77e2) )

	ROM_REGION( 0x100, "airraid_vid:tx_clut", 0 ) // taken from cshootert, not verified for this PCB
	ROM_LOAD( "63s281.d16", 0x0000, 0x0100, CRC(0b8b914b) SHA1(8cf4910b846de79661cc187887171ed8ebfd6719) ) // clut

	// ### MODULE 1 ### Background generation / graphics
	ROM_REGION( 0x40000, "airraid_vid:bg_map", 0 )
	ROM_LOAD16_BYTE( "bg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:bg_gfx", 0 )
	ROM_LOAD16_BYTE( "bg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:bg_clut", 0 )
	ROM_LOAD( "bg_clut",   0x000, 0x100, NO_DUMP )

	// ### MODULE 2 ### Foreground generation / graphics
	ROM_REGION( 0x40000, "airraid_vid:fg_map", 0 )
	ROM_LOAD16_BYTE( "fg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:fg_gfx", 0 )
	ROM_LOAD16_BYTE( "fg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:fg_clut", 0 )
	ROM_LOAD( "fg_clut",   0x000, 0x100, NO_DUMP )

	// ### MODULE 3 ### Sprite graphics
	ROM_REGION( 0x40000, "airraid_vid:spr_gfx", 0 )
	ROM_LOAD16_BYTE( "sprite_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "sprite_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:spr_clut", 0 )
	ROM_LOAD( "spr_clut",   0x000, 0x100, NO_DUMP )
ROM_END


/*

Cross Shooter
(C) J K H Corp  (Seibu?)

Seibu Hardware
PCB is coloured black

PCB No. S-0087-011A-0
CPU: SHARP LH0080B (Z80B)
SND: YM2151, Z80A, SEI80BU 611 787, YM3012, SEI0100BU YM3931
RAM: TMM2015 x 7, TMM2063 x 1
DIPs: 2 x 8 position
CMOS Gate Arrays: SEI0020BU TC17G008AN-0015 (x 3), SEI10040BU TC15G008AP-0048,
                  SEI0030BU TC17G005AN-0026, SEI0060BU TC17G008AN-0024
OTHER: SEI0050BU M  6 4 0 00
XTAL: 14.318 MHz (near SEI80BU), xx.000 MHz (cant read speed, near SEI0040BU)

There are 3 BIG custom black packs on the PCB.

ROMS:
Note, all ROMs have official sticker, "(C) SEIBU KAIHATSU INC." and a number.

1.k19  TMM27256      \
2.k20  TMM27512      / Program
3.f11  TMM2764         Gfx?
4.g8   TMM24256 Mask   Sound (Samples?)
5.g6   TMM2764         Sound program

*/

ROM_START( cshooter )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.k19",   0x00000, 0x08000, CRC(71418952) SHA1(9745ca006576381c9e9595d8e42ab276bab80a41) )

	ROM_REGION( 0x10000, "maindata", 0 ) // CPU data
	ROM_LOAD( "2.k20",   0x00000, 0x10000, CRC(5812fe72) SHA1(3b28bff6b62a411d2195bb228952db62ad32ef3d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5.6f",    0x00000, 0x02000, CRC(30be398c) SHA1(6c61200ee8888d6270c8cec50423b3b5602c2027) ) // 5.g6
	ROM_LOAD( "4.7f",    0x08000, 0x08000, CRC(3cd715b4) SHA1(da735fb5d262908ddf7ed7dacdea68899f1723ff) ) // 4.g8

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s129.c19",  0x0000, 0x0100, CRC(dcd4e66d) SHA1(17f53c7905c20f0c6c0a74ab519b8e4a442835c3) )
	ROM_LOAD( "63s281.a13",  0x0100, 0x0100, CRC(208d17ca) SHA1(a77d56337bcac8d9a7bc3411239dfb3045e069ec) )

	ROM_REGION( 0x02000, "airraid_vid:tx_gfx",  0 ) // TX Layer
	ROM_LOAD( "3.13e",   0x00000, 0x02000, CRC(704c26d7) SHA1(e5964f409cbc2c4752e3969f3e84ace08d5ad9cb) )

	ROM_REGION( 0x100, "airraid_vid:tx_clut", 0 )
	ROM_LOAD( "63s281.d16", 0x0000, 0x0100, CRC(0b8b914b) SHA1(8cf4910b846de79661cc187887171ed8ebfd6719) ) // clut

	// ### MODULE 1 ### Background generation / graphics
	ROM_REGION( 0x40000, "airraid_vid:bg_map", 0 )
	ROM_LOAD16_BYTE( "bg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:bg_gfx", 0 )
	ROM_LOAD16_BYTE( "bg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:bg_clut", 0 )
	ROM_LOAD( "bg_clut",   0x000, 0x100, NO_DUMP )

	// ### MODULE 2 ### Foreground generation / graphics
	ROM_REGION( 0x40000, "airraid_vid:fg_map", 0 )
	ROM_LOAD16_BYTE( "fg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:fg_gfx", 0 )
	ROM_LOAD16_BYTE( "fg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:fg_clut", 0 )
	ROM_LOAD( "fg_clut",   0x000, 0x100, NO_DUMP )

	// ### MODULE 3 ### Sprite graphics
	ROM_REGION( 0x40000, "airraid_vid:spr_gfx", 0 )
	ROM_LOAD16_BYTE( "sprite_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "sprite_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:spr_clut", 0 )
	ROM_LOAD( "spr_clut",   0x000, 0x100, NO_DUMP )
ROM_END


/*

Fail Gate

On Air Raid board, for black Seibu type only, not for green Taito type
Sold as conversion kit with install instructions (Japanese) + ROM chips + riser boards
3 large black packages on board are removed to install 3 riser boards SEI4BACK0 + SEI4BACK1 + SEI4OBJ
SEI004OBU pins bridged with flying wire + other wires on board reverse side

*/

ROM_START( failgate )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fail.k19",   0x00000, 0x08000,  CRC(b8ed4b0d) SHA1(9e204b7213625f0e17f87061fdcbe79a056b0986) )

	ROM_REGION( 0x10000, "maindata", 0 )
	ROM_LOAD( "fail.k20",   0x00000, 0x10000, CRC(893132a0) SHA1(2893d235130903b65be6a17e0f32b70e8c8fea73) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on main board
	ROM_LOAD( "fail.f6",    0x00000, 0x02000, CRC(9cb6e402) SHA1(089c6c186dc4816cc33207e5762c2721012041fa) )
	ROM_LOAD( "fail.f7",    0x08000, 0x08000, CRC(9d74085e) SHA1(8309ec3258590beaec342b4689a0d36c26137ed1) )

	ROM_REGION( 0x02000, "airraid_vid:tx_gfx", 0 )
	ROM_LOAD( "fail.f11",        0x00000, 0x02000, CRC(845047d0) SHA1(b39b402724ab2b486a4e32ca20e6cd289e354433) )
	ROM_REGION( 0x100, "airraid_vid:tx_clut", 0 )
	ROM_LOAD( "fail.d16",        0x00000, 0x00100, CRC(c784cc55) SHA1(acb053d5f353b68139456eac472dc9f7542df042) )

	// on riser board SEI4BACK1 with SEI001OBU + buffer RAM + other component
	ROM_REGION( 0x40000, "airraid_vid:bg_map", 0 )
	ROM_LOAD( "back1.fail.1c",   0x00000, 0x40000, CRC(de6bb59b) SHA1(ca991c7d0d4f30b6f499fb2133fc3054fa9d8953) )
	ROM_REGION( 0x40000, "airraid_vid:bg_gfx", 0 )
	ROM_LOAD( "back1.fail.1b",   0x00000, 0x40000, CRC(0421b0f7) SHA1(aeb78f05bb956cbec0ad9d45f24914177a48e15a) )
	ROM_REGION( 0x100, "airraid_vid:bg_clut", 0 )
	ROM_LOAD( "back1.fail.2b",   0x00000, 0x00100, CRC(36eead99) SHA1(cc934605c0cfc39abbab65bdc18fbb1d48ffdace) )

	// on riser board SEI4BACK0 with SEI001OBU + buffer RAM + other component
	ROM_REGION( 0x10000, "airraid_vid:fg_map", 0 )
	ROM_LOAD( "back0.fail.1c",   0x00000, 0x10000, CRC(71aae804) SHA1(d8c217364fede3db132849c76a09f28b701c045c) )
	ROM_REGION( 0x40000, "airraid_vid:fg_gfx", 0 )
	ROM_LOAD( "back0.fail.1b",   0x00000, 0x40000, CRC(a80066d6) SHA1(77a97b14b1f27f9b043e93f25dca427d6412bfdd) )
	ROM_REGION( 0x100, "airraid_vid:fg_clut", 0 )
	ROM_LOAD( "back0.fail.2b",   0x00000, 0x00100, CRC(dbb1709f) SHA1(4b82776bc0d852070a3187ee263513a73c03fac8) )

	// on riser board SEI4OBJ with SEI001OBU + buffer RAM + other component
	ROM_REGION( 0x20000, "airraid_vid:spr_gfx", 0 )
	ROM_LOAD( "obj.fail.1a",   0x00000, 0x20000, CRC(b32f1357) SHA1(92514eeb45ceef3463de741bc19da85737da8e1e) )
	ROM_REGION( 0x100, "airraid_vid:spr_clut", 0 )
	ROM_LOAD( "obj.fail.2a",   0x00000, 0x00100, CRC(5d11f0de) SHA1(c3d16395fd1b3f5501c73fd658a67a7ba91492ae) )
ROM_END



void airraid_state::init_airraid()
{
	m_mainbank->configure_entries(0, 4, memregion("maindata")->base(), 0x4000);
}

void airraid_state::init_airraide()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int A = 0x0000; A < 0x8000; A++)
	{
		// decode the opcodes
		m_decrypted_opcodes[A] = rom[A];

		if (BIT(A, 5) && !BIT(A, 3))
			m_decrypted_opcodes[A] ^= 0x40;

		if (BIT(A, 10) && !BIT(A, 9) && BIT(A, 3))
			m_decrypted_opcodes[A] ^= 0x20;

		if ((BIT(A, 10) ^ BIT(A, 9)) && BIT(A, 1))
			m_decrypted_opcodes[A] ^= 0x02;

		if (BIT(A, 9) || !BIT(A, 5) || BIT(A, 3))
			m_decrypted_opcodes[A] = bitswap<8>(m_decrypted_opcodes[A], 7, 6, 1, 4, 3, 2, 5, 0);

		// decode the data
		if (BIT(A, 5))
			rom[A] ^= 0x40;

		if (BIT(A, 9) || !BIT(A,5))
			rom[A] = bitswap<8>(rom[A], 7, 6, 1, 4, 3, 2, 5, 0);
	}

	init_airraid();
}

} // Anonymous namespace


// There's also an undumped International Games version
GAME( 1987, cshooter, airraid, airraide, airraid,  airraid_state, init_airraide, ROT270, "Seibu Kaihatsu (J.K.H. license)",  "Cross Shooter (Single PCB)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1987, airraid,  0,       airraide, airraid,  airraid_state, init_airraide, ROT270, "Seibu Kaihatsu",                   "Air Raid (Single PCB)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )

GAME( 1991, failgate, 0,       airraid,  failgate, airraid_state, init_airraid,  ROT270, "Success (Seibu Kaihatsu license)", "Fail Gate",                  MACHINE_SUPPORTS_SAVE ) // Seibu version of Final Gate (C) 1990 Success
