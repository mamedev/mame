// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  Edward Randy      (c) 1990 Data East Corporation (World version)
  Edward Randy      (c) 1990 Data East Corporation (Japanese version)
  Caveman Ninja     (c) 1991 Data East Corporation (World version)
  Caveman Ninja     (c) 1991 Data East Corporation (USA version)
  Joe & Mac         (c) 1991 Data East Corporation (Japanese version)
  Robocop 2         (c) 1991 Data East Corporation (USA version)
  Robocop 2         (c) 1991 Data East Corporation (Japanese version)
  Robocop 2         (c) 1991 Data East Corporation (World version)
  Stone Age         (Italian bootleg)
  Mutant Fighter    (c) 1992 Data East Corporation (World version)
  Death Brade       (c) 1992 Data East Corporation (Japanese version)

  Edward Randy runs on the same board as Caveman Ninja but the protection
  chip is different.  Robocop 2 also has a different protection chip but
  strangely makes very little use of it (only one check at the start).
  Robocop 2 is a different board but similar hardware.

  Edward Randy (World rev 1) seems much more polished than World rev 2 -
  better attract mode at least.

  The sound program of Stoneage is ripped from Block Out (by Technos!)

  Mutant Fighter introduced alpha-blending to this basic board design.
  The characters shadows sometimes jump around a little - a bug in the
  original board, not the emulation.

Caveman Ninja Issues:
  End of level 2 is corrupt.

  Emulation by Bryan McPhail, mish@tendril.co.uk

Note about version levels using Mutant Fighter as the example:
  Version 1  HD-00
  Version 2  HD-00-1
  Version 3  HD-00-2
  Version 4  HD-00-3
    ect...

***************************************************************************/

#include "emu.h"
#include "includes/cninja.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/decocrpt.h"
#include "sound/2203intf.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"
#include "speaker.h"

/**********************************************************************************/

template<int Chip>
WRITE16_MEMBER(cninja_state::cninja_pf_control_w)
{
	m_screen->update_partial(m_screen->vpos());
	m_deco_tilegen[Chip]->pf_control_w(offset, data, mem_mask);
}


READ16_MEMBER( cninja_state::cninja_protection_region_0_104_r )
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_ioprot->read_data( deco146_addr, cs );
	return data;
}

WRITE16_MEMBER( cninja_state::cninja_protection_region_0_104_w )
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_ioprot->write_data( deco146_addr, data, mem_mask, cs );
}

READ16_MEMBER(cninja_state::cninjabl2_sprite_dma_r)
{
	m_spriteram[0]->copy();
	return 0;
}


void cninja_state::cninja_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();

	map(0x140000, 0x14000f).w(FUNC(cninja_state::cninja_pf_control_w<0>));
	map(0x144000, 0x144fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x146000, 0x146fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x14c000, 0x14c7ff).writeonly().share("pf1_rowscroll");
	map(0x14e000, 0x14e7ff).ram().share("pf2_rowscroll");

	map(0x150000, 0x15000f).w(FUNC(cninja_state::cninja_pf_control_w<1>));
	map(0x154000, 0x154fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x156000, 0x156fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x15c000, 0x15c7ff).ram().share("pf3_rowscroll");
	map(0x15e000, 0x15e7ff).ram().share("pf4_rowscroll");

	map(0x184000, 0x187fff).ram();
	map(0x190000, 0x190007).m("irq", FUNC(deco_irq_device::map)).umask16(0x00ff);
	map(0x19c000, 0x19dfff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x1a4000, 0x1a47ff).ram().share("spriteram1");           /* Sprites */
	map(0x1b4000, 0x1b4001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write)); /* DMA flag */
	map(0x1bc000, 0x1bffff).rw(FUNC(cninja_state::cninja_protection_region_0_104_r), FUNC(cninja_state::cninja_protection_region_0_104_w)).share("prot16ram"); /* Protection device */

	map(0x308000, 0x308fff).nopw(); /* Bootleg only */
}

void cninja_state::cninjabl_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();

	map(0x138000, 0x1387ff).ram().share("spriteram1"); /* bootleg sprite-ram (sprites rewritten here in new format) */

	map(0x140000, 0x14000f).w(FUNC(cninja_state::cninja_pf_control_w<0>));
	map(0x144000, 0x144fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x146000, 0x146fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x14c000, 0x14c7ff).writeonly().share("pf1_rowscroll");
	map(0x14e000, 0x14e7ff).ram().share("pf2_rowscroll");

	map(0x150000, 0x15000f).w(FUNC(cninja_state::cninja_pf_control_w<1>));    // not used / incorrect on this
	map(0x154000, 0x154fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x156000, 0x156fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x15c000, 0x15c7ff).ram().share("pf3_rowscroll");
	map(0x15e000, 0x15e7ff).ram().share("pf4_rowscroll");

	map(0x17ff22, 0x17ff23).portr("DSW");
	map(0x17ff28, 0x17ff29).portr("SYSTEM");
	map(0x17ff2b, 0x17ff2b).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x17ff2c, 0x17ff2d).portr("INPUTS");

	map(0x180000, 0x187fff).ram(); // more ram on bootleg?

	map(0x190000, 0x190007).m("irq", FUNC(deco_irq_device::map)).umask16(0x00ff);
	map(0x19c000, 0x19dfff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x1b4000, 0x1b4001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write)); /* DMA flag */
}

READ16_MEMBER( cninja_state::edrandy_protection_region_8_146_r )
{
	int real_address = 0x1a0000 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_ioprot->read_data( deco146_addr, cs );
	return data;
}

WRITE16_MEMBER( cninja_state::edrandy_protection_region_8_146_w )
{
	int real_address = 0x1a0000 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_ioprot->write_data( deco146_addr, data, mem_mask, cs );
}

READ16_MEMBER( cninja_state::edrandy_protection_region_6_146_r )
{
//  uint16_t realdat = deco16_60_prot_r(space,offset&0x3ff,mem_mask);

	int real_address = 0x198000 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_ioprot->read_data( deco146_addr, cs );


//  if ((realdat & mem_mask) != (data & mem_mask))
//      printf("returned %04x instead of %04x (real address %08x)\n", data, realdat, real_address);

	return data;
}

WRITE16_MEMBER( cninja_state::edrandy_protection_region_6_146_w )
{
//  deco16_60_prot_w(space,offset&0x3ff,data,mem_mask);


	int real_address = 0x198000 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_ioprot->write_data( deco146_addr, data, mem_mask, cs );
}

void cninja_state::edrandy_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();

	map(0x140000, 0x14000f).w(FUNC(cninja_state::cninja_pf_control_w<0>));
	map(0x144000, 0x144fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x146000, 0x146fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x14c000, 0x14c7ff).ram().share("pf1_rowscroll");
	map(0x14e000, 0x14e7ff).ram().share("pf2_rowscroll");

	map(0x150000, 0x15000f).w(FUNC(cninja_state::cninja_pf_control_w<1>));
	map(0x154000, 0x154fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x156000, 0x156fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x15c000, 0x15c7ff).ram().share("pf3_rowscroll");
	map(0x15e000, 0x15e7ff).ram().share("pf4_rowscroll");

	map(0x188000, 0x189fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x194000, 0x197fff).ram(); /* Main ram */
	map(0x198000, 0x19bfff).rw(FUNC(cninja_state::edrandy_protection_region_6_146_r), FUNC(cninja_state::edrandy_protection_region_6_146_w)).share("prot16ram"); /* Protection device */
//  map(0x198000, 0x1987ff).rw(FUNC(cninja_state::edrandy_protection_region_6_146_r), FUNC(cninja_state::edrandy_protection_region_6_146_w)).share("prot16ram"); /* Protection device */

	map(0x1a0000, 0x1a3fff).rw(FUNC(cninja_state::edrandy_protection_region_8_146_r), FUNC(cninja_state::edrandy_protection_region_8_146_w));

	map(0x1a4000, 0x1a4007).m("irq", FUNC(deco_irq_device::map)).umask16(0x00ff);
	map(0x1ac000, 0x1ac001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write)); /* DMA flag */
	map(0x1bc000, 0x1bc7ff).ram().share("spriteram1"); /* Sprites */
	map(0x1bc800, 0x1bcfff).nopw(); /* Another bug in game code?  Sprite list can overrun.  Doesn't seem to mirror */
}

WRITE16_MEMBER(cninja_state::robocop2_priority_w)
{
	COMBINE_DATA(&m_priority);
}

void cninja_state::robocop2_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();

	map(0x140000, 0x14000f).w(FUNC(cninja_state::cninja_pf_control_w<0>));
	map(0x144000, 0x144fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x146000, 0x146fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x14c000, 0x14c7ff).ram().share("pf1_rowscroll");
	map(0x14e000, 0x14e7ff).ram().share("pf2_rowscroll");

	map(0x150000, 0x15000f).w(FUNC(cninja_state::cninja_pf_control_w<1>));
	map(0x154000, 0x154fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x156000, 0x156fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x15c000, 0x15c7ff).ram().share("pf3_rowscroll");
	map(0x15e000, 0x15e7ff).ram().share("pf4_rowscroll");

	map(0x180000, 0x1807ff).ram().share("spriteram1");
//  map(0x18c000, 0x18c0ff).w(FUNC(cninja_state::cninja_loopback_w)) /* Protection writes */
//  map(0x18c000, 0x18c7ff).r(m_ioprot, FUNC(deco146_device,robocop2_prot_r)) /* Protection device */
//  map(0x18c064, 0x18c065).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x18c000, 0x18ffff).rw(FUNC(cninja_state::mutantf_protection_region_0_146_r), FUNC(cninja_state::mutantf_protection_region_0_146_w)).share("prot16ram"); /* Protection device */

	map(0x198000, 0x198001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write)); /* DMA flag */
	map(0x1a8000, 0x1a9fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x1b0000, 0x1b0007).m("irq", FUNC(deco_irq_device::map)).umask16(0x00ff);
	map(0x1b8000, 0x1bbfff).ram(); /* Main ram */
	map(0x1f0000, 0x1f0001).w(FUNC(cninja_state::robocop2_priority_w));
	map(0x1f8000, 0x1f8001).portr("DSW3"); /* Dipswitch #3 */
}


READ16_MEMBER( cninja_state::mutantf_protection_region_0_146_r )
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_ioprot->read_data( deco146_addr, cs );
	return data;
}

WRITE16_MEMBER( cninja_state::mutantf_protection_region_0_146_w )
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_ioprot->write_data( deco146_addr, data, mem_mask, cs );
}

READ16_MEMBER( cninja_state::mutantf_71_r )
{
	return 0xffff; // todo
}

void cninja_state::mutantf_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x120000, 0x1207ff).ram().share("spriteram1");
	map(0x140000, 0x1407ff).ram().share("spriteram2");
	map(0x160000, 0x161fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x180001).w(FUNC(cninja_state::robocop2_priority_w));
	map(0x180002, 0x180003).nopw(); /* VBL irq ack */
	map(0x1a0000, 0x1a3fff).rw(FUNC(cninja_state::mutantf_protection_region_0_146_r), FUNC(cninja_state::mutantf_protection_region_0_146_w)).share("prot16ram"); /* Protection device */
	map(0x1c0000, 0x1c0001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write)).r(FUNC(cninja_state::mutantf_71_r));
	map(0x1e0000, 0x1e0001).w(m_spriteram[1], FUNC(buffered_spriteram16_device::write));

	map(0x300000, 0x30000f).w(FUNC(cninja_state::cninja_pf_control_w<0>));
	map(0x304000, 0x305fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x306000, 0x307fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x308000, 0x3087ff).ram().share("pf1_rowscroll");
	map(0x30a000, 0x30a7ff).ram().share("pf2_rowscroll");

	map(0x310000, 0x31000f).w(FUNC(cninja_state::cninja_pf_control_w<1>));
	map(0x314000, 0x315fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x316000, 0x317fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x318000, 0x3187ff).ram().share("pf3_rowscroll");
	map(0x31a000, 0x31a7ff).ram().share("pf4_rowscroll");

	map(0xad00ac, 0xad00ff).nopr(); /* Reads from here seem to be a game code bug */
}

/******************************************************************************/

void cninja_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x110000, 0x110001).rw("ym2", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw(m_oki2, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140000).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}

void cninja_state::sound_map_mutantf(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).noprw();
	map(0x110000, 0x110001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw(m_oki2, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140001).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}

void cninja_state::stoneage_s_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9800).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
}

void cninja_state::cninjabl_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9800).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void cninja_state::cninjabl2_s_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(cninja_state::cninjabl2_oki_bank_w));
	map(0x9800, 0x9800).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
}

void cninja_state::cninjabl2_oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom().region("oki1", 0);
	map(0x30000, 0x3ffff).bankr("okibank");
}

/***********************************************************
              Basic INPUT PORTS, DIPs
***********************************************************/

#define DATAEAST_2BUTTON \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 ) \
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) \
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) \
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) \
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) \
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) \
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) \
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

#define DATAEAST_COINAGE \
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3") \
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6") \
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )

/**********************************************************************************/

static INPUT_PORTS_START( edrandy )

	PORT_START("INPUTS")
	DATAEAST_2BUTTON

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW")
	DATAEAST_COINAGE

	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Credit(s) to Start" ) PORT_DIPLOCATION("SW1:8") /* Also, if Coin A and B are on 1/1, 0x00 gives 2 to start, 1 to continue */
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_DIPNAME( 0x0300, 0x0300, "Player's Power" ) PORT_DIPLOCATION("SW2:1,2")    /* Energy */
	PORT_DIPSETTING(      0x0100, DEF_STR( Very_Low ) ) /* 2.5 */
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )      /* 3 */
	PORT_DIPSETTING(      0x0300, DEF_STR( Medium ) )   /* 3.5 */
	PORT_DIPSETTING(      0x0200, DEF_STR( High ) )     /* 4.5 */
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")    /* Listed as "Don't Change" in the manual */
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")    /* Listed as "Don't Change" in the manual */
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Continues ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( edrandc )
	PORT_INCLUDE(edrandy)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")    /* Listed as "Don't Change" in the manual */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cninja )

	PORT_START("INPUTS")
	DATAEAST_2BUTTON

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW")   /* Dip switch bank 1 */

	DATAEAST_COINAGE

	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")    /* If DS #1-#6 are all ON */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )      /*  Standard Coin Credit */
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )       /*  2 Coins to Start / 1 Coin to Continue */

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2") /* Dip switch bank 2 */
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Restore Life Meter" ) PORT_DIPLOCATION("SW2:5")  /* Recovery of Life After Defeated Boss */
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")    /* Listed as "Don't Change" in the manual */
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")    /* Listed as "Don't Change" in the manual */
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cninjau )
	PORT_INCLUDE(cninja)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, "Credit(s) to Start" ) PORT_DIPLOCATION("SW1:8") /* Also, if Coin A and B are on 1/1, 0x00 gives 2 to start, 1 to continue */
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
INPUT_PORTS_END

