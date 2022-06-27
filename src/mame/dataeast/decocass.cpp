// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
/*******************************************************************************

    DECO Cassette System driver
    by Juergen Buchmueller

    with contributions by: David Widel, Nicola Salmoria, Aaron Giles, Brian Troha,
    Fabio Priuli, Lord Nightmare, The Dumping Union, Team Japump!!!, Hau,
    Jean-Francois Del Nero, Omar Cornut, Game Preservation Society, Joseph Redon


    The DECO cassette system consists of three PCBs in a card cage:

    **** Early boardset: (1980-1983) (proms unknown for this boardset, no schematics for this boardset) ****

    One DE-0069C-0 RMS-3 pcb with a 6502 processor, D8041C MCU (DECO Cassette control),
    two ay-3-8910s, and one 2708 eprom holding the audio bios. (audio, needs external
    amp and volume control)

    One DE-0068B-0 DSP-3 pcb with a 'DECO CPU-3' custom, two 2716 eproms. (main processor
    and bios, graphics, dipswitches?)

    One DE-0070C-0 BIO-3 pcb with an analog ADC0908 8-bit adc.

    One DE-0066B-0 card rack board that the other three boards plug into.
    This boardset has two versions: MD, known as "shokase" in Japan, and MT, known as "daikase",
    which is using bigger data tapes. (MT was only sold in Japan, not emulated yet)

    **** Later boardset: (1984 onward, schematic is dated October 1983) ****

    One DE-0097C-0 RMS-8 pcb with a 6502 processor, two ay-3-8910s, two eproms (2716 and 2732)
    plus one prom, and 48k worth of 4116 16kx1 DRAMs; the 6502 processor has its own 4K of SRAM.
    (audio processor and RAM, Main processor's dram, dipswitches)

    One DE-0096C-0 DSP-8 board with a 'DECO 222' custom on it (labeled '8049 // C10707-2') which
    appears to really be a 'cleverly' disguised 6502, and two proms, plus 4K of sram, and three
    hm2511-1 1kx1 srams. (main processor, sprites, missiles, palette)

    One DE-0098C-0 B10-8 (BIO-8 on schematics) board with an 8041, an analog devices ADC0908 8-bit adc,
    and 4K of SRAM on it. (DECO Cassette control, inputs, tilemaps, headlights)

    One DE-0109C-0 card rack board that the other three boards plug into. (fourth connector for
    DE-109C-0 is shorter than in earlier versions)


    The actual cassettes use a custom player hooked to the BIO board, and are roughly microcassette
    form factor, but are larger and will not fit in a conventional microcassette player. Each cassette
    has one track on it and is separated into clock and data by two Magtek IC in the player, for
    a form of synchronous serial. The data is stored in blocks with headers and CRC16 checksums.
    The first block contains information such as the region (A:Japan, B:USA, C:UK, D:Europe)
    and the total number of blocks left to read. The last physical block on the cassette is a dummy
    block not used by the system. (only used to mark the end of last block)

*******************************************************************************/

#include "emu.h"
#include "includes/decocass.h"

#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/deco222.h"
#include "machine/decocass_tape.h"
#include "sound/ay8910.h"
#include "speaker.h"

#define MASTER_CLOCK    XTAL(12'000'000)
#define HCLK            (MASTER_CLOCK/2)
#define HCLK1           (HCLK/2)
#define HCLK2           (HCLK1/2)
#define HCLK4           (HCLK2/2)


/***************************************************************************
 *
 *  swizzled mirror handlers
 *
 ***************************************************************************/

void decocass_state::mirrorvideoram_w(offs_t offset, uint8_t data) { offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5); decocass_fgvideoram_w(offset, data); }
void decocass_state::mirrorcolorram_w(offs_t offset, uint8_t data) { offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5); decocass_colorram_w(offset, data); }

uint8_t decocass_state::mirrorvideoram_r(offs_t offset)
{
	offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5);
	return m_fgvideoram[offset];
}

uint8_t decocass_state::mirrorcolorram_r(offs_t offset)
{
	offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5);
	return m_colorram[offset];
}


void decocass_state::decocass_map(address_map &map)
{
	map(0x0000, 0x5fff).ram().share("rambase");
	map(0x6000, 0xbfff).ram().w(FUNC(decocass_state::decocass_charram_w)).share("charram"); /* still RMS3 RAM */
	map(0xc000, 0xc3ff).ram().w(FUNC(decocass_state::decocass_fgvideoram_w)).share("fgvideoram");  /* DSP3 RAM */
	map(0xc400, 0xc7ff).ram().w(FUNC(decocass_state::decocass_colorram_w)).share("colorram");
	map(0xc800, 0xcbff).rw(FUNC(decocass_state::mirrorvideoram_r), FUNC(decocass_state::mirrorvideoram_w));
	map(0xcc00, 0xcfff).rw(FUNC(decocass_state::mirrorcolorram_r), FUNC(decocass_state::mirrorcolorram_w));
	map(0xd000, 0xd7ff).ram().w(FUNC(decocass_state::decocass_tileram_w)).share("tileram");
	map(0xd800, 0xdbff).ram().w(FUNC(decocass_state::decocass_objectram_w)).share("objectram");
	map(0xe000, 0xe0ff).ram().w(FUNC(decocass_state::decocass_paletteram_w)).share("paletteram");
	map(0xe300, 0xe300).portr("DSW1").w(FUNC(decocass_state::decocass_watchdog_count_w));
	map(0xe301, 0xe301).portr("DSW2").w(FUNC(decocass_state::decocass_watchdog_flip_w));
	map(0xe302, 0xe302).w(FUNC(decocass_state::decocass_color_missiles_w));
	map(0xe400, 0xe400).w(FUNC(decocass_state::decocass_reset_w));

/* BIO-3 board */
	map(0xe402, 0xe402).w(FUNC(decocass_state::decocass_mode_set_w));      /* scroll mode regs + various enable regs */
	map(0xe403, 0xe403).w(FUNC(decocass_state::decocass_back_h_shift_w));  /* back (both)  tilemap x scroll */
	map(0xe404, 0xe404).w(FUNC(decocass_state::decocass_back_vl_shift_w)); /* back (left)  (top@rot0) tilemap y scroll */
	map(0xe405, 0xe405).w(FUNC(decocass_state::decocass_back_vr_shift_w)); /* back (right) (bot@rot0) tilemap y scroll */
	map(0xe406, 0xe406).w(FUNC(decocass_state::decocass_part_h_shift_w)); /* headlight */
	map(0xe407, 0xe407).w(FUNC(decocass_state::decocass_part_v_shift_w)); /* headlight */

	map(0xe410, 0xe410).w(FUNC(decocass_state::decocass_color_center_bot_w));
	map(0xe411, 0xe411).w(FUNC(decocass_state::decocass_center_h_shift_space_w));
	map(0xe412, 0xe412).w(FUNC(decocass_state::decocass_center_v_shift_w));
	map(0xe413, 0xe413).w(FUNC(decocass_state::decocass_coin_counter_w));
	map(0xe414, 0xe414).rw(FUNC(decocass_state::decocass_sound_command_main_r), FUNC(decocass_state::decocass_sound_command_w));
	map(0xe415, 0xe416).w(FUNC(decocass_state::decocass_quadrature_decoder_reset_w));
	map(0xe417, 0xe417).w(FUNC(decocass_state::decocass_nmi_reset_w));
	map(0xe420, 0xe42f).w(FUNC(decocass_state::decocass_adc_w));

	map(0xe500, 0xe5ff).rw(FUNC(decocass_state::decocass_e5xx_r), FUNC(decocass_state::decocass_e5xx_w)); /* read data from 8041/status */

	map(0xe600, 0xe6ff).r(FUNC(decocass_state::decocass_input_r));      /* inputs */
	map(0xe700, 0xe700).r(FUNC(decocass_state::decocass_sound_data_r)); /* read sound CPU data */
	map(0xe701, 0xe701).r(FUNC(decocass_state::decocass_sound_ack_r));  /* read sound CPU ack status */

	map(0xf000, 0xffff).rom();
}

void decocass_state::decocrom_map(address_map &map)
{
	decocass_map(map);
	map(0x6000, 0xafff).bankr("bank1").w(FUNC(decocass_state::decocass_de0091_w));
	map(0xe900, 0xe900).w(FUNC(decocass_state::decocass_e900_w));
}

void decocass_state::decocass_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x17ff).rw(FUNC(decocass_state::decocass_sound_nmi_enable_r), FUNC(decocass_state::decocass_sound_nmi_enable_w));
	map(0x1800, 0x1fff).rw(FUNC(decocass_state::decocass_sound_data_ack_reset_r), FUNC(decocass_state::decocass_sound_data_ack_reset_w));
	map(0x2000, 0x2fff).w("ay1", FUNC(ay8910_device::data_w));
	map(0x4000, 0x4fff).w("ay1", FUNC(ay8910_device::address_w));
	map(0x6000, 0x6fff).w("ay2", FUNC(ay8910_device::data_w));
	map(0x8000, 0x8fff).w("ay2", FUNC(ay8910_device::address_w));
	map(0xa000, 0xafff).r(FUNC(decocass_state::decocass_sound_command_r));
	map(0xc000, 0xcfff).w(FUNC(decocass_state::decocass_sound_data_w));
	map(0xf800, 0xffff).rom();
}


static INPUT_PORTS_START( decocass )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_START1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_START2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )                       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )                       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x30, "Type of Tape" )                          PORT_DIPLOCATION("SW1:5,6")   /* Used by the "bios" */
	PORT_DIPSETTING(    0x00, "MT (Big)" )          /* Was listed as "Board Type" with this being "OLD" */
	PORT_DIPSETTING(    0x10, "invalid?" )
	PORT_DIPSETTING(    0x20, "invalid?" )
	PORT_DIPSETTING(    0x30, "MD (Small)" )        /* Was listed as "Board Type" with this being "NEW" */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )                      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")

	PORT_START("DSW2") /* Start with all Unknown as each can change per game, except for Country Code */
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1")        /* Most Dipswitch Settings sheets show this as "Number of Players" (Lives) */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2")        /* Most Dipswitch Settings sheets show 2 & 3 as "Bonus Players" */
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4")        /* Most Dipswitch Settings sheets show 4 (with/without 5) as some form of Diffculty */
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5")
	PORT_DIPNAME( 0xe0, 0xe0, "Country Code" )                          PORT_DIPLOCATION("SW2:6,7,8") /* Always Listed as "DON'T CHANGE" */
	PORT_DIPSETTING(    0xe0, "A" )
	PORT_DIPSETTING(    0xc0, "B" )
	PORT_DIPSETTING(    0xa0, "C" )
	PORT_DIPSETTING(    0x80, "D" )
	PORT_DIPSETTING(    0x60, "E" )
	PORT_DIPSETTING(    0x40, "F" )
