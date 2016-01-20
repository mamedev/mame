// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Paul Hampson, Quench
/*
China Gate.
By Paul Hampson from First Principles
(IE: Roms + a description of their contents and a list of CPUs on board.)

Based on ddragon.c:
"Double Dragon, Double Dragon (bootleg) & Double Dragon II"
"By Carlos A. Lozano & Rob Rosenbrock et. al."

NOTES:
A couple of things unaccounted for:

No backgrounds ROMs from the original board...
- This may be related to the SubCPU. I don't think it's contributing
  much right now, but I could be wrong. And it would explain that vast
  expanse of bankswitch ROM on a slave CPU....
- Just had a look at the sprites, and they seem like kosher sprites all
  the way up.... So it must be hidden in the sub-cpu somewhere?
- Got two bootleg sets with background gfx roms. Using those on the
  original games for now.

OBVIOUS SPEED PROBLEMS...
- Timers are too fast and/or too slow, and the whole thing's moving too fast

Port 0x2800 on the Sub CPU.
- All those I/O looking ports on the main CPU (0x3exx and 0x3fxx)
- One's scroll control. Probably other video control as well.
- Location 0x1a2ec in cgate51.bin (The main CPU's ROM) is 88. This is
  copied to videoram, and causes that minor visual discrepancy on
  the title screen. But the CPU tests that part of the ROM and passes
  it OK. Since it's just a simple summing of words, another word
  somewhere (or others in total) has lost 0x8000. Or the original
  game had this problem. (Not on the screenshot I got)
- The Japanese ones have a different title screen so I can't check.

ADPCM in the bootlegs is not quite right.... Misusing the data?
- They're nibble-swapped versions of the original roms...
- There's an Intel i8748 CPU on the bootlegs (bootleg 1 lists D8749 but
  the microcode dump's the same). This in conjunction with the different
  ADPCM chip (msm5205) are used to 'fake' a M6295.
- Bootleg 1 ADPCM is now wired up, but still not working :-(
  Definantly sync problems between the i8049 and the m5205 which need
  further looking at.


There's also a few small dumps from the boards.


MAJOR DIFFERENCES FROM DOUBLE DRAGON:
Sound system is like Double Dragon II (In fact for MAME's
purposes it's identical. I think DD3 and one or two others
also use this. Was it an addon on the original?
The dual-CPU setup looked similar to DD at first, but
the second CPU doesn't talk to the sprite RAM at all, but
just through the shared memory (which DD1 doesn't have,
except for the sprite RAM.)
Also the 2nd CPU in China Gate has just as much code as
the first CPU, and bankswitches similarly, where DD1 and DD2 have
different Sprite CPUs but only a small bank of code each.
More characters and colours of characters than DD1 or 2.
More sprites than DD1, less than DD2.
But the formats are the same (allowing for extra chars and colours)
Video hardware's like DD1 (thank god)
Input is unique but has a few similarities to DD2 (the coin inputs)

2008-07
Dip locations and factory settings verified with China Gate US manual.
*/



#include "emu.h"
#include "cpu/m6809/hd6309.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/2151intf.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "sound/msm5205.h"
#include "includes/ddragon.h"

#define MAIN_CLOCK      XTAL_12MHz
#define PIXEL_CLOCK     MAIN_CLOCK / 2

class chinagat_state : public ddragon_state
{
public:
	chinagat_state(const machine_config &mconfig, device_type type, std::string tag)
		: ddragon_state(mconfig, type, tag),
		m_adpcm(*this, "adpcm") { };

	TIMER_DEVICE_CALLBACK_MEMBER(chinagat_scanline);
	DECLARE_DRIVER_INIT(chinagat);
	DECLARE_MACHINE_START(chinagat);
	DECLARE_MACHINE_RESET(chinagat);
	DECLARE_VIDEO_START(chinagat);
	DECLARE_WRITE8_MEMBER( chinagat_interrupt_w );
	DECLARE_WRITE8_MEMBER( chinagat_video_ctrl_w );
	DECLARE_WRITE8_MEMBER( chinagat_bankswitch_w );
	DECLARE_WRITE8_MEMBER( chinagat_sub_bankswitch_w );
	DECLARE_READ8_MEMBER( saiyugoub1_mcu_command_r );
	DECLARE_WRITE8_MEMBER( saiyugoub1_mcu_command_w );
	DECLARE_WRITE8_MEMBER( saiyugoub1_adpcm_rom_addr_w );
	DECLARE_WRITE8_MEMBER( saiyugoub1_adpcm_control_w );
	DECLARE_WRITE8_MEMBER( saiyugoub1_m5205_clk_w );
	DECLARE_READ8_MEMBER( saiyugoub1_m5205_irq_r );
	DECLARE_WRITE_LINE_MEMBER(saiyugoub1_m5205_irq_w);
	DECLARE_WRITE_LINE_MEMBER(chinagat_irq_handler);
	optional_device<msm5205_device> m_adpcm;
};


VIDEO_START_MEMBER(chinagat_state,chinagat)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(chinagat_state::get_bg_tile_info),this),tilemap_mapper_delegate(FUNC(chinagat_state::background_scan),this), 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(chinagat_state::get_fg_16color_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrolldy(-8, -8);
	m_bg_tilemap->set_scrolldy(-8, -8);
}
/*
    Based on the Solar Warrior schematics, vertical timing counts as follows:

        08,09,0A,0B,...,FC,FD,FE,FF,E8,E9,EA,EB,...,FC,FD,FE,FF,
        08,09,....

    Thus, it counts from 08 to FF, then resets to E8 and counts to FF again.
    This gives (256 - 8) + (256 - 232) = 248 + 24 = 272 total scanlines.

    VBLK is signalled starting when the counter hits F8, and continues through
    the reset to E8 and through until the next reset to 08 again.

    Since MAME's video timing is 0-based, we need to convert this.
*/

