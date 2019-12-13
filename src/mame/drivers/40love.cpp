// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/****************************************************************************

    Forty-Love (c) Taito 1984

    driver by Jarek Burczynski

****************************************************************************/

/*
    TO DO:
    - sprites graphics decoding could be changed to only use
      color codes 8-15 (now it decodes all 64 colors).

    - sprite memory needs to be buffered ?

    - inputs labels for Field Day, also fix the p3 / p4 controls (actually routes to p1 / p2)

    - pixel layer needs priority ?
*/

/*
Game                     : 40-0  (Forty-Love Tennis game)
Manufacturer             : Taito
Game number              : A30
Vintage                  : 1984
Game serial/model number : M4300006B ?

I don't have the wiring harness for this board, so don't know if it works.
One GFX ROM is bad though.
See A30-26.u23\A30-26.txt for details about the bad ROM.
To summarise:
  Dumps from GFX ROM A30-26.u23 were inconsistent. Reads with checksums
  41A3 and 415F occurred a couple of times, and the difference is one byte
  at offset $0004 (data $CC or $88). Maybe one of these reads is correct
  or closest to the real ROM. We are using the one with checksum 415F,
  the other one makes one sprite looks worse.

This is a four board system - Main, Video, ROM, and Sound boards.

Details:
  Main board:    J1100004A / K1100010A <--label with K1100026A covers these
    1 x NEC D780C-1 (Z80) CPU
    1 x Motorola M68705P5S MCU
    1 x Mitsubishi M5517P SRAM chip        1 x (8bit x 2048) used
    1 x Fujitsu MB14241 ??? chip           ( DAC ????? )
    4 x Fujitsu MB81416-10 DRAM chips      4 x (4bit x 16384) 1/2 used
    1 x TD62003P (lamps/LEDs driver)
    1 x 8MHz xtal
    3 x 8way DSW

  Sound board:   J1100005A / K1100011A
    1 x NEC D780C-1 (Z80) CPU
    1 x Mitsubishi M5517P SRAM chip        1 x (8bit x 2048) used
    1 x Yamaha YM2149
    1 x OKI M5232
    1 x Fujitsu MB3731 Audio amp
    1 x 8MHz xtal

  Video board:   J1100008A / K1100025A
    4 x AMD AM93422 RAM chips              4 x (4bit x 256)     1/2 used
    2 x Mitsubishi M5517P SRAM chips       2 x (8bit x 2048)    1/2 used
    6 x Mitsubishi M53357P (=LS157)
    1 x 18.432MHz xtal

  ROM board:     J9100005A / K9100008A


ROMS:       Programmer   Device           Legend:
             Checksum                ___
A30-08.u08     AD5C       2764          \
A30-09.u09     C1E4       2764           \
A30-10.u10     C6B1       2764            \ Sound Board ROMs
A30-11.u37     ACD8       2764            /
A30-12.u38     B7C4       2764           /
A30-13.u39     6B43       2764       ___/
A30-14.u41                M68705P5S  ___> Main board MCU
A30-15.u03     2F6E       6353          \
A30-16.u01     2E97       6353           \ Video board BPROMs
A30-17.u02     2CD3       6353           / All read as 82S137
A30-18.u13     15E5       7643       ___/
A30-19.ic1     C88C       2764          \
A30-20.ic2     7B40       2764           \  ROM board ROMs.
A30-21.ic3     E2B4       2764            \ These are Program ROMs for
A30-22.ic4     8937       2764            / the main board
A30-23.ic5     A6F6       2764           /
A30-24.ic6     75DC       2764       ___/
A30-25.u22     1903       2764          \
A30-26.u23      ??        2764  BAD !    \
A30-27.u24     005B       2764            \
A30-28.u25     279F       2764             \ Video board GFX ROMs
A30-29.u62     BAA4       2764             /
A30-30.u63     0BB6       2764            /
A30-31.u64     461C       2764           /
A30-32.u65     E764       2764       ___/

Notes,
Programmer used: HiLo All-08A programmer.

Q and P connectors, provide connection between the main and video board,
via ribbon cables.
The following are board layouts, and component locations.

     Main-Board    J1100004A / K1100010A <--label with K1100026A covers these
 +--------------------------------------------------+
 |                           DSW1 DSW2 DSW3         |
 |  5517       Z80                                  |
 =                                                  |
 =  ROMskt                                          |
 P                                                  ==
 =  ROMskt     68705P5 (A30-14)                     ==
 =                                                  ==  Wiring harness
 |  ROMskt     MB14241            TD62003           ==  connector
 |                                                  ==
 =                                                  ==
 =                                                  ==
 Q       81416                 Ribbon    =          |
 =       81416                 cable     =          ===  Wiring harness
 =       81416                 to sound  S          ===  connector
 |       81416                 board --> =          ===
 | 8MHz                                  =          |
 +--------------------------------------------------+

     ROM board     J9100005A / K9100008A

 +------------------------+
 |                        |
 | ROMskt  A30-23  A30-24 |
 |                        |
 | ROMskt  A30-21  A30-22 |
 |                        |
 | ROMskt  A30-19  A30-20 |
 +------------------------+


     Video Board   J1100008A / K1100025A
 +--------------------------------------------------+
 |          A30-32                         A30-15   |
 |                           A30-28 A30-18 A30-17   |
 =          A30-31                         A30-16   |
 =                           A30-27                 |
 Q          A30-30                                  |
 =                           A30-26                 ==
 =18.432MHz A30-29                                  ==
 |     53357      53357      A30-25                 ==  Wiring harness
 |                                                  ==  connector
 =     53357  5517                                  ==
 =     53357                           53357        ==
 P     53357                     93422              |
 =                               93422              |
 =                               93422              |
 |            5517               93422              |
 |                                                  |
 +--------------------------------------------------+

     Sound Board   J1100005A / K1100011A
     (The following representation is rotated 90 degrees anti-clockwise)
                               _______  Ribbon cable
 +------------------------+   \|/       to main board
 | A30-13  A30-14  A30-15 +----------+
 |                          |||S|||  |
 |                  Z80              |
 | YM2149                     MB3731 |
 |                  A30-08           |
 |                  A30-09           |
 | M5232            A30-10           |
 |                  5517             |
 |                                   |
 |                                   |
 | 8MHz                              |
 +-----------------------------------+

     Side view

       --Sound board-------------------
    -----ROM board---
 --------Main board--------------------------------
 --------Video board-------------------------------

Details by Quench

*************************************************************

FieldDay by Taito

Same board as 40-Love

M4300048A

18.432 mhz crystal
2x m5m5517
4x am93422 (2101)

A17-15->18 6353 1024x4 prom

M4300049A (relabeled J1100004A/K1100010A)

8 mhz crystal
4x 4416
1x m5m5517
z80c

a17_14 protection processor. 28 Pin Motorola 15-00011-01 DA68235 (Labeled 8909)
Next to MB14241

Rom Daughterboard

K9100013A (relabeled J9100005A/K9100008A)

Sound Board

K1100024A (relabeled J1100005A/K1100011A)

8 Mhz crystal
DZ80C
YM2149
Oki M5232 (6532?)

Notes - Has jumper setting for 122HZ or 61HZ)

*/

