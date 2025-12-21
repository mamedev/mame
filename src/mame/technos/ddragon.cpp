// license:BSD-3-Clause
// copyright-holders: Philip Bennett,Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino, Ernesto Corvi, David Haywood, R. Belmont
/***************************************************************************

Double Dragon     (c) 1987 Technos Japan
Double Dragon II  (c) 1988 Technos Japan

Driver by Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino, Ernesto Corvi
Toffy / Super Toffy added by David Haywood
Thanks to Bryan McPhail for spotting the Toffy program ROM encryption
Toffy / Super Toffy sound hooked up by R. Belmont.


Modifications by Phil Bennett Sep 2013:

Cleanups based on Double Dragon schematics.
Fixed sub CPU interrupt handling and common RAM access.
Removed now-unnecessary workarounds.

Modifications by Bryan McPhail, June-November 2003:

Correct video & interrupt timing derived from Xain schematics and confirmed on real DD board.
Corrected interrupt handling, especially to MCU (but one semi-hack remains).
TStrike now boots but sprites don't appear (I had them working at one point, can't remember what broke them again).
Dangerous Dungeons fixed.
World version of Double Dragon added (actually same ROMs as the bootleg, but confirmed from real board)
Removed stereo audio flag (still on Toffy - does it have it?)

todo:

banking in Toffy / Super toffy

-- Read Me --

Super Toffy - Unico 1994

Main CPU:   MC6809EP
Sound CPU:  MC6809P
Sound:      YM2151
Clocks:     12 MHz, 3.579MHz

Graphics custom: MDE-2001

-- --

Does this make Super Toffy the sequel to a rip-off / bootleg of a
conversion kit which could be applied to a bootleg double dragon :-p?


2008-08
Dip locations verified with manual for ddragon & ddragon2

***************************************************************************/

#include "emu.h"
#include "ddragon.h"

#include "cpu/m6800/m6801.h"
#include "cpu/m6809/hd6309.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"

#include "speaker.h"


/*************************************
 *
 *  Video timing
 *
 *************************************/

/*
    Vertical timing counts as follows:

        08,09,0A,0B,...,FC,FD,FE,FF,E8,E9,EA,EB,...,FC,FD,FE,FF,
        08,09,....

    Thus, it counts from 08 to FF, then resets to E8 and counts to FF again.
    This gives (256 - 8) + (256 - 232) = 248 + 24 = 272 total scanlines.

    VBLK is signalled starting when the counter hits F8, and continues through
    the reset to E8 and through until the next reset to 08 again.

    Since MAME's video timing is 0-based, we need to convert this.
*/

int ddragon_state::scanline_to_vcount(int scanline)
{
	int vcount = scanline + 8;
	if (vcount < 0x100)
		return vcount;
	else
		return (vcount - 0x18) | 0x100;
}

TIMER_DEVICE_CALLBACK_MEMBER(ddragon_state::scanline)
{
	int const scanline = param;
	int const screen_height = m_screen->height();
	int const vcount_old = scanline_to_vcount((scanline == 0) ? screen_height - 1 : scanline - 1);
	int const vcount = scanline_to_vcount(scanline);

	// update to the current point
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	// on the rising edge of VBLK (vcount == F8), signal an NMI
	if (vcount == 0xf8)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// set 1ms signal on rising edge of vcount & 8
	if (!(vcount_old & 8) && (vcount & 8))
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}



/*************************************
 *
 *  System setup and initialization
 *
 *************************************/

void ddragon_state::machine_start()
{
	// configure banks
	if (m_mainbank)
		m_mainbank->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);

	// register for save states
	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrolly_hi));
	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_end));
	save_item(NAME(m_adpcm_idle));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_ddragon_sub_port));
}


void ddragon_state::machine_reset()
{
	m_scrollx_hi = 0;
	m_scrolly_hi = 0;
	m_ddragon_sub_port = 0;
	m_adpcm_pos[0] = m_adpcm_pos[1] = 0;
	m_adpcm_end[0] = m_adpcm_end[1] = 0;
	m_adpcm_idle[0] = m_adpcm_idle[1] = true;
	m_adpcm_data[0] = m_adpcm_data[1] = -1;
}



/*************************************
 *
 *  Bank switching
 *
 *************************************/

void ddragon_state::bankswitch_w(uint8_t data)
{
	/*
	    76543210
	    .......x    X-scroll D9 (H9BT)
	    ......x.    Y-scroll D9 (V9BT)
	    .....x..    /Screen flip (*1P/2P)
	    ....x...    /Sub CPU reset (*RESET)
	    ...x....    /Sub CPU halt (*HALT)
	    xxx.....    ROM bank (*BANK)
	*/
	m_scrollx_hi = data & 0x01;
	m_scrolly_hi = (data & 0x02) >> 1;
	flip_screen_set(~data & 0x04);

	m_subcpu->set_input_line(INPUT_LINE_RESET, data & 0x08 ? CLEAR_LINE : ASSERT_LINE);
	m_subcpu->set_input_line(INPUT_LINE_HALT, data & 0x10 ? ASSERT_LINE : CLEAR_LINE);
	m_mainbank->set_entry((data & 0xe0) >> 5);
}


void toffy_state::bankswitch_w(uint8_t data)
{
	m_scrollx_hi = data & 0x01;
	m_scrolly_hi = (data & 0x02) >> 1;

//  flip_screen_set(~data & 0x04);

	// I don't know ...
	m_mainbank->set_entry((data & 0x20) >> 5);
}


uint8_t darktowr_state::mcu_bank_r(offs_t offset)
{
	// logerror("BankRead %05x %08x\n", m_maincpu->pc(), offset);

	/* Horrible hack - the alternate TStrike set is mismatched against the MCU,
	so just hack around the protection here.  (The hacks are 'right' as I have
	the original source code & notes to this version of TStrike to examine).
	*/
	if (!strcmp(machine().system().name, "tstrike"))
	{
		// Static protection checks at boot-up
		if (m_maincpu->pc() == 0x9ace)
			return 0;
		if (m_maincpu->pc() == 0x9ae4)
			return 0x63;

		// Just return whatever the code is expecting
		return m_rambase[0xbe1];
	}

	if (offset == 0x1401 || offset == 1)
		return m_mcu_port_a_out;

	logerror("Unmapped MCU bank read %04x\n", offset);
	return 0xff;
}


void darktowr_state::mcu_bank_w(offs_t offset, uint8_t data)
{
	logerror("BankWrite %05x %08x %08x\n", m_maincpu->pc(), offset, data);

	if (offset == 0x1400 || offset == 0)
	{
		uint8_t const value(bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7));
		m_mcu->pb_w(value);
		logerror("MCU PORT 1 -> %04x (from %04x)\n", value, data);
	}
}


void darktowr_state::bankswitch_w(uint8_t data)
{
	m_scrollx_hi = (data & 0x01);
	m_scrolly_hi = ((data & 0x02) >> 1);

//  flip_screen_set(~data & 0x04);

	m_subcpu->set_input_line(INPUT_LINE_RESET, data & 0x08 ? CLEAR_LINE : ASSERT_LINE);
	m_subcpu->set_input_line(INPUT_LINE_HALT, data & 0x10 ? ASSERT_LINE : CLEAR_LINE);

	m_bank->set_bank((data & 0xe0) >> 5);
}



/*************************************
 *
 *  Interrupt control
 *
 *************************************/

void ddragon_state::interrupt_ack(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // 380b - NMI ack
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			break;

		case 1: // 380c - FIRQ ack
			m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
			break;

		case 2: // 380d - IRQ ack
			m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
			break;

		case 3: // 380e - SND IRQ and latch
			m_soundlatch->write(data);
			break;

		case 4: // 380f - MCU IRQ
			if (m_subcpu)
				m_subcpu->set_input_line(m_sprite_irq, ASSERT_LINE);
			break;
	}
}


uint8_t ddragon_state::interrupt_r(offs_t offset)
{
	interrupt_ack(offset, 0xff);
	return 0xff;
}


void ddragon_state::interrupt_w(offs_t offset, uint8_t data)
{
	interrupt_ack(offset, data);
}


void ddragon_state::ddragon2_sub_irq_ack_w(uint8_t data)
{
	m_subcpu->set_input_line(m_sprite_irq, CLEAR_LINE);
}


void ddragon_state::ddragon2_sub_irq_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}


void ddragon_state::ddragonbla_port_w(uint8_t data)
{
	if ((data & 0x8) == 0)
		m_subcpu->set_input_line(m_sprite_irq, CLEAR_LINE);

	if (!(m_ddragon_sub_port & 0x10) && (data & 0x10))
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);

	m_ddragon_sub_port = data;
}



/*************************************
 *
 *  MCU handlers
 *
 *************************************/

int ddragon_state::subcpu_bus_free_r()
{
	// Corresponds to BA (Bus Available) on the HD63701
	if (m_subcpu)
		return m_subcpu->suspended(SUSPEND_REASON_RESET | SUSPEND_REASON_HALT);
	else
		return 0;
}


void darktowr_state::mcu_port_a_w(offs_t offset, uint8_t data)
{
	logerror("%s: McuWrite %08x %08x\n", machine().describe_context(), offset, data);
	m_mcu_port_a_out = data;
}


void ddragon_state::sub_port6_w(uint8_t data)
{
	// Port 6
	if ((data & 0x1) == 0)
		m_subcpu->set_input_line(m_sprite_irq, CLEAR_LINE);

	if (!(m_ddragon_sub_port & 0x2) && (data & 0x2))
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);

	m_ddragon_sub_port = data;
}



/*************************************
 *
 *  Sprite RAM hacks
 *
 *************************************/

uint8_t ddragon_state::comram_r(offs_t offset)
{
	// Access to shared RAM is prevented when the sub CPU is active
	if (!m_subcpu->suspended(SUSPEND_REASON_RESET | SUSPEND_REASON_HALT))
		return 0xff;

	return m_comram[offset];
}


void ddragon_state::comram_w(offs_t offset, uint8_t data)
{
	if (!m_subcpu->suspended(SUSPEND_REASON_RESET | SUSPEND_REASON_HALT))
		return;

	m_comram[offset] = data;
}



/*************************************
 *
 *  ADPCM sound
 *
 *************************************/

void ddragon_state::ddragon_adpcm_w(offs_t offset, uint8_t data)
{
	int const chip = offset & 1;

	switch (offset >> 1)
	{
		case 3:
			m_adpcm_idle[chip] = true;
			m_adpcm[chip]->reset_w(1);
			break;

		case 2:
			m_adpcm_pos[chip] = (data & 0x7f) << 9;
			break;

		case 1:
			m_adpcm_end[chip] = (data & 0x7f) << 9;
			break;

		case 0:
			m_adpcm_idle[chip] = false;
			m_adpcm[chip]->reset_w(0);
			break;
	}
}

template <uint8_t Which>
void ddragon_state::ddragon_adpcm_int(int state)
{
	if (m_adpcm_pos[Which] >= m_adpcm_end[Which] || m_adpcm_pos[Which] >= m_adpcm_rom[Which].length())
	{
		m_adpcm_idle[Which] = true;
		m_adpcm[Which]->reset_w(1);
	}
	else if (m_adpcm_data[Which] != -1)
	{
		m_adpcm[Which]->data_w(m_adpcm_data[Which] & 0x0f);
		m_adpcm_data[Which] = -1;
	}
	else
	{
		m_adpcm_data[Which] = m_adpcm_rom[Which][m_adpcm_pos[Which]++];
		m_adpcm[Which]->data_w(m_adpcm_data[Which] >> 4);
	}
}

