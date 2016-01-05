// license:BSD-3-Clause
// copyright-holders:Philip Bennett,Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino, Ernesto Corvi, David Haywood, R. Belmont
/***************************************************************************

Double Dragon     (c) 1987 Technos Japan
Double Dragon II  (c) 1988 Technos Japan

Driver by Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino, Ernesto Corvi
Toffy / Super Toffy added by David Haywood
Thanks to Bryan McPhail for spotting the Toffy program rom encryption
Toffy / Super Toffy sound hooked up by R. Belmont.


Modifications by Phil Bennett Sep 2013:

Cleanups based on Double Dragon schematics.
Fixed sub CPU interrupt handling and common RAM access.
Removed now-unnecessary workarounds.

Modifications by Bryan McPhail, June-November 2003:

Correct video & interrupt timing derived from Xain schematics and confirmed on real DD board.
Corrected interrupt handling, epecially to MCU (but one semi-hack remains).
TStrike now boots but sprites don't appear (I had them working at one point, can't remember what broke them again).
Dangerous Dungeons fixed.
World version of Double Dragon added (actually same roms as the bootleg, but confirmed from real board)
Removed stereo audio flag (still on Toffy - does it have it?)

todo:

banking in Toffy / Super toffy

-- Read Me --

Super Toffy - Unico 1994

Main cpu:   MC6809EP
Sound cpu:  MC6809P
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
#include "cpu/m6809/hd6309.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/msm5205.h"
#include "includes/ddragon.h"


#define MAIN_CLOCK      XTAL_12MHz
#define SOUND_CLOCK     XTAL_3_579545MHz
#define MCU_CLOCK       MAIN_CLOCK / 3
#define PIXEL_CLOCK     MAIN_CLOCK / 2


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

int ddragon_state::scanline_to_vcount( int scanline )
{
	int vcount = scanline + 8;
	if (vcount < 0x100)
		return vcount;
	else
		return (vcount - 0x18) | 0x100;
}

TIMER_DEVICE_CALLBACK_MEMBER(ddragon_state::ddragon_scanline)
{
	int scanline = param;
	int screen_height = m_screen->height();
	int vcount_old = scanline_to_vcount((scanline == 0) ? screen_height - 1 : scanline - 1);
	int vcount = scanline_to_vcount(scanline);

	/* update to the current point */
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	/* on the rising edge of VBLK (vcount == F8), signal an NMI */
	if (vcount == 0xf8)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	/* set 1ms signal on rising edge of vcount & 8 */
	if (!(vcount_old & 8) && (vcount & 8))
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}



/*************************************
 *
 *  System setup and intialization
 *
 *************************************/

MACHINE_START_MEMBER(ddragon_state,ddragon)
{
	/* configure banks */
	membank("bank1")->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);

	/* register for save states */
	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrolly_hi));
	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_end));
	save_item(NAME(m_adpcm_idle));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_ddragon_sub_port));
}


MACHINE_RESET_MEMBER(ddragon_state,ddragon)
{
	m_scrollx_hi = 0;
	m_scrolly_hi = 0;
	m_ddragon_sub_port = 0;
	m_adpcm_pos[0] = m_adpcm_pos[1] = 0;
	m_adpcm_end[0] = m_adpcm_end[1] = 0;
	m_adpcm_idle[0] = m_adpcm_idle[1] = 1;
	m_adpcm_data[0] = m_adpcm_data[1] = -1;
}



/*************************************
 *
 *  Bank switching
 *
 *************************************/

WRITE8_MEMBER(ddragon_state::ddragon_bankswitch_w)
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
	membank("bank1")->set_entry((data & 0xe0) >> 5);
}


WRITE8_MEMBER(ddragon_state::toffy_bankswitch_w)
{
	m_scrollx_hi = data & 0x01;
	m_scrolly_hi = (data & 0x02) >> 1;

//  flip_screen_set(machine(), ~data & 0x04);

	/* I don't know ... */
	membank("bank1")->set_entry((data & 0x20) >> 5);
}


READ8_MEMBER(ddragon_state::darktowr_mcu_bank_r)
{
	// logerror("BankRead %05x %08x\n",space.device().safe_pc(),offset);

	/* Horrible hack - the alternate TStrike set is mismatched against the MCU,
	so just hack around the protection here.  (The hacks are 'right' as I have
	the original source code & notes to this version of TStrike to examine).
	*/
	if (!strcmp(machine().system().name, "tstrike"))
	{
		/* Static protection checks at boot-up */
		if (space.device().safe_pc() == 0x9ace)
			return 0;
		if (space.device().safe_pc() == 0x9ae4)
			return 0x63;

		/* Just return whatever the code is expecting */
		return m_rambase[0xbe1];
	}

	if (offset == 0x1401 || offset == 1)
		return m_darktowr_mcu_ports[0];

	logerror("Unmapped mcu bank read %04x\n",offset);
	return 0xff;
}


WRITE8_MEMBER(ddragon_state::darktowr_mcu_bank_w)
{
	logerror("BankWrite %05x %08x %08x\n", space.device().safe_pc(), offset, data);

	if (offset == 0x1400 || offset == 0)
	{
		m_darktowr_mcu_ports[1] = BITSWAP8(data,0,1,2,3,4,5,6,7);
		logerror("MCU PORT 1 -> %04x (from %04x)\n", BITSWAP8(data,0,1,2,3,4,5,6,7), data);
	}
}


WRITE8_MEMBER(ddragon_state::darktowr_bankswitch_w)
{
	m_scrollx_hi = (data & 0x01);
	m_scrolly_hi = ((data & 0x02) >> 1);

//  flip_screen_set(machine(), ~data & 0x04);

	m_subcpu->set_input_line(INPUT_LINE_RESET, data & 0x08 ? CLEAR_LINE : ASSERT_LINE);
	m_subcpu->set_input_line(INPUT_LINE_HALT, data & 0x10 ? ASSERT_LINE : CLEAR_LINE);

	int oldbank = membank("bank1")->entry();
	int newbank = (data & 0xe0) >> 5;

	membank("bank1")->set_entry(newbank);
	if (newbank == 4 && oldbank != 4)
		space.install_readwrite_handler(0x4000, 0x7fff, read8_delegate(FUNC(ddragon_state::darktowr_mcu_bank_r),this), write8_delegate(FUNC(ddragon_state::darktowr_mcu_bank_w),this));
	else if (newbank != 4 && oldbank == 4)
		space.install_readwrite_bank(0x4000, 0x7fff, "bank1");
}



/*************************************
 *
 *  Interrupt control
 *
 *************************************/

void ddragon_state::ddragon_interrupt_ack(address_space &space, offs_t offset, UINT8 data)
{
	switch (offset)
	{
		case 0: /* 380b - NMI ack */
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			break;

		case 1: /* 380c - FIRQ ack */
			m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
			break;

		case 2: /* 380d - IRQ ack */
			m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
			break;

		case 3: /* 380e - SND IRQ and latch */
			soundlatch_byte_w(space, 0, data);
			m_soundcpu->set_input_line(m_sound_irq, ASSERT_LINE);
			break;

		case 4: /* 380f - MCU IRQ */
			if (m_subcpu)
				m_subcpu->set_input_line(m_sprite_irq, ASSERT_LINE);
			break;
	}
}


READ8_MEMBER(ddragon_state::ddragon_interrupt_r)
{
	ddragon_interrupt_ack(space, offset, 0xff);
	return 0xff;
}


WRITE8_MEMBER(ddragon_state::ddragon_interrupt_w)
{
	ddragon_interrupt_ack(space, offset, data);
}


WRITE8_MEMBER(ddragon_state::ddragon2_sub_irq_ack_w)
{
	m_subcpu->set_input_line(m_sprite_irq, CLEAR_LINE);
}


WRITE8_MEMBER(ddragon_state::ddragon2_sub_irq_w)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}


WRITE_LINE_MEMBER(ddragon_state::irq_handler)
{
	m_soundcpu->set_input_line(m_ym_irq, state ? ASSERT_LINE : CLEAR_LINE);
}


READ8_MEMBER(ddragon_state::soundlatch_ack_r)
{
	m_soundcpu->set_input_line(m_sound_irq, CLEAR_LINE);
	return soundlatch_byte_r(space, 0);
}


WRITE8_MEMBER(ddragon_state::ddragonba_port_w)
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

CUSTOM_INPUT_MEMBER(ddragon_state::subcpu_bus_free)
{
	// Corresponds to BA (Bus Available) on the HD63701
	if (m_subcpu)
		return m_subcpu->suspended(SUSPEND_REASON_RESET | SUSPEND_REASON_HALT);
	else
		return 0;
}


WRITE8_MEMBER(ddragon_state::darktowr_mcu_w)
{
	logerror("McuWrite %05x %08x %08x\n",space.device().safe_pc(), offset, data);
	m_darktowr_mcu_ports[offset] = data;
}


READ8_MEMBER(ddragon_state::ddragon_hd63701_internal_registers_r)
{
	logerror("%04x: read %d\n", space.device().safe_pc(), offset);
	return 0;
}


WRITE8_MEMBER(ddragon_state::ddragon_hd63701_internal_registers_w)
{
	// Port 6
	if (offset == 0x17)
	{
		if ((data & 0x1) == 0)
			m_subcpu->set_input_line(m_sprite_irq, CLEAR_LINE);

		if (!(m_ddragon_sub_port & 0x2) && (data & 0x2))
			m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);

		m_ddragon_sub_port = data;
	}
}



/*************************************
 *
 *  Sprite RAM hacks
 *
 *************************************/

READ8_MEMBER(ddragon_state::ddragon_comram_r)
{
	// Access to shared RAM is prevented when the sub CPU is active
	if (!m_subcpu->suspended(SUSPEND_REASON_RESET | SUSPEND_REASON_HALT))
		return 0xff;

	return m_comram[offset];
}