#include "emu.h"
#include "includes/40love.h"

#include "cpu/m6805/m6805.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"


#if 0
WRITE8_MEMBER(fortyl_state::fortyl_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(offset,data);
}
#endif


WRITE8_MEMBER(fortyl_state::bank_select_w)
{
	if ((data != 0x02) && (data != 0xfd))
	{
//      logerror("WRONG BANK SELECT = %x !!!!\n",data);
//      popmessage("WRONG BANK SELECT = %x !!!!\n",data);
	}

	membank("bank1")->set_entry(data & 1);
}

WRITE8_MEMBER(fortyl_state::pix1_w)
{
//  if (data > 7)
//      logerror("pix1 = %2x\n", data);

	m_pix1 = data;
}

READ8_MEMBER(fortyl_state::fortyl_mcu_status_r)
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}

WRITE8_MEMBER(fortyl_state::pix1_mcu_w)
{
//  if (data > 7)
//      logerror("pix1 = %2x\n", data);

	m_pix1 = data;
}

WRITE8_MEMBER(fortyl_state::pix2_w)
{
//  if ((data!=0x00) && (data != 0xff))
//      logerror("pix2 = %2x\n", data);

	m_pix2[0] = m_pix2[1];
	m_pix2[1] = data;
}

#if 0
READ8_MEMBER(fortyl_state::pix1_r)
{
	return m_pix1;
}
#endif

READ8_MEMBER(fortyl_state::pix2_r)
{
	int res;
	int d1 = m_pix1 & 7;

	res = (((m_pix2[1] << (d1 + 8)) | (m_pix2[0] << d1)) & 0xff00) >> 8;

	return res;
}


/***************************************************************************/

void fortyl_state::driver_init()
{
	uint8_t *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ROM[0x10000], 0x2000);

	m_pix_color[0] = 0x000;
	m_pix_color[1] = 0x1e3;
	m_pix_color[2] = 0x16c;
	m_pix_color[3] = 0x1ec;
}

/***************************************************************************/

READ8_MEMBER(fortyl_state::snd_flag_r)
{
	return (m_soundlatch2->pending_r() ? 2 : 0) | 0xfd;
}

/***************************************************************************/