INPUT_PORTS_END

static INPUT_PORTS_START( cterrani )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "3000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x02, "7000" )
	PORT_DIPNAME( 0x08, 0x08, "Player's Rocket Movement" )              PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x10, 0x10, "Alien Craft Movement" )                  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	/* Switches 6, 7 & 8 are shown as completly blank */
INPUT_PORTS_END

static INPUT_PORTS_START( csuperas )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPNAME( 0x08, 0x08, "Alien Craft Movement" )                  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END


static INPUT_PORTS_START( cocean1a ) /* 10 */
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW1") /* Switches 5 and 6 defined as blank/2100/2100/2100 and "Don't change" */
	PORT_DIPNAME( 0x07, 0x07, "Number of 1 Coin Credit")                PORT_DIPLOCATION("SW1:1,2,3") /* Credits for 1 coin */
	PORT_DIPSETTING(    0x07, "1")
	PORT_DIPSETTING(    0x06, "2")
	PORT_DIPSETTING(    0x05, "5")
	PORT_DIPSETTING(    0x04, "10")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x02, "30")
	PORT_DIPSETTING(    0x01, "40")
	PORT_DIPSETTING(    0x00, "50")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( None ) )                         PORT_DIPLOCATION("SW1:4")     /* Default is "Off" and shown as "Don't change" */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Key Switch Credit" )                     PORT_DIPLOCATION("SW2:1,2")   /* Unknown */
	PORT_DIPSETTING(    0x03, "1 Coin 10 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin 20 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin 100 Credits" )
	PORT_DIPNAME( 0x04, 0x04, "Game Select" )                           PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "1 to 8 Lines" )
	PORT_DIPSETTING(    0x00, "Center Line" )
	PORT_DIPNAME( 0x08, 0x08, "Background Music" )                      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Pay Out %" )                             PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Payout 75%")
	PORT_DIPSETTING(    0x00, "Payout 85%")
	/* Switches 6 to 8 are shown as completly blank and "Don't change" */
INPUT_PORTS_END

static INPUT_PORTS_START( clocknchj )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "15000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	/* Switches 4, 5, 6, 7 & 8 are listed as "Not Used" and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( clocknch )
	PORT_INCLUDE( clocknchj )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, "Game Speed" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cprogolf )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3") /* You must shoot equal to or under the listed value for a bonus */
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x02, "6 Under" )
	PORT_DIPSETTING(    0x04, "3 Under" )
	PORT_DIPSETTING(    0x06, "1 Under" )
	PORT_DIPNAME( 0x08, 0x08, "Number of Strokes" )                     PORT_DIPLOCATION("SW2:4") /* You must shoot equal to or under to continue, else you lose a life */
	PORT_DIPSETTING(    0x00, "Par +2" )
	PORT_DIPSETTING(    0x08, "Par +3" )
	PORT_DIPNAME( 0x10, 0x10, "Show Stroke Power/Ball Direction" )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( ctower )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_JOYSTICKLEFT_DOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_JOYSTICKRIGHT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_JOYSTICKRIGHT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_JOYSTICKRIGHT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_JOYSTICKRIGHT_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_JOYSTICKLEFT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_JOYSTICKLEFT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_JOYSTICKLEFT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_JOYSTICKLEFT_DOWN ) PORT_COCKTAIL

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( cdsteljn )
	PORT_INCLUDE( decocass )

	PORT_START("P1_MP0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_MP1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_MP2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_MP3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_MP0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_MP1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_MP2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_MP3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( cexplore )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "1500000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )                   PORT_DIPLOCATION("SW2:4") /* Listed as "Missle" */
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x10, 0x10, "Number of UFOs" )                        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Few" )
	PORT_DIPSETTING(    0x00, "Many" )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END
static INPUT_PORTS_START( ctornado )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPNAME( 0x08, 0x08, "Crash Bombs" )                           PORT_DIPLOCATION("SW2:4") /* Printed English translation "Hero Destructor" */
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Alens' Speed" )                          PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cmissnx )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "5000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x02, "15000" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )                   PORT_DIPLOCATION("SW2:4,5") /* Listed as "Game Level" */
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cbtime )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x08, 0x08, "Enemies" )                               PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "End of Level Pepper" )                   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cgraplop )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x02, "70000" )
	PORT_DIPNAME( 0x08, 0x08, "Number of Up Sign" )                     PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Few" )
	PORT_DIPSETTING(    0x00, "Many" )
	PORT_DIPNAME( 0x10, 0x10, "Falling Speed" )                         PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	/* Switches 6, 7 & 8 are listed as "Not Used" and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cnightst )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "When Night Star Completed (First 2 Times)" )
	PORT_DIPSETTING(    0x04, "When Night Star Completed (First Time Only)" )
	PORT_DIPSETTING(    0x02, "Every 70000"  )
	PORT_DIPSETTING(    0x00, "30000 Only"  )
	PORT_DIPNAME( 0x08, 0x08, "Number of Missles" )                     PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Few" )
	PORT_DIPSETTING(    0x00, "Many" )
	PORT_DIPNAME( 0x10, 0x10, "Enemy's Speed" )                         PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cskater )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1") /* Listed as "Number of Balls" */
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, "60000" )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPNAME( 0x08, 0x08, "Enemy's Speed" )                         PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x10, 0x10, "Number of Skates" )                      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Small" )
	PORT_DIPSETTING(    0x00, "Large" )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cpsoccer )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1") /* Listed as "Number of Balls" */
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Number of Nice Goal" )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )                  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )                   PORT_DIPLOCATION("SW2:4") /* Listed as "Class" */
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( csdtenis )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1") /* Listed as "Number of Balls" */
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "Every 1set" )
	PORT_DIPSETTING(    0x04, "Every 2set" )
	PORT_DIPSETTING(    0x02, "Every 3set" )
	PORT_DIPNAME( 0x08, 0x08, "Speed Level" )                           PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Low Speed" )
	PORT_DIPSETTING(    0x00, "High Speed" )
	PORT_DIPNAME( 0x10, 0x10, "Attack Level" )                          PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cscrtry )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x02, "70000" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )                   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x10, 0x10, "Timer(Don't Change)" )                   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Timer decrease" )
	PORT_DIPSETTING(    0x00, "Timer infinity" )
	/* Switches 6, 7 & 8 are listed as "Special Purpose" and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cfghtice )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Very_Difficult ) )               PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( Very_Easy )  )
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x08, 0x08, "Enemy's Speed" )                         PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_SERVICE_DIPLOC(  0x10, IP_ACTIVE_LOW, "SW2:5" )    /* Listed as Test Mode, but doesn't seem to work??? */
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cbdash )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )                   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )       /* Number of Diamonds Little, Timer: Long */
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )     /* Number of Diamonds Little, Timer: Long */
	PORT_DIPSETTING(    0x08, DEF_STR( Harder ) )       /* Number of Diamonds Many, Timer: Short */
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      /* Number of Diamonds Many, Timer: Short */
	/* Switches 6, 7 & 8 are listed as "Country Code" A through F and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cfishing )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )                   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	/* Switches 5, 6, 7 & 8 are listed as "Not Used" and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( cfboy0a1 ) /* 12 */
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Number of The Deco Kids" )                PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Bonus Points" )                           PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "5000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x02, "15000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPNAME( 0x08, 0x08, "Number of Alien Missiles" )               PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x10, 0x10, "Alien Craft Movement" )                   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	/* Switches 6 to 8 are shown as "Country Code" (A to F) and "Don't Change" */
INPUT_PORTS_END

static INPUT_PORTS_START( ctisland )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "15000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "25000" )
	PORT_DIPSETTING(    0x00, "30000"  )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( cppicf )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x06, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x02, "70000" )
	PORT_DIPNAME( 0x10, 0x10, "Infinite Lives" )                        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( clapapa )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( cburnrub )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x06, "Every 30000" )
	PORT_DIPSETTING(    0x04, "Every 70000" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )               PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( cmanhat )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( cluckypo )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Show Dealer Hand" )                      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( czeroize )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR ( None ) )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x02, "70000" )
	PORT_DIPSETTING(    0x00, "90000"  )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( cflyball )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR ( None ) )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x02, "70000" )
	PORT_DIPNAME( 0x08, 0x08, "Push Backs" )                            PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( cdiscon1 )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR ( None ) )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPNAME( 0x08, 0x08, "Music Weapons" )                         PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( csweetht )
	PORT_INCLUDE( cdiscon1 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, "Music Weapons" )                         PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "8" )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( chwy )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( castfant )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	/* other dips not verified */
INPUT_PORTS_END