uint8_t ddragon_state::ddragon_adpcm_status_r()
{
	return (m_adpcm_idle[0] ? 1 : 0) | (m_adpcm_idle[1] ? 2 : 0);
}



/*************************************
 *
 *  Main CPU memory maps
 *
 *************************************/

void ddragon_state::base_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("rambase");
	map(0x1000, 0x11ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x1200, 0x13ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1fff).ram().w(FUNC(ddragon_state::fgvideoram_w)).share(m_fgvideoram);
	map(0x2000, 0x21ff).rw(FUNC(ddragon_state::comram_r), FUNC(ddragon_state::comram_w)).mirror(0x0600);
	map(0x2800, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x37ff).ram().w(FUNC(ddragon_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x3800, 0x3800).portr("P1");
	map(0x3801, 0x3801).portr("P2");
	map(0x3802, 0x3802).portr("EXTRA");
	map(0x3803, 0x3803).portr("DSW0");
	map(0x3804, 0x3804).portr("DSW1");
	map(0x3809, 0x3809).writeonly().share(m_scrollx_lo);
	map(0x380a, 0x380a).writeonly().share(m_scrolly_lo);
	map(0x380b, 0x380f).rw(FUNC(ddragon_state::interrupt_r), FUNC(ddragon_state::interrupt_w));
	map(0x8000, 0xffff).rom();
}

void ddragon_state::ddragon_main_map(address_map &map)
{
	base_map(map);
	map(0x3808, 0x3808).w(FUNC(ddragon_state::bankswitch_w));
	map(0x4000, 0x7fff).bankr(m_mainbank);
}

void toffy_state::main_map(address_map &map)
{
	base_map(map);
	map(0x3808, 0x3808).w(FUNC(toffy_state::bankswitch_w));
	map(0x4000, 0x7fff).bankr(m_mainbank);
}

void darktowr_state::main_map(address_map &map)
{
	base_map(map);
	map(0x3808, 0x3808).w(FUNC(darktowr_state::bankswitch_w));
	map(0x4000, 0x7fff).m(m_bank, FUNC(address_map_bank_device::amap8));
}

void darktowr_state::banked_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("maincpu", 0x10000);
	map(0x10000, 0x13fff).rw(FUNC(darktowr_state::mcu_bank_r), FUNC(darktowr_state::mcu_bank_w));
	map(0x14000, 0x1ffff).rom().region("maincpu", 0x24000); // TODO : ROM? empty at most of darktowr_state games
}

void ddragon_state::ddragon2_main_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().w(FUNC(ddragon_state::fgvideoram_w)).share(m_fgvideoram);
	map(0x2000, 0x21ff).rw(FUNC(ddragon_state::comram_r), FUNC(ddragon_state::comram_w)).mirror(0x0600);
	map(0x2800, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x37ff).ram().w(FUNC(ddragon_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x3800, 0x3800).portr("P1");
	map(0x3801, 0x3801).portr("P2");
	map(0x3802, 0x3802).portr("EXTRA");
	map(0x3803, 0x3803).portr("DSW0");
	map(0x3804, 0x3804).portr("DSW1");
	map(0x3808, 0x3808).w(FUNC(ddragon_state::bankswitch_w));
	map(0x3809, 0x3809).writeonly().share(m_scrollx_lo);
	map(0x380a, 0x380a).writeonly().share(m_scrolly_lo);
	map(0x380b, 0x380f).rw(FUNC(ddragon_state::interrupt_r), FUNC(ddragon_state::interrupt_w));
	map(0x3c00, 0x3dff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x3e00, 0x3fff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}


/*************************************
 *
 *  Sub CPU memory maps
 *
 *************************************/

void ddragon_state::ddragon_sub_map(address_map &map)
{
	map(0x8000, 0x81ff).ram().share(m_comram);
}

void ddragon_state::sub_6309_map(address_map &map)
{
	map(0x0005, 0x0005).nopw();
	map(0x0016, 0x0016).nopw();
	map(0x0017, 0x0017).w(FUNC(ddragon_state::sub_port6_w));
	map(0x0020, 0x0fff).ram();
	map(0x8000, 0x81ff).ram().share(m_comram);
	map(0xc000, 0xffff).rom();
}

void ddragon_state::sub_6809_map(address_map &map) // TODO: everything
{
	map(0x8000, 0xffff).rom().region("sub", 0);
}

void ddragon_state::ddragonbla_sub_map(address_map &map)
{
	map(0x0100, 0x0fff).ram();
	map(0x8000, 0x81ff).ram().share(m_comram);
	map(0xc000, 0xffff).rom();
}

void ddragon_state::ddragon2_sub_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc3ff).ram().share(m_comram);
	map(0xd000, 0xd000).w(FUNC(ddragon_state::ddragon2_sub_irq_ack_w));
	map(0xe000, 0xe000).w(FUNC(ddragon_state::ddragon2_sub_irq_w));
}


/*************************************
 *
 *  Sound CPU memory maps
 *
 *************************************/

void ddragon_state::ddragon_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x1800, 0x1800).r(FUNC(ddragon_state::ddragon_adpcm_status_r));
	map(0x2800, 0x2801).rw("fmsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x3800, 0x3807).w(FUNC(ddragon_state::ddragon_adpcm_w));
	map(0x8000, 0xffff).rom();
}

void ddragon_state::ddragon6809_sound_map(address_map &map) // TODO: everything
{
	// 2x YM2203, 2xMSM5205
	map(0x8000, 0xffff).rom();
}

void ddragon_state::ddragon2_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("fmsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( ddragon )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "20k" )
	PORT_DIPSETTING(    0x00, "40k" )
	PORT_DIPSETTING(    0x30, "30k and every 60k" )
	PORT_DIPSETTING(    0x20, "40k and every 80k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ddragon_state::subcpu_bus_free_r))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( ddragon2 )
	PORT_INCLUDE(ddragon)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x00, "Hurricane Kick" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x30, 0x10, "Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPSETTING(    0x30, "Normal" )
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END


static INPUT_PORTS_START( tstrike )
	PORT_INCLUDE(ddragon)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0xc0, "100k and 200k" )
	PORT_DIPSETTING(    0x80, "200k and 300k" )
	PORT_DIPSETTING(    0x40, "300k and 400k" )
	PORT_DIPSETTING(    0x00, "400k and 500k" )
INPUT_PORTS_END


static INPUT_PORTS_START( ddungeon )
	PORT_INCLUDE(ddragon)

	// Dangerous Dungeons installation guide recommends 4-way joystick "for maximum profits"
	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0xf0, 0x90, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x90, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x70, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( darktowr )
	PORT_INCLUDE(ddungeon)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( toffy )
	PORT_INCLUDE(ddungeon)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x0f, "4 Coins/6 Credits" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0e, "3 Coins/6 Credits" )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0xf0, "4 Coins/6 Credits" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xe0, "3 Coins/6 Credits" )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "30k, 50k and 100k" )
	PORT_DIPSETTING(    0x08, "50k and 100k" )
	PORT_DIPSETTING(    0x18, "100k and 200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,2) },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ STEP8(0,8) },
	32*8
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
			32*8+3, 32*8+2, 32*8+1, 32*8+0, 48*8+3, 48*8+2, 48*8+1, 48*8+0 },
	{ STEP16(0,8) },
	64*8
};


static GFXDECODE_START( gfx_ddragon )
	GFXDECODE_ENTRY( "chars",   0, char_layout,   0, 8 )   // colors   0-127
	GFXDECODE_ENTRY( "sprites", 0, tile_layout, 128, 8 )   // colors 128-255
	GFXDECODE_ENTRY( "tiles",   0, tile_layout, 256, 8 )   // colors 256-383
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static constexpr XTAL MAIN_CLOCK = 12_MHz_XTAL;
static constexpr XTAL SOUND_CLOCK = 3.579545_MHz_XTAL;
static constexpr XTAL PIXEL_CLOCK = MAIN_CLOCK / 2;

void ddragon_state::ddragon(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, MAIN_CLOCK / 4); // HD63C09EP, 3 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &ddragon_state::ddragon_main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(ddragon_state::scanline), "screen", 0, 1);

	hd63701y0_cpu_device &subcpu(HD63701Y0(config, m_subcpu, MAIN_CLOCK / 2)); // HD63701Y0P, 6 MHz / 4 internally
	subcpu.set_addrmap(AS_PROGRAM, &ddragon_state::ddragon_sub_map);
	subcpu.out_p6_cb().set(FUNC(ddragon_state::sub_port6_w));

	MC6809(config, m_soundcpu, MAIN_CLOCK / 2); // HD68A09P, 6 MHz / 4 internally
	m_soundcpu->set_addrmap(AS_PROGRAM, &ddragon_state::ddragon_sound_map);

	config.set_maximum_quantum(attotime::from_hz(60000)); // heavy interleaving to sync up sprite<->main CPUs

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ddragon);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240);
	m_screen->set_screen_update(FUNC(ddragon_state::screen_update));
	m_screen->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, M6809_IRQ_LINE);

	ym2151_device &fmsnd(YM2151(config, "fmsnd", SOUND_CLOCK));
	fmsnd.irq_handler().set_inputline(m_soundcpu, M6809_FIRQ_LINE);
	fmsnd.add_route(0, "mono", 0.35);
	fmsnd.add_route(1, "mono", 0.35);

	MSM5205(config, m_adpcm[0], MAIN_CLOCK / 32);
	m_adpcm[0]->vck_legacy_callback().set(FUNC(ddragon_state::ddragon_adpcm_int<0>));
	m_adpcm[0]->set_prescaler_selector(msm5205_device::S48_4B); // 8kHz
	m_adpcm[0]->add_route(ALL_OUTPUTS, "mono", 1.0);

	MSM5205(config, m_adpcm[1], MAIN_CLOCK / 32);
	m_adpcm[1]->vck_legacy_callback().set(FUNC(ddragon_state::ddragon_adpcm_int<1>));
	m_adpcm[1]->set_prescaler_selector(msm5205_device::S48_4B); // 8kHz
	m_adpcm[1]->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void ddragon_state::ddragonbl(machine_config &config)
{
	ddragon(config);

	// basic machine hardware
	HD6309E(config.replace(), m_subcpu, MAIN_CLOCK / 8); // 1.5MHz; labeled "ENC EL1200AR" on one PCB
	m_subcpu->set_addrmap(AS_PROGRAM, &ddragon_state::sub_6309_map);
}

void ddragon_state::ddragonbla(machine_config &config)
{
	ddragon(config);

	// basic machine hardware
	m6803_cpu_device &sub(M6803(config.replace(), "sub", MAIN_CLOCK / 2)); // 6MHz / 4 internally
	sub.set_addrmap(AS_PROGRAM, &ddragon_state::ddragonbla_sub_map);
	sub.out_p2_cb().set(FUNC(ddragon_state::ddragonbla_port_w));
}