WRITE8_MEMBER(ddragon_state::ddragon_comram_w)
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

WRITE8_MEMBER(ddragon_state::dd_adpcm_w)
{
	int chip = offset & 1;
	msm5205_device *adpcm = chip ? m_adpcm2 : m_adpcm1;

	switch (offset / 2)
	{
		case 3:
			m_adpcm_idle[chip] = 1;
			adpcm->reset_w(1);
			break;

		case 2:
			m_adpcm_pos[chip] = (data & 0x7f) * 0x200;
			break;

		case 1:
			m_adpcm_end[chip] = (data & 0x7f) * 0x200;
			break;

		case 0:
			m_adpcm_idle[chip] = 0;
			adpcm->reset_w(0);
			break;
	}
}

void ddragon_state::dd_adpcm_int( msm5205_device *device, int chip )
{
	if (m_adpcm_pos[chip] >= m_adpcm_end[chip] || m_adpcm_pos[chip] >= 0x10000)
	{
		m_adpcm_idle[chip] = 1;
		device->reset_w(1);
	}
	else if (m_adpcm_data[chip] != -1)
	{
		device->data_w(m_adpcm_data[chip] & 0x0f);
		m_adpcm_data[chip] = -1;
	}
	else
	{
		UINT8 *ROM = memregion("adpcm")->base() + 0x10000 * chip;

		m_adpcm_data[chip] = ROM[m_adpcm_pos[chip]++];
		device->data_w(m_adpcm_data[chip] >> 4);
	}
}

WRITE_LINE_MEMBER(ddragon_state::dd_adpcm_int_1)
{
	dd_adpcm_int(m_adpcm1, 0);
}

WRITE_LINE_MEMBER(ddragon_state::dd_adpcm_int_2)
{
	dd_adpcm_int(m_adpcm2, 1);
}


READ8_MEMBER(ddragon_state::dd_adpcm_status_r)
{
	return m_adpcm_idle[0] + (m_adpcm_idle[1] << 1);
}



/*************************************
 *
 *  Main CPU memory maps
 *
 *************************************/

static ADDRESS_MAP_START( ddragon_map, AS_PROGRAM, 8, ddragon_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x1000, 0x11ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x1200, 0x13ff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE(ddragon_fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x2000, 0x21ff) AM_READWRITE(ddragon_comram_r, ddragon_comram_w) AM_SHARE("comram") AM_MIRROR(0x0600)
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x3000, 0x37ff) AM_RAM_WRITE(ddragon_bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x3800, 0x3800) AM_READ_PORT("P1")
	AM_RANGE(0x3801, 0x3801) AM_READ_PORT("P2")
	AM_RANGE(0x3802, 0x3802) AM_READ_PORT("EXTRA")
	AM_RANGE(0x3803, 0x3803) AM_READ_PORT("DSW0")
	AM_RANGE(0x3804, 0x3804) AM_READ_PORT("DSW1")
	AM_RANGE(0x3808, 0x3808) AM_WRITE(ddragon_bankswitch_w)
	AM_RANGE(0x3809, 0x3809) AM_WRITEONLY AM_SHARE("scrollx_lo")
	AM_RANGE(0x380a, 0x380a) AM_WRITEONLY AM_SHARE("scrolly_lo")
	AM_RANGE(0x380b, 0x380f) AM_READWRITE(ddragon_interrupt_r, ddragon_interrupt_w)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( dd2_map, AS_PROGRAM, 8, ddragon_state )
	AM_RANGE(0x0000, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE(ddragon_fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x2000, 0x21ff) AM_READWRITE(ddragon_comram_r, ddragon_comram_w) AM_SHARE("comram") AM_MIRROR(0x0600)
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x3000, 0x37ff) AM_RAM_WRITE(ddragon_bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x3800, 0x3800) AM_READ_PORT("P1")
	AM_RANGE(0x3801, 0x3801) AM_READ_PORT("P2")
	AM_RANGE(0x3802, 0x3802) AM_READ_PORT("EXTRA")
	AM_RANGE(0x3803, 0x3803) AM_READ_PORT("DSW0")
	AM_RANGE(0x3804, 0x3804) AM_READ_PORT("DSW1")
	AM_RANGE(0x3808, 0x3808) AM_WRITE(ddragon_bankswitch_w)
	AM_RANGE(0x3809, 0x3809) AM_WRITEONLY AM_SHARE("scrollx_lo")
	AM_RANGE(0x380a, 0x380a) AM_WRITEONLY AM_SHARE("scrolly_lo")
	AM_RANGE(0x380b, 0x380f) AM_READWRITE(ddragon_interrupt_r, ddragon_interrupt_w)
	AM_RANGE(0x3c00, 0x3dff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x3e00, 0x3fff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sub CPU memory maps
 *
 *************************************/

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8, ddragon_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(ddragon_hd63701_internal_registers_r, ddragon_hd63701_internal_registers_w)
	AM_RANGE(0x001f, 0x0fff) AM_RAM
	AM_RANGE(0x8000, 0x81ff) AM_RAM AM_SHARE("comram")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( ddragonba_sub_map, AS_PROGRAM, 8, ddragon_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x8000, 0x81ff) AM_RAM AM_SHARE("comram")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( dd2_sub_map, AS_PROGRAM, 8, ddragon_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_SHARE("comram")
	AM_RANGE(0xd000, 0xd000) AM_WRITE(ddragon2_sub_irq_ack_w)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(ddragon2_sub_irq_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ddragonba_sub_portmap, AS_IO, 8, ddragon_state )
	AM_RANGE(0x0000, 0xffff) AM_WRITE(ddragonba_port_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory maps
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, ddragon_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_READ(soundlatch_ack_r)
	AM_RANGE(0x1800, 0x1800) AM_READ(dd_adpcm_status_r)
	AM_RANGE(0x2800, 0x2801) AM_DEVREADWRITE("fmsnd", ym2151_device, read, write)
	AM_RANGE(0x3800, 0x3807) AM_WRITE(dd_adpcm_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( dd2_sound_map, AS_PROGRAM, 8, ddragon_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("fmsnd", ym2151_device, read, write)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_ack_r)
ADDRESS_MAP_END



/*************************************
 *
 *  MCU memory maps
 *
 *************************************/

static ADDRESS_MAP_START( mcu_map, AS_PROGRAM, 8, ddragon_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0007) AM_RAM_WRITE(darktowr_mcu_w) AM_SHARE("darktowr_mcu")
	AM_RANGE(0x0008, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END



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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ddragon_state, subcpu_bus_free, NULL)
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
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPSETTING(    0x10, "65" )
	PORT_DIPSETTING(    0x30, "70" )
	PORT_DIPSETTING(    0x20, "80" )
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
	PORT_INCLUDE(ddragon)

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
	PORT_DIPSETTING(    0x0f, "4 Coin/6 Credits" )
	PORT_DIPSETTING(    0x0a, "3 Coin/5 Credits" )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0e, "3 Coin/6 Credits" )
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
	PORT_DIPSETTING(    0xf0, "4 Coin/6 Credits" )
	PORT_DIPSETTING(    0xa0, "3 Coin/5 Credits" )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xe0, "3 Coin/6 Credits" )
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
	{ 0, 2, 4, 6 },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
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
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	64*8
};


static GFXDECODE_START( ddragon )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout,   0, 8 )   /* colors   0-127 */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 128, 8 )   /* colors 128-255 */
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout, 256, 8 )   /* colors 256-383 */
GFXDECODE_END