static INPUT_PORTS_START( cptennis )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )                   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )                   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Amateur" )
	PORT_DIPSETTING(    0x00, "Professional" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5")
	// DIPs verified from DIPSW sheet
INPUT_PORTS_END

static INPUT_PORTS_START( cprobowl )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Show Bonus Instructions" )                        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	/* other dips not verified */
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	1024,
	3,
	{ 2*1024*8*8, 1*1024*8*8, 0*1024*8*8 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	256,
	3,
	{ 2*256*16*16, 1*256*16*16, 0*256*16*16 },
	{ STEP8(16*8,1), STEP8(0*8,1) },
	{ STEP16(0,8) },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	16,
	3,
	{ 2*16*16*16+4, 2*16*16*16+0, 4 },
	{ STEP4(3*16*8,1), STEP4(2*16*8,1), STEP4(1*16*8,1), STEP4(0*16*8,1) },
	{ STEP16(0,8) },
	2*16*16
};

static const uint32_t objlayout_xoffset[64] =
{
	STEP8(7*8,1), STEP8(6*8,1), STEP8(5*8,1), STEP8(4*8,1),
	STEP8(3*8,1), STEP8(2*8,1), STEP8(1*8,1), STEP8(0*8,1)
};

static const uint32_t objlayout_yoffset[64] =
{
	STEP32(63*2*64, -1*2*64),
	STEP32(31*2*64, -1*2*64)
};

static const gfx_layout objlayout =
{
	64,64,  /* 64x64 object */
	2,      /* 2 objects */
	1,      /* 1 bits per pixel */
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	8*8, /* object takes 8 consecutive bytes */
	objlayout_xoffset,
	objlayout_yoffset
};

static GFXDECODE_START( gfx_decocass )
	GFXDECODE_ENTRY( nullptr, 0x6000, charlayout,       0, 4 )  /* char set #1 */
	GFXDECODE_ENTRY( nullptr, 0x6000, spritelayout,     0, 4 )  /* sprites */
	GFXDECODE_ENTRY( nullptr, 0xd000, tilelayout,       0, 8 )  /* background tiles */
	GFXDECODE_ENTRY( nullptr, 0xd800, objlayout,        0, 64 )  /* object */
GFXDECODE_END

void decocass_state::decocass_palette(palette_device &palette) const
{
	// set up 32 colors 1:1 pens and flipped colors for background tiles (D7 of color_center_bot)
	for (int i = 0; i < 32; i++)
	{
		palette.set_pen_indirect(i, i);
		palette.set_pen_indirect(32 + i, bitswap<8>(i, 7, 6, 5, 4, 3, 1, 2, 0));
	}
}


void decocass_state::decocass(machine_config &config)
{
	/* basic machine hardware */
	DECO_222(config, m_maincpu, HCLK4); /* the earlier revision board doesn't have the 222 but must have the same thing implemented in logic for the M6502 */
	m_maincpu->set_addrmap(AS_PROGRAM, &decocass_state::decocass_map);

	M6502(config, m_audiocpu, HCLK1/3/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &decocass_state::decocass_sound_map);
	TIMER(config, "audionmi").configure_scanline(FUNC(decocass_state::decocass_audio_nmi_gen), "screen", 0, 8);

	I8041A(config, m_mcu, HCLK);
	m_mcu->p1_in_cb().set(FUNC(decocass_state::i8041_p1_r));
	m_mcu->p1_out_cb().set(FUNC(decocass_state::i8041_p1_w));
	m_mcu->p2_in_cb().set(FUNC(decocass_state::i8041_p2_r));
	m_mcu->p2_out_cb().set(FUNC(decocass_state::i8041_p2_w));

	config.set_maximum_quantum(attotime::from_hz(4200));              /* interleave CPUs */

	WATCHDOG_TIMER(config, m_watchdog);

	DECOCASS_TAPE(config, m_cassette, 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(HCLK, 384, 0*8, 256, 272, 1*8, 248);
	m_screen->set_screen_update(FUNC(decocass_state::screen_update_decocass));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_decocass);
	PALETTE(config, m_palette, FUNC(decocass_state::decocass_palette), 64, 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	AY8910(config, "ay1", HCLK2).add_route(ALL_OUTPUTS, "mono", 0.40);

	AY8910(config, "ay2", HCLK2).add_route(ALL_OUTPUTS, "mono", 0.40);
}

void decocass_state::decocrom(machine_config &config)
{
	decocass(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &decocass_state::decocrom_map);
}


void decocass_type1_state::ctsttape(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,ctsttape)
}

void decocass_type1_state::cprogolfj(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cprogolfj)
}

void decocass_type1_state::cdsteljn(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cdsteljn)
}

void decocass_type1_state::cmanhat(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cmanhat)
}

void decocass_type3_state::cfishing(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cfishing)
}


void decocass_type1_state::chwy(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,chwy)
}


void decocass_type1_state::cterrani(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cterrani)
}


void decocass_type1_state::castfant(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,castfant)
}


void decocass_type1_state::csuperas(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,csuperas)
}


void decocass_type1_state::cocean1a(machine_config &config) /* 10 */
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cocean1a)
}


void decocass_type1_state::clocknch(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,clocknch)
}

void decocass_type1_state::clocknchj(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,clocknchj)
}

void decocass_type1_state::cfboy0a1(machine_config &config) /* 12 */
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cfboy0a1)
}


void decocass_type1_state::cprogolf(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cprogolf)
}


void decocass_type1_state::cluckypo(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cluckypo)
}


void decocass_type1_state::ctisland(machine_config &config)
{
	decocrom(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,ctisland)
}

void decocass_type1_state::ctisland3(machine_config &config)
{
	decocrom(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,ctisland3)
}

void decocass_type1_state::cexplore(machine_config &config)
{
	decocrom(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type1_state,cexplore)
}




void decocass_type3_state::cbtime(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cbtime)
}


void decocass_type3_state::cburnrub(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cburnrub)
}


void decocass_type3_state::cgraplop(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cgraplop)
}


void decocass_type3_state::cgraplop2(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cgraplop2)
}


void decocass_type3_state::clapapa(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,clapapa)
}


void decocass_type3_state::cskater(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cskater)
}


void decocass_type3_state::cprobowl(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cprobowl)
}


void decocass_type3_state::cnightst(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cnightst)
}


void decocass_type3_state::cpsoccer(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cpsoccer)
}


void decocass_type3_state::csdtenis(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,csdtenis)
}


void decocass_type3_state::czeroize(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,czeroize)
}


void decocass_type3_state::cppicf(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cppicf)
}


void decocass_type3_state::cfghtice(machine_config &config)
{
	decocass(config);

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_type3_state,cfghtice)
}



#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))


/************ Version A bios roms *************/

/* v0a.7e, New boardset bios, country code A */
#define DECOCASS_BIOS_A_MAINCPU \
	ROM_SYSTEM_BIOS( 0, "a",   "Bios A (Japan)" ) \
	ROM_LOAD_BIOS( 0, "v0a-.7e",    0xf000, 0x1000, CRC(3d33ac34) SHA1(909d59e7a993affd10224402b4370e82a5f5545c) ) /* from RMS-8 board: 2732 EPROM @7E w/'V0A-' label (has HDRA01HDR string inside it), bios code */
#define DECOCASS_BIOS_A_AUDIOCPU \
	ROM_LOAD_BIOS( 0, "v1-.5a",     0xf800, 0x0800, CRC(b66b2c2a) SHA1(0097f38beb4872e735e560148052e258a26b08fd) ) /* from RMS-8 board: 2716 eprom @5A w/V1- label,  contains audio cpu code */
#define DECOCASS_BIOS_A_PROMS \
	ROM_LOAD_BIOS( 0, "v2.3m",      0x0000, 0x0020, CRC(238fdb40) SHA1(b88e8fabb82092105c3828154608ea067acbf2e5) ) /* from DSP-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @3M w/'V2' stamp, unknown purpose (gfx related: row/interrupt/vblank related? vertical counter related) */ \
	ROM_LOAD_BIOS( 0, "v4.10d",     0x0020, 0x0020, CRC(3b5836b4) SHA1(b630bb277d9ec09d46ef26b944014dd6165b35d8) ) /* from DSP-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @10D w/'V4' stamp, unknown purpose (gfx related: tile banking? horizontal counter related) */ \
	ROM_LOAD_BIOS( 0, "v3.3j",      0x0040, 0x0020, CRC(51eef657) SHA1(eaedce5caf55624ad6ae706aedf82c5717c60f1f) ) /* from RMS-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @3J w/'V3' stamp, handles DRAM banking and timing */

/* Old boardset bios, country code A (for Japan), 2x 2716 EPROM, MD labbeled as RMS-3D and MT as RMS-3T, region code (letter) is (not always) inserted after "-" */ \
#define DECOCASS_BIOS_A0_MAINCPU \
	ROM_SYSTEM_BIOS( 1, "a0",   "Bios A (Japan, older)" ) \
	ROM_LOAD_BIOS( 1, "dsp-3_p0-a.m9",      0xf000, 0x0800, CRC(2541e34b) SHA1(4f983513dbae1350c83a433dea77a4465748b9c6) ) \
	ROM_LOAD_BIOS( 1, "dsp-3_p1-.l9",       0xf800, 0x0800, CRC(3bfff5f3) SHA1(4e9437cb1b76d64da6b37f01bd6e879fb399e8ce) )
#define DECOCASS_BIOS_A0_AUDIOCPU \
	ROM_LOAD_BIOS( 1, "rms-3_p2-.c9",       0xfc00, 0x0400, CRC(6c4a891f) SHA1(5c00cf8b1accfdbb1d61e9b3f6db1594dfbc608b) ) /* 2708 EPROM, contains audio cpu code */
#define DECOCASS_BIOS_A0_PROMS \
	ROM_LOAD_BIOS( 1, "dsp-3_p3-.e5",       0x0000, 0x0020, CRC(539a5a64) SHA1(7b7d3cc58ac6f95242240c97046e770d2fd20c96) ) /* M3-7603-5 (82s123 equiv, 32x8 TS) PROM, unknown purpose (gfx related: row/interrupt/vblank related? vertical counter related) */ \
	ROM_LOAD_BIOS( 1, "rms-3_p4-.f6",       0x0020, 0x0020, CRC(9014c0fd) SHA1(7405d39a5f4fcad821448ddaf6bd4e27c0c9e145) ) /* M3-7603-5 (82s123 equiv, 32x8 TS) PROM, unknown purpose (gfx related: tile banking? horizontal counter related) */ \
	ROM_LOAD_BIOS( 1, "dsp-3_p5-.m4",       0x0040, 0x0020, CRC(e52089a0) SHA1(d85c17809b089c6977ee9571f976af6f107fd4d3) ) /* M3-7603-5 (82s123 equiv, 32x8 TS) PROM, handles DRAM banking and timing */ \

/************ Version B bios roms *************/

/* rms8.7e, New boardset bios, country code B */ \
#define DECOCASS_BIOS_B_MAINCPU \
	ROM_SYSTEM_BIOS( 2, "b",   "Bios B (USA)" ) \
	ROM_LOAD_BIOS( 2, "v0b-.7e",    0xf000, 0x1000, CRC(23d929b7) SHA1(063f83020ba3d6f43ab8471f95ca919767b93aa4) ) /* from RMS-8 board: 2732 EPROM @7E w/'V0B-' label (has HDRB01HDR string inside it), bios code */
#define DECOCASS_BIOS_B_AUDIOCPU \
	ROM_LOAD_BIOS( 2, "v1-.5a",     0xf800, 0x0800, CRC(b66b2c2a) SHA1(0097f38beb4872e735e560148052e258a26b08fd) ) /* from RMS-8 board: 2716 eprom @5A w/V1- label,  contains audio cpu code */
#define DECOCASS_BIOS_B_PROMS \
	ROM_LOAD_BIOS( 2, "v2.3m",      0x0000, 0x0020, CRC(238fdb40) SHA1(b88e8fabb82092105c3828154608ea067acbf2e5) ) /* from DSP-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @3M w/'V2' stamp, unknown purpose (gfx related: row/interrupt/vblank related? vertical counter related) */ \
	ROM_LOAD_BIOS( 2, "v4.10d",     0x0020, 0x0020, CRC(3b5836b4) SHA1(b630bb277d9ec09d46ef26b944014dd6165b35d8) ) /* from DSP-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @10D w/'V4' stamp, unknown purpose (gfx related: tile banking? horizontal counter related) */ \
	ROM_LOAD_BIOS( 2, "v3.3j",      0x0040, 0x0020, CRC(51eef657) SHA1(eaedce5caf55624ad6ae706aedf82c5717c60f1f) ) /* from RMS-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @3J w/'V3' stamp, handles DRAM banking and timing */