/*
3x EF68B09EP (main)
2x YM2203C (sound)
2x OKI M5202 (sound)
2x Y3014B (sound)
2x LM324N (sound)
1x TDA2003 (sound)
1x oscillator 20.000 near sound section
1x oscillator 24.575 (24.000 on another PCB) near main and sub CPU
1x orange resonator CSB445E (CSB500E on another PCB)
*/
void ddragon_state::ddragon6809(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 24_MHz_XTAL / 16); // divisor not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &ddragon_state::ddragon_main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(ddragon_state::scanline), "screen", 0, 1);

	MC6809E(config, m_subcpu, 24_MHz_XTAL / 16); // divisor not verified
	m_subcpu->set_addrmap(AS_PROGRAM, &ddragon_state::sub_6809_map);

	MC6809E(config, m_soundcpu, 20_MHz_XTAL / 12); // divisor not verified
	m_soundcpu->set_addrmap(AS_PROGRAM, &ddragon_state::ddragon6809_sound_map);

	config.set_maximum_quantum(attotime::from_hz(60000)); // heavy interleaving to sync up sprite<->main CPUs

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ddragon);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240);
	m_screen->set_screen_update(FUNC(ddragon_state::screen_update));
	m_screen->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, M6809_IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 20_MHz_XTAL / 6)); // divisor not verified
	ym1.add_route(ALL_OUTPUTS, "mono", 0.35);

	ym2203_device &ym2(YM2203(config, "ym2", 20_MHz_XTAL / 6)); // divisor not verified
	ym2.add_route(ALL_OUTPUTS, "mono", 0.35);

	MSM5205(config, m_adpcm[0], 500_kHz_XTAL);
	m_adpcm[0]->vck_legacy_callback().set(FUNC(ddragon_state::ddragon_adpcm_int<0>));
	m_adpcm[0]->set_prescaler_selector(msm5205_device::S48_4B); // 8kHz
	m_adpcm[0]->add_route(ALL_OUTPUTS, "mono", 1.0);

	MSM5205(config, m_adpcm[1], 500_kHz_XTAL);
	m_adpcm[1]->vck_legacy_callback().set(FUNC(ddragon_state::ddragon_adpcm_int<1>));
	m_adpcm[1]->set_prescaler_selector(msm5205_device::S48_4B); // 8kHz
	m_adpcm[1]->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void ddragon_state::ddragon2(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, MAIN_CLOCK / 4); // HD63C09EP, 3 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &ddragon_state::ddragon2_main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(ddragon_state::scanline), "screen", 0, 1);

	Z80(config, m_subcpu, MAIN_CLOCK / 3); // 4 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &ddragon_state::ddragon2_sub_map);

	Z80(config, m_soundcpu, 3'579'545);
	m_soundcpu->set_addrmap(AS_PROGRAM, &ddragon_state::ddragon2_sound_map);

	config.set_maximum_quantum(attotime::from_hz(60000)); // heavy interleaving to sync up sprite<->main CPUs

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ddragon);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240);
	m_screen->set_screen_update(FUNC(ddragon_state::screen_update));
	m_screen->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	ym2151_device &fmsnd(YM2151(config, "fmsnd", SOUND_CLOCK));
	fmsnd.irq_handler().set_inputline(m_soundcpu, 0);
	fmsnd.add_route(0, "mono", 0.35);
	fmsnd.add_route(1, "mono", 0.35);

	okim6295_device &oki(OKIM6295(config, "oki", 1'056'000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 verified on bootleg PCB by Jose Tejada
	oki.add_route(ALL_OUTPUTS, "mono", 0.20);
}

void darktowr_state::darktowr(machine_config &config)
{
	ddragon(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &darktowr_state::main_map);

	M68705P3(config, m_mcu, XTAL(4'000'000));
	m_mcu->porta_w().set(FUNC(darktowr_state::mcu_port_a_w));

	ADDRESS_MAP_BANK(config, "darktowr_bank").set_map(&darktowr_state::banked_map).set_options(ENDIANNESS_BIG, 8, 17, 0x4000);
}

void toffy_state::toffy(machine_config &config)
{
	ddragon(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &toffy_state::main_map);

	config.device_remove("sub");

	// sound hardware
	config.device_remove("adpcm1");
	config.device_remove("adpcm2");
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ddragon )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "21j-1.26",     0x08000, 0x08000, CRC(ae714964) SHA1(072522b97ca4edd099c6b48d7634354dc7088c53) )
	ROM_LOAD( "21j-2-3.25",   0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-3.24",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21j-4.23",     0x20000, 0x08000, CRC(6c9f46fa) SHA1(df251a4aea69b2328f7a543bf085b9c35933e2c1) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x4000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21jm-0.ic55",    0x0000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) // 63701Y0P MCU

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

ROM_START( ddragona )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "e1-1.26",       0x08000, 0x08000, CRC(4b951643) SHA1(efb1f9ef2e46597d76123c9770854c1d83639eb2) )
	ROM_LOAD( "21a-2-4.25",    0x10000, 0x08000, CRC(5cd67657) SHA1(96bc7a5354a76524bd43a4d7eb8b0053a89e39c4) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-3.24",      0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) // banked at 0x4000-0x8000
	ROM_LOAD( "e4-1.23",       0x20000, 0x08000, CRC(b1e26935) SHA1(dfff666fd5e9dc4dfb2a1d891eced88730cbaf30) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x4000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21jm-0.ic55",    0x0000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) // 63701Y0P MCU

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

ROM_START( ddragonu )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "21a-1-5.26",   0x08000, 0x08000, CRC(e24a6e11) SHA1(9dd97dd712d5c896f91fd80df58be9b8a2b198ee) )
	ROM_LOAD( "21j-2-3.25",   0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-3.24",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-4.23",     0x20000, 0x08000, CRC(6ea16072) SHA1(0b3b84a0d54f7a3aba411586009babbfee653f9a) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x4000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21jm-0.ic55",    0x0000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) // 63701Y0P MCU

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

ROM_START( ddragonua )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "21a-1",     0x08000, 0x08000, CRC(1d625008) SHA1(84cc19a55e7c91fca1943d9624d93e0347ed4150) )
	ROM_LOAD( "21a-2_4",   0x10000, 0x08000, CRC(5cd67657) SHA1(96bc7a5354a76524bd43a4d7eb8b0053a89e39c4) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-3",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-4_2",   0x20000, 0x08000, CRC(9b019598) SHA1(59f3aa15389f53c4646d21a39634cb1502e66ff6) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x4000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21jm-0.ic55",    0x0000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) // 63701Y0P MCU

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END


ROM_START( ddragonub )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "21a-1_6.bin",0x08000,0x08000, CRC(f354b0e1) SHA1(f2fe5d6102564691a0054d2b8dd98673fdc8a348) )
	ROM_LOAD( "21a-2_4",   0x10000, 0x08000, CRC(5cd67657) SHA1(96bc7a5354a76524bd43a4d7eb8b0053a89e39c4) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-3",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-4_2",   0x20000, 0x08000, CRC(9b019598) SHA1(59f3aa15389f53c4646d21a39634cb1502e66ff6) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x4000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21jm-0.ic55",    0x0000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) // 63701Y0P MCU

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

ROM_START( ddragonj )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "21j-1-5.26",   0x08000, 0x08000, CRC(42045dfd) SHA1(0983705ea3bb87c4c239692f400e02f15c243479) )
	ROM_LOAD( "21j-2-3.25",   0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21j-3.24",     0x18000, 0x08000, CRC(3bdea613) SHA1(d9038c80646a6ce3ea61da222873237b0383680e) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21j-4-1.23",   0x20000, 0x08000, CRC(728f87b9) SHA1(d7442be24d41bb9fc021587ef44ae5b830e4503d) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x4000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21jm-0.ic55",    0x0000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) // 63701Y0P MCU

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0",      0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // Layer priority
	ROM_LOAD( "21j-l-0",      0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // unknown
ROM_END

ROM_START( ddragonja )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "21j-1-8.26",   0x08000, 0x08000, BAD_DUMP CRC(54c1ef98) SHA1(7ad9db27b5b73ab0470b73e846d5733534fe04af) ) // didn't read consistently, hand-fixed
	ROM_LOAD( "21j-2-4.25",   0x10000, 0x08000, CRC(5cd67657) SHA1(96bc7a5354a76524bd43a4d7eb8b0053a89e39c4) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21j-3.24",     0x18000, 0x08000, CRC(3bdea613) SHA1(d9038c80646a6ce3ea61da222873237b0383680e) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21j-4-2.23",   0x20000, 0x08000, CRC(6312767d) SHA1(d18b865c3f0819d7f8fe0ec8b81876ca11437893) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x4000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21jm-0.ic55",    0x0000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) // 63701Y0P MCU

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0",      0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // Layer priority
	ROM_LOAD( "21j-l-0",      0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // unknown
ROM_END

ROM_START( ddragonbl ) // Same program ROMs as the World set
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "21j-1.26",     0x08000, 0x08000, CRC(ae714964) SHA1(072522b97ca4edd099c6b48d7634354dc7088c53) )
	ROM_LOAD( "21j-2-3.25",   0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21a-3.24",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) // banked at 0x4000-0x8000
	ROM_LOAD( "21j-4.23",     0x20000, 0x08000, CRC(6c9f46fa) SHA1(df251a4aea69b2328f7a543bf085b9c35933e2c1) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x10000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "ic38",         0x0c000, 0x04000, CRC(6a6a0325) SHA1(98a940a9f23ce9154ff94f7f2ce29efe9a92f71b) ) // HD6903 instead of HD63701

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

ROM_START( ddragonbla )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "5.bin",     0x08000, 0x08000, CRC(ae714964) SHA1(072522b97ca4edd099c6b48d7634354dc7088c53) )
	ROM_LOAD( "4.bin",     0x10000, 0x08000, CRC(48045762) SHA1(ca39eea71ca76627a98210ce9cc61457a58f16b9) ) // banked at 0x4000-0x8000
	ROM_CONTINUE(0x20000,0x8000) // banked at 0x4000-0x8000
	ROM_LOAD( "3.bin",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x10000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "2_32.bin",         0x0c000, 0x04000, CRC(67875473) SHA1(66405cb22d41d353335f037ce5aee69e4c6f05c4) ) // 6803 instead of HD63701

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "6.bin",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "1.bin",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "8.bin",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "7.bin",        0x00000, 0x10000, CRC(f9311f72) SHA1(aa554ef020e04dc896e5495bcddc64e489d0ffff) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

ROM_START( ddragonbl2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "b2_4.bin",     0x08000, 0x08000, CRC(668dfa19) SHA1(9b2ff1b66eeba0989e4ed850b7df1f5719ba5572) )
	ROM_LOAD( "b2_5.bin",     0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) // banked at 0x4000-0x8000
	ROM_LOAD( "b2_6.bin",     0x18000, 0x08000, CRC(3bdea613) SHA1(d9038c80646a6ce3ea61da222873237b0383680e) ) // banked at 0x4000-0x8000
	ROM_LOAD( "b2_7.bin",     0x20000, 0x08000, CRC(728f87b9) SHA1(d7442be24d41bb9fc021587ef44ae5b830e4503d) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x4000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "63701.bin",    0x0000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b2_3.bin",     0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "b2_8.bin",     0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "11.bin",       0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )
	ROM_LOAD( "12.bin",       0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "13.bin",       0x20000, 0x10000, CRC(c8b91e17) SHA1(0ce6f6ef68ecc7309a2923f7e756d5e2bf5c7a4a) )
	ROM_LOAD( "14.bin",       0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "15.bin",       0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "16.bin",       0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "17.bin",       0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "18.bin",       0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "9.bin",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )
	ROM_LOAD( "10.bin",       0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "19.bin",       0x20000, 0x10000, CRC(22d65df2) SHA1(2f286a24ea7af438b39126a4ed0c515745981416) )
	ROM_LOAD( "20.bin",       0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "b2_1.bin",     0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "2.bin",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

/* this is a well known Italian bootleg of Double Dragon. It can be identified by the following gameplay trait
 -- The Boss of level 4 is coloured like level 1 and 5 instead of green, and is invulnerable to rocks attack.

 in terms of code the game code has been heavily modified, banking writes appear to have been removed, and
 the graphic ROMs are all scrambled.  The game also runs on 3x M6809 rather than the original CPUs.

 */
ROM_START( ddragon6809 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "6809_20.bin",   0x08000, 0x08000, CRC(67e3b4f1) SHA1(4945d76b0694299f2f4739ebfba98da6d96fe4cb) )
	ROM_LOAD( "6809_19.bin",   0x10000, 0x08000, CRC(090e2baf) SHA1(29b775c59c7a4d30a33e3d10e736cd1a83baf3bb) ) // banked at 0x4000-0x8000
	ROM_LOAD( "6809_18.bin",   0x18000, 0x08000, CRC(154d50c4) SHA1(4ffdd29406b6c6b552344f820f83715b1c7727d1) ) // banked at 0x4000-0x8000
	ROM_LOAD( "6809_17.bin",   0x20000, 0x08000, CRC(6489d637) SHA1(fd17fd870e9386a3e3bdd56c8d731c73d8c70b88) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x8000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21.bin",      0x00000, 0x08000, CRC(4437fc51) SHA1(fffcf2bec50d0b79861904b4abc607206b7794e6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "6809_16.bin",   0x08000, 0x08000, CRC(f4c72690) SHA1(c70d032355acf3f7f6586b6e57a94f80e099bf1a) )

	// all the gfx ROMs are scrambled on this set
	ROM_REGION( 0x08000, "chars", ROMREGION_ERASEFF )

	ROM_REGION( 0x08000, "enc_chars", 0 )
	ROM_LOAD( "6809_13.bin",   0x00000, 0x08000, CRC(b5a54537) SHA1(a6157cde4f9738565008d11a4a6d8576ae3abfef) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "22.bin",        0x00000, 0x08000, CRC(fe08ef61) SHA1(50404936934dc61f3553add4d4b918529b3b5ef3) )
	ROM_LOAD( "23.bin",        0x08000, 0x08000, CRC(988bea93) SHA1(053ebb5a71dfdb68ae88ef49d8409a99f8c6926d) )
	ROM_LOAD( "24.bin",        0x10000, 0x08000, CRC(437501fc) SHA1(e7758e0fb226ae46eb398bd95f5e95c90b6adb93) )
	ROM_LOAD( "25.bin",        0x18000, 0x08000, CRC(d302f69b) SHA1(64d4d8ae38457ee6b361b5157ec0557f9a7639a8) )
	ROM_LOAD( "26.bin",        0x20000, 0x08000, CRC(8ece953e) SHA1(12a43e1ed1a99b04299941a9506228490649b181) )
	ROM_LOAD( "27.bin",        0x28000, 0x08000, CRC(15cd16cb) SHA1(ab2068ebba14da256e8f2600f34dca0e048a1de9) )
	ROM_LOAD( "28.bin",        0x30000, 0x08000, CRC(51b8a217) SHA1(60c067cd7272f856e29cdb64312535236656891a) )
	ROM_LOAD( "29.bin",        0x38000, 0x08000, CRC(e4ec2394) SHA1(43376ce2a07c1fc3053f7ac9b750e944d289105b) )
	ROM_LOAD( "6809_1.bin",    0x40000, 0x08000, CRC(2485a71d) SHA1(3e987a2f3e9a59da5fdc7bb779a43736ca67aac7) )
	ROM_LOAD( "6809_2.bin",    0x48000, 0x08000, CRC(6940120d) SHA1(bbe94f095ef983f54658c936f916ba6a72a84ead) )
	ROM_LOAD( "6809_3.bin",    0x50000, 0x08000, CRC(c67aac12) SHA1(aab535507e3889bf1bdc2f4fe4828a70a350ba63) )
	ROM_LOAD( "6809_4.bin",    0x58000, 0x08000, CRC(941dcd08) SHA1(266dee264f28affe8c3f57fe569929817ae16508) )
	ROM_LOAD( "6809_5.bin",    0x60000, 0x08000, CRC(42d36bc3) SHA1(080cbc3ffda8ab26dc65a8e9eaf948c509d064b3) )
	ROM_LOAD( "6809_6.bin",    0x68000, 0x08000, CRC(d5d19a8d) SHA1(c4b044dd12d6468c0ad114644f01813d4fe9a673) )
	ROM_LOAD( "6809_7.bin",    0x70000, 0x08000, CRC(d4e350cd) SHA1(78ed2baa8c52b766f998091e7ce9e1a2941352e7) )
	ROM_LOAD( "6809_8.bin",    0x78000, 0x08000, CRC(204fdb7d) SHA1(f75b1bc6f65e7a33927cd451267fcd7e2aa44f7e) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "6809_9.bin",    0x00000, 0x10000, CRC(736eff0f) SHA1(ae2ec2d5c8ab1db579a08256d874426dc5d889c6) )
	ROM_LOAD( "6809_10.bin",   0x10000, 0x10000, CRC(a670d088) SHA1(27e7b49645753dd039f104c3e0a7e6513a98710d) )
	ROM_LOAD( "6809_11.bin",   0x20000, 0x10000, CRC(4171b70d) SHA1(dc300c9bca6481417e97ad03c973e47389f261c1) )
	ROM_LOAD( "6809_12.bin",   0x30000, 0x10000, CRC(5f6a6d6f) SHA1(7d546a226cda81c28e7ccfb4c5daebc65072198d) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "6809_14.bin",   0x00000, 0x08000, CRC(678f8657) SHA1(2652fdc6719d2c889ca87802f6e2cefae59fc2eb) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "6809_15.bin",   0x00000, 0x08000, CRC(10f21dea) SHA1(739cf649f91490384297a81a2cc9855acb58a1c0) )
ROM_END

/*
CPU

3x EF68B09EP (main)
2x YM2203C (sound)
2x OKI M5202 (sound)
2x Y3014B (sound)
2x LM324N (sound)
1x TDA2003 (sound)
1x oscillator 20.000
1x oscillator 24.575 (24.000 on another PCB)
1x orange resonator CSB445E (CSB500E on another PCB)

ROMs

12x 27C512 (1-12)
9x 27256 (13-21)
2x AM27S21PC
1x N82S123N
1x AM27S29PC
1x PAL16r4
1x PAL16R6

Note

1x JAMMA edge connector
1x trimmer (volume)
2x 8x2 switches dip

*/

ROM_START( ddragon6809a )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "20.7f",   0x08000, 0x08000, CRC(c804819f) SHA1(cc570a90b7bef1c6263f5e1fd96ed377c508fe2b) )
	ROM_LOAD( "19.7g",   0x10000, 0x08000, CRC(de08db4d) SHA1(e63b90c3bb3af01d2855de9a996b51068bed7b52) ) // banked at 0x4000-0x8000
	ROM_LOAD( "18.7h",   0x18000, 0x08000, CRC(154d50c4) SHA1(4ffdd29406b6c6b552344f820f83715b1c7727d1) ) // banked at 0x4000-0x8000
	ROM_LOAD( "17.7j",   0x20000, 0x08000, CRC(4052f37a) SHA1(9444a30ce32a2d35c601324d79c0ba602be4f288) ) // banked at 0x4000-0x8000

	ROM_REGION( 0x8000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21.7d",   0x00000, 0x8000, CRC(4437fc51) SHA1(fffcf2bec50d0b79861904b4abc607206b7794e6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "16.7n",   0x08000, 0x08000, CRC(f4c72690) SHA1(c70d032355acf3f7f6586b6e57a94f80e099bf1a) )

	// all the gfx roms are scrambled on this set
	ROM_REGION( 0x08000, "chars", ROMREGION_ERASEFF )

	ROM_REGION( 0x08000, "enc_chars", 0 )
	ROM_LOAD( "13.5f",   0x00000, 0x08000, CRC(b5a54537) SHA1(a6157cde4f9738565008d11a4a6d8576ae3abfef) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "1.1t",         0x00000, 0x10000, CRC(5e810a6d) SHA1(5eba3e982b271bc284ca333429cd0b3759c9c8d1) )
	ROM_LOAD( "2.1r",         0x10000, 0x10000, CRC(7300b785) SHA1(6d3b72bd7208e2bd790517a753c9d5192c88d20f) )
	ROM_LOAD( "3.1q",         0x20000, 0x10000, CRC(19405de8) SHA1(ac1aa40478b92af5ccdde89812be78b7c9f7d20d) )
	ROM_LOAD( "4.1p",         0x30000, 0x10000, CRC(4b10defd) SHA1(fb43eba7c8a7f77f0fdd6253d51b40b0e64598f5) )
	ROM_LOAD( "5.1n",         0x40000, 0x10000, CRC(5b1bb493) SHA1(dd947d7d381af5952acece4b2cefc9fc4847ec68) )
	ROM_LOAD( "6.1m",         0x50000, 0x10000, CRC(e8a2d2e7) SHA1(abc871e57a5280728b9f90625fb91011b848a4d8) )
	ROM_LOAD( "7.1l",         0x60000, 0x10000, CRC(8010fcca) SHA1(9401c41088776beea91c32aaff8eb2fbe92b5e37) )
	ROM_LOAD( "8.1j",         0x70000, 0x10000, CRC(bfa4da27) SHA1(68a649aec43e18dc79b4690c1dff2e2a6fc0065a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "9.2e",         0x00000, 0x10000, CRC(736eff0f) SHA1(ae2ec2d5c8ab1db579a08256d874426dc5d889c6) )
	ROM_LOAD( "10.2d",        0x10000, 0x10000, CRC(a670d088) SHA1(27e7b49645753dd039f104c3e0a7e6513a98710d) )
	ROM_LOAD( "11.2b",        0x20000, 0x10000, CRC(4171b70d) SHA1(dc300c9bca6481417e97ad03c973e47389f261c1) )
	ROM_LOAD( "12.2a",        0x30000, 0x10000, CRC(5f6a6d6f) SHA1(7d546a226cda81c28e7ccfb4c5daebc65072198d) )

	ROM_REGION( 0x10000, "adpcm1", 0 ) // yes these really are smaller than the original game..
	ROM_LOAD( "14.7q",        0x00000, 0x08000, CRC(678f8657) SHA1(2652fdc6719d2c889ca87802f6e2cefae59fc2eb) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "15.7o",        0x00000, 0x08000, CRC(10f21dea) SHA1(739cf649f91490384297a81a2cc9855acb58a1c0) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "27s21.5o",        0x00000, 0x100, CRC(673f68c3) SHA1(9381453e8f868d80b6069264509a339e20e2b6b1) )
	ROM_LOAD( "27s21.5p",        0x00000, 0x100, CRC(2dc270f2) SHA1(9f124ab2c98680bcc249218ee0de09ba49c09a84) )
	ROM_LOAD( "27s29.6g",        0x00000, 0x200, CRC(095fb461) SHA1(7fd213fd8b8bbe30334523ccf06d4606c67b472e) )
	ROM_LOAD( "82s129.4h",       0x00000, 0x100, CRC(7683cadd) SHA1(ff6fecf273c1d8812814cacc72fb71642ec32b6d) )

	ROM_REGION( 0x20000, "plds", 0 )
	ROM_LOAD( "pal16r4.8g",        0x00000, 0x104, CRC(5b0263fd) SHA1(ddca425f82f5eb06b56f2ab116fb9a9b192e1097) )
	ROM_LOAD( "pal16r6.2f",        0x00000, 0x104, CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e) )
ROM_END

/*
  Double Dragon
  Single bootleg board from Argentina

  PCB etched 10-07-87

  CPU:
  3x EF68B09EP

  RAM:
  2x TMM2016BP (1-1)
  7x AM2148/49 (3-2-2)
  1x TMM2064

  Audio:
  2x YM2203
  2x OKI 5205
  2x Y3014
  1x TDA2002 (8W audio amplifier)

  Other:
  1x 24.576 MHz crystal
  1x 20.000 MHz crystal
  1x 3.579545 MHz crystal (for audio)
  2x 8 DIP switches banks
  1x Jamma edge connector

*/
ROM_START( ddragon6809b )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "20.7e",   0x08000, 0x08000, CRC(5002d27d) SHA1(719dabbd5bf38a647a09296daba76b3e928f3d7b) ) // lot of differences
	ROM_LOAD( "19.7g",   0x10000, 0x08000, CRC(398e950d) SHA1(b090f0ef9cc616c507a7ab3f80413dd0dc4d3655) ) // banked at 0x4000-0x8000, 4 bytes different
	ROM_LOAD( "18.7h",   0x18000, 0x08000, CRC(154d50c4) SHA1(4ffdd29406b6c6b552344f820f83715b1c7727d1) ) // banked at 0x4000-0x8000
	ROM_LOAD( "17.7i",   0x20000, 0x08000, CRC(6489d637) SHA1(fd17fd870e9386a3e3bdd56c8d731c73d8c70b88) ) // banked at 0x4000-0x8000, removed copyright

	ROM_REGION( 0x8000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "21.7d",   0x00000, 0x8000, CRC(4437fc51) SHA1(fffcf2bec50d0b79861904b4abc607206b7794e6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "16.7n",   0x08000, 0x08000, CRC(f4c72690) SHA1(c70d032355acf3f7f6586b6e57a94f80e099bf1a) )

	// all the gfx roms are scrambled on this set
	ROM_REGION( 0x08000, "chars", ROMREGION_ERASEFF )

	ROM_REGION( 0x08000, "enc_chars", 0 )
	ROM_LOAD( "13.5f",   0x00000, 0x08000, CRC(b5a54537) SHA1(a6157cde4f9738565008d11a4a6d8576ae3abfef) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "1.1t",         0x00000, 0x10000, CRC(5e810a6d) SHA1(5eba3e982b271bc284ca333429cd0b3759c9c8d1) )
	ROM_LOAD( "2.1r",         0x10000, 0x10000, CRC(7300b785) SHA1(6d3b72bd7208e2bd790517a753c9d5192c88d20f) )
	ROM_LOAD( "3.1q",         0x20000, 0x10000, CRC(19405de8) SHA1(ac1aa40478b92af5ccdde89812be78b7c9f7d20d) )
	ROM_LOAD( "4.1p",         0x30000, 0x10000, CRC(4b10defd) SHA1(fb43eba7c8a7f77f0fdd6253d51b40b0e64598f5) )
	ROM_LOAD( "5.1n",         0x40000, 0x10000, CRC(5b1bb493) SHA1(dd947d7d381af5952acece4b2cefc9fc4847ec68) )
	ROM_LOAD( "6.1m",         0x50000, 0x10000, CRC(e8a2d2e7) SHA1(abc871e57a5280728b9f90625fb91011b848a4d8) )
	ROM_LOAD( "7.1l",         0x60000, 0x10000, CRC(8010fcca) SHA1(9401c41088776beea91c32aaff8eb2fbe92b5e37) )
	ROM_LOAD( "8.1j",         0x70000, 0x10000, CRC(bfa4da27) SHA1(68a649aec43e18dc79b4690c1dff2e2a6fc0065a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "9.2e",         0x00000, 0x10000, CRC(736eff0f) SHA1(ae2ec2d5c8ab1db579a08256d874426dc5d889c6) )
	ROM_LOAD( "10.2d",        0x10000, 0x10000, CRC(a670d088) SHA1(27e7b49645753dd039f104c3e0a7e6513a98710d) )
	ROM_LOAD( "11.2c",        0x20000, 0x10000, CRC(4171b70d) SHA1(dc300c9bca6481417e97ad03c973e47389f261c1) )
	ROM_LOAD( "12.2a",        0x30000, 0x10000, CRC(5f6a6d6f) SHA1(7d546a226cda81c28e7ccfb4c5daebc65072198d) )

	ROM_REGION( 0x10000, "adpcm1", 0 ) // yes these really are smaller than the original game..
	ROM_LOAD( "14.7q",        0x00000, 0x08000, CRC(678f8657) SHA1(2652fdc6719d2c889ca87802f6e2cefae59fc2eb) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "15.7o",        0x00000, 0x08000, CRC(10f21dea) SHA1(739cf649f91490384297a81a2cc9855acb58a1c0) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "27s21.5o",        0x00000, 0x100, CRC(673f68c3) SHA1(9381453e8f868d80b6069264509a339e20e2b6b1) )
	ROM_LOAD( "27s21.5p",        0x00000, 0x100, CRC(2dc270f2) SHA1(9f124ab2c98680bcc249218ee0de09ba49c09a84) )
	ROM_LOAD( "27s29.6g",        0x00000, 0x200, CRC(095fb461) SHA1(7fd213fd8b8bbe30334523ccf06d4606c67b472e) )
	ROM_LOAD( "82s129.4h",       0x00000, 0x100, CRC(7683cadd) SHA1(ff6fecf273c1d8812814cacc72fb71642ec32b6d) )
ROM_END


ROM_START( ddragon2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "26a9-04.bin",  0x08000, 0x8000, CRC(f2cfc649) SHA1(d3f1e0bae02472914a940222e4f600170a91736d) )
	ROM_LOAD( "26aa-03.bin",  0x10000, 0x8000, CRC(44dd5d4b) SHA1(427c4e419668b41545928cfc96435c010ecdc88b) )
	ROM_LOAD( "26ab-0.bin",   0x18000, 0x8000, CRC(49ddddcd) SHA1(91dc53718d04718b313f23d86e241027c89d1a03) )
	ROM_LOAD( "26ac-0e.63",   0x20000, 0x8000, CRC(57acad2c) SHA1(938e2a78af38ecd7e9e08fb10acc1940f7585f5e) )

	ROM_REGION( 0x10000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "26ae-0.bin",   0x00000, 0x10000, CRC(ea437867) SHA1(cd910203af0565f981b9bdef51ea6e9c33ee82d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "26ad-0.bin",   0x00000, 0x8000, CRC(75e36cd6) SHA1(f24805f4f6925b3ac508e66a6fc25c275b05f3b9) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "26a8-0e.19",   0x00000, 0x10000, CRC(4e80cd36) SHA1(dcae0709f27f32effb359f6b943f61b102749f2a) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD( "26j0-0.bin",   0x00000, 0x20000, CRC(db309c84) SHA1(ee095e4a3bc86737539784945decb1f63da47b9b) )
	ROM_LOAD( "26j1-0.bin",   0x20000, 0x20000, CRC(c3081e0c) SHA1(c4a9ae151aae21073a2c79c5ac088c72d4f3d9db) )
	ROM_LOAD( "26af-0.bin",   0x40000, 0x20000, CRC(3a615aad) SHA1(ec90a35224a177d00327de6fd1a299df38abd790) )
	ROM_LOAD( "26j2-0.bin",   0x60000, 0x20000, CRC(589564ae) SHA1(1e6e0ef623545615e8409b6d3ba586a71e2612b6) )
	ROM_LOAD( "26j3-0.bin",   0x80000, 0x20000, CRC(daf040d6) SHA1(ab0fd5482625dbe64f0f0b0baff5dcde05309b81) )
	ROM_LOAD( "26a10-0.bin",  0xa0000, 0x20000, CRC(6d16d889) SHA1(3bc62b3e7f4ddc3200a9cf8469239662da80c854) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "26j4-0.bin",   0x00000, 0x20000, CRC(a8c93e76) SHA1(54d64f052971e7fa0d21c5ce12f87b0fa2b648d6) )
	ROM_LOAD( "26j5-0.bin",   0x20000, 0x20000, CRC(ee555237) SHA1(f9698f3e57f933a43e508f60667c860dee034d05) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "26j6-0.bin",   0x00000, 0x20000, CRC(a84b2a29) SHA1(9cb529e4939c16a0a42f45dd5547c76c2f86f07b) )
	ROM_LOAD( "26j7-0.bin",   0x20000, 0x20000, CRC(bc6a48d5) SHA1(04c434f8cd42a8f82a263548183569396f9b684d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0",      0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // Layer priority (same as ddragon)
	ROM_LOAD( "prom.16",      0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing (same as ddragon)
ROM_END


ROM_START( ddragon2j )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "26a9-0_j.ic38", 0x08000, 0x8000, CRC(5e4fcdff) SHA1(78bf79a0b4f248c3355fef40448c76eb028f9163) )
	ROM_LOAD( "26aa-0_j.ic52", 0x10000, 0x8000, CRC(bfb4ee04) SHA1(3692bbdef7d5b7cc3eb76362945b91b4a0f6ad4b) )
	ROM_LOAD( "26ab-0.ic53",   0x18000, 0x8000, CRC(49ddddcd) SHA1(91dc53718d04718b313f23d86e241027c89d1a03) )
	ROM_LOAD( "26ac-0_j.ic63", 0x20000, 0x8000, CRC(165858c7) SHA1(a00953df924cff9e79d28061849070f5401014d7) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "26ae-0.ic37",   0x00000, 0x10000, CRC(ea437867) SHA1(cd910203af0565f981b9bdef51ea6e9c33ee82d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "26ad-0.ic41",   0x00000, 0x8000, CRC(3788af3b) SHA1(7f8833b01522553c767c470a9c27d24e638f37b9) ) // why is this different, label was the same

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "26a8-0e.19",   0x00000, 0x10000, CRC(4e80cd36) SHA1(dcae0709f27f32effb359f6b943f61b102749f2a) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD( "26j0-0.bin",   0x00000, 0x20000, CRC(db309c84) SHA1(ee095e4a3bc86737539784945decb1f63da47b9b) )
	ROM_LOAD( "26j1-0.bin",   0x20000, 0x20000, CRC(c3081e0c) SHA1(c4a9ae151aae21073a2c79c5ac088c72d4f3d9db) )
	ROM_LOAD( "26af-0.bin",   0x40000, 0x20000, CRC(3a615aad) SHA1(ec90a35224a177d00327de6fd1a299df38abd790) )
	ROM_LOAD( "26j2-0.bin",   0x60000, 0x20000, CRC(589564ae) SHA1(1e6e0ef623545615e8409b6d3ba586a71e2612b6) )
	ROM_LOAD( "26j3-0.bin",   0x80000, 0x20000, CRC(daf040d6) SHA1(ab0fd5482625dbe64f0f0b0baff5dcde05309b81) )
	ROM_LOAD( "26a10-0.bin",  0xa0000, 0x20000, CRC(6d16d889) SHA1(3bc62b3e7f4ddc3200a9cf8469239662da80c854) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "26j4-0.bin",   0x00000, 0x20000, CRC(a8c93e76) SHA1(54d64f052971e7fa0d21c5ce12f87b0fa2b648d6) )
	ROM_LOAD( "26j5-0.bin",   0x20000, 0x20000, CRC(ee555237) SHA1(f9698f3e57f933a43e508f60667c860dee034d05) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "26j6-0.bin",   0x00000, 0x20000, CRC(a84b2a29) SHA1(9cb529e4939c16a0a42f45dd5547c76c2f86f07b) )
	ROM_LOAD( "26j7-0.bin",   0x20000, 0x20000, CRC(bc6a48d5) SHA1(04c434f8cd42a8f82a263548183569396f9b684d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0",      0x0000, 0x0100, BAD_DUMP CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // Layer priority (same as ddragon)
	ROM_LOAD( "prom.16",      0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing (same as ddragon)
ROM_END

ROM_START( ddragon2u )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "26a9-04.bin",  0x08000, 0x8000, CRC(f2cfc649) SHA1(d3f1e0bae02472914a940222e4f600170a91736d) )
	ROM_LOAD( "26aa-03.bin",  0x10000, 0x8000, CRC(44dd5d4b) SHA1(427c4e419668b41545928cfc96435c010ecdc88b) )
	ROM_LOAD( "26ab-0.bin",   0x18000, 0x8000, CRC(49ddddcd) SHA1(91dc53718d04718b313f23d86e241027c89d1a03) )
	ROM_LOAD( "26ac-02.bin",  0x20000, 0x8000, CRC(097eaf26) SHA1(60504abd30fec44c45197cdf3832c87d05ef577d) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "26ae-0.bin",   0x00000, 0x10000, CRC(ea437867) SHA1(cd910203af0565f981b9bdef51ea6e9c33ee82d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "26ad-0.bin",   0x00000, 0x8000, CRC(75e36cd6) SHA1(f24805f4f6925b3ac508e66a6fc25c275b05f3b9) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "26a8-0.bin",   0x00000, 0x10000, CRC(3ad1049c) SHA1(11d9544a56f8e6a84beb307a5c8a9ff8afc55c66) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD( "26j0-0.bin",   0x00000, 0x20000, CRC(db309c84) SHA1(ee095e4a3bc86737539784945decb1f63da47b9b) )
	ROM_LOAD( "26j1-0.bin",   0x20000, 0x20000, CRC(c3081e0c) SHA1(c4a9ae151aae21073a2c79c5ac088c72d4f3d9db) )
	ROM_LOAD( "26af-0.bin",   0x40000, 0x20000, CRC(3a615aad) SHA1(ec90a35224a177d00327de6fd1a299df38abd790) )
	ROM_LOAD( "26j2-0.bin",   0x60000, 0x20000, CRC(589564ae) SHA1(1e6e0ef623545615e8409b6d3ba586a71e2612b6) )
	ROM_LOAD( "26j3-0.bin",   0x80000, 0x20000, CRC(daf040d6) SHA1(ab0fd5482625dbe64f0f0b0baff5dcde05309b81) )
	ROM_LOAD( "26a10-0.bin",  0xa0000, 0x20000, CRC(6d16d889) SHA1(3bc62b3e7f4ddc3200a9cf8469239662da80c854) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "26j4-0.bin",   0x00000, 0x20000, CRC(a8c93e76) SHA1(54d64f052971e7fa0d21c5ce12f87b0fa2b648d6) )
	ROM_LOAD( "26j5-0.bin",   0x20000, 0x20000, CRC(ee555237) SHA1(f9698f3e57f933a43e508f60667c860dee034d05) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "26j6-0.bin",   0x00000, 0x20000, CRC(a84b2a29) SHA1(9cb529e4939c16a0a42f45dd5547c76c2f86f07b) )
	ROM_LOAD( "26j7-0.bin",   0x20000, 0x20000, CRC(bc6a48d5) SHA1(04c434f8cd42a8f82a263548183569396f9b684d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0",      0x0000, 0x0100, BAD_DUMP CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // Layer priority (same as ddragon)
	ROM_LOAD( "prom.16",      0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing (same as ddragon)
ROM_END

ROM_START( ddragon2bl )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "3",   0x08000, 0x8000, CRC(5cc38bad) SHA1(8ebbb998cce48b5baa4a738c2d4c2e481e2637fb) )
	ROM_LOAD( "4",   0x10000, 0x8000, CRC(78750947) SHA1(6b8349c3cd27c37a4329cea213b6ff0167c4edee) )
	ROM_LOAD( "5",   0x18000, 0x8000, CRC(49ddddcd) SHA1(91dc53718d04718b313f23d86e241027c89d1a03) )
	ROM_LOAD( "6",   0x20000, 0x8000, CRC(097eaf26) SHA1(60504abd30fec44c45197cdf3832c87d05ef577d) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "2",   0x00000, 0x10000, CRC(ea437867) SHA1(cd910203af0565f981b9bdef51ea6e9c33ee82d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "11",   0x00000, 0x8000, CRC(75e36cd6) SHA1(f24805f4f6925b3ac508e66a6fc25c275b05f3b9) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "1",   0x00000, 0x10000, CRC(3ad1049c) SHA1(11d9544a56f8e6a84beb307a5c8a9ff8afc55c66) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD( "27",   0x00000, 0x10000, CRC(fe42df5d) SHA1(aab801346c2db04263cb61c97c6e086387675586) )
	ROM_LOAD( "26",   0x10000, 0x10000, CRC(42f582c6) SHA1(bb269f677321f706043aa33a12bd3ddda4c32e55) )
	ROM_LOAD( "23",   0x20000, 0x10000, CRC(e157319f) SHA1(8b898fa20329b12293e7cb7ffc2e1b17304f826f) )
	ROM_LOAD( "22",   0x30000, 0x10000, CRC(82e952c9) SHA1(d340262c11f3c0ef3640c487e6a78745a2fe97d4) )
	ROM_LOAD( "25",   0x40000, 0x10000, CRC(4a4a085d) SHA1(80786c6fda135af1f9e9d8191879ab27baf36167) )
	ROM_LOAD( "24",   0x50000, 0x10000, CRC(c9d52536) SHA1(54f9236c4d22e3fd79d66c3f45b134f1fc9a1d32) )
	ROM_LOAD( "21",   0x60000, 0x10000, CRC(32ab0897) SHA1(f992dc3876621896b6e1fd6518f576b48d54a631) )
	ROM_LOAD( "20",   0x70000, 0x10000, CRC(a68e168f) SHA1(6ae596c097d7d435b767207012de1d23316d86d4) )
	ROM_LOAD( "17",   0x80000, 0x10000, CRC(882f99b1) SHA1(2fbb9171a2c9ddab177efe1e89e96426643d382b) )
	ROM_LOAD( "16",   0x90000, 0x10000, CRC(e2fe3eca) SHA1(bfd2e91261b9a002a99998486a2b606d4ee2e59b) )
	ROM_LOAD( "18",   0xa0000, 0x10000, CRC(0e1c6c63) SHA1(506e43161992c41d9b77c1df11228117f0587cbd) )
	ROM_LOAD( "19",   0xb0000, 0x10000, CRC(0e21eae0) SHA1(0cde9cdc6dbe2015e7f38b391c78cf3f16658e5c) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "15",   0x00000, 0x10000, CRC(3c3f16f6) SHA1(2fccbf1dd072c59b5923631fc1c6d40f7ea63996))
	ROM_LOAD( "13",   0x10000, 0x10000, CRC(7c21be72) SHA1(9935c983d0f7613ee192758ddcd8d8592e8bf78a) )
	ROM_LOAD( "14",   0x20000, 0x10000, CRC(e92f91f4) SHA1(4351b2b117c1104dcdb6f48531ddad385691c945) )
	ROM_LOAD( "12",   0x30000, 0x10000, CRC(6896e2f7) SHA1(d230d2406ae451f59d1d0783b1d670a0d3e28d8c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7",   0x00000, 0x10000, CRC(6d9e3f0f) SHA1(5c3e7fb2e46939dd3c540b9e1af9591dbfd15b19) )
	ROM_LOAD( "9",   0x10000, 0x10000, CRC(0c15dec9) SHA1(b0a6bb13216f4321b5fc01a649ea84d2d1d51088) )
	ROM_LOAD( "8",   0x20000, 0x10000, CRC(151b22b4) SHA1(3b0470df9b719dd76115d8c549010ec92e28d0d0) )
	ROM_LOAD( "10",  0x30000, 0x10000, CRC(ae2fc028) SHA1(94fea9088b7b412706b6aaf7aac856709649fb63) )

	ROM_REGION( 0x0300, "proms", 0 ) // wasn't in this set, is it still present?
	ROM_LOAD( "21j-k-0",      0x0000, 0x0100, BAD_DUMP CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // Layer priority (same as ddragon)
	ROM_LOAD( "prom.16",      0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing (same as ddragon)
ROM_END

ROM_START( tstrike )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "prog.rom",        0x08000, 0x08000, CRC(bf011a00) SHA1(09a55042a219dd37cb9e7feeab092ebfb903ddde) )
	ROM_LOAD( "tstrike.25",      0x10000, 0x08000, CRC(b6a0c2f3) SHA1(3434689ca217f5af268058ad34c277db672d389c) ) // banked at 0x4000-0x8000
	ROM_LOAD( "tstrike.24",      0x18000, 0x08000, CRC(363816fa) SHA1(65c1ccbb950e09230196b49dc7312a13a34f3f79) ) // banked at 0x4000-0x8000
	// IC23 is replaced with a daughterboard containing a 68705 MCU

	ROM_REGION( 0x10000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "63701.bin",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tstrike.30",      0x08000, 0x08000, CRC(3f3f04a1) SHA1(45d2b4542ec783c1c4122616606be6c160f76c06) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "68705prt.mcu",   0x00000, 0x0800, CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "alpha.rom",        0x00000, 0x08000, CRC(3a7c3185) SHA1(1ccaa6a1f46d66feda49fdea337b8eb32f14c7b5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "tstrike.117",  0x00000, 0x10000, CRC(f7122c0d) SHA1(2b6b359585d9df966c1fc0041fb972aac9b1ab93) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) ) // from ddragon (116)
	ROM_LOAD( "tstrike.115",  0x20000, 0x10000, CRC(a13c7b62) SHA1(d929d8db7eb2b949cd3bd77238611ecc54b2e885) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) ) // from ddragon (114)
	ROM_LOAD( "tstrike.113",  0x40000, 0x10000, CRC(5ad60938) SHA1(a0af9b227157d87fa6d4ea88b34227a97baff20e) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) ) // from ddragon (112)
	ROM_LOAD( "tstrike.111",  0x60000, 0x10000, CRC(7b9c87ad) SHA1(429049f84b2084bb074e380dca63b75150e7e69f) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) ) // from ddragon (110)

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "tstrike.78",   0x00000, 0x10000, CRC(88284aec) SHA1(f07bc5f84f2b2f976c911541c8f1ff2558f569ca) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) ) // from ddragon (77)
	ROM_LOAD( "tstrike.109",  0x20000, 0x10000, CRC(8c2cd0bb) SHA1(364a708484c7750f38162d463104216bbd555b86) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) ) // from ddragon (108)

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "tstrike.94",        0x00000, 0x10000, CRC(8a2c09fc) SHA1(f59a43c3fa814b169a51744f9604d36ae63c190f) ) // first+second half identical

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "tstrike.95",        0x00000, 0x08000, CRC(1812eecb) SHA1(9b7d526f30a86682cdf088600b25ea5a56b112ef) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

ROM_START( tstrikea )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "tstrike.26",      0x08000, 0x08000, CRC(871b10bc) SHA1(c824775cf72c039612fda76c4a518cd89e4c8657) )
	ROM_LOAD( "tstrike.25",      0x10000, 0x08000, CRC(b6a0c2f3) SHA1(3434689ca217f5af268058ad34c277db672d389c) ) // banked at 0x4000-0x8000
	ROM_LOAD( "tstrike.24",      0x18000, 0x08000, CRC(363816fa) SHA1(65c1ccbb950e09230196b49dc7312a13a34f3f79) ) // banked at 0x4000-0x8000
	// IC23 is replaced with a daughterboard containing a 68705 MCU

	ROM_REGION( 0x10000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "63701.bin",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tstrike.30",      0x08000, 0x08000, CRC(3f3f04a1) SHA1(45d2b4542ec783c1c4122616606be6c160f76c06) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "68705prt.mcu",   0x00000, 0x0800, CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "tstrike.20",        0x00000, 0x08000, CRC(b6b8bfa0) SHA1(ce50f8eb1a84873ef3df621d971a6b087473d6c2) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "tstrike.117",  0x00000, 0x10000, CRC(f7122c0d) SHA1(2b6b359585d9df966c1fc0041fb972aac9b1ab93) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) ) // from ddragon (116)
	ROM_LOAD( "tstrike.115",  0x20000, 0x10000, CRC(a13c7b62) SHA1(d929d8db7eb2b949cd3bd77238611ecc54b2e885) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) ) // from ddragon (114)
	ROM_LOAD( "tstrike.113",  0x40000, 0x10000, CRC(5ad60938) SHA1(a0af9b227157d87fa6d4ea88b34227a97baff20e) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) ) // from ddragon (112)
	ROM_LOAD( "tstrike.111",  0x60000, 0x10000, CRC(7b9c87ad) SHA1(429049f84b2084bb074e380dca63b75150e7e69f) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) ) // from ddragon (110)

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "tstrike.78",   0x00000, 0x10000, CRC(88284aec) SHA1(f07bc5f84f2b2f976c911541c8f1ff2558f569ca) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) ) // from ddragon (77)
	ROM_LOAD( "tstrike.109",  0x20000, 0x10000, CRC(8c2cd0bb) SHA1(364a708484c7750f38162d463104216bbd555b86) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) ) // from ddragon (108)

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "tstrike.94",        0x00000, 0x10000, CRC(8a2c09fc) SHA1(f59a43c3fa814b169a51744f9604d36ae63c190f) ) // first+second half identical

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "tstrike.95",        0x00000, 0x08000, CRC(1812eecb) SHA1(9b7d526f30a86682cdf088600b25ea5a56b112ef) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END


ROM_START( ddungeon )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "dd25.25",    0x10000, 0x8000,  CRC(922e719c) SHA1(d1c73f56913cd368158abc613d7bbab669509742) )
	ROM_LOAD( "dd26.26",    0x08000, 0x8000,  CRC(a6e7f608) SHA1(83b9301c39bfdc1e50a37f2bdc4d4f65a1111bee) )
	// IC23 is replaced with a daughterboard containing a 68705 MCU

	ROM_REGION( 0x10000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "63701.bin",  0xc000,  0x4000,  CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "dd30.30",    0x08000, 0x08000, CRC(ef1af99a) SHA1(7ced695b81ca9efbb7b28b78013e112edac85672) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "dd_mcu.bin", 0x00000, 0x0800,  CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "dd20.20",    0x00000, 0x08000, CRC(d976b78d) SHA1(e1cd47032a0f91d812c3925d1f1267a9972bf48e) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "dd117.117",  0x00000, 0x08000, CRC(e912ca81) SHA1(8c274400170f46f84042f4f9cffba8d2fe9fbc10) )
	ROM_LOAD( "dd113.113",  0x10000, 0x08000, CRC(43264ad8) SHA1(74f031d6179390bc4fa99f4929a6886db8c2b510) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "dd78.78",    0x00000, 0x08000, CRC(3deacae9) SHA1(6663f054ed3eed50c5cacfa5d22d465dfb179964) )
	ROM_LOAD( "dd109.109",  0x10000, 0x08000, CRC(5a2f31eb) SHA1(1b85533443e148adb2a9c2c09c43cbf2c35c86bc) )

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",      0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) ) // at IC95

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",      0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) ) // at IC94

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END

/* the only differences with this set are 2x graphic ROMs, and the sound program.
   this version uses the sound program from double dragon, and as this configuration has been found on at least
   4 boards it's likely that the updated sound ROM in the parent set was only shipped with the 'Game Room'
   version of the game */
ROM_START( ddungeone )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "dd25.25",    0x10000, 0x8000,  CRC(922e719c) SHA1(d1c73f56913cd368158abc613d7bbab669509742) ) // 3 on this board
	ROM_LOAD( "dd26.26",    0x08000, 0x8000,  CRC(a6e7f608) SHA1(83b9301c39bfdc1e50a37f2bdc4d4f65a1111bee) ) // 2 on this board
	// IC23 is replaced with a daughterboard containing a 68705 MCU

	ROM_REGION( 0x10000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "63701.bin",  0xc000,  0x4000,  CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) ) // from ddragon

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "dd_mcu.bin", 0x00000, 0x0800,  CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "dd6.bin",    0x00000, 0x08000, CRC(057588ca) SHA1(d4a5dd3ea8cf455b54657473d4d52ab5e838ae15) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "dd-7r.bin",  0x00000, 0x08000, CRC(50d6ab5d) SHA1(4c9cbd72d38b631ea2ca231045ef3f3e11cc7c07) )
	ROM_LOAD( "dd113.113",  0x10000, 0x08000, CRC(43264ad8) SHA1(74f031d6179390bc4fa99f4929a6886db8c2b510) ) // 7K on this board

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "dd78.78",    0x00000, 0x08000, CRC(3deacae9) SHA1(6663f054ed3eed50c5cacfa5d22d465dfb179964) ) // 6B on this board
	ROM_LOAD( "dd109.109",  0x10000, 0x08000, CRC(5a2f31eb) SHA1(1b85533443e148adb2a9c2c09c43cbf2c35c86bc) ) // 7C on this board

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",      0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",      0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END


ROM_START( darktowr )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "dt.26",         0x08000, 0x08000, CRC(8134a472) SHA1(7d42d2ed8d09855241d98ed94bce140a314c2f66) )
	ROM_LOAD( "21j-2-3.25",    0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) // from ddragon
	ROM_LOAD( "dt.24",         0x18000, 0x08000, CRC(523a5413) SHA1(71c04287e4f2e792c98abdeb97fe70abd0d5e918) ) // banked at 0x4000-0x8000
	// IC23 is replaced with a daughterboard containing a 68705 MCU

	ROM_REGION( 0x10000, "sub", 0 ) // sprite CPU
	ROM_LOAD( "63701.bin",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) ) // from ddragon

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "68705prt.mcu",   0x00000, 0x0800, CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "dt.20",        0x00000, 0x08000, CRC(860b0298) SHA1(087e4e6511c5bed74ffbfd077ece55a756b13253) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dt.117",       0x00000, 0x10000, CRC(750dd0fa) SHA1(d95b95a54c7ed87a27edb8660810dd89efa10c9f) )
	ROM_LOAD( "dt.116",       0x10000, 0x10000, CRC(22cfa87b) SHA1(0008a41f307be96be91f491bdeaa1fa450dd0fdf) )
	ROM_LOAD( "dt.115",       0x20000, 0x10000, CRC(8a9f1c34) SHA1(1f07f424b2ab14a051f2c84b3d89fc5d35c5f20b) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) ) // from ddragon
	ROM_LOAD( "dt.113",       0x40000, 0x10000, CRC(7b4bbf9c) SHA1(d0caa3c38e059d3ee48e3e801da36f67457ed542) )
	ROM_LOAD( "dt.112",       0x50000, 0x10000, CRC(df3709d4) SHA1(9cca44be97260e730786db8244a0d655c86537aa) )
	ROM_LOAD( "dt.111",       0x60000, 0x10000, CRC(59032154) SHA1(637372e4619472a958f4971b50a6fe0985bffc8b) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) ) // from ddragon

	ROM_REGION( 0x40000, "tiles", 0 ) /* tiles */
	ROM_LOAD( "dt.78",        0x00000, 0x10000, CRC(72c15604) SHA1(202b46a2445eea5877e986a871bb0a6b76b88a6f) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) ) // from ddragon
	ROM_LOAD( "dt.109",       0x20000, 0x10000, CRC(15bdcb62) SHA1(75382a3805dc333b196e119d28b5c3f320bd9f2a) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) ) // from ddragon

	ROM_REGION( 0x10000, "adpcm1", 0 )
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) ) // from ddragon

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "21j-7",        0x00000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) ) // from ddragon

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    // layer priorities
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    // sprite timing
ROM_END