/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( ddragon, ddragon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, MAIN_CLOCK)     /* 12 MHz / 4 internally */
	MCFG_CPU_PROGRAM_MAP(ddragon_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", ddragon_state, ddragon_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", HD63701, MAIN_CLOCK / 2)    /* 6 MHz / 4 internally */
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_CPU_ADD("soundcpu", M6809, MAIN_CLOCK / 8) /* 1.5 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000)) /* heavy interleaving to sync up sprite<->main CPUs */

	MCFG_MACHINE_START_OVERRIDE(ddragon_state,ddragon)
	MCFG_MACHINE_RESET_OVERRIDE(ddragon_state,ddragon)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ddragon)
	MCFG_PALETTE_ADD("palette", 384)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(ddragon_state, screen_update_ddragon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(ddragon_state,ddragon)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("fmsnd", SOUND_CLOCK)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(ddragon_state, irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.60)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)

	MCFG_SOUND_ADD("adpcm1", MSM5205, MAIN_CLOCK / 32)
	MCFG_MSM5205_VCLK_CB(WRITELINE(ddragon_state, dd_adpcm_int_1))   /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)  /* 8kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("adpcm2", MSM5205, MAIN_CLOCK / 32)
	MCFG_MSM5205_VCLK_CB(WRITELINE(ddragon_state, dd_adpcm_int_2))   /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)  /* 8kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ddragonb, ddragon )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("sub", M6809, MAIN_CLOCK / 8)  /* 1.5MHz */
	MCFG_CPU_PROGRAM_MAP(sub_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ddragonba, ddragon )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("sub", M6803, MAIN_CLOCK / 2)  /* 6MHz / 4 internally */
	MCFG_CPU_PROGRAM_MAP(ddragonba_sub_map)
	MCFG_CPU_IO_MAP(ddragonba_sub_portmap)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ddragon6809, ddragon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MAIN_CLOCK / 8)  /* 1.5 MHz */
	MCFG_CPU_PROGRAM_MAP(ddragon_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", ddragon_state, ddragon_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M6809, MAIN_CLOCK / 8)  /* 1.5 Mhz */
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_CPU_ADD("soundcpu", M6809, MAIN_CLOCK / 8) /* 1.5 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000)) /* heavy interleaving to sync up sprite<->main CPUs */

	MCFG_MACHINE_START_OVERRIDE(ddragon_state,ddragon)
	MCFG_MACHINE_RESET_OVERRIDE(ddragon_state,ddragon)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ddragon)
	MCFG_PALETTE_ADD("palette", 384)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(ddragon_state, screen_update_ddragon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(ddragon_state,ddragon)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("fmsnd", SOUND_CLOCK)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(ddragon_state,irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.60)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)

	MCFG_SOUND_ADD("adpcm1", MSM5205, MAIN_CLOCK/32)
	MCFG_MSM5205_VCLK_CB(WRITELINE(ddragon_state, dd_adpcm_int_1))   /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)  /* 8kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("adpcm2", MSM5205, MAIN_CLOCK/32)
	MCFG_MSM5205_VCLK_CB(WRITELINE(ddragon_state, dd_adpcm_int_2))   /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)  /* 8kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ddragon2, ddragon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, MAIN_CLOCK)     /* 12 MHz / 4 internally */
	MCFG_CPU_PROGRAM_MAP(dd2_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", ddragon_state, ddragon_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", Z80, MAIN_CLOCK / 3)        /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(dd2_sub_map)

	MCFG_CPU_ADD("soundcpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(dd2_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000)) /* heavy interleaving to sync up sprite<->main CPUs */

	MCFG_MACHINE_START_OVERRIDE(ddragon_state,ddragon)
	MCFG_MACHINE_RESET_OVERRIDE(ddragon_state,ddragon)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ddragon)
	MCFG_PALETTE_ADD("palette", 384)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(ddragon_state, screen_update_ddragon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(ddragon_state,ddragon)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("fmsnd", SOUND_CLOCK)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(ddragon_state,irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.60)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( darktowr, ddragon )

	/* basic machine hardware */
	MCFG_CPU_ADD("mcu", M68705,XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(mcu_map)

	/* video hardware */
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( toffy, ddragon )

	/* basic machine hardware */
	MCFG_DEVICE_REMOVE("sub")

	/* sound hardware */
	MCFG_DEVICE_REMOVE("adpcm1")
	MCFG_DEVICE_REMOVE("adpcm2")
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ddragon )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "21j-1-5.26",   0x08000, 0x08000, CRC(42045dfd) SHA1(0983705ea3bb87c4c239692f400e02f15c243479) )
	ROM_LOAD( "21j-2-3.25",   0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21j-3.24",     0x18000, 0x08000, CRC(3bdea613) SHA1(d9038c80646a6ce3ea61da222873237b0383680e) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21j-4-1.23",   0x20000, 0x08000, CRC(728f87b9) SHA1(d7442be24d41bb9fc021587ef44ae5b830e4503d) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "21jm-0.ic55",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) /* 63701Y0P MCU */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "21j-7",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0",      0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* unknown */
	ROM_LOAD( "21j-l-0",      0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* Layer priority */
ROM_END

ROM_START( ddragonw )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "21j-1.26",     0x08000, 0x08000, CRC(ae714964) SHA1(072522b97ca4edd099c6b48d7634354dc7088c53) )
	ROM_LOAD( "21j-2-3.25",   0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-3.24",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21j-4.23",     0x20000, 0x08000, CRC(6c9f46fa) SHA1(df251a4aea69b2328f7a543bf085b9c35933e2c1) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "21jm-0.ic55",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) /* 63701Y0P MCU */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "21j-7",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END

ROM_START( ddragonw1 )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "e1-1.26",       0x08000, 0x08000, CRC(4b951643) SHA1(efb1f9ef2e46597d76123c9770854c1d83639eb2) )
	ROM_LOAD( "21a-2-4.25",    0x10000, 0x08000, CRC(5cd67657) SHA1(96bc7a5354a76524bd43a4d7eb8b0053a89e39c4) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-3.24",      0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "e4-1.23",       0x20000, 0x08000, CRC(b1e26935) SHA1(dfff666fd5e9dc4dfb2a1d891eced88730cbaf30) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "21jm-0.ic55",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) /* 63701Y0P MCU */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "21j-7",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END

ROM_START( ddragonu )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "21a-1-5.26",   0x08000, 0x08000, CRC(e24a6e11) SHA1(9dd97dd712d5c896f91fd80df58be9b8a2b198ee) )
	ROM_LOAD( "21j-2-3.25",   0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-3.24",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-4.23",     0x20000, 0x08000, CRC(6ea16072) SHA1(0b3b84a0d54f7a3aba411586009babbfee653f9a) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "21jm-0.ic55",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) /* 63701Y0P MCU */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "21j-7",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END

ROM_START( ddragonua )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "21a-1",     0x08000, 0x08000, CRC(1d625008) SHA1(84cc19a55e7c91fca1943d9624d93e0347ed4150) )
	ROM_LOAD( "21a-2_4",   0x10000, 0x08000, CRC(5cd67657) SHA1(96bc7a5354a76524bd43a4d7eb8b0053a89e39c4) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-3",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-4_2",   0x20000, 0x08000, CRC(9b019598) SHA1(59f3aa15389f53c4646d21a39634cb1502e66ff6) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "21jm-0.ic55",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) /* 63701Y0P MCU */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "21j-7",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END


ROM_START( ddragonub )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "21a-1_6.bin",0x08000,0x08000, CRC(f354b0e1) SHA1(f2fe5d6102564691a0054d2b8dd98673fdc8a348) )
	ROM_LOAD( "21a-2_4",   0x10000, 0x08000, CRC(5cd67657) SHA1(96bc7a5354a76524bd43a4d7eb8b0053a89e39c4) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-3",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-4_2",   0x20000, 0x08000, CRC(9b019598) SHA1(59f3aa15389f53c4646d21a39634cb1502e66ff6) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "21jm-0.ic55",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) ) /* 63701Y0P MCU */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "21j-7",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END


ROM_START( ddragonb ) /* Same program roms as the World set */
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "21j-1.26",     0x08000, 0x08000, CRC(ae714964) SHA1(072522b97ca4edd099c6b48d7634354dc7088c53) )
	ROM_LOAD( "21j-2-3.25",   0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21a-3.24",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "21j-4.23",     0x20000, 0x08000, CRC(6c9f46fa) SHA1(df251a4aea69b2328f7a543bf085b9c35933e2c1) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "ic38",         0x0c000, 0x04000, CRC(6a6a0325) SHA1(98a940a9f23ce9154ff94f7f2ce29efe9a92f71b) ) /* HD6903 instead of HD63701 */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "21j-5",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "21j-7",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END

ROM_START( ddragonba )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "5.bin",     0x08000, 0x08000, CRC(ae714964) SHA1(072522b97ca4edd099c6b48d7634354dc7088c53) )
	ROM_LOAD( "4.bin",     0x10000, 0x08000, CRC(48045762) SHA1(ca39eea71ca76627a98210ce9cc61457a58f16b9) ) /* banked at 0x4000-0x8000 */
	ROM_CONTINUE(0x20000,0x8000) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "3.bin",     0x18000, 0x08000, CRC(dbf24897) SHA1(1504faaf07c541330cd43b72dc6846911dfd85a3) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "2_32.bin",         0x0c000, 0x04000, CRC(67875473) SHA1(66405cb22d41d353335f037ce5aee69e4c6f05c4) ) /* 6803 instead of HD63701 */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "6.bin",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "1.bin",        0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "21j-a",        0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "21j-c",        0x20000, 0x10000, CRC(bb0bc76f) SHA1(37b2225e0593335f636c1e5fded9b21fdeab2f5a) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "21j-e",        0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "21j-g",        0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "21j-8",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "21j-i",        0x20000, 0x10000, CRC(5effb0a0) SHA1(1f21acb15dad824e831ed9a42b3fde096bb31141) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "8.bin",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "7.bin",        0x10000, 0x10000, CRC(f9311f72) SHA1(aa554ef020e04dc896e5495bcddc64e489d0ffff) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END