/* Old boardset bios, version B for USA, 2x 2716 EPROM, MD labbeled as RMS-3D and MT as RMS-3T, region code (letter) is (not always) inserted after "-" */ \
/* dsp3.p0b/p1b, Old boardset bios, country code B?; from DSP-3 board? has HDRB01x string in it, 2x 2716 EPROM? */ \
#define DECOCASS_BIOS_B0_MAINCPU \
	ROM_SYSTEM_BIOS( 3, "b0",   "Bios B (USA, older)" ) \
	ROM_LOAD_BIOS( 3, "dsp-3_p0-b.m9",      0xf000, 0x0800, CRC(b67a91d9) SHA1(681c040be0f0ed1ba0a50161b36d0ad8e1c8c5cb) ) \
	ROM_LOAD_BIOS( 3, "dsp-3_p1-.l9",       0xf800, 0x0800, CRC(3bfff5f3) SHA1(4e9437cb1b76d64da6b37f01bd6e879fb399e8ce) )
#define DECOCASS_BIOS_B0_AUDIOCPU \
	ROM_LOAD_BIOS( 3, "rms-3_p2-.c9",       0xfc00, 0x0400, CRC(6c4a891f) SHA1(5c00cf8b1accfdbb1d61e9b3f6db1594dfbc608b) ) /* 2708 EPROM, contains audio cpu code */
#define DECOCASS_BIOS_B0_PROMS \
	ROM_LOAD_BIOS( 3, "dsp-3_p3-.e5",       0x0000, 0x0020, CRC(539a5a64) SHA1(7b7d3cc58ac6f95242240c97046e770d2fd20c96) ) /* M3-7603-5 (82s123 equiv, 32x8 TS) PROM, unknown purpose (gfx related: row/interrupt/vblank related? vertical counter related) */ \
	ROM_LOAD_BIOS( 3, "rms-3_p4-.f6",       0x0020, 0x0020, CRC(9014c0fd) SHA1(7405d39a5f4fcad821448ddaf6bd4e27c0c9e145) ) /* M3-7603-5 (82s123 equiv, 32x8 TS) PROM, unknown purpose (gfx related: tile banking? horizontal counter related) */ \
	ROM_LOAD_BIOS( 3, "dsp-3_p5-.m4",       0x0040, 0x0020, CRC(e52089a0) SHA1(d85c17809b089c6977ee9571f976af6f107fd4d3) ) /* M3-7603-5 (82s123 equiv, 32x8 TS) PROM, handles DRAM banking and timing */ \

/* rms8.7e, New boardset bios, country code D */ \
#define DECOCASS_BIOS_D_MAINCPU \
	ROM_SYSTEM_BIOS( 4, "d",   "Bios D (Europe?)" ) \
	ROM_LOAD_BIOS( 4, "v0d-.7e",    0xf000, 0x1000, CRC(1e0c22b1) SHA1(5fec8fef500bbebc13d0173406afc55235d3affb) ) /* handcrafted (single byte changed) because ctisland3 requires region D */
#define DECOCASS_BIOS_D_AUDIOCPU \
	ROM_LOAD_BIOS( 4, "v1-.5a",     0xf800, 0x0800, CRC(b66b2c2a) SHA1(0097f38beb4872e735e560148052e258a26b08fd) ) /* from RMS-8 board: 2716 eprom @5A w/V1- label,  contains audio cpu code */
#define DECOCASS_BIOS_D_PROMS \
	ROM_LOAD_BIOS( 4, "v2.3m",      0x0000, 0x0020, CRC(238fdb40) SHA1(b88e8fabb82092105c3828154608ea067acbf2e5) ) /* from DSP-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @3M w/'V2' stamp, unknown purpose (gfx related: row/interrupt/vblank related? vertical counter related) */ \
	ROM_LOAD_BIOS( 4, "v4.10d",     0x0020, 0x0020, CRC(3b5836b4) SHA1(b630bb277d9ec09d46ef26b944014dd6165b35d8) ) /* from DSP-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @10D w/'V4' stamp, unknown purpose (gfx related: tile banking? horizontal counter related) */ \
	ROM_LOAD_BIOS( 4, "v3.3j",      0x0040, 0x0020, CRC(51eef657) SHA1(eaedce5caf55624ad6ae706aedf82c5717c60f1f) ) /* from RMS-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @3J w/'V3' stamp, handles DRAM banking and timing */



/************ Common MCU bios rom *************/

#define DECOCASS_BIOS_MCU \
	ROM_LOAD( "cassmcu.1c", 0x0000, 0x0400, CRC(a6df18fd) SHA1(1f9ea47e372d31767c936c15852b43df2b0ee8ff) ) /* from B10-B board: "NEC // JAPAN // X1202D-108 // D8041C 535" 8041 MCU @1C, handles cassette and other stuff; This info needs additional verification, as the d8041-535 mcu has not been dumped yet to prove code is the same. */



#define DECOCASS_BIOS_MAIN \
	ROM_REGION( 0x10000, "maincpu", 0 ) \
	DECOCASS_BIOS_A_MAINCPU \
	DECOCASS_BIOS_A0_MAINCPU \
	DECOCASS_BIOS_B_MAINCPU \
	DECOCASS_BIOS_B0_MAINCPU \
	DECOCASS_BIOS_D_MAINCPU \
	ROM_REGION( 0x10000, "audiocpu", 0 ) \
	DECOCASS_BIOS_A_AUDIOCPU \
	DECOCASS_BIOS_A0_AUDIOCPU \
	DECOCASS_BIOS_B_AUDIOCPU \
	DECOCASS_BIOS_B0_AUDIOCPU \
	DECOCASS_BIOS_D_AUDIOCPU \
	ROM_REGION( 0x00060, "proms", 0 ) \
	DECOCASS_BIOS_A_PROMS \
	DECOCASS_BIOS_A0_PROMS \
	DECOCASS_BIOS_B_PROMS \
	DECOCASS_BIOS_B0_PROMS \
	DECOCASS_BIOS_D_PROMS \
	ROM_REGION( 0x10000, "mcu", 0 )   /* 4k for the 8041 MCU (actually 1K ROM + 64 bytes RAM @ 0x800) */ \
	DECOCASS_BIOS_MCU


#define DECOCASS_BIOS_B_ROMS \
	DECOCASS_BIOS_MAIN \
	ROM_DEFAULT_BIOS( "b" )

#define DECOCASS_BIOS_BO_ROMS   \
	DECOCASS_BIOS_MAIN \
	ROM_DEFAULT_BIOS( "b0" )

#define DECOCASS_BIOS_A_ROMS \
	DECOCASS_BIOS_MAIN \
	ROM_DEFAULT_BIOS( "a" )

#define DECOCASS_BIOS_AO_ROMS   \
	DECOCASS_BIOS_MAIN \
	ROM_DEFAULT_BIOS( "a0" )

#define DECOCASS_BIOS_D_ROMS \
	DECOCASS_BIOS_MAIN \
	ROM_DEFAULT_BIOS( "d" )

ROM_START( decocass )
	DECOCASS_BIOS_MAIN
ROM_END

/* The Following use Dongle Type 1 (DE-0061)
    (dongle data same for each game)         */

ROM_START( ctsttape )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "testtape.cas", 0x0000, 0x2000, CRC(4f9d8efb) SHA1(5b77747dad1033e5703f06c0870441b54b4256c5) )
ROM_END

/* 01 Highway Chase */
ROM_START( chwy )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	/* The dongle data is reverse engineered from manual decryption */
	ROM_LOAD( "chwy.pro",   0x0000, 0x0020, BAD_DUMP CRC(2fae678e) SHA1(4a7de851442d4c1d690de03262f0e136a52fca35) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "chwy.cas",   0x0000, 0x8000, CRC(68a48064) SHA1(7e389737972fd0c54f398d296159c561f5ec3a93) )
ROM_END

/* 03 Manhatten */
ROM_START( cmanhat )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "manhattan.pro",   0x0000, 0x0020, CRC(1bc9fccb) SHA1(ffc59c7660d5c87a8deca294f80260b6bc7c3027) ) // == a-0061.dgl

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "manhattan.cas", 0x000000, 0x006000, CRC(92dae2b1) SHA1(cc048ac6601553675078230290beb3d59775bfe0) )
ROM_END

/* 04 Terranean */
ROM_START( cterrani )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1040.dgl", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1040.cas", 0x0000, 0x8000, CRC(eb71adbc) SHA1(67becfde39c034d4b8edc2eb100050de102773da) )
ROM_END

/* 07 Astro Fantasia */
ROM_START( castfant )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "castfant.cas", 0x0000, 0x8000, CRC(6d77d1b5) SHA1(821bd65fbe887cbeac9281a2ad3f88595918f886) )
ROM_END

/* 08 The Tower */
ROM_START( ctower ) // no copyright display? (possibly by design, as it lifts a lot from Crazy Climber)
	DECOCASS_BIOS_D_ROMS

	ROM_REGION( 0x10000, "cassette", 0 )  /* (max) 64k for cassette image */
	ROM_LOAD( "ctower.cas", 0x0000, 0x6900, CRC(94ad1dd6) SHA1(d54691ad8802b63ff4533202d6e6b29d4652c4f6) )

	/* this is handcrafted, it ends up being the same as ctisland3 but with the lower data and address lines
	   both swapped.  ctisland3 is also handcrafted and could in turn just be a transformation of one of the
	   standard PROMs, the data order ends up being arbitrary for these handcrafted PROMs as there are
	   multiple places transformations could be occurring */
	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "ctower.pro",   0x0000, 0x0020, CRC(32e9dcd7) SHA1(956b593911f6337008c375cda4dac43043d921dd) )
ROM_END

/* 09 Super Astro Fighter */
ROM_START( csuperas )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "csuperas.cas", 0x0000, 0x8000, CRC(fabcd07f) SHA1(4070c668ad6725f0710cf7fe6df0d5f80272a449) )
ROM_END