ROM_START( toffy )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "2-27512.rom", 0x00000, 0x10000, CRC(244709dd) SHA1(b2db51b910f1a031b94fb50e684351f657a465dc) )
	ROM_RELOAD( 0x10000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "u142.1", 0x00000, 0x10000, CRC(541bd7f0) SHA1(3f0097f5877eae50651f94d46d7dd9127037eb6e) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "7-27512.rom", 0x000, 0x10000, CRC(f9e8ec64) SHA1(36891cd8f28800e03fe0eac84b2484a70011eabb) )

	ROM_REGION( 0x20000, "tiles", 0 )
	// the same as 'Dangerous Dungeons' once decrypted
	ROM_LOAD( "4-27512.rom", 0x00000, 0x10000, CRC(94b5ef6f) SHA1(32967f6cfc6a077c31923318891ed508f83e67f6) )
	ROM_LOAD( "3-27512.rom", 0x10000, 0x10000, CRC(a7a053a3) SHA1(98625fe73a409c8d51136931a5f707a0bf75b66a) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "6-27512.rom", 0x00000, 0x10000, CRC(2ba7ca47) SHA1(ad709fc871f1f1a7d4b0fdf0f516c53fd4c8b685) )
	ROM_LOAD( "5-27512.rom", 0x10000, 0x10000, CRC(4f91eec6) SHA1(18a5f98dfba33837b73d032a6153eeb03263684b) )