void fortyl_state::_40love_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram(); /* M5517P on main board */
	map(0x8800, 0x8800).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0x8801, 0x8801).rw(FUNC(fortyl_state::fortyl_mcu_status_r), FUNC(fortyl_state::pix1_mcu_w));      //pixel layer related
	map(0x8802, 0x8802).w(FUNC(fortyl_state::bank_select_w));
	map(0x8803, 0x8803).rw(FUNC(fortyl_state::pix2_r), FUNC(fortyl_state::pix2_w));       //pixel layer related
	map(0x8804, 0x8804).r(m_soundlatch2, FUNC(generic_latch_8_device::read));
	map(0x8804, 0x8804).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x8805, 0x8805).r(FUNC(fortyl_state::snd_flag_r)).nopw(); /*sound_reset*/ //????
	map(0x8807, 0x8807).nopr(); /* unknown */
	map(0x8808, 0x8808).portr("DSW3");
	map(0x8809, 0x8809).portr("P1");
	map(0x880a, 0x880a).portr("SYSTEM");
	map(0x880b, 0x880b).portr("P2");
	map(0x880c, 0x880c).portr("DSW1").w(FUNC(fortyl_state::fortyl_pixram_sel_w)); /* pixram bank select */
	map(0x880d, 0x880d).portr("DSW2").nopw(); /* unknown */
	map(0x9000, 0x97ff).rw(FUNC(fortyl_state::fortyl_bg_videoram_r), FUNC(fortyl_state::fortyl_bg_videoram_w)).share("videoram");      /* #1 M5517P on video board */
	map(0x9800, 0x983f).ram().share("video_ctrl");          /* video control area */
	map(0x9840, 0x987f).ram().share("spriteram");   /* sprites part 1 */
	map(0x9880, 0x98bf).rw(FUNC(fortyl_state::fortyl_bg_colorram_r), FUNC(fortyl_state::fortyl_bg_colorram_w)).share("colorram");      /* background attributes (2 bytes per line) */
	map(0x98c0, 0x98ff).ram().share("spriteram2");/* sprites part 2 */
	map(0xa000, 0xbfff).bankr("bank1");
	//map(0xbf00, 0xbfff) writes here when zooms-in/out, left-over or pixel line clearance?
	map(0xc000, 0xffff).rw(FUNC(fortyl_state::fortyl_pixram_r), FUNC(fortyl_state::fortyl_pixram_w)); /* banked pixel layer */
}

void fortyl_state::undoukai_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("bank1");
	map(0xa000, 0xa7ff).ram().share("mcu_ram"); /* M5517P on main board */
	map(0xa800, 0xa800).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0xa801, 0xa801).rw(FUNC(fortyl_state::fortyl_mcu_status_r), FUNC(fortyl_state::pix1_w));        //pixel layer related
	map(0xa802, 0xa802).w(FUNC(fortyl_state::bank_select_w));
	map(0xa803, 0xa803).rw(FUNC(fortyl_state::pix2_r), FUNC(fortyl_state::pix2_w));       //pixel layer related
	map(0xa804, 0xa804).r(m_soundlatch2, FUNC(generic_latch_8_device::read));
	map(0xa804, 0xa804).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xa805, 0xa805).r(FUNC(fortyl_state::snd_flag_r)).nopw(); /*sound_reset*/    //????
	map(0xa807, 0xa807).nopr().nopw(); /* unknown */
	map(0xa808, 0xa808).portr("DSW3");
	map(0xa809, 0xa809).portr("P1");
	map(0xa80a, 0xa80a).portr("SYSTEM");
	map(0xa80b, 0xa80b).portr("P2");
	map(0xa80c, 0xa80c).portr("DSW1").w(FUNC(fortyl_state::fortyl_pixram_sel_w)); /* pixram bank select */
	map(0xa80d, 0xa80d).portr("DSW2").nopw(); /* unknown */
	map(0xb000, 0xb7ff).rw(FUNC(fortyl_state::fortyl_bg_videoram_r), FUNC(fortyl_state::fortyl_bg_videoram_w)).share("videoram");      /* #1 M5517P on video board */
	map(0xb800, 0xb83f).ram().share("video_ctrl");          /* video control area */
	map(0xb840, 0xb87f).ram().share("spriteram");   /* sprites part 1 */
	map(0xb880, 0xb8bf).rw(FUNC(fortyl_state::fortyl_bg_colorram_r), FUNC(fortyl_state::fortyl_bg_colorram_w)).share("colorram");      /* background attributes (2 bytes per line) */
	map(0xb8e0, 0xb8ff).ram().share("spriteram2"); /* sprites part 2 */
	map(0xc000, 0xffff).rw(FUNC(fortyl_state::fortyl_pixram_r), FUNC(fortyl_state::fortyl_pixram_w));
}

WRITE8_MEMBER(fortyl_state::sound_control_0_w)
{
	m_snd_ctrl0 = data & 0xff;
//  popmessage("SND0 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);

	/* this definitely controls main melody voice on 2'-1 and 4'-1 outputs */
	for(int i=0;i<4;i++)
		m_ta7630->set_channel_volume(m_msm,i,m_snd_ctrl0 >> 4);

	//m_msm->set_output_gain(0, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	//m_msm->set_output_gain(1, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	//m_msm->set_output_gain(2, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */
	//m_msm->set_output_gain(3, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group1 from msm5232 */

}
WRITE8_MEMBER(fortyl_state::sound_control_1_w)
{
	m_snd_ctrl1 = data & 0xff;
//  popmessage("SND1 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);
	for(int i=0;i<4;i++)
		m_ta7630->set_channel_volume(m_msm,i+4,m_snd_ctrl1 >> 4);

	//m_msm->set_output_gain(4, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	//m_msm->set_output_gain(5, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	//m_msm->set_output_gain(6, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
	//m_msm->set_output_gain(7, m_vol_ctrl[(m_snd_ctrl1 >> 4) & 15] / 100.0); /* group2 from msm5232 */
}