static INPUT_PORTS_START( robocop2 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW")   /* Dip switch bank 1 */

	DATAEAST_COINAGE

	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2") /* Dip switch bank 2 */
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "400 Seconds" )
	PORT_DIPSETTING(      0x0c00, "300 Seconds" )
	PORT_DIPSETTING(      0x0400, "200 Seconds" )
	PORT_DIPSETTING(      0x0000, "100 Seconds" )
	PORT_DIPNAME( 0x3000, 0x3000, "Health" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "17" )
	PORT_DIPSETTING(      0x1000, "24" )
	PORT_DIPSETTING(      0x3000, "33" )
	PORT_DIPSETTING(      0x2000, "40" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Continues ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0003, 0x0003, "Bullets" ) PORT_DIPLOCATION("SW3:1,2")   /* Dip switch bank 3 */
	PORT_DIPSETTING(      0x0000, "Least" )
	PORT_DIPSETTING(      0x0001, "Less" )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, "More" )
	PORT_DIPNAME( 0x000c, 0x000c, "Enemy Movement" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x0008, "Slow" )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x0030, 0x0030, "Enemy Strength" ) PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(      0x0020, "Less" )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, "More" )
	PORT_DIPSETTING(      0x0000, "Most" )
	PORT_DIPNAME( 0x0040, 0x0040, "Enemy Weapon Speed" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0x0080, 0x0080, "Game Over Message" ) PORT_DIPLOCATION("SW3:8") /* This refers to the system shut down text just before the actual "GAME OVER" */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mutantf )

	PORT_START("INPUTS")
	DATAEAST_2BUTTON

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW")   /* Dip switch bank 1 */

	DATAEAST_COINAGE

	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Credit(s) to Start" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_DIPNAME( 0x0300, 0x0300, "Timer Decrement" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "Slow" )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, "Fast" )
	PORT_DIPSETTING(      0x0000, "Very Fast" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Life Per Stage" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "Least" )
	PORT_DIPSETTING(      0x1000, "Little" )
	PORT_DIPSETTING(      0x2000, "Less" )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Continues ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
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
	16*8    /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static const gfx_layout tilelayout_8bpp =
{
	16,16,
	4096,
	8,
	{ 0x100000*8+8, 0x100000*8, 0x40000*8+8, 0x40000*8, 0xc0000*8+8, 0xc0000*8, 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static GFXDECODE_START( gfx_cninja )
	GFXDECODE_ENTRY( "chars",    0, charlayout,   0, 32 )  /* Characters 8x8 */
	GFXDECODE_ENTRY( "tiles1",   0, tilelayout,   0, 32 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "tiles2",   0, tilelayout, 512, 64 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout, 768, 32 )  /* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_robocop2 )
	GFXDECODE_ENTRY( "chars",    0, charlayout,        0, 32 )  /* Characters 8x8 */
	GFXDECODE_ENTRY( "tiles1",   0, tilelayout,        0, 32 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "tiles2",   0, tilelayout,      512, 64 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout,      768, 32 )  /* Sprites 16x16 */
	GFXDECODE_ENTRY( "tiles2",   0, tilelayout_8bpp, 512,  1 )  /* Tiles 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_mutantf )
	GFXDECODE_ENTRY( "chars",    0, charlayout, 0,  64 )  /* Characters 8x8 */
	GFXDECODE_ENTRY( "tiles1",   0, tilelayout, 0,  64 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "tiles2",   0, tilelayout, 0,  80 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout, 0, 128 )  /* Sprites 16x16 */
	GFXDECODE_ENTRY( "sprites2", 0, tilelayout, 0,  16 )  /* Sprites 16x16 */
GFXDECODE_END

/**********************************************************************************/

WRITE8_MEMBER(cninja_state::sound_bankswitch_w)
{
	/* the second OKIM6295 ROM is bank switched */
	m_oki2->set_rom_bank(data & 1);
}

WRITE8_MEMBER(cninja_state::cninjabl2_oki_bank_w)
{
	m_okibank->set_entry(data & 7);
}

/**********************************************************************************/

DECO16IC_BANK_CB_MEMBER(cninja_state::cninja_bank_callback)
{
	if ((bank >> 4) & 0xf)
		return 0x0000; /* Only 2 banks */
	return 0x1000;
}

DECO16IC_BANK_CB_MEMBER(cninja_state::robocop2_bank_callback)
{
	return (bank & 0x30) << 8;
}

DECO16IC_BANK_CB_MEMBER(cninja_state::mutantf_1_bank_callback)
{
	return ((bank >> 4) & 0x3) << 12;
}

DECO16IC_BANK_CB_MEMBER(cninja_state::mutantf_2_bank_callback)
{
	return ((bank >> 5) & 0x1) << 14;
}


DECOSPR_PRIORITY_CB_MEMBER(cninja_state::pri_callback)
{
	/* Sprite/playfield priority */
	switch (pri & 0xc000)
	{
		case 0x0000: return 0;
		case 0x4000: return 0xf0;
		case 0x8000: return 0xf0 | 0xcc;
		case 0xc000: return 0xf0 | 0xcc; /* Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer) */
	}

	return 0;
}

MACHINE_START_MEMBER(cninja_state,robocop2)
{
	save_item(NAME(m_priority));
}

MACHINE_RESET_MEMBER(cninja_state,robocop2)
{
	m_priority = 0;
}

void cninja_state::cninja(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cninja_state::cninja_map);

	h6280_device &audiocpu(H6280(config, m_audiocpu, XTAL(32'220'000) / 8));
	audiocpu.set_addrmap(AS_PROGRAM, &cninja_state::sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	deco_irq_device &irq(DECO_IRQ(config, "irq", 0));
	irq.set_screen_tag(m_screen);
	irq.raster1_irq_callback().set_inputline(m_maincpu, 3);
	irq.raster2_irq_callback().set_inputline(m_maincpu, 4);
	irq.vblank_irq_callback().set_inputline(m_maincpu, 5);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(24'000'000) / 4, 376, 0, 256, 274, 8, 248);
	m_screen->set_screen_update(FUNC(cninja_state::screen_update_cninja));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cninja);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 2048);

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf1_col_bank(0x00);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(cninja_state::cninja_bank_callback), this);
	m_deco_tilegen[1]->set_bank2_callback(FUNC(cninja_state::cninja_bank_callback), this);
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0);
	m_sprgen[0]->set_gfx_region(3);
	m_sprgen[0]->set_pri_callback(FUNC(cninja_state::pri_callback), this);
	m_sprgen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO104PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
	m_ioprot->set_use_magic_read_address_xor(true);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2203(config, "ym1", XTAL(32'220'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.60);

	ym2151_device &ym2(YM2151(config, "ym2", XTAL(32'220'000) / 9));
	ym2.irq_handler().set_inputline(m_audiocpu, 1); /* IRQ2 */
	ym2.port_write_handler().set(FUNC(cninja_state::sound_bankswitch_w));
	ym2.add_route(0, "mono", 0.45);
	ym2.add_route(1, "mono", 0.45);

	OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.75);

	OKIM6295(config, m_oki2, XTAL(32'220'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.60);
}


void cninja_state::stoneage(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cninja_state::cninja_map);

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cninja_state::stoneage_s_map);

	deco_irq_device &irq(DECO_IRQ(config, "irq", 0));
	irq.set_screen_tag(m_screen);
	irq.raster1_irq_callback().set_inputline(m_maincpu, 3);
	irq.raster2_irq_callback().set_inputline(m_maincpu, 4);
	irq.vblank_irq_callback().set_inputline(m_maincpu, 5);

	MCFG_VIDEO_START_OVERRIDE(cninja_state,cninja)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(24'000'000) / 4, 376, 0, 256, 274, 8, 248);
	m_screen->set_screen_update(FUNC(cninja_state::screen_update_cninja));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cninja);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 2048);

	MCFG_VIDEO_START_OVERRIDE(cninja_state,stoneage)

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf1_col_bank(0x00);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(cninja_state::cninja_bank_callback), this);
	m_deco_tilegen[1]->set_bank2_callback(FUNC(cninja_state::cninja_bank_callback), this);
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0);
	m_sprgen[0]->set_gfx_region(3);
	m_sprgen[0]->set_pri_callback(FUNC(cninja_state::pri_callback), this);
	m_sprgen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO104PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_ioprot->set_use_magic_read_address_xor(true);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(32'220'000) / 9));
	ymsnd.irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	ymsnd.add_route(0, "mono", 0.45);
	ymsnd.add_route(1, "mono", 0.45);

	OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.75);
}

void cninja_state::cninjabl2(machine_config &config)
{
	stoneage(config);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cninja_state::cninjabl2_s_map);

	m_screen->set_screen_update(FUNC(cninja_state::screen_update_cninjabl2));

	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);

	config.device_remove("ymsnd");

	okim6295_device &oki1(OKIM6295(config.replace(), "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_LOW));
	oki1.add_route(ALL_OUTPUTS, "mono", 1.0);
	oki1.set_addrmap(0, &cninja_state::cninjabl2_oki_map);
}

void cninja_state::cninjabl(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cninja_state::cninjabl_map);

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cninja_state::cninjabl_sound_map);

	deco_irq_device &irq(DECO_IRQ(config, "irq", 0));
	irq.set_screen_tag(m_screen);
	irq.raster1_irq_callback().set_inputline(m_maincpu, 3);
	irq.raster2_irq_callback().set_inputline(m_maincpu, 4);
	irq.vblank_irq_callback().set_inputline(m_maincpu, 5);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(24'000'000) / 4, 376, 0, 256, 274, 8, 248);
	m_screen->set_screen_update(FUNC(cninja_state::screen_update_cninjabl));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cninja);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 2048);

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf1_col_bank(0x00);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(cninja_state::cninja_bank_callback), this);
	m_deco_tilegen[1]->set_bank2_callback(FUNC(cninja_state::cninja_bank_callback), this);
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(32'220'000) / 9));
	ymsnd.irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	ymsnd.add_route(0, "mono", 0.45);
	ymsnd.add_route(1, "mono", 0.45);

	OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.75);
}


void cninja_state::edrandy(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cninja_state::edrandy_map);

	h6280_device &audiocpu(H6280(config, m_audiocpu, XTAL(32'220'000) / 8));
	audiocpu.set_addrmap(AS_PROGRAM, &cninja_state::sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	deco_irq_device &irq(DECO_IRQ(config, "irq", 0));
	irq.set_screen_tag(m_screen);
	irq.raster1_irq_callback().set_inputline(m_maincpu, 3);
	irq.raster2_irq_callback().set_inputline(m_maincpu, 4);
	irq.vblank_irq_callback().set_inputline(m_maincpu, 5);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(24'000'000) / 4, 376, 0, 256, 274, 8, 248);
	m_screen->set_screen_update(FUNC(cninja_state::screen_update_edrandy));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cninja);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 2048);

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf1_col_bank(0x00);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(cninja_state::cninja_bank_callback), this);
	m_deco_tilegen[1]->set_bank2_callback(FUNC(cninja_state::cninja_bank_callback), this);
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0);
	m_sprgen[0]->set_gfx_region(3);
	m_sprgen[0]->set_pri_callback(FUNC(cninja_state::pri_callback), this);
	m_sprgen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO146PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2203(config, "ym1", XTAL(32'220'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.60);

	ym2151_device &ym2(YM2151(config, "ym2", XTAL(32'220'000) / 9));
	ym2.irq_handler().set_inputline(m_audiocpu, 1); /* IRQ2 */
	ym2.port_write_handler().set(FUNC(cninja_state::sound_bankswitch_w));
	ym2.add_route(0, "mono", 0.45);
	ym2.add_route(1, "mono", 0.45);

	OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.75);

	OKIM6295(config, m_oki2, XTAL(32'220'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.60);
}


void cninja_state::robocop2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(28'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cninja_state::robocop2_map);

	h6280_device &audiocpu(H6280(config, m_audiocpu, XTAL(32'220'000) / 8));
	audiocpu.set_addrmap(AS_PROGRAM, &cninja_state::sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "lspeaker", 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "rspeaker", 0);

	deco_irq_device &irq(DECO_IRQ(config, "irq", 0));
	irq.set_screen_tag(m_screen);
	irq.raster1_irq_callback().set_inputline(m_maincpu, 3);
	irq.raster2_irq_callback().set_inputline(m_maincpu, 4);
	irq.vblank_irq_callback().set_inputline(m_maincpu, 5);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(cninja_state::screen_update_robocop2));
	m_screen->set_palette(m_palette);

	MCFG_MACHINE_START_OVERRIDE(cninja_state,robocop2)
	MCFG_MACHINE_RESET_OVERRIDE(cninja_state,robocop2)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_robocop2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 2048);

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank2_callback(FUNC(cninja_state::robocop2_bank_callback), this);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf1_col_bank(0x00);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(cninja_state::robocop2_bank_callback), this);
	m_deco_tilegen[1]->set_bank2_callback(FUNC(cninja_state::robocop2_bank_callback), this);
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0);
	m_sprgen[0]->set_gfx_region(3);
	m_sprgen[0]->set_pri_callback(FUNC(cninja_state::pri_callback), this);
	m_sprgen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO146PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
	m_ioprot->set_use_magic_read_address_xor(true);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(32'220'000) / 8));
	ym1.add_route(ALL_OUTPUTS, "lspeaker", 0.60);
	ym1.add_route(ALL_OUTPUTS, "rspeaker", 0.60);

	ym2151_device &ym2(YM2151(config, "ym2", XTAL(32'220'000) / 9));
	ym2.irq_handler().set_inputline(m_audiocpu, 1); /* IRQ2 */
	ym2.port_write_handler().set(FUNC(cninja_state::sound_bankswitch_w));
	ym2.add_route(0, "lspeaker", 0.45);
	ym2.add_route(1, "rspeaker", 0.45);

	okim6295_device &oki1(OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH));
	oki1.add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	oki1.add_route(ALL_OUTPUTS, "rspeaker", 0.75);

	OKIM6295(config, m_oki2, XTAL(32'220'000) / 16, okim6295_device::PIN7_HIGH);
	m_oki2->add_route(ALL_OUTPUTS, "lspeaker", 0.60);
	m_oki2->add_route(ALL_OUTPUTS, "rspeaker", 0.60);
}


void cninja_state::mutantf(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(28'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cninja_state::mutantf_map);
	m_maincpu->set_vblank_int("screen", FUNC(cninja_state::irq6_line_hold));

	h6280_device &audiocpu(H6280(config, m_audiocpu, XTAL(32'220'000) / 8));
	audiocpu.set_addrmap(AS_PROGRAM, &cninja_state::sound_map_mutantf);
	audiocpu.add_route(ALL_OUTPUTS, "lspeaker", 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "rspeaker", 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248); // same as robocop2? verify this from real pcb
	m_screen->set_screen_update(FUNC(cninja_state::screen_update_mutantf));

	MCFG_MACHINE_START_OVERRIDE(cninja_state,robocop2)
	MCFG_MACHINE_RESET_OVERRIDE(cninja_state,robocop2)
	MCFG_VIDEO_START_OVERRIDE(cninja_state,mutantf)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mutantf);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 2048);

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x30);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(cninja_state::mutantf_1_bank_callback), this);
	m_deco_tilegen[0]->set_bank2_callback(FUNC(cninja_state::mutantf_2_bank_callback), this);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_trans_mask(0x0f);
	m_deco_tilegen[1]->set_pf1_col_bank(0x20);
	m_deco_tilegen[1]->set_pf2_col_bank(0x40);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(cninja_state::mutantf_1_bank_callback), this);
	m_deco_tilegen[1]->set_bank2_callback(FUNC(cninja_state::mutantf_1_bank_callback), this);
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0);
	m_sprgen[0]->set_gfx_region(3);
	m_sprgen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[1], 0);
	m_sprgen[1]->set_gfx_region(4);
	m_sprgen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO146PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(32'220'000) / 9));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 1); /* IRQ2 */
	ymsnd.port_write_handler().set(FUNC(cninja_state::sound_bankswitch_w));
	ymsnd.add_route(0, "lspeaker", 0.45);
	ymsnd.add_route(1, "rspeaker", 0.45);

	okim6295_device &oki1(OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH));
	oki1.add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	oki1.add_route(ALL_OUTPUTS, "rspeaker", 0.75);

	OKIM6295(config, m_oki2, XTAL(32'220'000) / 16, okim6295_device::PIN7_HIGH);
	m_oki2->add_route(ALL_OUTPUTS, "lspeaker", 0.60);
	m_oki2->add_route(ALL_OUTPUTS, "rspeaker", 0.60);
}

/**********************************************************************************/