TIMER_DEVICE_CALLBACK_MEMBER(chinagat_state::chinagat_scanline)
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

	/* adjust for next scanline */
	if (++scanline >= screen_height)
		scanline = 0;
}

WRITE8_MEMBER(chinagat_state::chinagat_interrupt_w )
{
	switch (offset)
	{
		case 0: /* 3e00 - SND irq */
			soundlatch_byte_w(space, 0, data);
			m_soundcpu->set_input_line(m_sound_irq, (m_sound_irq == INPUT_LINE_NMI) ? PULSE_LINE : HOLD_LINE );
			break;

		case 1: /* 3e01 - NMI ack */
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			break;

		case 2: /* 3e02 - FIRQ ack */
			m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
			break;

		case 3: /* 3e03 - IRQ ack */
			m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
			break;

		case 4: /* 3e04 - sub CPU IRQ ack */
			m_subcpu->set_input_line(m_sprite_irq, (m_sprite_irq == INPUT_LINE_NMI) ? PULSE_LINE : HOLD_LINE );
			break;
	}
}

WRITE8_MEMBER(chinagat_state::chinagat_video_ctrl_w )
{
	/***************************
	---- ---x   X Scroll MSB
	---- --x-   Y Scroll MSB
	---- -x--   Flip screen
	--x- ----   Enable video ???
	****************************/
	m_scrolly_hi = ((data & 0x02) >> 1);
	m_scrollx_hi = data & 0x01;

	flip_screen_set(~data & 0x04);
}

WRITE8_MEMBER(chinagat_state::chinagat_bankswitch_w )
{
	membank("bank1")->set_entry(data & 0x07); // shall we check (data & 7) < 6 (# of banks)?
}

WRITE8_MEMBER(chinagat_state::chinagat_sub_bankswitch_w )
{
	membank("bank4")->set_entry(data & 0x07); // shall we check (data & 7) < 6 (# of banks)?
}

READ8_MEMBER(chinagat_state::saiyugoub1_mcu_command_r )
{
#if 0
	if (m_mcu_command == 0x78)
	{
		m_mcu->suspend(SUSPEND_REASON_HALT, 1); /* Suspend (speed up) */
	}
#endif
	return m_mcu_command;
}

WRITE8_MEMBER(chinagat_state::saiyugoub1_mcu_command_w )
{
	m_mcu_command = data;
#if 0
	if (data != 0x78)
	{
		m_mcu->resume(SUSPEND_REASON_HALT); /* Wake up */
	}
#endif
}

WRITE8_MEMBER(chinagat_state::saiyugoub1_adpcm_rom_addr_w )
{
	/* i8748 Port 1 write */
	m_i8748_P1 = data;
}

WRITE8_MEMBER(chinagat_state::saiyugoub1_adpcm_control_w )
{
	/* i8748 Port 2 write */
	UINT8 *saiyugoub1_adpcm_rom = memregion("adpcm")->base();

	if (data & 0x80)    /* Reset m5205 and disable ADPCM ROM outputs */
	{
		logerror("ADPCM output disabled\n");
		m_pcm_nibble = 0x0f;
		m_adpcm->reset_w(1);
	}
	else
	{
		if ((m_i8748_P2 & 0xc) != (data & 0xc))
		{
			if ((m_i8748_P2 & 0xc) == 0) /* Latch MSB Address */
			{
///             logerror("Latching MSB\n");
				m_adpcm_addr = (m_adpcm_addr & 0x3807f) | (m_i8748_P1 << 7);
			}
			if ((m_i8748_P2 & 0xc) == 4) /* Latch LSB Address */
			{
///             logerror("Latching LSB\n");
				m_adpcm_addr = (m_adpcm_addr & 0x3ff80) | (m_i8748_P1 >> 1);
				m_pcm_shift = (m_i8748_P1 & 1) * 4;
			}
		}

		m_adpcm_addr = ((m_adpcm_addr & 0x07fff) | (data & 0x70 << 11));

		m_pcm_nibble = saiyugoub1_adpcm_rom[m_adpcm_addr & 0x3ffff];

		m_pcm_nibble = (m_pcm_nibble >> m_pcm_shift) & 0x0f;

///     logerror("Writing %02x to m5205. $ROM=%08x  P1=%02x  P2=%02x  Prev_P2=%02x  Nibble=%08x\n", m_pcm_nibble, m_adpcm_addr, m_i8748_P1, data, m_i8748_P2, m_pcm_shift);

		if (((m_i8748_P2 & 0xc) >= 8) && ((data & 0xc) == 4))
		{
			m_adpcm->data_w(m_pcm_nibble);
			logerror("Writing %02x to m5205\n", m_pcm_nibble);
		}
		logerror("$ROM=%08x  P1=%02x  P2=%02x  Prev_P2=%02x  Nibble=%1x  PCM_data=%02x\n", m_adpcm_addr, m_i8748_P1, data, m_i8748_P2, m_pcm_shift, m_pcm_nibble);
	}
	m_i8748_P2 = data;
}