/* 10 Ocean to Ocean (World) */
ROM_START( cocean1a ) // version MD 1-A-0 verified, 061 blocks, decrypted main data CRC(b97ab3cb)
	DECOCASS_BIOS_AO_ROMS

	ROM_REGION( 0x10000, "cassette", 0 )  /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1101-a-0.cas", 0x0000, 0x3e00, CRC(db8ab848) SHA1(2b2acb249bf66e6c5e15d89b7ebd294ed2eee066) )

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1100-a.rom",   0x0000, 0x0020, CRC(1bc9fccb) SHA1(ffc59c7660d5c87a8deca294f80260b6bc7c3027) )
ROM_END

ROM_START( cocean6b ) // version MD 10-B-0 not verified, 068 blocks, decrypted main data CRC(410d1f19)
	DECOCASS_BIOS_BO_ROMS

	ROM_REGION( 0x10000, "cassette", 0 )  /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1106-b-0.cas", 0x0000, 0x4500, CRC(fa6ffc95) SHA1(95f881503aa8cd97d04b327abeb68891d053563f) )

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1100-b.rom",   0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )
ROM_END

/* 11 Lock'n'Chase */
ROM_START( clocknch )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1110_b.dgl", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "clocknch.cas",  0x0000, 0x8000, CRC(c9d163a4) SHA1(3ef55a8d8f603059e263776c08eb81f2cf18b75c) )
ROM_END

ROM_START( clocknchj )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "a-0061.dgl",   0x0000, 0x0020, CRC(1bc9fccb) SHA1(ffc59c7660d5c87a8deca294f80260b6bc7c3027) ) /* ? */

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1111-a-0.bin",  0x0000, 0x6300, CRC(9753e815) SHA1(fd0c8e4733e1548fe47a4d34a2f6ce48d9303e22) )
ROM_END

/* 12 Flash Boy (early vertical Japan version, then horizontal version), The Deco Kid (early vertical World version, then vertical version) */
ROM_START( cfboy0a1 ) // version MD 0-A-1 verified, 105 blocks, decrypted main data CRC(7ca358f0)
	DECOCASS_BIOS_AO_ROMS

	ROM_REGION( 0x10000, "cassette", 0 )  /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1120-a-1.cas", 0x0000, 0x6a00, CRC(c6746dc0) SHA1(816ccb61dfa2745a9ef918d9a50d1cd91493c9ed) )

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1120-a.rom",   0x0000, 0x0020, CRC(1bc9fccb) SHA1(ffc59c7660d5c87a8deca294f80260b6bc7c3027) )
ROM_END

/* 13 */
/* Photo of Dongle shows DP-1130B (the "B" is in a separate white box then the DP-1130 label) */
ROM_START( cprogolf ) // version 9-B-0
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1130_b.dgl", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1130_9b.cas",  0x0000, 0x8000, CRC(02123cd1) SHA1(e4c630ed293725f23d539cb43beb97953558dabd) )
ROM_END

ROM_START( cprogolfj ) // version 1-A
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "a-0061.dgl",   0x0000, 0x0020, CRC(1bc9fccb) SHA1(ffc59c7660d5c87a8deca294f80260b6bc7c3027) ) /* Should be dp-1130a?? */

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-113_a.cas",   0x0000, 0x8000, CRC(8408248f) SHA1(8b78c379bf6879916bc9b284d7a0956edfac78be) )
ROM_END

// version number is DT-1134-A-0 (Japanese, version 4 of Pro Golf, no revision number)
ROM_START( cprogolf18 )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "de-0061-a-0.rom",   0x0000, 0x0020, CRC(1bc9fccb) SHA1(ffc59c7660d5c87a8deca294f80260b6bc7c3027) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "progolf18.cas",   0x0000, 0x6700, CRC(3024396c) SHA1(c49d878bae46bf8bf0b0b098a5d94d9ec68b526d) )
ROM_END

/* 14 */
ROM_START( cdsteljn ) // version 4-A-3
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "a-0061.dgl",   0x0000, 0x0020, CRC(1bc9fccb) SHA1(ffc59c7660d5c87a8deca294f80260b6bc7c3027) ) /* Should be dp-1144a?? */

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1144-a3.cas", 0x000000, 0x007300, CRC(1336a912) SHA1(0c64e069713b411da38b43f14306953621726d35) )
ROM_END

/* 15 Lucky Poker */
/* Photo of Dongle shows DP-1150B (the "B" is in a separate white box then the DP-1150 label) */
ROM_START( cluckypo )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1150_b.dgl", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cluckypo.cas",  0x0000, 0x8000, CRC(2070c243) SHA1(cd3af309af8eb27937756c1fe6fd0504be5aaaf5) )
ROM_END

/* 16 Treasure Island */
ROM_START( ctisland )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "ctisland.cas", 0x0000, 0x8000, CRC(3f63b8f8) SHA1(2fd0679ef9750a228ebb098672ab6091fda75804) )

	ROM_REGION( 0xa000, "user3", ROMREGION_ERASEFF )      /* roms from the overlay pcb */
	ROM_LOAD( "deco-ti.x1",   0x0000, 0x1000, CRC(a7f8aeba) SHA1(0c9ba1a46d0636b36f40fad31638db89f374f778) )
	ROM_LOAD( "deco-ti.x2",   0x1000, 0x1000, CRC(2a0d3c91) SHA1(552d08fcddddbea5b52fa1e8decd188ae49c86ea) )
	ROM_LOAD( "deco-ti.x3",   0x2000, 0x1000, CRC(3a26b97c) SHA1(f57e76077806e149a9e455c85e5431eac2d42bc3) )
	ROM_LOAD( "deco-ti.x4",   0x3000, 0x1000, CRC(1cbe43de) SHA1(8f26ad224e96c87da810c60d3dd88d415400b9fc) )
ROM_END

ROM_START( ctisland2 )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "ctislnd2.cas", 0x0000, 0x8000, CRC(2854b4c0) SHA1(d3b4e0031dbb2340fbbe396a1ff9b8fbfd63663e) )

	ROM_REGION( 0xa000, "user3", ROMREGION_ERASEFF )      /* roms from the overlay pcb */
	ROM_LOAD( "deco-ti.x1",   0x0000, 0x1000, CRC(a7f8aeba) SHA1(0c9ba1a46d0636b36f40fad31638db89f374f778) )
	ROM_LOAD( "deco-ti.x2",   0x1000, 0x1000, CRC(2a0d3c91) SHA1(552d08fcddddbea5b52fa1e8decd188ae49c86ea) )
	ROM_LOAD( "deco-ti.x3",   0x2000, 0x1000, CRC(3a26b97c) SHA1(f57e76077806e149a9e455c85e5431eac2d42bc3) )
	ROM_LOAD( "deco-ti.x4",   0x3000, 0x1000, CRC(1cbe43de) SHA1(8f26ad224e96c87da810c60d3dd88d415400b9fc) )
ROM_END

ROM_START( ctisland3 )
	DECOCASS_BIOS_D_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "ctisland3.pro", 0x0000, 0x0020, CRC(b87b56a7) SHA1(ca84cd61e53985cf24230d297967374ae3822b3b) ) // handcrafted

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "ctislnd3.cas", 0x0000, 0x8000, CRC(45464e1e) SHA1(03275694d963c7ab0e0f5525e248e69da5f9b591) )

	ROM_REGION( 0xa000, "user3", ROMREGION_ERASEFF )      /* roms from the overlay pcb */
	ROM_LOAD( "deco-ti.x1",   0x0000, 0x1000, CRC(a7f8aeba) SHA1(0c9ba1a46d0636b36f40fad31638db89f374f778) )
	ROM_LOAD( "deco-ti.x2",   0x1000, 0x1000, CRC(2a0d3c91) SHA1(552d08fcddddbea5b52fa1e8decd188ae49c86ea) )
	ROM_LOAD( "deco-ti.x3",   0x2000, 0x1000, CRC(3a26b97c) SHA1(f57e76077806e149a9e455c85e5431eac2d42bc3) )
	ROM_LOAD( "deco-ti.x4",   0x3000, 0x1000, CRC(1cbe43de) SHA1(8f26ad224e96c87da810c60d3dd88d415400b9fc) )
ROM_END

/* 18 Explorer */
/* Photo of Dongle shows DP-1180B (the "B" is in a separate white box then the DP-1180 label) */
ROM_START( cexplore )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	/* The dongle data is reverse engineered by table analysis */
	ROM_LOAD( "dp-1180_b.dgl", 0x0000, 0x0020, BAD_DUMP CRC(c7a9ac8f) SHA1(b0a566d948f71a4eddcde0dd5e9e69ca96f71c36) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cexplore.cas", 0x0000, 0x8000, CRC(fae49c66) SHA1(4ae69e2f706fdf30204f0aa1277619395cacc21b) )

	ROM_REGION( 0xa000, "user3", ROMREGION_ERASEFF )      /* roms from the DECO GRO DE-0091C-1 overlay pcb */
	ROM_LOAD( "x1_made_in_japan_18.x1",   0x0000, 0x1000, CRC(f2ca58f0) SHA1(5c9faeca6247b70586dc2a3765805ac96960ac79) )
	ROM_LOAD( "x2_made_in_japan_18.x2",   0x1000, 0x1000, CRC(75d999bf) SHA1(7c257285d5b69642ec542dc56defdbb1f2072454) )
	ROM_LOAD( "x3_made_in_japan_18.x3",   0x2000, 0x1000, CRC(941539c6) SHA1(2e879107f56bf258ad90fb83c2ab278027acb0bb) ) // FIXED BITS (1xxxxxxx) (but correct?)
	ROM_LOAD( "x4_made_in_japan_18.x4",   0x3000, 0x1000, CRC(73388544) SHA1(9c98f79e431d0881e20eac4c6c4177db8973ce20) ) // FIXED BITS (1xxxxxxx) (but correct?)
	ROM_LOAD( "x5_made_in_japan_18.x5",   0x4000, 0x1000, CRC(b40699c5) SHA1(4934283d2845dbd3ea9a7fa349f663a34fcdfdf8) )
	ROM_LOAD( "y1_made_in_japan_18.y1",   0x5000, 0x1000, CRC(d887dc50) SHA1(9321e40d208bd029657ab87eaf815f8a09e49b48) )
	ROM_LOAD( "y2_made_in_japan_18.y2",   0x6000, 0x1000, CRC(fe325d0d) SHA1(3e4aaba87e2aa656346169d512d70083605692c6) )
	ROM_LOAD( "y3_made_in_japan_18.y3",   0x7000, 0x1000, CRC(7a787ecf) SHA1(5261747823b58be3fabb8d1a8cb4069082f95b30) )
	ROM_LOAD( "y4_made_in_japan_18.y4",   0x8000, 0x1000, CRC(ac30e8c7) SHA1(f8f53b982df356e5bf2624afe0f8a72635b3b4b3) )
	ROM_LOAD( "y5_made_in_japan_18.y5",   0x9000, 0x1000, CRC(0a6b8f03) SHA1(09b477579a5fed4c45299b6366141ef4a8c9a410) )
ROM_END

/* The Following use Dongle Type 2 (CS82-007)
    (dongle data differs for each game)      */

/* 19 Disco No.1 / Sweet Heart */
ROM_START( cdiscon1 )
/* Photo of Dongle shows DP-1190B (the "B" is in a separate white box then the DP-1190 label) */
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00800, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1190_b.dgl", 0x0000, 0x0800, CRC(0f793fab) SHA1(331f1b1b482fcd10f42c388a503f9af62d705401) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cdiscon1.cas", 0x0000, 0x8000, CRC(1429a397) SHA1(12f9e03fcda31dc6161a39bf5c3315a1e9e94565) )
ROM_END

