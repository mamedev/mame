// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
/***********************************************************************

    DECO Cassette System driver
    by Juergen Buchmueller
    with contributions by:
    David Widel
    Nicola Salmoria
    Aaron Giles
    Brian Troha
    Fabio Priuli
    Lord Nightmare
    The Dumping Union
    Team Japump!!!
    Hau

    The DECO cassette system consists of three PCBS in a card cage:
    Early boardset: (1980-1983) (proms unknown for this boardset, no schematics for this boardset)
    One DE-0069C-0 RMS-3 pcb with a 6502 processor, D8041C MCU (DECO Cassette control), two ay-3-8910s, and one 2708 eprom holding the audio bios. (audio, needs external amp and volume control)
    One DE-0068B-0 DSP-3 pcb with a 'DECO CPU-3' custom, two 2716 eproms (main processor and bios, graphics, dipswitches?)
    One DE-0070C-0 BIO-3 pcb with an analog ADC0908 8-bit adc
    One DE-0066B-0 card rack board that the other three boards plug into.
    TODO: get more info about this older boardset: D. Widel has some info about it on his page at http://www.widel.com/stuff/decopin.htm

    Later boardset: (1984 onward, schematic is dated 10/83)
    One DE-0097C-0 RMS-8 pcb with a 6502 processor, two ay-3-8910s, two eproms (2716 and 2732) plus one prom, and 48k worth of 4116 16kx1 DRAMs; the 6502 processor has its own 4K of SRAM. (audio processor and RAM, Main processor's dram, dipswitches)
    One DE-0096C-0 DSP-8 board with a 'DECO 222' custom on it (labeled '8049 // C10707-2') which appears to really be a 'cleverly' disguised 6502, and two proms, plus 4K of sram, and three hm2511-1 1kx1 srams. (main processor and graphics)
    One DE-0098C-0 B10-8 (BIO-8 on schematics) board with an 8041, an analog devices ADC0908 8-bit adc, and 4K of SRAM on it (DECO Cassette control, inputs)
    One DE-0109C-0 card rack board that the other three boards plug into.

    The actual cassettes use a custom player hooked to the BIO board, and are roughly microcassette form factor, but are larger and will not fit in a conventional microcassette player.
    Each cassette has two tracks on it: a clock track and a data track, for a form of synchronous serial. The data is stored in blocks with headers and checksums.

 ***********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "includes/decocass.h"
#include "machine/decocass_tape.h"
#include "sound/ay8910.h"
#include "machine/deco222.h"

#define MASTER_CLOCK    XTAL_12MHz
#define HCLK            (MASTER_CLOCK/2)
#define HCLK1           (HCLK/2)
#define HCLK2           (HCLK1/2)
#define HCLK4           (HCLK2/2)


/***************************************************************************
 *
 *  swizzled mirror handlers
 *
 ***************************************************************************/

WRITE8_MEMBER(decocass_state::mirrorvideoram_w) { offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5); decocass_fgvideoram_w(space, offset, data, mem_mask); }
WRITE8_MEMBER(decocass_state::mirrorcolorram_w) { offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5); decocass_colorram_w(space, offset, data, mem_mask); }

READ8_MEMBER(decocass_state::mirrorvideoram_r)
{
	offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5);
	return m_fgvideoram[offset];
}

READ8_MEMBER(decocass_state::mirrorcolorram_r)
{
	offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5);
	return m_colorram[offset];
}