WRITE8_MEMBER(chinagat_state::saiyugoub1_m5205_clk_w )
{
	/* i8748 T0 output clk mode */
	/* This signal goes through a divide by 8 counter */
	/* to the xtal pins of the MSM5205 */

	/* Actually, T0 output clk mode is not supported by the i8048 core */
#if 0
	m_m5205_clk++;
	if (m_m5205_clk == 8)
	{
		m_adpcm->vclk_w(1);      /* ??? */
		m_m5205_clk = 0;
	}
	else
		m_adpcm->vclk_w(0);      /* ??? */
#endif
}

READ8_MEMBER(chinagat_state::saiyugoub1_m5205_irq_r )
{
	if (m_adpcm_sound_irq)
	{
		m_adpcm_sound_irq = 0;
		return 1;
	}
	return 0;
}

WRITE_LINE_MEMBER(chinagat_state::saiyugoub1_m5205_irq_w)
{
	m_adpcm_sound_irq = 1;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, chinagat_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(ddragon_fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(ddragon_bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x3000, 0x317f) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x3400, 0x357f) AM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0x3800, 0x397f) AM_WRITE_BANK("bank3") AM_SHARE("spriteram")
	AM_RANGE(0x3e00, 0x3e04) AM_WRITE(chinagat_interrupt_w)
	AM_RANGE(0x3e06, 0x3e06) AM_WRITEONLY AM_SHARE("scrolly_lo")
	AM_RANGE(0x3e07, 0x3e07) AM_WRITEONLY AM_SHARE("scrollx_lo")
	AM_RANGE(0x3f00, 0x3f00) AM_WRITE(chinagat_video_ctrl_w)
	AM_RANGE(0x3f01, 0x3f01) AM_WRITE(chinagat_bankswitch_w)
	AM_RANGE(0x3f00, 0x3f00) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3f01, 0x3f01) AM_READ_PORT("DSW1")
	AM_RANGE(0x3f02, 0x3f02) AM_READ_PORT("DSW2")
	AM_RANGE(0x3f03, 0x3f03) AM_READ_PORT("P1")
	AM_RANGE(0x3f04, 0x3f04) AM_READ_PORT("P2")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8, chinagat_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x2000) AM_WRITE(chinagat_sub_bankswitch_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITEONLY /* Called on CPU start and after return from jump table */
//  AM_RANGE(0x2a2b, 0x2a2b) AM_READNOP /* What lives here? */
//  AM_RANGE(0x2a30, 0x2a30) AM_READNOP /* What lives here? */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank4")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, chinagat_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xA000, 0xA000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ym2203c_sound_map, AS_PROGRAM, 8, chinagat_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
// 8804 and/or 8805 make a gong sound when the coin goes in
// but only on the title screen....

	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
//  AM_RANGE(0x8802, 0x8802) AM_DEVREADWRITE("oki", okim6295_device, read, write)
//  AM_RANGE(0x8803, 0x8803) AM_DEVWRITE("oki", okim6295_device, write)
	AM_RANGE(0x8804, 0x8805) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
//  AM_RANGE(0x8804, 0x8804) AM_WRITEONLY
//  AM_RANGE(0x8805, 0x8805) AM_WRITEONLY

//  AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
//  AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xA000, 0xA000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( saiyugoub1_sound_map, AS_PROGRAM, 8, chinagat_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x9800, 0x9800) AM_WRITE(saiyugoub1_mcu_command_w)
	AM_RANGE(0xA000, 0xA000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8748_map, AS_PROGRAM, 8, chinagat_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0x07ff) AM_ROM     /* i8749 version */
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8748_portmap, AS_IO, 8, chinagat_state )
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READ(saiyugoub1_mcu_command_r)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_WRITE(saiyugoub1_m5205_clk_w)      /* Drives the clock on the m5205 at 1/8 of this frequency */
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(saiyugoub1_m5205_irq_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(saiyugoub1_adpcm_rom_addr_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(saiyugoub1_adpcm_control_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( chinagat )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
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
	/*PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )*/
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // "SW2:4" - Left empty in the manual
	PORT_DIPNAME( 0x30, 0x20, "Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x20, "55" )
	PORT_DIPSETTING(    0x30, "60" )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "4" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,            /* 8*8 chars */
	RGN_FRAC(1,1),  /* num of characters */
	4,              /* 4 bits per pixel */
	{ 0, 2, 4, 6 },     /* plane offset */
	{ 1, 0, 65, 64, 129, 128, 193, 192 },
	{ STEP8(0,8) },         /* { 0*8, 1*8 ... 6*8, 7*8 }, */
	32*8 /* every char takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,          /* 16x16 chars */
	RGN_FRAC(1,2),  /* num of Tiles/Sprites */
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0,4 }, /* plane offset */
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
		32*8+3,32*8+2 ,32*8+1 ,32*8+0 ,48*8+3 ,48*8+2 ,48*8+1 ,48*8+0 },
	{ STEP16(0,8) },        /* { 0*8, 1*8 ... 14*8, 15*8 }, */
	64*8 /* every char takes 64 consecutive bytes */
};

static GFXDECODE_START( chinagat )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0,16 )    /*  8x8  chars */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 128, 8 )    /* 16x16 sprites */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 256, 8 )    /* 16x16 background tiles */
GFXDECODE_END


WRITE_LINE_MEMBER(chinagat_state::chinagat_irq_handler)
{
	m_soundcpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE );
}

MACHINE_START_MEMBER(chinagat_state,chinagat)
{
	/* configure banks */
	membank("bank1")->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);

	/* register for save states */
	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrolly_hi));
	save_item(NAME(m_adpcm_sound_irq));
	save_item(NAME(m_adpcm_addr));
	save_item(NAME(m_pcm_shift));
	save_item(NAME(m_pcm_nibble));
	save_item(NAME(m_i8748_P1));
	save_item(NAME(m_i8748_P2));
	save_item(NAME(m_mcu_command));