ROM_END

ROM_START( toffya ) // original Midas board with original ROM stickers
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "mde2.u70", 0x00000, 0x10000, CRC(a27f1b49) SHA1(26aa1bc5af09f22207d764a598d99fa9218608e9) )
	ROM_RELOAD(           0x10000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mde1.u142", 0x00000, 0x10000, CRC(541bd7f0) SHA1(3f0097f5877eae50651f94d46d7dd9127037eb6e) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "mde7.u35", 0x00000, 0x10000, CRC(6ddc2867) SHA1(9b3c053048efea768651633a76cb636f40289b79) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x20000, "tiles", 0 )
	// the same as 'Dangerous Dungeons' once decrypted
	ROM_LOAD( "mde4.u78", 0x00000, 0x10000, CRC(9506b10d) SHA1(1a205a519fdbb7a3149c2e72c8620e79caa32f0d) ) // 1xxxxxxxxxxxxxxx = 0x00
	ROM_LOAD( "mde3.u77", 0x10000, 0x10000, CRC(77c097cc) SHA1(569b36a2be149d9c8b361a4a73c628c3c7471db4) ) // 1xxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "mde6.u80", 0x00000, 0x10000, CRC(aeef092a) SHA1(520148f663695207567f25fbb0634a650c7986e5) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "mde5.u79", 0x10000, 0x10000, CRC(14b52f76) SHA1(8d94bb1b404c483c0dbd69d40d9095fb97bd2faa) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

ROM_START( stoffy )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "2.u70", 0x00000, 0x10000, CRC(6203aeb5) SHA1(e57aa520e8096df01461b235f77557c267571a57) )
	ROM_RELOAD( 0x10000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "1.u142", 0x00000, 0x10000, CRC(541bd7f0) SHA1(3f0097f5877eae50651f94d46d7dd9127037eb6e) ) // same as 'toffy'

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "7.u35", 0x00000, 0x10000, CRC(1cf13736) SHA1(bff5b99ea20af32f1fc7f28f4f0b397ec987c7ca) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "4.u78", 0x00000, 0x10000, CRC(2066c3c7) SHA1(6778e654c0953a7e4ff18cbd326e9d3f8218a3b2) ) // 0
	ROM_LOAD( "3.u77", 0x10000, 0x10000, CRC(3625f813) SHA1(b44830896c69cd5c618c4740ccf471f31dfa34c1) ) // 0

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "6.u80", 0x00000, 0x10000, CRC(ff190865) SHA1(245e69651d0161fcb416bba8f743602b4ee83139) ) // 1
	ROM_LOAD( "5.u79", 0x10000, 0x10000, CRC(333d5b8a) SHA1(d3573db87e2318c144ee9ace6c975a70fc96f4c4) ) // 1