ROM_START( cninja ) /* World ver 4 */
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gn-02-3.1k", 0x00000, 0x20000, CRC(39aea12a) SHA1(5de4e26d2c03c559249720b6a204567673754774) )
	ROM_LOAD16_BYTE( "gn-05-2.3k", 0x00001, 0x20000, CRC(0f4360ef) SHA1(d60b3377e818a037d0f94383dd207865853f529d) )
	ROM_LOAD16_BYTE( "gn-01-2.1j", 0x40000, 0x20000, CRC(f740ef7e) SHA1(e70bf04e2407dc0c512617417581388365eb1d35) )
	ROM_LOAD16_BYTE( "gn-04-2.3j", 0x40001, 0x20000, CRC(c98fcb62) SHA1(b2ee52a9418190c62e0b34920e44111270d68286) )
	ROM_LOAD16_BYTE( "gn-00.1h",   0x80000, 0x20000, CRC(0b110b16) SHA1(a967c8aeae3f0cee1f354583cf26ee736636aaf8) )
	ROM_LOAD16_BYTE( "gn-03.1k",   0x80001, 0x20000, CRC(1e28e697) SHA1(2313e97f3a34892dfdc338944c0f00538fcae800) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gl-07.13k",  0x00000, 0x10000, CRC(ca8bef96) SHA1(fcdbd598c85e339a3389a2ef58cf2d5b3a2779af) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gl-08.6y", 0x00001, 0x10000, CRC(33a2b400) SHA1(fdb8de315f33705719c0ac03a61fb56ffbfdf597) )
	ROM_LOAD16_BYTE( "gl-09.6z", 0x00000, 0x10000, CRC(5a2d4752) SHA1(617dd10a99b5b55ca64dcdd22a0f133b0d6b770d) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mag-02.4z", 0x000000, 0x80000, CRC(de89c69a) SHA1(b41bdf859854b5541c7eae7cd541b910cea1f839) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mag-00.1y", 0x000000, 0x40000, CRC(a8f05d33) SHA1(a1330bc9ca4648219403db087622badfc632b47d) )
	ROM_CONTINUE(          0x080000, 0x40000 )
	ROM_LOAD( "mag-01.1z", 0x040000, 0x40000, CRC(5b399eed) SHA1(490f8f9c0c557b0ba94c6019e3fe680641a0787e) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )

	ROM_REGION( 0x200000, "sprites1", 0 )   /* sprites */
	ROM_LOAD( "mag-05.12y", 0x000000, 0x80000, CRC(56a53254) SHA1(10940cfdc6fbe9013865107de3394ca7f782d9c7) )
	ROM_LOAD( "mag-06.12z", 0x080000, 0x80000, CRC(82d44749) SHA1(c471fa573e00c2f8ae44068439ba6d849a124c68) )
	ROM_LOAD( "mag-03.9y",  0x100000, 0x80000, CRC(2220eb9f) SHA1(bdf0bd6e6ba375f0770b9d08a7efa32201cbb6ef) )
	ROM_LOAD( "mag-04.9z",  0x180000, 0x80000, CRC(144b94cc) SHA1(d982508608942a714b428a2b721bf24e1627cbb6) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gl-06.13j", 0x00000, 0x20000, CRC(d92e519d) SHA1(08238f12bf7058a3965ab6348b468e3d35d4cd23) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mag-07.13f", 0x00000, 0x80000, CRC(08eb5264) SHA1(3e33085f00b758acfc78034dc9a75fd6921fc3fe) )  /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "mb7122h.7v", 0x00000, 0x400, CRC(a1267336) SHA1(d11ea9d78526ac3c0dc6e57a2da5914273ad1e3f) )    /* Priority  Unused */

	ROM_REGION( 0x0e00, "plds", 0 )
	ROM_LOAD( "tj-00.9j", 0x0000, 0x0117, CRC(46defe8f) SHA1(50db2c265a0ab592938f780f212ef86070b2daa2) ) /* GAL16V8 */
	ROM_LOAD( "tj-01.9h", 0x0200, 0x0117, CRC(7a86902d) SHA1(7e116dbabe615ddae1588001b31a0a6e6e4dc46d) ) /* GAL16V8 */
	ROM_LOAD( "tj-02.9h", 0x0400, 0x0117, CRC(b476d59c) SHA1(c17b7884180b5041d1524e9fd479cddac787a3cb) ) /* GAL16V8 */
	ROM_LOAD( "tj-03.9e", 0x0600, 0x0117, CRC(cfb6e4aa) SHA1(fc70d1d43d8836ae7984c432b3f9e35c2256b18e) ) /* GAL16V8 */
	ROM_LOAD( "tj-04.5n", 0x0800, 0x0117, CRC(bca07086) SHA1(cf713185d7430e17077a95a58a42d28336432c95) ) /* GAL16V8 */
	ROM_LOAD( "tj-05.1r", 0x0a00, 0x0117, CRC(0dfc091b) SHA1(ab7ff0c8ada10633d27d5305f1f41f738647412a) ) /* GAL16V8 */
ROM_END

ROM_START( cninja1 ) /* World ver 1 */
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gn-02.1k", 0x00000, 0x20000, CRC(a6c40959) SHA1(01d223b76a7798d5bd8b542b9ce8e3ca203205be) )
	ROM_LOAD16_BYTE( "gn-05.3k", 0x00001, 0x20000, CRC(a002cbe4) SHA1(76f57e49fc41a779856f70feb14432a8ffd08bff) )
	ROM_LOAD16_BYTE( "gn-01.1j", 0x40000, 0x20000, CRC(18f0527c) SHA1(17b7ea68909c7c8b819578e2039f5be4a640ea75) )
	ROM_LOAD16_BYTE( "gn-04.3j", 0x40001, 0x20000, CRC(ea4b6d53) SHA1(263319750524756319587b6e51dfead0265809cb) )
	ROM_LOAD16_BYTE( "gn-00.1h", 0x80000, 0x20000, CRC(0b110b16) SHA1(a967c8aeae3f0cee1f354583cf26ee736636aaf8) )
	ROM_LOAD16_BYTE( "gn-03.1k", 0x80001, 0x20000, CRC(1e28e697) SHA1(2313e97f3a34892dfdc338944c0f00538fcae800) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gl-07.13k",  0x00000, 0x10000, CRC(ca8bef96) SHA1(fcdbd598c85e339a3389a2ef58cf2d5b3a2779af) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gl-08.6y", 0x00001, 0x10000, CRC(33a2b400) SHA1(fdb8de315f33705719c0ac03a61fb56ffbfdf597) )
	ROM_LOAD16_BYTE( "gl-09.6z", 0x00000, 0x10000, CRC(5a2d4752) SHA1(617dd10a99b5b55ca64dcdd22a0f133b0d6b770d) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mag-02.4z", 0x000000, 0x80000, CRC(de89c69a) SHA1(b41bdf859854b5541c7eae7cd541b910cea1f839) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mag-00.1y", 0x000000, 0x40000, CRC(a8f05d33) SHA1(a1330bc9ca4648219403db087622badfc632b47d) )
	ROM_CONTINUE(          0x080000, 0x40000 )
	ROM_LOAD( "mag-01.1z", 0x040000, 0x40000, CRC(5b399eed) SHA1(490f8f9c0c557b0ba94c6019e3fe680641a0787e) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )

	ROM_REGION( 0x200000, "sprites1", 0 )   /* sprites */
	ROM_LOAD( "mag-05.12y", 0x000000, 0x80000, CRC(56a53254) SHA1(10940cfdc6fbe9013865107de3394ca7f782d9c7) )
	ROM_LOAD( "mag-06.12z", 0x080000, 0x80000, CRC(82d44749) SHA1(c471fa573e00c2f8ae44068439ba6d849a124c68) )
	ROM_LOAD( "mag-03.9y",  0x100000, 0x80000, CRC(2220eb9f) SHA1(bdf0bd6e6ba375f0770b9d08a7efa32201cbb6ef) )
	ROM_LOAD( "mag-04.9z",  0x180000, 0x80000, CRC(144b94cc) SHA1(d982508608942a714b428a2b721bf24e1627cbb6) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gl-06.13j", 0x00000, 0x20000, CRC(d92e519d) SHA1(08238f12bf7058a3965ab6348b468e3d35d4cd23) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mag-07.13f", 0x00000, 0x80000, CRC(08eb5264) SHA1(3e33085f00b758acfc78034dc9a75fd6921fc3fe) )  /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "mb7122h.7v", 0x00000, 0x400, CRC(a1267336) SHA1(d11ea9d78526ac3c0dc6e57a2da5914273ad1e3f) )    /* Priority  Unused */

	ROM_REGION( 0x0e00, "plds", 0 )
	ROM_LOAD( "tj-00.9j", 0x0000, 0x0117, CRC(46defe8f) SHA1(50db2c265a0ab592938f780f212ef86070b2daa2) ) /* GAL16V8 */
	ROM_LOAD( "tj-01.9h", 0x0200, 0x0117, CRC(7a86902d) SHA1(7e116dbabe615ddae1588001b31a0a6e6e4dc46d) ) /* GAL16V8 */
	ROM_LOAD( "tj-02.9h", 0x0400, 0x0117, CRC(b476d59c) SHA1(c17b7884180b5041d1524e9fd479cddac787a3cb) ) /* GAL16V8 */
	ROM_LOAD( "tj-03.9e", 0x0600, 0x0117, CRC(cfb6e4aa) SHA1(fc70d1d43d8836ae7984c432b3f9e35c2256b18e) ) /* GAL16V8 */
	ROM_LOAD( "tj-04.5n", 0x0800, 0x0117, CRC(bca07086) SHA1(cf713185d7430e17077a95a58a42d28336432c95) ) /* GAL16V8 */
	ROM_LOAD( "tj-05.1r", 0x0a00, 0x0117, CRC(0dfc091b) SHA1(ab7ff0c8ada10633d27d5305f1f41f738647412a) ) /* GAL16V8 */
ROM_END

ROM_START( cninjau ) /* US ver 4 */
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gm-02-3.1k", 0x00000, 0x20000, CRC(d931c3b1) SHA1(336390072a3a085fc534d9e2443c76104093b24f) )
	ROM_LOAD16_BYTE( "gm-05-2.3k", 0x00001, 0x20000, CRC(7417d3fb) SHA1(24c65101585955d56440b63a307021b5c137d7b9) )
	ROM_LOAD16_BYTE( "gm-01-2.1j", 0x40000, 0x20000, CRC(72041f7e) SHA1(cad62d6f3d77e361c7bb642401544baf01aec40d) )
	ROM_LOAD16_BYTE( "gm-04-2.3j", 0x40001, 0x20000, CRC(2104d005) SHA1(7fcb33745f1200024a05feb87a35b82de6030bd2) )
	ROM_LOAD16_BYTE( "gn-00.1h",   0x80000, 0x20000, CRC(0b110b16) SHA1(a967c8aeae3f0cee1f354583cf26ee736636aaf8) )
	ROM_LOAD16_BYTE( "gn-03.1k",   0x80001, 0x20000, CRC(1e28e697) SHA1(2313e97f3a34892dfdc338944c0f00538fcae800) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gl-07.13k",  0x00000, 0x10000, CRC(ca8bef96) SHA1(fcdbd598c85e339a3389a2ef58cf2d5b3a2779af) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gl-08.6y", 0x00001, 0x10000, CRC(33a2b400) SHA1(fdb8de315f33705719c0ac03a61fb56ffbfdf597) )
	ROM_LOAD16_BYTE( "gl-09.6z", 0x00000, 0x10000, CRC(5a2d4752) SHA1(617dd10a99b5b55ca64dcdd22a0f133b0d6b770d) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mag-02.4z", 0x000000, 0x80000, CRC(de89c69a) SHA1(b41bdf859854b5541c7eae7cd541b910cea1f839) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mag-00.1y", 0x000000, 0x40000, CRC(a8f05d33) SHA1(a1330bc9ca4648219403db087622badfc632b47d) )
	ROM_CONTINUE(          0x080000, 0x40000 )
	ROM_LOAD( "mag-01.1z", 0x040000, 0x40000, CRC(5b399eed) SHA1(490f8f9c0c557b0ba94c6019e3fe680641a0787e) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )

	ROM_REGION( 0x200000, "sprites1", 0 )   /* sprites */
	ROM_LOAD( "mag-05.12y", 0x000000, 0x80000, CRC(56a53254) SHA1(10940cfdc6fbe9013865107de3394ca7f782d9c7) )
	ROM_LOAD( "mag-06.12z", 0x080000, 0x80000, CRC(82d44749) SHA1(c471fa573e00c2f8ae44068439ba6d849a124c68) )
	ROM_LOAD( "mag-03.9y",  0x100000, 0x80000, CRC(2220eb9f) SHA1(bdf0bd6e6ba375f0770b9d08a7efa32201cbb6ef) )
	ROM_LOAD( "mag-04.9z",  0x180000, 0x80000, CRC(144b94cc) SHA1(d982508608942a714b428a2b721bf24e1627cbb6) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gl-06.13j", 0x00000, 0x20000, CRC(d92e519d) SHA1(08238f12bf7058a3965ab6348b468e3d35d4cd23) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mag-07.13f", 0x00000, 0x80000, CRC(08eb5264) SHA1(3e33085f00b758acfc78034dc9a75fd6921fc3fe) )  /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "mb7122h.7v", 0x00000, 0x400, CRC(a1267336) SHA1(d11ea9d78526ac3c0dc6e57a2da5914273ad1e3f) )    /* Priority  Unused */

	ROM_REGION( 0x0e00, "plds", 0 )
	ROM_LOAD( "tj-00.9j", 0x0000, 0x0117, CRC(46defe8f) SHA1(50db2c265a0ab592938f780f212ef86070b2daa2) ) /* GAL16V8 */
	ROM_LOAD( "tj-01.9h", 0x0200, 0x0117, CRC(7a86902d) SHA1(7e116dbabe615ddae1588001b31a0a6e6e4dc46d) ) /* GAL16V8 */
	ROM_LOAD( "tj-02.9h", 0x0400, 0x0117, CRC(b476d59c) SHA1(c17b7884180b5041d1524e9fd479cddac787a3cb) ) /* GAL16V8 */
	ROM_LOAD( "tj-03.9e", 0x0600, 0x0117, CRC(cfb6e4aa) SHA1(fc70d1d43d8836ae7984c432b3f9e35c2256b18e) ) /* GAL16V8 */
	ROM_LOAD( "tj-04.5n", 0x0800, 0x0117, CRC(bca07086) SHA1(cf713185d7430e17077a95a58a42d28336432c95) ) /* GAL16V8 */
	ROM_LOAD( "tj-05.1r", 0x0a00, 0x0117, CRC(0dfc091b) SHA1(ab7ff0c8ada10633d27d5305f1f41f738647412a) ) /* GAL16V8 */
ROM_END