#if 0
	save_item(NAME(m_m5205_clk));
#endif
}


MACHINE_RESET_MEMBER(chinagat_state,chinagat)
{
	m_scrollx_hi = 0;
	m_scrolly_hi = 0;
	m_adpcm_sound_irq = 0;
	m_adpcm_addr = 0;
	m_pcm_shift = 0;
	m_pcm_nibble = 0;
	m_i8748_P1 = 0;
	m_i8748_P2 = 0;
	m_mcu_command = 0;
#if 0
	m_m5205_clk = 0;
#endif
}


static MACHINE_CONFIG_START( chinagat, chinagat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, MAIN_CLOCK / 2)     /* 1.5 MHz (12MHz oscillator / 4 internally) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", chinagat_state, chinagat_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", HD6309, MAIN_CLOCK / 2)     /* 1.5 MHz (12MHz oscillator / 4 internally) */
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_CPU_ADD("soundcpu", Z80, XTAL_3_579545MHz)     /* 3.579545 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000)) /* heavy interleaving to sync up sprite<->main cpu's */

	MCFG_MACHINE_START_OVERRIDE(chinagat_state,chinagat)
	MCFG_MACHINE_RESET_OVERRIDE(chinagat_state,chinagat)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240)   /* based on ddragon driver */
	MCFG_SCREEN_UPDATE_DRIVER(chinagat_state, screen_update_ddragon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", chinagat)
	MCFG_PALETTE_ADD("palette", 384)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(chinagat_state,chinagat)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("soundcpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.80)
	MCFG_SOUND_ROUTE(1, "mono", 0.80)

	MCFG_OKIM6295_ADD("oki", 1065000, OKIM6295_PIN7_HIGH) // pin 7 not verified, clock frequency estimated with recording
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( saiyugoub1, chinagat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MAIN_CLOCK / 8)      /* 68B09EP 1.5 MHz (12MHz oscillator) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", chinagat_state, chinagat_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M6809, MAIN_CLOCK / 8)      /* 68B09EP 1.5 MHz (12MHz oscillator) */
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_CPU_ADD("soundcpu", Z80, XTAL_3_579545MHz)     /* 3.579545 MHz oscillator */
	MCFG_CPU_PROGRAM_MAP(saiyugoub1_sound_map)

	MCFG_CPU_ADD("mcu", I8748, 9263750)     /* 9.263750 MHz oscillator, divided by 3*5 internally */
	MCFG_CPU_PROGRAM_MAP(i8748_map)
	MCFG_CPU_IO_MAP(i8748_portmap)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* heavy interleaving to sync up sprite<->main cpu's */

	MCFG_MACHINE_START_OVERRIDE(chinagat_state,chinagat)
	MCFG_MACHINE_RESET_OVERRIDE(chinagat_state,chinagat)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240)   /* based on ddragon driver */
	MCFG_SCREEN_UPDATE_DRIVER(chinagat_state, screen_update_ddragon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", chinagat)
	MCFG_PALETTE_ADD("palette", 384)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(chinagat_state,chinagat)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("soundcpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.80)
	MCFG_SOUND_ROUTE(1, "mono", 0.80)

	MCFG_SOUND_ADD("adpcm", MSM5205, 9263750 / 24)
	MCFG_MSM5205_VCLK_CB(WRITELINE(chinagat_state, saiyugoub1_m5205_irq_w)) /* Interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S64_4B)          /* vclk input mode (6030Hz, 4-bit) */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( saiyugoub2, chinagat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MAIN_CLOCK / 8)      /* 1.5 MHz (12MHz oscillator) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", chinagat_state, chinagat_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M6809, MAIN_CLOCK / 8)      /* 1.5 MHz (12MHz oscillator) */
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_CPU_ADD("soundcpu", Z80, XTAL_3_579545MHz)     /* 3.579545 MHz oscillator */
	MCFG_CPU_PROGRAM_MAP(ym2203c_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000)) /* heavy interleaving to sync up sprite<->main cpu's */

	MCFG_MACHINE_START_OVERRIDE(chinagat_state,chinagat)
	MCFG_MACHINE_RESET_OVERRIDE(chinagat_state,chinagat)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 0, 240)   /* based on ddragon driver */
	MCFG_SCREEN_UPDATE_DRIVER(chinagat_state, screen_update_ddragon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", chinagat)
	MCFG_PALETTE_ADD("palette", 384)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(chinagat_state,chinagat)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 3579545)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(chinagat_state, chinagat_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 0.80)

	MCFG_SOUND_ADD("ym2", YM2203, 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 0.80)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( chinagat )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* Main CPU: 128KB for code (bankswitched using $3F01) */
	ROM_LOAD( "cgate51.bin", 0x10000, 0x18000, CRC(439a3b19) SHA1(01393b4302ac7a66390270b01e2757582240f6b8) )   /* Banks 0x4000 long @ 0x4000 */
	ROM_CONTINUE(            0x08000, 0x08000 )             /* Static code */

	ROM_REGION( 0x28000, "sub", 0 ) /* Slave CPU: 128KB for code (bankswitched using $2000) */
	ROM_LOAD( "23j4-0.48",   0x10000, 0x18000, CRC(2914af38) SHA1(3d690fa50b7d36a22de82c026d59a16126a7b73c) ) /* Banks 0x4000 long @ 0x4000 */
	ROM_CONTINUE(            0x08000, 0x08000 )             /* Static code */

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Music CPU, 64KB */
	ROM_LOAD( "23j0-0.40",   0x00000, 0x08000, CRC(9ffcadb6) SHA1(606dbdd73aee3cabb2142200ac6f8c96169e4b19) )

	ROM_REGION(0x20000, "gfx1", 0 ) /* Text */
	ROM_LOAD( "cgate18.bin", 0x00000, 0x20000, CRC(8d88d64d) SHA1(57265138ebb0c6419542cce5953aee7335bfa2bd) )   /* 0,1,2,3 */

	ROM_REGION(0x80000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "23j7-0.103",  0x00000, 0x20000, CRC(2f445030) SHA1(3fcf32097e655e963d952d01a30396dc195269ca) )   /* 2,3 */
	ROM_LOAD( "23j8-0.102",  0x20000, 0x20000, CRC(237f725a) SHA1(47bebe5b9878ca10fe6efd4f353717e53a372416) )   /* 2,3 */
	ROM_LOAD( "23j9-0.101",  0x40000, 0x20000, CRC(8caf6097) SHA1(50ad192f831b055586a4a9974f8c6c2f2063ede5) )   /* 0,1 */
	ROM_LOAD( "23ja-0.100",  0x60000, 0x20000, CRC(f678594f) SHA1(4bdcf9407543925f4630a8c7f1f48b85f76343a9) )   /* 0,1 */

	ROM_REGION(0x40000, "gfx3", 0 ) /* Background */
	ROM_LOAD( "chinagat_a-13", 0x00000, 0x10000, BAD_DUMP CRC(b745cac4) SHA1(759767ca7c5123b03b9e1a42bb105d194cb76400) ) // not dumped yet, these were taken from the bootleg set instead
	ROM_LOAD( "chinagat_a-12", 0x10000, 0x10000, BAD_DUMP CRC(3c864299) SHA1(cb12616e4d6c53a82beb4cd51510a632894b359c) ) // Where are these on the real board?
	ROM_LOAD( "chinagat_a-15", 0x20000, 0x10000, BAD_DUMP CRC(2f268f37) SHA1(f82cfe3b2001d5ed2a709ca9c51febcf624bb627) )
	ROM_LOAD( "chinagat_a-14", 0x30000, 0x10000, BAD_DUMP CRC(aef814c8) SHA1(f6b9229ca7beb9a0e47d1f6a1083c6102fdd20c8) )

	ROM_REGION(0x40000, "oki", 0 )  /* ADPCM */
	ROM_LOAD( "23j1-0.53", 0x00000, 0x20000, CRC(f91f1001) SHA1(378402a3c966cabd61e9662ae5decd66672a228b) )
	ROM_LOAD( "23j2-0.52", 0x20000, 0x20000, CRC(8b6f26e9) SHA1(7da26ae846814b3957b19c38b6bf7e83617dc6cc) )

	ROM_REGION(0x300, "user1", 0 )  /* Unknown Bipolar PROMs */
	ROM_LOAD( "23jb-0.16", 0x000, 0x200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) ) /* 82S131 on video board */
	ROM_LOAD( "23j5-0.45", 0x200, 0x100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) ) /* 82S129 on main board */
