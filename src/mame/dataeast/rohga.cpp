// license:BSD-3-Clause
// copyright-holders: Bryan McPhail
/***************************************************************************

    Data East 'Rohga' era hardware:

    Rogha Armour Attack         (c) 1991 Data East Corporation
    Wizard Fire                 (c) 1992 Data East Corporation
    Nitro Ball/Gun Ball         (c) 1992 Data East Corporation
    Hangzo (prototype)          (c) 1992 Hot-B
    Schmeiser Robo              (c) 1993 Hot-B

    This hardware is capable of alpha-blending on sprites and playfields

    Todo:  Sprite priority errors in Nitro Ball.

    Schmeiser Robo runs on a slightly modified Rohga pcb.

    Emulation by Bryan McPhail, mish@tendril.co.uk

    Schmeiser Robo PCB Layout
    -------------------------

    DE-0353-3
    |---------------------------------------------------|
    |62256  62256                         104  DSW2 DSW1|
    |                                                   |
    |               59     PAL                          |
    |                      PAL                          |
    |SR001J  SR006J                            DSW3     |
    |                      6264                         |
    |SR002-74  PAL         6264           113           |
    |SR003-74       74     6264                        J|
    |                      6264   HB00031E             A|
    |                             PAL                  M|
    |                                                  M|
    |               55            SR011                A|
    |                             SR012   M6295#2       |
    |                                                   |
    |                      45     YM2151  M6295#1    CN2|
    |       SR007   52            LH5168                |
    |       SR008          71     SR013   YM3014        |
    |SR004  SR009   52                    JP3 VOL1 VOL2 |
    |SR005  SR010                                       |
    |                     28MHz  32.220MHz      TA8205  |
    |---------------------------------------------------|

    Notes:
                68000 clock: 14.000MHz
                VSync: 58kHz
                HSync: 15.68kHz
                YM2151 clock: 3.58MHz
                M6295#1 clock: 1.00MHz, Sample Rate: = / 132
                M6295#2 clock: 2.00MHz, Sample Rate: = / 132
                CN2: Connector for stereo sound out
                Vol1, Vol2: Volume POT for left/right speakers. In Mono mode, only right volume is used.
                JP3: jumper to set mono/stereo output
                PCB has several wire mods also

                SR001, SR002 : 27C040
                SR003, SR004 : 4M Mask (40 pin 16 bit)
                SR004 - SR010: 8M Mask (42 pin 16 bit)
                SR011, SR012 : 4M Mask (32 pin 8 bit)
                HB00031E     : MB7116 (512 bytes x 4 bit) PROM near chip 113

    Gun Ball PCB Layout
    ---------------------

    DE-0358-3
    |---------------------------------------------------|
    |62256   62256  |---|  PAL           |---|    CN3   |
    |JC00    JC01   |59 |  PAL           |146| DSW1 DSW2|
    |JC02    JC03   |---|  PAL           |---|          |
    |JC04-3  JC05-3        PAL                          |
    |                                                   |
    |MAV-00  JC06 |----|                 |-----|        |
    |MAV-01  JC07 | 56 |                 | 113 |        |
    |             |----|     6264        |     |       J|
    |                        6264        |-----|       A|
    |             |----|     6264                      M|
    |MAV-02 MAV-03| 74 |     6264                      M|
    |             |----|     PAL                       A|
    |             |----|     JN-00          M6295#2     |
    |MAV-04 MAV-05| 52 | |---|       MAV-11       JP7   |
    |             |----| | 71|       MAV-10 M6295#1  CN2|
    |MAV-06 MAV-07       |---|       JC08               |
    |             |----|       |----|LH5168  YM3012     |
    |             | 52 | |---| |6280|         VOL1 VOL2 |
    |             |----| | 71| |----|YM2151             |
    |MAV-08 MAV-09  28MHz|---|   32.220MHz      TA8205  |
    |---------------------------------------------------|

    Notes:
                68000 clock   : 14.000MHz [28/2]
                HuC6280 clock : 8.050MHz [32.200/4]
                YM2151 clock  : 3.577777778MHz [32.200/9]
                M6295#1 clock : 1.00625MHz [32.200/32], Sample Rate = 1006250 / 132
                M6295#2 clock : 2.0125MHz  [32.200/16], Sample Rate = 2012500 / 132
                CN2           : Connector for stereo sound out
                VOL1, VOL2    : Volume pot for left/right speakers. In Mono mode, only right volume is used.
                JP7           : Jumper to set mono/stereo output
                JN-00         : Fujitsu MB7116 PROM
                VSync         : 58kHz

    TODO:
        nitrobal : blending, priority function wrong
        wizdfire : Wizard Fire and Dark Seal 2 use the same mask ROM for
    ADPCM speech samples. Sample banks 0/1 are used for the Japanese
    version and 2/3 are used for the English version, whose ROM loading
    currently works around this by swapping the banks. It seems likely that
    the ROM's highest-order address line is not software-controlled, but
    either pulled high or grounded by an optional jumper. The PCB should
    be examined to determine what part it actually connects to.

***************************************************************************/

#include "emu.h"
#include "rohga.h"

#include "decocrpt.h"

#include "cpu/m68000/m68000.h"
#include "sound/ymopm.h"

#include "screen.h"
#include "speaker.h"

#include <algorithm>


u16 rohga_state::irq_ack_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(6, CLEAR_LINE);
	return 0;
}

void rohga_state::irq_ack_w(u16 data)
{
	/* This might actually do more, nitrobal for example sets 0xca->0xffff->0x80 at startup then writes 7 all the time
	   except when a credit is inserted (writes 6 twice).
	   Wizard Fire / Dark Seal 2 just writes 1 all the time, so I just don't trust it much for now... -AS */
	m_maincpu->set_input_line(6, CLEAR_LINE);
}

/**********************************************************************************/

u16 rohga_state::ioprot_r(offs_t offset)
{
	const int real_address = 0 + (offset * 2);
	const int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	u8 cs = 0;
	const u16 data = m_ioprot->read_data(deco146_addr, cs);
	return data;
}

void rohga_state::ioprot_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int real_address = 0 + (offset * 2);
	const int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	u8 cs = 0;
	m_ioprot->write_data(deco146_addr, data, mem_mask, cs);
}


void rohga_state::rohga_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();

	map(0x200000, 0x20000f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));
	map(0x240000, 0x24000f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));

	map(0x280000, 0x283fff).rw(FUNC(rohga_state::ioprot_r), FUNC(rohga_state::ioprot_w)).share("prot16ram"); // Protection device

	map(0x2c0000, 0x2c0001).portr("DSW3");

	map(0x300000, 0x300001).w(FUNC(rohga_state::rohga_buffer_spriteram16_w)); // write 1 for sprite DMA
	map(0x310000, 0x310009).nopw(); // Palette control?
	map(0x31000a, 0x31000b).w(m_decocomn, FUNC(decocomn_device::palette_dma_w)); // Write 1111 for DMA?  (Or any value?)
	map(0x320000, 0x320001).nopw(); // ?
	map(0x322000, 0x322001).w(m_decocomn, FUNC(decocomn_device::priority_w));
	map(0x321100, 0x321101).r(FUNC(rohga_state::irq_ack_r)); // IRQ ack?  Value not used

	map(0x3c0000, 0x3c1fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x3c2000, 0x3c2fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x3c4000, 0x3c4fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x3c6000, 0x3c6fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));

	map(0x3c8000, 0x3c9fff).ram().share(m_pf_rowscroll[0]);
	map(0x3ca000, 0x3cafff).mirror(0x1000).ram().share(m_pf_rowscroll[1]);
	map(0x3cc000, 0x3ccfff).mirror(0x1000).ram().share(m_pf_rowscroll[2]);
	map(0x3ce000, 0x3cefff).mirror(0x1000).ram().share(m_pf_rowscroll[3]);

	map(0x3d0000, 0x3d07ff).ram().share("spriteram1");
	map(0x3e0000, 0x3e1fff).ram().w(m_decocomn, FUNC(decocomn_device::buffered_palette_w)).share("paletteram");
	map(0x3f0000, 0x3f3fff).ram(); // Main RAM
}


void rohga_state::wizdfire_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();

	map(0x200000, 0x200fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x202000, 0x202fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x208000, 0x208fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x20a000, 0x20afff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));

	map(0x20b000, 0x20b3ff).nopw(); // ? Always 0 written
	map(0x20c000, 0x20c7ff).ram().share(m_pf_rowscroll[2]);
	map(0x20e000, 0x20e7ff).ram().share(m_pf_rowscroll[3]);

	map(0x300000, 0x30000f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));
	map(0x310000, 0x31000f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));

	map(0x320000, 0x320001).w(m_decocomn, FUNC(decocomn_device::priority_w)); // Priority
	map(0x320002, 0x320003).nopw(); // ?
	map(0x320004, 0x320005).w(FUNC(rohga_state::irq_ack_w)); // VBL IRQ ack

	map(0x340000, 0x3407ff).ram().share("spriteram1");
	map(0x350000, 0x350001).w("spriteram1", FUNC(buffered_spriteram16_device::write)); // Triggers DMA for spriteram
	map(0x360000, 0x3607ff).ram().share("spriteram2");
	map(0x370000, 0x370001).w("spriteram2", FUNC(buffered_spriteram16_device::write)); // Triggers DMA for spriteram

	map(0x380000, 0x381fff).ram().w(m_decocomn, FUNC(decocomn_device::buffered_palette_w)).share("paletteram");
	map(0x390008, 0x390009).w(m_decocomn, FUNC(decocomn_device::palette_dma_w));

	map(0xfdc000, 0xfe3fff).ram();
	map(0xfe4000, 0xfe7fff).rw(FUNC(rohga_state::ioprot_r), FUNC(rohga_state::ioprot_w)).share("prot16ram"); // Protection device
	map(0xfe8000, 0xffffff).ram();
}


void rohga_state::nitrobal_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();

	map(0x200000, 0x200fff).mirror(0x1000).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x202000, 0x2027ff).mirror(0x800).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x208000, 0x2087ff).mirror(0x800).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x20a000, 0x20a7ff).mirror(0x800).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));

	map(0x204000, 0x2047ff).ram().share(m_pf_rowscroll[0]);
	map(0x206000, 0x2067ff).ram().share(m_pf_rowscroll[1]);
	map(0x20c000, 0x20c7ff).ram().share(m_pf_rowscroll[2]);
	map(0x20e000, 0x20e7ff).ram().share(m_pf_rowscroll[3]);

	map(0x300000, 0x30000f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));
	map(0x310000, 0x31000f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));

	map(0x320000, 0x320001).portr("DSW3").w(m_decocomn, FUNC(decocomn_device::priority_w)); // Priority
	map(0x320002, 0x320003).nopw(); // ?
	map(0x320004, 0x320005).w(FUNC(rohga_state::irq_ack_w)); // VBL IRQ ack

	map(0x340000, 0x3407ff).ram().share("spriteram1");
	map(0x350000, 0x350001).w("spriteram1", FUNC(buffered_spriteram16_device::write)); // Triggers DMA for spriteram
	map(0x360000, 0x3607ff).ram().share("spriteram2");
	map(0x370000, 0x370001).w("spriteram2", FUNC(buffered_spriteram16_device::write)); // Triggers DMA for spriteram

	map(0x380000, 0x381fff).ram().w(m_decocomn, FUNC(decocomn_device::buffered_palette_w)).share("paletteram");
	map(0x390008, 0x390009).w(m_decocomn, FUNC(decocomn_device::palette_dma_w));

	map(0xfec000, 0xff3fff).ram();
	map(0xff4000, 0xff7fff).rw(FUNC(rohga_state::ioprot_r), FUNC(rohga_state::ioprot_w)).share("prot16ram"); // Protection device

	map(0xff8000, 0xffffff).ram();
}