static ADDRESS_MAP_START( decocass_map, AS_PROGRAM, 8, decocass_state )
	AM_RANGE(0x0000, 0x5fff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x6000, 0xbfff) AM_RAM_WRITE(decocass_charram_w) AM_SHARE("charram") /* still RMS3 RAM */
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_WRITE(decocass_fgvideoram_w) AM_SHARE("fgvideoram")  /* DSP3 RAM */
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_WRITE(decocass_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xc800, 0xcbff) AM_READWRITE(mirrorvideoram_r, mirrorvideoram_w)
	AM_RANGE(0xcc00, 0xcfff) AM_READWRITE(mirrorcolorram_r, mirrorcolorram_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(decocass_tileram_w) AM_SHARE("tileram")
	AM_RANGE(0xd800, 0xdbff) AM_RAM_WRITE(decocass_objectram_w) AM_SHARE("objectram")
	AM_RANGE(0xe000, 0xe0ff) AM_RAM_WRITE(decocass_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xe300, 0xe300) AM_READ_PORT("DSW1") AM_WRITE(decocass_watchdog_count_w)
	AM_RANGE(0xe301, 0xe301) AM_READ_PORT("DSW2") AM_WRITE(decocass_watchdog_flip_w)
	AM_RANGE(0xe302, 0xe302) AM_WRITE(decocass_color_missiles_w)
	AM_RANGE(0xe400, 0xe400) AM_WRITE(decocass_reset_w)

/* BIO-3 board */
	AM_RANGE(0xe402, 0xe402) AM_WRITE(decocass_mode_set_w)      /* scroll mode regs + various enable regs */
	AM_RANGE(0xe403, 0xe403) AM_WRITE(decocass_back_h_shift_w)  /* back (both)  tilemap x scroll */
	AM_RANGE(0xe404, 0xe404) AM_WRITE(decocass_back_vl_shift_w) /* back (left)  (top@rot0) tilemap y scroll */
	AM_RANGE(0xe405, 0xe405) AM_WRITE(decocass_back_vr_shift_w) /* back (right) (bot@rot0) tilemap y scroll */
	AM_RANGE(0xe406, 0xe406) AM_WRITE(decocass_part_h_shift_w) /* headlight */
	AM_RANGE(0xe407, 0xe407) AM_WRITE(decocass_part_v_shift_w) /* headlight */

	AM_RANGE(0xe410, 0xe410) AM_WRITE(decocass_color_center_bot_w)
	AM_RANGE(0xe411, 0xe411) AM_WRITE(decocass_center_h_shift_space_w)
	AM_RANGE(0xe412, 0xe412) AM_WRITE(decocass_center_v_shift_w)
	AM_RANGE(0xe413, 0xe413) AM_WRITE(decocass_coin_counter_w)
	AM_RANGE(0xe414, 0xe414) AM_READWRITE(decocass_sound_command_main_r, decocass_sound_command_w)
	AM_RANGE(0xe415, 0xe416) AM_WRITE(decocass_quadrature_decoder_reset_w)
	AM_RANGE(0xe417, 0xe417) AM_WRITE(decocass_nmi_reset_w)
	AM_RANGE(0xe420, 0xe42f) AM_WRITE(decocass_adc_w)

	AM_RANGE(0xe500, 0xe5ff) AM_READWRITE(decocass_e5xx_r, decocass_e5xx_w) /* read data from 8041/status */

	AM_RANGE(0xe600, 0xe6ff) AM_READ(decocass_input_r)      /* inputs */
	AM_RANGE(0xe700, 0xe700) AM_READ(decocass_sound_data_r) /* read sound CPU data */
	AM_RANGE(0xe701, 0xe701) AM_READ(decocass_sound_ack_r)  /* read sound CPU ack status */

	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decocass_sound_map, AS_PROGRAM, 8, decocass_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x17ff) AM_READWRITE(decocass_sound_nmi_enable_r, decocass_sound_nmi_enable_w)
	AM_RANGE(0x1800, 0x1fff) AM_READWRITE(decocass_sound_data_ack_reset_r, decocass_sound_data_ack_reset_w)
	AM_RANGE(0x2000, 0x2fff) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x4000, 0x4fff) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x6fff) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x8000, 0x8fff) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0xa000, 0xafff) AM_READ(decocass_sound_command_r)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(decocass_sound_data_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( decocass_mcu_portmap, AS_IO, 8, decocass_state )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(i8041_p1_r, i8041_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(i8041_p2_r, i8041_p2_w)
ADDRESS_MAP_END

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

static INPUT_PORTS_START( clocknch )
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

static const UINT32 objlayout_xoffset[64] =
{
	STEP8(7*8,1), STEP8(6*8,1), STEP8(5*8,1), STEP8(4*8,1),
	STEP8(3*8,1), STEP8(2*8,1), STEP8(1*8,1), STEP8(0*8,1)
};

static const UINT32 objlayout_yoffset[64] =
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

static GFXDECODE_START( decocass )
	GFXDECODE_ENTRY( 0x0000, 0x6000, charlayout,       0, 4 )  /* char set #1 */
	GFXDECODE_ENTRY( 0x0000, 0x6000, spritelayout,     0, 4 )  /* sprites */
	GFXDECODE_ENTRY( 0x0000, 0xd000, tilelayout,      32, 2 )  /* background tiles */
	GFXDECODE_ENTRY( 0x0000, 0xd800, objlayout,       48, 4 )  /* object */
GFXDECODE_END

PALETTE_INIT_MEMBER(decocass_state, decocass)
{
	int i;

	/* set up 32 colors 1:1 pens */
	for (i = 0; i < 32; i++)
		palette.set_pen_indirect(i, i);

	/* setup straight/flipped colors for background tiles (D7 of color_center_bot ?) */
	for (i = 0; i < 8; i++)
	{
		palette.set_pen_indirect(32+i, 3*8+i);
		palette.set_pen_indirect(40+i, 3*8+((i << 1) & 0x04) + ((i >> 1) & 0x02) + (i & 0x01));
	}

	/* setup 4 colors for 1bpp object */
	palette.set_pen_indirect(48+0*2+0, 0);
	palette.set_pen_indirect(48+0*2+1, 25); /* testtape red from 4th palette section? */
	palette.set_pen_indirect(48+1*2+0, 0);
	palette.set_pen_indirect(48+1*2+1, 28); /* testtape blue from 4th palette section? */
	palette.set_pen_indirect(48+2*2+0, 0);
	palette.set_pen_indirect(48+2*2+1, 26); /* testtape green from 4th palette section? */
	palette.set_pen_indirect(48+3*2+0, 0);
	palette.set_pen_indirect(48+3*2+1, 23); /* ???? */
}


static MACHINE_CONFIG_START( decocass, decocass_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", DECO_222, HCLK4) /* the earlier revision board doesn't have the 222 but must have the same thing implemented in logic for the M6502 */
	MCFG_CPU_PROGRAM_MAP(decocass_map)

	MCFG_CPU_ADD("audiocpu", M6502, HCLK1/3/2)
	MCFG_CPU_PROGRAM_MAP(decocass_sound_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("audionmi", decocass_state, decocass_audio_nmi_gen, "screen", 0, 8)

	MCFG_CPU_ADD("mcu", I8041, HCLK)
	MCFG_CPU_IO_MAP(decocass_mcu_portmap)

	MCFG_QUANTUM_TIME(attotime::from_hz(4200))              /* interleave CPUs */


	MCFG_DECOCASS_TAPE_ADD("cassette")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(HCLK, 384, 0*8, 256, 272, 1*8, 248)
	MCFG_SCREEN_UPDATE_DRIVER(decocass_state, screen_update_decocass)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", decocass)
	MCFG_PALETTE_ADD("palette", 32+2*8+2*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(decocass_state, decocass)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, HCLK2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_SOUND_ADD("ay2", AY8910, HCLK2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ctsttape, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,ctsttape)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cprogolfj, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cprogolfj)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cdsteljn, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cdsteljn)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cmanhat, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cmanhat)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cfishing, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cfishing)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( chwy, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,chwy)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cterrani, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cterrani)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( castfant, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,castfant)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( csuperas, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,csuperas)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( clocknch, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,clocknch)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cprogolf, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cprogolf)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cluckypo, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cluckypo)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ctisland, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,ctisland)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cexplore, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cexplore)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cdiscon1, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cdiscon1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ctornado, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,ctornado)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cmissnx, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cmissnx)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cptennis, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cptennis)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cbtime, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cbtime)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cburnrub, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cburnrub)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cgraplop, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cgraplop)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cgraplop2, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cgraplop2)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( clapapa, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,clapapa)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cskater, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cskater)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cprobowl, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cprobowl)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cnightst, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cnightst)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cpsoccer, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cpsoccer)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( csdtenis, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,csdtenis)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( czeroize, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,czeroize)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cppicf, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cppicf)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cfghtice, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cfghtice)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( type4, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,type4)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cbdash, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cbdash)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cflyball, decocass )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(decocass_state,cflyball)