ROM_START( joemac ) /* Japan ver 1 */
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gl-02-2.1k", 0x00000, 0x20000, CRC(80da12e2) SHA1(1037ed56c15dbe1eb8bb8b70f0bc3affc1119782) )
	ROM_LOAD16_BYTE( "gl-05-2.3k", 0x00001, 0x20000, CRC(fe4dbbbb) SHA1(85a3c5470270ebfc695fc5e937cf133a33860bec) )
	ROM_LOAD16_BYTE( "gl-01-2.1j", 0x40000, 0x20000, CRC(0b245307) SHA1(839735c0739cebb7ac5e328aa8b69170f390b96e) )
	ROM_LOAD16_BYTE( "gl-04-2.3j", 0x40001, 0x20000, CRC(1b331f61) SHA1(7811c3c25bd17188ae9cc792e106b303ccb14cde) )
	ROM_LOAD16_BYTE( "gn-00.1h",   0x80000, 0x20000, CRC(0b110b16) SHA1(a967c8aeae3f0cee1f354583cf26ee736636aaf8) )
	ROM_LOAD16_BYTE( "gn-03.1k",   0x80001, 0x20000, CRC(1e28e697) SHA1(2313e97f3a34892dfdc338944c0f00538fcae800) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gl-07.13k",  0x00000, 0x10000, CRC(ca8bef96) SHA1(fcdbd598c85e339a3389a2ef58cf2d5b3a2779af) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gl-08.6y", 0x00001, 0x10000, CRC(33a2b400) SHA1(fdb8de315f33705719c0ac03a61fb56ffbfdf597) )
	ROM_LOAD16_BYTE( "gl-09.6z", 0x00000, 0x10000, CRC(5a2d4752) SHA1(617dd10a99b5b55ca64dcdd22a0f133b0d6b770d) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mag-02.4z", 0x000000, 0x80000, CRC(de89c69a) SHA1(b41bdf859854b5541c7eae7cd541b910cea1f839) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mag-00.1y", 0x000000, 0x40000, CRC(a8f05d33) SHA1(a1330bc9ca4648219403db087622badfc632b47d) )
	ROM_CONTINUE(          0x080000, 0x40000 )
	ROM_LOAD( "mag-01.1z", 0x040000, 0x40000, CRC(5b399eed) SHA1(490f8f9c0c557b0ba94c6019e3fe680641a0787e) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )

	ROM_REGION( 0x200000, "sprites1", 0 )   /* sprites */
	ROM_LOAD( "mag-05.12y", 0x000000, 0x80000, CRC(56a53254) SHA1(10940cfdc6fbe9013865107de3394ca7f782d9c7) )
	ROM_LOAD( "mag-06.12z", 0x080000, 0x80000, CRC(82d44749) SHA1(c471fa573e00c2f8ae44068439ba6d849a124c68) )
	ROM_LOAD( "mag-03.9y",  0x100000, 0x80000, CRC(2220eb9f) SHA1(bdf0bd6e6ba375f0770b9d08a7efa32201cbb6ef) )
	ROM_LOAD( "mag-04.9z",  0x180000, 0x80000, CRC(144b94cc) SHA1(d982508608942a714b428a2b721bf24e1627cbb6) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gl-06.13j", 0x00000, 0x20000, CRC(d92e519d) SHA1(08238f12bf7058a3965ab6348b468e3d35d4cd23) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mag-07.13f", 0x00000, 0x80000, CRC(08eb5264) SHA1(3e33085f00b758acfc78034dc9a75fd6921fc3fe) )  /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "mb7122h.7v", 0x00000, 0x400, CRC(a1267336) SHA1(d11ea9d78526ac3c0dc6e57a2da5914273ad1e3f) )    /* Priority  Unused */

	ROM_REGION( 0x0e00, "plds", 0 )
	ROM_LOAD( "tj-00.9j", 0x0000, 0x0117, CRC(46defe8f) SHA1(50db2c265a0ab592938f780f212ef86070b2daa2) ) /* GAL16V8 */
	ROM_LOAD( "tj-01.9h", 0x0200, 0x0117, CRC(7a86902d) SHA1(7e116dbabe615ddae1588001b31a0a6e6e4dc46d) ) /* GAL16V8 */
	ROM_LOAD( "tj-02.9h", 0x0400, 0x0117, CRC(b476d59c) SHA1(c17b7884180b5041d1524e9fd479cddac787a3cb) ) /* GAL16V8 */
	ROM_LOAD( "tj-03.9e", 0x0600, 0x0117, CRC(cfb6e4aa) SHA1(fc70d1d43d8836ae7984c432b3f9e35c2256b18e) ) /* GAL16V8 */
	ROM_LOAD( "tj-04.5n", 0x0800, 0x0117, CRC(bca07086) SHA1(cf713185d7430e17077a95a58a42d28336432c95) ) /* GAL16V8 */
	ROM_LOAD( "tj-05.1r", 0x0a00, 0x0117, CRC(0dfc091b) SHA1(ab7ff0c8ada10633d27d5305f1f41f738647412a) ) /* GAL16V8 */
ROM_END

ROM_START( stoneage )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "sa_1_019.bin", 0x00000, 0x20000,  CRC(7fb8c44f) SHA1(0167805793a4288f545c0a8ea66bd1ad82bac437) )
	ROM_LOAD16_BYTE( "sa_1_033.bin", 0x00001, 0x20000,  CRC(961c752b) SHA1(b9ac7882662f84de7309c46f8c9344693215d9f7) )
	ROM_LOAD16_BYTE( "sa_1_018.bin", 0x40000, 0x20000,  CRC(a4043022) SHA1(084e80eaf4ffd9243996615ed20b7debcd185754) )
	ROM_LOAD16_BYTE( "sa_1_032.bin", 0x40001, 0x20000,  CRC(f52a3286) SHA1(04bc64ddefd1c52c87fe653423fb1e15746b8abc) )
	ROM_LOAD16_BYTE( "sa_1_017.bin", 0x80000, 0x20000,  CRC(08d6397a) SHA1(ae3a50a043b3247545378611381c593b3ceeb561) )
	ROM_LOAD16_BYTE( "sa_1_031.bin", 0x80001, 0x20000,  CRC(103079f5) SHA1(7ed28ab957be14974badeaa23f570f99ada61633) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "sa_1_012.bin",  0x00000,  0x10000, CRC(56058934) SHA1(99a007884c92c2d931d9270c6c2ec02fbc913922) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gl-08.rom",  0x00001,  0x10000,  CRC(33a2b400) SHA1(fdb8de315f33705719c0ac03a61fb56ffbfdf597) )
	ROM_LOAD16_BYTE( "gl-09.rom",  0x00000,  0x10000,  CRC(5a2d4752) SHA1(617dd10a99b5b55ca64dcdd22a0f133b0d6b770d) )

	/* The bootleg graphics are stored in a different arrangement but
	    seem to be the same as the original set */

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mag-02.rom", 0x000000, 0x80000,  CRC(de89c69a) SHA1(b41bdf859854b5541c7eae7cd541b910cea1f839) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mag-00.rom", 0x000000, 0x40000,  CRC(a8f05d33) SHA1(a1330bc9ca4648219403db087622badfc632b47d) )
	ROM_CONTINUE(           0x080000, 0x40000 )
	ROM_LOAD( "mag-01.rom", 0x040000, 0x40000,  CRC(5b399eed) SHA1(490f8f9c0c557b0ba94c6019e3fe680641a0787e) )
	ROM_CONTINUE(           0x0c0000, 0x40000 )

	ROM_REGION( 0x200000, "sprites1", 0 )   /* sprites */
	ROM_LOAD( "mag-05.rom", 0x000000, 0x80000,  CRC(56a53254) SHA1(10940cfdc6fbe9013865107de3394ca7f782d9c7) )
	ROM_LOAD( "mag-06.rom", 0x080000, 0x80000,  CRC(82d44749) SHA1(c471fa573e00c2f8ae44068439ba6d849a124c68) )
	ROM_LOAD( "mag-03.rom", 0x100000, 0x80000,  CRC(2220eb9f) SHA1(bdf0bd6e6ba375f0770b9d08a7efa32201cbb6ef) )
	ROM_LOAD( "mag-04.rom", 0x180000, 0x80000,  CRC(144b94cc) SHA1(d982508608942a714b428a2b721bf24e1627cbb6) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "sa_1_069.bin",  0x00000,  0x40000, CRC(2188f3ca) SHA1(9c29b62ed261e63d701ff8d43020089c89a64ab2) )
ROM_END

ROM_START( cninjabl )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "joe mac 3.68k", 0x00000, 0x80000,  CRC(dc931d80) SHA1(78103f74fb428c4735e77d99a143cdf28915ef26) )
	ROM_LOAD16_WORD_SWAP( "joe mac 4.68k", 0x80000, 0x40000,  CRC(e8dfe0b5) SHA1(f7f883c19023bc68146aea5eaf98d2fdd606d5e3) )
	ROM_IGNORE(0x40000) //  1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Sound CPU */
	ROM_LOAD( "joe mac 5.z80",  0x00000,  0x10000, CRC(d791b9d7) SHA1(7842ab7e960b692bdbcadf5c64f09ddd1a3fb861) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x400000, "gfxtemp", 0 ) // the bootleg has the gfx in 2 roms
	ROM_LOAD( "joe mac 1.gfx",  0x200000,  0x200000,  CRC(17ea5931) SHA1(cb686dea0d960d35ab3709f1f592598c2d757045) )
	ROM_LOAD( "joe mac 2.gfx",  0x000000,  0x200000,  CRC(cc95317b) SHA1(ffa97dde954f73d8e0f6e55387b44f5bcc08242b) )

	// split larger bootleg GFX roms into required regions
	ROM_REGION( 0x020000, "chars", ROMREGION_INVERT ) // chars
	ROM_COPY( "gfxtemp", 0x000000, 0x000000, 0x010000 )
	ROM_COPY( "gfxtemp", 0x200000, 0x010000, 0x010000 )

	ROM_REGION( 0x080000, "tiles1", ROMREGION_INVERT ) // tiles 3
	ROM_COPY( "gfxtemp", 0x040000, 0x000000, 0x040000 )
	ROM_COPY( "gfxtemp", 0x240000, 0x040000, 0x040000 )

	ROM_REGION( 0x100000, "tiles2", ROMREGION_INVERT ) // tiles 2
	ROM_COPY( "gfxtemp", 0x0c0000, 0x000000, 0x040000 )
	ROM_COPY( "gfxtemp", 0x080000, 0x040000, 0x040000 )
	ROM_COPY( "gfxtemp", 0x2c0000, 0x080000, 0x040000 )
	ROM_COPY( "gfxtemp", 0x280000, 0x0c0000, 0x040000 )

	ROM_REGION( 0x200000, "sprites1", 0 ) // sprites
	ROM_COPY( "gfxtemp", 0x100000, 0x000000, 0x100000 )
	ROM_COPY( "gfxtemp", 0x300000, 0x100000, 0x100000 )

	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "joe mac 6.samples",  0x00000,  0x80000, CRC(dbecad83) SHA1(de34653606f12d2c606ff7d1cbce993521772884) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

ROM_START( cninjabl2 )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "6.bin", 0x00001, 0x40000, CRC(a86ccfb7) SHA1(c4ac331d5750a35cd48e5d75f0247758b892b034) )
	ROM_LOAD16_BYTE( "4.bin", 0x00000, 0x40000, CRC(88c7043a) SHA1(9981d10f6c88556ceda083158d9835fdeb191511) )
	ROM_LOAD16_BYTE( "gn-00.rom",  0x80000, 0x20000, CRC(0b110b16) SHA1(a967c8aeae3f0cee1f354583cf26ee736636aaf8) )
	ROM_LOAD16_BYTE( "gn-03.rom",  0x80001, 0x20000, CRC(1e28e697) SHA1(2313e97f3a34892dfdc338944c0f00538fcae800) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "audio-prg.3",  0x00000,  0x8000,  CRC(3eb65b6d) SHA1(e6d94223a7b98d33470ad4e387d6ce399b76ea4a) ) // first half empty
	ROM_CONTINUE(               0x00000,  0x8000 )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gl-08.rom",  0x00001,  0x10000,  CRC(33a2b400) SHA1(fdb8de315f33705719c0ac03a61fb56ffbfdf597) )
	ROM_LOAD16_BYTE( "gl-09.rom",  0x00000,  0x10000,  CRC(5a2d4752) SHA1(617dd10a99b5b55ca64dcdd22a0f133b0d6b770d) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mag-02.rom", 0x000000, 0x80000,  CRC(de89c69a) SHA1(b41bdf859854b5541c7eae7cd541b910cea1f839) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mag-00.rom", 0x000000, 0x40000,  CRC(a8f05d33) SHA1(a1330bc9ca4648219403db087622badfc632b47d) )
	ROM_CONTINUE(           0x080000, 0x40000 )
	ROM_LOAD( "mag-01.rom", 0x040000, 0x40000,  CRC(5b399eed) SHA1(490f8f9c0c557b0ba94c6019e3fe680641a0787e) )
	ROM_CONTINUE(           0x0c0000, 0x40000 )

	ROM_REGION( 0x200000, "sprites1", 0 )   /* sprites */
	ROM_LOAD( "mag-05.rom", 0x000000, 0x80000,  CRC(56a53254) SHA1(10940cfdc6fbe9013865107de3394ca7f782d9c7) )
	ROM_LOAD( "mag-06.rom", 0x080000, 0x80000,  CRC(82d44749) SHA1(c471fa573e00c2f8ae44068439ba6d849a124c68) )
	ROM_LOAD( "mag-03.rom", 0x100000, 0x80000,  CRC(2220eb9f) SHA1(bdf0bd6e6ba375f0770b9d08a7efa32201cbb6ef) )
	ROM_LOAD( "mag-04.rom", 0x180000, 0x80000,  CRC(144b94cc) SHA1(d982508608942a714b428a2b721bf24e1627cbb6) )

	ROM_REGION( 0x30000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "audio-samp.2",  0x00000,  0x20000,  CRC(c6638568) SHA1(b5e38d807146b033d1a0b5fb013ac755cd4a2699) )
	ROM_LOAD( "audio-samp.1",  0x20000,  0x10000,  CRC(7815e6ab) SHA1(3112b4e8a4008b519f73e6f2d1393ef1e620a0c5) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "audio-samp.18", 0x00000,  0x80000,  CRC(06f1bc18) SHA1(fe551d78466dc5b098263520f0ab00200d651593) )   /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "mb7122h.7v", 0x00000,  0x400,  CRC(a1267336) SHA1(d11ea9d78526ac3c0dc6e57a2da5914273ad1e3f) )        /* Priority  Unused */
ROM_END

ROM_START( edrandy ) /* World ver 3 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gg-00-2.k1", 0x00000, 0x20000, CRC(ce1ba964) SHA1(da21734721344eff41a64a7f2382d5c027a24782) )
	ROM_LOAD16_BYTE( "gg-04-2.k3", 0x00001, 0x20000, CRC(24caed19) SHA1(bdca689dbb13685e71d3385a9ff7b356d2459d45) )
	ROM_LOAD16_BYTE( "gg-01-2.j1", 0x40000, 0x20000, CRC(33677b80) SHA1(d16b926053a61723d321a50f5cabf3e5faebadcf) )
	ROM_LOAD16_BYTE( "gg-05-2.j3", 0x40001, 0x20000, CRC(79a68ca6) SHA1(b1ec168ffe7aace481055a8f38d88ed71994191d) )
	ROM_LOAD16_BYTE( "ge-02.h1",   0x80000, 0x20000, CRC(c2969fbb) SHA1(faa7da7f5271108dbbc95d111caa2c986e494933) )
	ROM_LOAD16_BYTE( "ge-06.h3",   0x80001, 0x20000, CRC(5c2e6418) SHA1(b9ed769b27c37959fcba2acd6dba02ccd62149e7) )
	ROM_LOAD16_BYTE( "ge-03.f1",   0xc0000, 0x20000, CRC(5e7b19a8) SHA1(637945e36c3665c74d31f4b14e600e93ed9be054) )
	ROM_LOAD16_BYTE( "ge-07.f3",   0xc0001, 0x20000, CRC(5eb819a1) SHA1(1852cb624eccd0a424d404bc853b5df307875cc9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ge-09.k13",    0x00000, 0x10000, CRC(9f94c60b) SHA1(56edf63850189b2168c602e1f21492ef14662682) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gg-10.y6",    0x000001, 0x10000, CRC(b96c6cbe) SHA1(1f3a18387f360705d2f2ab8f5780a270621e107f) )
	ROM_LOAD16_BYTE( "gg-11.z6",    0x000000, 0x10000, CRC(ee567448) SHA1(40c673535b9edf7b8bbb4912235bbb09ef77e221) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mad-00",   0x000000, 0x40000, CRC(3735b22d) SHA1(fd9c3dc7a880592104c091730b9016641043987a) )
	ROM_CONTINUE(         0x080000, 0x40000 )
	ROM_LOAD( "mad-01",   0x040000, 0x40000, CRC(7bb13e1c) SHA1(2753e0345b746bb4e8a5572d057d0b888487cbc3) )
	ROM_CONTINUE(         0x0c0000, 0x40000 )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mad-02",   0x000000, 0x80000, CRC(6c76face) SHA1(e485b118e1e5bdf130c7ae29eea2f192f85f93a5) )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "mad-05",   0x000000, 0x80000, CRC(3f2ccf95) SHA1(ed9585f2162ca53a7621b86a9def45a46cd08331) )
	ROM_LOAD( "mad-06",   0x080000, 0x80000, CRC(60871f77) SHA1(45d3042986fba76951438fd69545a2a48e478a6a) )
	ROM_LOAD( "mad-08",   0x100000, 0x80000, CRC(1b420ec8) SHA1(291119e16121fc81f982216772dc6d8bb3b3b12d) )
	ROM_LOAD( "mad-11",   0x180000, 0x80000, CRC(03c1f982) SHA1(1cc63e4e96356d1d281b254c3b7de009866e865b) )
	ROM_LOAD( "mad-12",   0x200000, 0x80000, CRC(a0bd62b6) SHA1(a1ab365f3c63fc4edc32b09ecf8f982beb4bfae5) )
	ROM_LOAD( "mad-03",   0x280000, 0x80000, CRC(c0bff892) SHA1(a3f3bdcc68f6183031438c0572e1e7c2c6fafb6a) )
	ROM_LOAD( "mad-04",   0x300000, 0x80000, CRC(464f3eb9) SHA1(c86212f37e4ca97ef71680643487f1e2afb7ec8a) )
	ROM_LOAD( "mad-07",   0x380000, 0x80000, CRC(ac03466e) SHA1(bfaa779f1818d8cd2b7de7a6ad1c2c396ce7309e) )
	ROM_LOAD( "mad-10",   0x400000, 0x80000, CRC(42da8ef0) SHA1(704a154db952e89c13a1bd115bdb57d5a6da479a) )
	ROM_LOAD( "mad-09",   0x480000, 0x80000, CRC(930f4900) SHA1(dd09d3c8a251b8397996f6a3330e6e704f65d7fa) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "ge-08.j13",    0x00000, 0x20000, CRC(dfe28c7b) SHA1(aba55834b276cbab194e03858564077cad21eff1) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mad-13", 0x00000, 0x80000, CRC(6ab28eba) SHA1(12d3025478ee5af4bdea037656d9b1146cd9759f) )    /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "ge-12.v7", 0x00000,  0x400,  CRC(278f674f) SHA1(d4f5b9770d6d2ddebf1b999e291c80a3e274d492) )  /* Priority Unused, same as Robocop 2 */