ROM_END


ROM_START( saiyugou )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* Main CPU: 128KB for code (bankswitched using $3F01) */
	ROM_LOAD( "23j3-0.51",  0x10000, 0x18000, CRC(aa8132a2) SHA1(87c3bd447767f263113c4865afc905a0e484a625) )    /* Banks 0x4000 long @ 0x4000 */
	ROM_CONTINUE(           0x08000, 0x08000)               /* Static code */

	ROM_REGION( 0x28000, "sub", 0 ) /* Slave CPU: 128KB for code (bankswitched using $2000) */
	ROM_LOAD( "23j4-0.48",  0x10000, 0x18000, CRC(2914af38) SHA1(3d690fa50b7d36a22de82c026d59a16126a7b73c) )    /* Banks 0x4000 long @ 0x4000 */
	ROM_CONTINUE(           0x08000, 0x08000)               /* Static code */

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Music CPU, 64KB */
	ROM_LOAD( "23j0-0.40",  0x00000, 0x8000, CRC(9ffcadb6) SHA1(606dbdd73aee3cabb2142200ac6f8c96169e4b19) )

	ROM_REGION(0x20000, "gfx1", 0 ) /* Text */
	ROM_LOAD( "23j6-0.18",  0x00000, 0x20000, CRC(86d33df0) SHA1(3419959c28703c5177de9c11b61e1dba9e76aca5) )    /* 0,1,2,3 */

	ROM_REGION(0x80000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "23j7-0.103", 0x00000, 0x20000, CRC(2f445030) SHA1(3fcf32097e655e963d952d01a30396dc195269ca) )    /* 2,3 */
	ROM_LOAD( "23j8-0.102", 0x20000, 0x20000, CRC(237f725a) SHA1(47bebe5b9878ca10fe6efd4f353717e53a372416) )    /* 2,3 */
	ROM_LOAD( "23j9-0.101", 0x40000, 0x20000, CRC(8caf6097) SHA1(50ad192f831b055586a4a9974f8c6c2f2063ede5) )    /* 0,1 */
	ROM_LOAD( "23ja-0.100", 0x60000, 0x20000, CRC(f678594f) SHA1(4bdcf9407543925f4630a8c7f1f48b85f76343a9) )    /* 0,1 */

	ROM_REGION(0x40000, "gfx3", 0 ) /* Background */
	ROM_LOAD( "saiyugou_a-13", 0x00000, 0x10000, BAD_DUMP CRC(b745cac4) SHA1(759767ca7c5123b03b9e1a42bb105d194cb76400) ) // not dumped yet, these were taken from the bootleg set instead
	ROM_LOAD( "saiyugou_a-12", 0x10000, 0x10000, BAD_DUMP CRC(3c864299) SHA1(cb12616e4d6c53a82beb4cd51510a632894b359c) ) // Where are these on the real board?
	ROM_LOAD( "saiyugou_a-15", 0x20000, 0x10000, BAD_DUMP CRC(2f268f37) SHA1(f82cfe3b2001d5ed2a709ca9c51febcf624bb627) )
	ROM_LOAD( "saiyugou_a-14", 0x30000, 0x10000, BAD_DUMP CRC(aef814c8) SHA1(f6b9229ca7beb9a0e47d1f6a1083c6102fdd20c8) )

	ROM_REGION(0x40000, "oki", 0 )  /* ADPCM */
	ROM_LOAD( "23j1-0.53", 0x00000, 0x20000, CRC(f91f1001) SHA1(378402a3c966cabd61e9662ae5decd66672a228b) )
	ROM_LOAD( "23j2-0.52", 0x20000, 0x20000, CRC(8b6f26e9) SHA1(7da26ae846814b3957b19c38b6bf7e83617dc6cc) )

	ROM_REGION(0x300, "user1", 0 )  /* Unknown Bipolar PROMs */
	ROM_LOAD( "23jb-0.16", 0x000, 0x200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) ) /* 82S131 on video board */
	ROM_LOAD( "23j5-0.45", 0x200, 0x100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) ) /* 82S129 on main board */