MACHINE_CONFIG_END


#define DECOCASS_COMMON_ROMS    \
	ROM_REGION( 0x10000, "audiocpu", 0 )      \
	ROM_LOAD( "v1-.5a",     0xf800, 0x0800, CRC(b66b2c2a) SHA1(0097f38beb4872e735e560148052e258a26b08fd) ) /* from RMS-8 board: 2716 eprom @5A w/V1- label,  contains audio cpu code */ \
\
	ROM_REGION( 0x10000, "mcu", 0 )   /* 4k for the 8041 MCU (actually 1K ROM + 64 bytes RAM @ 0x800) */ \
	ROM_LOAD( "cassmcu.1c", 0x0000, 0x0400, CRC(a6df18fd) SHA1(1f9ea47e372d31767c936c15852b43df2b0ee8ff) ) /* from B10-B board: "NEC // JAPAN // X1202D-108 // D8041C 535" 8041 MCU @1C, handles cassette and other stuff; This info needs additional verification, as the d8041-535 mcu has not been dumped yet to prove code is the same. */ \
\
	ROM_REGION( 0x00060, "proms", 0 )     /* PROMS */ \
	ROM_LOAD( "v2.3m",      0x0000, 0x0020, CRC(238fdb40) SHA1(b88e8fabb82092105c3828154608ea067acbf2e5) ) /* from DSP-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @3M w/'V2' stamp, unknown purpose (gfx related: row/interrupt/vblank related? vertical counter related) */ \
	ROM_LOAD( "v4.10d",     0x0020, 0x0020, CRC(3b5836b4) SHA1(b630bb277d9ec09d46ef26b944014dd6165b35d8) ) /* from DSP-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @10D w/'V4' stamp, unknown purpose (gfx related: tile banking? horizontal counter related) */ \
	ROM_LOAD( "v3.3j",      0x0040, 0x0020, CRC(51eef657) SHA1(eaedce5caf55624ad6ae706aedf82c5717c60f1f) ) /* from RMS-8 board: M3-7603-5 (82s123 equiv, 32x8 TS) PROM @3J w/'V3' stamp, handles DRAM banking and timing */