ROM_END

ROM_START( edrandy2 ) /* World ver 2 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gg00-1.k1", 0x00000, 0x20000, CRC(a029cc4a) SHA1(3801fd6df6d1299972eeadbdbba1b0b7acf89139) )
	ROM_LOAD16_BYTE( "gg04-1.k3", 0x00001, 0x20000, CRC(8b7928a4) SHA1(4075713a830c9d5e324bb790468ec555fa747106) )
	ROM_LOAD16_BYTE( "gg01-1.j1", 0x40000, 0x20000, CRC(84360123) SHA1(3e9241cf68839c15d7a1209fe735b51ed90a1de7) )
	ROM_LOAD16_BYTE( "gg05-1.j3", 0x40001, 0x20000, CRC(0bf85d9d) SHA1(7b7c1c32d3f0de7e675cea3d2ba4f28e9ce387a9) )
	ROM_LOAD16_BYTE( "ge-02.h1",  0x80000, 0x20000, CRC(c2969fbb) SHA1(faa7da7f5271108dbbc95d111caa2c986e494933) )
	ROM_LOAD16_BYTE( "ge-06.h3",  0x80001, 0x20000, CRC(5c2e6418) SHA1(b9ed769b27c37959fcba2acd6dba02ccd62149e7) )
	ROM_LOAD16_BYTE( "ge-03.f1",  0xc0000, 0x20000, CRC(5e7b19a8) SHA1(637945e36c3665c74d31f4b14e600e93ed9be054) )
	ROM_LOAD16_BYTE( "ge-07.f3",  0xc0001, 0x20000, CRC(5eb819a1) SHA1(1852cb624eccd0a424d404bc853b5df307875cc9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ge-09.k13",    0x00000, 0x10000, CRC(9f94c60b) SHA1(56edf63850189b2168c602e1f21492ef14662682) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gg-10.y6",    0x000001, 0x10000, CRC(b96c6cbe) SHA1(1f3a18387f360705d2f2ab8f5780a270621e107f) )
	ROM_LOAD16_BYTE( "gg-11.z6",    0x000000, 0x10000, CRC(ee567448) SHA1(40c673535b9edf7b8bbb4912235bbb09ef77e221) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mad-00",   0x000000, 0x40000, CRC(3735b22d) SHA1(fd9c3dc7a880592104c091730b9016641043987a) )
	ROM_CONTINUE(         0x080000, 0x40000 )
	ROM_LOAD( "mad-01",   0x040000, 0x40000, CRC(7bb13e1c) SHA1(2753e0345b746bb4e8a5572d057d0b888487cbc3) )
	ROM_CONTINUE(         0x0c0000, 0x40000 )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mad-02",   0x000000, 0x80000, CRC(6c76face) SHA1(e485b118e1e5bdf130c7ae29eea2f192f85f93a5) )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "mad-05",   0x000000, 0x80000, CRC(3f2ccf95) SHA1(ed9585f2162ca53a7621b86a9def45a46cd08331) )
	ROM_LOAD( "mad-06",   0x080000, 0x80000, CRC(60871f77) SHA1(45d3042986fba76951438fd69545a2a48e478a6a) )
	ROM_LOAD( "mad-08",   0x100000, 0x80000, CRC(1b420ec8) SHA1(291119e16121fc81f982216772dc6d8bb3b3b12d) )
	ROM_LOAD( "mad-11",   0x180000, 0x80000, CRC(03c1f982) SHA1(1cc63e4e96356d1d281b254c3b7de009866e865b) )
	ROM_LOAD( "mad-12",   0x200000, 0x80000, CRC(a0bd62b6) SHA1(a1ab365f3c63fc4edc32b09ecf8f982beb4bfae5) )
	ROM_LOAD( "mad-03",   0x280000, 0x80000, CRC(c0bff892) SHA1(a3f3bdcc68f6183031438c0572e1e7c2c6fafb6a) )
	ROM_LOAD( "mad-04",   0x300000, 0x80000, CRC(464f3eb9) SHA1(c86212f37e4ca97ef71680643487f1e2afb7ec8a) )
	ROM_LOAD( "mad-07",   0x380000, 0x80000, CRC(ac03466e) SHA1(bfaa779f1818d8cd2b7de7a6ad1c2c396ce7309e) )
	ROM_LOAD( "mad-10",   0x400000, 0x80000, CRC(42da8ef0) SHA1(704a154db952e89c13a1bd115bdb57d5a6da479a) )
	ROM_LOAD( "mad-09",   0x480000, 0x80000, CRC(930f4900) SHA1(dd09d3c8a251b8397996f6a3330e6e704f65d7fa) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "ge-08.j13",    0x00000, 0x20000, CRC(dfe28c7b) SHA1(aba55834b276cbab194e03858564077cad21eff1) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mad-13", 0x00000, 0x80000, CRC(6ab28eba) SHA1(12d3025478ee5af4bdea037656d9b1146cd9759f) )    /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "ge-12.v7", 0x00000,  0x400,  CRC(278f674f) SHA1(d4f5b9770d6d2ddebf1b999e291c80a3e274d492) )  /* Priority Unused, same as Robocop 2 */
ROM_END