ROM_END

ROM_START( saiyugoub1 )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* Main CPU: 128KB for code (bankswitched using $3F01) */
	ROM_LOAD( "23j3-0.51",  0x10000, 0x18000, CRC(aa8132a2) SHA1(87c3bd447767f263113c4865afc905a0e484a625) )    /* Banks 0x4000 long @ 0x4000 */
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "a-5.bin", 0x10000, 0x10000, CRC(39795aa5) )      Banks 0x4000 long @ 0x4000
	   ROM_LOAD( "a-9.bin", 0x20000, 0x08000, CRC(051ebe92) )      Banks 0x4000 long @ 0x4000
	*/
	ROM_CONTINUE(           0x08000, 0x08000 )              /* Static code */

	ROM_REGION( 0x28000, "sub", 0 ) /* Slave CPU: 128KB for code (bankswitched using $2000) */
	ROM_LOAD( "23j4-0.48",  0x10000, 0x18000, CRC(2914af38) SHA1(3d690fa50b7d36a22de82c026d59a16126a7b73c) )    /* Banks 0x4000 long @ 0x4000 */
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "a-4.bin", 0x10000, 0x10000, CRC(9effddc1) )      Banks 0x4000 long @ 0x4000
	   ROM_LOAD( "a-8.bin", 0x20000, 0x08000, CRC(a436edb8) )      Banks 0x4000 long @ 0x4000
	*/
	ROM_CONTINUE(           0x08000, 0x08000 )              /* Static code */

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Music CPU, 64KB */
	ROM_LOAD( "a-1.bin",  0x00000, 0x8000,  CRC(46e5a6d4) SHA1(965ed7bdb727ab32ce3322ca49f1a4e3786e8051) )

	ROM_REGION( 0x800, "mcu", 0 )       /* ADPCM CPU, 1KB */
	ROM_LOAD( "mcu8748.bin", 0x000, 0x400, CRC(6d28d6c5) SHA1(20582c62a72545e68c2e155b063ee7e95e1228ce) )

	ROM_REGION(0x20000, "gfx1", 0 ) /* Text */
	ROM_LOAD( "23j6-0.18",  0x00000, 0x20000, CRC(86d33df0) SHA1(3419959c28703c5177de9c11b61e1dba9e76aca5) )    /* 0,1,2,3 */
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "a-2.bin", 0x00000, 0x10000, CRC(baa5a3b9) )      0,1
	   ROM_LOAD( "a-3.bin", 0x10000, 0x10000, CRC(532d59be) )      2,3
	*/

	ROM_REGION(0x80000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "23j7-0.103",  0x00000, 0x20000, CRC(2f445030) SHA1(3fcf32097e655e963d952d01a30396dc195269ca) )   /* 2,3 */
	ROM_LOAD( "23j8-0.102",  0x20000, 0x20000, CRC(237f725a) SHA1(47bebe5b9878ca10fe6efd4f353717e53a372416) )   /* 2,3 */
	ROM_LOAD( "23j9-0.101",  0x40000, 0x20000, CRC(8caf6097) SHA1(50ad192f831b055586a4a9974f8c6c2f2063ede5) )   /* 0,1 */
	ROM_LOAD( "23ja-0.100",  0x60000, 0x20000, CRC(f678594f) SHA1(4bdcf9407543925f4630a8c7f1f48b85f76343a9) )   /* 0,1 */
	/* Orientation of bootleg ROMs which are split, but otherwise the same
	   ROM_LOAD( "a-23.bin", 0x00000, 0x10000, CRC(12b56225) )     2,3
	   ROM_LOAD( "a-22.bin", 0x10000, 0x10000, CRC(b592aa9b) )     2,3
	   ROM_LOAD( "a-21.bin", 0x20000, 0x10000, CRC(a331ba3d) )     2,3
	   ROM_LOAD( "a-20.bin", 0x30000, 0x10000, CRC(2515d742) )     2,3
	   ROM_LOAD( "a-19.bin", 0x40000, 0x10000, CRC(d796f2e4) )     0,1
	   ROM_LOAD( "a-18.bin", 0x50000, 0x10000, CRC(c9e1c2f9) )     0,1
	   ROM_LOAD( "a-17.bin", 0x60000, 0x10000, CRC(00b6db0a) )     0,1
	   ROM_LOAD( "a-16.bin", 0x70000, 0x10000, CRC(f196818b) )     0,1
	*/

	ROM_REGION(0x40000, "gfx3", 0 ) /* Background */
	ROM_LOAD( "a-13", 0x00000, 0x10000, CRC(b745cac4) SHA1(759767ca7c5123b03b9e1a42bb105d194cb76400) )
	ROM_LOAD( "a-12", 0x10000, 0x10000, CRC(3c864299) SHA1(cb12616e4d6c53a82beb4cd51510a632894b359c) )
	ROM_LOAD( "a-15", 0x20000, 0x10000, CRC(2f268f37) SHA1(f82cfe3b2001d5ed2a709ca9c51febcf624bb627) )
	ROM_LOAD( "a-14", 0x30000, 0x10000, CRC(aef814c8) SHA1(f6b9229ca7beb9a0e47d1f6a1083c6102fdd20c8) )

	/* Some bootlegs have incorrectly halved the ADPCM data ! */
	/* These are same as the 128k sample except nibble-swapped */
	ROM_REGION(0x40000, "adpcm", 0 )    /* ADPCM */     /* Bootleggers wrong data */
	ROM_LOAD ( "a-6.bin",   0x00000, 0x10000, CRC(4da4e935) SHA1(235a1589165a23cfad29e07cf66d7c3a777fc904) )    /* 0x8000, 0x7cd47f01 */
	ROM_LOAD ( "a-7.bin",   0x10000, 0x10000, CRC(6284c254) SHA1(e01be1bd4768ae0ccb1cec65b3a6bc80ed7a4b00) )    /* 0x8000, 0x7091959c */
	ROM_LOAD ( "a-10.bin",  0x20000, 0x10000, CRC(b728ec6e) SHA1(433b5f907e4918e89b79bd927e2993ad3030017b) )    /* 0x8000, 0x78349cb6 */
	ROM_LOAD ( "a-11.bin",  0x30000, 0x10000, CRC(a50d1895) SHA1(0c2c1f8a2e945d6c53ce43413f0e63ced45bae17) )    /* 0x8000, 0xaa5b6834 */

	ROM_REGION(0x300, "user1", 0 )  /* Unknown Bipolar PROMs */
	ROM_LOAD( "23jb-0.16", 0x000, 0x200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) ) /* 82S131 on video board */
	ROM_LOAD( "23j5-0.45", 0x200, 0x100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) ) /* 82S129 on main board */