#define DECOCASS_BIOS_A_ROMS    \
	/* v0a.7e, New boardset bios, revision A */ \
\
	ROM_REGION( 0x10000, "maincpu", 0 ) \
	ROM_LOAD( "v0a-.7e",    0xf000, 0x1000, CRC(3d33ac34) SHA1(909d59e7a993affd10224402b4370e82a5f5545c) ) /* from RMS-8 board: 2732 EPROM @7E w/'V0A-' label (has HDRA01HDR string inside it), bios code */ \
\
	DECOCASS_COMMON_ROMS

#define DECOCASS_BIOS_B_ROMS    \
	/* rms8.7e, New boardset bios, revision B */ \
\
	ROM_REGION( 0x10000, "maincpu", 0 ) \
	ROM_LOAD( "v0b-.7e",    0xf000, 0x1000, CRC(23d929b7) SHA1(063f83020ba3d6f43ab8471f95ca919767b93aa4) ) /* from RMS-8 board: 2732 EPROM @7E w/'V0B-' label (has HDRB01HDR string inside it), bios code */ \
\
	DECOCASS_COMMON_ROMS

#define DECOCASS_BIOS_B2_ROMS   \
	/* dsp3.p0b/p1b, Old boardset bios, revision B?; from DSP-3 board? has HDRB01x string in it, 2x 2716 EPROM? */ \