ROM_END

ROM_START( stoffyu )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "u70.2", 0x00000, 0x10000, CRC(3c156610) SHA1(d7fdbc595bdc77c452da39da8b20774db0952e33) )
	ROM_RELOAD( 0x10000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "1.u142", 0x00000, 0x10000, CRC(541bd7f0) SHA1(3f0097f5877eae50651f94d46d7dd9127037eb6e) ) // same as 'toffy'

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "u35.7", 0x00000, 0x10000, CRC(83735d25) SHA1(d82c046db0112d7d2877339652b2111f12513a4f) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "u78.4", 0x00000, 0x10000, CRC(9743a74d) SHA1(876696c5e88e58e6e44671c33a4c140be02a941e) ) // 0
	ROM_LOAD( "u77.3", 0x10000, 0x10000, CRC(f267109a) SHA1(679d2147c79636796dda850345c04ad8a9daa6af) ) // 0

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "6.u80", 0x00000, 0x10000, CRC(ff190865) SHA1(245e69651d0161fcb416bba8f743602b4ee83139) ) // 1
	ROM_LOAD( "5.u79", 0x10000, 0x10000, CRC(333d5b8a) SHA1(d3573db87e2318c144ee9ace6c975a70fc96f4c4) ) // 1
ROM_END



/*************************************
 *
 *  Driver-specific initialization
 *
 *************************************/