ROM_END

ROM_START( saiyugoub2 )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* Main CPU: 128KB for code (bankswitched using $3F01) */
	ROM_LOAD( "23j3-0.51",   0x10000, 0x18000, CRC(aa8132a2) SHA1(87c3bd447767f263113c4865afc905a0e484a625) )   /* Banks 0x4000 long @ 0x4000 */
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "sai5.bin", 0x10000, 0x10000, CRC(39795aa5) )     Banks 0x4000 long @ 0x4000
	   ROM_LOAD( "sai9.bin", 0x20000, 0x08000, CRC(051ebe92) )     Banks 0x4000 long @ 0x4000
	*/
	ROM_CONTINUE(            0x08000, 0x08000 )             /* Static code */

	ROM_REGION( 0x28000, "sub", 0 ) /* Slave CPU: 128KB for code (bankswitched using $2000) */
	ROM_LOAD( "23j4-0.48", 0x10000, 0x18000, CRC(2914af38) SHA1(3d690fa50b7d36a22de82c026d59a16126a7b73c) ) /* Banks 0x4000 long @ 0x4000 */
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "sai4.bin", 0x10000, 0x10000, CRC(9effddc1) )     Banks 0x4000 long @ 0x4000
	   ROM_LOAD( "sai8.bin", 0x20000, 0x08000, CRC(a436edb8) )     Banks 0x4000 long @ 0x4000
	*/
	ROM_CONTINUE(         0x08000, 0x08000 )                /* Static code */

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Music CPU, 64KB */
	ROM_LOAD( "sai-alt1.bin", 0x00000, 0x8000, CRC(8d397a8d) SHA1(52599521c3dbcecc1ae56bb80dc855e76d700134) )

