// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 18 hardware

****************************************************************************

    Known bugs:
        * lghost sprites seem to be slightly out of sync
        * vdp gfx on 2nd attract level of lghost are corrupt at top of stairs
          after attract mode loops
        * pauses in lghost where all sprites vanish
        * some minor gfx bugs in moonwalker (see mametesters)

****************************************************************************

**  MC68000 + Z80
**  2xYM3438 + Custom PCM
**
**  Alien Storm
**  Bloxeed
**  Clutch Hitter
**  D.D. Crew
**  Laser Ghost
**  Michael Jackson's Moonwalker
**  Shadow Dancer
**  Wally wo Sagase! (Where's Wally?)

***************************************************************************/

#include "emu.h"
#include "machine/segaic16.h"
#include "machine/nvram.h"
#include "includes/segas18.h"
#include "sound/2612intf.h"
#include "sound/rf5c68.h"
#include "includes/segaipt.h"

/*************************************
 *
 *  Memory mapping tables
 *
 *************************************/

void segas18_state::memory_mapper(sega_315_5195_mapper_device &mapper, UINT8 index)
{
	UINT32 romsize = m_maincpu->region()->bytes();
	switch (index)
	{
		case 7:
			mapper.map_as_handler(0x00000, 0x04000, 0xffc000, read16_delegate(FUNC(segas18_state::misc_io_r), this), write16_delegate(FUNC(segas18_state::misc_io_w), this));
			break;

		case 6:
			mapper.map_as_ram(0x00000, 0x01000, 0xfff000, "paletteram", write16_delegate(FUNC(segas18_state::paletteram_w), this));
			break;

		case 5:
			mapper.map_as_ram(0x00000, 0x10000, 0xfe0000, "tileram", write16_delegate(FUNC(segas18_state::tileram_w), this));
			mapper.map_as_ram(0x10000, 0x01000, 0xfef000, "textram", write16_delegate(FUNC(segas18_state::textram_w), this));
			break;

		case 4:
			mapper.map_as_ram(0x00000, 0x00800, 0xfff800, "sprites", write16_delegate());
			break;

		case 3:
			mapper.map_as_ram(0x00000, 0x04000, 0xffc000, "workram", write16_delegate());
			break;

		case 2:
			switch (m_romboard)
			{
				case ROM_BOARD_171_SHADOW:  break;  // ???
				case ROM_BOARD_837_7525:
				case ROM_BOARD_171_5874:
				case ROM_BOARD_171_5987:    mapper.map_as_handler(0x00000, 0x00010, 0xfffff0, read16_delegate(FUNC(segas18_state::genesis_vdp_r), this), write16_delegate(FUNC(segas18_state::genesis_vdp_w), this)); break;
				default:                    assert(false);
			}
			break;

		case 1:
			switch (m_romboard)
			{
				case ROM_BOARD_171_SHADOW:  mapper.map_as_handler(0x00000, 0x00010, 0xfffff0, read16_delegate(FUNC(segas18_state::genesis_vdp_r), this), write16_delegate(FUNC(segas18_state::genesis_vdp_w), this)); break;
				case ROM_BOARD_171_5874:    mapper.map_as_rom(0x00000, 0x80000, 0xf80000, "rom1base", "decrypted_rom1base", 0x80000, write16_delegate()); break;
				case ROM_BOARD_171_5987:    if (romsize <= 0x100000)
												mapper.map_as_rom(0x00000, 0x80000, 0xf80000, "rom1base", "decrypted_rom1base", 0x80000, write16_delegate(FUNC(segas18_state::rom_5987_bank_w), this));
											else
												mapper.map_as_rom(0x00000,0x100000, 0xf00000, "rom1base", "decrypted_rom1base",0x100000, write16_delegate(FUNC(segas18_state::rom_5987_bank_w), this));
											break;
				case ROM_BOARD_837_7525:    mapper.map_as_rom(0x00000, 0x80000, 0xf80000, "rom1base", "decrypted_rom1base", 0x80000, write16_delegate(FUNC(segas18_state::rom_837_7525_bank_w), this));
				break;

				default:                    assert(false);
			}
			break;

		case 0:
			switch (m_romboard)
			{
				case ROM_BOARD_171_SHADOW:
				case ROM_BOARD_171_5874:    mapper.map_as_rom(0x00000, 0x80000, 0xf80000, "rom0base", "decrypted_rom0base", 0x00000, write16_delegate()); break;
				case ROM_BOARD_837_7525:
				case ROM_BOARD_171_5987:    if (romsize <= 0x100000)
												mapper.map_as_rom(0x00000, 0x80000, 0xf80000, "rom0base", "decrypted_rom0base", 0x00000, write16_delegate());
											else
												mapper.map_as_rom(0x00000,0x100000, 0xf00000, "rom0base", "decrypted_rom0base", 0x00000, write16_delegate());
											break;
				default:                    assert(false);
			}
			break;
	}
}



/*************************************
 *
 *  Configuration
 *
 *************************************/

UINT8 segas18_state::mapper_sound_r()
{
	return m_mcu_data;
}

void segas18_state::mapper_sound_w(UINT8 data)
{
	soundlatch_write(data & 0xff);
	m_soundcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

void segas18_state::init_generic(segas18_rom_board rom_board)
{
	// set the ROM board
	m_romboard = rom_board;

	// configure the NVRAM to point to our workram
	m_nvram->set_base(m_workram, m_workram.bytes());

	// configure VDP
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(FALSE);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);
	m_vdp->stop_timers(); // 315-5124 timers

	// save state
	save_item(NAME(m_mcu_data));
	save_item(NAME(m_lghost_value));
	save_item(NAME(m_lghost_select));
	save_item(NAME(m_wwally_last_x));
	save_item(NAME(m_wwally_last_y));
}



/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

void segas18_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_INITIAL_BOOST:
			machine().scheduler().boost_interleave(attotime::zero, attotime::from_msec(10));
			break;
	}
}


void segas18_state::machine_reset()
{
	m_segaic16vid->tilemap_reset(*m_screen);

	m_vdp->device_reset_old();

	// if we are running with a real live 8751, we need to boost the interleave at startup
	if (m_mcu != NULL && m_mcu->type() == I8751)
		synchronize(TID_INITIAL_BOOST);
}



/*************************************
 *
 *  I/O space
 *
 *************************************/

WRITE8_MEMBER( segas18_state::misc_outputs_w )
{
	// miscellaneous output
	set_grayscale(~data & 0x40);
	m_segaic16vid->tilemap_set_flip(0, data & 0x20);
	m_sprites->set_flip(data & 0x20);
	// These are correct according to cgfm's docs, but mwalker and ddcrew both
	// enable the lockout and never turn it off
	// coin_lockout_w(machine(), 1, data & 0x08);
	// coin_lockout_w(machine(), 0, data & 0x04);
	coin_counter_w(machine(), 1, data & 0x02);
	coin_counter_w(machine(), 0, data & 0x01);
}


READ16_MEMBER( segas18_state::misc_io_r )
{
	offset &= 0x1fff;
	switch (offset & (0x3000/2))
	{
		// I/O chip
		case 0x0000/2:
		case 0x1000/2:
			return m_io->read(space, offset) | (open_bus_r(space, 0, mem_mask) & 0xff00);

		// video control latch
		case 0x2000/2:
		{
			static const char *const portnames[] = { "SERVICE", "COINAGE" };
			return ioport(portnames[offset & 1])->read();
		}
	}

	if (!m_custom_io_r.isnull())
		return m_custom_io_r(space, offset, mem_mask);
	logerror("%06X:misc_io_r - unknown read access to address %04X\n", space.device().safe_pc(), offset * 2);
	return open_bus_r(space, 0, mem_mask);
}

WRITE16_MEMBER( segas18_state::misc_io_w )
{
	offset &= 0x1fff;
	switch (offset & (0x3000/2))
	{
		// I/O chip
		case 0x0000/2:
		case 0x1000/2:
			if (ACCESSING_BITS_0_7)
			{
				m_io->write(space, offset, data);
				return;
			}
			break;

		// video control latch
		case 0x2000/2:
			if (ACCESSING_BITS_0_7)
			{
				set_vdp_mixing(data & 0xff);
				return;
			}
			break;
	}

	if (!m_custom_io_w.isnull())
	{
		m_custom_io_w(space, offset, data, mem_mask);
		return;
	}
	logerror("%06X:misc_io_w - unknown write access to address %04X = %04X & %04X\n", space.device().safe_pc(), offset * 2, data, mem_mask);
}



/*************************************
 *
 *  Tile banking
 *
 *************************************/

WRITE8_MEMBER( segas18_state::rom_5874_bank_w )
{
	if (m_romboard == ROM_BOARD_171_5874 || m_romboard == ROM_BOARD_171_SHADOW)
	{
		for (int i = 0; i < 4; i++)
		{
			m_segaic16vid->tilemap_set_bank(0, 0 + i, (data & 0xf) * 4 + i);
			m_segaic16vid->tilemap_set_bank(0, 4 + i, ((data >> 4) & 0xf) * 4 + i);
		}
	}
}


WRITE16_MEMBER( segas18_state::rom_5987_bank_w )
{
	if (!ACCESSING_BITS_0_7)
		return;

	offset &= 0xf;
	data &= 0xff;

	// tile banking
	if (offset < 8)
	{
		int maxbanks = m_gfxdecode->gfx(0)->elements() / 1024;
		if (data >= maxbanks)
			data %= maxbanks;
		m_segaic16vid->tilemap_set_bank(0, offset, data);
	}

	// sprite banking
	else
	{
		int maxbanks = memregion("sprites")->bytes() / 0x40000;
		if (data >= maxbanks)
			data = 255;
		m_sprites->set_bank((offset - 8) * 2 + 0, data * 2 + 0);
		m_sprites->set_bank((offset - 8) * 2 + 1, data * 2 + 1);
	}
}

WRITE16_MEMBER( segas18_state::rom_837_7525_bank_w )
{
	if (!ACCESSING_BITS_0_7)
		return;

	offset &= 0xf;
	data &= 0xff;

	// tile banking
	if (offset < 8)
	{
		data &= 0x9f;

		if (data & 0x80) data += 0x20;
		data &= 0x3f;

		m_segaic16vid->tilemap_set_bank(0, offset, data);
	}

	// sprite banking
	else
	{
		//printf("%02x %02x\n", offset, data);
		// not needed?
	}
}


/*************************************
 *
 *  D.D.Crew Custom I/O
 *
 *************************************/

READ16_MEMBER( segas18_state::ddcrew_custom_io_r )
{
	switch (offset)
	{
		case 0x3020/2:
			return ioport("EXP3")->read();

		case 0x3022/2:
			return ioport("EXP4")->read();

		case 0x3024/2:
			return ioport("EXSERVICE")->read();
	}
	return open_bus_r(space, 0, mem_mask);
}



/*************************************
 *
 *  Laser Ghost Custom I/O
 *
 *************************************/

READ16_MEMBER( segas18_state::lghost_custom_io_r )
{
	UINT16 result;
	switch (offset)
	{
		case 0x3010/2:
		case 0x3012/2:
		case 0x3014/2:
		case 0x3016/2:
			result = m_lghost_value | 0x7f;
			m_lghost_value <<= 1;
			return result;
	}
	return open_bus_r(space, 0, mem_mask);
}

WRITE16_MEMBER( segas18_state::lghost_custom_io_w )
{
	switch (offset)
	{
		case 0x3010/2:
			m_lghost_value = 255 - ioport("GUNY1")->read();
			break;

		case 0x3012/2:
			m_lghost_value = ioport("GUNX1")->read();
			break;

		case 0x3014/2:
			m_lghost_value = 255 - ioport(m_lghost_select ? "GUNY3" : "GUNY2")->read();
			break;

		case 0x3016/2:
			m_lghost_value = ioport(m_lghost_select ? "GUNX3" : "GUNX2")->read();
			break;

		case 0x3020/2:
			m_lghost_select = data & 1;
			break;
	}
}


WRITE8_MEMBER( segas18_state::lghost_gun_recoil_w )
{
	output_set_value("P1_Gun_Recoil", (~data & 0x01));
	output_set_value("P2_Gun_Recoil", (~data & 0x02)>>1);
	output_set_value("P3_Gun_Recoil", (~data & 0x04)>>2);
}



/*************************************
 *
 *  Where's Wally Custom I/O
 *
 *************************************/

READ16_MEMBER( segas18_state::wwally_custom_io_r )
{
	switch (offset)
	{
		case 0x3000/2:
			return (ioport("TRACKX1")->read() - m_wwally_last_x[0]) & 0xff;

		case 0x3004/2:
			return (ioport("TRACKY1")->read() - m_wwally_last_y[0]) & 0xff;

		case 0x3008/2:
			return (ioport("TRACKX2")->read() - m_wwally_last_x[1]) & 0xff;

		case 0x300c/2:
			return (ioport("TRACKY2")->read() - m_wwally_last_y[1]) & 0xff;

		case 0x3010/2:
			return (ioport("TRACKX3")->read() - m_wwally_last_x[2]) & 0xff;

		case 0x3014/2:
			return (ioport("TRACKY3")->read() - m_wwally_last_y[2]) & 0xff;
	}
	return open_bus_r(space, 0, mem_mask);
}


WRITE16_MEMBER( segas18_state::wwally_custom_io_w )
{
	switch (offset)
	{
		case 0x3000/2:
		case 0x3004/2:
			m_wwally_last_x[0] = ioport("TRACKX1")->read();
			m_wwally_last_y[0] = ioport("TRACKY1")->read();
			break;

		case 0x3008/2:
		case 0x300c/2:
			m_wwally_last_x[1] = ioport("TRACKX2")->read();
			m_wwally_last_y[1] = ioport("TRACKY2")->read();
			break;

		case 0x3010/2:
		case 0x3014/2:
			m_wwally_last_x[2] = ioport("TRACKX3")->read();
			m_wwally_last_y[2] = ioport("TRACKY3")->read();
			break;
	}
}



/*************************************
 *
 *  Sound handlers
 *
 *************************************/

WRITE8_MEMBER( segas18_state::soundbank_w )
{
	membank("bank1")->set_base(memregion("soundcpu")->base() + 0x10000 + 0x2000 * data);
}


WRITE8_MEMBER( segas18_state::mcu_data_w )
{
	m_mcu_data = data;
	m_mcu->set_input_line(MCS51_INT1_LINE, HOLD_LINE);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system18_map, AS_PROGRAM, 16, segas18_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0xffffff) AM_DEVREADWRITE8("mapper", sega_315_5195_mapper_device, read, write, 0x00ff)

	// these get overwritten by the memory mapper above, but we put them here
	// so they are properly allocated and tracked for saving
	AM_RANGE(0x100000, 0x1007ff) AM_RAM AM_SHARE("sprites")
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_SHARE("paletteram")
	AM_RANGE(0x300000, 0x30ffff) AM_RAM AM_SHARE("tileram")
	AM_RANGE(0x400000, 0x400fff) AM_RAM AM_SHARE("textram")
	AM_RANGE(0x500000, 0x503fff) AM_RAM AM_SHARE("workram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 16, segas18_state )
	AM_RANGE(0x00000, 0xfffff) AM_ROMBANK("fd1094_decrypted_opcodes")
ADDRESS_MAP_END

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, segas18_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x9fff) AM_ROM AM_REGION("soundcpu", 0x10000)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc00f) AM_MIRROR(0x0ff0) AM_DEVWRITE("rfsnd", rf5c68_device, rf5c68_w)
	AM_RANGE(0xd000, 0xdfff) AM_DEVREADWRITE("rfsnd", rf5c68_device, rf5c68_mem_r, rf5c68_mem_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, segas18_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x0c) AM_DEVREADWRITE("ym1", ym3438_device, read, write)
	AM_RANGE(0x90, 0x93) AM_MIRROR(0x0c) AM_DEVREADWRITE("ym2", ym3438_device, read, write)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x1f) AM_WRITE(soundbank_w)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1f) AM_READ(soundlatch_byte_r) AM_WRITE(mcu_data_w)
ADDRESS_MAP_END