ROM_START( csweetht )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00800, "dongle", 0 )   /* dongle data */
	ROM_LOAD( "cdiscon1.pro", 0x0000, 0x0800, CRC(0f793fab) SHA1(331f1b1b482fcd10f42c388a503f9af62d705401) )

	ROM_REGION( 0x10000, "cassette", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "csweetht.cas", 0x0000, 0x8000, CRC(175ef706) SHA1(49b86233f69d0daf54a6e59b86e69b8159e8f6cc) )
ROM_END

/* 20 Tornado */
ROM_START( ctornado )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00800, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "ctornado.pro", 0x0000, 0x0800, CRC(c9a91697) SHA1(3f7163291edbdf1a596e3cd2b7a16bbb140ffb36) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "ctornado.cas", 0x0000, 0x8000, CRC(e4e36ce0) SHA1(48a11823121fb2e3de31ae08e453c0124fc4f7f3) )
ROM_END

/* 21 Mission-X */
/* Photo of Dongle shows DP-121B with Cassette DT-1213B (the "3B" is in a separate white box then the DP-121 label) */
ROM_START( cmissnx )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00800, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-121_b.dgl", 0x0000, 0x0800, CRC(8a41c071) SHA1(7b16d933707bf21d25dcd11db6a6c28834b11c5b) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cmissnx.cas",  0x0000, 0x8000, CRC(3a094e11) SHA1(c355fe14838187cbde19a799e5c60083c82615ac) ) /* Is this the 3B version? */
ROM_END

/* 22 Pro Tennis */
ROM_START( cptennis )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00800, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cptennis.pro", 0x0000, 0x0800, CRC(59b8cede) SHA1(514861a652b5256a11477fc357bc01dfd87f712b) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cptennis.cas", 0x0000, 0x8000, CRC(6bb257fe) SHA1(7554bf1996bc9e9c04a276aab050708d70103f54) )
ROM_END

ROM_START( cptennisj )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x00800, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1220-a-0.rom", 0x0000, 0x0800, CRC(1c603239) SHA1(6c97cfbb581f72e8c26a3fc5f06f9d6aa56883ba) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1222-a-0.bin", 0x0000, 0x6a00, CRC(ee29eba7) SHA1(fd3aebb81d83120d1afb4d9a216332363d695234) )
ROM_END



/* The Following use Dongle Type 3 (unknown part number?)
    (dongle data differs for each game)      */

/* 25 Fishing / Angler Dangler */
ROM_START( cadanglr ) // version 5-B-0
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1250-a-0.dgl", 0x0000, 0x1000, CRC(92a3b387) SHA1(e17a155d02e9ed806590b23a845dc7806b6720b1) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1255-b-0.cas",   0x0000, 0x7400, CRC(eb985257) SHA1(1285724352a59c96cc4edf4f43e89dd6d8c585b2) )
ROM_END

ROM_START( cfishing )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1250-a-0.dgl", 0x0000, 0x1000, CRC(92a3b387) SHA1(e17a155d02e9ed806590b23a845dc7806b6720b1) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1250-a-0.cas",   0x0000, 0x7500, CRC(d4a16425) SHA1(25afaabdc8b2217d5e73606a36ea9ba408d7bc4b) )
ROM_END


/* 26 Hamburger / Burger Time */
/* Photo of Dongle shows DP-126B with Cassette DT-1267B (the "7B" is in a separate white box then the DP-126 label) */
ROM_START( cbtime ) // version 7-B-0
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-126_b.dgl", 0x0000, 0x1000, CRC(25bec0f0) SHA1(9fb1f9699f37937421e26d4fb8fdbcd21a5ddc5c) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-126_7b.cas",   0x0000, 0x8000, CRC(56d7dc58) SHA1(34b2513c9ca7ab40f532b6d6d911aa3012113632) )
ROM_END

ROM_START( chamburger ) // version 0-A-0
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-126_a.dgl",   0x0000, 0x1000, CRC(25bec0f0) SHA1(9fb1f9699f37937421e26d4fb8fdbcd21a5ddc5c) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-126_a.cas",   0x0000, 0x8000, CRC(334fb987) SHA1(c55906bf6059686dd8a587dabbe3fb4d59200ab9) )
ROM_END

/* 27 Burnin' Rubber / Bump 'n' Jump */
/* Photo of Dongle shows DP-127B with Cassette DP-1275B (the "5B" is in a separate white box then the DP-127 label) */
ROM_START( cburnrub )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-127_b.pro",   0x0000, 0x1000, CRC(9f396832) SHA1(0e302fd094474ac792882948a018c73ce76e0759) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cburnrub.cas",   0x0000, 0x8000, CRC(4528ac22) SHA1(dc0fcc5e5fd21c1c858a90f43c175e36a24b3c3d) ) /* Is this the 5B version? */
ROM_END

ROM_START( cburnrub2 )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-127_b.pro",   0x0000, 0x1000, CRC(9f396832) SHA1(0e302fd094474ac792882948a018c73ce76e0759) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cburnrb2.cas",   0x0000, 0x8000, CRC(84a9ed66) SHA1(a9c536e46b89fc6b9c6271776292fed1241d2f3f) ) /* Is this the 5B version? */
ROM_END

ROM_START( cbnj )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-127_b.pro",   0x0000, 0x1000, CRC(9f396832) SHA1(0e302fd094474ac792882948a018c73ce76e0759) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cbnj.cas",       0x0000, 0x8000, CRC(eed41560) SHA1(85d5df76efac33cd10427f659c4259afabb3daaf) )
ROM_END

ROM_START( cburnrubj )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-127_b.pro",   0x0000, 0x1000, CRC(9f396832) SHA1(0e302fd094474ac792882948a018c73ce76e0759) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1271-a-0.bin",   0x0000, 0x7800, CRC(6bd0adab) SHA1(4c536991e4ec6cbdf4b74497dae9f0dba17ebb95) )
ROM_END

/* 28 Graplop / Cluster Buster */
ROM_START( cgraplop )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cgraplop.pro", 0x0000, 0x1000, CRC(ee93787d) SHA1(0c753d62fdce2fdbd5b329a5aa259a967d07a651) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cgraplop.cas", 0x0000, 0x8000, CRC(d2c1c1bb) SHA1(db67304caa11540363735e7d4bf03507ccbe9980) )
ROM_END

ROM_START( cgraplopj )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cgraplop.pro", 0x0000, 0x1000, CRC(ee93787d) SHA1(0c753d62fdce2fdbd5b329a5aa259a967d07a651) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-1280-a-0.bin", 0x0000, 0x7800, CRC(a0d7d1a7) SHA1(4260edd19786b6cf4cd0c426783637f0c61ca007) )
ROM_END

ROM_START( cgraplop2 )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cgraplop.pro", 0x0000, 0x1000, CRC(ee93787d) SHA1(0c753d62fdce2fdbd5b329a5aa259a967d07a651) ) /* is this right for this set? */

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cgraplop2.cas", 0x0000, 0x8000, CRC(2e728981) SHA1(83ba90d95858d647315a1c311b8643672afea5f7) )
ROM_END

/* 29 La-Pa-Pa / Rootin' Tootin' */
ROM_START( clapapa )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "clapapa.pro",  0x0000, 0x1000, CRC(e172819a) SHA1(3492775f4f0a0b31ce5a1a998076829b3f264e98) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "clapapa.cas",  0x0000, 0x8000, CRC(4ffbac24) SHA1(1ec0d7ac1886d4b430dc12be27f387e9d952d235) )
ROM_END

ROM_START( clapapa2 )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )   /* dongle data */
	ROM_LOAD( "clapapa.pro",  0x0000, 0x1000, CRC(e172819a) SHA1(3492775f4f0a0b31ce5a1a998076829b3f264e98) )

	ROM_REGION( 0x10000, "cassette", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "clapapa2.cas",  0x0000, 0x8000, CRC(069dd3c4) SHA1(5a19392c7ac5aea979187c96267e73bf5126307e) )
ROM_END

/* 30 Skater */
ROM_START( cskater )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-130_a.dgl",   0x0000, 0x1000,  CRC(469e80a8) SHA1(f581cd534ce6faba010c6616538cdf9d96d787da) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-130_a.cas",   0x0000, 0x8000,  CRC(1722e5e1) SHA1(e94066ead608df85d3f7310d4a81ba291da4bee6) )
ROM_END

/* 31 Pro Bowling */
ROM_START( cprobowl )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cprobowl.pro", 0x0000, 0x1000, CRC(e3a88e60) SHA1(e6e9a2e5ab26e0463c63201a15f7d5a429ec836e) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cprobowl.cas", 0x0000, 0x8000, CRC(cb86c5e1) SHA1(66c467418cff2ed6d7c121a8b1650ee97ae48fe9) )
ROM_END

/* 32 Night Star */
ROM_START( cnightst )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cnightst.pro", 0x0000, 0x1000, CRC(553b0fbc) SHA1(2cdf4560992b62e59b6de760d7996be4ed25f505) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cnightst.cas", 0x0000, 0x8000, CRC(c6f844cb) SHA1(5fc6154c20ee4e2f4049a78df6f3cacbb96b0dc0) )
ROM_END

ROM_START( cnightst2 )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )   /* dongle data */
	ROM_LOAD( "cnightst.pro", 0x0000, 0x1000, CRC(553b0fbc) SHA1(2cdf4560992b62e59b6de760d7996be4ed25f505) )

	ROM_REGION( 0x10000, "cassette", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "cnights2.cas", 0x0000, 0x8000, CRC(1a28128c) SHA1(4b620a1919d02814f734aba995115c09dc2db930) )