void ddragon_state::init_ddragon()
{
	m_sprite_irq = INPUT_LINE_NMI;
	m_technos_video_hw = 0;
}


void ddragon_state::init_ddragon2()
{
	m_sprite_irq = INPUT_LINE_NMI;
	m_technos_video_hw = 2;
}


void darktowr_state::init_darktowr()
{
	save_item(NAME(m_mcu_port_a_out));

	m_sprite_irq = INPUT_LINE_NMI;
	m_technos_video_hw = 0;
	m_mcu_port_a_out = 0xff;
}


void toffy_state::init_toffy()
{
	m_technos_video_hw = 0;

	// the program ROM has a simple bitswap encryption
	uint8_t *rom = memregion("maincpu")->base();
	int length = memregion("maincpu")->bytes();
	for (int i = 0; i < length; i++)
		rom[i] = bitswap<8>(rom[i], 6,7,5,4,3,2,1,0);

	// and the fg gfx ...
	rom = memregion("chars")->base();
	length = memregion("chars")->bytes();
	for (int i = 0; i < length; i++)
		rom[i] = bitswap<8>(rom[i], 7,6,5,3,4,2,1,0);

	// and the sprites gfx
	rom = memregion("sprites")->base();
	length = memregion("sprites")->bytes();
	for (int i = 0; i < length; i++)
		rom[i] = bitswap<8>(rom[i], 7,6,5,4,3,2,0,1);

	// and the bg gfx
	rom = memregion("tiles")->base();
	length = memregion("tiles")->bytes();
	for (int i = 0; i < length / 2; i++)
	{
		rom[i + 0*length/2] = bitswap<8>(rom[i + 0*length/2], 7,6,1,4,3,2,5,0);
		rom[i + 1*length/2] = bitswap<8>(rom[i + 1*length/2], 7,6,2,4,3,5,1,0);
	}

	// should the sound ROM be bitswapped too?
	// probably not, as the unencrypted set's sound ROM matches
}