WRITE8_MEMBER(fortyl_state::sound_control_2_w)
{
	m_snd_ctrl2 = data & 0xff;
//  popmessage("SND2 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);

	m_ta7630->set_device_volume(m_ay,m_snd_ctrl2 >> 4);
}

WRITE8_MEMBER(fortyl_state::sound_control_3_w)/* unknown */
{
	m_snd_ctrl3 = data & 0xff;
//  popmessage("SND3 0=%02x 1=%02x 2=%02x 3=%02x", m_snd_ctrl0, m_snd_ctrl1, m_snd_ctrl2, m_snd_ctrl3);
}

void fortyl_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc801).w(m_ay, FUNC(ym2149_device::address_data_w));
	map(0xca00, 0xca0d).w(m_msm, FUNC(msm5232_device::write));
	map(0xcc00, 0xcc00).w(FUNC(fortyl_state::sound_control_0_w));
	map(0xce00, 0xce00).w(FUNC(fortyl_state::sound_control_1_w));
	map(0xd800, 0xd800).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xd800, 0xd800).w(m_soundlatch2, FUNC(generic_latch_8_device::write));
	map(0xda00, 0xda00).nopr(); // unknown read
	map(0xda00, 0xda00).w("soundnmi", FUNC(input_merger_device::in_set<1>)); // enable NMI
	map(0xdc00, 0xdc00).w("soundnmi", FUNC(input_merger_device::in_clear<1>)); // disable NMI
	map(0xde00, 0xde00).nopr().w("dac", FUNC(dac_byte_interface::data_w));       /* signed 8-bit DAC - unknown read */
	map(0xe000, 0xefff).rom(); /* space for diagnostics ROM */
}


static INPUT_PORTS_START( 40love )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2") /* All OK */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )           PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )              PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Player Always Wins (Cheat)") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )                PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Stance Change Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Lob Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Stroke Button")

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Stance Change Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Lob Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Stroke Button")
INPUT_PORTS_END

static INPUT_PORTS_START( undoukai )
	PORT_INCLUDE( 40love )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "100000 200000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Players ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "1 Or 2" )
	PORT_DIPSETTING(    0x00, "1 To 4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3") /* & START */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                    PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "No Qualify (Cheat)")         PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START4 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout char_layout =
{
	8,8,
	0x400,
	4,
	{ 2*0x2000*8+0, 2*0x2000*8+4, 0,4 },
	{ 3,2,1,0, 11,10,9,8 },
	{ 0*8,2*8,4*8,6*8,8*8,10*8,12*8,14*8 },
	16*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0,4 },

	{ 3,2,1,0, 11,10,9,8,
		16*8+3, 16*8+2, 16*8+1, 16*8+0, 16*8+11, 16*8+10, 16*8+9, 16*8+8 },

	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8
};


static GFXDECODE_START( gfx_40love )
	GFXDECODE_ENTRY( "gfx2", 0, char_layout, 0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout, 0, 64 )
GFXDECODE_END

/*******************************************************************************/

void fortyl_state::machine_start()
{
	/* video */
	save_item(NAME(m_pix1));
	save_item(NAME(m_pix2));
	save_item(NAME(m_color_bank));
	save_item(NAME(m_screen_disable));
	/* sound */
	save_item(NAME(m_vol_ctrl));
	save_item(NAME(m_snd_ctrl0));
	save_item(NAME(m_snd_ctrl1));
	save_item(NAME(m_snd_ctrl2));
	save_item(NAME(m_snd_ctrl3));
}


void fortyl_state::machine_reset()
{
	/* video */
	m_pix1 = 0;
	m_pix2[0] = 0;
	m_pix2[1] = 0;
	m_color_bank = false;

	/* sound */
	m_snd_ctrl0 = 0;
	m_snd_ctrl1 = 0;
	m_snd_ctrl2 = 0;
	m_snd_ctrl3 = 0;
}