void rohga_state::hotb_base_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20000f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));
	map(0x240000, 0x24000f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));
	map(0x280000, 0x283fff).rw(FUNC(rohga_state::ioprot_r), FUNC(rohga_state::ioprot_w)).share("prot16ram"); // Protection device

	map(0x2c0000, 0x2c0001).portr("DSW3");
	map(0x300000, 0x300001).portr("DSW3").w(FUNC(rohga_state::rohga_buffer_spriteram16_w)); // write 1 for sprite DMA
	map(0x310002, 0x310003).portr("SYSTEM");
	map(0x310000, 0x310009).nopw(); // Palette control?
	map(0x31000a, 0x31000b).w(m_decocomn, FUNC(decocomn_device::palette_dma_w)); // Write 1111 for DMA?  (Or any value?)
	map(0x320000, 0x320001).nopw(); // bit 4: cleared on IRQ routine start, set on end
	map(0x322000, 0x322001).w(m_decocomn, FUNC(decocomn_device::priority_w));
	map(0x321100, 0x321101).w(FUNC(rohga_state::irq_ack_w));  // IRQ ack?  Value not used

	map(0x3c0000, 0x3c1fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x3c2000, 0x3c2fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x3c4000, 0x3c4fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x3c6000, 0x3c6fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x3c8000, 0x3c9fff).ram().share(m_pf_rowscroll[0]);
	map(0x3ca000, 0x3cafff).mirror(0x1000).ram().share(m_pf_rowscroll[1]);
	map(0x3cc000, 0x3ccfff).mirror(0x1000).ram().share(m_pf_rowscroll[2]);
	map(0x3ce000, 0x3cefff).mirror(0x1000).ram().share(m_pf_rowscroll[3]);

	map(0x3d0000, 0x3d07ff).ram().share("spriteram1");
	map(0x3e0000, 0x3e1fff).mirror(0x2000).ram().w(m_decocomn, FUNC(decocomn_device::buffered_palette_w)).share("paletteram");
}

void rohga_state::schmeisr_map(address_map &map)
{
	hotb_base_map(map);

	map(0xff0000, 0xff7fff).ram(); // Main RAM
}

void rohga_state::hangzo_map(address_map &map)
{
	hotb_base_map(map);

	map(0x3f0000, 0x3f3fff).ram(); // Main RAM
}

/******************************************************************************/

void rohga_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).noprw();
	map(0x110000, 0x110001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140000).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}


/**********************************************************************************/

static INPUT_PORTS_START( rohga )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "2 Credits to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:1" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:2" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" ) // Listed as "Unused"
	PORT_DIPNAME( 0x3000, 0x3000, "Player's Vitality" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x1000, "Lowest" )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")      // DIP switch bank 3
	PORT_DIPNAME( 0x0001, 0x0001, "Stage Clear Bonus" ) PORT_DIPLOCATION("SW3:1") // Life Recovery At stage clear
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Enemy's Vitality" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( High ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Highest ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Enemy Encounter Rate" ) PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( High ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Highest ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Enemy's Weapon Speed" ) PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(      0x0080, "Slow" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
INPUT_PORTS_END

static INPUT_PORTS_START( wizdfire )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "2 Credits to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0300, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Magic Gauge Speed" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "Very Slow" )
	PORT_DIPSETTING(      0x1000, "Slow" )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, "Fast" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nitrobal )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "2 Credits to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Difficulty?"  ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Split Coin Chutes" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Players) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x4000, 0x4000, "Shot Button to Start" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")       // Not really a DSW, but this way it's easier to read by tag
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )
INPUT_PORTS_END

static INPUT_PORTS_START( schmeisr )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_6C ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" ) // Listed as "Unused"

	PORT_START("DSW3")  // DIP switch bank 3 - This bank of switches are _NOT_ shown in the test mode screen
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug Mode" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Freeze Screen" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hangzo )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "2 Credits to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3") // Either 3 & 4 are Difficulty
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5") // or more likely 5 & 6 are Player's Vitality like Rohga (all other dips seem to match Rohga)
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Freeze Player" ) PORT_DIPLOCATION("SW3:2") // With on, player will not fall down to playing field and be stuck floating at the top of the screen
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Freeze Frame" ) PORT_DIPLOCATION("SW3:3") // P1 starts & stops freeze, when frozen P2 is single step frame
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Freeze Screen" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Debug Mode" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



/**********************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*8*2    // every char takes 16 consecutive bytes
};

static const gfx_layout tilelayout_6bpp =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+8, RGN_FRAC(2,3), RGN_FRAC(1,3)+8, RGN_FRAC(1,3), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	16*16*2
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	16*16*2
};

static GFXDECODE_START( gfx_rohga )
	GFXDECODE_ENTRY( "chars",  0, charlayout,    0, 32 )    // Characters 8x8
	GFXDECODE_ENTRY( "tiles1", 0, tilelayout,    0, 32 )    // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout,  512, 32 )    // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_rohga_spr )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout_6bpp, 1024, 16 )  // Sprites 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_wizdfire_spr1 )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout, 0/*1024*/, 128 ) // Sprites 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_wizdfire_spr2 )
	GFXDECODE_ENTRY( "sprites2", 0, tilelayout, 0/*1536*/, 128 )
GFXDECODE_END

static GFXDECODE_START( gfx_schmeisr_spr )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout, 1024, 64 )    // Sprites 16x16
GFXDECODE_END

/**********************************************************************************/

void rohga_state::sound_bankswitch_w(u8 data)
{
	m_oki[0]->set_rom_bank(BIT(data, 0));
	m_oki[1]->set_rom_bank(BIT(data, 1));
}

/**********************************************************************************/

DECO16IC_BANK_CB_MEMBER(rohga_state::bank_callback)
{
	return ((bank >> 4) & 0x3) << 12;
}

DECOSPR_PRIORITY_CB_MEMBER(rohga_state::rohga_pri_callback)
{
	switch (pri & 0x6000)
	{
		case 0x0000: return 0;
		case 0x4000: return 0xf0;
		case 0x6000: return 0xf0 | 0xcc;
		case 0x2000: return 0;//0xf0|0xcc; // Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer)
	}

	return 0;
}

DECOSPR_COLOUR_CB_MEMBER(rohga_state::rohga_col_callback)
{
	return (col >> 9) & 0xf;
}

DECOSPR_COLOUR_CB_MEMBER(rohga_state::schmeisr_col_callback)
{
	u16 colour = ((col >> 9) & 0xf) << 2;
	if (col & 0x8000)
		colour++;

	return colour;
}

void rohga_state::rohga_base(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14'000'000);
	m_maincpu->set_vblank_int("screen", FUNC(rohga_state::irq6_line_assert));

	H6280(config, m_audiocpu, 32'220'000/4/3); // verified on PCB (8.050Mhz is XIN on pin 10 of H6280
	m_audiocpu->set_addrmap(AS_PROGRAM, &rohga_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	m_audiocpu->add_route(ALL_OUTPUTS, "speaker", 0, 1);

	// video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram[0]);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);

	PALETTE(config, m_palette).set_entries(2048);

	DECOCOMN(config, m_decocomn, 0);
	m_decocomn->set_palette_tag(m_palette);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x64);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(rohga_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(rohga_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag("gfxdecode");

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x00);
	m_deco_tilegen[1]->set_pf2_col_bank(0x10);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(rohga_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(rohga_state::bank_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag("gfxdecode");

	GFXDECODE(config, "gfxdecode", m_palette, gfx_rohga);

	DECO104PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline("audiocpu", 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 32'220'000/9));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 1);    // IRQ2
	ymsnd.port_write_handler().set(FUNC(rohga_state::sound_bankswitch_w));
	ymsnd.add_route(0, "speaker", 0.36, 0);
	ymsnd.add_route(1, "speaker", 0.36, 1);

	OKIM6295(config, m_oki[0], 32'220'000/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.46, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.46, 1);

	OKIM6295(config, m_oki[1], 32'220'000/16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.18, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.18, 1);
}

void rohga_state::rohga(machine_config &config)
{
	rohga_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &rohga_state::rohga_map);

	// video hardware
	subdevice<screen_device>("screen")->set_screen_update(FUNC(rohga_state::screen_update_rohga));
	subdevice<screen_device>("screen")->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen[0], 0, m_palette, gfx_rohga_spr);
	m_sprgen[0]->set_pri_callback(FUNC(rohga_state::rohga_pri_callback));
	m_sprgen[0]->set_col_callback(FUNC(rohga_state::rohga_col_callback));
}

void rohga_state::wizdfire(machine_config &config)
{
	rohga_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &rohga_state::wizdfire_map);

	// video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(rohga_state::screen_update_wizdfire));

	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);

	DECO_SPRITE(config, m_sprgen[0], 0, m_palette, gfx_wizdfire_spr1);
	DECO_SPRITE(config, m_sprgen[1], 0, m_palette, gfx_wizdfire_spr2);

	m_ioprot->set_interface_scramble_reverse();

	MCFG_VIDEO_START_OVERRIDE(rohga_state, wizdfire)
}

void rohga_state::nitrobal(machine_config &config)
{
	rohga_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &rohga_state::nitrobal_map);

	// video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(rohga_state::screen_update_nitrobal));

	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_32x32);

	m_deco_tilegen[1]->set_pf1_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0);
	m_deco_tilegen[1]->set_pf2_col_bank(0);
	m_deco_tilegen[1]->set_pf1_col_mask(0);
	m_deco_tilegen[1]->set_pf2_col_mask(0);

	DECO_SPRITE(config, m_sprgen[0], 0, m_palette, gfx_wizdfire_spr1);
	DECO_SPRITE(config, m_sprgen[1], 0, m_palette, gfx_wizdfire_spr2);

	MCFG_VIDEO_START_OVERRIDE(rohga_state, wizdfire)

	DECO146PROT(config.replace(), m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline("audiocpu", 0);
	m_ioprot->set_interface_scramble_reverse();
	m_ioprot->set_use_magic_read_address_xor(true);
}

void rohga_state::schmeisr(machine_config &config)
{
	rohga_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &rohga_state::schmeisr_map);

	// video hardware
	subdevice<screen_device>("screen")->set_screen_update(FUNC(rohga_state::screen_update_rohga));
	subdevice<screen_device>("screen")->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen[0], 0, m_palette, gfx_schmeisr_spr);
	m_sprgen[0]->set_pri_callback(FUNC(rohga_state::rohga_pri_callback));
	m_sprgen[0]->set_col_callback(FUNC(rohga_state::schmeisr_col_callback)); // wire mods on pcb...
}

void rohga_state::hangzo(machine_config &config)
{
	schmeisr(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &rohga_state::hangzo_map);
}


/**********************************************************************************/