//  ROM_REGION( 0x800, "cpu3", 0 )     /* ADPCM CPU, 1KB */
//  ROM_LOAD( "sgr-8749.bin", 0x000, 0x800, CRC(9237e8c5) ) /* same as above but padded with 00 for different mcu */

	ROM_REGION(0x20000, "gfx1", 0 ) /* Text */
	ROM_LOAD( "23j6-0.18", 0x00000, 0x20000, CRC(86d33df0) SHA1(3419959c28703c5177de9c11b61e1dba9e76aca5) ) /* 0,1,2,3 */
	/* Orientation of bootleg ROMs which are split, but otherwise the same.
	   ROM_LOAD( "sai2.bin", 0x00000, 0x10000, CRC(baa5a3b9) )     0,1
	   ROM_LOAD( "sai3.bin", 0x10000, 0x10000, CRC(532d59be) )     2,3
	*/

	ROM_REGION(0x80000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "23j7-0.103",   0x00000, 0x20000, CRC(2f445030) SHA1(3fcf32097e655e963d952d01a30396dc195269ca) )  /* 2,3 */
	ROM_LOAD( "23j8-0.102",   0x20000, 0x20000, CRC(237f725a) SHA1(47bebe5b9878ca10fe6efd4f353717e53a372416) )  /* 2,3 */
	ROM_LOAD( "23j9-0.101",   0x40000, 0x20000, CRC(8caf6097) SHA1(50ad192f831b055586a4a9974f8c6c2f2063ede5) )  /* 0,1 */
	ROM_LOAD( "23ja-0.100",   0x60000, 0x20000, CRC(f678594f) SHA1(4bdcf9407543925f4630a8c7f1f48b85f76343a9) )  /* 0,1 */
	/* Orientation of bootleg ROMs which are split, but otherwise the same
	   ROM_LOAD( "sai23.bin", 0x00000, 0x10000, CRC(12b56225) )    2,3
	   ROM_LOAD( "sai22.bin", 0x10000, 0x10000, CRC(b592aa9b) )    2,3
	   ROM_LOAD( "sai21.bin", 0x20000, 0x10000, CRC(a331ba3d) )    2,3
	   ROM_LOAD( "sai20.bin", 0x30000, 0x10000, CRC(2515d742) )    2,3
	   ROM_LOAD( "sai19.bin", 0x40000, 0x10000, CRC(d796f2e4) )    0,1
	   ROM_LOAD( "sai18.bin", 0x50000, 0x10000, CRC(c9e1c2f9) )    0,1
	   ROM_LOAD( "roku17.bin",0x60000, 0x10000, CRC(00b6db0a) )    0,1
	   ROM_LOAD( "sai16.bin", 0x70000, 0x10000, CRC(f196818b) )    0,1
	*/

	ROM_REGION(0x40000, "gfx3", 0 ) /* Background */
	ROM_LOAD( "a-13", 0x00000, 0x10000, CRC(b745cac4) SHA1(759767ca7c5123b03b9e1a42bb105d194cb76400) )
	ROM_LOAD( "a-12", 0x10000, 0x10000, CRC(3c864299) SHA1(cb12616e4d6c53a82beb4cd51510a632894b359c) )
	ROM_LOAD( "a-15", 0x20000, 0x10000, CRC(2f268f37) SHA1(f82cfe3b2001d5ed2a709ca9c51febcf624bb627) )
	ROM_LOAD( "a-14", 0x30000, 0x10000, CRC(aef814c8) SHA1(f6b9229ca7beb9a0e47d1f6a1083c6102fdd20c8) )

	ROM_REGION(0x40000, "adpcm", 0 )    /* ADPCM */
	/* These are same as the 128k sample except nibble-swapped */
	/* Some bootlegs have incorrectly halved the ADPCM data !  Bootleggers wrong data */
	ROM_LOAD ( "a-6.bin",   0x00000, 0x10000, CRC(4da4e935) SHA1(235a1589165a23cfad29e07cf66d7c3a777fc904) )    /* 0x8000, 0x7cd47f01 */
	ROM_LOAD ( "a-7.bin",   0x10000, 0x10000, CRC(6284c254) SHA1(e01be1bd4768ae0ccb1cec65b3a6bc80ed7a4b00) )    /* 0x8000, 0x7091959c */
	ROM_LOAD ( "a-10.bin",  0x20000, 0x10000, CRC(b728ec6e) SHA1(433b5f907e4918e89b79bd927e2993ad3030017b) )    /* 0x8000, 0x78349cb6 */
	ROM_LOAD ( "a-11.bin",  0x30000, 0x10000, CRC(a50d1895) SHA1(0c2c1f8a2e945d6c53ce43413f0e63ced45bae17) )    /* 0x8000, 0xaa5b6834 */

	ROM_REGION(0x300, "user1", 0 )  /* Unknown Bipolar PROMs */
	ROM_LOAD( "23jb-0.16", 0x000, 0x200, CRC(46339529) SHA1(64f4c42a826d67b7cbaa8a23a45ebc4eb6248891) ) /* 82S131 on video board */
	ROM_LOAD( "23j5-0.45", 0x200, 0x100, CRC(fdb130a9) SHA1(4c4f214229b9fab2b5d69c745ec5428787b89e1f) ) /* 82S129 on main board */
ROM_END


DRIVER_INIT_MEMBER(chinagat_state,chinagat)
{
	UINT8 *MAIN = memregion("maincpu")->base();
	UINT8 *SUB = memregion("sub")->base();

	m_technos_video_hw = 1;
	m_sprite_irq = M6809_IRQ_LINE;
	m_sound_irq = INPUT_LINE_NMI;

	membank("bank1")->configure_entries(0, 6, &MAIN[0x10000], 0x4000);
	membank("bank4")->configure_entries(0, 6, &SUB[0x10000], 0x4000);
}


/*   ( YEAR  NAME      PARENT    MACHINE   INPUT     INIT    MONITOR COMPANY    FULLNAME     FLAGS ) */
GAME( 1988, chinagat,   0,        chinagat,   chinagat, chinagat_state, chinagat, ROT0, "Technos Japan (Taito / Romstar license)", "China Gate (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, saiyugou,   chinagat, chinagat,   chinagat, chinagat_state, chinagat, ROT0, "Technos Japan", "Sai Yu Gou Ma Roku (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, saiyugoub1, chinagat, saiyugoub1, chinagat, chinagat_state, chinagat, ROT0, "bootleg", "Sai Yu Gou Ma Roku (Japan bootleg 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1988, saiyugoub2, chinagat, saiyugoub2, chinagat, chinagat_state, chinagat, ROT0, "bootleg", "Sai Yu Gou Ma Roku (Japan bootleg 2)", MACHINE_SUPPORTS_SAVE )