void ddragon_state::init_ddragon6809()
{
	uint8_t *src = memregion("enc_chars")->base();
	uint8_t *dst = memregion("chars")->base();

	for (int i = 0; i < 0x8000; i++)
	{
		switch (i & 0x18)
		{
			case 0x00: dst[i] = src[(i & ~0x18) | 0x18]; break;
			case 0x08: dst[i] = src[(i & ~0x18) | 0x00]; break;
			case 0x10: dst[i] = src[(i & ~0x18) | 0x08]; break;
			case 0x18: dst[i] = src[(i & ~0x18) | 0x10]; break;
		}
	}

	m_sprite_irq = INPUT_LINE_NMI;
	m_technos_video_hw = 0;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1987, ddragon,      0,        ddragon,     ddragon,  ddragon_state,  init_ddragon,     ROT0, "Technos Japan (Taito license)",         "Double Dragon (World set 1)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragona,     ddragon,  ddragon,     ddragon,  ddragon_state,  init_ddragon,     ROT0, "Technos Japan (Taito license)",         "Double Dragon (World set 2)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonu,     ddragon,  ddragon,     ddragon,  ddragon_state,  init_ddragon,     ROT0, "Technos Japan (Taito America license)", "Double Dragon (US set 1)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonua,    ddragon,  ddragon,     ddragon,  ddragon_state,  init_ddragon,     ROT0, "Technos Japan (Taito America license)", "Double Dragon (US set 2)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonub,    ddragon,  ddragon,     ddragon,  ddragon_state,  init_ddragon,     ROT0, "Technos Japan (Taito America license)", "Double Dragon (US set 3)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonj,     ddragon,  ddragon,     ddragon,  ddragon_state,  init_ddragon,     ROT0, "Technos Japan",                         "Double Dragon (Japan set 1)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonja,    ddragon,  ddragon,     ddragon,  ddragon_state,  init_ddragon,     ROT0, "Technos Japan",                         "Double Dragon (Japan set 2)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonbl2,   ddragon,  ddragon,     ddragon,  ddragon_state,  init_ddragon,     ROT0, "bootleg",                               "Double Dragon (bootleg)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonbl,    ddragon,  ddragonbl,   ddragon,  ddragon_state,  init_ddragon,     ROT0, "bootleg",                               "Double Dragon (bootleg with HD6309)",         MACHINE_SUPPORTS_SAVE ) // according to dump notes
GAME( 1987, ddragonbla,   ddragon,  ddragonbla,  ddragon,  ddragon_state,  init_ddragon,     ROT0, "bootleg",                               "Double Dragon (bootleg with MC6803)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragon6809,  ddragon,  ddragon6809, ddragon,  ddragon_state,  init_ddragon6809, ROT0, "bootleg",                               "Double Dragon (bootleg with 3xM6809, set 1)", MACHINE_NOT_WORKING )
GAME( 1987, ddragon6809a, ddragon,  ddragon6809, ddragon,  ddragon_state,  init_ddragon6809, ROT0, "bootleg",                               "Double Dragon (bootleg with 3xM6809, set 2)", MACHINE_NOT_WORKING )
GAME( 1987, ddragon6809b, ddragon,  ddragon6809, ddragon,  ddragon_state,  init_ddragon6809, ROT0, "bootleg",                               "Double Dragon (bootleg with 3xM6809, set 3)", MACHINE_NOT_WORKING )

GAME( 1988, ddragon2,     0,        ddragon2,    ddragon2, ddragon_state,  init_ddragon2,    ROT0, "Technos Japan", "Double Dragon II: The Revenge (World)",       MACHINE_SUPPORTS_SAVE )
GAME( 1988, ddragon2u,    ddragon2, ddragon2,    ddragon2, ddragon_state,  init_ddragon2,    ROT0, "Technos Japan", "Double Dragon II: The Revenge (US)",          MACHINE_SUPPORTS_SAVE )
GAME( 1988, ddragon2j,    ddragon2, ddragon2,    ddragon2, ddragon_state,  init_ddragon2,    ROT0, "Technos Japan", "Double Dragon II: The Revenge (Japan)",       MACHINE_SUPPORTS_SAVE )
GAME( 1988, ddragon2bl,   ddragon2, ddragon2,    ddragon2, ddragon_state,  init_ddragon2,    ROT0, "bootleg",       "Double Dragon II: The Revenge (US, bootleg)", MACHINE_SUPPORTS_SAVE )

// these were conversions of Double Dragon
GAME( 1991, tstrike,      0,        darktowr,    tstrike,  darktowr_state, init_darktowr,    ROT0, "East Coast Coin Company", "Thunder Strike (set 1)",        MACHINE_SUPPORTS_SAVE ) // same manufacturer as The Game Room?
GAME( 1991, tstrikea,     tstrike,  darktowr,    tstrike,  darktowr_state, init_darktowr,    ROT0, "The Game Room",           "Thunder Strike (set 2, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, ddungeon,     0,        darktowr,    ddungeon, darktowr_state, init_darktowr,    ROT0, "The Game Room",           "Dangerous Dungeons (set 1)",    MACHINE_SUPPORTS_SAVE )
GAME( 1992, ddungeone,    ddungeon, darktowr,    ddungeon, darktowr_state, init_darktowr,    ROT0, "East Coast Coin Company", "Dangerous Dungeons (set 2)",    MACHINE_SUPPORTS_SAVE )
GAME( 1992, darktowr,     0,        darktowr,    darktowr, darktowr_state, init_darktowr,    ROT0, "The Game Room",           "Dark Tower",                    MACHINE_SUPPORTS_SAVE )

// these run on their own board, but are basically the same game. Toffy even has 'Dangerous Dungeons' text in it
GAME( 1993, toffy,        0,         toffy,      toffy,    toffy_state,    init_toffy,       ROT0, "Midas",                   "Toffy (encrypted)",             MACHINE_SUPPORTS_SAVE )
GAME( 1993, toffya,       toffy,     toffy,      toffy,    toffy_state,    empty_init,       ROT0, "Midas",                   "Toffy (unencrypted)",           MACHINE_SUPPORTS_SAVE )

GAME( 1994, stoffy,       0,         toffy,      toffy,    toffy_state,    init_toffy,       ROT0, "Midas",                   "Super Toffy",                   MACHINE_SUPPORTS_SAVE )
GAME( 1994, stoffyu,      stoffy,    toffy,      toffy,    toffy_state,    init_toffy,       ROT0, "Midas (Unico license)",   "Super Toffy (Unico license)",   MACHINE_SUPPORTS_SAVE )