/*************************************
 *
 *  i8751 MCU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mcu_io_map, AS_IO, 8, segas18_state )
	ADDRESS_MAP_UNMAP_HIGH
	// port 2 not used for high order address byte
	AM_RANGE(0x0000, 0x001f) AM_MIRROR(0xff00) AM_DEVREADWRITE("mapper", sega_315_5195_mapper_device, read, write)
ADDRESS_MAP_END



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( system18_generic )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINAGE")
	SEGA_COINAGE_LOC(SW1)

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END



/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( astorm )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x04, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Special" )
	PORT_DIPNAME( 0x20, 0x20, "Coin Chutes" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "1" )
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END

static INPUT_PORTS_START( astorm2p )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x04, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Special" )
	PORT_DIPNAME( 0x20, 0x20, "Coin Chutes" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "1" )
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( bloxeed )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Allow VS Mode" )         PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Lines Per Level" )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "High Speed Mode" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( cltchitr )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Time P1" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "2 Credits 18 Outcounts 14 Min." )
	PORT_DIPSETTING(    0x00, "1 Credit 6 Outcounts 7 Min." )
	PORT_DIPSETTING(    0x08, "1 Credit 12 Outcounts 12 Min." )
	PORT_DIPSETTING(    0x0c, "1Credit 6 Outcounts 8M./2Credits 18 Outcounts 14M." )
	PORT_DIPNAME( 0x30, 0x30, "Game Time P2" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "4 Credits 18 Outcounts 16 Min." )
	PORT_DIPSETTING(    0x00, "2 Credits 6 Outcounts 8 Min." )
	PORT_DIPSETTING(    0x20, "2 Credits 12 Outcounts 14 Min." )
	PORT_DIPSETTING(    0x30, "2Credits 6 Outcounts 8M./4Credits 18 Outcounts 16M." )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ddcrew )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits needed" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1 to start, 1 to continue" )
	PORT_DIPSETTING(    0x00, "2 to start, 1 to continue" )
	PORT_DIPNAME( 0x02, 0x02, "Switch to Start" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "Start" )
	PORT_DIPSETTING(    0x00, "Attack" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Chute" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Player Start/Continue" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3/3" )
	PORT_DIPSETTING(    0x20, "2/3" )
	PORT_DIPSETTING(    0x10, "2/2" )
	PORT_DIPSETTING(    0x00, "3/4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("EXP3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("EXP4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)

	PORT_START("EXSERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) // individual mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) // individual mode
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ddcrew2p )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits needed" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1 to start, 1 to continue" )
	PORT_DIPSETTING(    0x00, "2 to start, 1 to continue" )
	PORT_DIPNAME( 0x02, 0x02, "Switch to Start" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "Start" )
	PORT_DIPSETTING(    0x00, "Attack" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Chute" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Player Start/Continue" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3/3" )
	PORT_DIPSETTING(    0x20, "2/3" )
	PORT_DIPSETTING(    0x10, "2/2" )
	PORT_DIPSETTING(    0x00, "3/4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ddcrew3p )
	PORT_INCLUDE( ddcrew2p )

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static INPUT_PORTS_START( desertbr )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3) // individual mode

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:3" unused
	PORT_DIPNAME( 0x08, 0x08, "Play Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "2 players" )
	PORT_DIPSETTING(    0x08, "3 players" )
	PORT_DIPNAME( 0x10, 0x10, "Coin Chute" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x20, 0x20, "Start Button" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Use" )
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( hamaway )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	//"SW2:5" is unknown - Not listed in the service mode
	//"SW2:6" is unknown - Not listed in the service mode
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2 Credits to Start" )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( lghost )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // P2 joystick inputs, unused in lghost
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Extra Hardest" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Coin Chute" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "Common" )
	PORT_DIPSETTING(    0x40, "Individual" )
	//"SW2:8" unused

	PORT_START("GUNX1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("GUNY1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("GUNX2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("GUNY2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("GUNX3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(3)

	PORT_START("GUNY3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(3)
INPUT_PORTS_END


static INPUT_PORTS_START( mwalk )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 ) // individual mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) // individual mode

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x08, 0x00, "Player Vitality" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, "Play Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "2 Players" )
	PORT_DIPSETTING(    0x00, "3 Players" )
	PORT_DIPNAME( 0x20, 0x00, "Coin Chute" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mwalka )
	PORT_INCLUDE( mwalk )

	PORT_MODIFY("DSW")
	//"SW2:1" not divert from "mwalk"
	//"SW2:2" not divert from "mwalk"
	//"SW2:3" not divert from "mwalk"
	//"SW2:4" not divert from "mwalk"
	PORT_DIPNAME( 0x10, 0x10, "Play Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x10, "3 Players" )
	PORT_DIPNAME( 0x20, 0x20, "Coin Chute" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, "Common" )
	PORT_DIPSETTING(    0x20, "Individual" )
	//"SW2:7" not divert from "mwalk"
	//"SW2:8" not divert from "mwalk"
INPUT_PORTS_END


static INPUT_PORTS_START( shdancer )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Time Adjust" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "2.20" )
	PORT_DIPSETTING(    0x40, "2.40" )
	PORT_DIPSETTING(    0xc0, "3.00" )
	PORT_DIPSETTING(    0x80, "3.30" )
INPUT_PORTS_END


static INPUT_PORTS_START( wwally )
	PORT_INCLUDE( system18_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Chute" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	//"SW2:4" unused
	//"SW2:5" unused
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	//"SW2:8" unused

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5)

	PORT_START("TRACKX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("TRACKY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("TRACKX3")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_PLAYER(3) PORT_REVERSE

	PORT_START("TRACKY3")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_PLAYER(3)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( segas18 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1024 )
GFXDECODE_END



// are any of the VDP interrupt lines hooked up to anything?
WRITE_LINE_MEMBER(segas18_state::vdp_sndirqline_callback_s18)
{
}

WRITE_LINE_MEMBER(segas18_state::vdp_lv6irqline_callback_s18)
{
}

WRITE_LINE_MEMBER(segas18_state::vdp_lv4irqline_callback_s18)
{
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

WRITE_LINE_MEMBER(segas18_state::ym3438_irq_handler)
{
	m_soundcpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}


static MACHINE_CONFIG_START( system18, segas18_state )

	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(system18_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", segas18_state, irq4_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80, 8000000)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SEGA_315_5195_MAPPER_ADD("mapper", "maincpu", segas18_state, memory_mapper, mapper_sound_r, mapper_sound_w)

	MCFG_DEVICE_ADD("io", SEGA_315_5296, 16000000)
	MCFG_315_5296_IN_PORTA_CB(IOPORT("P1"))
	MCFG_315_5296_IN_PORTB_CB(IOPORT("P2"))
	MCFG_315_5296_IN_PORTC_CB(IOPORT("P3"))
	MCFG_315_5296_OUT_PORTD_CB(WRITE8(segas18_state, misc_outputs_w))
	MCFG_315_5296_IN_PORTE_CB(IOPORT("SERVICE"))
	MCFG_315_5296_IN_PORTF_CB(IOPORT("COINAGE"))
	MCFG_315_5296_IN_PORTG_CB(IOPORT("DSW"))
	MCFG_315_5296_OUT_PORTH_CB(WRITE8(segas18_state, rom_5874_bank_w))
	MCFG_315_5296_OUT_CNT1_CB(DEVWRITELINE("segaic16vid", segaic16_video_device, set_display_enable))
	MCFG_315_5296_OUT_CNT2_CB(WRITELINE(segas18_state, set_vdp_enable))

	MCFG_DEVICE_ADD("gen_vdp", SEGA315_5313, 0)
	MCFG_SEGA315_5313_IS_PAL(false)
	MCFG_SEGA315_5313_SND_IRQ_CALLBACK(WRITELINE(segas18_state, vdp_sndirqline_callback_s18));
	MCFG_SEGA315_5313_LV6_IRQ_CALLBACK(WRITELINE(segas18_state, vdp_lv6irqline_callback_s18));
	MCFG_SEGA315_5313_LV4_IRQ_CALLBACK(WRITELINE(segas18_state, vdp_lv4irqline_callback_s18));
	MCFG_SEGA315_5313_ALT_TIMING(1);
	MCFG_SEGA315_5313_PAL_WRITE_BASE(0x2000);
	MCFG_SEGA315_5313_PALETTE("palette")

	MCFG_TIMER_DEVICE_ADD_SCANLINE("scantimer", "gen_vdp", sega315_5313_device, megadriv_scanline_timer_callback_alt_timing, "screen", 0, 1)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.23) // verified on pcb
	MCFG_SCREEN_SIZE(342,262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(segas18_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", segas18)
	MCFG_PALETTE_ADD("palette", 2048*3+2048 + 64*3)

	MCFG_SEGA_SYS16B_SPRITES_ADD("sprites")
	MCFG_SEGAIC16VID_ADD("segaic16vid")
	MCFG_SEGAIC16VID_GFXDECODE("gfxdecode")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM3438, 8000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
	MCFG_YM2612_IRQ_HANDLER(WRITELINE(segas18_state, ym3438_irq_handler))

	MCFG_SOUND_ADD("ym2", YM3438, 8000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_RF5C68_ADD("rfsnd", 10000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( system18_fd1094, system18 )

	// basic machine hardware
	MCFG_CPU_REPLACE("maincpu", FD1094, 10000000)
	MCFG_CPU_PROGRAM_MAP(system18_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", segas18_state, irq4_line_hold)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lghost_fd1094, system18_fd1094 )

	// basic machine hardware
	MCFG_DEVICE_MODIFY("io")
	MCFG_315_5296_OUT_PORTC_CB(WRITE8(segas18_state, lghost_gun_recoil_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lghost, system18 )

	// basic machine hardware
	MCFG_DEVICE_MODIFY("io")
	MCFG_315_5296_OUT_PORTC_CB(WRITE8(segas18_state, lghost_gun_recoil_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( system18_i8751, system18 )

	// basic machine hardware
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_VBLANK_INT_REMOVE()

	MCFG_CPU_ADD("mcu", I8751, 8000000)
	MCFG_CPU_IO_MAP(mcu_io_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", segas18_state, irq0_line_hold)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( system18_fd1094_i8751, system18_fd1094 )

	// basic machine hardware
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_VBLANK_INT_REMOVE()

	MCFG_CPU_ADD("mcu", I8751, 8000000)
	MCFG_CPU_IO_MAP(mcu_io_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", segas18_state, irq0_line_hold)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Alien Storm (2 players World version), Sega System 18
    CPU: FD1094 (317-0154)
    ROM Board: 171-5873B
*/
ROM_START( astorm )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13182.a6", 0x000000, 0x40000, CRC(e31f2a1c) SHA1(690ee10c36e5bb6175470fb5564527e0e4a94d2c) )
	ROM_LOAD16_BYTE( "epr-13181.a5", 0x000001, 0x40000, CRC(78cd3b26) SHA1(a81b807c5da625d8e4648ae80c41e4ca3870c0fa) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0154.key", 0x0000, 0x2000, CRC(b86b6b8f) SHA1(869405383d563a3f3842c89462a7b2e184928532) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr-13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr-13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "mpr-13089.bin", 0x000000, 0x40000, CRC(2a4227f0) SHA1(47284dce8f896f8e8eace9c20302842cacb479c1) )
	ROM_LOAD16_BYTE( "mpr-13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr-13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr-13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr-13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr-13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr-13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13083a.bin", 0x010000, 0x20000, CRC(e7528e06) SHA1(1f4e618692c20aeb316d578c5ded12440eb072ab) ) // Also known to have EPR-13083B like astormj
	ROM_LOAD( "epr-13076.bin",  0x090000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr-13077.bin",  0x110000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr-13078.bin",  0x190000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END

/**************************************************************************************************************************
    Alien Storm (3 players World version), Sega System 18
    CPU: FD1094 (317-0148)
    ROM Board: 171-5873B
    Game numbers: 833-7379-02 (main pcb: 834-7381-02, rom pcb: 834-7380-02)
*/
ROM_START( astorm3 )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13165.a6", 0x000000, 0x40000, CRC(6efcd381) SHA1(547c6703a34c3b9b887f5a63ec59a7055067bf3b) )
	ROM_LOAD16_BYTE( "epr-13164.a5", 0x000001, 0x40000, CRC(97d693c6) SHA1(1a9aa98b32aae9367ed897e6931b2633b11b079e) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0148.key", 0x0000, 0x2000, CRC(72e4b64a) SHA1(945580d0cf25691370e9f2056cdffc01331d54ad) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr-13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr-13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "mpr-13089.bin", 0x000000, 0x40000, CRC(2a4227f0) SHA1(47284dce8f896f8e8eace9c20302842cacb479c1) )
	ROM_LOAD16_BYTE( "mpr-13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr-13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr-13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr-13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr-13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr-13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13083.bin", 0x010000, 0x20000, CRC(5df3af20) SHA1(e49105fcfd5bf37d14bd760f6adca5ce2412883d) )
	ROM_LOAD( "epr-13076.bin", 0x090000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr-13077.bin", 0x110000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr-13078.bin", 0x190000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END

ROM_START( astorm3d )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13165.a6", 0x000000, 0x40000, CRC(2a0dbff5) SHA1(c58f999e0362ec88f2596cd8dbb405af64c90209) )
	ROM_LOAD16_BYTE( "bootleg_epr-13164.a5", 0x000001, 0x40000, CRC(f981a183) SHA1(5077a28ece7f2284e6fb98243776a5e64745303c) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr-13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr-13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "mpr-13089.bin", 0x000000, 0x40000, CRC(2a4227f0) SHA1(47284dce8f896f8e8eace9c20302842cacb479c1) )
	ROM_LOAD16_BYTE( "mpr-13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr-13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr-13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr-13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr-13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr-13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13083.bin", 0x010000, 0x20000, CRC(5df3af20) SHA1(e49105fcfd5bf37d14bd760f6adca5ce2412883d) )
	ROM_LOAD( "epr-13076.bin", 0x090000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr-13077.bin", 0x110000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr-13078.bin", 0x190000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END

/**************************************************************************************************************************
    Alien Storm (3 players US version), Sega System 18
    CPU: FD1094 (317-0147)
*/
ROM_START( astormu )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13095.a6", 0x000000, 0x40000, CRC(55d40742) SHA1(c30fcd7da1fe062b1f00275dc8ac79c3c619b719) )
	ROM_LOAD16_BYTE( "epr-13094.a5", 0x000001, 0x40000, CRC(92b305f9) SHA1(d24a1de619d29a8f6ff9dfce455c2c7d6457ddbe) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0147.key", 0x0000, 0x2000, CRC(3fd54ba7) SHA1(2d74f44f2ed779ed2b119b4fc0bc844d90678c74) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr-13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr-13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "mpr-13089.bin", 0x000000, 0x40000, CRC(2a4227f0) SHA1(47284dce8f896f8e8eace9c20302842cacb479c1) )
	ROM_LOAD16_BYTE( "mpr-13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr-13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr-13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr-13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr-13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr-13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13083.bin", 0x010000, 0x20000, CRC(5df3af20) SHA1(e49105fcfd5bf37d14bd760f6adca5ce2412883d) )
	ROM_LOAD( "epr-13076.bin", 0x090000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr-13077.bin", 0x110000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr-13078.bin", 0x190000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END

ROM_START( astormud )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13095.a6", 0x000000, 0x40000, CRC(a29b1eea) SHA1(ff34ab6806a4aea3519cefe06c8b85962bd9942d) )
	ROM_LOAD16_BYTE( "bootleg_epr-13094.a5", 0x000001, 0x40000, CRC(81f4c6b6) SHA1(c2c69b281a90635746c76d285b2a8342151a1cd5) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr-13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr-13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "mpr-13089.bin", 0x000000, 0x40000, CRC(2a4227f0) SHA1(47284dce8f896f8e8eace9c20302842cacb479c1) )
	ROM_LOAD16_BYTE( "mpr-13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr-13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr-13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr-13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr-13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr-13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13083.bin", 0x010000, 0x20000, CRC(5df3af20) SHA1(e49105fcfd5bf37d14bd760f6adca5ce2412883d) )
	ROM_LOAD( "epr-13076.bin", 0x090000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr-13077.bin", 0x110000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr-13078.bin", 0x190000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END


/**************************************************************************************************************************
    Alien Storm, Sega System 18
    CPU: FD1094 (317-0146)
    ROM Board: 171-5873B
*/
ROM_START( astormj )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13085.a6", 0x000000, 0x40000, CRC(15f74e2d) SHA1(30d9d099ec18907edd3d20df312565c3bd5a80de) )
	ROM_LOAD16_BYTE( "epr-13084.a5", 0x000001, 0x40000, CRC(9687b38f) SHA1(cdeb5b4f06ad4ad8ca579392c1ec901487b08e76) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0146.key", 0x0000, 0x2000, CRC(e94991c5) SHA1(c9a8b56e01792654436f24b219d7a92c0852461f) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr-13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr-13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "mpr-13089.bin", 0x000000, 0x40000, CRC(2a4227f0) SHA1(47284dce8f896f8e8eace9c20302842cacb479c1) )
	ROM_LOAD16_BYTE( "mpr-13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr-13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr-13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr-13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr-13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr-13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13083b.bin", 0x010000, 0x20000, CRC(169b4b5f) SHA1(bb3dfc4aff939df5ad04cf3ecb8e7de366097743) )
	ROM_LOAD( "epr-13076.bin",  0x090000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr-13077.bin",  0x110000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr-13078.bin",  0x190000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END

ROM_START( astormjd )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13085.a6", 0x000000, 0x40000, CRC(c0d7f3c2) SHA1(1123f0d4cf25c3ee82a57212d9ffee49667c9f84) )
	ROM_LOAD16_BYTE( "bootleg_epr-13084.a5", 0x000001, 0x40000, CRC(006635f0) SHA1(7b4fce33151c7cfe5a33065a5b33ff678bac933c) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr-13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr-13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "mpr-13089.bin", 0x000000, 0x40000, CRC(2a4227f0) SHA1(47284dce8f896f8e8eace9c20302842cacb479c1) )
	ROM_LOAD16_BYTE( "mpr-13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr-13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr-13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr-13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr-13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr-13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13083b.bin", 0x010000, 0x20000, CRC(169b4b5f) SHA1(bb3dfc4aff939df5ad04cf3ecb8e7de366097743) )
	ROM_LOAD( "epr-13076.bin",  0x090000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr-13077.bin",  0x110000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr-13078.bin",  0x190000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Bloxeed, Sega System 18, 834-7307
    CPU: FD1094 (317-0139)
    ROM Board: 171-5874B
*/
ROM_START( bloxeed )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "epr-12911.a6", 0x000000, 0x20000, CRC(a481581a) SHA1(5ce5a0a082622919d2fe0e7d52ec807b2e2c25a2) )
	ROM_LOAD16_BYTE( "epr-12910.a5", 0x000001, 0x20000, CRC(dd1bc3bf) SHA1(c0d79862a349ea4dac103c17325633c5dd4a93d1) )

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // decryption key
	ROM_LOAD( "317-0139.key", 0x0000, 0x2000, CRC(9aae84cb) SHA1(806515d61ecacb260b2b5e5fe023b494d35ce315) )

	ROM_REGION( 0x30000, "gfx1", 0 ) // tiles
	ROM_LOAD( "opr-12884.b1", 0x00000, 0x10000, CRC(e024aa33) SHA1(d734be240cd05031aaadf9735c0b1b00e8e6d4cb) )
	ROM_LOAD( "opr-12885.b2", 0x10000, 0x10000, CRC(8041b814) SHA1(29fa49ba9a73eed07865a86ea774e2c6a60aed5b) )
	ROM_LOAD( "opr-12886.b3", 0x20000, 0x10000, CRC(de32285e) SHA1(8994dc128d6a23763e5fcfca1868b336d4aa0a21) )

	ROM_REGION( 0x20000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "opr-12891.a11", 0x00001, 0x10000, CRC(90d31a8c) SHA1(1747652a5109ce65add197cf06535f2463a99fdc) )
	ROM_LOAD16_BYTE( "opr-12887.b11", 0x00000, 0x10000, CRC(f0c0f49d) SHA1(7ecd591265165f3149241e2ceb5059faab88360f) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-12888.a4", 0x010000, 0x20000, CRC(6f2fc63c) SHA1(3cce22c8f80013f05b5a2d36c42a61a81e4d6cbd) )
ROM_END

ROM_START( bloxeedd )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bootleg_epr-12911.a6", 0x000000, 0x20000, CRC(2d634453) SHA1(7a17084057c89d581125b7823c04219a3fb599a5))
	ROM_LOAD16_BYTE( "bootleg_epr-12910.a5", 0x000001, 0x20000, CRC(e6d2e9cc) SHA1(3ee269bce104d0da9a2eb0d42a07924f60f4d3a8) )

	ROM_REGION( 0x30000, "gfx1", 0 ) // tiles
	ROM_LOAD( "opr-12884.b1", 0x00000, 0x10000, CRC(e024aa33) SHA1(d734be240cd05031aaadf9735c0b1b00e8e6d4cb) )
	ROM_LOAD( "opr-12885.b2", 0x10000, 0x10000, CRC(8041b814) SHA1(29fa49ba9a73eed07865a86ea774e2c6a60aed5b) )
	ROM_LOAD( "opr-12886.b3", 0x20000, 0x10000, CRC(de32285e) SHA1(8994dc128d6a23763e5fcfca1868b336d4aa0a21) )

	ROM_REGION( 0x20000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "opr-12891.a11", 0x00001, 0x10000, CRC(90d31a8c) SHA1(1747652a5109ce65add197cf06535f2463a99fdc) )
	ROM_LOAD16_BYTE( "opr-12887.b11", 0x00000, 0x10000, CRC(f0c0f49d) SHA1(7ecd591265165f3149241e2ceb5059faab88360f) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-12888.a4", 0x010000, 0x20000, CRC(6f2fc63c) SHA1(3cce22c8f80013f05b5a2d36c42a61a81e4d6cbd) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Clutch Hitter, Sega System 18
    CPU: FD1094 (317-0176)
    ROM Board: 171-5873B

    game No. 833-7916-01 CLUTCH HITTER
    rom  No. 834-7917-01
*/
ROM_START( cltchitr )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13794.a4", 0x00000, 0x40000, CRC(c8d80233) SHA1(69cdbb989f580abbb006820347becf8d233fa773) )
	ROM_LOAD16_BYTE( "epr-13795.a6", 0x00001, 0x40000, CRC(b0b60b67) SHA1(ee3325ddb7461008f556c1696157a766225b9ef5) )
	ROM_LOAD16_BYTE( "epr-13784.a5", 0x80000, 0x40000, CRC(80c8180d) SHA1(80e72ab7d97714009fd31b3d50176af84b0dcdb7) )
	ROM_LOAD16_BYTE( "epr-13786.a7", 0x80001, 0x40000, CRC(3095dac0) SHA1(20edce74b6f2a82a3865613e601a0e212615d0e4) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0176.key", 0x0000, 0x2000, CRC(9b072430) SHA1(3bc1c7a6d71b4351a42d85e68e70715a7659c096) )

	ROM_REGION( 0x180000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13773.c1",  0x000000, 0x80000, CRC(3fc600e5) SHA1(8bec4ecf6a4e4b38d13133960036a5a4800a668e) )
	ROM_LOAD( "mpr-13774.c2",  0x080000, 0x80000, CRC(2411a824) SHA1(0e383ccc4e0830ffb395d5102e2950610c147007) )
	ROM_LOAD( "mpr-13775.c3",  0x100000, 0x80000, CRC(cf527bf6) SHA1(1f9cf1f0e92709f0465dc97ebbdaa715a419f7a7) )

	ROM_REGION16_BE( 0x600000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13779.c10", 0x000001, 0x80000, CRC(c707f416) SHA1(e6a9d89849f7f1c303a3ca29a629f81397945a2d) )
	ROM_LOAD16_BYTE( "mpr-13787.a10", 0x000000, 0x80000, CRC(f05c68c6) SHA1(b6a0535b6c734a0c89fdb6506c32ffe6ab3aa8cd) )
	ROM_LOAD16_BYTE( "mpr-13780.c11", 0x200001, 0x80000, CRC(a4c341e0) SHA1(15a0b5a42b56465a7b7df98968cc2ed177ce6f59) )
	ROM_LOAD16_BYTE( "mpr-13788.a11", 0x200000, 0x80000, CRC(0106fea6) SHA1(e16e2a469ecbbc704021dee6264db30aa0898368) )
	ROM_LOAD16_BYTE( "mpr-13781.c12", 0x400001, 0x80000, CRC(f33b13af) SHA1(d3eb64dcf12d532454bf3cd6c86528c0f11ee316) )
	ROM_LOAD16_BYTE( "mpr-13789.a12", 0x400000, 0x80000, CRC(09ba8835) SHA1(72e83dd1793a7f4b2b881e71f262493e3d4992b3) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13793.c7", 0x010000, 0x80000, CRC(a3d31944) SHA1(44d17aa0598eacfac4b12c8955fd1067ca09abbd) )
	ROM_LOAD( "epr-13792.c6", 0x090000, 0x80000, CRC(808f9695) SHA1(d50677d20083ad56dbf0864db05f76f93a4e9eba) )
	ROM_LOAD( "epr-13791.c5", 0x110000, 0x80000, CRC(35c16d80) SHA1(7ed04600748c5efb63e25f066daa163e9c0d8038) )
ROM_END

ROM_START( cltchitrd )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13794.a4", 0x00000, 0x40000, CRC(b15de2c5) SHA1(06bbd1de3429a9543ebe69895f2c485feaf54187) )
	ROM_LOAD16_BYTE( "bootleg_epr-13795.a6", 0x00001, 0x40000, CRC(8fc7e524) SHA1(8c55cbb952e4b6aef51b9bd417f842284cb76663) )
	ROM_LOAD16_BYTE( "epr-13784.a5", 0x80000, 0x40000, CRC(80c8180d) SHA1(80e72ab7d97714009fd31b3d50176af84b0dcdb7) )
	ROM_LOAD16_BYTE( "epr-13786.a7", 0x80001, 0x40000, CRC(3095dac0) SHA1(20edce74b6f2a82a3865613e601a0e212615d0e4) )

	ROM_REGION( 0x180000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13773.c1",  0x000000, 0x80000, CRC(3fc600e5) SHA1(8bec4ecf6a4e4b38d13133960036a5a4800a668e) )
	ROM_LOAD( "mpr-13774.c2",  0x080000, 0x80000, CRC(2411a824) SHA1(0e383ccc4e0830ffb395d5102e2950610c147007) )
	ROM_LOAD( "mpr-13775.c3",  0x100000, 0x80000, CRC(cf527bf6) SHA1(1f9cf1f0e92709f0465dc97ebbdaa715a419f7a7) )

	ROM_REGION16_BE( 0x600000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13779.c10", 0x000001, 0x80000, CRC(c707f416) SHA1(e6a9d89849f7f1c303a3ca29a629f81397945a2d) )
	ROM_LOAD16_BYTE( "mpr-13787.a10", 0x000000, 0x80000, CRC(f05c68c6) SHA1(b6a0535b6c734a0c89fdb6506c32ffe6ab3aa8cd) )
	ROM_LOAD16_BYTE( "mpr-13780.c11", 0x200001, 0x80000, CRC(a4c341e0) SHA1(15a0b5a42b56465a7b7df98968cc2ed177ce6f59) )
	ROM_LOAD16_BYTE( "mpr-13788.a11", 0x200000, 0x80000, CRC(0106fea6) SHA1(e16e2a469ecbbc704021dee6264db30aa0898368) )
	ROM_LOAD16_BYTE( "mpr-13781.c12", 0x400001, 0x80000, CRC(f33b13af) SHA1(d3eb64dcf12d532454bf3cd6c86528c0f11ee316) )
	ROM_LOAD16_BYTE( "mpr-13789.a12", 0x400000, 0x80000, CRC(09ba8835) SHA1(72e83dd1793a7f4b2b881e71f262493e3d4992b3) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13793.c7", 0x010000, 0x80000, CRC(a3d31944) SHA1(44d17aa0598eacfac4b12c8955fd1067ca09abbd) )
	ROM_LOAD( "epr-13792.c6", 0x090000, 0x80000, CRC(808f9695) SHA1(d50677d20083ad56dbf0864db05f76f93a4e9eba) )
	ROM_LOAD( "epr-13791.c5", 0x110000, 0x80000, CRC(35c16d80) SHA1(7ed04600748c5efb63e25f066daa163e9c0d8038) )
ROM_END

/**************************************************************************************************************************
    Clutch Hitter, Sega System 18
    CPU: FD1094 (317-0175)
    ROM Board: 171-????
*/
ROM_START( cltchitrj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13783.a4", 0x00000, 0x40000, CRC(e2a1d5af) SHA1(cb6710fe2093889d5d159eaf55a3bd95c6f2ef87) )
	ROM_LOAD16_BYTE( "epr-13796.a6", 0x00001, 0x40000, CRC(06001c67) SHA1(3aa48631013e6dc55e4c1d222b465e6b41ece36b) )
	ROM_LOAD16_BYTE( "epr-13785.a5", 0x80000, 0x40000, CRC(09714762) SHA1(c75c88b1c313e172fdb7f9a570d57be38f959b2b) )
	ROM_LOAD16_BYTE( "epr-13797.a7", 0x80001, 0x40000, CRC(361ade9f) SHA1(a7fd48c55695fd322d0456ff7dc2d2b2bc3e561b) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0175.key", 0x0000, 0x2000, CRC(70d9d283) SHA1(ff309b2a221d9a03ccf301a208c76a7c2eaea790) )

	ROM_REGION( 0x180000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13773.c1",  0x000000, 0x80000, CRC(3fc600e5) SHA1(8bec4ecf6a4e4b38d13133960036a5a4800a668e) )
	ROM_LOAD( "mpr-13774.c2",  0x080000, 0x80000, CRC(2411a824) SHA1(0e383ccc4e0830ffb395d5102e2950610c147007) )
	ROM_LOAD( "mpr-13775.c3",  0x100000, 0x80000, CRC(cf527bf6) SHA1(1f9cf1f0e92709f0465dc97ebbdaa715a419f7a7) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13779.c10", 0x000001, 0x80000, CRC(c707f416) SHA1(e6a9d89849f7f1c303a3ca29a629f81397945a2d) )
	ROM_LOAD16_BYTE( "mpr-13787.a10", 0x000000, 0x80000, CRC(f05c68c6) SHA1(b6a0535b6c734a0c89fdb6506c32ffe6ab3aa8cd) )
	ROM_LOAD16_BYTE( "mpr-13780.c11", 0x200001, 0x80000, CRC(a4c341e0) SHA1(15a0b5a42b56465a7b7df98968cc2ed177ce6f59) )
	ROM_LOAD16_BYTE( "mpr-13788.a11", 0x200000, 0x80000, CRC(0106fea6) SHA1(e16e2a469ecbbc704021dee6264db30aa0898368) )
	ROM_LOAD16_BYTE( "mpr-13781.c12", 0x400001, 0x80000, CRC(f33b13af) SHA1(d3eb64dcf12d532454bf3cd6c86528c0f11ee316) )
	ROM_LOAD16_BYTE( "mpr-13789.a12", 0x400000, 0x80000, CRC(09ba8835) SHA1(72e83dd1793a7f4b2b881e71f262493e3d4992b3) )
	// extra gfx roms??*/
	ROM_LOAD16_BYTE( "epr-13782.c13", 0x600001, 0x40000, CRC(73790852) SHA1(891345cb8ce4b04bd293ee9bac9b1b9940d6bcb2) )
	ROM_LOAD16_BYTE( "epr-13790.a13", 0x600000, 0x40000, CRC(23849101) SHA1(1aeb0fefb6688dfd841bd7d0b17ffdfce69f1dd9) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	// another copy in different set is epr-13778.b7 - 9c54c038
	ROM_LOAD( "epr-13778.c7", 0x010000, 0x20000, CRC(35e86146) SHA1(9be82165dc12d5f32ef26f37ea02b29e3621893f) )
	ROM_LOAD( "epr-13777.c6", 0x090000, 0x80000, CRC(d1524782) SHA1(121c5804927ed686ea50d5d81d0226e041f50f6f) )
	ROM_LOAD( "epr-13776.c5", 0x110000, 0x80000, CRC(282ac9fe) SHA1(4f54d93779c35c036d7c85fce6736df80f3bbe33) )
ROM_END

ROM_START( cltchitrjd )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13783.a4", 0x00000, 0x40000, CRC(dafd48af) SHA1(5be2d1f4c75426f3d41cbb43d63b1eceae8c3d57) )
	ROM_LOAD16_BYTE( "bootleg_epr-13796.a6", 0x00001, 0x40000, CRC(219aae33) SHA1(2dc9c32cfd82abc9f4855459ea427f05a299d596) )
	ROM_LOAD16_BYTE( "epr-13785.a5", 0x80000, 0x40000, CRC(09714762) SHA1(c75c88b1c313e172fdb7f9a570d57be38f959b2b) )
	ROM_LOAD16_BYTE( "epr-13797.a7", 0x80001, 0x40000, CRC(361ade9f) SHA1(a7fd48c55695fd322d0456ff7dc2d2b2bc3e561b) )

	ROM_REGION( 0x180000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13773.c1",  0x000000, 0x80000, CRC(3fc600e5) SHA1(8bec4ecf6a4e4b38d13133960036a5a4800a668e) )
	ROM_LOAD( "mpr-13774.c2",  0x080000, 0x80000, CRC(2411a824) SHA1(0e383ccc4e0830ffb395d5102e2950610c147007) )
	ROM_LOAD( "mpr-13775.c3",  0x100000, 0x80000, CRC(cf527bf6) SHA1(1f9cf1f0e92709f0465dc97ebbdaa715a419f7a7) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13779.c10", 0x000001, 0x80000, CRC(c707f416) SHA1(e6a9d89849f7f1c303a3ca29a629f81397945a2d) )
	ROM_LOAD16_BYTE( "mpr-13787.a10", 0x000000, 0x80000, CRC(f05c68c6) SHA1(b6a0535b6c734a0c89fdb6506c32ffe6ab3aa8cd) )
	ROM_LOAD16_BYTE( "mpr-13780.c11", 0x200001, 0x80000, CRC(a4c341e0) SHA1(15a0b5a42b56465a7b7df98968cc2ed177ce6f59) )
	ROM_LOAD16_BYTE( "mpr-13788.a11", 0x200000, 0x80000, CRC(0106fea6) SHA1(e16e2a469ecbbc704021dee6264db30aa0898368) )
	ROM_LOAD16_BYTE( "mpr-13781.c12", 0x400001, 0x80000, CRC(f33b13af) SHA1(d3eb64dcf12d532454bf3cd6c86528c0f11ee316) )
	ROM_LOAD16_BYTE( "mpr-13789.a12", 0x400000, 0x80000, CRC(09ba8835) SHA1(72e83dd1793a7f4b2b881e71f262493e3d4992b3) )
	// extra gfx roms??*/
	ROM_LOAD16_BYTE( "epr-13782.c13", 0x600001, 0x40000, CRC(73790852) SHA1(891345cb8ce4b04bd293ee9bac9b1b9940d6bcb2) )
	ROM_LOAD16_BYTE( "epr-13790.a13", 0x600000, 0x40000, CRC(23849101) SHA1(1aeb0fefb6688dfd841bd7d0b17ffdfce69f1dd9) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	// another copy in different set is epr-13778.b7 - 9c54c038
	ROM_LOAD( "epr-13778.c7", 0x010000, 0x20000, CRC(35e86146) SHA1(9be82165dc12d5f32ef26f37ea02b29e3621893f) )
	ROM_LOAD( "epr-13777.c6", 0x090000, 0x80000, CRC(d1524782) SHA1(121c5804927ed686ea50d5d81d0226e041f50f6f) )
	ROM_LOAD( "epr-13776.c5", 0x110000, 0x80000, CRC(282ac9fe) SHA1(4f54d93779c35c036d7c85fce6736df80f3bbe33) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    D.D. Crew, Sega System 18
    CPU: FD1094 (317-0190)
    ROM Board: 171-5873B
*/
ROM_START( ddcrew )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-14160.a4", 0x00000, 0x40000, CRC(b9f897b7) SHA1(65cee6c8006f328eee648e144e11fa60b1433ff5) )
	ROM_LOAD16_BYTE( "epr-14161.a6", 0x00001, 0x40000, CRC(bb03c1f0) SHA1(9e7fbd2cda448992c6cbf4b96078b57305def097) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // decryption key
	ROM_LOAD( "317-0190.key", 0x0000, 0x2000, CRC(2d502b11) SHA1(c4e94da59b0e15a5a302ebe88988d1657e7e9814 ) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

ROM_START( ddcrewd )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-14160.a4", 0x00000, 0x40000, CRC(22a6c8e3) SHA1(a4ed9b776b1c61fb6ac158534084152b364d214d) )
	ROM_LOAD16_BYTE( "bootleg_epr-14161.a6", 0x00001, 0x40000, CRC(d9eaba00) SHA1(39d88d7ea22dce3456185aa2c4f2440c798a42d8) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

/**************************************************************************************************************************
    D.D. Crew, Sega System 18
    CPU: FD1094 (317-0186)
    ROM Board: 171-5873B
*/
ROM_START( ddcrewu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-14152.a4", 0x00000, 0x40000, CRC(69c7b571) SHA1(9fe4815a1cff0a46a754a6bdee12abaf7beb6501) )
	ROM_LOAD16_BYTE( "epr-14153.a6", 0x00001, 0x40000, CRC(e01fae0c) SHA1(7166f955324f73e94d8ae6d2a5b2f4b497e62933) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0186.key", 0x0000, 0x2000, CRC(7acaf1fd) SHA1(236d6382072adda8f7907d98d428fcca36700fa0) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

ROM_START( ddcrewud )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-14152.a4", 0x00000, 0x40000, CRC(41b782d0) SHA1(b84ebbe65b9b2aed0599a3309ff5afbdd40e3c13) )
	ROM_LOAD16_BYTE( "bootleg_epr-14153.a6", 0x00001, 0x40000, CRC(48070486) SHA1(2cac94337eff256b05d614a960dc2c9cd707fe28) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

/**************************************************************************************************************************
    D.D. Crew, Sega System 18
    CPU: FD1094 (317-0184)
    ROM Board: 171-5873B
*/
ROM_START( ddcrew2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-14148.a4", 0x00000, 0x40000, CRC(df4cb0cf) SHA1(153993997e9ceb06a9d4c73794ea66d0c585a291) )
	ROM_LOAD16_BYTE( "epr-14149.a6", 0x00001, 0x40000, CRC(380ff818) SHA1(7c7dd7aa825665f1a9aaebb5af4ecf5dd109b0ca) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // decryption key
	ROM_LOAD( "317-0184.key", 0x0000, 0x2000, CRC(cee06254) SHA1(d64903055fdefb49c584cbcd84f0d4fa811bd789) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

ROM_START( ddcrew2d )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-14148.a4", 0x00000, 0x40000, CRC(f3ddb3b9) SHA1(493ad0330362e7244927ee36587fab9634925ee1) )
	ROM_LOAD16_BYTE( "bootleg_epr-14149.a6", 0x00001, 0x40000, CRC(98a83ee1) SHA1(33e37becccf63d6005e901d38418c003834e469f) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END


/**************************************************************************************************************************
    D.D. Crew, Sega System 18
    CPU: FD1094 (317-0187)
    PCB Board: 171-5873-02B (833-8165-05)
    Rom Board : 171-5987A (834-8166-05)
*/
ROM_START( ddcrew1 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-14154.a4", 0x00000, 0x40000, CRC(9a0dadf0) SHA1(b55c8cdd3158607ec8203bfebc9e7aba24d6d565) )
	ROM_LOAD16_BYTE( "epr-14155.a6", 0x00001, 0x40000, CRC(e74362f4) SHA1(a6f96d714baeb826221b712b996e99831cf25abf) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0187.key", 0x0000, 0x2000, CRC(1dfb60be) SHA1(7bd42a2e54fca574076e5ed164ab4e0cbb645a4f) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

ROM_START( ddcrew1d )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-14154.a4", 0x00000, 0x40000, CRC(95749c77) SHA1(fdb924e0bb104326ba2e72e76b91d1fcc500aac0) )
	ROM_LOAD16_BYTE( "bootleg_epr-14155.a6", 0x00001, 0x40000, CRC(bef3c932) SHA1(f74f114aedafdace6ec51c4d3fa80a1b411b1543) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END


/**************************************************************************************************************************
    D.D. Crew, Sega System 18
    CPU: FD1094 (317-0185)
    ROM Board: 171-5873B (834-8166-03)
*/
ROM_START( ddcrewj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-14150.a4", 0x00000, 0x40000, CRC(fc866b19) SHA1(1e59d23c25ac5e34c8982cb21e09d52cccc7aca7) )
	ROM_LOAD16_BYTE( "epr-14151.a6", 0x00001, 0x40000, CRC(46d23fe4) SHA1(926136db7a7d6d3bcdc552156e8cb4cb4224c2e5) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0185.key", 0x0000, 0x2000, CRC(a650b506) SHA1(05ab361114f26576d69c3477943b42f7957fb879) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END


ROM_START( ddcrewjd )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-14150.a4", 0x00000, 0x40000, CRC(1f267de5) SHA1(24b5683c427ea8a56906d8246807e6be2ec8dbe6) )
	ROM_LOAD16_BYTE( "bootleg_epr-14151.a6", 0x00001, 0x40000, CRC(35074062) SHA1(a9b657a550c652a62096d396f7eccf8a0888b482))
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

/**************************************************************************************************************************
    D.D. Crew, Sega System 18
    CPU: FD1094 (317-0182 for 2P version, 317-0185 for 4P version, 317-0188 for 3P version)
    ROM Board: 171-5987A
    (4th Player on Sega System 18,24) I/O Board: 837-7968
*/
ROM_START( ddcrewj2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-14138.a4", 0x00000, 0x40000, CRC(df280b1b) SHA1(581e8b6cbf3231d772de6b0e26b94541931215fd) )
	ROM_LOAD16_BYTE( "epr-14140.a6", 0x00001, 0x40000, CRC(48f223ee) SHA1(5192b92d081829d2a4fcf2258675b24c94cfb4e5) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0182.key", 0x0000, 0x2000, CRC(2e8a3601) SHA1(8b6e10babfd2398c1669ba6bf9aad61cd02f23ba) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

ROM_START( ddcrewj2d )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-14138.a4", 0x00000, 0x40000, CRC(1a586bb2) SHA1(897f7b73a5099752980fe81a190cfac959481c1e) )
	ROM_LOAD16_BYTE( "bootleg_epr-14140.a6", 0x00001, 0x40000, CRC(43e030ae) SHA1(4c145709a726b2180801f65a0d68022ff6f0c619) )
	ROM_LOAD16_BYTE( "mpr-14139.a5", 0x80000, 0x40000, CRC(06c31531) SHA1(d084cb72bf83578b34e959bb60a0695faf4161f8) )
	ROM_LOAD16_BYTE( "mpr-14141.a7", 0x80001, 0x40000, CRC(080a494b) SHA1(64522dccbf6ed856ab80aa185454183df87d7ae9) )


	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-14127.c1", 0x00000, 0x40000, CRC(2228cd88) SHA1(5774bb6a401c3da05c5f3c9d3996b20bb3713cb2) )
	ROM_LOAD( "epr-14128.c2", 0x40000, 0x40000, CRC(edba8e10) SHA1(25a2833ead4ca363802ddc2eb97c40976502921a) )
	ROM_LOAD( "epr-14129.c3", 0x80000, 0x40000, CRC(e8ecc305) SHA1(a26d0c5c7826cd315f8b2c27e5a503a2a7b535c4) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14134.c10", 0x000001, 0x80000, CRC(4fda6a4b) SHA1(a9e582e494ab967e8f3ccf4d5844bb8ef889928c) )
	ROM_LOAD16_BYTE( "mpr-14142.a10", 0x000000, 0x80000, CRC(3cbf1f2a) SHA1(80b6b006936740087786acd538e28aca85fa6894) )
	ROM_LOAD16_BYTE( "mpr-14135.c11", 0x200001, 0x80000, CRC(e9c74876) SHA1(aff9d071e77f01c6937188bf67be38fa898343e6) )
	ROM_LOAD16_BYTE( "mpr-14143.a11", 0x200000, 0x80000, CRC(59022c31) SHA1(5e1409fe0f29284dc6a3ffacf69b761aae09f132) )
	ROM_LOAD16_BYTE( "mpr-14136.c12", 0x400001, 0x80000, CRC(720d9858) SHA1(8ebcb8b3e9555ca48b28908d47dcbbd654398b6f) )
	ROM_LOAD16_BYTE( "mpr-14144.a12", 0x400000, 0x80000, CRC(7775fdd4) SHA1(a03cac039b400b651a4bf2167a8f2338f488ce26) )
	ROM_LOAD16_BYTE( "epr-14137.c13", 0x600001, 0x80000, CRC(846c4265) SHA1(58d0c213d085fb4dee18b7aefb05087d9d522950) )
	ROM_LOAD16_BYTE( "epr-14145.a13", 0x600000, 0x80000, CRC(0e76c797) SHA1(9a44dc948e84e5acac36e80105c2349ee78e6cfa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14133.c7", 0x010000, 0x20000, CRC(cff96665) SHA1(b4dc7f1a03415ebebdb99a82ae89328c345e7678) )
	ROM_LOAD( "mpr-14132.c6", 0x090000, 0x80000, CRC(1fae0220) SHA1(8414c74318ea915816c6b67801ac7c8c3fc905f9) )
	ROM_LOAD( "mpr-14131.c5", 0x110000, 0x80000, CRC(be5a7d0b) SHA1(c2c598b0cf711273fdd568f3401375e9772c1d61) )
	ROM_LOAD( "epr-14130.c4", 0x190000, 0x80000, CRC(948f34a1) SHA1(d4c6728d5eea06cee6ac15a34ec8cccb4cc4b982) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Desert Breaker, Sega System 18

    game No. 833-8830-02
    pcb  No. 837-8832-02 (171-5873-02b)
    rom  No. 834-8831-02 (171-5987a)
    CPU Hiatchi FD1094 317-0196
*/
ROM_START( desertbr )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 code - custom CPU 317-0196
	ROM_LOAD16_BYTE( "epr-14802.a4", 0x000000, 0x80000, CRC(9ab93cbc) SHA1(a8d0013e17519c26c6ba7d28ec73e22ea5bde0e9) )
	ROM_LOAD16_BYTE( "epr-14902.a6", 0x000001, 0x80000, CRC(6724e7b1) SHA1(540c8bb7e848488dead81ca58f3bece45a87e611) )
	ROM_LOAD16_BYTE( "epr-14793.a5", 0x100000, 0x80000, CRC(dc9d7af3) SHA1(1fc1fedc1a4beed94cece268d0bb4bf62eeb407c) )
	ROM_LOAD16_BYTE( "epr-14795.a7", 0x100001, 0x80000, CRC(7e5bf7d9) SHA1(32ac68ee423a34e0f1bedc8765e03f40e01c3af1) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0196.key", 0x0000, 0x2000, CRC(cb942262) SHA1(7ad7cd3df887c6e6435d74784cb12ce016acd0da) )

	ROM_REGION( 0x300000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-14781.c1", 0x000000, 0x100000, CRC(c4f7d7aa) SHA1(3c69dd7a26efccd7111ef33dfa6e8b738095c0bf) )
	ROM_LOAD( "mpr-14782.c2", 0x100000, 0x100000, CRC(ccc98d05) SHA1(b89594bbfff45e3b4fe433aeeaf8b4073c2cabb5) )
	ROM_LOAD( "mpr-14783.c3", 0x200000, 0x100000, CRC(ef202bec) SHA1(b557092f8a3e1c9889d34588344c6dd2c6f06731) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14788.c10", 0x000001, 0x100000, CRC(b5b05536) SHA1(f8fde7ebca38c0a6f6a864c17771aa155e6fc30d) )
	ROM_LOAD16_BYTE( "mpr-14796.a10", 0x000000, 0x100000, CRC(c033220a) SHA1(279d3ef62b41d2c6a18ce1217a549402a874638b) )
	ROM_LOAD16_BYTE( "mpr-14789.c11", 0x200001, 0x100000, CRC(0f9bcb97) SHA1(c15ab3ece596c54e1c4d8e8a755473609334e8ba) )
	ROM_LOAD16_BYTE( "mpr-14797.a11", 0x200000, 0x100000, CRC(4c301cc9) SHA1(8aea8af0b078b81d1054331b273568b1b903531b) )
	ROM_LOAD16_BYTE( "mpr-14790.c12", 0x400001, 0x100000, CRC(6a07ac27) SHA1(0558b662c7965c5b74cbc552423194a8facbc092) )
	ROM_LOAD16_BYTE( "mpr-14798.a12", 0x400000, 0x100000, CRC(50634625) SHA1(f4baaebdb1f850e92ca865e103fbca68cdb0de0f) )
	ROM_LOAD16_BYTE( "mpr-14791.c13", 0x600001, 0x100000, CRC(a4ae352b) SHA1(dc814e1c2e167e191cd43fa554ff8ee974d47152) )
	ROM_LOAD16_BYTE( "mpr-14799.a13", 0x600000, 0x100000, CRC(aeb7b025) SHA1(9ce2a9a46176a110c8d2e77deb3d8b9b69b902fa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14787.c7", 0x010000, 0x40000, CRC(cc6feec7) SHA1(31cc243178b98681a52500a485d74ed9e1274888) )
	ROM_LOAD( "mpr-14786.c6", 0x090000, 0x80000, CRC(cc8349f2) SHA1(9f00d8ea372b70ba44a90dab497deadcc5be3dab) )
	ROM_LOAD( "mpr-14785.c5", 0x110000, 0x80000, CRC(7babba13) SHA1(190cd9ea0f73272e0df34bbdfd8e0035f9e9b0b0) )
	ROM_LOAD( "mpr-14784.c4", 0x190000, 0x80000, CRC(073878e4) SHA1(eff08080d06a16fccf4876e42b389fef599cceba) )
ROM_END

ROM_START( desertbrd )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-14802.a4", 0x000000, 0x80000, CRC(af74af1c) SHA1(42048fc8042816886a5f7e05f195ee9ee9af9c04) )
	ROM_LOAD16_BYTE( "bootleg_epr-14902.a6", 0x000001, 0x80000, CRC(c883404b) SHA1(e11bb91fc1280158d3b524d598ee87d72e6efe03) )
	ROM_LOAD16_BYTE( "epr-14793.a5", 0x100000, 0x80000, CRC(dc9d7af3) SHA1(1fc1fedc1a4beed94cece268d0bb4bf62eeb407c) )
	ROM_LOAD16_BYTE( "epr-14795.a7", 0x100001, 0x80000, CRC(7e5bf7d9) SHA1(32ac68ee423a34e0f1bedc8765e03f40e01c3af1) )

	ROM_REGION( 0x300000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-14781.c1", 0x000000, 0x100000, CRC(c4f7d7aa) SHA1(3c69dd7a26efccd7111ef33dfa6e8b738095c0bf) )
	ROM_LOAD( "mpr-14782.c2", 0x100000, 0x100000, CRC(ccc98d05) SHA1(b89594bbfff45e3b4fe433aeeaf8b4073c2cabb5) )
	ROM_LOAD( "mpr-14783.c3", 0x200000, 0x100000, CRC(ef202bec) SHA1(b557092f8a3e1c9889d34588344c6dd2c6f06731) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14788.c10", 0x000001, 0x100000, CRC(b5b05536) SHA1(f8fde7ebca38c0a6f6a864c17771aa155e6fc30d) )
	ROM_LOAD16_BYTE( "mpr-14796.a10", 0x000000, 0x100000, CRC(c033220a) SHA1(279d3ef62b41d2c6a18ce1217a549402a874638b) )
	ROM_LOAD16_BYTE( "mpr-14789.c11", 0x200001, 0x100000, CRC(0f9bcb97) SHA1(c15ab3ece596c54e1c4d8e8a755473609334e8ba) )
	ROM_LOAD16_BYTE( "mpr-14797.a11", 0x200000, 0x100000, CRC(4c301cc9) SHA1(8aea8af0b078b81d1054331b273568b1b903531b) )
	ROM_LOAD16_BYTE( "mpr-14790.c12", 0x400001, 0x100000, CRC(6a07ac27) SHA1(0558b662c7965c5b74cbc552423194a8facbc092) )
	ROM_LOAD16_BYTE( "mpr-14798.a12", 0x400000, 0x100000, CRC(50634625) SHA1(f4baaebdb1f850e92ca865e103fbca68cdb0de0f) )
	ROM_LOAD16_BYTE( "mpr-14791.c13", 0x600001, 0x100000, CRC(a4ae352b) SHA1(dc814e1c2e167e191cd43fa554ff8ee974d47152) )
	ROM_LOAD16_BYTE( "mpr-14799.a13", 0x600000, 0x100000, CRC(aeb7b025) SHA1(9ce2a9a46176a110c8d2e77deb3d8b9b69b902fa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14787.c7", 0x010000, 0x40000, CRC(cc6feec7) SHA1(31cc243178b98681a52500a485d74ed9e1274888) )
	ROM_LOAD( "mpr-14786.c6", 0x090000, 0x80000, CRC(cc8349f2) SHA1(9f00d8ea372b70ba44a90dab497deadcc5be3dab) )
	ROM_LOAD( "mpr-14785.c5", 0x110000, 0x80000, CRC(7babba13) SHA1(190cd9ea0f73272e0df34bbdfd8e0035f9e9b0b0) )
	ROM_LOAD( "mpr-14784.c4", 0x190000, 0x80000, CRC(073878e4) SHA1(eff08080d06a16fccf4876e42b389fef599cceba) )
ROM_END


/**************************************************************************************************************************
    Desert Breaker, Sega System 18

    game No. 833-8830?
    pcb  No. 837-8832? (171-5873B)
    rom  No. 834-8831 (171-5987A)
    CPU Hiatchi FD1094 317-0194
*/
ROM_START( desertbrj )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 code - custom CPU 317-0196
	ROM_LOAD16_BYTE( "epr-14792.a4", 0x000000, 0x80000, CRC(453d9d02) SHA1(df5d85c2e1a25c18860aa96462ea4893bb633190) )
	ROM_LOAD16_BYTE( "epr-14794.a6", 0x000001, 0x80000, CRC(4426758f) SHA1(d8bebcf05a83d34a93f74e2263f4290e995656ba) )
	ROM_LOAD16_BYTE( "epr-14793.a5", 0x100000, 0x80000, CRC(dc9d7af3) SHA1(1fc1fedc1a4beed94cece268d0bb4bf62eeb407c) )
	ROM_LOAD16_BYTE( "epr-14795.a7", 0x100001, 0x80000, CRC(7e5bf7d9) SHA1(32ac68ee423a34e0f1bedc8765e03f40e01c3af1) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0194.key", 0x0000, 0x2000, CRC(40cbc4cb) SHA1(51777d8a619268ac0b1fda6e7781cde753354419) )

	ROM_REGION( 0x300000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-14781.c1", 0x000000, 0x100000, CRC(c4f7d7aa) SHA1(3c69dd7a26efccd7111ef33dfa6e8b738095c0bf) )
	ROM_LOAD( "mpr-14782.c2", 0x100000, 0x100000, CRC(ccc98d05) SHA1(b89594bbfff45e3b4fe433aeeaf8b4073c2cabb5) )
	ROM_LOAD( "mpr-14783.c3", 0x200000, 0x100000, CRC(ef202bec) SHA1(b557092f8a3e1c9889d34588344c6dd2c6f06731) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14788.c10", 0x000001, 0x100000, CRC(b5b05536) SHA1(f8fde7ebca38c0a6f6a864c17771aa155e6fc30d) )
	ROM_LOAD16_BYTE( "mpr-14796.a10", 0x000000, 0x100000, CRC(c033220a) SHA1(279d3ef62b41d2c6a18ce1217a549402a874638b) )
	ROM_LOAD16_BYTE( "mpr-14789.c11", 0x200001, 0x100000, CRC(0f9bcb97) SHA1(c15ab3ece596c54e1c4d8e8a755473609334e8ba) )
	ROM_LOAD16_BYTE( "mpr-14797.a11", 0x200000, 0x100000, CRC(4c301cc9) SHA1(8aea8af0b078b81d1054331b273568b1b903531b) )
	ROM_LOAD16_BYTE( "mpr-14790.c12", 0x400001, 0x100000, CRC(6a07ac27) SHA1(0558b662c7965c5b74cbc552423194a8facbc092) )
	ROM_LOAD16_BYTE( "mpr-14798.a12", 0x400000, 0x100000, CRC(50634625) SHA1(f4baaebdb1f850e92ca865e103fbca68cdb0de0f) )
	ROM_LOAD16_BYTE( "mpr-14791.c13", 0x600001, 0x100000, CRC(a4ae352b) SHA1(dc814e1c2e167e191cd43fa554ff8ee974d47152) )
	ROM_LOAD16_BYTE( "mpr-14799.a13", 0x600000, 0x100000, CRC(aeb7b025) SHA1(9ce2a9a46176a110c8d2e77deb3d8b9b69b902fa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14787.c7", 0x010000, 0x40000, CRC(cc6feec7) SHA1(31cc243178b98681a52500a485d74ed9e1274888) )
	ROM_LOAD( "mpr-14786.c6", 0x090000, 0x80000, CRC(cc8349f2) SHA1(9f00d8ea372b70ba44a90dab497deadcc5be3dab) )
	ROM_LOAD( "mpr-14785.c5", 0x110000, 0x80000, CRC(7babba13) SHA1(190cd9ea0f73272e0df34bbdfd8e0035f9e9b0b0) )
	ROM_LOAD( "mpr-14784.c4", 0x190000, 0x80000, CRC(073878e4) SHA1(eff08080d06a16fccf4876e42b389fef599cceba) )
ROM_END


ROM_START( desertbrjd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bootleg_epr-14792.a4", 0x000000, 0x80000, CRC(fd8ed26d) SHA1(db5a9a68a05d03db7a1fe60a281310a0d38a86ed) )
	ROM_LOAD16_BYTE( "bootleg_epr-14794.a6", 0x000001, 0x80000, CRC(399f167d) SHA1(ee2d8cb1f6725b38f038e0021317e11b79ce81f0) )
	ROM_LOAD16_BYTE( "epr-14793.a5", 0x100000, 0x80000, CRC(dc9d7af3) SHA1(1fc1fedc1a4beed94cece268d0bb4bf62eeb407c) )
	ROM_LOAD16_BYTE( "epr-14795.a7", 0x100001, 0x80000, CRC(7e5bf7d9) SHA1(32ac68ee423a34e0f1bedc8765e03f40e01c3af1) )

	ROM_REGION( 0x300000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-14781.c1", 0x000000, 0x100000, CRC(c4f7d7aa) SHA1(3c69dd7a26efccd7111ef33dfa6e8b738095c0bf) )
	ROM_LOAD( "mpr-14782.c2", 0x100000, 0x100000, CRC(ccc98d05) SHA1(b89594bbfff45e3b4fe433aeeaf8b4073c2cabb5) )
	ROM_LOAD( "mpr-14783.c3", 0x200000, 0x100000, CRC(ef202bec) SHA1(b557092f8a3e1c9889d34588344c6dd2c6f06731) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14788.c10", 0x000001, 0x100000, CRC(b5b05536) SHA1(f8fde7ebca38c0a6f6a864c17771aa155e6fc30d) )
	ROM_LOAD16_BYTE( "mpr-14796.a10", 0x000000, 0x100000, CRC(c033220a) SHA1(279d3ef62b41d2c6a18ce1217a549402a874638b) )
	ROM_LOAD16_BYTE( "mpr-14789.c11", 0x200001, 0x100000, CRC(0f9bcb97) SHA1(c15ab3ece596c54e1c4d8e8a755473609334e8ba) )
	ROM_LOAD16_BYTE( "mpr-14797.a11", 0x200000, 0x100000, CRC(4c301cc9) SHA1(8aea8af0b078b81d1054331b273568b1b903531b) )
	ROM_LOAD16_BYTE( "mpr-14790.c12", 0x400001, 0x100000, CRC(6a07ac27) SHA1(0558b662c7965c5b74cbc552423194a8facbc092) )
	ROM_LOAD16_BYTE( "mpr-14798.a12", 0x400000, 0x100000, CRC(50634625) SHA1(f4baaebdb1f850e92ca865e103fbca68cdb0de0f) )
	ROM_LOAD16_BYTE( "mpr-14791.c13", 0x600001, 0x100000, CRC(a4ae352b) SHA1(dc814e1c2e167e191cd43fa554ff8ee974d47152) )
	ROM_LOAD16_BYTE( "mpr-14799.a13", 0x600000, 0x100000, CRC(aeb7b025) SHA1(9ce2a9a46176a110c8d2e77deb3d8b9b69b902fa) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14787.c7", 0x010000, 0x40000, CRC(cc6feec7) SHA1(31cc243178b98681a52500a485d74ed9e1274888) )
	ROM_LOAD( "mpr-14786.c6", 0x090000, 0x80000, CRC(cc8349f2) SHA1(9f00d8ea372b70ba44a90dab497deadcc5be3dab) )
	ROM_LOAD( "mpr-14785.c5", 0x110000, 0x80000, CRC(7babba13) SHA1(190cd9ea0f73272e0df34bbdfd8e0035f9e9b0b0) )
	ROM_LOAD( "mpr-14784.c4", 0x190000, 0x80000, CRC(073878e4) SHA1(eff08080d06a16fccf4876e42b389fef599cceba) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Hammer Away, Sega System 18 (prototype / unreleased)
    CPU: M68000
    ROM Board: 837-7525

    Japanese text on the mission screens, but no "For use in Japan..." warning. There are screen shots of a version without
    the Japanese text on mission screens and an alternate title screen, so a "World" proto might exist.
*/
ROM_START( hamaway )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "4.bin",  0x000000, 0x40000, CRC(cc0981e1) SHA1(63528bd36f27e62fdf40715101e6d05b73e48f16) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "6.bin",  0x000001, 0x40000, CRC(e8599ee6) SHA1(3e32b025403aecbaecfcdd0325e4acd676e99c4e) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "5.bin",  0x080000, 0x40000, CRC(fdb247fd) SHA1(ee9db799fb5de27f81904f8ef792203415b6d4a6) )
	ROM_LOAD16_BYTE( "7.bin",  0x080001, 0x40000, CRC(63711470) SHA1(6c4be3a0cf0f897c34ef0b3bf549f52b185bb915) )

	ROM_REGION( 0x180000, "gfx1", 0 ) // tiles
	ROM_LOAD( "c10.bin",  0x000000, 0x40000, CRC(c55cb5cf) SHA1(396179632b29ac5f8b7f8f3c91d7cf834e548bdf) )
	ROM_LOAD( "1.bin",    0x040000, 0x40000, CRC(33be003f) SHA1(134fa6b3347c306d9e30882dfcf24632b49f85ea) )
	ROM_LOAD( "c11.bin",  0x080000, 0x40000, CRC(37787915) SHA1(c8d251be6c41de3aed2da6da70aa87071b70b1f6) )
	ROM_LOAD( "2.bin",    0x0c0000, 0x40000, CRC(60ca5c9f) SHA1(6358ea00125a5e3f55acf73aeb9c36b1db6e711e) )
	ROM_LOAD( "c12.bin",  0x100000, 0x40000, CRC(f12f1cf3) SHA1(45e883029c58e617a2a20ac1ab5c5f598c95f4bd) )
	ROM_LOAD( "3.bin",    0x140000, 0x40000, CRC(520aa7ae) SHA1(9584206aedd8be5ce9dca0ed370f8fe77aabaf76) )

	ROM_REGION16_BE( 0x200000, "sprites", ROMREGION_ERASEFF ) // sprites
	ROM_LOAD16_BYTE( "c17.bin", 0x000001, 0x40000, CRC(aa28d7aa) SHA1(3dd5d95b05e408c023f9bd77753c37080714239d) )
	ROM_LOAD16_BYTE( "10.bin",  0x000000, 0x40000, CRC(c4c95161) SHA1(2e313a4ca9506f53a2062b4a8e5ba7b381ba93ae) )
	ROM_LOAD16_BYTE( "c18.bin", 0x080001, 0x40000, CRC(0f8fe8bb) SHA1(e6f68442b8d4def29b106458496a47344f70d511) )
	ROM_LOAD16_BYTE( "11.bin",  0x080000, 0x40000, CRC(2b5eacbc) SHA1(ba3690501588b9c88a31022b44bc3c82b44ae26b) )
	ROM_LOAD16_BYTE( "c19.bin", 0x100001, 0x40000, CRC(3c616caa) SHA1(d48a6239b7a52ac13971f7513a65a17af492bfdf) ) // 11xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "12.bin",  0x100000, 0x40000, CRC(c7bbd579) SHA1(ab87bfdad66ea241cb23c9bbfea05f5a1574d6c9) ) // 1ST AND 2ND HALF IDENTICAL (but ok, because pairing ROM has no data in the 2nd half anyway)

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "c16.bin", 0x010000, 0x40000, CRC(913cc18c) SHA1(4bf4ec14937586c3ae77fcad57dcb21f6433ef81) )
	ROM_LOAD( "c15.bin", 0x090000, 0x40000, CRC(b53694fc) SHA1(0e42be2730abce1b52ea94a9fe61cbd1c9a0ccae) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Laser Ghost, Sega System 18
    CPU: FD1094 (317-0166)
    ROM Board: 171-5873B
*/
ROM_START( lghost )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13429.a4", 0x00000, 0x40000, CRC(09bd65c0) SHA1(f2b332a86d52af3c5270f668bdd43a0d44eca346) )
	ROM_LOAD16_BYTE( "epr-13430.a6", 0x00001, 0x40000, CRC(51009fe0) SHA1(f1e6e3714c01c15c0e932470a9e1a17abb59e958) )
	ROM_LOAD16_BYTE( "epr-13411.a5", 0x80000, 0x40000, CRC(5160167b) SHA1(3d176a18c7527b1e485f10b144bb4db1b945e709) )
	ROM_LOAD16_BYTE( "epr-13413.a7", 0x80001, 0x40000, CRC(656b3bd8) SHA1(db81d4ae3138308dce1e3db7a859f1d63c4ff815) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0166.key", 0x0000, 0x2000, CRC(8379961f) SHA1(44e0662e92ece65ad2049b2cd804f516e631166e) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13414.c1", 0x00000, 0x40000, CRC(dada2419) SHA1(f6ffd02d75232a09ea83fd199e5e30b2773b0cf5) )
	ROM_LOAD( "epr-13415.c2", 0x40000, 0x40000, CRC(bbb62c48) SHA1(7a4c5bd11b73a92deece72b55627f48ac167acd6) )
	ROM_LOAD( "epr-13416.c3", 0x80000, 0x40000, CRC(1d11dbae) SHA1(331aa49c6b38d32ec33184dbd0851888463ddbc7) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "epr-13603.a10", 0x000000, 0x80000, CRC(5350a94e) SHA1(47e99803cab4b508feb51069c940d6c824d6961d) )
	ROM_LOAD16_BYTE( "epr-13604.c10", 0x000001, 0x80000, CRC(4009c8e5) SHA1(97f513d312f4c90f8bffdf797afa3749779989a5) )
	ROM_LOAD16_BYTE( "mpr-13421.a11", 0x200000, 0x80000, CRC(2fc75890) SHA1(9f97f07dba3b978df8eb357894168ad74f151d30) )
	ROM_LOAD16_BYTE( "mpr-13424.c11", 0x200001, 0x80000, CRC(fb98d920) SHA1(cebdebe88902e96c631df6053ac2589f794da155) )
	ROM_LOAD16_BYTE( "mpr-13422.a12", 0x400000, 0x80000, CRC(48a0754d) SHA1(9fead9f8319593adb4bddaaa4d053b21ca726009) )
	ROM_LOAD16_BYTE( "mpr-13425.c12", 0x400001, 0x80000, CRC(f8252589) SHA1(5a1ed24296d0609393e53df3ee585a366da4ee46) )
	ROM_LOAD16_BYTE( "mpr-13423.a13", 0x600000, 0x80000, CRC(335bbc9d) SHA1(78793335b2f8a1bb05809259521db193c17c9b98) )
	ROM_LOAD16_BYTE( "mpr-13426.c13", 0x600001, 0x80000, CRC(5cfb1e25) SHA1(1dd57475604f339e58bf946e17ae0dc5cf4a3dba) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13417.c7",   0x010000, 0x20000, CRC(cd7beb49) SHA1(2435ce000f1eefdd06b27ea93e22fd82c0e999d2) )
	// these seem best 3 from the different sized dumps
	ROM_LOAD( "mpr-13420.c6",   0x090000, 0x40000, CRC(3de0dee4) SHA1(31833684df5a34d5e9ef04f2ab42355b8e9cbb45) )
	ROM_LOAD( "mpr-13419.c5",   0x110000, 0x40000, CRC(e7021b0a) SHA1(82e390fac63965d4f80ae01758c19ae951c39475) )
	ROM_LOAD( "mpr-13418.c4",   0x190000, 0x40000, CRC(0732594d) SHA1(9fbeae29f1a31d136ddc9a49c786b2a08a523e0d) )
ROM_END

ROM_START( lghostd )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13429.a4", 0x00000, 0x40000, CRC(3d641671) SHA1(68941985c19732b073363fa1eb3848b78e31bc05) )
	ROM_LOAD16_BYTE( "bootleg_epr-13430.a6", 0x00001, 0x40000, CRC(f0ba14be) SHA1(a771ec7ca9f7087ed990d6565cce5c15d08766a4) )
	ROM_LOAD16_BYTE( "epr-13411.a5", 0x80000, 0x40000, CRC(5160167b) SHA1(3d176a18c7527b1e485f10b144bb4db1b945e709) )
	ROM_LOAD16_BYTE( "epr-13413.a7", 0x80001, 0x40000, CRC(656b3bd8) SHA1(db81d4ae3138308dce1e3db7a859f1d63c4ff815) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13414.c1", 0x00000, 0x40000, CRC(dada2419) SHA1(f6ffd02d75232a09ea83fd199e5e30b2773b0cf5) )
	ROM_LOAD( "epr-13415.c2", 0x40000, 0x40000, CRC(bbb62c48) SHA1(7a4c5bd11b73a92deece72b55627f48ac167acd6) )
	ROM_LOAD( "epr-13416.c3", 0x80000, 0x40000, CRC(1d11dbae) SHA1(331aa49c6b38d32ec33184dbd0851888463ddbc7) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "epr-13603.a10", 0x000000, 0x80000, CRC(5350a94e) SHA1(47e99803cab4b508feb51069c940d6c824d6961d) )
	ROM_LOAD16_BYTE( "epr-13604.c10", 0x000001, 0x80000, CRC(4009c8e5) SHA1(97f513d312f4c90f8bffdf797afa3749779989a5) )
	ROM_LOAD16_BYTE( "mpr-13421.a11", 0x200000, 0x80000, CRC(2fc75890) SHA1(9f97f07dba3b978df8eb357894168ad74f151d30) )
	ROM_LOAD16_BYTE( "mpr-13424.c11", 0x200001, 0x80000, CRC(fb98d920) SHA1(cebdebe88902e96c631df6053ac2589f794da155) )
	ROM_LOAD16_BYTE( "mpr-13422.a12", 0x400000, 0x80000, CRC(48a0754d) SHA1(9fead9f8319593adb4bddaaa4d053b21ca726009) )
	ROM_LOAD16_BYTE( "mpr-13425.c12", 0x400001, 0x80000, CRC(f8252589) SHA1(5a1ed24296d0609393e53df3ee585a366da4ee46) )
	ROM_LOAD16_BYTE( "mpr-13423.a13", 0x600000, 0x80000, CRC(335bbc9d) SHA1(78793335b2f8a1bb05809259521db193c17c9b98) )
	ROM_LOAD16_BYTE( "mpr-13426.c13", 0x600001, 0x80000, CRC(5cfb1e25) SHA1(1dd57475604f339e58bf946e17ae0dc5cf4a3dba) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13417.c7",   0x010000, 0x20000, CRC(cd7beb49) SHA1(2435ce000f1eefdd06b27ea93e22fd82c0e999d2) )
	// these seem best 3 from the different sized dumps
	ROM_LOAD( "mpr-13420.c6",   0x090000, 0x40000, CRC(3de0dee4) SHA1(31833684df5a34d5e9ef04f2ab42355b8e9cbb45) )
	ROM_LOAD( "mpr-13419.c5",   0x110000, 0x40000, CRC(e7021b0a) SHA1(82e390fac63965d4f80ae01758c19ae951c39475) )
	ROM_LOAD( "mpr-13418.c4",   0x190000, 0x40000, CRC(0732594d) SHA1(9fbeae29f1a31d136ddc9a49c786b2a08a523e0d) )
ROM_END

/**************************************************************************************************************************
    Laser Ghost, Sega System 18
    CPU: FD1094 (317-0165)
      Game BD: 833-7627-04 LASER GHOST
    ROM Board: 834-7597-04 (type 171-5873B)
    A/D BD NO. 837-7536
*/
ROM_START( lghostu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13427.a4",  0x00000, 0x40000, CRC(5bf8fb6b) SHA1(587bbf45b5e470da7d9166b2cbf4ac58a1f2a825) )
	ROM_LOAD16_BYTE( "epr-13428.a6",  0x00001, 0x40000, CRC(276775f5) SHA1(5dd5dabe7e9311b3520d0405dda0c983e9bc4ba2) )
	ROM_LOAD16_BYTE( "epr-13411.a5",  0x80000, 0x40000, CRC(5160167b) SHA1(3d176a18c7527b1e485f10b144bb4db1b945e709) )
	ROM_LOAD16_BYTE( "epr-13413.a7",  0x80001, 0x40000, CRC(656b3bd8) SHA1(db81d4ae3138308dce1e3db7a859f1d63c4ff815) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0165.key", 0x0000, 0x2000,  CRC(a04267ab) SHA1(688ee59dfaaf240e23de4cada648689d1717ab04) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13414.c1", 0x00000, 0x40000, CRC(dada2419) SHA1(f6ffd02d75232a09ea83fd199e5e30b2773b0cf5) )
	ROM_LOAD( "epr-13415.c2", 0x40000, 0x40000, CRC(bbb62c48) SHA1(7a4c5bd11b73a92deece72b55627f48ac167acd6) )
	ROM_LOAD( "epr-13416.c3", 0x80000, 0x40000, CRC(1d11dbae) SHA1(331aa49c6b38d32ec33184dbd0851888463ddbc7) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "epr-13603.a10", 0x000000, 0x80000, CRC(5350a94e) SHA1(47e99803cab4b508feb51069c940d6c824d6961d) )
	ROM_LOAD16_BYTE( "epr-13604.c10", 0x000001, 0x80000, CRC(4009c8e5) SHA1(97f513d312f4c90f8bffdf797afa3749779989a5) )
	ROM_LOAD16_BYTE( "mpr-13421.a11", 0x200000, 0x80000, CRC(2fc75890) SHA1(9f97f07dba3b978df8eb357894168ad74f151d30) )
	ROM_LOAD16_BYTE( "mpr-13424.c11", 0x200001, 0x80000, CRC(fb98d920) SHA1(cebdebe88902e96c631df6053ac2589f794da155) )
	ROM_LOAD16_BYTE( "mpr-13422.a12", 0x400000, 0x80000, CRC(48a0754d) SHA1(9fead9f8319593adb4bddaaa4d053b21ca726009) )
	ROM_LOAD16_BYTE( "mpr-13425.c12", 0x400001, 0x80000, CRC(f8252589) SHA1(5a1ed24296d0609393e53df3ee585a366da4ee46) )
	ROM_LOAD16_BYTE( "mpr-13423.a13", 0x600000, 0x80000, CRC(335bbc9d) SHA1(78793335b2f8a1bb05809259521db193c17c9b98) )
	ROM_LOAD16_BYTE( "mpr-13426.c13", 0x600001, 0x80000, CRC(5cfb1e25) SHA1(1dd57475604f339e58bf946e17ae0dc5cf4a3dba) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13417.c7",   0x010000, 0x20000, CRC(cd7beb49) SHA1(2435ce000f1eefdd06b27ea93e22fd82c0e999d2) )
	// these seem best 3 from the different sized dumps
	ROM_LOAD( "mpr-13420.c6",   0x090000, 0x40000, CRC(3de0dee4) SHA1(31833684df5a34d5e9ef04f2ab42355b8e9cbb45) )
	ROM_LOAD( "mpr-13419.c5",   0x110000, 0x40000, CRC(e7021b0a) SHA1(82e390fac63965d4f80ae01758c19ae951c39475) )
	ROM_LOAD( "mpr-13418.c4",   0x190000, 0x40000, CRC(0732594d) SHA1(9fbeae29f1a31d136ddc9a49c786b2a08a523e0d) )
ROM_END


ROM_START( lghostud )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13427.a4",  0x00000, 0x40000, CRC(8e9e092e) SHA1(3751a5d74d729f72d34b881bede34b6029ab7f95) )
	ROM_LOAD16_BYTE( "bootleg_epr-13428.a6",  0x00001, 0x40000, CRC(346d4ba3) SHA1(d020bc1456843abc3a4132ea212c3e8140bfd4d2) )
	ROM_LOAD16_BYTE( "epr-13411.a5",  0x80000, 0x40000, CRC(5160167b) SHA1(3d176a18c7527b1e485f10b144bb4db1b945e709) )
	ROM_LOAD16_BYTE( "epr-13413.a7",  0x80001, 0x40000, CRC(656b3bd8) SHA1(db81d4ae3138308dce1e3db7a859f1d63c4ff815) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13414.c1", 0x00000, 0x40000, CRC(dada2419) SHA1(f6ffd02d75232a09ea83fd199e5e30b2773b0cf5) )
	ROM_LOAD( "epr-13415.c2", 0x40000, 0x40000, CRC(bbb62c48) SHA1(7a4c5bd11b73a92deece72b55627f48ac167acd6) )
	ROM_LOAD( "epr-13416.c3", 0x80000, 0x40000, CRC(1d11dbae) SHA1(331aa49c6b38d32ec33184dbd0851888463ddbc7) )

	ROM_REGION16_BE( 0x800000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "epr-13603.a10", 0x000000, 0x80000, CRC(5350a94e) SHA1(47e99803cab4b508feb51069c940d6c824d6961d) )
	ROM_LOAD16_BYTE( "epr-13604.c10", 0x000001, 0x80000, CRC(4009c8e5) SHA1(97f513d312f4c90f8bffdf797afa3749779989a5) )
	ROM_LOAD16_BYTE( "mpr-13421.a11", 0x200000, 0x80000, CRC(2fc75890) SHA1(9f97f07dba3b978df8eb357894168ad74f151d30) )
	ROM_LOAD16_BYTE( "mpr-13424.c11", 0x200001, 0x80000, CRC(fb98d920) SHA1(cebdebe88902e96c631df6053ac2589f794da155) )
	ROM_LOAD16_BYTE( "mpr-13422.a12", 0x400000, 0x80000, CRC(48a0754d) SHA1(9fead9f8319593adb4bddaaa4d053b21ca726009) )
	ROM_LOAD16_BYTE( "mpr-13425.c12", 0x400001, 0x80000, CRC(f8252589) SHA1(5a1ed24296d0609393e53df3ee585a366da4ee46) )
	ROM_LOAD16_BYTE( "mpr-13423.a13", 0x600000, 0x80000, CRC(335bbc9d) SHA1(78793335b2f8a1bb05809259521db193c17c9b98) )
	ROM_LOAD16_BYTE( "mpr-13426.c13", 0x600001, 0x80000, CRC(5cfb1e25) SHA1(1dd57475604f339e58bf946e17ae0dc5cf4a3dba) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13417.c7",   0x010000, 0x20000, CRC(cd7beb49) SHA1(2435ce000f1eefdd06b27ea93e22fd82c0e999d2) )
	// these seem best 3 from the different sized dumps
	ROM_LOAD( "mpr-13420.c6",   0x090000, 0x40000, CRC(3de0dee4) SHA1(31833684df5a34d5e9ef04f2ab42355b8e9cbb45) )
	ROM_LOAD( "mpr-13419.c5",   0x110000, 0x40000, CRC(e7021b0a) SHA1(82e390fac63965d4f80ae01758c19ae951c39475) )
	ROM_LOAD( "mpr-13418.c4",   0x190000, 0x40000, CRC(0732594d) SHA1(9fbeae29f1a31d136ddc9a49c786b2a08a523e0d) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Moonwalker, Sega System 18
    CPU: FD1094 (317-0159)
    ROM Board: 171-5873B
*/
ROM_START( mwalk )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code - custom cpu 317-0159
	ROM_LOAD16_BYTE( "epr-13235.a6", 0x000000, 0x40000, CRC(6983e129) SHA1(a8dd430620ab8ce11df46aa208d762d47f510464) )
	ROM_LOAD16_BYTE( "epr-13234.a5", 0x000001, 0x40000, CRC(c9fd20f2) SHA1(9476e6481e6d8f223acd52f543fa04f408d48dc3) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0159.key", 0x0000, 0x2000, CRC(507838f0) SHA1(0c92d313da40b5dec7398c05b57698de6153b4b0) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13216.b1", 0x00000, 0x40000, CRC(862d2c03) SHA1(3c5446d702a639b62a602c6d687f9875d8450218) )
	ROM_LOAD( "mpr-13217.b2", 0x40000, 0x40000, CRC(7d1ac3ec) SHA1(8495357304f1df135bba77ef3b96e79a883b8ff0) )
	ROM_LOAD( "mpr-13218.b3", 0x80000, 0x40000, CRC(56d3393c) SHA1(50a2d065060692c9ecaa56046a781cb21d93e554) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13224.b11", 0x000001, 0x40000, CRC(c59f107b) SHA1(10fa60fca6e34eda277c483bb1c0e81bb88c8a47) )
	ROM_LOAD16_BYTE( "mpr-13231.a11", 0x000000, 0x40000, CRC(a5e96346) SHA1(a854f4dd5dc16975373255110fdb8ab3d121b1af) )
	ROM_LOAD16_BYTE( "mpr-13223.b10", 0x080001, 0x40000, CRC(364f60ff) SHA1(9ac887ec0b2e32b504b7c6a5f3bb1ce3fe41a15a) )
	ROM_LOAD16_BYTE( "mpr-13230.a10", 0x080000, 0x40000, CRC(9550091f) SHA1(bb6e898f7b540e130fd338c10f74609a7604cef4) )
	ROM_LOAD16_BYTE( "mpr-13222.b9",  0x100001, 0x40000, CRC(523df3ed) SHA1(2e496125e75decd674c3a08404fbdb53791a965d) )
	ROM_LOAD16_BYTE( "mpr-13229.a9",  0x100000, 0x40000, CRC(f40dc45d) SHA1(e9468cef428f52ecdf6837c6d9a9fea934e7676c) )
	ROM_LOAD16_BYTE( "epr-13221.b8",  0x180001, 0x40000, CRC(9ae7546a) SHA1(5413b0131881b0b32bac8de51da9a299835014bb) )
	ROM_LOAD16_BYTE( "epr-13228.a8",  0x180000, 0x40000, CRC(de3786be) SHA1(2279bb390aa3efab9aeee0a643e5cb6a4f5933b6) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13225.a4", 0x010000, 0x20000, CRC(56c2e82b) SHA1(d5755a1bb6e889d274dc60e883d4d65f12fdc877) )
	ROM_LOAD( "mpr-13219.b4", 0x090000, 0x40000, CRC(19e2061f) SHA1(2dcf1718a43dab4da53b4f67722664e70ddd2169) )
	ROM_LOAD( "mpr-13220.b5", 0x110000, 0x40000, CRC(58d4d9ce) SHA1(725e73a656845b02702ef131b4c0aa2a73cdd02e) )
	ROM_LOAD( "mpr-13249.b6", 0x190000, 0x40000, CRC(623edc5d) SHA1(c32d9f818d40f311877fbe6532d9e95b6045c3c4) )

	ROM_REGION( 0x10000, "mcu", 0 ) // protection MCU
	ROM_LOAD( "315-5437.ic4", 0x00000, 0x1000,  CRC(4bf63bc1) SHA1(2766ab30b466b079febb30c488adad9ea56813f7) )
ROM_END

ROM_START( mwalkd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bootleg_epr-13235.a6", 0x000000, 0x40000, CRC(9973f6c3) SHA1(e6267361e808499fcd1c49af6243596b7ebe434d) )
	ROM_LOAD16_BYTE( "bootleg_epr-13234.a5", 0x000001, 0x40000, CRC(ef5098e9) SHA1(2bd17e07477d0f31bddc7e0f53e3447065913072) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13216.b1", 0x00000, 0x40000, CRC(862d2c03) SHA1(3c5446d702a639b62a602c6d687f9875d8450218) )
	ROM_LOAD( "mpr-13217.b2", 0x40000, 0x40000, CRC(7d1ac3ec) SHA1(8495357304f1df135bba77ef3b96e79a883b8ff0) )
	ROM_LOAD( "mpr-13218.b3", 0x80000, 0x40000, CRC(56d3393c) SHA1(50a2d065060692c9ecaa56046a781cb21d93e554) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13224.b11", 0x000001, 0x40000, CRC(c59f107b) SHA1(10fa60fca6e34eda277c483bb1c0e81bb88c8a47) )
	ROM_LOAD16_BYTE( "mpr-13231.a11", 0x000000, 0x40000, CRC(a5e96346) SHA1(a854f4dd5dc16975373255110fdb8ab3d121b1af) )
	ROM_LOAD16_BYTE( "mpr-13223.b10", 0x080001, 0x40000, CRC(364f60ff) SHA1(9ac887ec0b2e32b504b7c6a5f3bb1ce3fe41a15a) )
	ROM_LOAD16_BYTE( "mpr-13230.a10", 0x080000, 0x40000, CRC(9550091f) SHA1(bb6e898f7b540e130fd338c10f74609a7604cef4) )
	ROM_LOAD16_BYTE( "mpr-13222.b9",  0x100001, 0x40000, CRC(523df3ed) SHA1(2e496125e75decd674c3a08404fbdb53791a965d) )
	ROM_LOAD16_BYTE( "mpr-13229.a9",  0x100000, 0x40000, CRC(f40dc45d) SHA1(e9468cef428f52ecdf6837c6d9a9fea934e7676c) )
	ROM_LOAD16_BYTE( "epr-13221.b8",  0x180001, 0x40000, CRC(9ae7546a) SHA1(5413b0131881b0b32bac8de51da9a299835014bb) )
	ROM_LOAD16_BYTE( "epr-13228.a8",  0x180000, 0x40000, CRC(de3786be) SHA1(2279bb390aa3efab9aeee0a643e5cb6a4f5933b6) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13225.a4", 0x010000, 0x20000, CRC(56c2e82b) SHA1(d5755a1bb6e889d274dc60e883d4d65f12fdc877) )
	ROM_LOAD( "mpr-13219.b4", 0x090000, 0x40000, CRC(19e2061f) SHA1(2dcf1718a43dab4da53b4f67722664e70ddd2169) )
	ROM_LOAD( "mpr-13220.b5", 0x110000, 0x40000, CRC(58d4d9ce) SHA1(725e73a656845b02702ef131b4c0aa2a73cdd02e) )
	ROM_LOAD( "mpr-13249.b6", 0x190000, 0x40000, CRC(623edc5d) SHA1(c32d9f818d40f311877fbe6532d9e95b6045c3c4) )

	ROM_REGION( 0x10000, "mcu", 0 ) // protection MCU
	ROM_LOAD( "315-5437.ic4", 0x00000, 0x1000,  CRC(4bf63bc1) SHA1(2766ab30b466b079febb30c488adad9ea56813f7) )
ROM_END


/**************************************************************************************************************************
    Moonwalker, Sega System 18
    CPU: FD1094 (317-0158)
    ROM Board: 171-5873B
*/
ROM_START( mwalku )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code - custom cpu 317-0158
	ROM_LOAD16_BYTE( "epr-13233.a6", 0x000000, 0x40000, CRC(f3dac671) SHA1(cd9d372c7e272d2371bc1f9fb0167831c804423f) )
	ROM_LOAD16_BYTE( "epr-13232.a5", 0x000001, 0x40000, CRC(541d8bdf) SHA1(6a99153fddca246ba070e93c4bacd145f15f76bf) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0158.key", 0x0000, 0x2000, CRC(a8a50e8c) SHA1(6e05a40dbf31b4007df1bb27eee85a78da3d8417) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13216.b1", 0x00000, 0x40000, CRC(862d2c03) SHA1(3c5446d702a639b62a602c6d687f9875d8450218) )
	ROM_LOAD( "mpr-13217.b2", 0x40000, 0x40000, CRC(7d1ac3ec) SHA1(8495357304f1df135bba77ef3b96e79a883b8ff0) )
	ROM_LOAD( "mpr-13218.b3", 0x80000, 0x40000, CRC(56d3393c) SHA1(50a2d065060692c9ecaa56046a781cb21d93e554) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13224.b11", 0x000001, 0x40000, CRC(c59f107b) SHA1(10fa60fca6e34eda277c483bb1c0e81bb88c8a47) )
	ROM_LOAD16_BYTE( "mpr-13231.a11", 0x000000, 0x40000, CRC(a5e96346) SHA1(a854f4dd5dc16975373255110fdb8ab3d121b1af) )
	ROM_LOAD16_BYTE( "mpr-13223.b10", 0x080001, 0x40000, CRC(364f60ff) SHA1(9ac887ec0b2e32b504b7c6a5f3bb1ce3fe41a15a) )
	ROM_LOAD16_BYTE( "mpr-13230.a10", 0x080000, 0x40000, CRC(9550091f) SHA1(bb6e898f7b540e130fd338c10f74609a7604cef4) )
	ROM_LOAD16_BYTE( "mpr-13222.b9",  0x100001, 0x40000, CRC(523df3ed) SHA1(2e496125e75decd674c3a08404fbdb53791a965d) )
	ROM_LOAD16_BYTE( "mpr-13229.a9",  0x100000, 0x40000, CRC(f40dc45d) SHA1(e9468cef428f52ecdf6837c6d9a9fea934e7676c) )
	ROM_LOAD16_BYTE( "epr-13221.b8",  0x180001, 0x40000, CRC(9ae7546a) SHA1(5413b0131881b0b32bac8de51da9a299835014bb) )
	ROM_LOAD16_BYTE( "epr-13228.a8",  0x180000, 0x40000, CRC(de3786be) SHA1(2279bb390aa3efab9aeee0a643e5cb6a4f5933b6) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13225.a4", 0x010000, 0x20000, CRC(56c2e82b) SHA1(d5755a1bb6e889d274dc60e883d4d65f12fdc877) )
	ROM_LOAD( "mpr-13219.b4", 0x090000, 0x40000, CRC(19e2061f) SHA1(2dcf1718a43dab4da53b4f67722664e70ddd2169) )
	ROM_LOAD( "mpr-13220.b5", 0x110000, 0x40000, CRC(58d4d9ce) SHA1(725e73a656845b02702ef131b4c0aa2a73cdd02e) )
	ROM_LOAD( "mpr-13249.b6", 0x190000, 0x40000, CRC(623edc5d) SHA1(c32d9f818d40f311877fbe6532d9e95b6045c3c4) )

	ROM_REGION( 0x10000, "mcu", 0 ) // protection MCU
	ROM_LOAD( "315-5437.ic4", 0x00000, 0x1000,  CRC(4bf63bc1) SHA1(2766ab30b466b079febb30c488adad9ea56813f7) )
ROM_END

ROM_START( mwalkud )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bootleg_epr-13233.a6", 0x000000, 0x40000, CRC(b34fe725) SHA1(9d656b68ba758fba9739413cf404ee9b1471fd68) )
	ROM_LOAD16_BYTE( "bootleg_epr-13232.a5", 0x000001, 0x40000, CRC(0d100fd4) SHA1(8c09c00dcb11fac2c7226e4fcc36b25458ccccff) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13216.b1", 0x00000, 0x40000, CRC(862d2c03) SHA1(3c5446d702a639b62a602c6d687f9875d8450218) )
	ROM_LOAD( "mpr-13217.b2", 0x40000, 0x40000, CRC(7d1ac3ec) SHA1(8495357304f1df135bba77ef3b96e79a883b8ff0) )
	ROM_LOAD( "mpr-13218.b3", 0x80000, 0x40000, CRC(56d3393c) SHA1(50a2d065060692c9ecaa56046a781cb21d93e554) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13224.b11", 0x000001, 0x40000, CRC(c59f107b) SHA1(10fa60fca6e34eda277c483bb1c0e81bb88c8a47) )
	ROM_LOAD16_BYTE( "mpr-13231.a11", 0x000000, 0x40000, CRC(a5e96346) SHA1(a854f4dd5dc16975373255110fdb8ab3d121b1af) )
	ROM_LOAD16_BYTE( "mpr-13223.b10", 0x080001, 0x40000, CRC(364f60ff) SHA1(9ac887ec0b2e32b504b7c6a5f3bb1ce3fe41a15a) )
	ROM_LOAD16_BYTE( "mpr-13230.a10", 0x080000, 0x40000, CRC(9550091f) SHA1(bb6e898f7b540e130fd338c10f74609a7604cef4) )
	ROM_LOAD16_BYTE( "mpr-13222.b9",  0x100001, 0x40000, CRC(523df3ed) SHA1(2e496125e75decd674c3a08404fbdb53791a965d) )
	ROM_LOAD16_BYTE( "mpr-13229.a9",  0x100000, 0x40000, CRC(f40dc45d) SHA1(e9468cef428f52ecdf6837c6d9a9fea934e7676c) )
	ROM_LOAD16_BYTE( "epr-13221.b8",  0x180001, 0x40000, CRC(9ae7546a) SHA1(5413b0131881b0b32bac8de51da9a299835014bb) )
	ROM_LOAD16_BYTE( "epr-13228.a8",  0x180000, 0x40000, CRC(de3786be) SHA1(2279bb390aa3efab9aeee0a643e5cb6a4f5933b6) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13225.a4", 0x010000, 0x20000, CRC(56c2e82b) SHA1(d5755a1bb6e889d274dc60e883d4d65f12fdc877) )
	ROM_LOAD( "mpr-13219.b4", 0x090000, 0x40000, CRC(19e2061f) SHA1(2dcf1718a43dab4da53b4f67722664e70ddd2169) )
	ROM_LOAD( "mpr-13220.b5", 0x110000, 0x40000, CRC(58d4d9ce) SHA1(725e73a656845b02702ef131b4c0aa2a73cdd02e) )
	ROM_LOAD( "mpr-13249.b6", 0x190000, 0x40000, CRC(623edc5d) SHA1(c32d9f818d40f311877fbe6532d9e95b6045c3c4) )

	ROM_REGION( 0x10000, "mcu", 0 ) // protection MCU
	ROM_LOAD( "315-5437.ic4", 0x00000, 0x1000,  CRC(4bf63bc1) SHA1(2766ab30b466b079febb30c488adad9ea56813f7) )
ROM_END

/**************************************************************************************************************************
    Moonwalker, Sega System 18
    CPU: FD1094 (317-0157, version uses i8751(315-5437) known to be exist)
    ROM Board: 171-5873B
*/
ROM_START( mwalkj )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code - custom cpu 317-0157
	ROM_LOAD16_BYTE( "epr-13227.a6", 0x000000, 0x40000, CRC(6c0534b3) SHA1(23f35d1a15275cbc4b6d2f81f5634abac3832282) )
	ROM_LOAD16_BYTE( "epr-13226.a5", 0x000001, 0x40000, CRC(99765854) SHA1(c00776c676b77fed4e94bb02f52f905c845ee73c) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0157.key", 0x0000, 0x2000, CRC(324d6931) SHA1(f8f4530a75aeeace1c8456da37118975c5c43316) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13216.b1", 0x00000, 0x40000, CRC(862d2c03) SHA1(3c5446d702a639b62a602c6d687f9875d8450218) )
	ROM_LOAD( "mpr-13217.b2", 0x40000, 0x40000, CRC(7d1ac3ec) SHA1(8495357304f1df135bba77ef3b96e79a883b8ff0) )
	ROM_LOAD( "mpr-13218.b3", 0x80000, 0x40000, CRC(56d3393c) SHA1(50a2d065060692c9ecaa56046a781cb21d93e554) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13224.b11", 0x000001, 0x40000, CRC(c59f107b) SHA1(10fa60fca6e34eda277c483bb1c0e81bb88c8a47) )
	ROM_LOAD16_BYTE( "mpr-13231.a11", 0x000000, 0x40000, CRC(a5e96346) SHA1(a854f4dd5dc16975373255110fdb8ab3d121b1af) )
	ROM_LOAD16_BYTE( "mpr-13223.b10", 0x080001, 0x40000, CRC(364f60ff) SHA1(9ac887ec0b2e32b504b7c6a5f3bb1ce3fe41a15a) )
	ROM_LOAD16_BYTE( "mpr-13230.a10", 0x080000, 0x40000, CRC(9550091f) SHA1(bb6e898f7b540e130fd338c10f74609a7604cef4) )
	ROM_LOAD16_BYTE( "mpr-13222.b9",  0x100001, 0x40000, CRC(523df3ed) SHA1(2e496125e75decd674c3a08404fbdb53791a965d) )
	ROM_LOAD16_BYTE( "mpr-13229.a9",  0x100000, 0x40000, CRC(f40dc45d) SHA1(e9468cef428f52ecdf6837c6d9a9fea934e7676c) )
	ROM_LOAD16_BYTE( "epr-13221.b8",  0x180001, 0x40000, CRC(9ae7546a) SHA1(5413b0131881b0b32bac8de51da9a299835014bb) )
	ROM_LOAD16_BYTE( "epr-13228.a8",  0x180000, 0x40000, CRC(de3786be) SHA1(2279bb390aa3efab9aeee0a643e5cb6a4f5933b6) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13225.a4", 0x010000, 0x20000, CRC(56c2e82b) SHA1(d5755a1bb6e889d274dc60e883d4d65f12fdc877) )
	ROM_LOAD( "mpr-13219.b4", 0x090000, 0x40000, CRC(19e2061f) SHA1(2dcf1718a43dab4da53b4f67722664e70ddd2169) )
	ROM_LOAD( "mpr-13220.b5", 0x110000, 0x40000, CRC(58d4d9ce) SHA1(725e73a656845b02702ef131b4c0aa2a73cdd02e) )
	ROM_LOAD( "mpr-13249.b6", 0x190000, 0x40000, CRC(623edc5d) SHA1(c32d9f818d40f311877fbe6532d9e95b6045c3c4) )

	ROM_REGION( 0x10000, "mcu", 0 ) // protection MCU
	// not verified if mcu is the same as the other sets..
	ROM_LOAD( "315-5437.ic4", 0x00000, 0x1000, BAD_DUMP CRC(4bf63bc1) SHA1(2766ab30b466b079febb30c488adad9ea56813f7) )
ROM_END

ROM_START( mwalkjd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bootleg_epr-13227.a6", 0x000000, 0x40000, CRC(d8e00521) SHA1(a53cdf7e359b25527f4a39ae1a5d0a461ac73242) )
	ROM_LOAD16_BYTE( "bootleg_epr-13226.a5", 0x000001, 0x40000, CRC(cb0ba822) SHA1(c2db762525e7cc65a7272e7a732ba9e3cee29c5d) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-13216.b1", 0x00000, 0x40000, CRC(862d2c03) SHA1(3c5446d702a639b62a602c6d687f9875d8450218) )
	ROM_LOAD( "mpr-13217.b2", 0x40000, 0x40000, CRC(7d1ac3ec) SHA1(8495357304f1df135bba77ef3b96e79a883b8ff0) )
	ROM_LOAD( "mpr-13218.b3", 0x80000, 0x40000, CRC(56d3393c) SHA1(50a2d065060692c9ecaa56046a781cb21d93e554) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-13224.b11", 0x000001, 0x40000, CRC(c59f107b) SHA1(10fa60fca6e34eda277c483bb1c0e81bb88c8a47) )
	ROM_LOAD16_BYTE( "mpr-13231.a11", 0x000000, 0x40000, CRC(a5e96346) SHA1(a854f4dd5dc16975373255110fdb8ab3d121b1af) )
	ROM_LOAD16_BYTE( "mpr-13223.b10", 0x080001, 0x40000, CRC(364f60ff) SHA1(9ac887ec0b2e32b504b7c6a5f3bb1ce3fe41a15a) )
	ROM_LOAD16_BYTE( "mpr-13230.a10", 0x080000, 0x40000, CRC(9550091f) SHA1(bb6e898f7b540e130fd338c10f74609a7604cef4) )
	ROM_LOAD16_BYTE( "mpr-13222.b9",  0x100001, 0x40000, CRC(523df3ed) SHA1(2e496125e75decd674c3a08404fbdb53791a965d) )
	ROM_LOAD16_BYTE( "mpr-13229.a9",  0x100000, 0x40000, CRC(f40dc45d) SHA1(e9468cef428f52ecdf6837c6d9a9fea934e7676c) )
	ROM_LOAD16_BYTE( "epr-13221.b8",  0x180001, 0x40000, CRC(9ae7546a) SHA1(5413b0131881b0b32bac8de51da9a299835014bb) )
	ROM_LOAD16_BYTE( "epr-13228.a8",  0x180000, 0x40000, CRC(de3786be) SHA1(2279bb390aa3efab9aeee0a643e5cb6a4f5933b6) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13225.a4", 0x010000, 0x20000, CRC(56c2e82b) SHA1(d5755a1bb6e889d274dc60e883d4d65f12fdc877) )
	ROM_LOAD( "mpr-13219.b4", 0x090000, 0x40000, CRC(19e2061f) SHA1(2dcf1718a43dab4da53b4f67722664e70ddd2169) )
	ROM_LOAD( "mpr-13220.b5", 0x110000, 0x40000, CRC(58d4d9ce) SHA1(725e73a656845b02702ef131b4c0aa2a73cdd02e) )
	ROM_LOAD( "mpr-13249.b6", 0x190000, 0x40000, CRC(623edc5d) SHA1(c32d9f818d40f311877fbe6532d9e95b6045c3c4) )

	ROM_REGION( 0x10000, "mcu", 0 ) // protection MCU
	// not verified if mcu is the same as the other sets..
	ROM_LOAD( "315-5437.ic4", 0x00000, 0x1000, BAD_DUMP CRC(4bf63bc1) SHA1(2766ab30b466b079febb30c488adad9ea56813f7) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Pontoon, Sega System 18
    CPU: FD1094 317-0153
    ROM Board: 171-5873B
*/
ROM_START( pontoon )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13175.a6", 0x000000, 0x40000, CRC(a2a5d0f5) SHA1(e22b13f152e0edadeb0f84b4a93ad366201cbae9) )
	ROM_LOAD16_BYTE( "epr-13174.a5", 0x000001, 0x40000, CRC(db976b13) SHA1(3970968b21491beb8aac109eeb753b69ca752205) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0153.key", 0x0000, 0x2000, CRC(bcac8c7a) SHA1(1ee9db8f21a55cbfc391af9731d6a1dcf7f2d4c2) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13097.b1", 0x00000, 0x40000, CRC(6474b245) SHA1(af7db8ac8e74628a8ba61d0860960deeca4a3e3f) )
	ROM_LOAD( "epr-13098.b2", 0x40000, 0x40000, CRC(89fc9a9b) SHA1(d98691eb5fef8aca4ad5e416d0a3797d6ca9b012) )
	ROM_LOAD( "epr-13099.b3", 0x80000, 0x40000, CRC(790e0ac6) SHA1(a4999b7015ef27d6cbe4f53bc0d7fe05ee40d178) )

	ROM_REGION16_BE( 0x80000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "epr-13173.b11",  0x000001, 0x40000, CRC(40a0ddfa) SHA1(f3917361f627865d2f1a22791904da056ce8a93a) )
	ROM_LOAD16_BYTE( "epr-13176.a11",  0x000000, 0x40000, CRC(1184fbd2) SHA1(685ee2d7c4a0134af13ccf5d15f2e56a6b905195) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-12826a.a4", 0x10000, 0x20000, CRC(d41e2a3f) SHA1(087a6515ebefc3252a1feab5bf7b8a22bff9e379) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Shadow Dancer, Sega System 18
    CPU: 68000
    ROM Board: 171-5873B
*/
ROM_START( shdancer )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "shdancer.a6",  0x000000, 0x40000, CRC(3d5b3fa9) SHA1(370dd6e8ab9fb9e77eee9262d13fbdb4cf575abc) )
	ROM_LOAD16_BYTE( "shdancer.a5",  0x000001, 0x40000, CRC(2596004e) SHA1(1b993aa74e7559f7e99253fd2144db9449c04cce) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-12712.b1",  0x00000, 0x40000, CRC(9bdabe3d) SHA1(4bb30fa2d4cdefe4a864cef7153b516bc5b02c42) )
	ROM_LOAD( "mpr-12713.b2",  0x40000, 0x40000, CRC(852d2b1c) SHA1(8e5bc83d45e48b621ea3016207f2028fe41701e6) )
	ROM_LOAD( "mpr-12714.b3",  0x80000, 0x40000, CRC(448226ce) SHA1(3060e4a43311069e2691d659c1e0c1a48edfeedb) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-12719.b11", 0x000001, 0x40000, CRC(d6888534) SHA1(2201f1921a68cf39e5a94b487c90e48d032d630f) )
	ROM_LOAD16_BYTE( "mpr-12726.a11", 0x000000, 0x40000, CRC(ff344945) SHA1(2743778c42f53321f9691d60bbf94ea8baf1382f) )
	ROM_LOAD16_BYTE( "mpr-12718.b10", 0x080001, 0x40000, CRC(ba2efc0c) SHA1(459a1a280f870c94aefb70127ed007cb090ed203) )
	ROM_LOAD16_BYTE( "mpr-12725.a10", 0x080000, 0x40000, CRC(268a0c17) SHA1(2756054fa3c3aed30a1fce5e41acb0ceaebe90b5) )
	ROM_LOAD16_BYTE( "mpr-12717.b9",  0x100001, 0x40000, CRC(c81cc4f8) SHA1(22f364e85057ceef533e051c8d0755b9691c5ec4) )
	ROM_LOAD16_BYTE( "mpr-12724.a9",  0x100000, 0x40000, CRC(0f4903dc) SHA1(851bd60e877c9e39be23dc1fde91efc9da513c29) )
	ROM_LOAD16_BYTE( "epr-12716.b8",  0x180001, 0x40000, CRC(a870e629) SHA1(29f6633240f9737ec19e16100decc7aa045b2060) )
	ROM_LOAD16_BYTE( "epr-12723.a8",  0x180000, 0x40000, CRC(c606cf90) SHA1(cb53ae9a6da1eb31c584173d1fbbd1c8539fb54c) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-12720.a4",  0x10000, 0x20000, CRC(7a0d8de1) SHA1(eca5e2104e5b3e772d083a718171234f06ea8a55) )
	ROM_LOAD( "mpr-12715.b4",  0x90000, 0x40000, CRC(07051a52) SHA1(d48658497f4a34665d3e051f893ff057c38925ae) )
ROM_END

/**************************************************************************************************************************
    Shadow Dancer, Sega System 18
    CPU: 68000
    ROM Board: 171-5873B
*/
ROM_START( shdancerj )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12722b.a6",  0x000000, 0x40000, CRC(c00552a2) SHA1(74fddfe596bc00bc11c4a06e2103417e8fd334f6) )
	ROM_LOAD16_BYTE( "epr-12721b.a5",  0x000001, 0x40000, CRC(653d351a) SHA1(1a03a154cb81a5a2f28c38aecdd6b5d107ea7ffa) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-12712.b1",  0x00000, 0x40000, CRC(9bdabe3d) SHA1(4bb30fa2d4cdefe4a864cef7153b516bc5b02c42) )
	ROM_LOAD( "mpr-12713.b2",  0x40000, 0x40000, CRC(852d2b1c) SHA1(8e5bc83d45e48b621ea3016207f2028fe41701e6) )
	ROM_LOAD( "mpr-12714.b3",  0x80000, 0x40000, CRC(448226ce) SHA1(3060e4a43311069e2691d659c1e0c1a48edfeedb) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-12719.b11", 0x000001, 0x40000, CRC(d6888534) SHA1(2201f1921a68cf39e5a94b487c90e48d032d630f) )
	ROM_LOAD16_BYTE( "mpr-12726.a11", 0x000000, 0x40000, CRC(ff344945) SHA1(2743778c42f53321f9691d60bbf94ea8baf1382f) )
	ROM_LOAD16_BYTE( "mpr-12718.b10", 0x080001, 0x40000, CRC(ba2efc0c) SHA1(459a1a280f870c94aefb70127ed007cb090ed203) )
	ROM_LOAD16_BYTE( "mpr-12725.a10", 0x080000, 0x40000, CRC(268a0c17) SHA1(2756054fa3c3aed30a1fce5e41acb0ceaebe90b5) )
	ROM_LOAD16_BYTE( "mpr-12717.b9",  0x100001, 0x40000, CRC(c81cc4f8) SHA1(22f364e85057ceef533e051c8d0755b9691c5ec4) )
	ROM_LOAD16_BYTE( "mpr-12724.a9",  0x100000, 0x40000, CRC(0f4903dc) SHA1(851bd60e877c9e39be23dc1fde91efc9da513c29) )
	ROM_LOAD16_BYTE( "epr-12716.b8",  0x180001, 0x40000, CRC(a870e629) SHA1(29f6633240f9737ec19e16100decc7aa045b2060) )
	ROM_LOAD16_BYTE( "epr-12723.a8",  0x180000, 0x40000, CRC(c606cf90) SHA1(cb53ae9a6da1eb31c584173d1fbbd1c8539fb54c) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-12720.a4",  0x10000, 0x20000, CRC(7a0d8de1) SHA1(eca5e2104e5b3e772d083a718171234f06ea8a55) )
	ROM_LOAD( "mpr-12715.b4",  0x90000, 0x40000, CRC(07051a52) SHA1(d48658497f4a34665d3e051f893ff057c38925ae) )
ROM_END

/**************************************************************************************************************************
    Shadow Dancer, Sega System 18
    CPU: 68000
    ROM Board: 171-5873B

    game No. 833-7246-01
    pcb  No. 837-7248
    rom  No. 834-7247-01
*/
ROM_START( shdancer1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12772b.a6",  0x000000, 0x40000, CRC(6868a4d4) SHA1(f0d142c81fe1eba4f5c59a0163e25c80ccfe85d7) )
	ROM_LOAD16_BYTE( "epr-12771b.a5",  0x000001, 0x40000, CRC(04e30c84) SHA1(6c5705f7de6ee1bd754b51988cc7d1008f817c78) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-12712.b1",  0x00000, 0x40000, CRC(9bdabe3d) SHA1(4bb30fa2d4cdefe4a864cef7153b516bc5b02c42) )
	ROM_LOAD( "mpr-12713.b2",  0x40000, 0x40000, CRC(852d2b1c) SHA1(8e5bc83d45e48b621ea3016207f2028fe41701e6) )
	ROM_LOAD( "mpr-12714.b3",  0x80000, 0x40000, CRC(448226ce) SHA1(3060e4a43311069e2691d659c1e0c1a48edfeedb) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-12719.b11", 0x000001, 0x40000, CRC(d6888534) SHA1(2201f1921a68cf39e5a94b487c90e48d032d630f) )
	ROM_LOAD16_BYTE( "mpr-12726.a11", 0x000000, 0x40000, CRC(ff344945) SHA1(2743778c42f53321f9691d60bbf94ea8baf1382f) )
	ROM_LOAD16_BYTE( "mpr-12718.b10", 0x080001, 0x40000, CRC(ba2efc0c) SHA1(459a1a280f870c94aefb70127ed007cb090ed203) )
	ROM_LOAD16_BYTE( "mpr-12725.a10", 0x080000, 0x40000, CRC(268a0c17) SHA1(2756054fa3c3aed30a1fce5e41acb0ceaebe90b5) )
	ROM_LOAD16_BYTE( "mpr-12717.b9",  0x100001, 0x40000, CRC(c81cc4f8) SHA1(22f364e85057ceef533e051c8d0755b9691c5ec4) )
	ROM_LOAD16_BYTE( "mpr-12724.a9",  0x100000, 0x40000, CRC(0f4903dc) SHA1(851bd60e877c9e39be23dc1fde91efc9da513c29) )
	ROM_LOAD16_BYTE( "epr-12716.b8",  0x180001, 0x40000, CRC(a870e629) SHA1(29f6633240f9737ec19e16100decc7aa045b2060) )
	ROM_LOAD16_BYTE( "epr-12723.a8",  0x180000, 0x40000, CRC(c606cf90) SHA1(cb53ae9a6da1eb31c584173d1fbbd1c8539fb54c) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-12720.a4",  0x10000, 0x20000, CRC(7a0d8de1) SHA1(eca5e2104e5b3e772d083a718171234f06ea8a55) )
	ROM_LOAD( "mpr-12715.b4",  0x90000, 0x40000, CRC(07051a52) SHA1(d48658497f4a34665d3e051f893ff057c38925ae) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Wally wo Sagase! (Where's Wally?), Sega System 18
    CPU: FD1094 317-0197B
    ROM Board: 171-5873B
*/
ROM_START( wwallyj )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code - custom CPU 317-0197 (?)
	ROM_LOAD16_BYTE( "epr-14730b.a4", 0x000000, 0x40000, CRC(e72bc17a) SHA1(ac3b7d86571a6f510c202735134c1bc4809aa26e) )
	ROM_LOAD16_BYTE( "epr-14731b.a6", 0x000001, 0x40000, CRC(6e3235b9) SHA1(11d5628644e8301550c36c93e5f137c67c11e735) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0197b.key", 0x0000, 0x2000, CRC(f5b7c5b4) SHA1(be971a2349e7c3adc995581355fea48f5123421c) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-14719.c1", 0x000000, 0x40000, CRC(8b58c743) SHA1(ee50baa184d68558d62d1817b2b9c138226ed25a) )
	ROM_LOAD( "mpr-14720.c2", 0x040000, 0x40000, CRC(f96d19f4) SHA1(e350b551db8d01062397cd9a0c952a7b5dbf3fe6) )
	ROM_LOAD( "mpr-14721.c3", 0x080000, 0x40000, CRC(c4ced91d) SHA1(4c8a070959ca10e2dddf407ddbba212415d78727) )

	ROM_REGION16_BE( 0x500000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14726.c10", 0x000001, 0x100000, CRC(7213d1d3) SHA1(b70346a1dd3aa112bc696a7f4dd9ca908c3e5afa) )
	ROM_LOAD16_BYTE( "mpr-14732.a10", 0x000000, 0x100000, CRC(04ced549) SHA1(59fd6510f0e14613d830ac6527f12ccc0b9351a5) )
	ROM_LOAD16_BYTE( "mpr-14727.c11", 0x200001, 0x100000, CRC(3b74e0f0) SHA1(40432dbbbf75dae1e5e32b7cc2c4884f5e9e3bf5) )
	ROM_LOAD16_BYTE( "mpr-14733.a11", 0x200000, 0x100000, CRC(6da0444f) SHA1(80c32895af19bda3277376fdbd1c163f0ed25665) )
	ROM_LOAD16_BYTE( "mpr-14728.c12", 0x400001, 0x080000, CRC(5b921587) SHA1(2779dc658bb7a51f2399af5a6463ccb2dd388e88) )
	ROM_LOAD16_BYTE( "mpr-14734.a12", 0x400000, 0x080000, CRC(6f3f5ed9) SHA1(01972c8bd5bfde58715bc0adc7dea73bf6350a26) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14725.c7",   0x010000, 0x20000, CRC(2b29684f) SHA1(b83962a4f475f9b3e79d4f7ff577b170c4043156) )
	ROM_LOAD( "mpr-14724.c6",   0x090000, 0x80000, CRC(47cbea86) SHA1(c70d1fed2912c7c05223ce0bb0941706f957295f) )
	ROM_LOAD( "mpr-14723.c5",   0x110000, 0x80000, CRC(bc5adc27) SHA1(09405002b940a3d3eb0f1258f37af51e0b7581b9) )
	ROM_LOAD( "mpr-14722.c4",   0x190000, 0x80000, CRC(1bd081f8) SHA1(e5b0b5d8334486f813d7c430bb7fce3f69605a21) )
ROM_END

ROM_START( wwallyjd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bootleg_epr-14730b.a4", 0x000000, 0x40000, CRC(1d46f791) SHA1(5bfffd108254db69e582a4e4c1012b9c9e223830) )
	ROM_LOAD16_BYTE( "bootleg_epr-14731b.a6", 0x000001, 0x40000, CRC(a0f6325e) SHA1(a952d08c3d2324425db9d959e4e43f0c7024165b) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-14719.c1", 0x000000, 0x40000, CRC(8b58c743) SHA1(ee50baa184d68558d62d1817b2b9c138226ed25a) )
	ROM_LOAD( "mpr-14720.c2", 0x040000, 0x40000, CRC(f96d19f4) SHA1(e350b551db8d01062397cd9a0c952a7b5dbf3fe6) )
	ROM_LOAD( "mpr-14721.c3", 0x080000, 0x40000, CRC(c4ced91d) SHA1(4c8a070959ca10e2dddf407ddbba212415d78727) )

	ROM_REGION16_BE( 0x500000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14726.c10", 0x000001, 0x100000, CRC(7213d1d3) SHA1(b70346a1dd3aa112bc696a7f4dd9ca908c3e5afa) )
	ROM_LOAD16_BYTE( "mpr-14732.a10", 0x000000, 0x100000, CRC(04ced549) SHA1(59fd6510f0e14613d830ac6527f12ccc0b9351a5) )
	ROM_LOAD16_BYTE( "mpr-14727.c11", 0x200001, 0x100000, CRC(3b74e0f0) SHA1(40432dbbbf75dae1e5e32b7cc2c4884f5e9e3bf5) )
	ROM_LOAD16_BYTE( "mpr-14733.a11", 0x200000, 0x100000, CRC(6da0444f) SHA1(80c32895af19bda3277376fdbd1c163f0ed25665) )
	ROM_LOAD16_BYTE( "mpr-14728.c12", 0x400001, 0x080000, CRC(5b921587) SHA1(2779dc658bb7a51f2399af5a6463ccb2dd388e88) )
	ROM_LOAD16_BYTE( "mpr-14734.a12", 0x400000, 0x080000, CRC(6f3f5ed9) SHA1(01972c8bd5bfde58715bc0adc7dea73bf6350a26) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14725.c7",   0x010000, 0x20000, CRC(2b29684f) SHA1(b83962a4f475f9b3e79d4f7ff577b170c4043156) )
	ROM_LOAD( "mpr-14724.c6",   0x090000, 0x80000, CRC(47cbea86) SHA1(c70d1fed2912c7c05223ce0bb0941706f957295f) )
	ROM_LOAD( "mpr-14723.c5",   0x110000, 0x80000, CRC(bc5adc27) SHA1(09405002b940a3d3eb0f1258f37af51e0b7581b9) )
	ROM_LOAD( "mpr-14722.c4",   0x190000, 0x80000, CRC(1bd081f8) SHA1(e5b0b5d8334486f813d7c430bb7fce3f69605a21) )
ROM_END


/**************************************************************************************************************************
    Wally wo Sagase! (Where's Wally?), Sega System 18
    CPU: FD1094 317-0197A
    ROM Board: 171-5873B
*/
ROM_START( wwallyja )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code - custom CPU 317-0197a
	ROM_LOAD16_BYTE( "epr-14730a.a4", 0x000000, 0x40000, CRC(daa7880e) SHA1(9ea83e04c3e07d84afa67097c28b3951c9db8d00) )
	ROM_LOAD16_BYTE( "epr-14731a.a6", 0x000001, 0x40000, CRC(5e36353b) SHA1(488c54bbef3c8a129785465887bff3b301e11387) )

	ROM_REGION( 0x2000, "maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0197a.key", 0x0000, 0x2000, CRC(2fb6a9a1) SHA1(2649d0905527dbe0dd0ad5cf68c457b4aa5fb32c) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-14719.c1", 0x000000, 0x40000, CRC(8b58c743) SHA1(ee50baa184d68558d62d1817b2b9c138226ed25a) )
	ROM_LOAD( "mpr-14720.c2", 0x040000, 0x40000, CRC(f96d19f4) SHA1(e350b551db8d01062397cd9a0c952a7b5dbf3fe6) )
	ROM_LOAD( "mpr-14721.c3", 0x080000, 0x40000, CRC(c4ced91d) SHA1(4c8a070959ca10e2dddf407ddbba212415d78727) )

	ROM_REGION16_BE( 0x500000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14726.c10", 0x000001, 0x100000, CRC(7213d1d3) SHA1(b70346a1dd3aa112bc696a7f4dd9ca908c3e5afa) )
	ROM_LOAD16_BYTE( "mpr-14732.a10", 0x000000, 0x100000, CRC(04ced549) SHA1(59fd6510f0e14613d830ac6527f12ccc0b9351a5) )
	ROM_LOAD16_BYTE( "mpr-14727.c11", 0x200001, 0x100000, CRC(3b74e0f0) SHA1(40432dbbbf75dae1e5e32b7cc2c4884f5e9e3bf5) )
	ROM_LOAD16_BYTE( "mpr-14733.a11", 0x200000, 0x100000, CRC(6da0444f) SHA1(80c32895af19bda3277376fdbd1c163f0ed25665) )
	ROM_LOAD16_BYTE( "mpr-14728.c12", 0x400001, 0x080000, CRC(5b921587) SHA1(2779dc658bb7a51f2399af5a6463ccb2dd388e88) )
	ROM_LOAD16_BYTE( "mpr-14734.a12", 0x400000, 0x080000, CRC(6f3f5ed9) SHA1(01972c8bd5bfde58715bc0adc7dea73bf6350a26) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14725.c7",   0x010000, 0x20000, CRC(2b29684f) SHA1(b83962a4f475f9b3e79d4f7ff577b170c4043156) )
	ROM_LOAD( "mpr-14724.c6",   0x090000, 0x80000, CRC(47cbea86) SHA1(c70d1fed2912c7c05223ce0bb0941706f957295f) )
	ROM_LOAD( "mpr-14723.c5",   0x110000, 0x80000, CRC(bc5adc27) SHA1(09405002b940a3d3eb0f1258f37af51e0b7581b9) )
	ROM_LOAD( "mpr-14722.c4",   0x190000, 0x80000, CRC(1bd081f8) SHA1(e5b0b5d8334486f813d7c430bb7fce3f69605a21) )
ROM_END

ROM_START( wwallyjad )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bootleg_epr-14730a.a4", 0x000000, 0x40000, CRC(0d9fb6df) SHA1(ea9c6ff545e31a8269c2e23620582e95817dac60))
	ROM_LOAD16_BYTE( "bootleg_epr-14731a.a6", 0x000001, 0x40000, CRC(b88c140d) SHA1(407ded758caf76644d728e046d94b464fa03ca02) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "mpr-14719.c1", 0x000000, 0x40000, CRC(8b58c743) SHA1(ee50baa184d68558d62d1817b2b9c138226ed25a) )
	ROM_LOAD( "mpr-14720.c2", 0x040000, 0x40000, CRC(f96d19f4) SHA1(e350b551db8d01062397cd9a0c952a7b5dbf3fe6) )
	ROM_LOAD( "mpr-14721.c3", 0x080000, 0x40000, CRC(c4ced91d) SHA1(4c8a070959ca10e2dddf407ddbba212415d78727) )

	ROM_REGION16_BE( 0x500000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr-14726.c10", 0x000001, 0x100000, CRC(7213d1d3) SHA1(b70346a1dd3aa112bc696a7f4dd9ca908c3e5afa) )
	ROM_LOAD16_BYTE( "mpr-14732.a10", 0x000000, 0x100000, CRC(04ced549) SHA1(59fd6510f0e14613d830ac6527f12ccc0b9351a5) )
	ROM_LOAD16_BYTE( "mpr-14727.c11", 0x200001, 0x100000, CRC(3b74e0f0) SHA1(40432dbbbf75dae1e5e32b7cc2c4884f5e9e3bf5) )
	ROM_LOAD16_BYTE( "mpr-14733.a11", 0x200000, 0x100000, CRC(6da0444f) SHA1(80c32895af19bda3277376fdbd1c163f0ed25665) )
	ROM_LOAD16_BYTE( "mpr-14728.c12", 0x400001, 0x080000, CRC(5b921587) SHA1(2779dc658bb7a51f2399af5a6463ccb2dd388e88) )
	ROM_LOAD16_BYTE( "mpr-14734.a12", 0x400000, 0x080000, CRC(6f3f5ed9) SHA1(01972c8bd5bfde58715bc0adc7dea73bf6350a26) )

	ROM_REGION( 0x210000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-14725.c7",   0x010000, 0x20000, CRC(2b29684f) SHA1(b83962a4f475f9b3e79d4f7ff577b170c4043156) )
	ROM_LOAD( "mpr-14724.c6",   0x090000, 0x80000, CRC(47cbea86) SHA1(c70d1fed2912c7c05223ce0bb0941706f957295f) )
	ROM_LOAD( "mpr-14723.c5",   0x110000, 0x80000, CRC(bc5adc27) SHA1(09405002b940a3d3eb0f1258f37af51e0b7581b9) )
	ROM_LOAD( "mpr-14722.c4",   0x190000, 0x80000, CRC(1bd081f8) SHA1(e5b0b5d8334486f813d7c430bb7fce3f69605a21) )
ROM_END



/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(segas18_state,generic_shad)
{
	init_generic(ROM_BOARD_171_SHADOW);
}

DRIVER_INIT_MEMBER(segas18_state,generic_5874)
{
	init_generic(ROM_BOARD_171_5874);
}

DRIVER_INIT_MEMBER(segas18_state,generic_5987)
{
	init_generic(ROM_BOARD_171_5987);
}

DRIVER_INIT_MEMBER(segas18_state,hamaway)
{
	init_generic(ROM_BOARD_837_7525);
}


/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

DRIVER_INIT_MEMBER(segas18_state,ddcrew)
{
	init_generic_5987();
	m_custom_io_r = read16_delegate(FUNC(segas18_state::ddcrew_custom_io_r), this);
}

DRIVER_INIT_MEMBER(segas18_state,lghost)
{
	init_generic_5987();
	m_custom_io_r = read16_delegate(FUNC(segas18_state::lghost_custom_io_r), this);
	m_custom_io_w = write16_delegate(FUNC(segas18_state::lghost_custom_io_w), this);
}

DRIVER_INIT_MEMBER(segas18_state,wwally)
{
	init_generic_5987();
	m_custom_io_r = read16_delegate(FUNC(segas18_state::wwally_custom_io_r), this);
	m_custom_io_w = write16_delegate(FUNC(segas18_state::wwally_custom_io_w), this);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

//    YEAR, NAME,      PARENT,   MACHINE,              INPUT,                   INIT,         MONITOR, COMPANY,        FULLNAME,                                        FLAGS
GAME( 1990, astorm,    0,        system18_fd1094,      astorm2p, segas18_state, generic_5874, ROT0,   "Sega",          "Alien Storm (World, 2 Players) (FD1094 317-0154)", 0 )
GAME( 1990, astorm3,   astorm,   system18_fd1094,      astorm,   segas18_state, generic_5874, ROT0,   "Sega",          "Alien Storm (World, 3 Players) (FD1094 317-0148)", 0 )
GAME( 1990, astormu,   astorm,   system18_fd1094,      astorm,   segas18_state, generic_5874, ROT0,   "Sega",          "Alien Storm (US, 3 Players) (FD1094 317-0147)", 0 )
GAME( 1990, astormj,   astorm,   system18_fd1094,      astorm2p, segas18_state, generic_5874, ROT0,   "Sega",          "Alien Storm (Japan, 2 Players) (FD1094 317-0146)", 0 )

GAME( 1989, bloxeed,   0,        system18_fd1094,      bloxeed,  segas18_state, generic_5874, ROT0,   "Sega",          "Bloxeed (Japan) (FD1094 317-0139)", 0 )

GAME( 1991, cltchitr,  0,        system18_fd1094,      cltchitr, segas18_state, generic_5987, ROT0,   "Sega",          "Clutch Hitter (US) (FD1094 317-0176)", 0 )
GAME( 1991, cltchitrj, cltchitr, system18_fd1094,      cltchitr, segas18_state, generic_5987, ROT0,   "Sega",          "Clutch Hitter (Japan) (FD1094 317-0175)", 0 )

GAME( 1992, desertbr,  0,        system18_fd1094,      desertbr, segas18_state, generic_5987, ROT270, "Sega",          "Desert Breaker (World) (FD1094 317-0196)", 0 )
GAME( 1992, desertbrj, desertbr, system18_fd1094,      desertbr, segas18_state, generic_5987, ROT270, "Sega",          "Desert Breaker (Japan) (FD1094 317-0194)", 0 )

GAME( 1991, ddcrew,    0,        system18_fd1094,      ddcrew3p, segas18_state, ddcrew,       ROT0,   "Sega",          "D. D. Crew (World, 3 Players) (FD1094 317-0190)", 0 )
GAME( 1991, ddcrewu,   ddcrew,   system18_fd1094,      ddcrew,   segas18_state, ddcrew,       ROT0,   "Sega",          "D. D. Crew (US, 4 Players) (FD1094 317-0186)", 0 )
GAME( 1991, ddcrew2,   ddcrew,   system18_fd1094,      ddcrew2p, segas18_state, ddcrew,       ROT0,   "Sega",          "D. D. Crew (World, 2 Players) (FD1094 317-0184)", 0 )
GAME( 1991, ddcrew1,   ddcrew,   system18_fd1094,      ddcrew,   segas18_state, ddcrew,       ROT0,   "Sega",          "D. D. Crew (World, 4 Players) (FD1094 317-0187)", 0 )
GAME( 1991, ddcrewj,   ddcrew,   system18_fd1094,      ddcrew,   segas18_state, ddcrew,       ROT0,   "Sega",          "D. D. Crew (Japan, 4 Players) (FD1094 317-0185)", 0 )
GAME( 1991, ddcrewj2,  ddcrew,   system18_fd1094,      ddcrew2p, segas18_state, ddcrew,       ROT0,   "Sega",          "D. D. Crew (Japan, 2 Players) (FD1094 317-0182)", 0 )

GAME( 1991, hamaway,   0,        system18,             hamaway,  segas18_state, hamaway,      ROT90,  "Sega / Santos", "Hammer Away (Japan, prototype)", 0 )

GAME( 1990, lghost,    0,        lghost_fd1094,        lghost,   segas18_state, lghost,       ROT0,   "Sega",          "Laser Ghost (World) (FD1094 317-0166)", 0 )
GAME( 1990, lghostu,   lghost,   lghost_fd1094,        lghost,   segas18_state, lghost,       ROT0,   "Sega",          "Laser Ghost (US) (FD1094 317-0165)", 0 )

GAME( 1990, mwalk,     0,        system18_fd1094_i8751,mwalk,    segas18_state, generic_5874, ROT0,   "Sega",          "Michael Jackson's Moonwalker (World) (FD1094/8751 317-0159)", 0 )
GAME( 1990, mwalku,    mwalk,    system18_fd1094_i8751,mwalka,   segas18_state, generic_5874, ROT0,   "Sega",          "Michael Jackson's Moonwalker (US) (FD1094/8751 317-0158)", 0 )
GAME( 1990, mwalkj,    mwalk,    system18_fd1094_i8751,mwalk,    segas18_state, generic_5874, ROT0,   "Sega",          "Michael Jackson's Moonwalker (Japan) (FD1094/8751 317-0157)", 0 )

GAME( 1989, pontoon,   0,        system18_fd1094,      shdancer, segas18_state, generic_5874, ROT0,   "Sega",          "Pontoon (FD1094 317-0153)", MACHINE_NOT_WORKING ) // satellite/networked gambling game?

GAME( 1989, shdancer,  0,        system18,             shdancer, segas18_state, generic_shad, ROT0,   "Sega",          "Shadow Dancer (World)", 0 )
GAME( 1989, shdancerj, shdancer, system18,             shdancer, segas18_state, generic_shad, ROT0,   "Sega",          "Shadow Dancer (Japan)", 0 )
GAME( 1989, shdancer1, shdancer, system18,             shdancer, segas18_state, generic_shad, ROT0,   "Sega",          "Shadow Dancer (US)", 0 )

GAME( 1992, wwallyj,   0,        system18_fd1094,      wwally,   segas18_state, wwally,       ROT0,   "Sega",          "Wally wo Sagase! (rev B, Japan) (FD1094 317-0197B)", 0 ) // the roms do contain an english logo so maybe there is a world / us set too
GAME( 1992, wwallyja,  wwallyj,  system18_fd1094,      wwally,   segas18_state, wwally,       ROT0,   "Sega",          "Wally wo Sagase! (rev A, Japan) (FD1094 317-0197A)", 0 )

// decrypted bootleg sets

GAME( 1990, astorm3d,   astorm,   system18,      astorm,   segas18_state, generic_5874, ROT0,   "bootleg",          "Alien Storm (World, 3 Players) (bootleg of FD1094 317-0148 set)", 0 )
GAME( 1990, astormud,   astorm,   system18,      astorm,   segas18_state, generic_5874, ROT0,   "bootleg",          "Alien Storm (US, 3 Players) (bootleg of FD1094 317-0147 set)", 0 )
GAME( 1990, astormjd,   astorm,   system18,      astorm2p, segas18_state, generic_5874, ROT0,   "bootleg",          "Alien Storm (Japan, 2 Players) (bootleg of FD1094 317-0146 set)", 0 )

GAME( 1989, bloxeedd,   bloxeed,  system18,      bloxeed,  segas18_state, generic_5874, ROT0,   "bootleg",          "Bloxeed (Japan) (bootleg of FD1094 317-0139 set)", 0 )

GAME( 1991, cltchitrd,  cltchitr, system18,      cltchitr, segas18_state, generic_5987, ROT0,   "bootleg",          "Clutch Hitter (US) (bootleg of FD1094 317-0176 set)", 0 )
GAME( 1991, cltchitrjd, cltchitr, system18,      cltchitr, segas18_state, generic_5987, ROT0,   "bootleg",          "Clutch Hitter (Japan) (bootleg of FD1094 317-0175 set)", 0 )

GAME( 1992, desertbrd,  desertbr, system18,      desertbr, segas18_state, generic_5987, ROT270, "bootleg",          "Desert Breaker (World) (bootleg of FD1094 317-0196 set)", 0 )
GAME( 1992, desertbrjd, desertbr, system18,      desertbr, segas18_state, generic_5987, ROT270, "bootleg",          "Desert Breaker (Japan) (bootleg of FD1094 317-0194 set)", 0 )

GAME( 1991, ddcrewd,    ddcrew,   system18,      ddcrew3p, segas18_state, ddcrew,       ROT0,   "bootleg",          "D. D. Crew (World, 3 Players) (bootleg of FD1094 317-0190 set)", 0 )
GAME( 1991, ddcrewud,   ddcrew,   system18,      ddcrew,   segas18_state, ddcrew,       ROT0,   "bootleg",          "D. D. Crew (US, 4 Players) (bootleg of FD1094 317-0186 set)", 0 )
GAME( 1991, ddcrew2d,   ddcrew,   system18,      ddcrew2p, segas18_state, ddcrew,       ROT0,   "bootleg",          "D. D. Crew (World, 2 Players) (bootleg of FD1094 317-0184 set)", 0 )
GAME( 1991, ddcrew1d,   ddcrew,   system18,      ddcrew,   segas18_state, ddcrew,       ROT0,   "bootleg",          "D. D. Crew (World, 4 Players) (bootleg of FD1094 317-0187 set)", 0 )
GAME( 1991, ddcrewjd,   ddcrew,   system18,      ddcrew,   segas18_state, ddcrew,       ROT0,   "bootleg",          "D. D. Crew (Japan, 4 Players) (bootleg of FD1094 317-0185 set)", 0 )
GAME( 1991, ddcrewj2d,  ddcrew,   system18,      ddcrew2p, segas18_state, ddcrew,       ROT0,   "bootleg",          "D. D. Crew (Japan, 2 Players) (bootleg of FD1094 317-0182 set)", 0 )

GAME( 1990, lghostd,    lghost,   lghost,        lghost,   segas18_state, lghost,       ROT0,   "bootleg",          "Laser Ghost (World) (bootleg of FD1094 317-0166 set)", 0 )
GAME( 1990, lghostud,   lghost,   lghost,        lghost,   segas18_state, lghost,       ROT0,   "bootleg",          "Laser Ghost (US) (bootleg of FD1094 317-0165 set)", 0 )

GAME( 1990, mwalkd,     mwalk,    system18_i8751,mwalk,    segas18_state, generic_5874, ROT0,   "bootleg",          "Michael Jackson's Moonwalker (World) (bootleg of FD1094/8751 317-0159)", 0 )
GAME( 1990, mwalkud,    mwalk,    system18_i8751,mwalka,   segas18_state, generic_5874, ROT0,   "bootleg",          "Michael Jackson's Moonwalker (US) (bootleg of FD1094/8751 317-0158)", 0 )
GAME( 1990, mwalkjd,    mwalk,    system18_i8751,mwalk,    segas18_state, generic_5874, ROT0,   "bootleg",          "Michael Jackson's Moonwalker (Japan) (bootleg of FD1094/8751 317-0157 set)", 0 )

GAME( 1992, wwallyjd,   wwallyj,  system18,      wwally,   segas18_state, wwally,       ROT0,   "bootleg",          "Wally wo Sagase! (rev B, Japan) (bootleg of FD1094 317-0197B set)", 0 )
GAME( 1992, wwallyjad,  wwallyj,  system18,      wwally,   segas18_state, wwally,       ROT0,   "bootleg",          "Wally wo Sagase! (rev A, Japan) (bootleg of FD1094 317-0197A set)", 0 )