ROM_START( ddragonb2 )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "b2_4.bin",     0x08000, 0x08000, CRC(668dfa19) SHA1(9b2ff1b66eeba0989e4ed850b7df1f5719ba5572) )
	ROM_LOAD( "b2_5.bin",     0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "b2_6.bin",     0x18000, 0x08000, CRC(3bdea613) SHA1(d9038c80646a6ce3ea61da222873237b0383680e) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "b2_7.bin",     0x20000, 0x08000, CRC(728f87b9) SHA1(d7442be24d41bb9fc021587ef44ae5b830e4503d) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "63701.bin",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "b2_3.bin",     0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b2_8.bin",     0x00000, 0x08000, CRC(7a8b8db4) SHA1(8368182234f9d4d763d4714fd7567a9e31b7ebeb) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "11.bin",       0x00000, 0x10000, CRC(574face3) SHA1(481fe574cb79d0159a65ff7486cbc945d50538c5) )  /* sprites */
	ROM_LOAD( "12.bin",       0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) )
	ROM_LOAD( "13.bin",       0x20000, 0x10000, CRC(c8b91e17) SHA1(0ce6f6ef68ecc7309a2923f7e756d5e2bf5c7a4a) )
	ROM_LOAD( "14.bin",       0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) )
	ROM_LOAD( "15.bin",       0x40000, 0x10000, CRC(a0a0c261) SHA1(25c534d82bd237386d447d72feee8d9541a5ded4) )
	ROM_LOAD( "16.bin",       0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) )
	ROM_LOAD( "17.bin",       0x60000, 0x10000, CRC(3220a0b6) SHA1(24a16ea509e9aff82b9ddd14935d61bb71acff84) )
	ROM_LOAD( "18.bin",       0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "9.bin",        0x00000, 0x10000, CRC(7c435887) SHA1(ecb76f2148fa9773426f05aac208eb3ac02747db) )  /* tiles */
	ROM_LOAD( "10.bin",       0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) )
	ROM_LOAD( "19.bin",       0x20000, 0x10000, CRC(22d65df2) SHA1(2f286a24ea7af438b39126a4ed0c515745981416) )
	ROM_LOAD( "20.bin",       0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "b2_1.bin",     0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "2.bin",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END

/* this is a well known italian bootleg of Double Dragon it can be identified by the following gameplay trait
 -- The Boss of level 4 is coloured like level 1 and 5 instead of green, and is invulnerable to rocks attack.

 in terms of code the game code has been heavily modified, banking writes appear to have been removed, and
 the graphic roms are all scrambled.  The game also runs on 3x M6809 rather than the original CPUs.

 */
ROM_START( ddragon6809 )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "6809_16.bin",   0x08000, 0x08000, CRC(f4c72690) SHA1(c70d032355acf3f7f6586b6e57a94f80e099bf1a) )
	ROM_LOAD( "6809_17.bin",   0x10000, 0x08000, CRC(6489d637) SHA1(fd17fd870e9386a3e3bdd56c8d731c73d8c70b88) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "6809_18.bin",   0x18000, 0x08000, CRC(154d50c4) SHA1(4ffdd29406b6c6b552344f820f83715b1c7727d1) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "6809_19.bin",   0x20000, 0x08000, CRC(090e2baf) SHA1(29b775c59c7a4d30a33e3d10e736cd1a83baf3bb) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "6809_20.bin",   0x8000, 0x8000, CRC(67e3b4f1) SHA1(4945d76b0694299f2f4739ebfba98da6d96fe4cb) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21.bin",      0x08000, 0x08000, CRC(4437fc51) SHA1(fffcf2bec50d0b79861904b4abc607206b7794e6) )

	/* all the gfx roms are scrambled on this set */
	ROM_REGION( 0x08000, "gfx1", ROMREGION_ERASEFF )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "6809_13.bin",   0x00000, 0x08000, CRC(b5a54537) SHA1(a6157cde4f9738565008d11a4a6d8576ae3abfef) ) /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
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

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "6809_9.bin",    0x00000, 0x10000, CRC(736eff0f) SHA1(ae2ec2d5c8ab1db579a08256d874426dc5d889c6) )
	ROM_LOAD( "6809_10.bin",   0x10000, 0x10000, CRC(a670d088) SHA1(27e7b49645753dd039f104c3e0a7e6513a98710d) )
	ROM_LOAD( "6809_11.bin",   0x20000, 0x10000, CRC(4171b70d) SHA1(dc300c9bca6481417e97ad03c973e47389f261c1) )
	ROM_LOAD( "6809_12.bin",   0x30000, 0x10000, CRC(5f6a6d6f) SHA1(7d546a226cda81c28e7ccfb4c5daebc65072198d) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples  */
	ROM_LOAD( "6809_14.bin",   0x00000, 0x08000, CRC(678f8657) SHA1(2652fdc6719d2c889ca87802f6e2cefae59fc2eb) )
	ROM_LOAD( "6809_15.bin",   0x10000, 0x08000, CRC(10f21dea) SHA1(739cf649f91490384297a81a2cc9855acb58a1c0) )
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
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "20.7f",   0x08000, 0x08000, CRC(c804819f) SHA1(cc570a90b7bef1c6263f5e1fd96ed377c508fe2b) )
	ROM_LOAD( "19.7g",   0x10000, 0x08000, CRC(de08db4d) SHA1(e63b90c3bb3af01d2855de9a996b51068bed7b52) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "18.7h",   0x18000, 0x08000, CRC(154d50c4) SHA1(4ffdd29406b6c6b552344f820f83715b1c7727d1) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "17.7j",   0x20000, 0x08000, CRC(4052f37a) SHA1(9444a30ce32a2d35c601324d79c0ba602be4f288) ) /* banked at 0x4000-0x8000 */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "21.7d",   0x08000, 0x8000, CRC(4437fc51) SHA1(fffcf2bec50d0b79861904b4abc607206b7794e6) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "16.7n",   0x08000, 0x08000, CRC(f4c72690) SHA1(c70d032355acf3f7f6586b6e57a94f80e099bf1a) )

	/* all the gfx roms are scrambled on this set */
	ROM_REGION( 0x08000, "gfx1", ROMREGION_ERASEFF )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "13.5f",   0x00000, 0x08000, CRC(b5a54537) SHA1(a6157cde4f9738565008d11a4a6d8576ae3abfef) )   /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "1.1t",         0x00000, 0x10000, CRC(5e810a6d) SHA1(5eba3e982b271bc284ca333429cd0b3759c9c8d1) )
	ROM_LOAD( "1.1r",         0x10000, 0x10000, CRC(7300b785) SHA1(6d3b72bd7208e2bd790517a753c9d5192c88d20f) )
	ROM_LOAD( "3.1q",         0x20000, 0x10000, CRC(19405de8) SHA1(ac1aa40478b92af5ccdde89812be78b7c9f7d20d) )
	ROM_LOAD( "4.1p",         0x30000, 0x10000, CRC(4b10defd) SHA1(fb43eba7c8a7f77f0fdd6253d51b40b0e64598f5) )
	ROM_LOAD( "5.1n",         0x40000, 0x10000, CRC(5b1bb493) SHA1(dd947d7d381af5952acece4b2cefc9fc4847ec68) )
	ROM_LOAD( "6.1m",         0x50000, 0x10000, CRC(e8a2d2e7) SHA1(abc871e57a5280728b9f90625fb91011b848a4d8) )
	ROM_LOAD( "7.1l",         0x60000, 0x10000, CRC(8010fcca) SHA1(9401c41088776beea91c32aaff8eb2fbe92b5e37) )
	ROM_LOAD( "8.1j",         0x70000, 0x10000, CRC(bfa4da27) SHA1(68a649aec43e18dc79b4690c1dff2e2a6fc0065a) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "9.2e",         0x00000, 0x10000, CRC(736eff0f) SHA1(ae2ec2d5c8ab1db579a08256d874426dc5d889c6) )
	ROM_LOAD( "10.2d",        0x10000, 0x10000, CRC(a670d088) SHA1(27e7b49645753dd039f104c3e0a7e6513a98710d) )
	ROM_LOAD( "11.2b",        0x20000, 0x10000, CRC(4171b70d) SHA1(dc300c9bca6481417e97ad03c973e47389f261c1) )
	ROM_LOAD( "12.2a",        0x30000, 0x10000, CRC(5f6a6d6f) SHA1(7d546a226cda81c28e7ccfb4c5daebc65072198d) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples (yes these really are smaller than the original game..)  */
	ROM_LOAD( "14.7q",        0x00000, 0x08000, CRC(678f8657) SHA1(2652fdc6719d2c889ca87802f6e2cefae59fc2eb) )
	ROM_LOAD( "15.7o",        0x10000, 0x08000, CRC(10f21dea) SHA1(739cf649f91490384297a81a2cc9855acb58a1c0) )

	ROM_REGION( 0x20000, "user1", 0 ) /* PROMs */
	ROM_LOAD( "27s21.5o",        0x00000, 0x100, CRC(673f68c3) SHA1(9381453e8f868d80b6069264509a339e20e2b6b1) )
	ROM_LOAD( "27s21.5p",        0x00000, 0x100, CRC(2dc270f2) SHA1(9f124ab2c98680bcc249218ee0de09ba49c09a84) )
	ROM_LOAD( "27s29.6g",        0x00000, 0x200, CRC(095fb461) SHA1(7fd213fd8b8bbe30334523ccf06d4606c67b472e) )
	ROM_LOAD( "82s129.4h",       0x00000, 0x100, CRC(7683cadd) SHA1(ff6fecf273c1d8812814cacc72fb71642ec32b6d) )

	ROM_REGION( 0x20000, "user2", 0 ) /* PALs */
	ROM_LOAD( "pal16r4.8g",        0x00000, 0x104, CRC(5b0263fd) SHA1(ddca425f82f5eb06b56f2ab116fb9a9b192e1097) )
	ROM_LOAD( "pal16r6.2f",        0x00000, 0x104, CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e) )
ROM_END