void fortyl_state::common(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8000000/2);
	m_maincpu->set_vblank_int("screen", FUNC(fortyl_state::irq0_line_hold));

	Z80(config, m_audiocpu, 8000000/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fortyl_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(fortyl_state::irq0_line_hold), attotime::from_hz(2*60)); /* source/number of IRQs is unknown */

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set("soundnmi", FUNC(input_merger_device::in_w<0>));

	GENERIC_LATCH_8(config, m_soundlatch2);

	INPUT_MERGER_ALL_HIGH(config, "soundnmi").output_handler().set_inputline("audiocpu", INPUT_LINE_NMI);

	TAITO68705_MCU(config, m_bmcu, 18432000/6); /* OK */

	config.set_maximum_quantum(attotime::from_hz(6000));  /* high interleave to ensure proper synchronization of CPUs */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(128,128+255, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(fortyl_state::screen_update_fortyl));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_40love);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 1024);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	TA7630(config, m_ta7630);

	YM2149(config, m_ay, 2000000);
	m_ay->port_a_write_callback().set(FUNC(fortyl_state::sound_control_2_w));
	m_ay->port_b_write_callback().set(FUNC(fortyl_state::sound_control_3_w));
	m_ay->add_route(ALL_OUTPUTS, "speaker", 0.1);

	MSM5232(config, m_msm, 8000000/4);
	m_msm->set_capacitors(1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6); /* 1.0 uF capacitors (verified on real PCB) */
	m_msm->add_route(0, "speaker", 1.0);   // pin 28  2'-1
	m_msm->add_route(1, "speaker", 1.0);   // pin 29  4'-1
	m_msm->add_route(2, "speaker", 1.0);   // pin 30  8'-1
	m_msm->add_route(3, "speaker", 1.0);   // pin 31 16'-1
	m_msm->add_route(4, "speaker", 1.0);   // pin 36  2'-2
	m_msm->add_route(5, "speaker", 1.0);   // pin 35  4'-2
	m_msm->add_route(6, "speaker", 1.0);   // pin 34  8'-2
	m_msm->add_route(7, "speaker", 1.0);   // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.2); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

void fortyl_state::_40love(machine_config &config)
{
	common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &fortyl_state::_40love_map);
}

void fortyl_state::undoukai(machine_config &config)
{
	common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &fortyl_state::undoukai_map);
}

/*******************************************************************************/

ROM_START( 40love )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 main CPU */
	ROM_LOAD( "a30-19.ic1", 0x00000, 0x2000, CRC(7baca598) SHA1(b1767f5af9b3f484afb4423afe1f9c15db92c2ac) )
	ROM_LOAD( "a30-20.ic2", 0x02000, 0x2000, CRC(a7b4f2cc) SHA1(67f570874fa0feb21f2a9a0712fadf78ebaad91c) )
	ROM_LOAD( "a30-21.ic3", 0x04000, 0x2000, CRC(49a372e8) SHA1(7c15fac65369d2e90b432c0f5c8e1d7295c379d1) )
	ROM_LOAD( "a30-22.ic4", 0x06000, 0x2000, CRC(0c06d2b3) SHA1(e5b0c8e57b0a6d131496e168023e12bacc17e93e) )
	ROM_LOAD( "a30-23.ic5", 0x10000, 0x2000, CRC(6dcd186e) SHA1(c8d88a2f35ba77ea822bdd8133033c8eb0bb5f72) ) /* banked at 0xa000 */
	ROM_LOAD( "a30-24.ic6", 0x12000, 0x2000, CRC(590c20c8) SHA1(93689d6a299dfbe33ffec42d13378091d8589b34) ) /* banked at 0xa000 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 sound CPU */
	ROM_LOAD( "a30-08.u08", 0x0000, 0x2000, CRC(2fc42ee1) SHA1(b56e5f9acbcdc476252e188f41ad7249dba6f8e1) )
	ROM_LOAD( "a30-09.u09", 0x2000, 0x2000, CRC(3a75abce) SHA1(ad2df26789d38196c0677c22ab8f176e99604b18) )
	ROM_LOAD( "a30-10.u10", 0x4000, 0x2000, CRC(393c4b5b) SHA1(a8e1dd5c33e929bc832cccc13b85ecd13fff1eb2) )
	ROM_LOAD( "a30-11.u37", 0x6000, 0x2000, CRC(11b2c6d2) SHA1(d55690512a37c4df2386a845e0cfb14f8052295b) )
	ROM_LOAD( "a30-12.u38", 0x8000, 0x2000, CRC(f7afd475) SHA1(dd09d5ca7fec5e0454f9efb8ebc722561010f124) )
	ROM_LOAD( "a30-13.u39", 0xa000, 0x2000, CRC(e806630f) SHA1(09022aae88ea0171a0aacf3260fa3a95e8faeb21) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a30-14"    , 0x0000, 0x0800, CRC(c4690279) SHA1(60bc77e03b9be434bb97a374a2fedeb8d049a660) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a30-25.u22", 0x0000, 0x2000, CRC(15e594cf) SHA1(d2d506a55f6ac2c191e5d5b3127021cde366c71c) )
	ROM_LOAD( "a30-26.u23", 0x2000, 0x2000, BAD_DUMP CRC(3a45a205) SHA1(0939ecaabbb9be2a0719ef252e3f244299734ba6)  )    /* this actually seems good, but we need to find another one to verify */
	ROM_LOAD( "a30-27.u24", 0x4000, 0x2000, CRC(57c67f6f) SHA1(293e5bfa7c859886abd70f78fe2e4b13a3fce3f5) )
	ROM_LOAD( "a30-28.u25", 0x6000, 0x2000, CRC(d581d067) SHA1(ce132cf2503917f0846b838c6ce4ad4183181bf9) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "a30-29.u62", 0x0000, 0x2000, CRC(02deaf40) SHA1(fb424a40bd9d959664a6d1ddf477fc16e694b9fa) )
	ROM_LOAD( "a30-30.u63", 0x2000, 0x2000, CRC(439f3731) SHA1(4661149baa8472989cc8ac85c51e55df69957d99) )
	ROM_LOAD( "a30-31.u64", 0x4000, 0x2000, CRC(7ed70e81) SHA1(f90a3ce701ebe746803cf01ea1f6725c552007de) )
	ROM_LOAD( "a30-32.u65", 0x6000, 0x2000, CRC(0434655b) SHA1(261c5e60e830967564c053dc1d40fbf1e7194fc8) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "a30-15.u03", 0x0000, 0x0400, CRC(55e38cc7) SHA1(823a6d7f29eadf5d12702d782d4297b0d4c65a0e) )  /* red */
	ROM_LOAD( "a30-16.u01", 0x0400, 0x0400, CRC(13997e20) SHA1(9fae1cf633409a88263dc66a17b1c2eeccd05f4f) )  /* green */
	ROM_LOAD( "a30-17.u02", 0x0800, 0x0400, CRC(5031f2f3) SHA1(1836d82fdc9f39cb318a791af2a935c27baabfd7) )  /* blue */
	ROM_LOAD( "a30-18.u13", 0x0c00, 0x0400, CRC(78697c0f) SHA1(31382ed4c0d44024f7f57a9de6407527f4d5b0d1) )  /* ??? */