ROM_END

/* 33 Pro Soccer */
ROM_START( cpsoccer )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cprosocc.pro", 0x0000, 0x1000,  CRC(919fabb2) SHA1(3d6a0676cea7b0be0fe69d06e04ca08c36b2851a) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cprosocc.cas", 0x0000, 0x10000, CRC(76b1ad2c) SHA1(6188667e5bc001dfdf83deaf7251eae794de4702) )
ROM_END

ROM_START( cpsoccerj )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-133_a.dgl",   0x0000, 0x1000,  CRC(919fabb2) SHA1(3d6a0676cea7b0be0fe69d06e04ca08c36b2851a) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-133_a.cas",   0x0000, 0x10000, CRC(de682a29) SHA1(2ee0dd8cb7fb595020d730a9da5d9cccda3f1264) )
ROM_END

/* 34 Super Doubles Tennis */
ROM_START( csdtenis )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-134_a.dgl",   0x0000, 0x1000,  CRC(e484d2f5) SHA1(ee4e4c221933d391aeed8ff7182fa931a4e01466) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-134_a.cas",   0x0000, 0x10000, CRC(9a69d961) SHA1(f88e267815ca0697708aca0ac9fa6f7664a0519c) )
ROM_END

/* 37 Zeroize */
ROM_START( czeroize )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "czeroize.pro",  0x0000, 0x1000, NO_DUMP ) /* The Following have unknown Dongles (dongle data not read) */

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "czeroize.cas",   0x0000, 0x10000, CRC(3ef0a406) SHA1(645b34cd477e0bb5539c8fe937a7a2dbd8369003) )
ROM_END

/* 39 Peter Pepper's Ice Cream Factory */
ROM_START( cppicf )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cppicf.pro",   0x0000, 0x1000, CRC(0b1a1ecb) SHA1(2106da6837c78812c102b0eaaa1127fcc21ea780) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cppicf.cas",   0x0000, 0x8000, CRC(8c02f160) SHA1(03430dd8d4b2e6ca931986dac4d39be6965ffa6f) )
ROM_END

ROM_START( cppicf2 )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )   /* dongle data */
	ROM_LOAD( "cppicf.pro",   0x0000, 0x1000, CRC(0b1a1ecb) SHA1(2106da6837c78812c102b0eaaa1127fcc21ea780) )

	ROM_REGION( 0x10000, "cassette", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "cppicf2.cas",   0x0000, 0x8000, CRC(78ffa1bc) SHA1(d15f2a240ae7b45885d32b5f507243f82e820d4b) )
ROM_END

/* 40 Fighting Ice Hockey */
ROM_START( cfghtice )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cfghtice.pro", 0x0000, 0x1000, CRC(5abd27b5) SHA1(2ab1c171adffd491759036d6ce2433706654aad2) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cfghtice.cas", 0x0000, 0x10000, CRC(906dd7fb) SHA1(894a7970d5476ed035edd15656e5cf10d6ddcf57) )
ROM_END

/* The Following use Dongle Type 4 (unknown part number?)
    (dongle data is used for most of the graphics) */

/* 38 Scrum Try */
ROM_START( cscrtry )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x08000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cscrtry.pro",  0x0000, 0x8000, CRC(7bc3460b) SHA1(7c5668ff9a5073e27f4a83b02d79892eb4df6b92) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cscrtry.cas",  0x0000, 0x8000, CRC(5625f0ca) SHA1(f4b0a6f2ca908880386838f06b626479b4b74134) )
ROM_END

ROM_START( cscrtry2 )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x08000, "dongle", 0 )   /* dongle data */
	ROM_LOAD( "cscrtry.pro",  0x0000, 0x8000, CRC(7bc3460b) SHA1(7c5668ff9a5073e27f4a83b02d79892eb4df6b92) )

	ROM_REGION( 0x10000, "cassette", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "cscrtry2.cas",  0x0000, 0x8000, CRC(04597842) SHA1(7f1fc3e06b61df880debe9056bdfbbb8600af739) )
ROM_END