ROM_START( ddragon2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "26a9-04.bin",  0x08000, 0x8000, CRC(f2cfc649) SHA1(d3f1e0bae02472914a940222e4f600170a91736d) )
	ROM_LOAD( "26aa-03.bin",  0x10000, 0x8000, CRC(44dd5d4b) SHA1(427c4e419668b41545928cfc96435c010ecdc88b) )
	ROM_LOAD( "26ab-0.bin",   0x18000, 0x8000, CRC(49ddddcd) SHA1(91dc53718d04718b313f23d86e241027c89d1a03) )
	ROM_LOAD( "26ac-0e.63",   0x20000, 0x8000, CRC(57acad2c) SHA1(938e2a78af38ecd7e9e08fb10acc1940f7585f5e) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite CPU 64kb (Upper 16kb = 0) */
	ROM_LOAD( "26ae-0.bin",   0x00000, 0x10000, CRC(ea437867) SHA1(cd910203af0565f981b9bdef51ea6e9c33ee82d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* music CPU, 64kb */
	ROM_LOAD( "26ad-0.bin",   0x00000, 0x8000, CRC(75e36cd6) SHA1(f24805f4f6925b3ac508e66a6fc25c275b05f3b9) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "26a8-0e.19",   0x00000, 0x10000, CRC(4e80cd36) SHA1(dcae0709f27f32effb359f6b943f61b102749f2a) )  /* chars */

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD( "26j0-0.bin",   0x00000, 0x20000, CRC(db309c84) SHA1(ee095e4a3bc86737539784945decb1f63da47b9b) )  /* sprites */
	ROM_LOAD( "26j1-0.bin",   0x20000, 0x20000, CRC(c3081e0c) SHA1(c4a9ae151aae21073a2c79c5ac088c72d4f3d9db) )
	ROM_LOAD( "26af-0.bin",   0x40000, 0x20000, CRC(3a615aad) SHA1(ec90a35224a177d00327de6fd1a299df38abd790) )
	ROM_LOAD( "26j2-0.bin",   0x60000, 0x20000, CRC(589564ae) SHA1(1e6e0ef623545615e8409b6d3ba586a71e2612b6) )
	ROM_LOAD( "26j3-0.bin",   0x80000, 0x20000, CRC(daf040d6) SHA1(ab0fd5482625dbe64f0f0b0baff5dcde05309b81) )
	ROM_LOAD( "26a10-0.bin",  0xa0000, 0x20000, CRC(6d16d889) SHA1(3bc62b3e7f4ddc3200a9cf8469239662da80c854) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "26j4-0.bin",   0x00000, 0x20000, CRC(a8c93e76) SHA1(54d64f052971e7fa0d21c5ce12f87b0fa2b648d6) )  /* tiles */
	ROM_LOAD( "26j5-0.bin",   0x20000, 0x20000, CRC(ee555237) SHA1(f9698f3e57f933a43e508f60667c860dee034d05) )

	ROM_REGION( 0x40000, "oki", 0 ) /* adpcm samples */
	ROM_LOAD( "26j6-0.bin",   0x00000, 0x20000, CRC(a84b2a29) SHA1(9cb529e4939c16a0a42f45dd5547c76c2f86f07b) )
	ROM_LOAD( "26j7-0.bin",   0x20000, 0x20000, CRC(bc6a48d5) SHA1(04c434f8cd42a8f82a263548183569396f9b684d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.16",      0x0000, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing (same as ddragon) */
ROM_END

// came from a dead board, 2 of the program roms were failing
// if you attempt to use ROMs from another set it fails on Game Over due to code which should be different at 0x1800 in ic63
ROM_START( ddragon2j )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "26a9-0_j.ic38", 0x08000, 0x8000, CRC(5e4fcdff) SHA1(78bf79a0b4f248c3355fef40448c76eb028f9163) )
	ROM_LOAD( "26aa-0_j.ic52", 0x10000, 0x8000, CRC(bfb4ee04) SHA1(3692bbdef7d5b7cc3eb76362945b91b4a0f6ad4b) )
	ROM_LOAD( "26ab-0.ic53",   0x18000, 0x8000, NO_DUMP) // proper dump might match other sets
	ROM_LOAD( "26ac-0_j.ic63", 0x20000, 0x8000, NO_DUMP) // should be different

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite CPU 64kb (Upper 16kb = 0) */
	ROM_LOAD( "26ae-0.bin",   0x00000, 0x10000, CRC(ea437867) SHA1(cd910203af0565f981b9bdef51ea6e9c33ee82d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* music CPU, 64kb */
	ROM_LOAD( "26ad-0.ic41",   0x00000, 0x8000, CRC(3788af3b) SHA1(7f8833b01522553c767c470a9c27d24e638f37b9) ) // why is this different, label was the same

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "26a8-0e.19",   0x00000, 0x10000, CRC(4e80cd36) SHA1(dcae0709f27f32effb359f6b943f61b102749f2a) )  /* chars */

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD( "26j0-0.bin",   0x00000, 0x20000, CRC(db309c84) SHA1(ee095e4a3bc86737539784945decb1f63da47b9b) )  /* sprites */
	ROM_LOAD( "26j1-0.bin",   0x20000, 0x20000, CRC(c3081e0c) SHA1(c4a9ae151aae21073a2c79c5ac088c72d4f3d9db) )
	ROM_LOAD( "26af-0.bin",   0x40000, 0x20000, CRC(3a615aad) SHA1(ec90a35224a177d00327de6fd1a299df38abd790) )
	ROM_LOAD( "26j2-0.bin",   0x60000, 0x20000, CRC(589564ae) SHA1(1e6e0ef623545615e8409b6d3ba586a71e2612b6) )
	ROM_LOAD( "26j3-0.bin",   0x80000, 0x20000, CRC(daf040d6) SHA1(ab0fd5482625dbe64f0f0b0baff5dcde05309b81) )
	ROM_LOAD( "26a10-0.bin",  0xa0000, 0x20000, CRC(6d16d889) SHA1(3bc62b3e7f4ddc3200a9cf8469239662da80c854) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "26j4-0.bin",   0x00000, 0x20000, CRC(a8c93e76) SHA1(54d64f052971e7fa0d21c5ce12f87b0fa2b648d6) )  /* tiles */
	ROM_LOAD( "26j5-0.bin",   0x20000, 0x20000, CRC(ee555237) SHA1(f9698f3e57f933a43e508f60667c860dee034d05) )

	ROM_REGION( 0x40000, "oki", 0 ) /* adpcm samples */
	ROM_LOAD( "26j6-0.bin",   0x00000, 0x20000, CRC(a84b2a29) SHA1(9cb529e4939c16a0a42f45dd5547c76c2f86f07b) )
	ROM_LOAD( "26j7-0.bin",   0x20000, 0x20000, CRC(bc6a48d5) SHA1(04c434f8cd42a8f82a263548183569396f9b684d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.16",      0x0000, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing (same as ddragon) */
ROM_END

ROM_START( ddragon2u )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "26a9-04.bin",  0x08000, 0x8000, CRC(f2cfc649) SHA1(d3f1e0bae02472914a940222e4f600170a91736d) )
	ROM_LOAD( "26aa-03.bin",  0x10000, 0x8000, CRC(44dd5d4b) SHA1(427c4e419668b41545928cfc96435c010ecdc88b) )
	ROM_LOAD( "26ab-0.bin",   0x18000, 0x8000, CRC(49ddddcd) SHA1(91dc53718d04718b313f23d86e241027c89d1a03) )
	ROM_LOAD( "26ac-02.bin",  0x20000, 0x8000, CRC(097eaf26) SHA1(60504abd30fec44c45197cdf3832c87d05ef577d) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite CPU 64kb (Upper 16kb = 0) */
	ROM_LOAD( "26ae-0.bin",   0x00000, 0x10000, CRC(ea437867) SHA1(cd910203af0565f981b9bdef51ea6e9c33ee82d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* music CPU, 64kb */
	ROM_LOAD( "26ad-0.bin",   0x00000, 0x8000, CRC(75e36cd6) SHA1(f24805f4f6925b3ac508e66a6fc25c275b05f3b9) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "26a8-0.bin",   0x00000, 0x10000, CRC(3ad1049c) SHA1(11d9544a56f8e6a84beb307a5c8a9ff8afc55c66) )  /* chars */

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD( "26j0-0.bin",   0x00000, 0x20000, CRC(db309c84) SHA1(ee095e4a3bc86737539784945decb1f63da47b9b) )  /* sprites */
	ROM_LOAD( "26j1-0.bin",   0x20000, 0x20000, CRC(c3081e0c) SHA1(c4a9ae151aae21073a2c79c5ac088c72d4f3d9db) )
	ROM_LOAD( "26af-0.bin",   0x40000, 0x20000, CRC(3a615aad) SHA1(ec90a35224a177d00327de6fd1a299df38abd790) )
	ROM_LOAD( "26j2-0.bin",   0x60000, 0x20000, CRC(589564ae) SHA1(1e6e0ef623545615e8409b6d3ba586a71e2612b6) )
	ROM_LOAD( "26j3-0.bin",   0x80000, 0x20000, CRC(daf040d6) SHA1(ab0fd5482625dbe64f0f0b0baff5dcde05309b81) )
	ROM_LOAD( "26a10-0.bin",  0xa0000, 0x20000, CRC(6d16d889) SHA1(3bc62b3e7f4ddc3200a9cf8469239662da80c854) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "26j4-0.bin",   0x00000, 0x20000, CRC(a8c93e76) SHA1(54d64f052971e7fa0d21c5ce12f87b0fa2b648d6) )  /* tiles */
	ROM_LOAD( "26j5-0.bin",   0x20000, 0x20000, CRC(ee555237) SHA1(f9698f3e57f933a43e508f60667c860dee034d05) )

	ROM_REGION( 0x40000, "oki", 0 ) /* adpcm samples */
	ROM_LOAD( "26j6-0.bin",   0x00000, 0x20000, CRC(a84b2a29) SHA1(9cb529e4939c16a0a42f45dd5547c76c2f86f07b) )
	ROM_LOAD( "26j7-0.bin",   0x20000, 0x20000, CRC(bc6a48d5) SHA1(04c434f8cd42a8f82a263548183569396f9b684d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.16",      0x0000, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing (same as ddragon) */
ROM_END

ROM_START( ddragon2b )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "3",   0x08000, 0x8000, CRC(5cc38bad) SHA1(8ebbb998cce48b5baa4a738c2d4c2e481e2637fb) )
	ROM_LOAD( "4",   0x10000, 0x8000, CRC(78750947) SHA1(6b8349c3cd27c37a4329cea213b6ff0167c4edee) )
	ROM_LOAD( "5",   0x18000, 0x8000, CRC(49ddddcd) SHA1(91dc53718d04718b313f23d86e241027c89d1a03) )
	ROM_LOAD( "6",   0x20000, 0x8000, CRC(097eaf26) SHA1(60504abd30fec44c45197cdf3832c87d05ef577d) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite CPU 64kb (Upper 16kb = 0) */
	ROM_LOAD( "2",   0x00000, 0x10000, CRC(ea437867) SHA1(cd910203af0565f981b9bdef51ea6e9c33ee82d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* music CPU, 64kb */
	ROM_LOAD( "11",   0x00000, 0x8000, CRC(75e36cd6) SHA1(f24805f4f6925b3ac508e66a6fc25c275b05f3b9) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "1",   0x00000, 0x10000, CRC(3ad1049c) SHA1(11d9544a56f8e6a84beb307a5c8a9ff8afc55c66) )  /* chars */

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD( "27",   0x00000, 0x10000, CRC(fe42df5d) SHA1(aab801346c2db04263cb61c97c6e086387675586) )  /* sprites */
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

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "15",   0x00000, 0x10000, CRC(3c3f16f6) SHA1(2fccbf1dd072c59b5923631fc1c6d40f7ea63996))
	ROM_LOAD( "13",   0x10000, 0x10000, CRC(7c21be72) SHA1(9935c983d0f7613ee192758ddcd8d8592e8bf78a) )
	ROM_LOAD( "14",   0x20000, 0x10000, CRC(e92f91f4) SHA1(4351b2b117c1104dcdb6f48531ddad385691c945) )
	ROM_LOAD( "12",   0x30000, 0x10000, CRC(6896e2f7) SHA1(d230d2406ae451f59d1d0783b1d670a0d3e28d8c) )

	ROM_REGION( 0x40000, "oki", 0 ) /* adpcm samples */
	ROM_LOAD( "7",   0x00000, 0x10000, CRC(6d9e3f0f) SHA1(5c3e7fb2e46939dd3c540b9e1af9591dbfd15b19) )
	ROM_LOAD( "9",   0x10000, 0x10000, CRC(0c15dec9) SHA1(b0a6bb13216f4321b5fc01a649ea84d2d1d51088) )
	ROM_LOAD( "8",   0x20000, 0x10000, CRC(151b22b4) SHA1(3b0470df9b719dd76115d8c549010ec92e28d0d0) )
	ROM_LOAD( "10",  0x30000, 0x10000, CRC(ae2fc028) SHA1(94fea9088b7b412706b6aaf7aac856709649fb63) )

	ROM_REGION( 0x0200, "proms", 0 ) // wasn't in this set, is it still present?
	ROM_LOAD( "prom.16",      0x0000, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing (same as ddragon) */
ROM_END

ROM_START( tstrike )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "prog.rom",        0x08000, 0x08000, CRC(bf011a00) SHA1(09a55042a219dd37cb9e7feeab092ebfb903ddde) )
	ROM_LOAD( "tstrike.25",      0x10000, 0x08000, CRC(b6a0c2f3) SHA1(3434689ca217f5af268058ad34c277db672d389c) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "tstrike.24",      0x18000, 0x08000, CRC(363816fa) SHA1(65c1ccbb950e09230196b49dc7312a13a34f3f79) ) /* banked at 0x4000-0x8000 */
	/* IC23 is replaced with a daughterboard containing a 68705 MCU */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "63701.bin",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "tstrike.30",      0x08000, 0x08000, CRC(3f3f04a1) SHA1(45d2b4542ec783c1c4122616606be6c160f76c06) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 8k for the microcontroller */
	ROM_LOAD( "68705prt.mcu",   0x00000, 0x0800, CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "alpha.rom",        0x00000, 0x08000, CRC(3a7c3185) SHA1(1ccaa6a1f46d66feda49fdea337b8eb32f14c7b5) )  /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "tstrike.117",  0x00000, 0x10000, CRC(f7122c0d) SHA1(2b6b359585d9df966c1fc0041fb972aac9b1ab93) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) ) /* from ddragon (116) */
	ROM_LOAD( "tstrike.115",  0x20000, 0x10000, CRC(a13c7b62) SHA1(d929d8db7eb2b949cd3bd77238611ecc54b2e885) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) ) /* from ddragon (114) */
	ROM_LOAD( "tstrike.113",  0x40000, 0x10000, CRC(5ad60938) SHA1(a0af9b227157d87fa6d4ea88b34227a97baff20e) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) ) /* from ddragon (112) */
	ROM_LOAD( "tstrike.111",  0x60000, 0x10000, CRC(7b9c87ad) SHA1(429049f84b2084bb074e380dca63b75150e7e69f) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) ) /* from ddragon (110) */

	ROM_REGION( 0x40000, "gfx3", 0 )    /* tiles */
	ROM_LOAD( "tstrike.78",   0x00000, 0x10000, CRC(88284aec) SHA1(f07bc5f84f2b2f976c911541c8f1ff2558f569ca) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) ) /* from ddragon (77) */
	ROM_LOAD( "tstrike.109",  0x20000, 0x10000, CRC(8c2cd0bb) SHA1(364a708484c7750f38162d463104216bbd555b86) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) ) /* from ddragon (108) */

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "tstrike.94",        0x00000, 0x10000, CRC(8a2c09fc) SHA1(f59a43c3fa814b169a51744f9604d36ae63c190f) ) /* first+second half identical */
	ROM_LOAD( "tstrike.95",        0x10000, 0x08000, CRC(1812eecb) SHA1(9b7d526f30a86682cdf088600b25ea5a56b112ef) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END

ROM_START( tstrikea )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "tstrike.26",      0x08000, 0x08000, CRC(871b10bc) SHA1(c824775cf72c039612fda76c4a518cd89e4c8657) )
	ROM_LOAD( "tstrike.25",      0x10000, 0x08000, CRC(b6a0c2f3) SHA1(3434689ca217f5af268058ad34c277db672d389c) ) /* banked at 0x4000-0x8000 */
	ROM_LOAD( "tstrike.24",      0x18000, 0x08000, CRC(363816fa) SHA1(65c1ccbb950e09230196b49dc7312a13a34f3f79) ) /* banked at 0x4000-0x8000 */
	/* IC23 is replaced with a daughterboard containing a 68705 MCU */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "63701.bin",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "tstrike.30",      0x08000, 0x08000, CRC(3f3f04a1) SHA1(45d2b4542ec783c1c4122616606be6c160f76c06) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 8k for the microcontroller */
	ROM_LOAD( "68705prt.mcu",   0x00000, 0x0800, CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tstrike.20",        0x00000, 0x08000, CRC(b6b8bfa0) SHA1(ce50f8eb1a84873ef3df621d971a6b087473d6c2) ) /* chars */

	ROM_REGION( 0x80000, "gfx2", 0 )    /* sprites */
	ROM_LOAD( "tstrike.117",  0x00000, 0x10000, CRC(f7122c0d) SHA1(2b6b359585d9df966c1fc0041fb972aac9b1ab93) )
	ROM_LOAD( "21j-b",        0x10000, 0x10000, CRC(40507a76) SHA1(74581a4b6f48100bddf20f319903af2fe36f39fa) ) /* from ddragon (116) */
	ROM_LOAD( "tstrike.115",  0x20000, 0x10000, CRC(a13c7b62) SHA1(d929d8db7eb2b949cd3bd77238611ecc54b2e885) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) ) /* from ddragon (114) */
	ROM_LOAD( "tstrike.113",  0x40000, 0x10000, CRC(5ad60938) SHA1(a0af9b227157d87fa6d4ea88b34227a97baff20e) )
	ROM_LOAD( "21j-f",        0x50000, 0x10000, CRC(6ba152f6) SHA1(a301ff809be0e1471f4ff8305b30c2fa4aa57fae) ) /* from ddragon (112) */
	ROM_LOAD( "tstrike.111",  0x60000, 0x10000, CRC(7b9c87ad) SHA1(429049f84b2084bb074e380dca63b75150e7e69f) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) ) /* from ddragon (110) */

	ROM_REGION( 0x40000, "gfx3", 0 )    /* tiles */
	ROM_LOAD( "tstrike.78",   0x00000, 0x10000, CRC(88284aec) SHA1(f07bc5f84f2b2f976c911541c8f1ff2558f569ca) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) ) /* from ddragon (77) */
	ROM_LOAD( "tstrike.109",  0x20000, 0x10000, CRC(8c2cd0bb) SHA1(364a708484c7750f38162d463104216bbd555b86) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) ) /* from ddragon (108) */

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "tstrike.94",        0x00000, 0x10000, CRC(8a2c09fc) SHA1(f59a43c3fa814b169a51744f9604d36ae63c190f) ) /* first+second half identical */
	ROM_LOAD( "tstrike.95",        0x10000, 0x08000, CRC(1812eecb) SHA1(9b7d526f30a86682cdf088600b25ea5a56b112ef) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END


ROM_START( ddungeon )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Main CPU? */
	ROM_LOAD( "dd25.25",    0x10000, 0x8000,  CRC(922e719c) SHA1(d1c73f56913cd368158abc613d7bbab669509742) )
	ROM_LOAD( "dd26.26",    0x08000, 0x8000,  CRC(a6e7f608) SHA1(83b9301c39bfdc1e50a37f2bdc4d4f65a1111bee) )
	/* IC23 is replaced with a daughterboard containing a 68705 MCU */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "63701.bin",  0xc000,  0x4000,  CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "dd30.30",    0x08000, 0x08000, CRC(ef1af99a) SHA1(7ced695b81ca9efbb7b28b78013e112edac85672) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 8k for the microcontroller */
	ROM_LOAD( "dd_mcu.bin", 0x00000, 0x0800,  CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* GFX? */
	ROM_LOAD( "dd20.20",    0x00000, 0x08000, CRC(d976b78d) SHA1(e1cd47032a0f91d812c3925d1f1267a9972bf48e) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "dd117.117",  0x00000, 0x08000, CRC(e912ca81) SHA1(8c274400170f46f84042f4f9cffba8d2fe9fbc10) )
	ROM_LOAD( "dd113.113",  0x10000, 0x08000, CRC(43264ad8) SHA1(74f031d6179390bc4fa99f4929a6886db8c2b510) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* GFX */
	ROM_LOAD( "dd78.78",    0x00000, 0x08000, CRC(3deacae9) SHA1(6663f054ed3eed50c5cacfa5d22d465dfb179964) )
	ROM_LOAD( "dd109.109",  0x10000, 0x08000, CRC(5a2f31eb) SHA1(1b85533443e148adb2a9c2c09c43cbf2c35c86bc) )

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",      0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) ) /* at IC95 */
	ROM_LOAD( "21j-7",      0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) ) /* at IC94 */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END

/* the only differences with this set are 2x graphic roms, and the sound program.
   this version uses the sound program from double dragon, and as this configuration has been found on at least
   4 boards it's likely that the updated sound rom in the parent set was only shipped with the 'game room'
   version of the game */
ROM_START( ddungeone )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Main CPU? */
	ROM_LOAD( "dd25.25",    0x10000, 0x8000,  CRC(922e719c) SHA1(d1c73f56913cd368158abc613d7bbab669509742) ) /* 3 on this board */
	ROM_LOAD( "dd26.26",    0x08000, 0x8000,  CRC(a6e7f608) SHA1(83b9301c39bfdc1e50a37f2bdc4d4f65a1111bee) ) /* 2 on this board */
	/* IC23 is replaced with a daughterboard containing a 68705 MCU */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "63701.bin",  0xc000,  0x4000,  CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) ) /* from ddragon */

	ROM_REGION( 0x0800, "mcu", 0 )  /* 8k for the microcontroller */
	ROM_LOAD( "dd_mcu.bin", 0x00000, 0x0800,  CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* GFX? */
	ROM_LOAD( "dd6.bin",    0x00000, 0x08000, CRC(057588ca) SHA1(d4a5dd3ea8cf455b54657473d4d52ab5e838ae15) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "dd-7r.bin",  0x00000, 0x08000, CRC(50d6ab5d) SHA1(4c9cbd72d38b631ea2ca231045ef3f3e11cc7c07) )
	ROM_LOAD( "dd113.113",  0x10000, 0x08000, CRC(43264ad8) SHA1(74f031d6179390bc4fa99f4929a6886db8c2b510) ) /* 7K on this board */

	ROM_REGION( 0x20000, "gfx3", 0 ) /* GFX */
	ROM_LOAD( "dd78.78",    0x00000, 0x08000, CRC(3deacae9) SHA1(6663f054ed3eed50c5cacfa5d22d465dfb179964) ) /* 6B on this board */
	ROM_LOAD( "dd109.109",  0x10000, 0x08000, CRC(5a2f31eb) SHA1(1b85533443e148adb2a9c2c09c43cbf2c35c86bc) ) /* 7C on this board */

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",      0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) )
	ROM_LOAD( "21j-7",      0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END


ROM_START( darktowr )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + bankswitched memory */
	ROM_LOAD( "dt.26",         0x08000, 0x08000, CRC(8134a472) SHA1(7d42d2ed8d09855241d98ed94bce140a314c2f66) )
	ROM_LOAD( "21j-2-3.25",    0x10000, 0x08000, CRC(5779705e) SHA1(4b8f22225d10f5414253ce0383bbebd6f720f3af) ) /* from ddragon */
	ROM_LOAD( "dt.24",         0x18000, 0x08000, CRC(523a5413) SHA1(71c04287e4f2e792c98abdeb97fe70abd0d5e918) ) /* banked at 0x4000-0x8000 */
	/* IC23 is replaced with a daughterboard containing a 68705 MCU */

	ROM_REGION( 0x10000, "sub", 0 ) /* sprite cpu */
	ROM_LOAD( "63701.bin",    0xc000, 0x4000, CRC(f5232d03) SHA1(e2a194e38633592fd6587690b3cb2669d93985c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* audio cpu */
	ROM_LOAD( "21j-0-1",      0x08000, 0x08000, CRC(9efa95bb) SHA1(da997d9cc7b9e7b2c70a4b6d30db693086a6f7d8) ) /* from ddragon */

	ROM_REGION( 0x0800, "mcu", 0 )  /* 8k for the microcontroller */
	ROM_LOAD( "68705prt.mcu",   0x00000, 0x0800, CRC(34cbb2d3) SHA1(8e0c3b13c636012d88753d547c639b1a8af85680) )

	ROM_REGION( 0x08000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dt.20",        0x00000, 0x08000, CRC(860b0298) SHA1(087e4e6511c5bed74ffbfd077ece55a756b13253) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* sprites */
	ROM_LOAD( "dt.117",       0x00000, 0x10000, CRC(750dd0fa) SHA1(d95b95a54c7ed87a27edb8660810dd89efa10c9f) )
	ROM_LOAD( "dt.116",       0x10000, 0x10000, CRC(22cfa87b) SHA1(0008a41f307be96be91f491bdeaa1fa450dd0fdf) )
	ROM_LOAD( "dt.115",       0x20000, 0x10000, CRC(8a9f1c34) SHA1(1f07f424b2ab14a051f2c84b3d89fc5d35c5f20b) )
	ROM_LOAD( "21j-d",        0x30000, 0x10000, CRC(cb4f231b) SHA1(9f2270f9ceedfe51c5e9a9bbb00d6f43dbc4a3ea) ) /* from ddragon */
	ROM_LOAD( "dt.113",       0x40000, 0x10000, CRC(7b4bbf9c) SHA1(d0caa3c38e059d3ee48e3e801da36f67457ed542) )
	ROM_LOAD( "dt.112",       0x50000, 0x10000, CRC(df3709d4) SHA1(9cca44be97260e730786db8244a0d655c86537aa) )
	ROM_LOAD( "dt.111",       0x60000, 0x10000, CRC(59032154) SHA1(637372e4619472a958f4971b50a6fe0985bffc8b) )
	ROM_LOAD( "21j-h",        0x70000, 0x10000, CRC(65c7517d) SHA1(f177ba9c1c7cc75ff04d5591b9865ee364788f94) ) /* from ddragon */

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "dt.78",        0x00000, 0x10000, CRC(72c15604) SHA1(202b46a2445eea5877e986a871bb0a6b76b88a6f) )
	ROM_LOAD( "21j-9",        0x10000, 0x10000, CRC(c6640aed) SHA1(f156c337f48dfe4f7e9caee9a72c7ea3d53e3098) ) /* from ddragon */
	ROM_LOAD( "dt.109",       0x20000, 0x10000, CRC(15bdcb62) SHA1(75382a3805dc333b196e119d28b5c3f320bd9f2a) )
	ROM_LOAD( "21j-j",        0x30000, 0x10000, CRC(5fb42e7c) SHA1(7953316712c56c6f8ca6bba127319e24b618b646) ) /* from ddragon */

	ROM_REGION( 0x20000, "adpcm", 0 ) /* adpcm samples */
	ROM_LOAD( "21j-6",        0x00000, 0x10000, CRC(34755de3) SHA1(57c06d6ce9497901072fa50a92b6ed0d2d4d6528) ) /* from ddragon */
	ROM_LOAD( "21j-7",        0x10000, 0x10000, CRC(904de6f8) SHA1(3623e5ea05fd7c455992b7ed87e605b87c3850aa) ) /* from ddragon */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "21j-k-0.101",  0x0000, 0x0100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) )    /* layer priorities */
	ROM_LOAD( "21j-l-0.16",   0x0100, 0x0200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) )    /* sprite timing */
ROM_END


ROM_START( toffy )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Main CPU */
	ROM_LOAD( "2-27512.rom", 0x00000, 0x10000, CRC(244709dd) SHA1(b2db51b910f1a031b94fb50e684351f657a465dc) )
	ROM_RELOAD( 0x10000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Sound CPU */
	ROM_LOAD( "u142.1", 0x00000, 0x10000, CRC(541bd7f0) SHA1(3f0097f5877eae50651f94d46d7dd9127037eb6e) )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "7-27512.rom", 0x000, 0x10000, CRC(f9e8ec64) SHA1(36891cd8f28800e03fe0eac84b2484a70011eabb) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* GFX */
	/* the same as 'Dangerous Dungeons' once decrypted */
	ROM_LOAD( "4-27512.rom", 0x00000, 0x10000, CRC(94b5ef6f) SHA1(32967f6cfc6a077c31923318891ed508f83e67f6) )
	ROM_LOAD( "3-27512.rom", 0x10000, 0x10000, CRC(a7a053a3) SHA1(98625fe73a409c8d51136931a5f707a0bf75b66a) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "6-27512.rom", 0x00000, 0x10000, CRC(2ba7ca47) SHA1(ad709fc871f1f1a7d4b0fdf0f516c53fd4c8b685) )
	ROM_LOAD( "5-27512.rom", 0x10000, 0x10000, CRC(4f91eec6) SHA1(18a5f98dfba33837b73d032a6153eeb03263684b) )
ROM_END

ROM_START( stoffy )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Main CPU */
	ROM_LOAD( "2.u70", 0x00000, 0x10000, CRC(6203aeb5) SHA1(e57aa520e8096df01461b235f77557c267571a57) )
	ROM_RELOAD( 0x10000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Sound CPU */
	ROM_LOAD( "1.u142", 0x00000, 0x10000, CRC(541bd7f0) SHA1(3f0097f5877eae50651f94d46d7dd9127037eb6e) ) // same as 'toffy'

	ROM_REGION( 0x10000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "7.u35", 0x00000, 0x10000, CRC(1cf13736) SHA1(bff5b99ea20af32f1fc7f28f4f0b397ec987c7ca) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* GFX */
	ROM_LOAD( "4.u78", 0x00000, 0x10000, CRC(2066c3c7) SHA1(6778e654c0953a7e4ff18cbd326e9d3f8218a3b2) ) // 0
	ROM_LOAD( "3.u77", 0x10000, 0x10000, CRC(3625f813) SHA1(b44830896c69cd5c618c4740ccf471f31dfa34c1) ) // 0

	ROM_REGION( 0x20000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "6.u80", 0x00000, 0x10000, CRC(ff190865) SHA1(245e69651d0161fcb416bba8f743602b4ee83139) ) // 1
	ROM_LOAD( "5.u79", 0x10000, 0x10000, CRC(333d5b8a) SHA1(d3573db87e2318c144ee9ace6c975a70fc96f4c4) ) // 1
ROM_END

ROM_START( stoffyu )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Main CPU */
	ROM_LOAD( "u70.2", 0x00000, 0x10000, CRC(3c156610) SHA1(d7fdbc595bdc77c452da39da8b20774db0952e33) )
	ROM_RELOAD( 0x10000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Sound CPU */
	ROM_LOAD( "1.u142", 0x00000, 0x10000, CRC(541bd7f0) SHA1(3f0097f5877eae50651f94d46d7dd9127037eb6e) ) // same as 'toffy'

	ROM_REGION( 0x10000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "u35.7", 0x00000, 0x10000, CRC(83735d25) SHA1(d82c046db0112d7d2877339652b2111f12513a4f) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* GFX */
	ROM_LOAD( "u78.4", 0x00000, 0x10000, CRC(9743a74d) SHA1(876696c5e88e58e6e44671c33a4c140be02a941e) ) // 0
	ROM_LOAD( "u77.3", 0x10000, 0x10000, CRC(f267109a) SHA1(679d2147c79636796dda850345c04ad8a9daa6af) ) // 0

	ROM_REGION( 0x20000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "6.u80", 0x00000, 0x10000, CRC(ff190865) SHA1(245e69651d0161fcb416bba8f743602b4ee83139) ) // 1
	ROM_LOAD( "5.u79", 0x10000, 0x10000, CRC(333d5b8a) SHA1(d3573db87e2318c144ee9ace6c975a70fc96f4c4) ) // 1
ROM_END



/*************************************
 *
 *  Driver-specific initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(ddragon_state,ddragon)
{
	m_sprite_irq = INPUT_LINE_NMI;
	m_sound_irq = M6809_IRQ_LINE;
	m_ym_irq = M6809_FIRQ_LINE;
	m_technos_video_hw = 0;
}


DRIVER_INIT_MEMBER(ddragon_state,ddragon2)
{
	m_sprite_irq = INPUT_LINE_NMI;
	m_sound_irq = INPUT_LINE_NMI;
	m_ym_irq = 0;
	m_technos_video_hw = 2;
}


DRIVER_INIT_MEMBER(ddragon_state,darktowr)
{
	m_sprite_irq = INPUT_LINE_NMI;
	m_sound_irq = M6809_IRQ_LINE;
	m_ym_irq = M6809_FIRQ_LINE;
	m_technos_video_hw = 0;
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x3808, 0x3808, write8_delegate(FUNC(ddragon_state::darktowr_bankswitch_w),this));
}


DRIVER_INIT_MEMBER(ddragon_state,toffy)
{
	int i, length;
	UINT8 *rom;

	m_sound_irq = M6809_IRQ_LINE;
	m_ym_irq = M6809_FIRQ_LINE;
	m_technos_video_hw = 0;
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x3808, 0x3808, write8_delegate(FUNC(ddragon_state::toffy_bankswitch_w),this));

	/* the program rom has a simple bitswap encryption */
	rom = memregion("maincpu")->base();
	length = memregion("maincpu")->bytes();
	for (i = 0; i < length; i++)
		rom[i] = BITSWAP8(rom[i], 6,7,5,4,3,2,1,0);

	/* and the fg gfx ... */
	rom = memregion("gfx1")->base();
	length = memregion("gfx1")->bytes();
	for (i = 0; i < length; i++)
		rom[i] = BITSWAP8(rom[i], 7,6,5,3,4,2,1,0);

	/* and the sprites gfx */
	rom = memregion("gfx2")->base();
	length = memregion("gfx2")->bytes();
	for (i = 0; i < length; i++)
		rom[i] = BITSWAP8(rom[i], 7,6,5,4,3,2,0,1);

	/* and the bg gfx */
	rom = memregion("gfx3")->base();
	length = memregion("gfx3")->bytes();
	for (i = 0; i < length / 2; i++)
	{
		rom[i + 0*length/2] = BITSWAP8(rom[i + 0*length/2], 7,6,1,4,3,2,5,0);
		rom[i + 1*length/2] = BITSWAP8(rom[i + 1*length/2], 7,6,2,4,3,5,1,0);
	}

	/* should the sound rom be bitswapped too? */
}

DRIVER_INIT_MEMBER(ddragon_state,ddragon6809)
{
	int i;
	UINT8 *dst,*src;

	src = memregion("chars")->base();
	dst = memregion("gfx1")->base();

	for (i = 0; i < 0x8000; i++)
	{
		switch(i & 0x18)
		{
			case 0x00: dst[i] = src[(i & ~0x18) | 0x18]; break;
			case 0x08: dst[i] = src[(i & ~0x18) | 0x00]; break;
			case 0x10: dst[i] = src[(i & ~0x18) | 0x08]; break;
			case 0x18: dst[i] = src[(i & ~0x18) | 0x10]; break;
		}
	}

	m_sprite_irq = INPUT_LINE_NMI;
	m_sound_irq = M6809_IRQ_LINE;
	m_ym_irq = M6809_FIRQ_LINE;
	m_technos_video_hw = 0;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1987, ddragon,     0,        ddragon,  ddragon, ddragon_state,  ddragon,  ROT0, "Technos Japan", "Double Dragon (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonw,    ddragon,  ddragon,  ddragon, ddragon_state,  ddragon,  ROT0, "Technos Japan (Taito license)", "Double Dragon (World set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonw1,   ddragon,  ddragon,  ddragon, ddragon_state,  ddragon,  ROT0, "Technos Japan (Taito license)", "Double Dragon (World set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonu,    ddragon,  ddragon,  ddragon, ddragon_state,  ddragon,  ROT0, "Technos Japan (Taito America license)", "Double Dragon (US set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonua,   ddragon,  ddragon,  ddragon, ddragon_state,  ddragon,  ROT0, "Technos Japan (Taito America license)", "Double Dragon (US set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonub,   ddragon,  ddragon,  ddragon, ddragon_state,  ddragon,  ROT0, "Technos Japan (Taito America license)", "Double Dragon (US set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonb2,   ddragon,  ddragon,  ddragon, ddragon_state,  ddragon,  ROT0, "bootleg", "Double Dragon (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragonb,    ddragon,  ddragonb, ddragon, ddragon_state,  ddragon,  ROT0, "bootleg", "Double Dragon (bootleg with HD6309)", MACHINE_SUPPORTS_SAVE ) // according to dump notes
GAME( 1987, ddragonba,   ddragon,  ddragonba,   ddragon, ddragon_state,  ddragon,  ROT0, "bootleg", "Double Dragon (bootleg with M6803)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ddragon6809, ddragon,  ddragon6809, ddragon, ddragon_state,  ddragon6809, ROT0, "bootleg", "Double Dragon (bootleg with 3xM6809, set 1)", MACHINE_NOT_WORKING )
GAME( 1987, ddragon6809a,ddragon,  ddragon6809, ddragon, ddragon_state,  ddragon6809, ROT0, "bootleg", "Double Dragon (bootleg with 3xM6809, set 2)", MACHINE_NOT_WORKING )

GAME( 1988, ddragon2,    0,        ddragon2, ddragon2, ddragon_state, ddragon2, ROT0, "Technos Japan", "Double Dragon II - The Revenge (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, ddragon2u,   ddragon2, ddragon2, ddragon2, ddragon_state, ddragon2, ROT0, "Technos Japan", "Double Dragon II - The Revenge (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, ddragon2j,   ddragon2, ddragon2, ddragon2, ddragon_state, ddragon2, ROT0, "Technos Japan", "Double Dragon II - The Revenge (Japan)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // bad dump
GAME( 1988, ddragon2b,   ddragon2, ddragon2, ddragon2, ddragon_state, ddragon2, ROT0, "Technos Japan", "Double Dragon II - The Revenge (US, bootleg)", MACHINE_SUPPORTS_SAVE )

/* these were conversions of double dragon */
GAME( 1991, tstrike,  0,        darktowr, tstrike, ddragon_state,  darktowr, ROT0, "East Coast Coin Company", "Thunder Strike (set 1)", MACHINE_SUPPORTS_SAVE ) // same manufacturer as The Game Room?
GAME( 1991, tstrikea, tstrike,  darktowr, tstrike, ddragon_state,  darktowr, ROT0, "The Game Room", "Thunder Strike (set 2, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, ddungeon, 0,        darktowr, ddungeon, ddragon_state, darktowr, ROT0, "The Game Room", "Dangerous Dungeons (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, ddungeone,ddungeon, darktowr, ddungeon, ddragon_state, darktowr, ROT0, "East Coast Coin Company", "Dangerous Dungeons (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, darktowr, 0,        darktowr, darktowr, ddragon_state, darktowr, ROT0, "The Game Room", "Dark Tower", MACHINE_SUPPORTS_SAVE )

/* these run on their own board, but are basically the same game. Toffy even has 'dangerous dungeons' text in it */
GAME( 1993, toffy,    0,        toffy,    toffy, ddragon_state,    toffy,    ROT0, "Midas", "Toffy", MACHINE_SUPPORTS_SAVE )

GAME( 1994, stoffy,   0,        toffy,    toffy, ddragon_state,    toffy,    ROT0, "Midas", "Super Toffy", MACHINE_SUPPORTS_SAVE )
GAME( 1994, stoffyu,  stoffy,   toffy,    toffy, ddragon_state,    toffy,    ROT0, "Midas (Unico license)", "Super Toffy (Unico license)", MACHINE_SUPPORTS_SAVE )