ROM_START( rohga ) // Asia/Europe v5.0
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ht-00-1.2a",  0x000000, 0x40000, CRC(1ed84a67) SHA1(4778e3878aa20c12ab8d3dffeb732b90e2de7cfe) )
	ROM_LOAD16_BYTE( "ht-03-1.2d",  0x000001, 0x40000, CRC(84e7ebf6) SHA1(e51884431c7d66d9795d9939aa1e928f662171be) )
	ROM_LOAD16_BYTE( "mam00.8a",    0x100000, 0x80000, CRC(0fa440a6) SHA1(f0f84c630fc30ec164acc21de871c857d391c398) )
	ROM_LOAD16_BYTE( "mam07.8d",    0x100001, 0x80000, CRC(f8bc7f20) SHA1(909324248bd207f3b01d9f694975b629d8ccaa08) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ha04.18p",  0x00000,  0x10000,  CRC(eb6608eb) SHA1(0233677970aba12783dd4d6d58d70568ef641115) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "ha01.13a",  0x00000,  0x10000,  CRC(fb8f8519) SHA1(0a237426561e5fef6a062e1ad5ae02204f72d5f9) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "ha02.14a",  0x00001,  0x10000,  CRC(aa47c17f) SHA1(830dfcbfaef90133d93b0fbf3cf2067498fa658b) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mam01.10a", 0x000000, 0x080000,  CRC(dbf4fbcc) SHA1(2f289556fd25beb7d30501cba17ac35ad28c5b91) ) // Encrypted tiles
	ROM_LOAD( "mam02.11a", 0x080000, 0x080000,  CRC(b1fac481) SHA1(da370499ea8ff7b3dd338b31f3799b760fd0d981) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mam08.17d",  0x000000, 0x100000,  CRC(ca97a83f) SHA1(2e097840ae56cf19ad2651d59c31182f47239d60) ) // tiles 1 & 2
	ROM_LOAD( "mam09.18d",  0x100000, 0x100000,  CRC(3f57d56f) SHA1(0d4537da6ab62762179215deae72fe2e6a7869e1) )

	ROM_REGION( 0x600000, "sprites1", 0 )
	ROM_LOAD( "mam05.19a", 0x000000, 0x100000,  CRC(307a2cd1) SHA1(d7a795e47cf1533f0bb5a96162c8025282abe09f) ) // 6bpp sprites
	ROM_LOAD( "mam06.20a", 0x100000, 0x100000,  CRC(a1119a2d) SHA1(876f9295c2032ce491b45a103ffafc750d8c78e1) )
	ROM_LOAD( "mam10.19d", 0x200000, 0x100000,  CRC(99f48f9f) SHA1(685787de54e9158ced80f3821996c3a63f2a72a2) )
	ROM_LOAD( "mam11.20d", 0x300000, 0x100000,  CRC(c3f12859) SHA1(45fdfd55f606316c936f0a9e6b4940740138d344) )
	ROM_LOAD( "mam03.17a", 0x400000, 0x100000,  CRC(fc4dfd48) SHA1(0c5f5a09833ebeb3018e65edd6f7ce06d4ba84ed) )
	ROM_LOAD( "mam04.18a", 0x500000, 0x100000,  CRC(7d3b38bf) SHA1(9f83ad7497ed57405ad648f403eb69f776567a50) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mam12.14p", 0x00000,  0x80000,  CRC(6f00b791) SHA1(c9fbc9ab5ce84fec79efa0a23373be97a27bf898) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mam13.15p", 0x00000,  0x80000,  CRC(525b9461) SHA1(1d9bb3725dfe601b05a779b84b4191455087b969) )

	ROM_REGION( 512, "proms", 0 )
	ROM_LOAD( "hb-00.11p", 0x00000,  0x200,  CRC(b7a7baad) SHA1(39781c3412493b985d3616ac31142fc00bbcddf4) ) // ?
ROM_END

ROM_START( rohga1 ) // Asia/Europe v3.0
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "jd00.bin",  0x000000, 0x40000, CRC(e046c77a) SHA1(bb4d987a579a1a1524bc150ebda9cd24ed77a733) )
	ROM_LOAD16_BYTE( "jd03.bin",  0x000001, 0x40000, CRC(2c5120b8) SHA1(41b6618f0f086efd48486f72ada2fb6f184ad85b) )
	ROM_LOAD16_BYTE( "mam00.8a",  0x100000, 0x80000, CRC(0fa440a6) SHA1(f0f84c630fc30ec164acc21de871c857d391c398) )
	ROM_LOAD16_BYTE( "mam07.8d",  0x100001, 0x80000, CRC(f8bc7f20) SHA1(909324248bd207f3b01d9f694975b629d8ccaa08) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ha04.18p",  0x00000,  0x10000,  CRC(eb6608eb) SHA1(0233677970aba12783dd4d6d58d70568ef641115) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "ha01.13a",  0x00000,  0x10000,  CRC(fb8f8519) SHA1(0a237426561e5fef6a062e1ad5ae02204f72d5f9) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "ha02.14a",  0x00001,  0x10000,  CRC(aa47c17f) SHA1(830dfcbfaef90133d93b0fbf3cf2067498fa658b) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mam01.10a", 0x000000, 0x080000,  CRC(dbf4fbcc) SHA1(2f289556fd25beb7d30501cba17ac35ad28c5b91) ) // Encrypted tiles
	ROM_LOAD( "mam02.11a", 0x080000, 0x080000,  CRC(b1fac481) SHA1(da370499ea8ff7b3dd338b31f3799b760fd0d981) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mam08.17d",  0x000000, 0x100000,  CRC(ca97a83f) SHA1(2e097840ae56cf19ad2651d59c31182f47239d60) ) // tiles 1 & 2
	ROM_LOAD( "mam09.18d",  0x100000, 0x100000,  CRC(3f57d56f) SHA1(0d4537da6ab62762179215deae72fe2e6a7869e1) )

	ROM_REGION( 0x600000, "sprites1", 0 )
	ROM_LOAD( "mam05.19a", 0x000000, 0x100000,  CRC(307a2cd1) SHA1(d7a795e47cf1533f0bb5a96162c8025282abe09f) ) // 6bpp sprites
	ROM_LOAD( "mam06.20a", 0x100000, 0x100000,  CRC(a1119a2d) SHA1(876f9295c2032ce491b45a103ffafc750d8c78e1) )
	ROM_LOAD( "mam10.19d", 0x200000, 0x100000,  CRC(99f48f9f) SHA1(685787de54e9158ced80f3821996c3a63f2a72a2) )
	ROM_LOAD( "mam11.20d", 0x300000, 0x100000,  CRC(c3f12859) SHA1(45fdfd55f606316c936f0a9e6b4940740138d344) )
	ROM_LOAD( "mam03.17a", 0x400000, 0x100000,  CRC(fc4dfd48) SHA1(0c5f5a09833ebeb3018e65edd6f7ce06d4ba84ed) )
	ROM_LOAD( "mam04.18a", 0x500000, 0x100000,  CRC(7d3b38bf) SHA1(9f83ad7497ed57405ad648f403eb69f776567a50) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mam12.14p", 0x00000,  0x80000,  CRC(6f00b791) SHA1(c9fbc9ab5ce84fec79efa0a23373be97a27bf898) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mam13.15p", 0x00000,  0x80000,  CRC(525b9461) SHA1(1d9bb3725dfe601b05a779b84b4191455087b969) )

	ROM_REGION( 512, "proms", 0 )
	ROM_LOAD( "hb-00.11p", 0x00000,  0x200,  CRC(b7a7baad) SHA1(39781c3412493b985d3616ac31142fc00bbcddf4) ) // ?
ROM_END

ROM_START( rohga2 ) // Asia/Europe v3.0 Alternate Set
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "hts-00-3.2a",  0x000000, 0x40000, CRC(154f02ec) SHA1(677975cfc542e5a00091330a8096b85465261a4c) )
	ROM_LOAD16_BYTE( "hts-03-3.2d",  0x000001, 0x40000, CRC(5e69d3d8) SHA1(832cad250b42f269786b128b98b05ab2cdb238bb) )
	ROM_LOAD16_BYTE( "mam00.8a",     0x100000, 0x80000, CRC(0fa440a6) SHA1(f0f84c630fc30ec164acc21de871c857d391c398) )
	ROM_LOAD16_BYTE( "mam07.8d",     0x100001, 0x80000, CRC(f8bc7f20) SHA1(909324248bd207f3b01d9f694975b629d8ccaa08) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ha04.18p",  0x00000,  0x10000,  CRC(eb6608eb) SHA1(0233677970aba12783dd4d6d58d70568ef641115) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "ha01.13a",  0x00000,  0x10000,  CRC(fb8f8519) SHA1(0a237426561e5fef6a062e1ad5ae02204f72d5f9) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "ha02.14a",  0x00001,  0x10000,  CRC(aa47c17f) SHA1(830dfcbfaef90133d93b0fbf3cf2067498fa658b) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mam01.10a", 0x000000, 0x080000,  CRC(dbf4fbcc) SHA1(2f289556fd25beb7d30501cba17ac35ad28c5b91) ) // Encrypted tiles
	ROM_LOAD( "mam02.11a", 0x080000, 0x080000,  CRC(b1fac481) SHA1(da370499ea8ff7b3dd338b31f3799b760fd0d981) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mam08.17d",  0x000000, 0x100000,  CRC(ca97a83f) SHA1(2e097840ae56cf19ad2651d59c31182f47239d60) ) // tiles 1 & 2
	ROM_LOAD( "mam09.18d",  0x100000, 0x100000,  CRC(3f57d56f) SHA1(0d4537da6ab62762179215deae72fe2e6a7869e1) )

	ROM_REGION( 0x600000, "sprites1", 0 )
	ROM_LOAD( "mam05.19a", 0x000000, 0x100000,  CRC(307a2cd1) SHA1(d7a795e47cf1533f0bb5a96162c8025282abe09f) ) // 6bpp sprites
	ROM_LOAD( "mam06.20a", 0x100000, 0x100000,  CRC(a1119a2d) SHA1(876f9295c2032ce491b45a103ffafc750d8c78e1) )
	ROM_LOAD( "mam10.19d", 0x200000, 0x100000,  CRC(99f48f9f) SHA1(685787de54e9158ced80f3821996c3a63f2a72a2) )
	ROM_LOAD( "mam11.20d", 0x300000, 0x100000,  CRC(c3f12859) SHA1(45fdfd55f606316c936f0a9e6b4940740138d344) )
	ROM_LOAD( "mam03.17a", 0x400000, 0x100000,  CRC(fc4dfd48) SHA1(0c5f5a09833ebeb3018e65edd6f7ce06d4ba84ed) )
	ROM_LOAD( "mam04.18a", 0x500000, 0x100000,  CRC(7d3b38bf) SHA1(9f83ad7497ed57405ad648f403eb69f776567a50) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mam12.14p", 0x00000,  0x80000,  CRC(6f00b791) SHA1(c9fbc9ab5ce84fec79efa0a23373be97a27bf898) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mam13.15p", 0x00000,  0x80000,  CRC(525b9461) SHA1(1d9bb3725dfe601b05a779b84b4191455087b969) )

	ROM_REGION( 512, "proms", 0 )
	ROM_LOAD( "hb-00.11p", 0x00000,  0x200,  CRC(b7a7baad) SHA1(39781c3412493b985d3616ac31142fc00bbcddf4) ) // ?
ROM_END


/*
PCB No: DE-0353-3
Mask ROM code: MAM
Sound : 45 (HuC6280 CPU), OKI M6295 (x2), YM2151, YM3012
RAM   : 6264 (x4 near 113 + x1 near 18P - 5 total)
Xtals: 32.200MHz, 28.000MHz
DIPSW: 8 position x3
PALs: PAL16L8 x4 labelled TP-00 (near MAM-07), TP-01, TP-02 (both near 68000),
      TP-04 (near 113 and HB-00)
DE ICs:
59 (CPU, 68000)
55, 56 (BG)
52 x2, 71 (Sprites)
104 (I/O+Protection)
113 (Linked to 104, Alpha Blending)

ROMs:
JD00-2.2A  \
JD03-2.2D  / Main Program
HB-00.11P  - PROM located near 113 and 4x 6264 RAMs
MAM-13.15P - Oki Samples, this was missing from existing archive

Sound out is stereo, there is a jumper to select Mono or Stereo output.
*/

ROM_START( rohgah ) // Hong Kong v3.0
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "jd00-2.2a", 0x000000, 0x40000, CRC(ec70646a) SHA1(5e25fe8ce0dfebf8f5903ebe9aa5ef01ca7aa2f0) )
	ROM_LOAD16_BYTE( "jd03-2.2d", 0x000001, 0x40000, CRC(11d4c9a2) SHA1(9afe684d749665f65e44a3665d5a1dc61458faa0) )
	ROM_LOAD16_BYTE( "mam00.8a",  0x100000, 0x80000, CRC(0fa440a6) SHA1(f0f84c630fc30ec164acc21de871c857d391c398) )
	ROM_LOAD16_BYTE( "mam07.8d",  0x100001, 0x80000, CRC(f8bc7f20) SHA1(909324248bd207f3b01d9f694975b629d8ccaa08) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ha04.18p",  0x00000,  0x10000,  CRC(eb6608eb) SHA1(0233677970aba12783dd4d6d58d70568ef641115) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "ha01.13a",  0x00000,  0x10000,  CRC(fb8f8519) SHA1(0a237426561e5fef6a062e1ad5ae02204f72d5f9) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "ha02.14a",  0x00001,  0x10000,  CRC(aa47c17f) SHA1(830dfcbfaef90133d93b0fbf3cf2067498fa658b) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mam01.10a", 0x000000, 0x080000,  CRC(dbf4fbcc) SHA1(2f289556fd25beb7d30501cba17ac35ad28c5b91) ) // Encrypted tiles
	ROM_LOAD( "mam02.11a", 0x080000, 0x080000,  CRC(b1fac481) SHA1(da370499ea8ff7b3dd338b31f3799b760fd0d981) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mam08.17d",  0x000000, 0x100000,  CRC(ca97a83f) SHA1(2e097840ae56cf19ad2651d59c31182f47239d60) ) // tiles 1 & 2
	ROM_LOAD( "mam09.18d",  0x100000, 0x100000,  CRC(3f57d56f) SHA1(0d4537da6ab62762179215deae72fe2e6a7869e1) )

	ROM_REGION( 0x600000, "sprites1", 0 )
	ROM_LOAD( "mam05.19a", 0x000000, 0x100000,  CRC(307a2cd1) SHA1(d7a795e47cf1533f0bb5a96162c8025282abe09f) ) // 6bpp sprites
	ROM_LOAD( "mam06.20a", 0x100000, 0x100000,  CRC(a1119a2d) SHA1(876f9295c2032ce491b45a103ffafc750d8c78e1) )
	ROM_LOAD( "mam10.19d", 0x200000, 0x100000,  CRC(99f48f9f) SHA1(685787de54e9158ced80f3821996c3a63f2a72a2) )
	ROM_LOAD( "mam11.20d", 0x300000, 0x100000,  CRC(c3f12859) SHA1(45fdfd55f606316c936f0a9e6b4940740138d344) )
	ROM_LOAD( "mam03.17a", 0x400000, 0x100000,  CRC(fc4dfd48) SHA1(0c5f5a09833ebeb3018e65edd6f7ce06d4ba84ed) )
	ROM_LOAD( "mam04.18a", 0x500000, 0x100000,  CRC(7d3b38bf) SHA1(9f83ad7497ed57405ad648f403eb69f776567a50) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mam12.14p", 0x00000,  0x80000,  CRC(6f00b791) SHA1(c9fbc9ab5ce84fec79efa0a23373be97a27bf898) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mam13.15p", 0x00000,  0x80000,  CRC(525b9461) SHA1(1d9bb3725dfe601b05a779b84b4191455087b969) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "hb-00.11p", 0x00000,  0x200,  CRC(b7a7baad) SHA1(39781c3412493b985d3616ac31142fc00bbcddf4) ) // ?
ROM_END

ROM_START( rohgau ) // US v1.0
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ha00.2a",  0x000000, 0x40000, CRC(d8d13052) SHA1(24113244200f15a16fed82c64de3e9e4e87d1257) )
	ROM_LOAD16_BYTE( "ha03.2d",  0x000001, 0x40000, CRC(5f683bbf) SHA1(a367b833fd1f64bff9618ce06be22aed218d4225) )
	ROM_LOAD16_BYTE( "mam00.8a",  0x100000, 0x80000, CRC(0fa440a6) SHA1(f0f84c630fc30ec164acc21de871c857d391c398) )
	ROM_LOAD16_BYTE( "mam07.8d",  0x100001, 0x80000, CRC(f8bc7f20) SHA1(909324248bd207f3b01d9f694975b629d8ccaa08) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ha04.18p",  0x00000,  0x10000,  CRC(eb6608eb) SHA1(0233677970aba12783dd4d6d58d70568ef641115) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "ha01.13a",  0x00000,  0x10000,  CRC(fb8f8519) SHA1(0a237426561e5fef6a062e1ad5ae02204f72d5f9) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "ha02.14a",  0x00001,  0x10000,  CRC(aa47c17f) SHA1(830dfcbfaef90133d93b0fbf3cf2067498fa658b) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mam01.10a", 0x000000, 0x080000,  CRC(dbf4fbcc) SHA1(2f289556fd25beb7d30501cba17ac35ad28c5b91) ) // Encrypted tiles
	ROM_LOAD( "mam02.11a", 0x080000, 0x080000,  CRC(b1fac481) SHA1(da370499ea8ff7b3dd338b31f3799b760fd0d981) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mam08.17d",  0x000000, 0x100000,  CRC(ca97a83f) SHA1(2e097840ae56cf19ad2651d59c31182f47239d60) ) // tiles 1 & 2
	ROM_LOAD( "mam09.18d",  0x100000, 0x100000,  CRC(3f57d56f) SHA1(0d4537da6ab62762179215deae72fe2e6a7869e1) )

	ROM_REGION( 0x600000, "sprites1", 0 )
	ROM_LOAD( "mam05.19a", 0x000000, 0x100000,  CRC(307a2cd1) SHA1(d7a795e47cf1533f0bb5a96162c8025282abe09f) ) // 6bpp sprites
	ROM_LOAD( "mam06.20a", 0x100000, 0x100000,  CRC(a1119a2d) SHA1(876f9295c2032ce491b45a103ffafc750d8c78e1) )
	ROM_LOAD( "mam10.19d", 0x200000, 0x100000,  CRC(99f48f9f) SHA1(685787de54e9158ced80f3821996c3a63f2a72a2) )
	ROM_LOAD( "mam11.20d", 0x300000, 0x100000,  CRC(c3f12859) SHA1(45fdfd55f606316c936f0a9e6b4940740138d344) )
	ROM_LOAD( "mam03.17a", 0x400000, 0x100000,  CRC(fc4dfd48) SHA1(0c5f5a09833ebeb3018e65edd6f7ce06d4ba84ed) )
	ROM_LOAD( "mam04.18a", 0x500000, 0x100000,  CRC(7d3b38bf) SHA1(9f83ad7497ed57405ad648f403eb69f776567a50) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mam12.14p", 0x00000,  0x80000,  CRC(6f00b791) SHA1(c9fbc9ab5ce84fec79efa0a23373be97a27bf898) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mam13.15p", 0x00000,  0x80000,  CRC(525b9461) SHA1(1d9bb3725dfe601b05a779b84b4191455087b969) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "hb-00.11p", 0x00000,  0x200,  CRC(b7a7baad) SHA1(39781c3412493b985d3616ac31142fc00bbcddf4) ) // ?
ROM_END

ROM_START( wolffang ) // Japan
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "hw_00-1.2a",  0x000000, 0x40000, CRC(69dc611e) SHA1(37fc54df32cc8e0502b59d33d94a805dbd7018c5) )
	ROM_LOAD16_BYTE( "hw_03-1.2d",  0x000001, 0x40000, CRC(b66d9680) SHA1(6bb97a720cc8a5df4051d8ae55586baf03cdc6bc) )
	ROM_LOAD16_BYTE( "mam00.8a",  0x100000, 0x80000, CRC(0fa440a6) SHA1(f0f84c630fc30ec164acc21de871c857d391c398) )
	ROM_LOAD16_BYTE( "mam07.8d",  0x100001, 0x80000, CRC(f8bc7f20) SHA1(909324248bd207f3b01d9f694975b629d8ccaa08) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hw_04-.18p",  0x00000,  0x10000,  CRC(eb6608eb) SHA1(0233677970aba12783dd4d6d58d70568ef641115) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "hw_01-.13a",  0x00000,  0x10000,  CRC(d9810ca4) SHA1(f8d85c93eaf8e26f115afff6193617caa864a6b7) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "hw_02-.14a",  0x00001,  0x10000,  CRC(2a27ac8e) SHA1(9ed752cc212d29a621226deee79e62585232c923) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mam01.10a", 0x000000, 0x080000,  CRC(dbf4fbcc) SHA1(2f289556fd25beb7d30501cba17ac35ad28c5b91) ) // Encrypted tiles
	ROM_LOAD( "mam02.11a", 0x080000, 0x080000,  CRC(b1fac481) SHA1(da370499ea8ff7b3dd338b31f3799b760fd0d981) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mam08.17d",  0x000000, 0x100000,  CRC(ca97a83f) SHA1(2e097840ae56cf19ad2651d59c31182f47239d60) ) // tiles 1 & 2
	ROM_LOAD( "mam09.18d",  0x100000, 0x100000,  CRC(3f57d56f) SHA1(0d4537da6ab62762179215deae72fe2e6a7869e1) )

	ROM_REGION( 0x600000, "sprites1", 0 )
	ROM_LOAD( "mam05.19a", 0x000000, 0x100000,  CRC(307a2cd1) SHA1(d7a795e47cf1533f0bb5a96162c8025282abe09f) ) // 6bpp sprites
	ROM_LOAD( "mam06.20a", 0x100000, 0x100000,  CRC(a1119a2d) SHA1(876f9295c2032ce491b45a103ffafc750d8c78e1) )
	ROM_LOAD( "mam10.19d", 0x200000, 0x100000,  CRC(99f48f9f) SHA1(685787de54e9158ced80f3821996c3a63f2a72a2) )
	ROM_LOAD( "mam11.20d", 0x300000, 0x100000,  CRC(c3f12859) SHA1(45fdfd55f606316c936f0a9e6b4940740138d344) )
	ROM_LOAD( "mam03.17a", 0x400000, 0x100000,  CRC(fc4dfd48) SHA1(0c5f5a09833ebeb3018e65edd6f7ce06d4ba84ed) )
	ROM_LOAD( "mam04.18a", 0x500000, 0x100000,  CRC(7d3b38bf) SHA1(9f83ad7497ed57405ad648f403eb69f776567a50) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mam12.14p", 0x00000,  0x80000,  CRC(6f00b791) SHA1(c9fbc9ab5ce84fec79efa0a23373be97a27bf898) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mam13.15p", 0x00000,  0x80000,  CRC(525b9461) SHA1(1d9bb3725dfe601b05a779b84b4191455087b969) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "hb-00.11p", 0x00000,  0x200,  CRC(b7a7baad) SHA1(39781c3412493b985d3616ac31142fc00bbcddf4) ) // ?
ROM_END


ROM_START( wizdfire )
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "je-01.3d", 0x000000, 0x20000, CRC(b6d62367) SHA1(e9521b28660f62b70e6e33c3e9cf345fc106eff6) ) // Version 2.1 Over Sea
	ROM_LOAD16_BYTE( "je-00.3a", 0x000001, 0x20000, CRC(f33de278) SHA1(9407a73f578ec312944fc69f247e83704d713174) )
	ROM_LOAD16_BYTE( "je-03.5d", 0x040000, 0x20000, CRC(5217d404) SHA1(7cfcdb9e2c812bf0d4ac8306834242876ac47844) )
	ROM_LOAD16_BYTE( "je-02.5a", 0x040001, 0x20000, CRC(36a1ce28) SHA1(62d52d720c89022de97759777230c45c460d8fb6) )
	ROM_LOAD16_BYTE( "mas13",    0x080000, 0x80000, CRC(7e5256ce) SHA1(431d78ad185ba0216097f131fb2583a1a067e4f0) )
	ROM_LOAD16_BYTE( "mas12",    0x080001, 0x80000, CRC(005bd499) SHA1(862079022f97bd11f2f33677dce55bd3b144a81b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "je-06.20r",  0x00000,  0x10000,  CRC(79042546) SHA1(231561df9415a289756a533709f610894fb9176e) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "je-04.10d",  0x00000,  0x10000,  CRC(73cba800) SHA1(dd7612fe1482713fcee5960b7db158be872d7fda) ) // Chars
	ROM_LOAD16_BYTE( "je-05.12d",  0x00001,  0x10000,  CRC(22e2c49d) SHA1(06cc2d0476156d1f521c4c57621ce3922a23aa04) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mas00", 0x000000, 0x100000,  CRC(3d011034) SHA1(167d6d088d51a41f196be104d795ffe24297c96a) ) // Tiles
	ROM_LOAD( "mas01", 0x100000, 0x100000,  CRC(6d0c9d0b) SHA1(63e19dfd6451810637664b08e880aef139ca6ed5) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mas02", 0x000000, 0x080000,  CRC(af00e620) SHA1(43f4680b22ac6baf840274462c07fee68a2fbdfb) )
	ROM_LOAD( "mas03", 0x080000, 0x080000,  CRC(2fe61ea2) SHA1(0909e6c689c3e10225d7c074bd654ff2ada96983) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mas04", 0x000000, 0x100000,  CRC(1e56953b) SHA1(0655ac7f3c5030a80c2d6bad5c3a79b2cb1ae4a2) ) // Sprites #1
	ROM_LOAD( "mas05", 0x200000, 0x100000,  CRC(3826b8f8) SHA1(d59197b4e0525b86876f9cce6fbf80caba976851) )
	ROM_LOAD( "mas06", 0x100000, 0x100000,  CRC(3b8bbd45) SHA1(c9f9d4daf9c0cba5385af26f3762b29c291ff62b) )
	ROM_LOAD( "mas07", 0x300000, 0x100000,  CRC(31303769) SHA1(509604be06ec8e0c1b56a81a8ffccdf0f79e9fd7) )

	ROM_REGION( 0x100000, "sprites2", 0 )
	ROM_LOAD( "mas08", 0x000000, 0x080000,  CRC(e224fb7a) SHA1(9aa92fb98bddff313db2077c4db102e94c7af09b) ) // Sprites #2
	ROM_LOAD( "mas09", 0x080000, 0x080000,  CRC(5f6deb41) SHA1(850d0e157b4355e866ec770a2012293b2c55648f) )

	ROM_REGION(0x100000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mas10",  0x80000,  0x80000,  CRC(f4b4c8a1) SHA1(c9e80c55e42a78e358b6b14dadc3be7b28bd5d62) )
	ROM_CONTINUE(0x00000, 0x80000)

	ROM_REGION(0x080000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mas11",  0x00000,  0x080000,  CRC(c2f0a4f2) SHA1(af71d649aea273c17d7fbcf8693e8a1d4b31f7f8) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "mb7122h.16l", 0x00000,  0x400,  CRC(2bee57cc) SHA1(bc48670aa7c39f6ff7fae4c819eab22ed2db875b) )   // Priority (unused)
ROM_END

ROM_START( wizdfirea )
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mm-01.3d", 0x000000, 0x20000, CRC(ebaecff7) SHA1(965bc57e530e4143932ce7c2fd1e338feee1bbcd) ) // Version 1.0 Over Sea
	ROM_LOAD16_BYTE( "mm-00.3a", 0x000001, 0x20000, CRC(56e05ec0) SHA1(d18cca32a6ebc20aeb3d6aa5d183766491302fcf) )
	ROM_LOAD16_BYTE( "je-03.5d", 0x040000, 0x20000, CRC(5217d404) SHA1(7cfcdb9e2c812bf0d4ac8306834242876ac47844) )
	ROM_LOAD16_BYTE( "je-02.5a", 0x040001, 0x20000, CRC(36a1ce28) SHA1(62d52d720c89022de97759777230c45c460d8fb6) )
	ROM_LOAD16_BYTE( "mas13",    0x080000, 0x80000, CRC(7e5256ce) SHA1(431d78ad185ba0216097f131fb2583a1a067e4f0) )
	ROM_LOAD16_BYTE( "mas12",    0x080001, 0x80000, CRC(005bd499) SHA1(862079022f97bd11f2f33677dce55bd3b144a81b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "je-06.20r",  0x00000,  0x10000, CRC(79042546) SHA1(231561df9415a289756a533709f610894fb9176e) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "je-04.10d", 0x00000, 0x10000, CRC(73cba800) SHA1(dd7612fe1482713fcee5960b7db158be872d7fda) ) // Chars
	ROM_LOAD16_BYTE( "je-05.12d", 0x00001, 0x10000, CRC(22e2c49d) SHA1(06cc2d0476156d1f521c4c57621ce3922a23aa04) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mas00", 0x000000, 0x100000, CRC(3d011034) SHA1(167d6d088d51a41f196be104d795ffe24297c96a) ) // Tiles
	ROM_LOAD( "mas01", 0x100000, 0x100000, CRC(6d0c9d0b) SHA1(63e19dfd6451810637664b08e880aef139ca6ed5) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mas02", 0x000000, 0x080000, CRC(af00e620) SHA1(43f4680b22ac6baf840274462c07fee68a2fbdfb) )
	ROM_LOAD( "mas03", 0x080000, 0x080000, CRC(2fe61ea2) SHA1(0909e6c689c3e10225d7c074bd654ff2ada96983) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mas04", 0x000000, 0x100000, CRC(1e56953b) SHA1(0655ac7f3c5030a80c2d6bad5c3a79b2cb1ae4a2) ) // Sprites #1
	ROM_LOAD( "mas05", 0x200000, 0x100000, CRC(3826b8f8) SHA1(d59197b4e0525b86876f9cce6fbf80caba976851) )
	ROM_LOAD( "mas06", 0x100000, 0x100000, CRC(3b8bbd45) SHA1(c9f9d4daf9c0cba5385af26f3762b29c291ff62b) )
	ROM_LOAD( "mas07", 0x300000, 0x100000, CRC(31303769) SHA1(509604be06ec8e0c1b56a81a8ffccdf0f79e9fd7) )

	ROM_REGION( 0x100000, "sprites2", 0 )
	ROM_LOAD( "mas08", 0x000000, 0x080000, CRC(e224fb7a) SHA1(9aa92fb98bddff313db2077c4db102e94c7af09b) ) // Sprites #2
	ROM_LOAD( "mas09", 0x080000, 0x080000, CRC(5f6deb41) SHA1(850d0e157b4355e866ec770a2012293b2c55648f) )

	ROM_REGION(0x100000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mas10",  0x80000,  0x80000, CRC(f4b4c8a1) SHA1(c9e80c55e42a78e358b6b14dadc3be7b28bd5d62) )
	ROM_CONTINUE(0x00000, 0x80000)

	ROM_REGION(0x080000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mas11",  0x00000,  0x080000, CRC(c2f0a4f2) SHA1(af71d649aea273c17d7fbcf8693e8a1d4b31f7f8) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "mb7122h.16l", 0x000, 0x400, CRC(2bee57cc) SHA1(bc48670aa7c39f6ff7fae4c819eab22ed2db875b) ) // Priority (unused)
ROM_END

ROM_START( wizdfireu )
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "jf-01.3d", 0x000000, 0x20000, CRC(bde42a41) SHA1(0379de9c4cdcce35554b5dc15241ed2c4f0d7611) ) // Version 1.1 US
	ROM_LOAD16_BYTE( "jf-00.3a", 0x000001, 0x20000, CRC(bca3c995) SHA1(dbebc9e301c04ee82ca4b658d3ab870790d1605b) )
	ROM_LOAD16_BYTE( "jf-03.5d", 0x040000, 0x20000, CRC(5217d404) SHA1(7cfcdb9e2c812bf0d4ac8306834242876ac47844) )
	ROM_LOAD16_BYTE( "jf-02.5a", 0x040001, 0x20000, CRC(36a1ce28) SHA1(62d52d720c89022de97759777230c45c460d8fb6) )
	ROM_LOAD16_BYTE( "mas13",    0x080000, 0x80000, CRC(7e5256ce) SHA1(431d78ad185ba0216097f131fb2583a1a067e4f0) )
	ROM_LOAD16_BYTE( "mas12",    0x080001, 0x80000, CRC(005bd499) SHA1(862079022f97bd11f2f33677dce55bd3b144a81b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "jf-06.20r",  0x00000,  0x10000,  CRC(79042546) SHA1(231561df9415a289756a533709f610894fb9176e) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "jf-04.10d",  0x00000,  0x10000,  CRC(73cba800) SHA1(dd7612fe1482713fcee5960b7db158be872d7fda) ) // Chars
	ROM_LOAD16_BYTE( "jf-05.12d",  0x00001,  0x10000,  CRC(22e2c49d) SHA1(06cc2d0476156d1f521c4c57621ce3922a23aa04) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mas00", 0x000000, 0x100000,  CRC(3d011034) SHA1(167d6d088d51a41f196be104d795ffe24297c96a) ) // Tiles
	ROM_LOAD( "mas01", 0x100000, 0x100000,  CRC(6d0c9d0b) SHA1(63e19dfd6451810637664b08e880aef139ca6ed5) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mas02", 0x000000, 0x080000,  CRC(af00e620) SHA1(43f4680b22ac6baf840274462c07fee68a2fbdfb) )
	ROM_LOAD( "mas03", 0x080000, 0x080000,  CRC(2fe61ea2) SHA1(0909e6c689c3e10225d7c074bd654ff2ada96983) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mas04", 0x000000, 0x100000,  CRC(1e56953b) SHA1(0655ac7f3c5030a80c2d6bad5c3a79b2cb1ae4a2) ) // Sprites #1
	ROM_LOAD( "mas05", 0x200000, 0x100000,  CRC(3826b8f8) SHA1(d59197b4e0525b86876f9cce6fbf80caba976851) )
	ROM_LOAD( "mas06", 0x100000, 0x100000,  CRC(3b8bbd45) SHA1(c9f9d4daf9c0cba5385af26f3762b29c291ff62b) )
	ROM_LOAD( "mas07", 0x300000, 0x100000,  CRC(31303769) SHA1(509604be06ec8e0c1b56a81a8ffccdf0f79e9fd7) )

	ROM_REGION( 0x100000, "sprites2", 0 )
	ROM_LOAD( "mas08", 0x000000, 0x080000,  CRC(e224fb7a) SHA1(9aa92fb98bddff313db2077c4db102e94c7af09b) ) // Sprites #2
	ROM_LOAD( "mas09", 0x080000, 0x080000,  CRC(5f6deb41) SHA1(850d0e157b4355e866ec770a2012293b2c55648f) )

	ROM_REGION(0x100000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mas10",  0x80000,  0x80000,  CRC(f4b4c8a1) SHA1(c9e80c55e42a78e358b6b14dadc3be7b28bd5d62) )
	ROM_CONTINUE(0x00000, 0x80000)

	ROM_REGION(0x080000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mas11",  0x00000,  0x080000,  CRC(c2f0a4f2) SHA1(af71d649aea273c17d7fbcf8693e8a1d4b31f7f8) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "mb7122h.16l", 0x00000,  0x400,  CRC(2bee57cc) SHA1(bc48670aa7c39f6ff7fae4c819eab22ed2db875b) )   // Priority (unused)
ROM_END

ROM_START( darkseal2 )
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "jb-01-3.3d", 0x000000, 0x20000, CRC(82308c01) SHA1(aa0733e244f14f2c84b6929236771cbc99532bb2) ) // Version 2.1 Japan
	ROM_LOAD16_BYTE( "jb-00-3.3a", 0x000001, 0x20000, CRC(1d38113a) SHA1(69dc5a4dbe9d9737df198240f3db6f2115e311a5) )
	ROM_LOAD16_BYTE( "jb-03.5d",   0x040000, 0x20000, CRC(5217d404) SHA1(7cfcdb9e2c812bf0d4ac8306834242876ac47844) )
	ROM_LOAD16_BYTE( "jb-02.5a",   0x040001, 0x20000, CRC(36a1ce28) SHA1(62d52d720c89022de97759777230c45c460d8fb6) )
	ROM_LOAD16_BYTE( "mas13",      0x080000, 0x80000, CRC(7e5256ce) SHA1(431d78ad185ba0216097f131fb2583a1a067e4f0) )
	ROM_LOAD16_BYTE( "mas12",      0x080001, 0x80000, CRC(005bd499) SHA1(862079022f97bd11f2f33677dce55bd3b144a81b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "jb-06.20r",  0x00000,  0x10000,  CRC(2066a1dd) SHA1(a0d136e90825fa9c089894a6852c634676d64579) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "jb-04.10d",  0x00000,  0x10000,  CRC(73cba800) SHA1(dd7612fe1482713fcee5960b7db158be872d7fda) ) // Chars
	ROM_LOAD16_BYTE( "jb-05.12d",  0x00001,  0x10000,  CRC(22e2c49d) SHA1(06cc2d0476156d1f521c4c57621ce3922a23aa04) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mas00", 0x000000, 0x100000,  CRC(3d011034) SHA1(167d6d088d51a41f196be104d795ffe24297c96a) ) // Tiles
	ROM_LOAD( "mas01", 0x100000, 0x100000,  CRC(6d0c9d0b) SHA1(63e19dfd6451810637664b08e880aef139ca6ed5) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mas02", 0x000000, 0x080000,  CRC(af00e620) SHA1(43f4680b22ac6baf840274462c07fee68a2fbdfb) )
	ROM_LOAD( "mas03", 0x080000, 0x080000,  CRC(2fe61ea2) SHA1(0909e6c689c3e10225d7c074bd654ff2ada96983) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mas04", 0x000000, 0x100000,  CRC(1e56953b) SHA1(0655ac7f3c5030a80c2d6bad5c3a79b2cb1ae4a2) ) // Sprites #1
	ROM_LOAD( "mas05", 0x200000, 0x100000,  CRC(3826b8f8) SHA1(d59197b4e0525b86876f9cce6fbf80caba976851) )
	ROM_LOAD( "mas06", 0x100000, 0x100000,  CRC(3b8bbd45) SHA1(c9f9d4daf9c0cba5385af26f3762b29c291ff62b) )
	ROM_LOAD( "mas07", 0x300000, 0x100000,  CRC(31303769) SHA1(509604be06ec8e0c1b56a81a8ffccdf0f79e9fd7) )

	ROM_REGION( 0x100000, "sprites2", 0 )
	ROM_LOAD( "mas08", 0x000000, 0x080000,  CRC(e224fb7a) SHA1(9aa92fb98bddff313db2077c4db102e94c7af09b) ) // Sprites #2
	ROM_LOAD( "mas09", 0x080000, 0x080000,  CRC(5f6deb41) SHA1(850d0e157b4355e866ec770a2012293b2c55648f) )

	ROM_REGION(0x100000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mas10",  0x00000,  0x100000,  CRC(f4b4c8a1) SHA1(c9e80c55e42a78e358b6b14dadc3be7b28bd5d62) )

	ROM_REGION(0x080000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mas11",  0x00000,  0x080000,  CRC(c2f0a4f2) SHA1(af71d649aea273c17d7fbcf8693e8a1d4b31f7f8) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "mb7122h.16l", 0x00000,  0x400,  CRC(2bee57cc) SHA1(bc48670aa7c39f6ff7fae4c819eab22ed2db875b) ) // Priority (unused)
ROM_END


ROM_START( nitrobal )
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "jl01-4.d3",   0x000000, 0x20000, CRC(0414e409) SHA1(bc19e7d2d9e768ce4052511043867c0ef9b0b61b) )
	ROM_LOAD16_BYTE( "jl00-4.b3",   0x000001, 0x20000, CRC(dd9e2bcc) SHA1(dede49a4fafcfa03f38ba6c1149c9f8b115fb306) )
	ROM_LOAD16_BYTE( "jl03-4.d5",   0x040000, 0x20000, CRC(ea264ac5) SHA1(ccdb87bbdd9e38537dd290d237d76ec32559efa3) )
	ROM_LOAD16_BYTE( "jl02-4.b5",   0x040001, 0x20000, CRC(74047997) SHA1(bfd2f24889250e06945bb4798b40a56f832a9b19) )
	ROM_LOAD16_BYTE( "jl05-2.d6",   0x080000, 0x40000, CRC(b820fa20) SHA1(8509567cf988fe27552d37241b25b66a6e1a9c39) )
	ROM_LOAD16_BYTE( "jl04-2.b6",   0x080001, 0x40000, CRC(1fd8995b) SHA1(75d77835500e4b7caca92ba634859d7a2ad9b84c) )
	// Two empty ROM slots at d7, b7

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "jl08.r20",  0x00000,  0x10000,  CRC(93d93fe1) SHA1(efc618724251d23a23b3019d475f7739a7e88751) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "jl06.d10",  0x00000,  0x10000,  CRC(91cf668e) SHA1(fc153eaa09777f79369037a139470ad1118e8d7e) ) // Chars
	ROM_LOAD16_BYTE( "jl07.d12",  0x00001,  0x10000,  CRC(e61d0e42) SHA1(80d6ada356c721b0be826554ec6731dbbc19e0ab) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mav00.b10", 0x00000, 0x80000,  CRC(34785d97) SHA1(094f881cd699d1b9fd079778f20f8c9d83283e6e) ) // Tiles
	ROM_LOAD( "mav01.b12", 0x80000, 0x80000,  CRC(8b531b16) SHA1(f734286f4510b2c09dc2d6d2b8c8da9dc4424287) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mav02.b16", 0x000000, 0x100000,  CRC(20723bf7) SHA1(b3491d98ff415701fec2b58d85f99c743d71b013) ) // Tiles
	ROM_LOAD( "mav03.e16", 0x100000, 0x100000,  CRC(ef6195f0) SHA1(491bc030519c78b84396f7f8a21df9daf8acc140) )

	ROM_REGION( 0x300000, "sprites1", 0 )
	ROM_LOAD( "mav04.b19", 0x000000, 0x100000,  CRC(8ba48385) SHA1(926ae1e0e99b8e022b6798ceb29dd080cfc1bada) ) // Sprites #1
	ROM_LOAD( "mav05.e19", 0x180000, 0x100000,  CRC(d92d769c) SHA1(8012e7f2b9a7cbccde8da90025647443beb6c47c) )
	ROM_LOAD( "mav06.b20", 0x100000, 0x080000,  CRC(ae6201a5) SHA1(c0ae87fa96d12377c5522cb8adfed03373ab3757) )
	ROM_LOAD( "mav07.e20", 0x280000, 0x080000,  CRC(5fc10ccd) SHA1(7debcf223802d5c2ea3d29d39850c8756c863b31) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD( "mav08.b23", 0x000000, 0x040000,  CRC(64966576) SHA1(40c14c0f62eef0317abfb7192505e0337fb5cde5) ) // Sprites #2
	ROM_LOAD( "mav09.e23", 0x040000, 0x040000,  CRC(1ce7b51a) SHA1(17ed8f34bf6d057e0504e72e95f448d5923aa82e) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mav10.r17",  0x00000,  0x80000,  CRC(8ad734b0) SHA1(768b9f54bbf4b54591cafecb7a27960da919ce84) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mav11.r19",  0x00000,  0x80000,  CRC(ef513908) SHA1(72db6c704071d7a784b3768c256fc51087e9e93c) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "jn-00.17l", 0x00000,  0x400,  CRC(6ac77b84) SHA1(9e1e2cabdb20b819e592a0f07d15658062227fa4) ) // Priority (unused)
ROM_END

ROM_START( nitrobala )
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code // roms had no labels
	ROM_LOAD16_BYTE( "3d",   0x000000, 0x20000, CRC(48f77c19) SHA1(d6d4f10246c6f098503a0c126c1d8e3155063c32) )
	ROM_LOAD16_BYTE( "3b",   0x000001, 0x20000, CRC(fb1284e9) SHA1(13e9852011965b1cf12d8568d043aec92f6df2cd) )
	ROM_LOAD16_BYTE( "5d",   0x040000, 0x20000, CRC(ac47367a) SHA1(f0efe6860ca83e7506491b62bc0ad62fa4c2ed1c) )
	ROM_LOAD16_BYTE( "5b",   0x040001, 0x20000, CRC(a8e9d7dd) SHA1(19b4ed6501c44c9ac0fdbbfe4dec65efec4b700f) )
	ROM_LOAD16_BYTE( "6d",   0x080000, 0x40000, CRC(59e620cc) SHA1(60d4e9622008a32c2addf12a266817cc723635ab) ) // 7ee3 - 7f06 are 0xff instead of 0x00 in the nitrobal set, why?
	ROM_LOAD16_BYTE( "6b",   0x080001, 0x40000, CRC(1fd8995b) SHA1(75d77835500e4b7caca92ba634859d7a2ad9b84c) ) // this half of the pair matches
	// Two empty ROM slots at d7, b7

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "jl08.r20",  0x00000,  0x10000,  CRC(93d93fe1) SHA1(efc618724251d23a23b3019d475f7739a7e88751) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "jl06.d10",  0x00000,  0x10000,  CRC(91cf668e) SHA1(fc153eaa09777f79369037a139470ad1118e8d7e) ) // Chars
	ROM_LOAD16_BYTE( "jl07.d12",  0x00001,  0x10000,  CRC(e61d0e42) SHA1(80d6ada356c721b0be826554ec6731dbbc19e0ab) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mav00.b10", 0x00000, 0x80000,  CRC(34785d97) SHA1(094f881cd699d1b9fd079778f20f8c9d83283e6e) ) // Tiles
	ROM_LOAD( "mav01.b12", 0x80000, 0x80000,  CRC(8b531b16) SHA1(f734286f4510b2c09dc2d6d2b8c8da9dc4424287) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mav02.b16", 0x000000, 0x100000,  CRC(20723bf7) SHA1(b3491d98ff415701fec2b58d85f99c743d71b013) ) // Tiles
	ROM_LOAD( "mav03.e16", 0x100000, 0x100000,  CRC(ef6195f0) SHA1(491bc030519c78b84396f7f8a21df9daf8acc140) )

	ROM_REGION( 0x300000, "sprites1", 0 )
	ROM_LOAD( "mav04.b19", 0x000000, 0x100000,  CRC(8ba48385) SHA1(926ae1e0e99b8e022b6798ceb29dd080cfc1bada) ) // Sprites #1
	ROM_LOAD( "mav05.e19", 0x180000, 0x100000,  CRC(d92d769c) SHA1(8012e7f2b9a7cbccde8da90025647443beb6c47c) )
	ROM_LOAD( "mav06.b20", 0x100000, 0x080000,  CRC(ae6201a5) SHA1(c0ae87fa96d12377c5522cb8adfed03373ab3757) )
	ROM_LOAD( "mav07.e20", 0x280000, 0x080000,  CRC(5fc10ccd) SHA1(7debcf223802d5c2ea3d29d39850c8756c863b31) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD( "mav08.b23", 0x000000, 0x040000,  CRC(64966576) SHA1(40c14c0f62eef0317abfb7192505e0337fb5cde5) ) // Sprites #2
	ROM_LOAD( "mav09.e23", 0x040000, 0x040000,  CRC(1ce7b51a) SHA1(17ed8f34bf6d057e0504e72e95f448d5923aa82e) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mav10.r17",  0x00000,  0x80000,  CRC(8ad734b0) SHA1(768b9f54bbf4b54591cafecb7a27960da919ce84) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mav11.r19",  0x00000,  0x80000,  CRC(ef513908) SHA1(72db6c704071d7a784b3768c256fc51087e9e93c) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "jn-00.17l", 0x00000,  0x400,  CRC(6ac77b84) SHA1(9e1e2cabdb20b819e592a0f07d15658062227fa4) ) // Priority (unused)
ROM_END

ROM_START( nitrobalb )
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mm-01.d3",   0x000000, 0x20000, CRC(81ca7ef0) SHA1(7090dc77eaee57805f723ef1c21e245d8c9e5c20) )
	ROM_LOAD16_BYTE( "mm-00.b3",   0x000001, 0x20000, CRC(e3416c4f) SHA1(8cbe6e1d5248c85112d7059aeecc1e185fdd77d7) )
	ROM_LOAD16_BYTE( "mm-03.d5",   0x040000, 0x20000, CRC(ed9792aa) SHA1(b540a828b5d7f4859e2b927b8ca843f0462d0b0b) )
	ROM_LOAD16_BYTE( "mm-02.b5",   0x040001, 0x20000, CRC(9a568ed3) SHA1(acc254af3c9079eb91af52fcfda18c1cfadfd1ce) )
	ROM_LOAD16_BYTE( "mm-05.d6",   0x080000, 0x40000, CRC(b820fa20) SHA1(8509567cf988fe27552d37241b25b66a6e1a9c39) )
	ROM_LOAD16_BYTE( "mm-04.b6",   0x080001, 0x40000, CRC(1fd8995b) SHA1(75d77835500e4b7caca92ba634859d7a2ad9b84c) )
	// Two empty ROM slots at d7, b7

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "mm-08.r20",  0x00000,  0x10000,  CRC(93d93fe1) SHA1(efc618724251d23a23b3019d475f7739a7e88751) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "mm-06.d10",  0x00000,  0x10000,  CRC(91cf668e) SHA1(fc153eaa09777f79369037a139470ad1118e8d7e) ) // Chars
	ROM_LOAD16_BYTE( "mm-07.d12",  0x00001,  0x10000,  CRC(e61d0e42) SHA1(80d6ada356c721b0be826554ec6731dbbc19e0ab) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mav00.b10", 0x00000, 0x80000,  CRC(34785d97) SHA1(094f881cd699d1b9fd079778f20f8c9d83283e6e) ) // Tiles
	ROM_LOAD( "mav01.b12", 0x80000, 0x80000,  CRC(8b531b16) SHA1(f734286f4510b2c09dc2d6d2b8c8da9dc4424287) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mav02.b16", 0x000000, 0x100000,  CRC(20723bf7) SHA1(b3491d98ff415701fec2b58d85f99c743d71b013) ) // Tiles
	ROM_LOAD( "mav03.e16", 0x100000, 0x100000,  CRC(ef6195f0) SHA1(491bc030519c78b84396f7f8a21df9daf8acc140) )

	ROM_REGION( 0x300000, "sprites1", 0 )
	ROM_LOAD( "mav04.b19", 0x000000, 0x100000,  CRC(8ba48385) SHA1(926ae1e0e99b8e022b6798ceb29dd080cfc1bada) ) // Sprites #1
	ROM_LOAD( "mav05.e19", 0x180000, 0x100000,  CRC(d92d769c) SHA1(8012e7f2b9a7cbccde8da90025647443beb6c47c) )
	ROM_LOAD( "mav06.b20", 0x100000, 0x080000,  CRC(ae6201a5) SHA1(c0ae87fa96d12377c5522cb8adfed03373ab3757) )
	ROM_LOAD( "mav07.e20", 0x280000, 0x080000,  CRC(5fc10ccd) SHA1(7debcf223802d5c2ea3d29d39850c8756c863b31) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD( "mav08.b23", 0x000000, 0x040000,  CRC(64966576) SHA1(40c14c0f62eef0317abfb7192505e0337fb5cde5) ) // Sprites #2
	ROM_LOAD( "mav09.e23", 0x040000, 0x040000,  CRC(1ce7b51a) SHA1(17ed8f34bf6d057e0504e72e95f448d5923aa82e) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mav10.r17",  0x00000,  0x80000,  CRC(8ad734b0) SHA1(768b9f54bbf4b54591cafecb7a27960da919ce84) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mav11.r19",  0x00000,  0x80000,  CRC(ef513908) SHA1(72db6c704071d7a784b3768c256fc51087e9e93c) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "jn-00.17l", 0x00000,  0x400,  CRC(6ac77b84) SHA1(9e1e2cabdb20b819e592a0f07d15658062227fa4) ) // Priority (unused)
ROM_END

ROM_START( gunball )
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "jc01.3d",     0x000000, 0x20000, CRC(61bfa998) SHA1(bee57cf5f1759d68948d27a2aaa817c4fc1e5e5a) )
	ROM_LOAD16_BYTE( "jc00.3b",     0x000001, 0x20000, CRC(73ba8f74) SHA1(7bb27e6f81c6ff79fe391faf6e05114a6cd85a5b) )
	ROM_LOAD16_BYTE( "jc03.5d",     0x040000, 0x20000, CRC(19231612) SHA1(fc9e4a2fd09d4a27631260261bb138bc134e0882) )
	ROM_LOAD16_BYTE( "jc02.5b",     0x040001, 0x20000, CRC(a254f34c) SHA1(25ff595eccd6fc2734fefdcda5d35c65112506c4) )
	ROM_LOAD16_BYTE( "jc05-3.6d",   0x080000, 0x40000, CRC(f750a709) SHA1(d339bbac2be95e2947f1195816e4d147e1d38a8f) )
	ROM_LOAD16_BYTE( "jc04-3.6b",   0x080001, 0x40000, CRC(ad711767) SHA1(b5df0fa521ff08ddf5b6203b73a7cb8c6d3121b8) )
	// Two empty ROM slots at d7, b7

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "jl08.r20",  0x00000,  0x10000,  CRC(93d93fe1) SHA1(efc618724251d23a23b3019d475f7739a7e88751) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "jl06.d10",  0x00000,  0x10000,  CRC(91cf668e) SHA1(fc153eaa09777f79369037a139470ad1118e8d7e) ) // Chars
	ROM_LOAD16_BYTE( "jl07.d12",  0x00001,  0x10000,  CRC(e61d0e42) SHA1(80d6ada356c721b0be826554ec6731dbbc19e0ab) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mav00.b10", 0x00000, 0x80000,  CRC(34785d97) SHA1(094f881cd699d1b9fd079778f20f8c9d83283e6e) ) // Tiles
	ROM_LOAD( "mav01.b12", 0x80000, 0x80000,  CRC(8b531b16) SHA1(f734286f4510b2c09dc2d6d2b8c8da9dc4424287) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mav02.b16", 0x000000, 0x100000,  CRC(20723bf7) SHA1(b3491d98ff415701fec2b58d85f99c743d71b013) ) // Tiles
	ROM_LOAD( "mav03.e16", 0x100000, 0x100000,  CRC(ef6195f0) SHA1(491bc030519c78b84396f7f8a21df9daf8acc140) )

	ROM_REGION( 0x300000, "sprites1", 0 )
	ROM_LOAD( "mav04.b19", 0x000000, 0x100000,  CRC(8ba48385) SHA1(926ae1e0e99b8e022b6798ceb29dd080cfc1bada) ) // Sprites #1
	ROM_LOAD( "mav05.e19", 0x180000, 0x100000,  CRC(d92d769c) SHA1(8012e7f2b9a7cbccde8da90025647443beb6c47c) )
	ROM_LOAD( "mav06.b20", 0x100000, 0x080000,  CRC(ae6201a5) SHA1(c0ae87fa96d12377c5522cb8adfed03373ab3757) )
	ROM_LOAD( "mav07.e20", 0x280000, 0x080000,  CRC(5fc10ccd) SHA1(7debcf223802d5c2ea3d29d39850c8756c863b31) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD( "mav08.b23", 0x000000, 0x040000,  CRC(64966576) SHA1(40c14c0f62eef0317abfb7192505e0337fb5cde5) ) // Sprites #2
	ROM_LOAD( "mav09.e23", 0x040000, 0x040000,  CRC(1ce7b51a) SHA1(17ed8f34bf6d057e0504e72e95f448d5923aa82e) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "mav10.r17",  0x00000,  0x80000,  CRC(8ad734b0) SHA1(768b9f54bbf4b54591cafecb7a27960da919ce84) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "mav11.r19",  0x00000,  0x80000,  CRC(ef513908) SHA1(72db6c704071d7a784b3768c256fc51087e9e93c) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "jn-00.17l", 0x00000,  0x400,  CRC(6ac77b84) SHA1(9e1e2cabdb20b819e592a0f07d15658062227fa4) ) // Priority (unused)
ROM_END


ROM_START( schmeisr )
	ROM_REGION(0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "sr001j.8a",  0x000000, 0x80000, CRC(ed31f3ff) SHA1(3e0ae92a07ef94f377730c19069560bda864a64b) )
	ROM_LOAD16_BYTE( "sr006j.8d",  0x000001, 0x80000, CRC(9e9cfa5d) SHA1(10421198739f76e5a5b7ec85b57ead83ae4572d4) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "sr013.18p",  0x00000,  0x10000,  CRC(4ac00cbb) SHA1(cbc21e13978ae5e8940c8c22932dc424605c0ba4) )

	ROM_REGION( 0x040000, "chars", ROMREGION_ERASE00 )
	// Filled in later

	ROM_REGION( 0x100000, "tiles1", 0 ) // Encrypted tiles
	ROM_LOAD( "sr002-74.9a",  0x000000, 0x080000,  CRC(97e15c7b) SHA1(8697115d4b5ed94a1392034060821d3e354bceb0) )
	ROM_LOAD( "sr003-74.11a", 0x080000, 0x080000,  CRC(ea367971) SHA1(365c27bdef4daa01e926fbcf11ce622186133106) )

	ROM_REGION( 0x200000, "tiles2", 0 ) // Encrypted tiles
	ROM_LOAD( "sr007.17d",  0x000000, 0x100000,  CRC(886f80c7) SHA1(c06efc1ce7f51d4e503267e63dc9f762d55ad528) )
	ROM_LOAD( "sr008.18d",  0x100000, 0x100000,  CRC(a74cbc90) SHA1(1aabfec7cd64e7097aa55f0ddc5a2c9e1e25618a) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "sr004.19a", 0x000000, 0x100000,  CRC(e25434a1) SHA1(136ebb36e9b6caeac885423e8f365008ddcea778) )
	ROM_LOAD( "sr005.20a", 0x100000, 0x100000,  CRC(1630033b) SHA1(e2a5fd7f8839db9d5b41d3cada598a6c07a97368) )
	ROM_LOAD( "sr009.19d", 0x200000, 0x100000,  CRC(7b9d982f) SHA1(55d89ee68ceaf3ca8059177721b6c9a16103b1b4) )
	ROM_LOAD( "sr010.20d", 0x300000, 0x100000,  CRC(6e9e5352) SHA1(357659ff5ab9ce94df3313e9a60125769c7fe10a) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "sr011.14p", 0x00000,  0x80000,  CRC(81805616) SHA1(cdca2eb6d12924b9b578b4ce95d5816c7d82f345) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "sr012.15p", 0x00000,  0x80000,  CRC(38843d4d) SHA1(0eda60a4d2caa1e57582c354b8be926905d7fb0c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "hb-00.11p", 0x00000,  0x200,  CRC(b7a7baad) SHA1(39781c3412493b985d3616ac31142fc00bbcddf4) )
ROM_END


ROM_START( hangzo ) // Found on a Data East DE-0353-3 PCB
	ROM_REGION(0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "pro0h 12.18.2a.27c1001", 0x000000, 0x20000, CRC(ac8087db) SHA1(518193372cde6024fda96c6ed1862245e0bfb465) )
	ROM_LOAD16_BYTE( "pro0h 12.18.2d.27c1001", 0x000001, 0x20000, CRC(a6b7f4f4) SHA1(1b3a00ef124d130317171d9042018fbb30662fec) )
	ROM_LOAD16_BYTE( "pro1h 12.10.4a.27c010",  0x040000, 0x20000, CRC(0d04f43d) SHA1(167b595450f6f9b842dc909f6c61a96fa34b7991) )
	ROM_LOAD16_BYTE( "pro1l 12.10.4d.27c010",  0x040001, 0x20000, CRC(2e323918) SHA1(f3d9168f395e835b075dfcbb0464770044d350f3) )
	ROM_LOAD16_BYTE( "pro2h 12.10.6a.27c010",  0x080000, 0x20000, CRC(bb3185a6) SHA1(fa4ba7b4b53a5b3486c36441463a290b12c2acbe) )
	ROM_LOAD16_BYTE( "pro2l 12.10.6d.27c010",  0x080001, 0x20000, CRC(11ce97bb) SHA1(d9c1872762f9acaeed1ebc640a71fa7a4b9d013c) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "snd 12.18.18p.27c512",  0x00000,  0x10000,  CRC(97c592dc) SHA1(7a0e08f3ffd42d07d1d0a9db52c7fd85dba28bd8) )

	ROM_REGION( 0x040000, "chars", ROMREGION_ERASEFF )
	// Filled in later

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "bk1l 12.10.9a.574200",  0x000000, 0x080000,  CRC(5199729b) SHA1(1cb8e7b91e5d0b3a699e47b6bbb3e6e9c53c8590) ) // Tiles
	ROM_LOAD( "bk1h 12.10.11a.574200", 0x080000, 0x080000,  CRC(85887bd8) SHA1(6cb6f9c9e6e5824c6b8006ab195f27014031907e) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "bk23l 12.10.17d.574200",  0x000000, 0x080000,  CRC(ed4e47c6) SHA1(b09ed1a6bf1b42139c7817bae18cd5580c42cf91) ) // tiles 1 & 2
	ROM_LOAD( "bk23h 12.10.18d.574200",  0x100000, 0x080000,  CRC(6a725fb2) SHA1(f4da4da62eb7e3ec2f1a54b57eaf94dc748dec68) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "obj01l 12.10.19a.27c4000", 0x000000, 0x080000,  CRC(c141e310) SHA1(81eb0b977aaf44a110a663416e385ca617de8f28) ) // 4bpp sprites
	ROM_LOAD( "obj01h 12.10.20a.27c4000", 0x100000, 0x080000,  CRC(6a7b4252) SHA1(4bd588bc96c07cc9367afdeab4976af6f8dcc823) )
	ROM_LOAD( "obj23l 12.10.19d.27c4000", 0x200000, 0x080000,  CRC(0db6df6c) SHA1(fe7ef7b5a279656d9e46334c4833ab8911caa5db) )
	ROM_LOAD( "obj23h 12.10.20d.27c4000", 0x300000, 0x080000,  CRC(165031a1) SHA1(0e88fe45fd78d352fdbd398c1d98feefe1b43917) )

	ROM_REGION(0x80000, "oki2", 0 ) // Oki samples
	ROM_LOAD( "pcm16k 11.5.14p.574000", 0x00000,  0x80000,  CRC(5b95c6c7) SHA1(587e7f87d085af3a5d24f317fffc1716c8027e43) )

	ROM_REGION(0x80000, "oki1", 0 ) // Oki samples
	ROM_LOAD( "pcm8k 11.5.15p.27c020", 0x00000,  0x40000,  CRC(02682a9a) SHA1(914ffc7c16e90c1ac28a228df415a956684f8192) )

	ROM_REGION( 0x200, "proms", ROMREGION_ERASEFF )
	ROM_LOAD( "hb-00.11p", 0x00000,  0x200,  CRC(b7a7baad) SHA1(39781c3412493b985d3616ac31142fc00bbcddf4) ) // ?
ROM_END



/**********************************************************************************/

void rohga_state::init_rohga()
{
	deco56_decrypt_gfx(machine(), "chars");
	deco56_decrypt_gfx(machine(), "tiles1");
}

void rohga_state::init_wizdfire()
{
	deco74_decrypt_gfx(machine(), "chars");
	deco74_decrypt_gfx(machine(), "tiles1");
	deco74_decrypt_gfx(machine(), "tiles2");
}

void rohga_state::init_nitrobal()
{
	deco56_decrypt_gfx(machine(), "chars");
	deco56_decrypt_gfx(machine(), "tiles1");
	deco74_decrypt_gfx(machine(), "tiles2");
}

void rohga_state::init_hangzo()
{
	const u8 *src = memregion("tiles1")->base();
	u8 *dst = memregion("chars")->base();

	std::copy(&src[0], &src[0x20000], &dst[0]);
	std::copy(&src[0x80000], &src[0xa0000], &dst[0x20000]);
}

void rohga_state::init_schmeisr()
{
	init_hangzo();

	deco74_decrypt_gfx(machine(), "chars");
	deco74_decrypt_gfx(machine(), "tiles1");
}

GAME( 1991, rohga,     0,        rohga,    rohga,    rohga_state, init_rohga,    ROT0,   "Data East Corporation", "Rohga: Armor Force (Asia/Europe v5.0)",       MACHINE_SUPPORTS_SAVE )
GAME( 1991, rohga1,    rohga,    rohga,    rohga,    rohga_state, init_rohga,    ROT0,   "Data East Corporation", "Rohga: Armor Force (Asia/Europe v3.0 set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, rohga2,    rohga,    rohga,    rohga,    rohga_state, init_rohga,    ROT0,   "Data East Corporation", "Rohga: Armor Force (Asia/Europe v3.0 set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, rohgah,    rohga,    rohga,    rohga,    rohga_state, init_rohga,    ROT0,   "Data East Corporation", "Rohga: Armor Force (Hong Kong v3.0)",         MACHINE_SUPPORTS_SAVE )
GAME( 1991, rohgau,    rohga,    rohga,    rohga,    rohga_state, init_rohga,    ROT0,   "Data East Corporation", "Rohga: Armor Force (US v1.0)",                MACHINE_SUPPORTS_SAVE )
GAME( 1991, wolffang,  rohga,    rohga,    rohga,    rohga_state, init_rohga,    ROT0,   "Data East Corporation", "Wolf Fang: Kuuga 2001 (Japan)",               MACHINE_SUPPORTS_SAVE )

GAME( 1992, wizdfire,  0,        wizdfire, wizdfire, rohga_state, init_wizdfire, ROT0,   "Data East Corporation", "Wizard Fire (Over Sea v2.1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, wizdfirea, wizdfire, wizdfire, wizdfire, rohga_state, init_wizdfire, ROT0,   "Data East Corporation", "Wizard Fire (Over Sea v1.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, wizdfireu, wizdfire, wizdfire, wizdfire, rohga_state, init_wizdfire, ROT0,   "Data East Corporation", "Wizard Fire (US v1.1)",       MACHINE_SUPPORTS_SAVE )
GAME( 1992, darkseal2, wizdfire, wizdfire, wizdfire, rohga_state, init_wizdfire, ROT0,   "Data East Corporation", "Dark Seal 2 (Japan v2.1)",    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1992, nitrobal,  0,        nitrobal, nitrobal, rohga_state, init_nitrobal, ROT270, "Data East Corporation", "Nitro Ball (World, set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, nitrobala, nitrobal, nitrobal, nitrobal, rohga_state, init_nitrobal, ROT270, "Data East Corporation", "Nitro Ball (World, set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // was marked 'US' but doesn't seem to have a 'Winners Don't Use Drugs' screen, so unlikely
GAME( 1992, nitrobalb, nitrobal, nitrobal, nitrobal, rohga_state, init_nitrobal, ROT270, "Data East Corporation", "Nitro Ball (World, set 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, gunball,   nitrobal, nitrobal, nitrobal, rohga_state, init_nitrobal, ROT270, "Data East Corporation", "Gun Ball (Japan)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1993, schmeisr,  0,        schmeisr, schmeisr, rohga_state, init_schmeisr, ROT0,   "Hot-B",                 "Schmeiser Robo (Japan)",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1992, hangzo,    0,        hangzo,   hangzo,   rohga_state, init_hangzo,   ROT0,   "Hot-B",                 "Hangzo (Japan, prototype)", MACHINE_SUPPORTS_SAVE ) // ROM contains a '(c)1992 Data East Corporation' string, but other sources indicate the game is by Hot-B