ROM_START( edrandy1 ) /* World ver 1 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "1.k1",     0x00000, 0x20000, CRC(f184cdaa) SHA1(7d4a1e8acf6737a9d74d78eb414f32885ffa9846) ) /* roms were simply labeled 1 through 12 */
	ROM_LOAD16_BYTE( "5.k3",     0x00001, 0x20000, CRC(7e3a4b81) SHA1(e768dd710a8b38add9fd8d9bfc88ad3a3c353ba5) )
	ROM_LOAD16_BYTE( "2.j1",     0x40000, 0x20000, CRC(212cd593) SHA1(2f4feeffa1c4a5f1345d78586a303a85fd365c23) )
	ROM_LOAD16_BYTE( "6.j3",     0x40001, 0x20000, CRC(4a96fb07) SHA1(5b7f46b2fa6ef947e0467f31ecca04877318ead4) )
	ROM_LOAD16_BYTE( "ge-02.h1", 0x80000, 0x20000, CRC(c2969fbb) SHA1(faa7da7f5271108dbbc95d111caa2c986e494933) ) /* labeled as "3" */
	ROM_LOAD16_BYTE( "ge-06.h3", 0x80001, 0x20000, CRC(5c2e6418) SHA1(b9ed769b27c37959fcba2acd6dba02ccd62149e7) ) /* labeled as "7" */
	ROM_LOAD16_BYTE( "ge-03.f1", 0xc0000, 0x20000, CRC(5e7b19a8) SHA1(637945e36c3665c74d31f4b14e600e93ed9be054) ) /* labeled as "4" */
	ROM_LOAD16_BYTE( "ge-07.f3", 0xc0001, 0x20000, CRC(5eb819a1) SHA1(1852cb624eccd0a424d404bc853b5df307875cc9) ) /* labeled as "8" */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ge-09.k13",    0x00000, 0x10000, CRC(9f94c60b) SHA1(56edf63850189b2168c602e1f21492ef14662682) ) /* labeled as "9" */

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */ /* Original graphics, later revised for the World sets above?? */
	ROM_LOAD16_BYTE( "ge-10.y6",    0x000001, 0x10000, CRC(2528d795) SHA1(8081b5d13875287a75f868a0566a2d06e0e42949) ) /* labeled as "12" */
	ROM_LOAD16_BYTE( "ge-11.z6",    0x000000, 0x10000, CRC(e34a931e) SHA1(0e06359347e48d53ee96d6551d34685110b0f5fb) ) /* labeled as "11" */

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mad-00",   0x000000, 0x40000, CRC(3735b22d) SHA1(fd9c3dc7a880592104c091730b9016641043987a) )
	ROM_CONTINUE(         0x080000, 0x40000 )
	ROM_LOAD( "mad-01",   0x040000, 0x40000, CRC(7bb13e1c) SHA1(2753e0345b746bb4e8a5572d057d0b888487cbc3) )
	ROM_CONTINUE(         0x0c0000, 0x40000 )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mad-02",   0x000000, 0x80000, CRC(6c76face) SHA1(e485b118e1e5bdf130c7ae29eea2f192f85f93a5) )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "mad-05",   0x000000, 0x80000, CRC(3f2ccf95) SHA1(ed9585f2162ca53a7621b86a9def45a46cd08331) )
	ROM_LOAD( "mad-06",   0x080000, 0x80000, CRC(60871f77) SHA1(45d3042986fba76951438fd69545a2a48e478a6a) )
	ROM_LOAD( "mad-08",   0x100000, 0x80000, CRC(1b420ec8) SHA1(291119e16121fc81f982216772dc6d8bb3b3b12d) )
	ROM_LOAD( "mad-11",   0x180000, 0x80000, CRC(03c1f982) SHA1(1cc63e4e96356d1d281b254c3b7de009866e865b) )
	ROM_LOAD( "mad-12",   0x200000, 0x80000, CRC(a0bd62b6) SHA1(a1ab365f3c63fc4edc32b09ecf8f982beb4bfae5) )
	ROM_LOAD( "mad-03",   0x280000, 0x80000, CRC(c0bff892) SHA1(a3f3bdcc68f6183031438c0572e1e7c2c6fafb6a) )
	ROM_LOAD( "mad-04",   0x300000, 0x80000, CRC(464f3eb9) SHA1(c86212f37e4ca97ef71680643487f1e2afb7ec8a) )
	ROM_LOAD( "mad-07",   0x380000, 0x80000, CRC(ac03466e) SHA1(bfaa779f1818d8cd2b7de7a6ad1c2c396ce7309e) )
	ROM_LOAD( "mad-10",   0x400000, 0x80000, CRC(42da8ef0) SHA1(704a154db952e89c13a1bd115bdb57d5a6da479a) )
	ROM_LOAD( "mad-09",   0x480000, 0x80000, CRC(930f4900) SHA1(dd09d3c8a251b8397996f6a3330e6e704f65d7fa) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "ge-08.j13",    0x00000, 0x20000, CRC(dfe28c7b) SHA1(aba55834b276cbab194e03858564077cad21eff1) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mad-13", 0x00000, 0x80000, CRC(6ab28eba) SHA1(12d3025478ee5af4bdea037656d9b1146cd9759f) )    /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "ge-12.v7", 0x00000,  0x400,  CRC(278f674f) SHA1(d4f5b9770d6d2ddebf1b999e291c80a3e274d492) )  /* Priority Unused, same as Robocop 2 */
ROM_END

ROM_START( edrandyj ) /* Japan ver 3 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ge-00-2.k1",   0x00000, 0x20000, CRC(b3d2403c) SHA1(9747dbe7905e1453e3e7764c874c523c54970e2e) )
	ROM_LOAD16_BYTE( "ge-04-2.k3",   0x00001, 0x20000, CRC(8a9624d6) SHA1(d5a9b56bc8a1d67fa28df95299cb205e9c965310) )
	ROM_LOAD16_BYTE( "ge-01-2.j1",   0x40000, 0x20000, CRC(84360123) SHA1(3e9241cf68839c15d7a1209fe735b51ed90a1de7) )
	ROM_LOAD16_BYTE( "ge-05-2.j3",   0x40001, 0x20000, CRC(0bf85d9d) SHA1(7b7c1c32d3f0de7e675cea3d2ba4f28e9ce387a9) )
	ROM_LOAD16_BYTE( "ge-02.h1",     0x80000, 0x20000, CRC(c2969fbb) SHA1(faa7da7f5271108dbbc95d111caa2c986e494933) )
	ROM_LOAD16_BYTE( "ge-06.h3",     0x80001, 0x20000, CRC(5c2e6418) SHA1(b9ed769b27c37959fcba2acd6dba02ccd62149e7) )
	ROM_LOAD16_BYTE( "ge-03.f1",     0xc0000, 0x20000, CRC(5e7b19a8) SHA1(637945e36c3665c74d31f4b14e600e93ed9be054) )
	ROM_LOAD16_BYTE( "ge-07.f3",     0xc0001, 0x20000, CRC(5eb819a1) SHA1(1852cb624eccd0a424d404bc853b5df307875cc9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ge-09.k13",    0x00000, 0x10000, CRC(9f94c60b) SHA1(56edf63850189b2168c602e1f21492ef14662682) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "ge-10.y6",    0x000001, 0x10000, CRC(2528d795) SHA1(8081b5d13875287a75f868a0566a2d06e0e42949) )
	ROM_LOAD16_BYTE( "ge-11.z6",    0x000000, 0x10000, CRC(e34a931e) SHA1(0e06359347e48d53ee96d6551d34685110b0f5fb) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mad-00",   0x000000, 0x40000, CRC(3735b22d) SHA1(fd9c3dc7a880592104c091730b9016641043987a) )
	ROM_CONTINUE(         0x080000, 0x40000 )
	ROM_LOAD( "mad-01",   0x040000, 0x40000, CRC(7bb13e1c) SHA1(2753e0345b746bb4e8a5572d057d0b888487cbc3) )
	ROM_CONTINUE(         0x0c0000, 0x40000 )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mad-02",   0x000000, 0x80000, CRC(6c76face) SHA1(e485b118e1e5bdf130c7ae29eea2f192f85f93a5) )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "mad-05",   0x000000, 0x80000, CRC(3f2ccf95) SHA1(ed9585f2162ca53a7621b86a9def45a46cd08331) )
	ROM_LOAD( "mad-06",   0x080000, 0x80000, CRC(60871f77) SHA1(45d3042986fba76951438fd69545a2a48e478a6a) )
	ROM_LOAD( "mad-08",   0x100000, 0x80000, CRC(1b420ec8) SHA1(291119e16121fc81f982216772dc6d8bb3b3b12d) )
	ROM_LOAD( "mad-11",   0x180000, 0x80000, CRC(03c1f982) SHA1(1cc63e4e96356d1d281b254c3b7de009866e865b) )
	ROM_LOAD( "mad-12",   0x200000, 0x80000, CRC(a0bd62b6) SHA1(a1ab365f3c63fc4edc32b09ecf8f982beb4bfae5) )
	ROM_LOAD( "mad-03",   0x280000, 0x80000, CRC(c0bff892) SHA1(a3f3bdcc68f6183031438c0572e1e7c2c6fafb6a) )
	ROM_LOAD( "mad-04",   0x300000, 0x80000, CRC(464f3eb9) SHA1(c86212f37e4ca97ef71680643487f1e2afb7ec8a) )
	ROM_LOAD( "mad-07",   0x380000, 0x80000, CRC(ac03466e) SHA1(bfaa779f1818d8cd2b7de7a6ad1c2c396ce7309e) )
	ROM_LOAD( "mad-10",   0x400000, 0x80000, CRC(42da8ef0) SHA1(704a154db952e89c13a1bd115bdb57d5a6da479a) )
	ROM_LOAD( "mad-09",   0x480000, 0x80000, CRC(930f4900) SHA1(dd09d3c8a251b8397996f6a3330e6e704f65d7fa) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "ge-08.j13",    0x00000, 0x20000, CRC(dfe28c7b) SHA1(aba55834b276cbab194e03858564077cad21eff1) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mad-13", 0x00000, 0x80000, CRC(6ab28eba) SHA1(12d3025478ee5af4bdea037656d9b1146cd9759f) )    /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "ge-12.v7", 0x00000,  0x400,  CRC(278f674f) SHA1(d4f5b9770d6d2ddebf1b999e291c80a3e274d492) )  /* Priority Unused, same as Robocop 2 */
ROM_END

ROM_START( robocop2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gq-03.k1",   0x00000, 0x20000, CRC(a7e90c28) SHA1(e1ff720e4e63de3adc94505a566e7340f65567d5) )
	ROM_LOAD16_BYTE( "gq-07.k3",   0x00001, 0x20000, CRC(d2287ec1) SHA1(8f596205c69b0ed3974cb0bd17fcc3b3bf47a0ca) )
	ROM_LOAD16_BYTE( "gq-02.j1",   0x40000, 0x20000, CRC(6777b8a0) SHA1(9081bd187c3b5923efab3e4abde952e9ab29d946) )
	ROM_LOAD16_BYTE( "gq-06.j3",   0x40001, 0x20000, CRC(e11e27b5) SHA1(03570da040b7cef2cecebce51b27f8a8fcf62eb1) )
	ROM_LOAD16_BYTE( "go-01-1.h1", 0x80000, 0x20000, CRC(ab5356c0) SHA1(297a89b4d9212c916745997bbb959b0ed660f909) )
	ROM_LOAD16_BYTE( "go-05-1.h3", 0x80001, 0x20000, CRC(ce21bda5) SHA1(615701d4abdb56d50da44589e6e03909f4b28d45) )
	ROM_LOAD16_BYTE( "go-00.f1",   0xc0000, 0x20000, CRC(a93369ea) SHA1(9e13c36112eb7ebc97dc919e24d0b2955c57e10e) )
	ROM_LOAD16_BYTE( "go-04.f3",   0xc0001, 0x20000, CRC(ee2f6ad9) SHA1(3abc07792f444a3415fd32e50d6855843e900b1d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gp-09.k13",  0x00000,  0x10000,  CRC(4a4e0f8d) SHA1(5408465667d2854bbade23a26ba619d42a0c22f8) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gp10-1.y6",  0x00001,  0x10000,  CRC(d25d719c) SHA1(be874cf403ec0e607eb9b54b7cfff0a53f4d59a2) )
	ROM_LOAD16_BYTE( "gp11-1.z6",  0x00000,  0x10000,  CRC(030ded47) SHA1(59ded540b2601ec37255e871e38ac71a47c8d007) )

	ROM_REGION( 0x100000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mah-04.z4", 0x000000, 0x40000,  CRC(9b6ca18c) SHA1(29a20200ea50b9e8e79da072c1b6e580e6ca180f) )
	ROM_CONTINUE(          0x080000, 0x40000 )
	ROM_LOAD( "mah-03.y4", 0x040000, 0x40000,  CRC(37894ddc) SHA1(ee08440b3b2023ec6ee2af6d509b642bcead2e60) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )

	ROM_REGION( 0x180000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mah-01.z1", 0x000000, 0x40000,  CRC(26e0dfff) SHA1(8cca2dbcda64f4bc6ee0842486da7dc7df3046fd) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )
	ROM_LOAD( "mah-00.y1", 0x040000, 0x40000,  CRC(7bd69e41) SHA1(296adbf7d40f1092bf38599b3bad51f39d8093b2) )
	ROM_CONTINUE(          0x100000, 0x40000 )
	ROM_LOAD( "mah-02.a1", 0x080000, 0x40000,  CRC(328a247d) SHA1(879f75452dc7c327fd5b35c960c58bc0c0efd33c) )
	ROM_CONTINUE(          0x140000, 0x40000 )

	ROM_REGION( 0x300000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "mah-08.y12", 0x000000, 0x80000,  CRC(88d310a5) SHA1(82d249f331f51b7c72f9114ecf4e835ccdae7e97) )
	ROM_LOAD( "mah-09.z12", 0x080000, 0x80000,  CRC(a58c43a7) SHA1(0b7f743cf0443d998479b7d5d95b8f2aaf1ef136) )
	ROM_LOAD( "mah-10.a12", 0x100000, 0x80000,  CRC(14b770da) SHA1(6d57da630da1ec457ebaeed8c251e85bd737e97c) )
	ROM_LOAD( "mah-05.y9",  0x180000, 0x80000,  CRC(6773e613) SHA1(ee6cb4272bb9f80e0d918dc059b40e0a47db0876) )
	ROM_LOAD( "mah-06.z9",  0x200000, 0x80000,  CRC(27a8808a) SHA1(cb14992d1073de38406e36f5884d77933dd6b765) )
	ROM_LOAD( "mah-07.a9",  0x280000, 0x80000,  CRC(526f4190) SHA1(23cb79230ec267b8e4236381b5a596d7af8ec5b3) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gp-08.j13",  0x00000,  0x20000,  CRC(365183b1) SHA1(7d3c201c49981c3ac84022283b048e380cbb7ec3) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mah-11.f13", 0x00000,  0x80000,  CRC(642bc692) SHA1(8d9e446b7633bb6acc46d9f92044a69b99a0ccc9) )  /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "go-12.v7", 0x00000,  0x400,  CRC(278f674f) SHA1(d4f5b9770d6d2ddebf1b999e291c80a3e274d492) )  /* Priority  Unused */
ROM_END


ROM_START( robocop2u )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gp03-3.k1",   0x00000, 0x20000, CRC(c016a84b) SHA1(3ac4c61fd87899831e093596ffd7e8a213b946a2) )
	ROM_LOAD16_BYTE( "gp07-3.k3",   0x00001, 0x20000, CRC(54c541ae) SHA1(c9aa46bdc182aceff3cfb1ad958965deabb335e3) )
	ROM_LOAD16_BYTE( "gp02-3.j1",   0x40000, 0x20000, CRC(6777b8a0) SHA1(9081bd187c3b5923efab3e4abde952e9ab29d946) ) // == gq-02.j1 in 'robocop2'
	ROM_LOAD16_BYTE( "gp06-3.j3",   0x40001, 0x20000, CRC(73b8cf96) SHA1(6c651db1d4322946529569f3e516f382f1becde7) )
	ROM_LOAD16_BYTE( "gp01-.h1",    0x80000, 0x20000, CRC(ab5356c0) SHA1(297a89b4d9212c916745997bbb959b0ed660f909) ) // no '-1' but matches other '-1' roms we have
	ROM_LOAD16_BYTE( "gp05-.h3",    0x80001, 0x20000, CRC(ce21bda5) SHA1(615701d4abdb56d50da44589e6e03909f4b28d45) )
	ROM_LOAD16_BYTE( "gp00-.f1",    0xc0000, 0x20000, CRC(a93369ea) SHA1(9e13c36112eb7ebc97dc919e24d0b2955c57e10e) )
	ROM_LOAD16_BYTE( "gp04-.f3",    0xc0001, 0x20000, CRC(ee2f6ad9) SHA1(3abc07792f444a3415fd32e50d6855843e900b1d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gp-09.k13",  0x00000,  0x10000,  CRC(4a4e0f8d) SHA1(5408465667d2854bbade23a26ba619d42a0c22f8) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gp10-1.y6",  0x00001,  0x10000,  CRC(d25d719c) SHA1(be874cf403ec0e607eb9b54b7cfff0a53f4d59a2) )
	ROM_LOAD16_BYTE( "gp11-1.z6",  0x00000,  0x10000,  CRC(030ded47) SHA1(59ded540b2601ec37255e871e38ac71a47c8d007) )

	ROM_REGION( 0x100000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mah-04.z4", 0x000000, 0x40000,  CRC(9b6ca18c) SHA1(29a20200ea50b9e8e79da072c1b6e580e6ca180f) )
	ROM_CONTINUE(          0x080000, 0x40000 )
	ROM_LOAD( "mah-03.y4", 0x040000, 0x40000,  CRC(37894ddc) SHA1(ee08440b3b2023ec6ee2af6d509b642bcead2e60) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )

	ROM_REGION( 0x180000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mah-01.z1", 0x000000, 0x40000,  CRC(26e0dfff) SHA1(8cca2dbcda64f4bc6ee0842486da7dc7df3046fd) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )
	ROM_LOAD( "mah-00.y1", 0x040000, 0x40000,  CRC(7bd69e41) SHA1(296adbf7d40f1092bf38599b3bad51f39d8093b2) )
	ROM_CONTINUE(          0x100000, 0x40000 )
	ROM_LOAD( "mah-02.a1", 0x080000, 0x40000,  CRC(328a247d) SHA1(879f75452dc7c327fd5b35c960c58bc0c0efd33c) )
	ROM_CONTINUE(          0x140000, 0x40000 )

	ROM_REGION( 0x300000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "mah-08.y12", 0x000000, 0x80000,  CRC(88d310a5) SHA1(82d249f331f51b7c72f9114ecf4e835ccdae7e97) )
	ROM_LOAD( "mah-09.z12", 0x080000, 0x80000,  CRC(a58c43a7) SHA1(0b7f743cf0443d998479b7d5d95b8f2aaf1ef136) )
	ROM_LOAD( "mah-10.a12", 0x100000, 0x80000,  CRC(14b770da) SHA1(6d57da630da1ec457ebaeed8c251e85bd737e97c) )
	ROM_LOAD( "mah-05.y9",  0x180000, 0x80000,  CRC(6773e613) SHA1(ee6cb4272bb9f80e0d918dc059b40e0a47db0876) )
	ROM_LOAD( "mah-06.z9",  0x200000, 0x80000,  CRC(27a8808a) SHA1(cb14992d1073de38406e36f5884d77933dd6b765) )
	ROM_LOAD( "mah-07.a9",  0x280000, 0x80000,  CRC(526f4190) SHA1(23cb79230ec267b8e4236381b5a596d7af8ec5b3) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gp-08.j13",  0x00000,  0x20000,  CRC(365183b1) SHA1(7d3c201c49981c3ac84022283b048e380cbb7ec3) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mah-11.f13", 0x00000,  0x80000,  CRC(642bc692) SHA1(8d9e446b7633bb6acc46d9f92044a69b99a0ccc9) )  /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "go-12.v7", 0x00000,  0x400,  CRC(278f674f) SHA1(d4f5b9770d6d2ddebf1b999e291c80a3e274d492) )  /* Priority  Unused */
ROM_END


ROM_START( robocop2ua )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "robo03.k1",  0x00000, 0x20000, CRC(f4c96cc9) SHA1(2eb58aca1134c33f2084267e65a565f9adc6ba49) )
	ROM_LOAD16_BYTE( "robo07.k3",  0x00001, 0x20000, CRC(11e53a7c) SHA1(cdeb7f1983a771238d9d2000f99aed35ae4a06ee) )
	ROM_LOAD16_BYTE( "robo02.j1",  0x40000, 0x20000, CRC(fa086a0d) SHA1(34a3f9c6890e1fbacbde3e39a861e42d511cd8ec) )
	ROM_LOAD16_BYTE( "robo06.j3",  0x40001, 0x20000, CRC(703b49d0) SHA1(be51644fe730d0cb95e1b09f8595da2e36c09aeb) )
	ROM_LOAD16_BYTE( "go-01-1.h1", 0x80000, 0x20000, CRC(ab5356c0) SHA1(297a89b4d9212c916745997bbb959b0ed660f909) )
	ROM_LOAD16_BYTE( "go-05-1.h3", 0x80001, 0x20000, CRC(ce21bda5) SHA1(615701d4abdb56d50da44589e6e03909f4b28d45) )
	ROM_LOAD16_BYTE( "go-00.f1",   0xc0000, 0x20000, CRC(a93369ea) SHA1(9e13c36112eb7ebc97dc919e24d0b2955c57e10e) )
	ROM_LOAD16_BYTE( "go-04.f3",   0xc0001, 0x20000, CRC(ee2f6ad9) SHA1(3abc07792f444a3415fd32e50d6855843e900b1d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gp-09.k13",  0x00000,  0x10000,  CRC(4a4e0f8d) SHA1(5408465667d2854bbade23a26ba619d42a0c22f8) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gp10-1.y6",  0x00001,  0x10000,  CRC(d25d719c) SHA1(be874cf403ec0e607eb9b54b7cfff0a53f4d59a2) )
	ROM_LOAD16_BYTE( "gp11-1.z6",  0x00000,  0x10000,  CRC(030ded47) SHA1(59ded540b2601ec37255e871e38ac71a47c8d007) )

	ROM_REGION( 0x100000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mah-04.z4", 0x000000, 0x40000,  CRC(9b6ca18c) SHA1(29a20200ea50b9e8e79da072c1b6e580e6ca180f) )
	ROM_CONTINUE(          0x080000, 0x40000 )
	ROM_LOAD( "mah-03.y4", 0x040000, 0x40000,  CRC(37894ddc) SHA1(ee08440b3b2023ec6ee2af6d509b642bcead2e60) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )

	ROM_REGION( 0x180000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mah-01.z1", 0x000000, 0x40000,  CRC(26e0dfff) SHA1(8cca2dbcda64f4bc6ee0842486da7dc7df3046fd) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )
	ROM_LOAD( "mah-00.y1", 0x040000, 0x40000,  CRC(7bd69e41) SHA1(296adbf7d40f1092bf38599b3bad51f39d8093b2) )
	ROM_CONTINUE(          0x100000, 0x40000 )
	ROM_LOAD( "mah-02.a1", 0x080000, 0x40000,  CRC(328a247d) SHA1(879f75452dc7c327fd5b35c960c58bc0c0efd33c) )
	ROM_CONTINUE(          0x140000, 0x40000 )

	ROM_REGION( 0x300000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "mah-08.y12", 0x000000, 0x80000,  CRC(88d310a5) SHA1(82d249f331f51b7c72f9114ecf4e835ccdae7e97) )
	ROM_LOAD( "mah-09.z12", 0x080000, 0x80000,  CRC(a58c43a7) SHA1(0b7f743cf0443d998479b7d5d95b8f2aaf1ef136) )
	ROM_LOAD( "mah-10.a12", 0x100000, 0x80000,  CRC(14b770da) SHA1(6d57da630da1ec457ebaeed8c251e85bd737e97c) )
	ROM_LOAD( "mah-05.y9",  0x180000, 0x80000,  CRC(6773e613) SHA1(ee6cb4272bb9f80e0d918dc059b40e0a47db0876) )
	ROM_LOAD( "mah-06.z9",  0x200000, 0x80000,  CRC(27a8808a) SHA1(cb14992d1073de38406e36f5884d77933dd6b765) )
	ROM_LOAD( "mah-07.a9",  0x280000, 0x80000,  CRC(526f4190) SHA1(23cb79230ec267b8e4236381b5a596d7af8ec5b3) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gp-08.j13",  0x00000,  0x20000,  CRC(365183b1) SHA1(7d3c201c49981c3ac84022283b048e380cbb7ec3) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mah-11.f13", 0x00000,  0x80000,  CRC(642bc692) SHA1(8d9e446b7633bb6acc46d9f92044a69b99a0ccc9) )  /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "go-12.v7", 0x00000,  0x400,  CRC(278f674f) SHA1(d4f5b9770d6d2ddebf1b999e291c80a3e274d492) )  /* Priority  Unused */
ROM_END

ROM_START( robocop2j )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "go-03-1.k1", 0x00000, 0x20000, CRC(52506608) SHA1(a0e738fe1083a17cb40f28ad95b695b6caebf3b1) )
	ROM_LOAD16_BYTE( "go-07-1.k3", 0x00001, 0x20000, CRC(739cda17) SHA1(5a69873d79beabace4739ad313e8c090919206ba) )
	ROM_LOAD16_BYTE( "go-02-1.j1", 0x40000, 0x20000, CRC(48c0ace9) SHA1(cf53eb97552aa503e62eb3361af4a19494dfe1ff) )
	ROM_LOAD16_BYTE( "go-06-1.j3", 0x40001, 0x20000, CRC(41abec87) SHA1(83d24d9344508124a8ced402bdc5749e5fcc8e9c) )
	ROM_LOAD16_BYTE( "go-01-1.h1", 0x80000, 0x20000, CRC(ab5356c0) SHA1(297a89b4d9212c916745997bbb959b0ed660f909) )
	ROM_LOAD16_BYTE( "go-05-1.h3", 0x80001, 0x20000, CRC(ce21bda5) SHA1(615701d4abdb56d50da44589e6e03909f4b28d45) )
	ROM_LOAD16_BYTE( "go-00.f1",   0xc0000, 0x20000, CRC(a93369ea) SHA1(9e13c36112eb7ebc97dc919e24d0b2955c57e10e) )
	ROM_LOAD16_BYTE( "go-04.f3",   0xc0001, 0x20000, CRC(ee2f6ad9) SHA1(3abc07792f444a3415fd32e50d6855843e900b1d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gp-09.k13",  0x00000,  0x10000,  CRC(4a4e0f8d) SHA1(5408465667d2854bbade23a26ba619d42a0c22f8) )

	ROM_REGION( 0x020000, "chars", 0 )   /* chars */
	ROM_LOAD16_BYTE( "gp10-1.y6",  0x00001,  0x10000,  CRC(d25d719c) SHA1(be874cf403ec0e607eb9b54b7cfff0a53f4d59a2) )
	ROM_LOAD16_BYTE( "gp11-1.z6",  0x00000,  0x10000,  CRC(030ded47) SHA1(59ded540b2601ec37255e871e38ac71a47c8d007) )

	ROM_REGION( 0x100000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "mah-04.z4", 0x000000, 0x40000,  CRC(9b6ca18c) SHA1(29a20200ea50b9e8e79da072c1b6e580e6ca180f) )
	ROM_CONTINUE(          0x080000, 0x40000 )
	ROM_LOAD( "mah-03.y4", 0x040000, 0x40000,  CRC(37894ddc) SHA1(ee08440b3b2023ec6ee2af6d509b642bcead2e60) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )

	ROM_REGION( 0x180000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "mah-01.z1", 0x000000, 0x40000,  CRC(26e0dfff) SHA1(8cca2dbcda64f4bc6ee0842486da7dc7df3046fd) )
	ROM_CONTINUE(          0x0c0000, 0x40000 )
	ROM_LOAD( "mah-00.y1", 0x040000, 0x40000,  CRC(7bd69e41) SHA1(296adbf7d40f1092bf38599b3bad51f39d8093b2) )
	ROM_CONTINUE(          0x100000, 0x40000 )
	ROM_LOAD( "mah-02.a1", 0x080000, 0x40000,  CRC(328a247d) SHA1(879f75452dc7c327fd5b35c960c58bc0c0efd33c) )
	ROM_CONTINUE(          0x140000, 0x40000 )

	ROM_REGION( 0x300000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "mah-08.y12", 0x000000, 0x80000,  CRC(88d310a5) SHA1(82d249f331f51b7c72f9114ecf4e835ccdae7e97) )
	ROM_LOAD( "mah-09.z12", 0x080000, 0x80000,  CRC(a58c43a7) SHA1(0b7f743cf0443d998479b7d5d95b8f2aaf1ef136) )
	ROM_LOAD( "mah-10.a12", 0x100000, 0x80000,  CRC(14b770da) SHA1(6d57da630da1ec457ebaeed8c251e85bd737e97c) )
	ROM_LOAD( "mah-05.y9",  0x180000, 0x80000,  CRC(6773e613) SHA1(ee6cb4272bb9f80e0d918dc059b40e0a47db0876) )
	ROM_LOAD( "mah-06.z9",  0x200000, 0x80000,  CRC(27a8808a) SHA1(cb14992d1073de38406e36f5884d77933dd6b765) )
	ROM_LOAD( "mah-07.a9",  0x280000, 0x80000,  CRC(526f4190) SHA1(23cb79230ec267b8e4236381b5a596d7af8ec5b3) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gp-08.j13",  0x00000,  0x20000,  CRC(365183b1) SHA1(7d3c201c49981c3ac84022283b048e380cbb7ec3) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "mah-11.f13", 0x00000,  0x80000,  CRC(642bc692) SHA1(8d9e446b7633bb6acc46d9f92044a69b99a0ccc9) )  /* banked */

	ROM_REGION( 1024, "proms", 0 )
	ROM_LOAD( "go-12.v7", 0x00000,  0x400,  CRC(278f674f) SHA1(d4f5b9770d6d2ddebf1b999e291c80a3e274d492) )  /* Priority  Unused */
ROM_END

ROM_START( mutantf ) /* World ver 5 */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hd-03-4.2c", 0x00000, 0x20000, CRC(94859545) SHA1(4b218442bf1ba01b9b6b54c0037c76c827b79d35) )
	ROM_LOAD16_BYTE("hd-00-4.2a", 0x00001, 0x20000, CRC(3cdb648f) SHA1(f803d2894d4c32de770861c70f837377afd329fe) )
	ROM_LOAD16_BYTE("hd-04-1.4c", 0x40000, 0x20000, CRC(fd2ea8d7) SHA1(00e50d42fcc7f6d5076963b02f5abb36275dc993) )
	ROM_LOAD16_BYTE("hd-01-1.4a", 0x40001, 0x20000, CRC(48a247ac) SHA1(c69d67e44ffae92b261de247f8d3eac2e02fcf11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "hd-12.21j",  0x00000,  0x10000,  CRC(13d55f11) SHA1(6438dca57f43b3ca6d273bf82b62104a49260132) )

	ROM_REGION( 0x0a0000, "chars", 0 ) /* chars */
	ROM_LOAD16_BYTE( "hd-06-1.8d", 0x000000, 0x10000, CRC(8b7a558b) SHA1(06f1f6910b9a54e208ee9a0b734d5df946016236) )
	ROM_LOAD16_BYTE( "hd-07-1.9d", 0x000001, 0x10000, CRC(d2a3d449) SHA1(10ad634eb0238f6e9ba04266e4dbaf9470f5d169) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "maf-00.8a", 0x000000, 0x80000,  CRC(e56f528d) SHA1(3908d9b189fa4895c532d1d1f133df0913810cf9) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "maf-01.9a",  0x000000, 0x40000,  CRC(c3d5173d) SHA1(2b6559bf65d7cc5b957ad347b64cf6a18f661686) )
	ROM_CONTINUE(           0x080000, 0x40000 )
	ROM_LOAD( "maf-02.11a", 0x040000, 0x40000,  CRC(0b37d849) SHA1(a0606fb8130a2e86a241ce5ce0b4f61373a88c17) )
	ROM_CONTINUE(           0x0c0000, 0x40000 )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "maf-06.18d",   0x000000, 0x100000, CRC(f5c7a9b5) SHA1(92efc9401347598c90acf62c9aef30109c990ad6) )
	ROM_LOAD( "maf-07.20d",   0x100000, 0x100000, CRC(fd6008a3) SHA1(7b680424eca3804c70fa0c4dc415d665c8626498) )
	ROM_LOAD( "maf-08.21d",   0x200000, 0x080000, CRC(e41cf1e7) SHA1(06524e1aed0adc4c32c92e16a00dc983014f4994) )
	ROM_LOAD( "maf-03.18a",   0x280000, 0x100000, CRC(f4366d2c) SHA1(20964d0e1b879b3e5cb5d18a46d2a17dca2b4171) )
	ROM_LOAD( "maf-04.20a",   0x380000, 0x100000, CRC(0c8f654e) SHA1(e566d4b789b345e20caf7e061e43be7c2e1be9b2) )
	ROM_LOAD( "maf-05.21a",   0x480000, 0x080000, CRC(b0cfeb80) SHA1(b8519c604b03eb8bcf26d00a43b39d48f1b45ab5) )

	ROM_REGION( 0x40000, "sprites2", 0 ) /* sprites 2 */
	ROM_LOAD16_BYTE("hf-08.15a", 0x00000, 0x10000, CRC(93b7279f) SHA1(14304a1ffe1bc791bfa83f8200793d897449133c) )
	ROM_LOAD16_BYTE("hf-09.17a", 0x00001, 0x10000, CRC(05e2c074) SHA1(ec95303e8196424864964b5d2ae862bf75571e83) )
	ROM_LOAD16_BYTE("hf-10.15c", 0x20000, 0x10000, CRC(9b06f418) SHA1(d1579ae36676e38c96ee55a1ffa20aa307a21654) )
	ROM_LOAD16_BYTE("hf-11.17c", 0x20001, 0x10000, CRC(3859a531) SHA1(a2a0c1aa28181b5ef6c075ff0118178340389693) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-10.20l",    0x00000, 0x40000, CRC(7c57f48b) SHA1(9a5624553b3b038d70f9b517f410a635c00a8771) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-09.18l",    0x00000, 0x80000, CRC(28e7ed81) SHA1(e168a2748b75c647f6f9c0d7d25d4f046aa98094) )
ROM_END

ROM_START( mutantf4 ) /* World ver 4 */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hd-03-3.2c", 0x00000, 0x20000, CRC(e6f53574) SHA1(98d5a76bda52346e4bee5b1b0755e3fee4ad8283) )
	ROM_LOAD16_BYTE("hd-00-3.2a", 0x00001, 0x20000, CRC(d3055454) SHA1(83531ae52e5928ac64279bcb98878eef291f8f70) )
	ROM_LOAD16_BYTE("hd-04-1.4c", 0x40000, 0x20000, CRC(fd2ea8d7) SHA1(00e50d42fcc7f6d5076963b02f5abb36275dc993) )
	ROM_LOAD16_BYTE("hd-01-1.4a", 0x40001, 0x20000, CRC(48a247ac) SHA1(c69d67e44ffae92b261de247f8d3eac2e02fcf11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "hd-12.21j",  0x00000,  0x10000,  CRC(13d55f11) SHA1(6438dca57f43b3ca6d273bf82b62104a49260132) )

	ROM_REGION( 0x0a0000, "chars", 0 ) /* chars */
	ROM_LOAD16_BYTE( "hd-06-1.8d", 0x000000, 0x10000, CRC(8b7a558b) SHA1(06f1f6910b9a54e208ee9a0b734d5df946016236) )
	ROM_LOAD16_BYTE( "hd-07-1.9d", 0x000001, 0x10000, CRC(d2a3d449) SHA1(10ad634eb0238f6e9ba04266e4dbaf9470f5d169) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "maf-00.8a", 0x000000, 0x80000,  CRC(e56f528d) SHA1(3908d9b189fa4895c532d1d1f133df0913810cf9) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "maf-01.9a",  0x000000, 0x40000,  CRC(c3d5173d) SHA1(2b6559bf65d7cc5b957ad347b64cf6a18f661686) )
	ROM_CONTINUE(           0x080000, 0x40000 )
	ROM_LOAD( "maf-02.11a", 0x040000, 0x40000,  CRC(0b37d849) SHA1(a0606fb8130a2e86a241ce5ce0b4f61373a88c17) )
	ROM_CONTINUE(           0x0c0000, 0x40000 )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "maf-06.18d",   0x000000, 0x100000, CRC(f5c7a9b5) SHA1(92efc9401347598c90acf62c9aef30109c990ad6) )
	ROM_LOAD( "maf-07.20d",   0x100000, 0x100000, CRC(fd6008a3) SHA1(7b680424eca3804c70fa0c4dc415d665c8626498) )
	ROM_LOAD( "maf-08.21d",   0x200000, 0x080000, CRC(e41cf1e7) SHA1(06524e1aed0adc4c32c92e16a00dc983014f4994) )
	ROM_LOAD( "maf-03.18a",   0x280000, 0x100000, CRC(f4366d2c) SHA1(20964d0e1b879b3e5cb5d18a46d2a17dca2b4171) )
	ROM_LOAD( "maf-04.20a",   0x380000, 0x100000, CRC(0c8f654e) SHA1(e566d4b789b345e20caf7e061e43be7c2e1be9b2) )
	ROM_LOAD( "maf-05.21a",   0x480000, 0x080000, CRC(b0cfeb80) SHA1(b8519c604b03eb8bcf26d00a43b39d48f1b45ab5) )

	ROM_REGION( 0x40000, "sprites2", 0 ) /* sprites 2 */
	ROM_LOAD16_BYTE("hf-08.15a", 0x00000, 0x10000, CRC(93b7279f) SHA1(14304a1ffe1bc791bfa83f8200793d897449133c) )
	ROM_LOAD16_BYTE("hf-09.17a", 0x00001, 0x10000, CRC(05e2c074) SHA1(ec95303e8196424864964b5d2ae862bf75571e83) )
	ROM_LOAD16_BYTE("hf-10.15c", 0x20000, 0x10000, CRC(9b06f418) SHA1(d1579ae36676e38c96ee55a1ffa20aa307a21654) )
	ROM_LOAD16_BYTE("hf-11.17c", 0x20001, 0x10000, CRC(3859a531) SHA1(a2a0c1aa28181b5ef6c075ff0118178340389693) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-10.20l",    0x00000, 0x40000, CRC(7c57f48b) SHA1(9a5624553b3b038d70f9b517f410a635c00a8771) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-09.18l",    0x00000, 0x80000, CRC(28e7ed81) SHA1(e168a2748b75c647f6f9c0d7d25d4f046aa98094) )
ROM_END

ROM_START( mutantf3 ) /* World ver 3 */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hd-03-2.2c", 0x00000, 0x20000, CRC(0586c4fa) SHA1(bba9f7e57be0e70185b8103af26443ce832e7413) )
	ROM_LOAD16_BYTE("hd-00-2.2a", 0x00001, 0x20000, CRC(6f8ec48e) SHA1(5fcc2ae4ce409598ca9d0c28ba60f3de3874efa5) )
	ROM_LOAD16_BYTE("hd-04-1.4c", 0x40000, 0x20000, CRC(fd2ea8d7) SHA1(00e50d42fcc7f6d5076963b02f5abb36275dc993) )
	ROM_LOAD16_BYTE("hd-01-1.4a", 0x40001, 0x20000, CRC(48a247ac) SHA1(c69d67e44ffae92b261de247f8d3eac2e02fcf11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "hd-12.21j",  0x00000,  0x10000,  CRC(13d55f11) SHA1(6438dca57f43b3ca6d273bf82b62104a49260132) )

	ROM_REGION( 0x0a0000, "chars", 0 ) /* chars */
	ROM_LOAD16_BYTE( "hd-06-1.8d", 0x000000, 0x10000, CRC(8b7a558b) SHA1(06f1f6910b9a54e208ee9a0b734d5df946016236) )
	ROM_LOAD16_BYTE( "hd-07-1.9d", 0x000001, 0x10000, CRC(d2a3d449) SHA1(10ad634eb0238f6e9ba04266e4dbaf9470f5d169) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "maf-00.8a", 0x000000, 0x80000,  CRC(e56f528d) SHA1(3908d9b189fa4895c532d1d1f133df0913810cf9) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "maf-01.9a",  0x000000, 0x40000,  CRC(c3d5173d) SHA1(2b6559bf65d7cc5b957ad347b64cf6a18f661686) )
	ROM_CONTINUE(           0x080000, 0x40000 )
	ROM_LOAD( "maf-02.11a", 0x040000, 0x40000,  CRC(0b37d849) SHA1(a0606fb8130a2e86a241ce5ce0b4f61373a88c17) )
	ROM_CONTINUE(           0x0c0000, 0x40000 )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "maf-06.18d",   0x000000, 0x100000, CRC(f5c7a9b5) SHA1(92efc9401347598c90acf62c9aef30109c990ad6) )
	ROM_LOAD( "maf-07.20d",   0x100000, 0x100000, CRC(fd6008a3) SHA1(7b680424eca3804c70fa0c4dc415d665c8626498) )
	ROM_LOAD( "maf-08.21d",   0x200000, 0x080000, CRC(e41cf1e7) SHA1(06524e1aed0adc4c32c92e16a00dc983014f4994) )
	ROM_LOAD( "maf-03.18a",   0x280000, 0x100000, CRC(f4366d2c) SHA1(20964d0e1b879b3e5cb5d18a46d2a17dca2b4171) )
	ROM_LOAD( "maf-04.20a",   0x380000, 0x100000, CRC(0c8f654e) SHA1(e566d4b789b345e20caf7e061e43be7c2e1be9b2) )
	ROM_LOAD( "maf-05.21a",   0x480000, 0x080000, CRC(b0cfeb80) SHA1(b8519c604b03eb8bcf26d00a43b39d48f1b45ab5) )

	ROM_REGION( 0x40000, "sprites2", 0 ) /* sprites 2 */
	ROM_LOAD16_BYTE("hf-08.15a", 0x00000, 0x10000, CRC(93b7279f) SHA1(14304a1ffe1bc791bfa83f8200793d897449133c) )
	ROM_LOAD16_BYTE("hf-09.17a", 0x00001, 0x10000, CRC(05e2c074) SHA1(ec95303e8196424864964b5d2ae862bf75571e83) )
	ROM_LOAD16_BYTE("hf-10.15c", 0x20000, 0x10000, CRC(9b06f418) SHA1(d1579ae36676e38c96ee55a1ffa20aa307a21654) )
	ROM_LOAD16_BYTE("hf-11.17c", 0x20001, 0x10000, CRC(3859a531) SHA1(a2a0c1aa28181b5ef6c075ff0118178340389693) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-10.20l",    0x00000, 0x40000, CRC(7c57f48b) SHA1(9a5624553b3b038d70f9b517f410a635c00a8771) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-09.18l",    0x00000, 0x80000, CRC(28e7ed81) SHA1(e168a2748b75c647f6f9c0d7d25d4f046aa98094) )
ROM_END


ROM_START( mutantf2 ) /* World ver 2 */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hd-03-1.2c", 0x00000, 0x20000, CRC(7110cefc) SHA1(26b470cf2fcd026542c7ff4fca11c2095a17501d) )
	ROM_LOAD16_BYTE("hd-00-1.2a", 0x00001, 0x20000, CRC(b279875b) SHA1(fc148f3f70f289ae1d383f714c41d7b50381803e) )
	ROM_LOAD16_BYTE("hd-04-1.4c", 0x40000, 0x20000, CRC(fd2ea8d7) SHA1(00e50d42fcc7f6d5076963b02f5abb36275dc993) )
	ROM_LOAD16_BYTE("hd-01-1.4a", 0x40001, 0x20000, CRC(48a247ac) SHA1(c69d67e44ffae92b261de247f8d3eac2e02fcf11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "hd-12.21j",  0x00000,  0x10000,  CRC(13d55f11) SHA1(6438dca57f43b3ca6d273bf82b62104a49260132) )

	ROM_REGION( 0x0a0000, "chars", 0 ) /* chars */
	ROM_LOAD16_BYTE( "hd-06-1.8d", 0x000000, 0x10000, CRC(8b7a558b) SHA1(06f1f6910b9a54e208ee9a0b734d5df946016236) )
	ROM_LOAD16_BYTE( "hd-07-1.9d", 0x000001, 0x10000, CRC(d2a3d449) SHA1(10ad634eb0238f6e9ba04266e4dbaf9470f5d169) )

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "maf-00.8a", 0x000000, 0x80000,  CRC(e56f528d) SHA1(3908d9b189fa4895c532d1d1f133df0913810cf9) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "maf-01.9a",  0x000000, 0x40000,  CRC(c3d5173d) SHA1(2b6559bf65d7cc5b957ad347b64cf6a18f661686) )
	ROM_CONTINUE(           0x080000, 0x40000 )
	ROM_LOAD( "maf-02.11a", 0x040000, 0x40000,  CRC(0b37d849) SHA1(a0606fb8130a2e86a241ce5ce0b4f61373a88c17) )
	ROM_CONTINUE(           0x0c0000, 0x40000 )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "maf-06.18d",   0x000000, 0x100000, CRC(f5c7a9b5) SHA1(92efc9401347598c90acf62c9aef30109c990ad6) )
	ROM_LOAD( "maf-07.20d",   0x100000, 0x100000, CRC(fd6008a3) SHA1(7b680424eca3804c70fa0c4dc415d665c8626498) )
	ROM_LOAD( "maf-08.21d",   0x200000, 0x080000, CRC(e41cf1e7) SHA1(06524e1aed0adc4c32c92e16a00dc983014f4994) )
	ROM_LOAD( "maf-03.18a",   0x280000, 0x100000, CRC(f4366d2c) SHA1(20964d0e1b879b3e5cb5d18a46d2a17dca2b4171) )
	ROM_LOAD( "maf-04.20a",   0x380000, 0x100000, CRC(0c8f654e) SHA1(e566d4b789b345e20caf7e061e43be7c2e1be9b2) )
	ROM_LOAD( "maf-05.21a",   0x480000, 0x080000, CRC(b0cfeb80) SHA1(b8519c604b03eb8bcf26d00a43b39d48f1b45ab5) )

	ROM_REGION( 0x40000, "sprites2", 0 ) /* sprites 2 */
	ROM_LOAD16_BYTE("hf-08.15a", 0x00000, 0x10000, CRC(93b7279f) SHA1(14304a1ffe1bc791bfa83f8200793d897449133c) )
	ROM_LOAD16_BYTE("hf-09.17a", 0x00001, 0x10000, CRC(05e2c074) SHA1(ec95303e8196424864964b5d2ae862bf75571e83) )
	ROM_LOAD16_BYTE("hf-10.15c", 0x20000, 0x10000, CRC(9b06f418) SHA1(d1579ae36676e38c96ee55a1ffa20aa307a21654) )
	ROM_LOAD16_BYTE("hf-11.17c", 0x20001, 0x10000, CRC(3859a531) SHA1(a2a0c1aa28181b5ef6c075ff0118178340389693) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-10.20l",    0x00000, 0x40000, CRC(7c57f48b) SHA1(9a5624553b3b038d70f9b517f410a635c00a8771) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-09.18l",    0x00000, 0x80000, CRC(28e7ed81) SHA1(e168a2748b75c647f6f9c0d7d25d4f046aa98094) )
ROM_END

ROM_START( deathbrd ) /* Japan ver 3 */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hf-03-2.2c", 0x00000, 0x20000, CRC(fb86fff3) SHA1(af4cfc19ec85e0aa49b5e46d95bdd94a20922cce) )
	ROM_LOAD16_BYTE("hf-00-2.2a", 0x00001, 0x20000, CRC(099aa422) SHA1(b62f261b1903dd2d1a308f7abb9584b3726204b5) )
	ROM_LOAD16_BYTE("hd-04-1.4c", 0x40000, 0x20000, CRC(fd2ea8d7) SHA1(00e50d42fcc7f6d5076963b02f5abb36275dc993) ) /* May have the "HD" or "HF" region code label */
	ROM_LOAD16_BYTE("hd-01-1.4a", 0x40001, 0x20000, CRC(48a247ac) SHA1(c69d67e44ffae92b261de247f8d3eac2e02fcf11) ) /* May have the "HD" or "HF" region code label */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "hd-12.21j",  0x00000,  0x10000,  CRC(13d55f11) SHA1(6438dca57f43b3ca6d273bf82b62104a49260132) ) /* May have the "HD" or "HF" region code label */

	ROM_REGION( 0x0a0000, "chars", 0 ) /* chars */
	ROM_LOAD16_BYTE( "hd-06-1.8d", 0x000000, 0x10000, CRC(8b7a558b) SHA1(06f1f6910b9a54e208ee9a0b734d5df946016236) ) /* May have the "HD" or "HF" region code label */
	ROM_LOAD16_BYTE( "hd-07-1.9d", 0x000001, 0x10000, CRC(d2a3d449) SHA1(10ad634eb0238f6e9ba04266e4dbaf9470f5d169) ) /* May have the "HD" or "HF" region code label */

	ROM_REGION( 0x080000, "tiles1", 0 )  /* tiles 1 */
	ROM_LOAD( "maf-00.8a", 0x000000, 0x80000,  CRC(e56f528d) SHA1(3908d9b189fa4895c532d1d1f133df0913810cf9) )

	ROM_REGION( 0x100000, "tiles2", 0 )  /* tiles 2 */
	ROM_LOAD( "maf-01.9a",  0x000000, 0x40000,  CRC(c3d5173d) SHA1(2b6559bf65d7cc5b957ad347b64cf6a18f661686) )
	ROM_CONTINUE(           0x080000, 0x40000 )
	ROM_LOAD( "maf-02.11a", 0x040000, 0x40000,  CRC(0b37d849) SHA1(a0606fb8130a2e86a241ce5ce0b4f61373a88c17) )
	ROM_CONTINUE(           0x0c0000, 0x40000 )

	ROM_REGION( 0x500000, "sprites1", 0 ) /* sprites */
	ROM_LOAD( "maf-06.18d",   0x000000, 0x100000, CRC(f5c7a9b5) SHA1(92efc9401347598c90acf62c9aef30109c990ad6) )
	ROM_LOAD( "maf-07.20d",   0x100000, 0x100000, CRC(fd6008a3) SHA1(7b680424eca3804c70fa0c4dc415d665c8626498) )
	ROM_LOAD( "maf-08.21d",   0x200000, 0x080000, CRC(e41cf1e7) SHA1(06524e1aed0adc4c32c92e16a00dc983014f4994) )
	ROM_LOAD( "maf-03.18a",   0x280000, 0x100000, CRC(f4366d2c) SHA1(20964d0e1b879b3e5cb5d18a46d2a17dca2b4171) )
	ROM_LOAD( "maf-04.20a",   0x380000, 0x100000, CRC(0c8f654e) SHA1(e566d4b789b345e20caf7e061e43be7c2e1be9b2) )
	ROM_LOAD( "maf-05.21a",   0x480000, 0x080000, CRC(b0cfeb80) SHA1(b8519c604b03eb8bcf26d00a43b39d48f1b45ab5) )

	ROM_REGION( 0x40000, "sprites2", 0 ) /* sprites 2 */
	ROM_LOAD16_BYTE("hf-08.15a", 0x00000, 0x10000, CRC(93b7279f) SHA1(14304a1ffe1bc791bfa83f8200793d897449133c) )
	ROM_LOAD16_BYTE("hf-09.17a", 0x00001, 0x10000, CRC(05e2c074) SHA1(ec95303e8196424864964b5d2ae862bf75571e83) )
	ROM_LOAD16_BYTE("hf-10.15c", 0x20000, 0x10000, CRC(9b06f418) SHA1(d1579ae36676e38c96ee55a1ffa20aa307a21654) )
	ROM_LOAD16_BYTE("hf-11.17c", 0x20001, 0x10000, CRC(3859a531) SHA1(a2a0c1aa28181b5ef6c075ff0118178340389693) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-10.20l",    0x00000, 0x40000, CRC(7c57f48b) SHA1(9a5624553b3b038d70f9b517f410a635c00a8771) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "maf-09.18l",    0x00000, 0x80000, CRC(28e7ed81) SHA1(e168a2748b75c647f6f9c0d7d25d4f046aa98094) )
ROM_END

/**********************************************************************************/

void cninja_state::init_cninjabl2()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0x180000, 0x18ffff);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x1b4000, 0x1b4001, read16_delegate(FUNC(cninja_state::cninjabl2_sprite_dma_r),this));

	m_okibank->configure_entries(0, 8, memregion("oki2")->base(), 0x10000);
}

void cninja_state::init_mutantf()
{
	const uint8_t *src = memregion("tiles1")->base();
	uint8_t *dst = memregion("chars")->base();

	/* The 16x16 graphic has some 8x8 chars in it - decode them in GFX1 */
	memcpy(dst + 0x50000, dst + 0x10000, 0x10000);
	memcpy(dst + 0x10000, src, 0x40000);
	memcpy(dst + 0x60000, src + 0x40000, 0x40000);

	deco56_decrypt_gfx(machine(), "chars");
	deco56_decrypt_gfx(machine(), "tiles1");
}

/**********************************************************************************/

GAME( 1990, edrandy,    0,        edrandy,   edrandy,  cninja_state, empty_init,     ROT0, "Data East Corporation", "The Cliffhanger - Edward Randy (World ver 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, edrandy2,   edrandy,  edrandy,   edrandc,  cninja_state, empty_init,     ROT0, "Data East Corporation", "The Cliffhanger - Edward Randy (World ver 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, edrandy1,   edrandy,  edrandy,   edrandc,  cninja_state, empty_init,     ROT0, "Data East Corporation", "The Cliffhanger - Edward Randy (World ver 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, edrandyj,   edrandy,  edrandy,   edrandc,  cninja_state, empty_init,     ROT0, "Data East Corporation", "The Cliffhanger - Edward Randy (Japan ver 3)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, cninja,     0,        cninja,    cninja,   cninja_state, empty_init,     ROT0, "Data East Corporation", "Caveman Ninja (World ver 4)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1991, cninja1,    cninja,   cninja,    cninja,   cninja_state, empty_init,     ROT0, "Data East Corporation", "Caveman Ninja (World ver 1)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1991, cninjau,    cninja,   cninja,    cninjau,  cninja_state, empty_init,     ROT0, "Data East Corporation", "Caveman Ninja (US ver 4)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1991, joemac,     cninja,   cninja,    cninja,   cninja_state, empty_init,     ROT0, "Data East Corporation", "Tatakae Genshizin Joe & Mac (Japan ver 1)",    MACHINE_SUPPORTS_SAVE )
GAME( 1991, stoneage,   cninja,   stoneage,  cninja,   cninja_state, empty_init,     ROT0, "bootleg",               "Stoneage (bootleg of Caveman Ninja)",          MACHINE_SUPPORTS_SAVE )
GAME( 1991, cninjabl,   cninja,   cninjabl,  cninja,   cninja_state, empty_init,     ROT0, "bootleg",               "Caveman Ninja (bootleg)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1991, cninjabl2,  cninja,   cninjabl2, cninja,   cninja_state, init_cninjabl2, ROT0, "bootleg",               "Tatakae Genshizin Joe & Mac (Japan, bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // tile layers need adjusting

GAME( 1991, robocop2,   0,        robocop2,  robocop2, cninja_state, empty_init,     ROT0, "Data East Corporation", "Robocop 2 (Euro/Asia v0.10)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, robocop2u,  robocop2, robocop2,  robocop2, cninja_state, empty_init,     ROT0, "Data East Corporation", "Robocop 2 (US v0.10)",        MACHINE_SUPPORTS_SAVE )
GAME( 1991, robocop2ua, robocop2, robocop2,  robocop2, cninja_state, empty_init,     ROT0, "Data East Corporation", "Robocop 2 (US v0.05)",        MACHINE_SUPPORTS_SAVE )
GAME( 1991, robocop2j,  robocop2, robocop2,  robocop2, cninja_state, empty_init,     ROT0, "Data East Corporation", "Robocop 2 (Japan v0.11)",     MACHINE_SUPPORTS_SAVE )

GAME( 1992, mutantf,    0,        mutantf,   mutantf,  cninja_state, init_mutantf,   ROT0, "Data East Corporation", "Mutant Fighter (World ver EM-5)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mutantf4,   mutantf,  mutantf,   mutantf,  cninja_state, init_mutantf,   ROT0, "Data East Corporation", "Mutant Fighter (World ver EM-4)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mutantf3,   mutantf,  mutantf,   mutantf,  cninja_state, init_mutantf,   ROT0, "Data East Corporation", "Mutant Fighter (World ver EM-3)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mutantf2,   mutantf,  mutantf,   mutantf,  cninja_state, init_mutantf,   ROT0, "Data East Corporation", "Mutant Fighter (World ver EM-2)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, deathbrd,   mutantf,  mutantf,   mutantf,  cninja_state, init_mutantf,   ROT0, "Data East Corporation", "Death Brade (Japan ver JM-3)",    MACHINE_SUPPORTS_SAVE )