ROM_END

ROM_START( 40lovej )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 main CPU */
	ROM_LOAD( "a30_01.70",    0x000000, 0x004000, CRC(1b89829e) SHA1(d875a4e3586fd9fb2e354e4353c9144ad68ce620) )
	ROM_LOAD( "a30_02.71",    0x004000, 0x004000, CRC(1468e71e) SHA1(e251ddf42ab51e9c391c213f54b709b71a3f1519) )
	ROM_LOAD( "a30_03.72",    0x010000, 0x004000, CRC(dbc0049d) SHA1(1fca22ca0794564bbd1f946afb644fef0342acca) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASEFF ) /* Z80 sound CPU */
	// not in the provided set, identical?
	ROM_LOAD( "a30-08.u08", 0x0000, 0x2000, BAD_DUMP CRC(2fc42ee1) SHA1(b56e5f9acbcdc476252e188f41ad7249dba6f8e1) )
	ROM_LOAD( "a30-09.u09", 0x2000, 0x2000, BAD_DUMP CRC(3a75abce) SHA1(ad2df26789d38196c0677c22ab8f176e99604b18) )
	ROM_LOAD( "a30-10.u10", 0x4000, 0x2000, BAD_DUMP CRC(393c4b5b) SHA1(a8e1dd5c33e929bc832cccc13b85ecd13fff1eb2) )
	ROM_LOAD( "a30-11.u37", 0x6000, 0x2000, BAD_DUMP CRC(11b2c6d2) SHA1(d55690512a37c4df2386a845e0cfb14f8052295b) )
	ROM_LOAD( "a30-12.u38", 0x8000, 0x2000, BAD_DUMP CRC(f7afd475) SHA1(dd09d5ca7fec5e0454f9efb8ebc722561010f124) )
	ROM_LOAD( "a30-13.u39", 0xa000, 0x2000, BAD_DUMP CRC(e806630f) SHA1(09022aae88ea0171a0aacf3260fa3a95e8faeb21) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	 // taken from World set, the provided one causes bad hw message
	ROM_LOAD( "a30-14"    , 0x0000, 0x0800, BAD_DUMP CRC(c4690279) SHA1(60bc77e03b9be434bb97a374a2fedeb8d049a660) )
//    ROM_LOAD( "a30_14",       0x000000, 0x000800, BAD_DUMP CRC(a4f770ce) SHA1(868e98528e8824e7329e9a298603e456bbc1f1f0) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a30_04.18",    0x000000, 0x004000, CRC(529a7489) SHA1(cf3fa83f16e2e62c1a4aa74b00080f1e167865a6) )
	ROM_LOAD( "a30_05.19",    0x004000, 0x004000, CRC(7017e5f1) SHA1(fc614fd41109a9a6236ed4a331eda74e5d49b946) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "a30_06.59",    0x000000, 0x004000, CRC(f744ea8e) SHA1(0bf6deabfac47237347af810332bc3716e3a26f0) )
	ROM_LOAD( "a30_07.60",    0x004000, 0x004000, CRC(b2af1359) SHA1(6a21e38cfb65d52b7e1209101f0dd497f9a71f46) )

	ROM_REGION( 0x1000, "proms", 0 )
	// not provided
	ROM_LOAD( "a30-15.u03", 0x0000, 0x0400, BAD_DUMP CRC(55e38cc7) SHA1(823a6d7f29eadf5d12702d782d4297b0d4c65a0e) )  /* red */
	ROM_LOAD( "a30-16.u01", 0x0400, 0x0400, BAD_DUMP CRC(13997e20) SHA1(9fae1cf633409a88263dc66a17b1c2eeccd05f4f) )  /* green */
	ROM_LOAD( "a30-17.u02", 0x0800, 0x0400, BAD_DUMP CRC(5031f2f3) SHA1(1836d82fdc9f39cb318a791af2a935c27baabfd7) )  /* blue */
	ROM_LOAD( "a30-18.u13", 0x0c00, 0x0400, BAD_DUMP CRC(78697c0f) SHA1(31382ed4c0d44024f7f57a9de6407527f4d5b0d1) )  /* ??? */
ROM_END

ROM_START( fieldday )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 main CPU  */
	ROM_LOAD( "a17_44.bin", 0x00000, 0x2000, CRC(d59812e1) SHA1(f3e7e2f09fba5964c92813cd652aa093fe3e4415) )
	ROM_LOAD( "a17_45.bin", 0x02000, 0x2000, CRC(828bfb9a) SHA1(0be24ec076b715d65e9c8e01e3be76628e4f60ed) )
	ROM_LOAD( "a23_05.bin", 0x04000, 0x2000, CRC(2670cad3) SHA1(8ba3a6b788fa4e997f9153226f6f13b32fc33124) )
	ROM_LOAD( "a23_06.bin", 0x06000, 0x2000, CRC(737ce7de) SHA1(52a46fe14978e217de81dcd529d16d62fb5a4e46) )
	ROM_LOAD( "a23_07.bin", 0x10000, 0x2000, CRC(ee2fb306) SHA1(f2b0a6af279b459fe61d56ba4d36d519318376fb) )
	ROM_LOAD( "a23_08.bin", 0x12000, 0x2000, CRC(1ed2f1ad) SHA1(e3cf954dd2c34759147d0c85da7a716a8eb0e820) )
	ROM_COPY( "maincpu" , 0x10000, 0x8000, 0x2000 ) /* to avoid 'bank bug' */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 sound CPU */
	ROM_LOAD( "a17_24.bin", 0x0000, 0x2000, CRC(6bac6b7f) SHA1(eb95192204a868737d609b789312ac37c31d3071) )
	ROM_LOAD( "a17_25.bin", 0x2000, 0x2000, CRC(570b90b1) SHA1(2a8c3bebd15655ffbfeaf40c2db90292afbb11ef) )
	ROM_LOAD( "a17_26.bin", 0x4000, 0x2000, CRC(7a8ea7f4) SHA1(1d9d2b54645266f95aa89cdbec6f82d4ac20d6e4) )
	ROM_LOAD( "a17_27.bin", 0x6000, 0x2000, CRC(e10594d9) SHA1(3df15b8b0c7af9fed93eca27237e15e6a7a8f835) )
	ROM_LOAD( "a17_28.bin", 0x8000, 0x2000, CRC(1a4d1dae) SHA1(fbc3c55ad9f15ead432c136eec648fe22e523ea7) )
	ROM_LOAD( "a17_29.bin", 0xa000, 0x2000, CRC(3c540007) SHA1(549e7ff260214c538913ff548dcb088987845911) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a17_14.bin", 0x0000, 0x0800, CRC(a686894d) SHA1(0578747a1869db98ee1962822491103f3f082cba) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a17_36.bin", 0x0000, 0x2000, CRC(e3dd51f7) SHA1(95a97ea925c5bc7bdc00887e6d17d817b36befc4) )
	ROM_LOAD( "a17_37.bin", 0x2000, 0x2000, CRC(1623f71f) SHA1(f5df7498b9a08e82ea11cb1b1fcdabca48cbf33a) )
	ROM_LOAD( "a17_38.bin", 0x4000, 0x2000, CRC(ca9f74db) SHA1(a002f1dfa9497793bfb18292e7a71ae12d70fb88) )
	ROM_LOAD( "a17_39.bin", 0x6000, 0x2000, CRC(fb6c667c) SHA1(da56be8d997db199588ee22fae30cc6d87e80704) )


	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "a23_09.bin", 0x0000, 0x2000, CRC(1e430be5) SHA1(9296e1a0d820bb218578d55b739b4fc5fdafb125) )
	ROM_LOAD( "a23_10.bin", 0x2000, 0x2000, CRC(ee2e54f0) SHA1(0a92fa39696a8005f9441131b6d98205b7c26e7b) )
	ROM_LOAD( "a23_11.bin", 0x4000, 0x2000, CRC(6d37f15c) SHA1(3eb9a2e230d88f2871e6972a01d8e7cc7db1b123) )
	ROM_LOAD( "a23_12.bin", 0x6000, 0x2000, CRC(86da42d2) SHA1(aa79cd954c96217ca2daf37addac168f8cca24f9) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "a17-15.10v", 0x0000, 0x0400, CRC(9df472b7) SHA1(0cd9dd735238daf8e8228ba9481df57fb8925328) )  /* red */
	ROM_LOAD( "a17-16.8v",  0x0400, 0x0400, CRC(3bf1ff5f) SHA1(a0453851aefa9acdba4a86aaca8c442cb8550987) )  /* green */
	ROM_LOAD( "a17-17.9v",  0x0800, 0x0400, CRC(c42ae956) SHA1(057ce3783305c98622f7dfc0ee7d4882137a2ef8) )  /* blue */
	ROM_LOAD( "a17-18.23v", 0x0c00, 0x0400, CRC(3023a1da) SHA1(08ce4c6e99d04b358d66f0588852311d07183619) )  /* ??? */