\
	ROM_REGION( 0x10000, "maincpu", 0 ) \
	ROM_LOAD( "dsp3.p0b",   0xf000, 0x0800, CRC(b67a91d9) SHA1(681c040be0f0ed1ba0a50161b36d0ad8e1c8c5cb) ) \
	ROM_LOAD( "dsp3.p1b",   0xf800, 0x0800, CRC(3bfff5f3) SHA1(4e9437cb1b76d64da6b37f01bd6e879fb399e8ce) ) \
\
	DECOCASS_COMMON_ROMS

ROM_START( decocass )
	DECOCASS_BIOS_B_ROMS

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

/* 09 Super Astro Fighter */
ROM_START( csuperas )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "csuperas.cas", 0x0000, 0x8000, CRC(fabcd07f) SHA1(4070c668ad6725f0710cf7fe6df0d5f80272a449) )
ROM_END

/* 11 Lock'n'Chase */
ROM_START( clocknch )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "dp-1110_b.dgl", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "clocknch.cas",  0x0000, 0x8000, CRC(c9d163a4) SHA1(3ef55a8d8f603059e263776c08eb81f2cf18b75c) )
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

	ROM_REGION( 0x4000, "user3", 0 )      /* roms from the overlay pcb */
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

	ROM_REGION( 0x4000, "user3", 0 )      /* roms from the overlay pcb */
	ROM_LOAD( "deco-ti.x1",   0x0000, 0x1000, CRC(a7f8aeba) SHA1(0c9ba1a46d0636b36f40fad31638db89f374f778) )
	ROM_LOAD( "deco-ti.x2",   0x1000, 0x1000, CRC(2a0d3c91) SHA1(552d08fcddddbea5b52fa1e8decd188ae49c86ea) )
	ROM_LOAD( "deco-ti.x3",   0x2000, 0x1000, CRC(3a26b97c) SHA1(f57e76077806e149a9e455c85e5431eac2d42bc3) )
	ROM_LOAD( "deco-ti.x4",   0x3000, 0x1000, CRC(1cbe43de) SHA1(8f26ad224e96c87da810c60d3dd88d415400b9fc) )
ROM_END

ROM_START( ctisland3 )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x00020, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "ctislnd3.cas", 0x0000, 0x8000, CRC(45464e1e) SHA1(03275694d963c7ab0e0f5525e248e69da5f9b591) )

	ROM_REGION( 0x4000, "user3", 0 )      /* roms from the overlay pcb */
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

	ROM_REGION( 0x4000, "user3", 0 )      /* roms from the overlay pcb */
	ROM_LOAD( "cexplore_overlay_roms", 0x0000, 0x4000, NO_DUMP )
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

/* 28 Graplop / Cluster Buster */
ROM_START( cgraplop )
	DECOCASS_BIOS_B_ROMS

	ROM_REGION( 0x01000, "dongle", 0 )    /* dongle data */
	ROM_LOAD( "cgraplop.pro", 0x0000, 0x1000, CRC(ee93787d) SHA1(0c753d62fdce2fdbd5b329a5aa259a967d07a651) )

	ROM_REGION( 0x10000, "cassette", 0 )      /* (max) 64k for cassette image */
	ROM_LOAD( "cgraplop.cas", 0x0000, 0x8000, CRC(d2c1c1bb) SHA1(db67304caa11540363735e7d4bf03507ccbe9980) )
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


DRIVER_INIT_MEMBER(decocass_state,decocass)
{
	/* Call the state save setup code in machine/decocass.c */
	decocass_machine_state_save_init();
	/* and in video/decocass.c, too */
	decocass_video_state_save_init();
}

DRIVER_INIT_MEMBER(decocass_state,decocrom)
{
	/* standard init */
	DRIVER_INIT_CALL(decocass);

	/* convert charram to a banked ROM */
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x6000, 0xafff, "bank1");
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x6000, 0xafff, write8_delegate(FUNC(decocass_state::decocass_de0091_w),this));
	membank("bank1")->configure_entry(0, m_charram);
	membank("bank1")->configure_entry(1, memregion("user3")->base());
	membank("bank1")->set_entry(0);

	/* install the bank selector */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe900, 0xe900, write8_delegate(FUNC(decocass_state::decocass_e900_w),this));
}