/* 41 Oozumou - The Grand Sumo */
ROM_START( coozumou )
	DECOCASS_BIOS_A_ROMS

	ROM_REGION( 0x08000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-141_a.dgl",   0x0000, 0x8000,  CRC(bc379d2c) SHA1(bab19dcb6d68fdbd547ebab1598353f436321157) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "dt-141_1a.cas",  0x0000, 0x10000, CRC(20c2e86a) SHA1(a18248ba00b847a09df0bea7752a21162af8af76) )
ROM_END

/* 44 Boulder Dash */
ROM_START( cbdash )
	DECOCASS_BIOS_B_ROMS

/*  ROM_REGION( 0x01000, "dongle", 0 ) */ /* (max) 4k for dongle data */
	/* no proms */

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cbdash.cas",   0x0000, 0x8000, CRC(cba4c1af) SHA1(5d163d8e31c58b20679c6be06b1aa02df621822b) )
ROM_END

/* The Following have no Dongles at all */

/* 35 Flying Ball*/
ROM_START( cflyball )
	DECOCASS_BIOS_B_ROMS

	/* no dongle data */

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cflyball.cas",   0x0000, 0x10000, CRC(cb40d043) SHA1(57698bac7e0d552167efa99d08116bf19a3b29c9) )
ROM_END

ROM_START( decomult )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "widlbios.v0b",    0xf800, 0x0800, CRC(9ad7c451) SHA1(cda3513ca9904cd9f097a4a79226e3e30f83bb1c) )

	ROM_REGION( 0x100000, "dongle", 0 )
	ROM_LOAD( "widldeco.low",    0x00000, 0x80000, CRC(fd4dc36c) SHA1(1ef7f9e1dd333a1adc7b94e2b20eda41fe73a9f8) )
	ROM_LOAD( "widldeco.hgh",    0x80000, 0x80000, CRC(a8a30112) SHA1(b4feaa3e68c5d347c97958bc3c06472dd66df2f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "v1-.5a",     0xf800, 0x0800, CRC(b66b2c2a) SHA1(0097f38beb4872e735e560148052e258a26b08fd) )

	ROM_REGION( 0x00060, "proms", 0 )
	ROM_LOAD( "v2.3m",      0x0000, 0x0020, CRC(238fdb40) SHA1(b88e8fabb82092105c3828154608ea067acbf2e5) )
	ROM_LOAD( "v4.10d",     0x0020, 0x0020, CRC(3b5836b4) SHA1(b630bb277d9ec09d46ef26b944014dd6165b35d8) )
	ROM_LOAD( "v3.3j",      0x0040, 0x0020, CRC(51eef657) SHA1(eaedce5caf55624ad6ae706aedf82c5717c60f1f) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "cassmcu.1c", 0x0000, 0x0400, CRC(a6df18fd) SHA1(1f9ea47e372d31767c936c15852b43df2b0ee8ff) )
ROM_END


void decocass_state::init_decocass()
{
	/* Call the state save setup code in machine/decocass.cpp */
	decocass_machine_state_save_init();
	/* and in video/decocass.cpp, too */
	decocass_video_state_save_init();
}

void decocass_state::init_decocrom()
{
	/* standard init */
	init_decocass();

	/* convert charram to a banked ROM */
	membank("bank1")->configure_entry(0, m_charram);
	membank("bank1")->configure_entry(1, memregion("user3")->base());
	membank("bank1")->configure_entry(2, memregion("user3")->base()+0x5000);
	membank("bank1")->set_entry(0);
}

uint8_t decocass_state::cdsteljn_input_r(offs_t offset)
{
	uint8_t res;
	static const char *const portnames[2][4] = {
		{"P1_MP0", "P1_MP1", "P1_MP2", "P1_MP3"},
		{"P2_MP0", "P2_MP1", "P2_MP2", "P2_MP3"}         };

	if(offset & 6)
		return decocass_input_r(offset);

	res = ioport(portnames[offset & 1][m_mux_data])->read();

	return res;
}

void decocass_state::cdsteljn_mux_w(uint8_t data)
{
	m_mux_data = (data & 0xc) >> 2;
	/* bit 0 and 1 are p1/p2 lamps */

	if(data & ~0xf)
		printf("%02x\n",data);
}

void decocass_state::init_cdsteljn()
{
	/* standard init */
	init_decocass();

	/* install custom mahjong panel */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe413, 0xe413, write8smo_delegate(*this, FUNC(decocass_state::cdsteljn_mux_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xe600, 0xe6ff, read8sm_delegate(*this, FUNC(decocass_state::cdsteljn_input_r)));
}

/* -- */ GAME( 1981, decocass,  0,        decocass, decocass, decocass_state,        init_decocass, ROT270, "Data East Corporation", "DECO Cassette System", MACHINE_IS_BIOS_ROOT )
/* -- */ GAME( 1981, ctsttape,  decocass, ctsttape, decocass, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Test Tape (DECO Cassette) (US)", 0 )
/* 01 */ GAME( 1980, chwy,      decocass, chwy,     chwy,     decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Highway Chase (DECO Cassette) (US)", 0 )
/* 02 */ // 1980.12 Sengoku Ninjatai
/* 03 */ GAME( 1981, cmanhat,   decocass, cmanhat,  cmanhat,  decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Manhattan (DECO Cassette) (Japan)", MACHINE_IMPERFECT_GRAPHICS )
/* 04 */ GAME( 1981, cterrani,  decocass, cterrani, cterrani, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Terranean (DECO Cassette) (US)", 0 )
/* 05 */ // 1981.?? Missile Sprinter
/* 06 */ // 1980.12 Nebula
/* 07 */ GAME( 1981, castfant,  decocass, castfant, castfant, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Astro Fantasia (DECO Cassette) (US)", 0 )
/* 08 */ GAME( 1981, ctower,    decocass, cfboy0a1, ctower,   decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "The Tower (DECO Cassette) (Europe?)", 0 ) // 1981.03 The Tower (1981.02.04 in cassette data)
/* 09 */ GAME( 1981, csuperas,  decocass, csuperas, csuperas, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Super Astro Fighter (DECO Cassette) (US)", 0 )
/* 10 */ GAME( 1981, cocean1a,  decocass, cocean1a, cocean1a, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Ocean to Ocean (Medal) (DECO Cassette MD) (No.10/Ver.1,Japan)", 0 ) /* no lever, 1P/2P buttons used to switch player, cocktail mode not emulated */
/*    */ GAME( 1981, cocean6b,  cocean1a, cocean1a, cocean1a, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Ocean to Ocean (Medal) (DECO Cassette MD) (No.10/Ver.6,US)", 0 ) /* lever, 1P/2P buttons used to switch player, cocktail mode not emulated */
/* 11 */ GAME( 1981, clocknch,  decocass, clocknch, clocknch, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Lock'n'Chase (DECO Cassette) (US)", 0 )
/*    */ GAME( 1981, clocknchj, clocknch, clocknchj,clocknchj,decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Lock'n'Chase (DECO Cassette) (Japan)", 0 )
/* 12 */ GAME( 1981, cfboy0a1,  decocass, cfboy0a1, cfboy0a1, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Flash Boy (vertical) (DECO Cassette MD) (No.12/Ver.0/Set.1,Japan)", 0 )
/* 13 */ GAME( 1981, cprogolf,  decocass, cprogolf, cprogolf, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Tournament Pro Golf (DECO Cassette) (US)", 0 )
/*    */ GAME( 1981, cprogolfj, cprogolf, cprogolfj,cprogolf, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Tournament Pro Golf (DECO Cassette) (Japan)", 0 )
/* 14 */ GAME( 1981, cdsteljn,  decocass, cdsteljn, cdsteljn, decocass_type1_state,  init_cdsteljn, ROT270, "Data East Corporation", "DS Telejan (DECO Cassette) (Japan)", 0 )
/* 15 */ GAME( 1981, cluckypo,  decocass, cluckypo, cluckypo, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "Lucky Poker (DECO Cassette) (US)", 0 )
/* 16 */ GAME( 1981, ctisland,  decocass, ctisland, ctisland, decocass_type1_state,  init_decocrom, ROT270, "Data East Corporation", "Treasure Island (DECO Cassette) (US) (set 1)", 0 )
/*    */ GAME( 1981, ctisland2, ctisland, ctisland, ctisland, decocass_type1_state,  init_decocrom, ROT270, "Data East Corporation", "Treasure Island (DECO Cassette) (US) (set 2)", 0 ) /* newer? has instructions in attract */
/*    */ GAME( 1981, ctisland3, ctisland, ctisland3,ctisland, decocass_type1_state,  init_decocrom, ROT270, "Data East Corporation", "Treasure Island (DECO Cassette) (Europe?)", 0 )
/* 17 */ // 1981.10 Bobbitto
/* 18 */ GAME( 1982, cexplore,  decocass, cexplore, cexplore, decocass_type1_state,  init_decocrom, ROT270, "Data East Corporation", "Explorer (DECO Cassette) (US)", 0 )
/* 19 */ GAME( 1982, cdiscon1,  decocass, decocass, cdiscon1, decocass_type2_state,  init_decocass, ROT270, "Data East Corporation", "Disco No.1 (DECO Cassette) (US)", 0 )
/*    */ GAME( 1982, csweetht,  cdiscon1, decocass, csweetht, decocass_type2_state,  init_decocass, ROT270, "Data East Corporation", "Sweet Heart (DECO Cassette) (US)", 0 )
/* 20 */ GAME( 1982, ctornado,  decocass, decocass, ctornado, decocass_type2_state,  init_decocass, ROT270, "Data East Corporation", "Tornado (DECO Cassette) (US)", 0 )
/* 21 */ GAME( 1982, cmissnx,   decocass, decocass, cmissnx,  decocass_type2_state,  init_decocass, ROT270, "Data East Corporation", "Mission-X (DECO Cassette) (US)", 0 )
/* 22 */ GAME( 1982, cptennis,  decocass, decocass, cptennis, decocass_type2_state,  init_decocass, ROT270, "Data East Corporation", "Pro Tennis (DECO Cassette) (US)", 0 )
/*    */ GAME( 1982, cptennisj, cptennis, decocass, cptennis, decocass_type2_state,  init_decocass, ROT270, "Data East Corporation", "Pro Tennis (DECO Cassette) (Japan)", 0 )
/* 23 */ GAME( 1982, cprogolf18,cprogolf, cprogolfj,cprogolf, decocass_type1_state,  init_decocass, ROT270, "Data East Corporation", "18 Challenge Pro Golf (DECO Cassette) (Japan)", 0 ) // 1982.?? 18 Hole Pro Golf
/* 24 */ // 1982.07 Tsumego Kaisyou
/* 25 */ GAME( 1982, cadanglr,  decocass, cfishing, cfishing, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Angler Dangler (DECO Cassette) (US)", 0 )
/* 25 */ GAME( 1982, cfishing,  cadanglr, cfishing, cfishing, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Fishing (DECO Cassette) (Japan)", 0 )
/* 26 */ GAME( 1983, cbtime,    decocass, cbtime,   cbtime,   decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Burger Time (DECO Cassette) (US)", 0 )
/*    */ GAME( 1982, chamburger,cbtime,   cbtime,   cbtime,   decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Hamburger (DECO Cassette) (Japan)", 0 )
/* 27 */ GAME( 1982, cburnrub,  decocass, cburnrub, cburnrub, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Burnin' Rubber (DECO Cassette) (US) (set 1)", 0 ) /* large stylized red title (newer release?) */
/*    */ GAME( 1982, cburnrub2, cburnrub, cburnrub, cburnrub, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Burnin' Rubber (DECO Cassette) (US) (set 2)", 0 ) /* large monochrome title (original release?) */
/*    */ GAME( 1982, cburnrubj, cburnrub, cburnrub, cburnrub, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Burnin' Rubber (DECO Cassette) (Japan)", 0 ) /* large monochrome title (original release?) */
/*    */ GAME( 1982, cbnj,      cburnrub, cburnrub, cburnrub, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Bump 'n' Jump (DECO Cassette) (US)", 0 ) /* name was changed from Burnin' Rubber to Bump 'n' Jump (newest release?) */
/* 28 */ GAME( 1983, cgraplop,  decocass, cgraplop, cgraplop, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Cluster Buster (DECO Cassette) (US)", 0 )
/*    */ GAME( 1983, cgraplopj, cgraplop, cgraplop, cgraplop, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Graplop (DECO Cassette) (Japan)", 0 )
/*    */ GAME( 1983, cgraplop2, cgraplop, cgraplop2,cgraplop, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Graplop (DECO Cassette) (US) (Prototype?)", 0 ) /* button 1 does nothing, infinite shield despite the attract mode claiming otherwise, no title screen (was marked Cluster Buster in a previous MAME release?), repetitive level design, most likely a proto unless the encryption is still an issue (unlikely) */
/* 29 */ GAME( 1983, clapapa,   decocass, clapapa,  clapapa,  decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Rootin' Tootin' / La-Pa-Pa (DECO Cassette) (US)" , 0) /* Displays 'La-Pa-Pa during attract */
/*    */ GAME( 1983, clapapa2,  clapapa,  clapapa,  clapapa,  decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Rootin' Tootin' (DECO Cassette) (US)" , 0) /* Displays 'Rootin' Tootin' during attract */
/* 30 */ GAME( 1983, cskater,   decocass, cskater,  cskater,  decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Skater (DECO Cassette) (Japan)", 0 )
/* 31 */ GAME( 1983, cprobowl,  decocass, cprobowl, cprobowl, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Pro Bowling (DECO Cassette) (US)", 0 )
/* 32 */ GAME( 1983, cnightst,  decocass, cnightst, cnightst, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Night Star (DECO Cassette) (US) (set 1)", 0 )
/*    */ GAME( 1983, cnightst2, cnightst, cnightst, cnightst, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Night Star (DECO Cassette) (US) (set 2)", 0 )
/* 33 */ GAME( 1983, cpsoccer,  decocass, cpsoccer, cpsoccer, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Pro Soccer (DECO Cassette) (US)", 0 )
/*    */ GAME( 1983, cpsoccerj, cpsoccer, cpsoccer, cpsoccer, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Pro Soccer (DECO Cassette) (Japan)", 0 )
/* 34 */ GAME( 1983, csdtenis,  decocass, csdtenis, csdtenis, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Super Doubles Tennis (DECO Cassette) (Japan)", 0 )
/* 35 */ GAME( 1985, cflyball,  decocass, decocass, cflyball, decocass_nodong_state, init_decocass, ROT270, "Data East Corporation", "Flying Ball (DECO Cassette) (US)", 0 )
/* 36 */ // 1984.04 Genesis/Boomer Rang'r
/* 37 */ GAME( 1983, czeroize,  decocass, czeroize, czeroize, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Zeroize (DECO Cassette) (US)", 0 )
/* 38 */ GAME( 1984, cscrtry,   decocass, decocass, cscrtry,  decocass_type4_state,  init_decocass, ROT270, "Data East Corporation", "Scrum Try (DECO Cassette) (US) (set 1)", 0 )
/*    */ GAME( 1984, cscrtry2,  cscrtry,  decocass, cscrtry,  decocass_type4_state,  init_decocass, ROT270, "Data East Corporation", "Scrum Try (DECO Cassette) (US) (set 2)", 0 )
/* 39 */ GAME( 1984, cppicf,    decocass, cppicf,   cppicf,   decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Peter Pepper's Ice Cream Factory (DECO Cassette) (US) (set 1)", 0 )
/*    */ GAME( 1984, cppicf2,   cppicf,   cppicf,   cppicf,   decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Peter Pepper's Ice Cream Factory (DECO Cassette) (US) (set 2)", 0 )
/* 40 */ GAME( 1984, cfghtice,  decocass, cfghtice, cfghtice, decocass_type3_state,  init_decocass, ROT270, "Data East Corporation", "Fighting Ice Hockey (DECO Cassette) (US)", 0 )
/* 41 */ GAME( 1984, coozumou,  decocass, decocass, cscrtry,  decocass_type4_state,  init_decocass, ROT270, "Data East Corporation", "Oozumou - The Grand Sumo (DECO Cassette) (Japan)", 0 )
/* 42 */ // 1984.08 Hellow Gateball // not a typo, this is official spelling
/* 43 */ // 1984.08 Yellow Cab
/* 44 */ GAME( 1985, cbdash,    decocass, decocass, cbdash,   decocass_type5_state,  init_decocass, ROT270, "Data East Corporation", "Boulder Dash (DECO Cassette) (US)", 0 )

/* UX7 */ // 1984.12 Tokyo MIE Clinic/Tokyo MIE Shinryoujo
/* UX8 */ // 1985.01 Tokyo MIE Clinic/Tokyo MIE Shinryoujo Part 2
/* UX9 */ // 1985.05 Geinoujin Shikaku Shiken

/* xx */ GAME( 2008, decomult,  decocass, decocass, decocass, decocass_widel_state,  init_decocass, ROT270, "bootleg (David Widel)", "Deco Cassette System Multigame (ROM based)", 0 )