ROM_END

ROM_START( undoukai )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 main CPU  */
	ROM_LOAD( "a17-01.70c", 0x00000, 0x4000, CRC(6ce324d9) SHA1(9c5207ac897eaae5a6aa1a05a918c9cb58544664) )
	ROM_LOAD( "a17-02.71c", 0x04000, 0x4000, CRC(055c7ef1) SHA1(f974bd441b8e3621ac5f8d36104791c97051a97a) )
	ROM_LOAD( "a17-03.72c", 0x10000, 0x4000, CRC(9034a5c5) SHA1(bc3dae0dee08b6989275ac220fc76bfe61367154) ) /* banked at 0x8000 */
	ROM_COPY( "maincpu" , 0x10000, 0x8000, 0x2000 ) /* to avoid 'bank bug' */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 sound CPU */
	ROM_LOAD( "a17-08.8s",  0x0000, 0x2000, CRC(2545aa0e) SHA1(190ef99890251e1e49b14ffd28f2badb4d0d8fbe) )
	ROM_LOAD( "a17-09.9s",  0x2000, 0x2000, CRC(57e2cdbb) SHA1(ae6187d62fb36a37be06040e0fd85e0252cdf750) )
	ROM_LOAD( "a17-10.10s", 0x4000, 0x2000, CRC(38a288fe) SHA1(af4979cae59ca2569a3663132451b9b554552a79) )
	ROM_LOAD( "a17-11.37s", 0x6000, 0x2000, CRC(036d6969) SHA1(20e03dab14d44bf3c7c6aace3b28b2826581d1c7) )
	ROM_LOAD( "a17-12.38s", 0x8000, 0x2000, CRC(cb7e6dcd) SHA1(5286c6d340c1d465caebae5dd7e3d4ff8b7f8f5e) )
	ROM_LOAD( "a17-13.39s", 0xa000, 0x2000, CRC(0a40930e) SHA1(8c4b9fa0aed67a3e269c2136ef81791fc8acd1da) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a17_14.bin", 0x0000, 0x0800, CRC(a686894d) SHA1(0578747a1869db98ee1962822491103f3f082cba) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a17-04.18v", 0x0000, 0x4000, CRC(84dabee2) SHA1(698f12ee4201665988248853dafbf4b16dfc6517) )
	ROM_LOAD( "a17-05.19v", 0x4000, 0x4000, CRC(10bf3451) SHA1(23ebb1409c90d225ff5a13ad23d4dff1acaf904a) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "a17-06.59v", 0x0000, 0x4000, CRC(50f28ad9) SHA1(001555ad123ac85000999b1aa39c1b2568e26f46) )
	ROM_LOAD( "a17-07.60v", 0x4000, 0x4000, CRC(7a4b4238) SHA1(8e58803645e61a7144a659d403f318a8899d36e2) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "a17-15.10v", 0x0000, 0x0400, CRC(9df472b7) SHA1(0cd9dd735238daf8e8228ba9481df57fb8925328) )  /* red */
	ROM_LOAD( "a17-16.8v",  0x0400, 0x0400, CRC(3bf1ff5f) SHA1(a0453851aefa9acdba4a86aaca8c442cb8550987) )  /* green */
	ROM_LOAD( "a17-17.9v",  0x0800, 0x0400, CRC(c42ae956) SHA1(057ce3783305c98622f7dfc0ee7d4882137a2ef8) )  /* blue */
	ROM_LOAD( "a17-18.23v", 0x0c00, 0x0400, CRC(3023a1da) SHA1(08ce4c6e99d04b358d66f0588852311d07183619) )  /* ??? */
ROM_END

GAME( 1984, 40love,   0,        _40love,  40love,   fortyl_state, driver_init, ROT0, "Taito Corporation", "Forty-Love (World)",           MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, 40lovej,  40love,   _40love,  40love,   fortyl_state, driver_init, ROT0, "Taito Corporation", "Forty-Love (Japan)",           MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // several ROMs needs double checking
GAME( 1984, fieldday, 0,        undoukai, undoukai, fortyl_state, driver_init, ROT0, "Taito Corporation", "Field Day",            MACHINE_SUPPORTS_SAVE )
GAME( 1984, undoukai, fieldday, undoukai, undoukai, fortyl_state, driver_init, ROT0, "Taito Corporation", "The Undoukai (Japan)", MACHINE_SUPPORTS_SAVE )