READ8_MEMBER(decocass_state::cdsteljn_input_r )
{
	UINT8 res;
	static const char *const portnames[2][4] = {
		{"P1_MP0", "P1_MP1", "P1_MP2", "P1_MP3"},
		{"P2_MP0", "P2_MP1", "P2_MP2", "P2_MP3"}         };

	if(offset & 6)
		return decocass_input_r(space,offset);

	res = ioport(portnames[offset & 1][m_mux_data])->read();

	return res;
}

WRITE8_MEMBER(decocass_state::cdsteljn_mux_w )
{
	m_mux_data = (data & 0xc) >> 2;
	/* bit 0 and 1 are p1/p2 lamps */

	if(data & ~0xf)
		printf("%02x\n",data);
}

DRIVER_INIT_MEMBER(decocass_state,cdsteljn)
{
	/* standard init */
	DRIVER_INIT_CALL(decocass);

	/* install custom mahjong panel */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe413, 0xe413, write8_delegate(FUNC(decocass_state::cdsteljn_mux_w), this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xe600, 0xe6ff, read8_delegate(FUNC(decocass_state::cdsteljn_input_r), this));
}

/* -- */ GAME( 1981, decocass,  0,        decocass, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "DECO Cassette System", MACHINE_IS_BIOS_ROOT )
/* -- */ GAME( 1981, ctsttape,  decocass, ctsttape, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Test Tape (DECO Cassette)", 0 )
/* 01 */ GAME( 1980, chwy,      decocass, chwy,     decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Highway Chase (DECO Cassette)", 0 )
/* 02 */ // 1980.12 Sengoku Ninjatai
/* 03 */ GAME( 1981, cmanhat,   decocass, cmanhat,  decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Manhattan (DECO Cassette)", MACHINE_IMPERFECT_GRAPHICS )
/* 04 */ GAME( 1981, cterrani,  decocass, cterrani, cterrani, decocass_state, decocass, ROT270, "Data East Corporation", "Terranean (DECO Cassette)", 0 )
/* 05 */ // 1981.?? Missile Sprinter
/* 06 */ // 1980.12 Nebula
/* 07 */ GAME( 1981, castfant,  decocass, castfant, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Astro Fantasia (DECO Cassette)", 0 )
/* 08 */ // 1981.03 The Tower
/* 09 */ GAME( 1981, csuperas,  decocass, csuperas, csuperas, decocass_state, decocass, ROT270, "Data East Corporation", "Super Astro Fighter (DECO Cassette)", 0 )
/* 10 */ // 1981.?? Ocean to Ocean (medal)
/* 11 */ GAME( 1981, clocknch,  decocass, clocknch, clocknch, decocass_state, decocass, ROT270, "Data East Corporation", "Lock'n'Chase (DECO Cassette)", 0 )
/* 12 */ // 1981.08 Flash Boy/DECO Kid
/* 13 */ GAME( 1981, cprogolf,  decocass, cprogolf, cprogolf, decocass_state, decocass, ROT270, "Data East Corporation", "Tournament Pro Golf (DECO Cassette)", 0 )
/*    */ GAME( 1981, cprogolfj, cprogolf, cprogolfj,cprogolf, decocass_state, decocass, ROT270, "Data East Corporation", "Tournament Pro Golf (DECO Cassette, Japan)", 0 )
/* 14 */ GAME( 1981, cdsteljn,  decocass, cdsteljn, cdsteljn, decocass_state, cdsteljn, ROT270, "Data East Corporation", "DS Telejan (DECO Cassette, Japan)", 0 )
/* 15 */ GAME( 1981, cluckypo,  decocass, cluckypo, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Lucky Poker (DECO Cassette)", 0 )
/* 16 */ GAME( 1981, ctisland,  decocass, ctisland, decocass, decocass_state, decocrom, ROT270, "Data East Corporation", "Treasure Island (DECO Cassette, set 1)", 0 )
/*    */ GAME( 1981, ctisland2, ctisland, ctisland, decocass, decocass_state, decocrom, ROT270, "Data East Corporation", "Treasure Island (DECO Cassette, set 2)", 0 )
/*    */ GAME( 1981, ctisland3, ctisland, ctisland, decocass, decocass_state, decocrom, ROT270, "Data East Corporation", "Treasure Island (DECO Cassette, set 3)", MACHINE_NOT_WORKING ) /* Different Bitswap? */
/* 17 */ // 1981.10 Bobbitto
/* 18 */ GAME( 1982, cexplore,  decocass, cexplore, cexplore, decocass_state, decocass, ROT270, "Data East Corporation", "Explorer (DECO Cassette)", MACHINE_NOT_WORKING )
/* 19 */ GAME( 1982, cdiscon1,  decocass, cdiscon1, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Disco No.1 (DECO Cassette)", 0 )
/*    */ GAME( 1982, csweetht,  cdiscon1, cdiscon1, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Sweet Heart (DECO Cassette)", 0 )
/* 20 */ GAME( 1982, ctornado,  decocass, ctornado, ctornado, decocass_state, decocass, ROT270, "Data East Corporation", "Tornado (DECO Cassette)", 0 )
/* 21 */ GAME( 1982, cmissnx,   decocass, cmissnx,  cmissnx,  decocass_state, decocass, ROT270, "Data East Corporation", "Mission-X (DECO Cassette)", 0 )
/* 22 */ GAME( 1982, cptennis,  decocass, cptennis, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Pro Tennis (DECO Cassette)", 0 )
/* 23 */ GAME( 1982, cprogolf18,cprogolf, cprogolfj,cprogolf, decocass_state, decocass, ROT270, "Data East Corporation", "18 Challenge Pro Golf (DECO Cassette, Japan)", 0 ) // 1982.?? 18 Hole Pro Golf
/* 24 */ // 1982.07 Tsumego Kaisyou
/* 25 */ GAME( 1982, cadanglr,  decocass, cfishing, cfishing, decocass_state, decocass, ROT270, "Data East Corporation", "Angler Dangler (DECO Cassette)", 0 )
/* 25 */ GAME( 1982, cfishing,  cadanglr, cfishing, cfishing, decocass_state, decocass, ROT270, "Data East Corporation", "Fishing (DECO Cassette, Japan)", 0 )
/* 26 */ GAME( 1983, cbtime,    decocass, cbtime,   cbtime,   decocass_state, decocass, ROT270, "Data East Corporation", "Burger Time (DECO Cassette)", 0 )
/*    */ GAME( 1982, chamburger,cbtime,   cbtime,   cbtime,   decocass_state, decocass, ROT270, "Data East Corporation", "Hamburger (DECO Cassette, Japan)", 0 )
/* 27 */ GAME( 1982, cburnrub,  decocass, cburnrub, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Burnin' Rubber (DECO Cassette, set 1)", 0 )
/*    */ GAME( 1982, cburnrub2, cburnrub, cburnrub, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Burnin' Rubber (DECO Cassette, set 2)", 0 )
/*    */ GAME( 1982, cbnj,      cburnrub, cburnrub, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Bump 'n' Jump (DECO Cassette, Japan)", 0 )
/* 28 */ GAME( 1983, cgraplop,  decocass, cgraplop, cgraplop, decocass_state, decocass, ROT270, "Data East Corporation", "Cluster Buster (DECO Cassette)", 0 )
/*    */ GAME( 1983, cgraplop2, cgraplop, cgraplop2,cgraplop, decocass_state, decocass, ROT270, "Data East Corporation", "Graplop (no title screen) (DECO Cassette)", 0 ) // a version with title screen exists, see reference videos
/* 29 */ GAME( 1983, clapapa,   decocass, clapapa,  decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Rootin' Tootin' / La-Pa-Pa (DECO Cassette)" , 0) /* Displays 'La-Pa-Pa during attract */
/*    */ GAME( 1983, clapapa2,  clapapa,  clapapa,  decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Rootin' Tootin' (DECO Cassette)" , 0) /* Displays 'Rootin' Tootin' during attract */
/* 30 */ GAME( 1983, cskater,   decocass, cskater,  cskater,  decocass_state, decocass, ROT270, "Data East Corporation", "Skater (DECO Cassette, Japan)", 0 )
/* 31 */ GAME( 1983, cprobowl,  decocass, cprobowl, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Pro Bowling (DECO Cassette)", 0 )
/* 32 */ GAME( 1983, cnightst,  decocass, cnightst, cnightst, decocass_state, decocass, ROT270, "Data East Corporation", "Night Star (DECO Cassette, set 1)", 0 )
/*    */ GAME( 1983, cnightst2, cnightst, cnightst, cnightst, decocass_state, decocass, ROT270, "Data East Corporation", "Night Star (DECO Cassette, set 2)", 0 )
/* 33 */ GAME( 1983, cpsoccer,  decocass, cpsoccer, cpsoccer, decocass_state, decocass, ROT270, "Data East Corporation", "Pro Soccer (DECO Cassette)", 0 )
/*    */ GAME( 1983, cpsoccerj, cpsoccer, cpsoccer, cpsoccer, decocass_state, decocass, ROT270, "Data East Corporation", "Pro Soccer (DECO Cassette, Japan)", 0 )
/* 34 */ GAME( 1983, csdtenis,  decocass, csdtenis, csdtenis, decocass_state, decocass, ROT270, "Data East Corporation", "Super Doubles Tennis (DECO Cassette, Japan)", MACHINE_WRONG_COLORS )
/* 35 */ GAME( 1985, cflyball,  decocass, cflyball, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Flying Ball (DECO Cassette)", 0 )
/* 36 */ // 1984.04 Genesis/Boomer Rang'r
/* 37 */ GAME( 1983, czeroize,  decocass, czeroize, decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Zeroize (DECO Cassette)", 0 )
/* 38 */ GAME( 1984, cscrtry,   decocass, type4,    cscrtry,  decocass_state, decocass, ROT270, "Data East Corporation", "Scrum Try (DECO Cassette, set 1)", 0 )
/*    */ GAME( 1984, cscrtry2,  cscrtry,  type4,    cscrtry,  decocass_state, decocass, ROT270, "Data East Corporation", "Scrum Try (DECO Cassette, set 2)", 0 )
/* 39 */ GAME( 1984, cppicf,    decocass, cppicf,   decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Peter Pepper's Ice Cream Factory (DECO Cassette, set 1)", 0 )
/*    */ GAME( 1984, cppicf2,   cppicf,   cppicf,   decocass, decocass_state, decocass, ROT270, "Data East Corporation", "Peter Pepper's Ice Cream Factory (DECO Cassette, set 2)", 0 )
/* 40 */ GAME( 1984, cfghtice,  decocass, cfghtice, cfghtice, decocass_state, decocass, ROT270, "Data East Corporation", "Fighting Ice Hockey (DECO Cassette)", 0 )
/* 41 */ GAME( 1984, coozumou,  decocass, type4,    cscrtry,  decocass_state, decocass, ROT270, "Data East Corporation", "Oozumou - The Grand Sumo (DECO Cassette, Japan)", 0 )
/* 42 */ // 1984.08 Hellow Gateball // not a typo, this is official spelling
/* 43 */ // 1984.08 Yellow Cab
/* 44 */ GAME( 1985, cbdash,    decocass, cbdash,   cbdash,   decocass_state, decocass, ROT270, "Data East Corporation", "Boulder Dash (DECO Cassette)", 0 )

/* UX7 */ // 1984.12 Tokyo MIE Clinic/Tokyo MIE Shinryoujo
/* UX8 */ // 1985.01 Tokyo MIE Clinic/Tokyo MIE Shinryoujo Part 2
/* UX9 */ // 1985.05 Geinoujin Shikaku Shiken
