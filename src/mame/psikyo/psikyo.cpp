// license:BSD-3-Clause
// copyright-holders:Luca Elia,Olivier Galibert,Paul Priest
/***************************************************************************

                            -= Psikyo Games =-

                driver by   Luca Elia (l.elia@tin.it)


CPU:    68EC020 + PIC16C57 [Optional MCU]

Sound:  Z80A                +   YM2610
   Or:  LZ8420M (Z80 core)  +   YMF286-K (YM2610 compatible)

Chips:  PS2001B
        PS3103
        PS3204
        PS3305

---------------------------------------------------------------------------
Name                    Year    Board          Notes
---------------------------------------------------------------------------
Sengoku Ace         (J) 1993    SH201B
Gun Bird            (J) 1994    KA302C
Battle K-Road       (J) 1994    ""
Strikers 1945       (J) 1995    SH403/SH404    SH403 is similar to KA302C
Tengai              (J) 1996    SH404          SH404 has MCU, ymf278-b for sound and gfx banking
---------------------------------------------------------------------------

To Do:

- tengai / tengaij: "For use in Japan" screen is supposed to output the
  typical blue Psikyo backdrop gradient instead of being pure black as it is
  now;
- Flip Screen support

NOTE: Despite being mentioned in the manual Strikers 1945 doesn't seem to
      have a Free Play mode.

The tengai PIC dump has been tested on PCB with the available s1945 program ROMs:
s1945 (World) - working ok
s1945a (Japan / World) - resets when start pressed
s1945k (Korea) - working ok
s1945j (Japan) - boots but locks up with black screen when start pressed

***************************************************************************/

/***** Gun Bird Japan Crash Notes

The Following Section of Code in Gunbird causes reads from the
0x080000 - 0x0fffff region

002894: E2817000           asr.l   #1, D1
002896: 70001030           moveq   #$0, D0
002898: 10301804           move.b  ($4,A0,D1.l), D0   <-- *
00289C: 60183202           bra     28b6
00289E: 3202C2C7           move.w  D2, D1
0028A0: C2C77000           mulu.w  D7, D1
0028A2: 70003005           moveq   #$0, D0
0028A4: 3005D280           move.w  D5, D0
0028A6: D280E281           add.l   D0, D1
0028A8: E2812071           asr.l   #1, D1
0028AA: 20713515           movea.l ([A1],D3.w*4), A0
0028AE: 70001030           moveq   #$0, D0
0028B0: 10301804           move.b  ($4,A0,D1.l), D0   <-- *
0028B4: E880720F           asr.l   #4, D0
0028B6: 720FC041           moveq   #$f, D1

This causes Gunbird to crash if the ROM Region Size
allocated during loading is smaller than the ROM
region as it tries to read beyond the allocated rom region

This was pointed out by Bart Puype

*****/

#include "emu.h"
#include "psikyo.h"

#include "cpu/z80/z80.h"
#include "cpu/z80/lz8420m.h"
#include "cpu/m68000/m68020.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"
#include "speaker.h"


/***************************************************************************
                        Strikers 1945 / Tengai MCU
***************************************************************************/

int psikyo_state::mcu_status_r()
{
	int ret = 0x00;

	/* Don't know exactly what this bit is, but s1945 and tengai
	    both spin waiting for it to go low during POST.  Also,
	    the following code in tengai (don't know where or if it is
	    reached) waits for it to pulse:

	    01A546:  move.b  (A2), D0    ; A2 = $c00003
	    01A548:  andi.b  #$4, D0
	    01A54C:  beq     $1a546
	    01A54E:  move.b  (A2), D0
	    01A550:  andi.b  #$4, D0
	    01A554:  bne     $1a54e

	    Interestingly, s1945nj has the code that spins on this bit,
	    but said code is never reached.  Prototype? */
	if (m_mcu_status)
		ret = 0x01;

	if (!machine().side_effects_disabled())
		m_mcu_status = !m_mcu_status;   /* hack */

	return ret;
}

static const u8 s1945_table[256] = {
	0x00, 0x00, 0x64, 0xae, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc5, 0x1e,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xac, 0x5c, 0x00, 0x00, 0x2b, 0xc0
};

static const u8 s1945a_table[256] = {
	0x00, 0x00, 0x64, 0xbe, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc7, 0x2a,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xad, 0x4c, 0x00, 0x00, 0x2b, 0xc0
};

static const u8 s1945j_table[256] = {
	0x00, 0x00, 0x64, 0xb6, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc5, 0x92,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xac, 0x64, 0x00, 0x00, 0x2b, 0xc0
};

void psikyo_state::s1945_mcu_data_w(uint8_t data)
{
	m_s1945_mcu_inlatch = data;
}

void psikyo_state::s1945_mcu_control_w(uint8_t data)
{
	m_s1945_mcu_control = data;
}

void psikyo_state::s1945_mcu_direction_w(uint8_t data)
{
	m_s1945_mcu_direction = data;
}

void psikyo_state::s1945_mcu_bctrl_w(uint8_t data)
{
	switch_bgbanks(1, (data >> 6) & 3);
	switch_bgbanks(0, (data >> 4) & 3);
	m_s1945_mcu_bctrl = data;
}

void psikyo_state::s1945_mcu_command_w(uint8_t data)
{
	switch (data | (m_s1945_mcu_direction ? 0x100 : 0))
	{
	case 0x11c:
		m_s1945_mcu_latching = 5;
		m_s1945_mcu_index = m_s1945_mcu_inlatch;
		break;
	case 0x013:
//          logerror("MCU: Table read index %02x\n", m_s1945_mcu_index);
		m_s1945_mcu_latching = 1;
		if (m_s1945_mcu_table != nullptr)
			m_s1945_mcu_latch1 = m_s1945_mcu_table[m_s1945_mcu_index];
		break;
	case 0x113:
		m_s1945_mcu_mode = m_s1945_mcu_inlatch;
		if (m_s1945_mcu_mode == 1)
		{
			m_s1945_mcu_latching &= ~1;
			m_s1945_mcu_latch2 = 0x55;
		}
		else
		{
			// Go figure.
			m_s1945_mcu_latching &= ~1;
			m_s1945_mcu_latching |= 2;
		}
		m_s1945_mcu_latching &= ~4;
		m_s1945_mcu_latch1 = m_s1945_mcu_inlatch;
		break;
	case 0x010:
	case 0x110:
		m_s1945_mcu_latching |= 4;
		break;
	default:
//          logerror("MCU: function %02x, direction %02x, latch1 %02x, latch2 %02x (%x)\n", data, m_s1945_mcu_direction, m_s1945_mcu_latch1, m_s1945_mcu_latch2, m_maincpu->pc());
		break;
	}
}

// TODO: make this handler 8-bit
u32 psikyo_state::s1945_mcu_data_r()
{
	u32 res;
	if (m_s1945_mcu_control & 16)
	{
		res = m_s1945_mcu_latching & 4 ? 0x0000ff00 : m_s1945_mcu_latch1 << 8;
		if (!machine().side_effects_disabled())
			m_s1945_mcu_latching |= 4;
	}
	else
	{
		res = m_s1945_mcu_latching & 1 ? 0x0000ff00 : m_s1945_mcu_latch2 << 8;
		if (!machine().side_effects_disabled())
			m_s1945_mcu_latching |= 1;
	}
	res |= m_s1945_mcu_bctrl & 0xf0;
	return res;
}

uint8_t psikyo_state::s1945_mcu_control_r()
{
	return m_s1945_mcu_latching | 0x08;
}

template<int Layer>
u16 psikyo_state::vram_r(offs_t offset)
{
	return m_vram[Layer][offset];
}

template<int Layer>
void psikyo_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	const u32 tilemap_indexes = m_tilemap[Layer]->rows() * m_tilemap[Layer]->cols();
	while (offset < tilemap_indexes)
	{
		m_tilemap[Layer]->mark_tile_dirty(offset);
		offset += 0x1000;
	}
}


/***************************************************************************


                                Memory Map


***************************************************************************/

void psikyo_state::psikyo_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                 // ROM (not all used)
	map(0x400000, 0x401fff).ram().share("spriteram");       // Sprites, buffered by two frames (list buffered + fb buffered)
	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");    // Palette
	map(0x800000, 0x801fff).rw(FUNC(psikyo_state::vram_r<0>), FUNC(psikyo_state::vram_w<0>));      // Layer 0
	map(0x802000, 0x803fff).rw(FUNC(psikyo_state::vram_r<1>), FUNC(psikyo_state::vram_w<1>));      // Layer 1
	map(0x804000, 0x807fff).ram().share("vregs");                                                  // RAM + Vregs
//  map(0xc00000, 0xc0000b).r(FUNC(psikyo_state::input_r));                                        // Depends on board
//  map(0xc00004, 0xc0000b).w(FUNC(psikyo_state::s1945_mcu_w));                                    // MCU on sh404
//  map(0xc00010, 0xc00013).w(m_soundlatch, FUNC(generic_latch_8_device::write));                  // Depends on board
	map(0xfe0000, 0xffffff).ram();                                                                 // RAM
}

template<int Shift>
void psikyo_state::sound_bankswitch_w(u8 data)
{
	m_audiobank->set_entry((data >> Shift) & 0x03);
}

template <uint8_t Which>
void psikyo_state::s1945bl_okibank_w(u8 data)
{
	// not at all sure about this, it seems to write 0 too often
	if (data < 5)
		m_okibank[Which]->set_entry(data);
}

template <uint8_t Which>
void psikyo_state::s1945bl_oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank[Which]);
}

void psikyo_state::s1945bl_bootleg_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                 // ROM (not all used)
	map(0x200000, 0x200fff).ram().share("boot_spritebuf");              // RAM (it copies the spritelist here, the HW probably doesn't have automatic buffering like the originals?

	map(0x400000, 0x401fff).ram().share("spriteram");       // Sprites, buffered by two frames (list buffered + fb buffered)
	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");    // Palette
	map(0x800000, 0x801fff).rw(FUNC(psikyo_state::vram_r<0>), FUNC(psikyo_state::vram_w<0>));      // Layer 0
	map(0x802000, 0x803fff).rw(FUNC(psikyo_state::vram_r<1>), FUNC(psikyo_state::vram_w<1>));      // Layer 1
	map(0x804000, 0x807fff).ram().share("vregs");                                                  // RAM + Vregs
	map(0xc00000, 0xc0000b).r(FUNC(psikyo_state::gunbird_input_r));                                // input ports

	map(0xc00018, 0xc00018).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc00019, 0xc00019).w(FUNC(psikyo_state::s1945bl_okibank_w<0>));

	map(0xfe0000, 0xffffff).ram();                                                                 // RAM

}

// TODO: different video regs / sprite RAM
void psikyo_state::tengaibl_bootleg_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x200fff).ram().share("boot_spritebuf");

	map(0x400000, 0x401fff).ram().share("spriteram");
	map(0x604000, 0x605fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x800000, 0x801fff).rw(FUNC(psikyo_state::vram_r<0>), FUNC(psikyo_state::vram_w<0>));
	map(0x802000, 0x803fff).rw(FUNC(psikyo_state::vram_r<1>), FUNC(psikyo_state::vram_w<1>));
	map(0x804000, 0x807fff).ram().share("vregs");
	map(0xc00000, 0xc0000b).r(FUNC(psikyo_state::gunbird_input_r));

	map(0xc00014, 0xc00014).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc00015, 0xc00015).w(FUNC(psikyo_state::s1945bl_okibank_w<0>));

	map(0xc00018, 0xc00018).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc00019, 0xc00019).w(FUNC(psikyo_state::s1945bl_okibank_w<1>));

	map(0xfe0000, 0xffffff).ram();

}


/***************************************************************************
                        Sengoku Ace / Samurai Aces
***************************************************************************/

u32 psikyo_state::sngkace_input_r(offs_t offset)
{
	switch (offset)
	{
		case 0x0:   return m_in_p1_p2->read();
		case 0x1:   return m_in_dsw->read();
		case 0x2:   return m_in_coin->read();
		default:    logerror("PC %06X - Read input %02X !\n", m_maincpu->pc(), offset * 2);
				return 0;
	}
}

void psikyo_state::sngkace_map(address_map &map)
{
	psikyo_map(map);
	map(0xc00000, 0xc0000b).r(FUNC(psikyo_state::sngkace_input_r));
	map(0xc00013, 0xc00013).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void psikyo_state::sngkace_sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();                         // ROM
	map(0x7800, 0x7fff).ram();                         // RAM
	map(0x8000, 0xffff).bankr("audiobank");                    // Banked ROM
}

void psikyo_state::sngkace_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x04, 0x04).w(FUNC(psikyo_state::sound_bankswitch_w<0>));
	map(0x08, 0x08).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x0c, 0x0c).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}


/***************************************************************************
                                Gun Bird
***************************************************************************/

u32 psikyo_state::gunbird_input_r(offs_t offset)
{
	switch (offset)
	{
		case 0x0:   return m_in_p1_p2->read();
		case 0x1:   return m_in_dsw->read();
		default:    logerror("PC %06X - Read input %02X !\n", m_maincpu->pc(), offset * 2);
				return 0;
	}
}

void psikyo_state::gunbird_map(address_map &map)
{
	psikyo_map(map);
	map(0xc00000, 0xc0000b).r(FUNC(psikyo_state::gunbird_input_r));
	map(0xc00013, 0xc00013).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void psikyo_state::s1945n_map(address_map &map)
{
	psikyo_map(map);
	map(0xc00000, 0xc0000b).r(FUNC(psikyo_state::gunbird_input_r));
	map(0xc00011, 0xc00011).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void psikyo_state::gunbird_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();                         // ROM
	map(0x8000, 0x81ff).ram();                         // RAM
	map(0x8200, 0xffff).bankr("audiobank");                    // Banked ROM
}

void psikyo_state::gunbird_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(psikyo_state::sound_bankswitch_w<4>));
	map(0x04, 0x07).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x08, 0x08).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x0c, 0x0c).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}

/***************************************************************************
                        Strikers 1945 / Tengai
***************************************************************************/

u32 psikyo_state::s1945_input_r(offs_t offset)
{
	switch (offset)
	{
		case 0x0:   return m_in_p1_p2->read();
		case 0x1:   return (m_in_dsw->read() & 0xffff000f) | s1945_mcu_data_r();
		default:    logerror("PC %06X - Read input %02X !\n", m_maincpu->pc(), offset * 2);
					return 0;
	}
}

void psikyo_state::s1945_map(address_map &map)
{
	psikyo_map(map);
	map(0xc00000, 0xc00007).r(FUNC(psikyo_state::s1945_input_r)); // input ports
	map(0xc00006, 0xc00006).w(FUNC(psikyo_state::s1945_mcu_data_w));
	map(0xc00007, 0xc00007).w(FUNC(psikyo_state::s1945_mcu_bctrl_w)); // tile bank switching
	map(0xc00008, 0xc00008).rw(FUNC(psikyo_state::s1945_mcu_control_r), FUNC(psikyo_state::s1945_mcu_control_w));
	map(0xc00009, 0xc00009).w(FUNC(psikyo_state::s1945_mcu_direction_w));
	map(0xc0000b, 0xc0000b).w(FUNC(psikyo_state::s1945_mcu_command_w));
	map(0xc00011, 0xc00011).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void psikyo_state::s1945_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(psikyo_state::sound_bankswitch_w<4>));
	map(0x02, 0x03).nopw();
	map(0x08, 0x0d).rw("ymf", FUNC(ymf278b_device::read), FUNC(ymf278b_device::write));
	map(0x10, 0x10).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x18, 0x18).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}

/***************************************************************************


                                Input Ports


***************************************************************************/

int psikyo_state::z80_nmi_r()
{
	int ret = 0x00;

	if (m_soundlatch->pending_r())
	{
		ret = 0x01;

		/* main CPU might be waiting for sound CPU to finish NMI,
		   so set a timer to give sound CPU a chance to run */
		if (!machine().side_effects_disabled())
			machine().scheduler().synchronize();
//      logerror("%s - Read coin port during Z80 NMI\n", machine().describe_context());
	}

	return ret;
}

static INPUT_PORTS_START( psikyo_common )
	PORT_START("P1_P2")     /* c00000&1 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )   // these depends by the games
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("DSW")       /* c00004 -> c00007 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )   // these depends by the games
	PORT_DIPNAME( 0x00010000, 0x00010000, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(          0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00000000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(          0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c0000, 0x000c0000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(          0x00080000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000c0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00040000, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x00300000, 0x00300000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(          0x00200000, "1" )
	PORT_DIPSETTING(          0x00100000, "2" )
	PORT_DIPSETTING(          0x00300000, "3" )
	PORT_DIPSETTING(          0x00000000, "4" )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00400000, "400K" )
	PORT_DIPSETTING(          0x00000000, "600K" )
	PORT_SERVICE_DIPLOC(  0x00800000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x01000000, 0x01000000, "Coin Slot" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(          0x01000000, "Shared" )
	PORT_DIPSETTING(          0x00000000, "Individual [Free Play]" )
	PORT_DIPNAME( 0x0e000000, 0x0e000000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(          0x0a000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x0c000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x0e000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x08000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x06000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x04000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x02000000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00000000, "1C 6C [Free Play]" )
	PORT_DIPNAME( 0x70000000, 0x70000000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(          0x50000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x60000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x70000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x40000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x30000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x20000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x10000000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00000000, "1C 6C [Free Play]" )
	PORT_DIPNAME( 0x80000000, 0x80000000, "2C Start, 1C Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(          0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, "On [Free Play]" ) // Forces 1C_1C
INPUT_PORTS_END



/***************************************************************************
                        Samurai Aces / Sengoku Ace (Japan)
***************************************************************************/

static INPUT_PORTS_START( samuraia )
	PORT_INCLUDE(psikyo_common)

	PORT_START("COIN")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))    // vblank
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(psikyo_state::z80_nmi_r))   // From Sound CPU
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?

	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	/***********************************************

	This Dip port is bit based:

	Bit 0 1 2 3
	    1 1 1 1 World

	    0 1 1 1 USA With FBI logo
	    1 0 1 1 Korea With FBI logo??
	    1 1 0 1 Hong Kong With FBI logo??
	    1 1 1 0 Taiwan With FBI logo??

	************************************************/
	PORT_CONFNAME( 0x000000ff, 0x000000ff, DEF_STR( Region ) )
	PORT_CONFSETTING(          0x000000ff, DEF_STR( World ) )
	PORT_CONFSETTING(          0x000000ef, "USA & Canada" )
	PORT_CONFSETTING(          0x000000df, DEF_STR( Korea ) )
	PORT_CONFSETTING(          0x000000bf, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(          0x0000007f, DEF_STR( Taiwan ) )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
INPUT_PORTS_END

static INPUT_PORTS_START( sngkace )
	PORT_INCLUDE( samuraia )

	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	/***********************************************

	This Dip port is bit based:

	Bit 0 1 2 3
	    1 1 1 1 Japan

	    0 1 1 1 USA With FBI logo
	    1 0 1 1 Korea
	    1 1 0 1 Hong Kong
	    1 1 1 0 Taiwan

	************************************************/
#if 0 // See Patch in machine_reset, only text not logo
	PORT_CONFNAME( 0x000000ff, 0x000000ff, DEF_STR( Region ) )
	PORT_CONFSETTING(          0x000000ff, DEF_STR( Japan ) )
	PORT_CONFSETTING(          0x000000ef, "USA & Canada" )
	PORT_CONFSETTING(          0x000000df, DEF_STR( Korea ) )
	PORT_CONFSETTING(          0x000000bf, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(          0x0000007f, DEF_STR( Taiwan ) )
#endif
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Battle K-Road

Handicap Mode: When you turn ON the Handicap Mode, if the same player wins
 consecutive matches against other players, the challenger will get a
 handicap. However, it works only when the player challenges again while
 counting down continuous games.

***************************************************************************/

static INPUT_PORTS_START( btlkroad )
	PORT_INCLUDE(psikyo_common)

	PORT_MODIFY("P1_P2")            /* c00000 -> c00003 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(psikyo_state::z80_nmi_r))   // From Sound CPU
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	/***********************************************

	Bit 0 1 2 3
	    1 1 1 1 Japan

	    0 1 1 1 USA & Canada
	    0 0 1 1 Korea
	    0 1 0 1 Hong Kong
	    0 1 1 0 Taiwan
	    Other   World

	************************************************/
	PORT_CONFNAME( 0x0000000f, 0x00000000, DEF_STR( Region ) )
	PORT_CONFSETTING(          0x0000000f, DEF_STR( Japan ) )
	PORT_CONFSETTING(          0x0000000e, "USA & Canada (Jaleco license)" )
	PORT_CONFSETTING(          0x0000000c, DEF_STR( Korea ) )
	PORT_CONFSETTING(          0x0000000a, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(          0x00000006, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(          0x00000000, DEF_STR( World ) )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))   // vblank   ACTIVE_HIGH fixes slowdowns, but is it right?
	// This DSW is used for debugging the game
	PORT_DIPNAME( 0x00000100, 0x00000100, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")   // tested! - So leave as is until filled in
	PORT_DIPSETTING(          0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000200, 0x00000200, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")   // tested! - So leave as is until filled in
	PORT_DIPSETTING(          0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x00000400, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x00000800, IP_ACTIVE_LOW, "SW3:4" )
	PORT_DIPNAME( 0x00001000, 0x00001000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")   // tested! - So leave as is until filled in
	PORT_DIPSETTING(          0x00001000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x00002000, IP_ACTIVE_LOW, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x00004000, IP_ACTIVE_LOW, "SW3:7" )
	PORT_DIPNAME( 0x00008000, 0x00008000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")   // tested! - So leave as is until filled in
	PORT_DIPSETTING(          0x00008000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	PORT_DIPNAME( 0x00100000, 0x00100000, "Blood Effects" )     PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "Handicap Mode" )     PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(          0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "Use DSW 3 (Debug)" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( btlkroadk )
	PORT_INCLUDE( btlkroad )

	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	/***********************************************

	Bit 0 1 2 3
	    1 1 1 1 Japan

	    0 1 1 1 USA & Canada
	    0 0 1 1 Korea
	    0 1 0 1 Hong Kong
	    0 1 1 0 Taiwan
	    Other   World

	************************************************/
	PORT_CONFNAME( 0x0000000f, 0x0000000c, DEF_STR( Region ) ) // Game code supports multi-region, but set default to Korea
	PORT_CONFSETTING(          0x0000000f, DEF_STR( Japan ) )
	PORT_CONFSETTING(          0x0000000e, "USA & Canada (Jaleco license)" )
	PORT_CONFSETTING(          0x0000000c, DEF_STR( Korea ) )
	PORT_CONFSETTING(          0x0000000a, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(          0x00000006, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(          0x00000000, DEF_STR( World ) )
INPUT_PORTS_END


/***************************************************************************
                                Gun Bird
***************************************************************************/

static INPUT_PORTS_START( gunbird )
	PORT_INCLUDE(psikyo_common)

	PORT_MODIFY("P1_P2")            /* c00000 -> c00003 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(psikyo_state::z80_nmi_r))   // From Sound CPU
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	/***********************************************

	This Dip port is bit based:

	Bit 0 1 2 3
	    1 1 1 1 World (No "For use in ...." screen)

	    0 x x x USA With FBI logo
	    1 0 x x Korea
	    1 1 0 x Hong Kong
	    1 1 1 0 Taiwan

	    x = Doesn't Matter, see routine starting at 0108A4:

	Japan is listed in the code but how do you activate it?

	Has no effects on Japan or Korea versions.

	************************************************/
	PORT_CONFNAME( 0x0000000f, 0x0000000f, DEF_STR( Region ) )
	PORT_CONFSETTING(          0x0000000f, DEF_STR( World ) )
	PORT_CONFSETTING(          0x0000000e, DEF_STR( USA ) )
	PORT_CONFSETTING(          0x0000000d, DEF_STR( Korea ) )
	PORT_CONFSETTING(          0x0000000b, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(          0x00000007, DEF_STR( Taiwan ) )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))    // vblank
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )  // tested!
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // tested!
INPUT_PORTS_END

static INPUT_PORTS_START( gunbirdj )
	PORT_INCLUDE( gunbird )

	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	PORT_BIT( 0x0000000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Strikers 1945
***************************************************************************/

static INPUT_PORTS_START( s1945 )
	PORT_INCLUDE(psikyo_common)

	PORT_MODIFY("P1_P2")            /* c00000 -> c00003 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(psikyo_state::mcu_status_r))
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(psikyo_state::z80_nmi_r))   // From Sound CPU
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	/***********************************************

	This Dip port is bit based:

	Bit 0 1 2 3
	    1 1 1 1 World (No "For use in ...." screen), see:
	                    0149B8: tst.w   $fffe58a0.l

	    0 1 1 1 USA & Canada
	    1 0 1 1 Korea
	    1 1 0 1 Hong Kong
	    1 1 1 0 Taiwan
	                Other regions check see:
	                    005594: move.w  $fffe58a0.l, D0

	Came from a Japan board apparently!!!
	Japan is listed in the code but how do you activate it?
	No effect on set s1945j or s1945k

	************************************************/
	PORT_CONFNAME( 0x0000000f, 0x0000000f, DEF_STR( Region ) )
	PORT_CONFSETTING(          0x0000000f, DEF_STR( World ) )
	PORT_CONFSETTING(          0x0000000e, "USA & Canada" )
	PORT_CONFSETTING(          0x0000000d, DEF_STR( Korea ) )
	PORT_CONFSETTING(          0x0000000b, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(          0x00000007, DEF_STR( Taiwan ) )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))    // vblank
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )  // tested!
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // tested!

	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00400000, "600K" )
	PORT_DIPSETTING(          0x00000000, "800K" )

	PORT_DIPNAME( 0x01000000, 0x01000000, "Coin Slot" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(          0x01000000, "Shared" )
	PORT_DIPSETTING(          0x00000000, "Individual" )
	PORT_DIPNAME( 0x0e000000, 0x0e000000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(          0x0a000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x0c000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x0e000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x08000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x06000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x04000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x02000000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x70000000, 0x70000000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(          0x50000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x60000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x70000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x40000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x30000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x20000000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x10000000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80000000, 0x80000000, "2C Start, 1C Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(          0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) ) // Forces 1C_1C
INPUT_PORTS_END

static INPUT_PORTS_START( s1945a )
	PORT_INCLUDE( s1945 )

	PORT_MODIFY("DSW")
	/***********************************************

	This Dip port is bit based:

	Bit 0 1 2 3
	    1 1 1 1 Japan, anything but 0x0f = "World"
	************************************************/
	PORT_CONFNAME( 0x0000000f, 0x0000000f, DEF_STR( Region ) )
	PORT_CONFSETTING(          0x0000000f, DEF_STR( Japan ) )
	PORT_CONFSETTING(          0x0000000e, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( s1945j )
	PORT_INCLUDE( s1945 )

	PORT_MODIFY("DSW")
	PORT_BIT( 0x0000000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( s1945bl )
	PORT_INCLUDE( s1945 )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// I need to invert the Vblank on this to avoid excessive slowdown
	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))   // vblank
INPUT_PORTS_END


/***************************************************************************
                                Tengai
***************************************************************************/

static INPUT_PORTS_START( tengai )
	PORT_INCLUDE(psikyo_common)

	PORT_MODIFY("P1_P2")            /* c00000 -> c00003 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(psikyo_state::mcu_status_r))
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(psikyo_state::z80_nmi_r))   // From Sound CPU
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")      /* c00004 -> c00007 */
	/***********************************************

	This Dip port is bit based:

	Bit 0 1 2 3
	    1 1 1 1 World
	    0 1 1 1 USA & Canada
	    1 0 1 1 Korea
	    1 1 0 1 Hong Kong
	    1 1 1 0 Taiwan

	************************************************/
	PORT_CONFNAME( 0x0000000f, 0x0000000f, DEF_STR( Region ) )
	PORT_CONFSETTING(          0x0000000f, DEF_STR( World ) )
	PORT_CONFSETTING(          0x0000000e, "USA & Canada" )
	PORT_CONFSETTING(          0x0000000d, DEF_STR( Korea ) )
	PORT_CONFSETTING(          0x0000000b, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(          0x00000007, DEF_STR( Taiwan ) )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))    // vblank
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )  // tested!
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // tested!

	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00400000, "600K" )
	PORT_DIPSETTING(          0x00000000, "800K" )
INPUT_PORTS_END

static INPUT_PORTS_START( tengaij )
	PORT_INCLUDE( tengai )

	PORT_MODIFY("DSW")
	/***********************************************

	This Dip port is bit based:

	Bit 0 1 2 3
	    1 1 1 1 Japan, anything but 0x0f = "World"

	Text for other regions is present though.

	************************************************/
	PORT_CONFNAME( 0x0000000f, 0x0000000f, DEF_STR( Region ) )
	PORT_CONFSETTING(          0x0000000f, DEF_STR( Japan ) )
	PORT_CONFSETTING(          0x0000000e, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tengaibl )
	PORT_INCLUDE( tengai )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************


                                Gfx Layouts


***************************************************************************/

static GFXDECODE_START( gfx_psikyo )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_packed_msb, 0x000, 0x20 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_msb, 0x800, 0x48 ) // [1] Layer 0 + 1
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/

void psikyo_state::machine_start()
{
	// assumes it can make an address mask with m_spritelut.length() - 1
	assert(!(m_spritelut.length() & (m_spritelut.length() - 1)));

	save_item(NAME(m_mcu_status));
	save_item(NAME(m_tilemap_bank));
}

void psikyo_state::machine_reset()
{
	m_mcu_status = 0;
}


/***************************************************************************
                            Samurai Ace / Sengoku Aces
***************************************************************************/


void psikyo_state::sngkace(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, XTAL(32'000'000)/2); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyo_state::sngkace_map);
	m_maincpu->set_vblank_int("screen", FUNC(psikyo_state::irq4_line_hold));

	Z80(config, m_audiocpu, XTAL(32'000'000)/8); /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &psikyo_state::sngkace_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &psikyo_state::sngkace_sound_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 320, 262, 0, 224);  // Approximately 59.923Hz, 38 Lines in VBlank
	m_screen->set_screen_update(FUNC(psikyo_state::screen_update));
	m_screen->screen_vblank().set(FUNC(psikyo_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_psikyo);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000);

	BUFFERED_SPRITERAM32(config, m_spriteram);
	MCFG_VIDEO_START_OVERRIDE(psikyo_state,sngkace)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(32'000'000)/4)); /* verified on pcb */
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);
}



/***************************************************************************
        Gun Bird / Battle K-Road / Strikers 1945 (Japan, unprotected)
***************************************************************************/


void psikyo_state::gunbird(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, 16_MHz_XTAL);  // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyo_state::gunbird_map);
	m_maincpu->set_vblank_int("screen", FUNC(psikyo_state::irq4_line_hold));

	LZ8420M(config, m_audiocpu, 16_MHz_XTAL / 2);  // 8 MHz (16 / 2)
	m_audiocpu->set_addrmap(AS_PROGRAM, &psikyo_state::gunbird_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &psikyo_state::gunbird_sound_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 320, 262, 0, 224);  // Approximately 59.923Hz, 38 Lines in VBlank
	m_screen->set_screen_update(FUNC(psikyo_state::screen_update));
	m_screen->screen_vblank().set(FUNC(psikyo_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_psikyo);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000);

	BUFFERED_SPRITERAM32(config, m_spriteram);
	MCFG_VIDEO_START_OVERRIDE(psikyo_state,psikyo)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16_MHz_XTAL / 2));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono",  0.55);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);
}

void psikyo_state::s1945n(machine_config &config)
{
	gunbird(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyo_state::s1945n_map);
}

void psikyo_state::s1945bl(machine_config &config) /* Bootleg hardware based on the unprotected Japanese Strikers 1945 set */
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyo_state::s1945bl_bootleg_map);
	m_maincpu->set_vblank_int("screen", FUNC(psikyo_state::irq1_line_hold));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: accurate measurements
	m_screen->set_refresh_hz(59.3);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(320, 256);
	m_screen->set_visarea(0, 320-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(psikyo_state::screen_update));
	m_screen->screen_vblank().set(FUNC(psikyo_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_psikyo);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000);

	BUFFERED_SPRITERAM32(config, m_spriteram);
	MCFG_VIDEO_START_OVERRIDE(psikyo_state,psikyo)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki1(OKIM6295(config, "oki1", XTAL(16'000'000)/16, okim6295_device::PIN7_LOW)); // ?? clock
	oki1.add_route(ALL_OUTPUTS, "mono", 1.0);
	oki1.set_addrmap(0, &psikyo_state::s1945bl_oki_map<0>);
}

void psikyo_state::tengaibl(machine_config &config)
{
	s1945bl(config);

	m_maincpu->set_clock(28.636363_MHz_XTAL / 2 ); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyo_state::tengaibl_bootleg_map);

	okim6295_device &oki1(OKIM6295(config.replace(), "oki1", 1'000'000, okim6295_device::PIN7_LOW)); // clock and pin 7 not verified
	oki1.add_route(ALL_OUTPUTS, "mono", 1.0);
	oki1.set_addrmap(0, &psikyo_state::s1945bl_oki_map<0>);

	okim6295_device &oki2(OKIM6295(config, "oki2", 1'000'000, okim6295_device::PIN7_LOW)); // clock and pin 7 not verified
	oki2.add_route(ALL_OUTPUTS, "mono", 1.0);
	oki2.set_addrmap(0, &psikyo_state::s1945bl_oki_map<1>);
}



/***************************************************************************
                        Strikers 1945 / Tengai
***************************************************************************/

void psikyo_state::s1945(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, 16_MHz_XTAL);  // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyo_state::s1945_map);
	m_maincpu->set_vblank_int("screen", FUNC(psikyo_state::irq4_line_hold));

	LZ8420M(config, m_audiocpu, 16_MHz_XTAL / 2);  // 8 MHz (16 / 2)
	m_audiocpu->set_addrmap(AS_PROGRAM, &psikyo_state::gunbird_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &psikyo_state::s1945_sound_io_map);

	/* Dumped by decapping on a Tengai PCB (and there's one weirdly sized dump available from a Korean version of Strikers 1945).
	   TODO: verify it's good and hook it up. Verify if the same PIC dump works also on clones. */
	PIC16C57(config, "mcu", 4_MHz_XTAL).set_disable();

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 320, 262, 0, 224);  // Approximately 59.923Hz, 38 Lines in VBlank
	m_screen->set_screen_update(FUNC(psikyo_state::screen_update));
	m_screen->screen_vblank().set(FUNC(psikyo_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_psikyo);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000);

	BUFFERED_SPRITERAM32(config, m_spriteram);
	MCFG_VIDEO_START_OVERRIDE(psikyo_state,psikyo)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ymf278b_device &ymf(YMF278B(config, "ymf", 33.8688_MHz_XTAL)); // YMF268-K instead on some PCBs
	ymf.irq_handler().set_inputline(m_audiocpu, 0);
	ymf.add_route(ALL_OUTPUTS, "mono", 1.0);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);
}


/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

                                Samurai Aces
                          ( WORLD/USA/HK/KOREA/TAIWAN Ver.)

                                Sengoku Ace
                          (Samurai Aces JPN Ver.)

Board:  SH201B
CPU:    TMP68EC020F-16
Sound:  Z80A + YM2610
OSC:    32.000, 14.31818 MHz

***************************************************************************/

ROM_START( samuraia )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4-u127.bin", 0x000000, 0x040000, CRC(8c9911ca) SHA1(821ba648b9a1d495c600cbf4606f2dbddc6f9e6f) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5-u126.bin", 0x000002, 0x040000, CRC(d20c3ef0) SHA1(264e5a7e45e130a9e7152468733337668dc5b65f) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u58.bin", 0x00000, 0x20000, CRC(310f5c76) SHA1(dbfd1c5a7a514bccd89fc4f7191744cf76bb745d) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u14.bin",  0x000000, 0x200000, CRC(00a546cb) SHA1(30a8679b49928d5fcbe58b5eecc2ebd08173adf8) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layers 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x100000, CRC(e6a75bd8) SHA1(1aa84ea54584b6c8b2846194b48bf6d2afa67fee) )
	ROM_LOAD16_WORD_SWAP( "u35.bin",  0x100000, 0x100000, CRC(c4ca0164) SHA1(c75422de2e0127cdc23d8c223b674a5bd85b00fb) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* Samples */
	ROM_LOAD( "u68.bin",  0x000000, 0x100000, CRC(9a7f6c34) SHA1(c549b209bce1d2c6eeb512db198ad20c3f5fb0ea) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* Sprites LUT */
	ROM_LOAD( "u11.bin",  0x000000, 0x040000, CRC(11a04d91) SHA1(5d146a9a39a70f2ee212ceab9a5469598432449e) ) // x1xxxxxxxxxxxxxxxx = 0xFF
ROM_END

ROM_START( sngkace )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1-u127.bin", 0x000000, 0x040000, CRC(6c45b2f8) SHA1(08473297e174f3a6d67043f3b16f4e6b9c68b826) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2-u126.bin", 0x000002, 0x040000, CRC(845a6760) SHA1(3b8fed294e28d9d8ef5cb5ec88b9ade396146a48) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u58.bin", 0x00000, 0x20000, CRC(310f5c76) SHA1(dbfd1c5a7a514bccd89fc4f7191744cf76bb745d) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u14.bin",  0x000000, 0x200000, CRC(00a546cb) SHA1(30a8679b49928d5fcbe58b5eecc2ebd08173adf8) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layers 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x100000, CRC(e6a75bd8) SHA1(1aa84ea54584b6c8b2846194b48bf6d2afa67fee) )
	ROM_LOAD16_WORD_SWAP( "u35.bin",  0x100000, 0x100000, CRC(c4ca0164) SHA1(c75422de2e0127cdc23d8c223b674a5bd85b00fb) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* Samples */
	ROM_LOAD( "u68.bin",  0x000000, 0x100000, CRC(9a7f6c34) SHA1(c549b209bce1d2c6eeb512db198ad20c3f5fb0ea) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* Sprites LUT */
	ROM_LOAD( "u11.bin",  0x000000, 0x040000, CRC(11a04d91) SHA1(5d146a9a39a70f2ee212ceab9a5469598432449e) ) // x1xxxxxxxxxxxxxxxx = 0xFF
ROM_END


ROM_START( sngkacea ) // the roms have a very visible "." symbol after the number, it might indicate a newer revision.
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1.-u127.bin", 0x000000, 0x040000, CRC(3a43708d) SHA1(f38d22304f8957c6d81c8946a8a03676965d4dd4) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2.-u126.bin", 0x000002, 0x040000, CRC(7aa50c46) SHA1(bad250f64c6d796a61be5e2eb71e2f5774f4278e) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u58.bin", 0x00000, 0x20000, CRC(310f5c76) SHA1(dbfd1c5a7a514bccd89fc4f7191744cf76bb745d) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u14.bin",  0x000000, 0x200000, CRC(00a546cb) SHA1(30a8679b49928d5fcbe58b5eecc2ebd08173adf8) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layers 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x100000, CRC(e6a75bd8) SHA1(1aa84ea54584b6c8b2846194b48bf6d2afa67fee) )
	ROM_LOAD16_WORD_SWAP( "u35.bin",  0x100000, 0x100000, CRC(c4ca0164) SHA1(c75422de2e0127cdc23d8c223b674a5bd85b00fb) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* Samples */
	ROM_LOAD( "u68.bin",  0x000000, 0x100000, CRC(9a7f6c34) SHA1(c549b209bce1d2c6eeb512db198ad20c3f5fb0ea) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* Sprites LUT */
	ROM_LOAD( "u11.bin",  0x000000, 0x040000, CRC(11a04d91) SHA1(5d146a9a39a70f2ee212ceab9a5469598432449e) ) // x1xxxxxxxxxxxxxxxx = 0xFF
ROM_END


/***************************************************************************

                                Gun Bird (Korea)
                                Gun Bird (Japan)
                            Battle K-Road (Japan)

Board:  KA302C
CPU:    MC68EC020FG16
Sound:  LZ8420M (Z80 core) + YMF286-K
OSC:    16.000, 14.31818 MHz

Chips:  PS2001B
        PS3103
        PS3204
        PS3305

***************************************************************************/

ROM_START( gunbird )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4.u46", 0x000000, 0x040000, CRC(b78ec99d) SHA1(399b79931652d9df1632cd4d7ec3d214e473a5c3) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5.u39", 0x000002, 0x040000, CRC(925f095d) SHA1(301a536119a0320a756e9c6e51fb10e36b90ef16) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3.u71",     0x00000, 0x20000, CRC(2168e4ba) SHA1(ca7ad6acb5f806ce2528e7b52c19e8cceecb8543) )

	ROM_REGION( 0x700000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u14.bin",  0x000000, 0x200000, CRC(7d7e8a00) SHA1(9f35f5b54ae928e9bf2aa6ad4604f669857955ec) )
	ROM_LOAD16_WORD_SWAP( "u24.bin",  0x200000, 0x200000, CRC(5e3ffc9d) SHA1(c284eb9ef56c8e6261fe11f91a10c5c5a56c9803) )
	ROM_LOAD16_WORD_SWAP( "u15.bin",  0x400000, 0x200000, CRC(a827bfb5) SHA1(6e02436e12085016cf1982c9ae07b6c6dec82f1b) )
	ROM_LOAD16_WORD_SWAP( "u25.bin",  0x600000, 0x100000, CRC(ef652e0c) SHA1(6dd994a15ced31d1bbd1a3b0e9d8d86eca33e217) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layers 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u33.bin",  0x000000, 0x200000, CRC(54494e6b) SHA1(f5d090d2d34d908b56b53a246def194929eba990) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(9e07104d) SHA1(3bc54cb755bb3194197706965b532d62b48c4d12) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(e187ed4f) SHA1(05060723d89b1d05714447a14b5f5888ff3c2306) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(0905aeb2) SHA1(8cca09f7dfe3f804e77515f7b1b1bdbeb7bb3d80) )

	ROM_REGION( 0x0002, "pals", 0 )
	ROM_LOAD( "3021.u69", 0x0000, 0x0001, NO_DUMP ) /* TIBPAL16L8-15CN */
	ROM_LOAD( "3020.u19", 0x0000, 0x0001, NO_DUMP ) /* TIBPAL16L8-15CN */
ROM_END

ROM_START( gunbirdk )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1k.u46", 0x000000, 0x080000, CRC(745cee52) SHA1(6c5bb92c92c55f882484417bc1aa580684019610) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2k.u39", 0x000002, 0x080000, CRC(669632fb) SHA1(885dea42e6da35e9166a208b18dbd930642c26cd) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "k3.u71",    0x00000, 0x20000, CRC(11994055) SHA1(619776c178361f23de37ff14e87284ec0f1f4f10) )

	ROM_REGION( 0x700000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u14.bin",  0x000000, 0x200000, CRC(7d7e8a00) SHA1(9f35f5b54ae928e9bf2aa6ad4604f669857955ec) )
	ROM_LOAD16_WORD_SWAP( "u24.bin",  0x200000, 0x200000, CRC(5e3ffc9d) SHA1(c284eb9ef56c8e6261fe11f91a10c5c5a56c9803) )
	ROM_LOAD16_WORD_SWAP( "u15.bin",  0x400000, 0x200000, CRC(a827bfb5) SHA1(6e02436e12085016cf1982c9ae07b6c6dec82f1b) )
	ROM_LOAD16_WORD_SWAP( "u25.bin",  0x600000, 0x100000, CRC(ef652e0c) SHA1(6dd994a15ced31d1bbd1a3b0e9d8d86eca33e217) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layers 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u33.bin",  0x000000, 0x200000, CRC(54494e6b) SHA1(f5d090d2d34d908b56b53a246def194929eba990) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(9e07104d) SHA1(3bc54cb755bb3194197706965b532d62b48c4d12) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(e187ed4f) SHA1(05060723d89b1d05714447a14b5f5888ff3c2306) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(0905aeb2) SHA1(8cca09f7dfe3f804e77515f7b1b1bdbeb7bb3d80) )
ROM_END

ROM_START( gunbirdj )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1.u46", 0x000000, 0x040000, CRC(474abd69) SHA1(27f37333075f9c92849101aad4875e69004d747b) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2.u39", 0x000002, 0x040000, CRC(3e3e661f) SHA1(b5648546f390539b0f727a9a62d1b9516254ae21) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3.u71",     0x00000, 0x20000, CRC(2168e4ba) SHA1(ca7ad6acb5f806ce2528e7b52c19e8cceecb8543) )

	ROM_REGION( 0x700000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u14.bin",  0x000000, 0x200000, CRC(7d7e8a00) SHA1(9f35f5b54ae928e9bf2aa6ad4604f669857955ec) )
	ROM_LOAD16_WORD_SWAP( "u24.bin",  0x200000, 0x200000, CRC(5e3ffc9d) SHA1(c284eb9ef56c8e6261fe11f91a10c5c5a56c9803) )
	ROM_LOAD16_WORD_SWAP( "u15.bin",  0x400000, 0x200000, CRC(a827bfb5) SHA1(6e02436e12085016cf1982c9ae07b6c6dec82f1b) )
	ROM_LOAD16_WORD_SWAP( "u25.bin",  0x600000, 0x100000, CRC(ef652e0c) SHA1(6dd994a15ced31d1bbd1a3b0e9d8d86eca33e217) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layers 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u33.bin",  0x000000, 0x200000, CRC(54494e6b) SHA1(f5d090d2d34d908b56b53a246def194929eba990) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(9e07104d) SHA1(3bc54cb755bb3194197706965b532d62b48c4d12) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(e187ed4f) SHA1(05060723d89b1d05714447a14b5f5888ff3c2306) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(0905aeb2) SHA1(8cca09f7dfe3f804e77515f7b1b1bdbeb7bb3d80) )
ROM_END


ROM_START( btlkroad )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4-u46.bin", 0x000000, 0x040000, CRC(8a7a28b4) SHA1(f7197be673dfd0ddf46998af81792b81d8fe9fbf) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5-u39.bin", 0x000002, 0x040000, CRC(933561fa) SHA1(f6f3b1e14b1cfeca26ef8260ac4771dc1531c357) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u71.bin", 0x00000, 0x20000, CRC(22411fab) SHA1(1094cb51712e40ae65d0082b408572bdec06ae8b) )

	ROM_REGION( 0x700000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u14.bin",  0x000000, 0x200000, CRC(282d89c3) SHA1(3b4b17f4a37efa2f7e232488aaba7c77d10c84d2) )
	ROM_LOAD16_WORD_SWAP( "u24.bin",  0x200000, 0x200000, CRC(bbe9d3d1) SHA1(9da0b0b993e8271a8119e9c2f602e52325983f79) )
	ROM_LOAD16_WORD_SWAP( "u15.bin",  0x400000, 0x200000, CRC(d4d1b07c) SHA1(232109db8f6e137fbc8826f38a96057067cb19dc) )
//  ROM_LOAD16_WORD_SWAP( "u25.bin",  0x600000, 0x100000  NOT PRESENT

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layers 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u33.bin",  0x000000, 0x200000, CRC(4c8577f1) SHA1(d27043514632954a06667ac63f4a4e4a31870511) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(51d73682) SHA1(562038d08e9a4389ffa39f3a659b2a29b94dc156) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(0f33049f) SHA1(ca4fd5f3906685ace1af40b75f5678231d7324e8) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(30d541ed) SHA1(6f7fb5f5ecbce7c086185392de164ebb6887e780) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "tibpal16l8.u69", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "tibpal16l8.u19", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END


ROM_START( btlkroadk )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4,dot.u46", 0x000000, 0x040000, CRC(e724d429) SHA1(8b5f80366fd22d6f7e7d8a9623de4fe231303267) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5,dot.u39", 0x000002, 0x040000, CRC(c0d65765) SHA1(a6a26e6b9693a2ef245e9aaa4c9daa888aebb360) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3,k.u71", 0x00000, 0x20000, CRC(e0f0c597) SHA1(cc337633f1f579baf0f8ba1dd65c5d51122a7e97) )

	ROM_REGION( 0x700000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u14.bin",  0x000000, 0x200000, CRC(282d89c3) SHA1(3b4b17f4a37efa2f7e232488aaba7c77d10c84d2) )
	ROM_LOAD16_WORD_SWAP( "u24.bin",  0x200000, 0x200000, CRC(bbe9d3d1) SHA1(9da0b0b993e8271a8119e9c2f602e52325983f79) )
	ROM_LOAD16_WORD_SWAP( "u15.bin",  0x400000, 0x200000, CRC(d4d1b07c) SHA1(232109db8f6e137fbc8826f38a96057067cb19dc) )
//  ROM_LOAD16_WORD_SWAP( "u25.bin",  0x600000, 0x100000  NOT PRESENT

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layers 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u33.bin",  0x000000, 0x200000, CRC(4c8577f1) SHA1(d27043514632954a06667ac63f4a4e4a31870511) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(51d73682) SHA1(562038d08e9a4389ffa39f3a659b2a29b94dc156) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(0f33049f) SHA1(ca4fd5f3906685ace1af40b75f5678231d7324e8) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* Sprites LUT */
	ROM_LOAD( "u3.bin",  0x000000, 0x040000, CRC(30d541ed) SHA1(6f7fb5f5ecbce7c086185392de164ebb6887e780) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "tibpal16l8.u69", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "tibpal16l8.u19", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

/***************************************************************************

                            Strikers 1945 (Japan, unprotected)

Board:  SH403 (Similiar to KA302C)
CPU:    MC68EC020FG16
Sound:  LZ8420M (Z80 core) + YMF286-K?
OSC:    16.000, 14.31818 MHz

Chips:  PS2001B
        PS3103
        PS3204
        PS3305

***************************************************************************/

ROM_START( s1945n )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4.u46", 0x000000, 0x040000, CRC(28fb8181) SHA1(6d3cc6b6fdb0f8c0f92e69c064d62ffcffbfb031) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5.u39", 0x000002, 0x040000, CRC(8ca05f94) SHA1(fc6256fcf6bfc6f7c42f9cdc97bc025e5785d758) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u71.bin", 0x00000, 0x20000, CRC(e3e366bd) SHA1(1f5b5909745802e263a896265ea365df76d3eaa5) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD16_WORD_SWAP( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD16_WORD_SWAP( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD16_WORD_SWAP( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(fe1312c2) SHA1(8339a96a0885518d6e22cb3bdb9c2f82d011d86d) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(a44a4a9b) SHA1(5378256752d709daed0b5f4199deebbcffe84e10) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )
ROM_END

ROM_START( s1945nj )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1-u46.bin", 0x000000, 0x040000, CRC(95028132) SHA1(6ed8e53efb0511dca37c35a7da17217f5aa83734) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2-u39.bin", 0x000002, 0x040000, CRC(3df79a16) SHA1(6184f27579d846a40313fd11b57c46ac0d02fc76) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u71.bin", 0x00000, 0x20000, CRC(e3e366bd) SHA1(1f5b5909745802e263a896265ea365df76d3eaa5) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD16_WORD_SWAP( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD16_WORD_SWAP( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD16_WORD_SWAP( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM Samples */
	ROM_LOAD( "u56.bin",  0x000000, 0x100000, CRC(fe1312c2) SHA1(8339a96a0885518d6e22cb3bdb9c2f82d011d86d) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* DELTA-T Samples */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(a44a4a9b) SHA1(5378256752d709daed0b5f4199deebbcffe84e10) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )
ROM_END

ROM_START( s1945bl ) /* closely based on s1945nj set, unsurprising because it's unprotected */
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_BYTE( "27c010-1", 0x000000, 0x020000, CRC(d3361536) SHA1(430df1c98645603c17333222834d344efd4fb584) ) // 1-u46.bin    [odd 1/2]  99.797821%
	ROM_LOAD32_BYTE( "27c010-2", 0x000001, 0x020000, CRC(1d1916b1) SHA1(4e200454c16d0bd45c4146ee41902a811a55c008) ) // 1-u46.bin    [even 1/2] 99.793243%
	ROM_LOAD32_BYTE( "27c010-3", 0x000002, 0x020000, CRC(391e0387) SHA1(5c5c737629a450e8d07b088ad50280dae57aeded) ) // 2-u39.bin    [odd 1/2]  99.749756%
	ROM_LOAD32_BYTE( "27c010-4", 0x000003, 0x020000, CRC(2aebcf6b) SHA1(2aea1c5edc006f70c21d84b581a48082ec111f6a) ) // 2-u39.bin    [even 1/2] 99.743652%

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	// same content as original sets, alt rom layout
	ROM_LOAD( "rv27c3200.m4",  0x000000, 0x400000, CRC(70c8f72e) SHA1(90d25f4ecd6bfe72b51713099625f643b12aa674) )
	ROM_LOAD( "rv27c3200.m3",  0x400000, 0x400000, CRC(0dec2a8d) SHA1(b2f3143f2be50c825b61d5218cec26ba8ed1f07e) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "rv27c1600.m1",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x100000, "oki1", 0 )    /* OKI Samples */
	ROM_LOAD( "rv27c040.m6",  0x000000, 0x080000, CRC(c22e5b65) SHA1(d807bd7c136d6b51f54258b44ebf3eecbd5b35fa) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* */
	// in 27c010-a 0x460E:01 but should 0x00. Might be bit rot or error when bootleggers copied U1 as all other graphics data matches.
	ROM_LOAD16_BYTE( "27c010-b",  0x000000, 0x020000, CRC(e38d5ab7) SHA1(73a708ebc305cb6297efd3296da23c87898e805e) ) // u1.bin  [even]  IDENTICAL
	ROM_LOAD16_BYTE( "27c010-a",  0x000001, 0x020000, CRC(cb8c65ec) SHA1(a55c5c5067b50a1243e7ba60fa1f9569bfed5de8) ) // u1.bin  [odd]   99.999237%

	ROM_REGION( 0x080000, "unknown", 0 )    /* unknown - matches Semicom's Dream World */
	ROM_LOAD( "27c512",  0x000000, 0x010000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/***************************************************************************

Strikers 1945
1995, Psikyo

PCB Layout
----------

SH404
|----------------------------------------------------------|
|HA1388  2058D   62256 6264  62256  16MHz PAL1 * 3.U63 U61 |
|       YAC513-M 62256 62256 6264   14.31818MHz  33.8688MHz|
|  VOL                                                     |
|                         |------|62256 |-------||-------| |
|      4MHz   LH5168      |PS3204|62256 |LZ8420M||YMF278B| |
|     16C57   LH5168      |------|      |-------||-------| |
|         PAL2                                             |
|                                                          |
|J                                          |--------|     |
|A           |------| 5.U41   62256  62256  |        |     |
|M           |68020 | 4.U40   62256  62256  | PS3103 | U34 |
|M           |------|                       |        |     |
|A  |------|                                |--------|     |
|   |PS3305|                                               |
|   |------|              |--------|                       |
|                         |        |      U23 U22 U21 U20  |
| MB3771                  |PS2001B |                       |
|    6264   62256 62256   |        |                       |
|    6264   62256 62256   |--------|                       |
|                                                          |
|DIP1                     6116  6116                       |
|DIP2                     6116  6116        LH5168     U1  |
|----------------------------------------------------------|
Notes:
      68020    - Motorola MC68EC020 CPU (QFP100, running at 16.000MHz)
      PS3305   - Psikyo Custom (QFP100)
      PS3204   - Psikyo Custom (QFP100)
      PS2001B  - Psikyo Custom (QFP160)
      PS3103   - Psikyo Custom (QFP160)
      6116     - 2k x8 SRAM (x4, DIP24)
      LH5168   - Sharp LH5168 8k x8 SRAM (x3, DIP28)
      62256    - NKK N341256 32k x8 SRAM (x14, DIP28)
      6264     - ISSI IS61C64 8k x8 SRAM (x4, DIP28)
      MB3771   - Fujitsu MB3771 Master Reset IC (DIP8)
      DIP1     - 8 position DIP Switch
      DIP2     - 8 position DIP Switch
      PAL1     - AMD PALCE 16V8H (DIP20, stamped '4041')
      PAL2     - AMD PALCE 16V8H (DIP20, stamped '4040')
      YMF278B  - Yamaha YMF278B-F Sound Chip, running at 33.8688MHz (QFP64)
      3.U63    - Macronix MX27C1000 128k x8 EPROM (Sound Program, DIP32, labelled '3')
      4.U40    - AMD AM27C2048 256k x16 EPROM (Main Program, DIP40, labelled '4' & '5')
      5.U41    /
      16C57    - Microchip PIC16C57 Microcontroller (DIP24, running at 4.000MHz, labelled '1')
      YAC513   - Yamaha YAC513-M D/A Converter (SOIC16)
      LZ8420M  - Sharp LZ8420M Z80-core CPU (QFP64, running at 4.000MHz)
      HA1388   - Hitachi HA1388 Sound Amp
      2058D    - JRC 2058 Op Amp
      U61, U34,\
      U20, U21,- 16M mask ROM (SOP44)
      U22, U23 /
      U1       - 4M mask ROM (SOP44)
      VOL      - Master Volume Potentiometer
      *        - Unpopulated position for 16M SOP44 mask ROM


CPU:    MC68EC020FG16
OSC:    16.000MHz
        14.3181MHz
        33.8688MHz (YMF)
        4.000MHz (PIC)
SYNCS:  HSync 15.700kHz, VSync 59.923Hz
        HSync most likely derived from 14.3181MHz OSC (divided by 912)
        262 lines per frame consisting of:
          - Visible lines: 224
          - VBlank lines: 38 (Front/Back porch: 15 lines, VSync pulse: 8 lines)
1-U59   security (PIC16C57; not dumped)

***************************************************************************/

ROM_START( s1945 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "2s.u40", 0x000000, 0x040000, CRC(9b10062a) SHA1(cf963bba157422b659d8d64b4493eb7d69cd07b7) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "3s.u41", 0x000002, 0x040000, CRC(f87e871a) SHA1(567167c7fcfb622f78e211d74b04060c3d29d6b7) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u63.bin", 0x00000, 0x20000, CRC(42d40ae1) SHA1(530a5a3f78ac489b84a631ea6ce21010a4f4d31b) )

	ROM_REGION( 0x001000, "mcu", 0 )       /* MCU? */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x01000, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD16_WORD_SWAP( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD16_WORD_SWAP( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD16_WORD_SWAP( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x400000, "ymf", 0 )    /* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a839cf47) SHA1(e179eb505c80d5bb3ccd9e228f2cf428c62b72ee) )    // 8 bit signed pcm (16KHz)

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )
ROM_END

ROM_START( s1945a )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "4-u40.bin", 0x000000, 0x040000, CRC(29ffc217) SHA1(12dc3cb32253c3908f4c440c627a0e1b32ee7cac) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "5-u41.bin", 0x000002, 0x040000, CRC(c3d3fb64) SHA1(4388586bc0a6f3d62366b3c38b8b23f8a03dbf15) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u63.bin", 0x00000, 0x20000, CRC(42d40ae1) SHA1(530a5a3f78ac489b84a631ea6ce21010a4f4d31b) )

	ROM_REGION( 0x001000, "mcu", 0 )       /* MCU? */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x01000, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD16_WORD_SWAP( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD16_WORD_SWAP( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD16_WORD_SWAP( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x400000, "ymf", 0 )    /* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a839cf47) SHA1(e179eb505c80d5bb3ccd9e228f2cf428c62b72ee) )    // 8 bit signed pcm (16KHz)

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )
ROM_END

ROM_START( s1945j )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "1-u40.bin", 0x000000, 0x040000, CRC(c00eb012) SHA1(080dae010ca83548ebdb3324585d15e48baf0541) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "2-u41.bin", 0x000002, 0x040000, CRC(3f5a134b) SHA1(18bb3bb1e1adadcf522795f5cf7d4dc5a5dd1f30) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u63.bin", 0x00000, 0x20000, CRC(42d40ae1) SHA1(530a5a3f78ac489b84a631ea6ce21010a4f4d31b) )

	ROM_REGION( 0x001000, "mcu", 0 )       /* MCU */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x01000, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD16_WORD_SWAP( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD16_WORD_SWAP( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD16_WORD_SWAP( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x400000, "ymf", 0 )    /* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a839cf47) SHA1(e179eb505c80d5bb3ccd9e228f2cf428c62b72ee) )    // 8 bit signed pcm (16KHz)

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )
ROM_END

ROM_START( s1945k ) /* Same MCU as the current parent set, region dip has no effect on this set */
	ROM_REGION( 0x100000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD32_WORD_SWAP( "10.u40", 0x000000, 0x040000, CRC(5a32af36) SHA1(2eada37fd043c097a11bcf4e3e0bebb473bbc0df) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "9.u41",  0x000002, 0x040000, CRC(29cc6d7d) SHA1(aeee9e88922c25c75885483d115e064c6b71540b) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       /* Sound CPU Code */
	ROM_LOAD( "3-u63.bin", 0x00000, 0x20000, CRC(42d40ae1) SHA1(530a5a3f78ac489b84a631ea6ce21010a4f4d31b) )

	ROM_REGION( 0x001000, "mcu", 0 )       /* MCU */
	ROM_LOAD( "4-u59.bin", 0x00000, 0x01000, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_WORD_SWAP( "u20.bin",  0x000000, 0x200000, CRC(28a27fee) SHA1(913f3bc4d0c6fb6b776a020c8099bf96f16fd06f) )
	ROM_LOAD16_WORD_SWAP( "u22.bin",  0x200000, 0x200000, CRC(ca152a32) SHA1(63efee83cb5982c77ca473288b3d1a96b89e6388) )
	ROM_LOAD16_WORD_SWAP( "u21.bin",  0x400000, 0x200000, CRC(c5d60ea9) SHA1(e5ce90788211c856172e5323b01b2c7ab3d3fe50) )
	ROM_LOAD16_WORD_SWAP( "u23.bin",  0x600000, 0x200000, CRC(48710332) SHA1(db38b732a09b31ce55a96ec62987baae9b7a00c1) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* Layer 0 + 1 */
	ROM_LOAD16_WORD_SWAP( "u34.bin",  0x000000, 0x200000, CRC(aaf83e23) SHA1(1c75d09ff42c0c215f8c66c699ca75688c95a05e) )

	ROM_REGION( 0x400000, "ymf", 0 )    /* Samples */
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a839cf47) SHA1(e179eb505c80d5bb3ccd9e228f2cf428c62b72ee) )    // 8 bit signed pcm (16KHz)

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  /* */
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(dee22654) SHA1(5df05b0029ff7b1f7f04b41da7823d2aa8034bd2) )
ROM_END


/***************************************************************************

                                Tengai (World) / Sengoku Blade (Japan)

Board:  SH404
CPU:    MC68EC020FG16
Sound:  LZ8420M (Z80 core)
        YMF278B-F
OSC:    16.000MHz
        14.3181MHz
        33.8688MHz (YMF)
        4.000MHz (PIC)
SYNCS:  HSync 15.700kHz, VSync 59.923Hz
        HSync most likely derived from 14.3181MHz OSC (divided by 912)
        262 lines per frame consisting of:
          - Visible lines: 224
          - VBlank lines: 38 (Front/Back porch: 15 lines, VSync pulse: 8 lines)
Chips:  PS2001B
        PS3103
        PS3204
        PS3305

4-U59      security (PIC16C57)

***************************************************************************/

ROM_START( tengai )
	ROM_REGION( 0x100000, "maincpu", 0 )        // Main CPU Code
	ROM_LOAD32_WORD_SWAP( "5-u40.bin", 0x000000, 0x080000, CRC(90088195) SHA1(8ec48d581ecd14b3dad36edc65d5a273324cf3c1) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "4-u41.bin", 0x000002, 0x080000, CRC(0d53196c) SHA1(454bb4695b13ce44ca5dac7c6d4142a8b9afa798) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       // Sound CPU Code
	ROM_LOAD( "1-u63.bin", 0x00000, 0x20000, CRC(2025e387) SHA1(334b0eb3b416d46ccaadff3eee6f1abba63285fb) )

	ROM_REGION( 0x001000, "mcu", 0 )       // MCU, not hooked up
	/* PIC configuration:
	     -User ID: 37EA
	     -Watchdog Timer: unknown - tested on PCB: both settings work
	     -Oscillator Mode: probably XT (unconfirmed) - tested on PCB: HS and XT work, LP and RC don't
	*/
	ROM_LOAD( "4.u59", 0x00000, 0x01000, CRC(e563b054) SHA1(7593389d35851a71a8af2e094ec7e55cd818743a) )

	ROM_REGION( 0x600000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "u20.bin",  0x000000, 0x200000, CRC(ed42ef73) SHA1(74693fcc83a2654ddb18fd513d528033863d6116) )
	ROM_LOAD( "u22.bin",  0x200000, 0x200000, CRC(8d21caee) SHA1(2a68af8b2be2158dcb152c434e91a75871478d41) )
	ROM_LOAD( "u21.bin",  0x400000, 0x200000, CRC(efe34eed) SHA1(7891495b443a5acc7b2f17fe694584f6cb0afacc) )

	ROM_REGION( 0x400000, "gfx2", 0 )   // Layer 0 + 1
	ROM_LOAD( "u34.bin",  0x000000, 0x400000, CRC(2a2e2eeb) SHA1(f1d99353c0affc5c908985e6f2a5724e5223cccc) ) // four banks of 0x100000

	ROM_REGION( 0x400000, "ymf", 0 )    // Samples
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a63633c5) SHA1(89e75a40518926ebcc7d88dea86c01ba0bb496e5) ) // 8 bit signed pcm (16KHz)
	ROM_LOAD( "u62.bin",  0x200000, 0x200000, CRC(3ad0c357) SHA1(35f78cfa2eafa93ab96b24e336f569ee84af06b6) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  // Sprites LUT
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(681d7d55) SHA1(b0b28471440d747adbc4d22d1918f89f6ede1615) )
ROM_END

ROM_START( tengaij )
	ROM_REGION( 0x100000, "maincpu", 0 )        // Main CPU Code
	ROM_LOAD32_WORD_SWAP( "2-u40.bin", 0x000000, 0x080000, CRC(ab6fe58a) SHA1(6687a3af192b3eab60d75ca286ebb8e0636297b5) ) // 1&0
	ROM_LOAD32_WORD_SWAP( "3-u41.bin", 0x000002, 0x080000, CRC(02e42e39) SHA1(6cdb7b1cebab50c0a44cd60cd437f0e878ccac5c) ) // 3&2

	ROM_REGION( 0x020000, "audiocpu", 0 )       // Sound CPU Code
	ROM_LOAD( "1-u63.bin", 0x00000, 0x20000, CRC(2025e387) SHA1(334b0eb3b416d46ccaadff3eee6f1abba63285fb) )

	ROM_REGION( 0x001000, "mcu", 0 )       // MCU, not hooked up
	/* PIC configuration:
	     -User ID: 37EA
	     -Watchdog Timer: unknown - tested on PCB: both settings work
	     -Oscillator Mode: probably XT (unconfirmed) - tested on PCB: HS and XT work, LP and RC don't
	*/
	ROM_LOAD( "4.u59", 0x00000, 0x01000, CRC(e563b054) SHA1(7593389d35851a71a8af2e094ec7e55cd818743a) ) // From a World PCB

	ROM_REGION( 0x600000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "u20.bin",  0x000000, 0x200000, CRC(ed42ef73) SHA1(74693fcc83a2654ddb18fd513d528033863d6116) )
	ROM_LOAD( "u22.bin",  0x200000, 0x200000, CRC(8d21caee) SHA1(2a68af8b2be2158dcb152c434e91a75871478d41) )
	ROM_LOAD( "u21.bin",  0x400000, 0x200000, CRC(efe34eed) SHA1(7891495b443a5acc7b2f17fe694584f6cb0afacc) )

	ROM_REGION( 0x400000, "gfx2", 0 )   // Layer 0 + 1
	ROM_LOAD( "u34.bin",  0x000000, 0x400000, CRC(2a2e2eeb) SHA1(f1d99353c0affc5c908985e6f2a5724e5223cccc) ) // four banks of 0x100000

	ROM_REGION( 0x400000, "ymf", 0 )    // Samples
	ROM_LOAD( "u61.bin",  0x000000, 0x200000, CRC(a63633c5) SHA1(89e75a40518926ebcc7d88dea86c01ba0bb496e5) ) // 8 bit signed pcm (16KHz)
	ROM_LOAD( "u62.bin",  0x200000, 0x200000, CRC(3ad0c357) SHA1(35f78cfa2eafa93ab96b24e336f569ee84af06b6) )

	ROM_REGION16_LE( 0x040000, "spritelut", 0 )  // Sprites LUT
	ROM_LOAD( "u1.bin",  0x000000, 0x040000, CRC(681d7d55) SHA1(b0b28471440d747adbc4d22d1918f89f6ede1615) )
ROM_END

ROM_START( tengaibl ) // SK000701 PCB. Has 2 AD-65 (M6295 comp.) instead of YM. XTALs are 28'636'360 and 19'660'800
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "4_sk000701-upd27c4096.bin", 0x00000, 0x80000, CRC(1519c1c4) SHA1(7e8ff112c8d943d55797e32fe02bbce4ebd5c7b5) )
	ROM_LOAD32_WORD_SWAP( "5_sk000701-upd27c4096.bin", 0x00002, 0x80000, CRC(4f638dca) SHA1(cc15785a2acf6e45ca3ce13367d5d94f9a066a03) )

	// GFX ROMs are 5x MX29F1610MC-12. They are soldered and weren't dumped. Using the ones from the original for now.
	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD( "u20.bin", 0x000000, 0x200000, BAD_DUMP CRC(ed42ef73) SHA1(74693fcc83a2654ddb18fd513d528033863d6116) )
	ROM_LOAD( "u22.bin", 0x200000, 0x200000, BAD_DUMP CRC(8d21caee) SHA1(2a68af8b2be2158dcb152c434e91a75871478d41) )
	ROM_LOAD( "u21.bin", 0x400000, 0x200000, BAD_DUMP CRC(efe34eed) SHA1(7891495b443a5acc7b2f17fe694584f6cb0afacc) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "u34.bin", 0x000000, 0x400000, BAD_DUMP CRC(2a2e2eeb) SHA1(f1d99353c0affc5c908985e6f2a5724e5223cccc) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "1_sk000701-mx27c4000.bin", 0x00000, 0x80000, CRC(769f4c92) SHA1(a6ef5b180f691655a64f6121b1d00344c36a0ced) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "2_sk000701-mx27c4000.bin", 0x00000, 0x80000, CRC(f06f0a00) SHA1(2f8e216ef710f501d7a0a962ce82cca87f90611d) )

	ROM_REGION16_LE( 0x40000, "spritelut", 0 )
	ROM_LOAD( "3_sk000701-tms27c240.bin", 0x00000, 0x40000, CRC(cf44d2ba) SHA1(6e48b540c70050a7385d5a3fb412fd25aad923bb) ) // 1ST AND 2ND HALF IDENTICAL, otherwise identical to the original
	ROM_IGNORE(                                    0x40000 )

	ROM_REGION( 0x600, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal20v8b_1", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b_2", 0x200, 0x157, NO_DUMP )
	ROM_LOAD( "gal16v8d",   0x400, 0x117, NO_DUMP )
ROM_END

/***************************************************************************


                                Driver Initialization


***************************************************************************/

void psikyo_state::init_sngkace()
{
	{
		u8 *RAM = memregion("ymsnd:adpcma")->base();
		int len = memregion("ymsnd:adpcma")->bytes();

		/* Bit 6&7 of the samples are swapped. Naughty, naughty... */
		for (int i = 0; i < len; i++)
		{
			int x = RAM[i];
			RAM[i] = ((x & 0x40) << 1) | ((x & 0x80) >> 1) | (x & 0x3f);
		}
	}

	m_ka302c_banking = false; // SH201B doesn't have any gfx banking

	/* setup audiocpu banks */
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x8000);

	/* Enable other regions */
#if 0
	if (!strcmp(machine().system().name,"sngkace"))
	{
		u8 *ROM  =   memregion("maincpu")->base();
		ROM[0x995] = 0x4e;
		ROM[0x994] = 0x71;
		ROM[0x997] = 0x4e;
		ROM[0x996] = 0x71;

	}
#endif
}

void psikyo_state::s1945_mcu_init()
{
	m_s1945_mcu_direction = 0x00;
	m_s1945_mcu_inlatch = 0xff;
	m_s1945_mcu_latch1 = 0xff;
	m_s1945_mcu_latch2 = 0xff;
	m_s1945_mcu_latching = 0x5;
	m_s1945_mcu_control = 0xff;
	m_s1945_mcu_index = 0;
	m_s1945_mcu_mode = 0;
	m_s1945_mcu_bctrl = 0x00;

	save_item(NAME(m_s1945_mcu_direction));
	save_item(NAME(m_s1945_mcu_inlatch));
	save_item(NAME(m_s1945_mcu_latch1));
	save_item(NAME(m_s1945_mcu_latch2));
	save_item(NAME(m_s1945_mcu_latching));
	save_item(NAME(m_s1945_mcu_control));
	save_item(NAME(m_s1945_mcu_index));
	save_item(NAME(m_s1945_mcu_mode));
	save_item(NAME(m_s1945_mcu_bctrl));
}

void psikyo_state::init_tengai()
{
	s1945_mcu_init();
	m_s1945_mcu_table = nullptr;

	m_ka302c_banking = false; // Banking is controlled by mcu

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base() + 0x200, 0x8000);
}

void psikyo_state::init_gunbird()
{
	m_ka302c_banking = true;

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base() + 0x200, 0x8000);
}


void psikyo_state::init_s1945()
{
	s1945_mcu_init();
	m_s1945_mcu_table = s1945_table;

	m_ka302c_banking = false; // Banking is controlled by mcu

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base() + 0x200, 0x8000);
}

void psikyo_state::init_s1945a()
{
	s1945_mcu_init();
	m_s1945_mcu_table = s1945a_table;

	m_ka302c_banking = false; // Banking is controlled by mcu

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base() + 0x200, 0x8000);
}

void psikyo_state::init_s1945j()
{
	s1945_mcu_init();
	m_s1945_mcu_table = s1945j_table;

	m_ka302c_banking = false; // Banking is controlled by mcu

	/* setup audiocpu banks */
	/* The banked rom is seen at 8200-ffff, so the last 0x200 bytes of the rom not reachable. */
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base() + 0x200, 0x8000);
}

void psikyo_state::init_s1945bl()
{
	m_ka302c_banking = true;

	m_okibank[0]->configure_entries(0, 5, memregion("oki1")->base() + 0x30000, 0x10000);
	m_okibank[0]->set_entry(0);
}

void psikyo_state::init_tengaibl()
{
	m_ka302c_banking = true;

	m_okibank[0]->configure_entries(0, 5, memregion("oki1")->base() + 0x30000, 0x10000);
	m_okibank[0]->set_entry(0);

	m_okibank[1]->configure_entries(0, 5, memregion("oki2")->base() + 0x30000, 0x10000);
	m_okibank[1]->set_entry(0);
}


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1993, samuraia,  0,        sngkace,  samuraia,  psikyo_state, init_sngkace,  ROT270, "Psikyo (Banpresto license)",  "Samurai Aces (World)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1993, sngkace,   samuraia, sngkace,  sngkace,   psikyo_state, init_sngkace,  ROT270, "Psikyo (Banpresto license)",  "Sengoku Ace (Japan, set 1)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1993, sngkacea,  samuraia, sngkace,  sngkace,   psikyo_state, init_sngkace,  ROT270, "Psikyo (Banpresto license)",  "Sengoku Ace (Japan, set 2)",                 MACHINE_SUPPORTS_SAVE )

GAME( 1994, gunbird,   0,        gunbird,  gunbird,   psikyo_state, init_gunbird,  ROT270, "Psikyo",  "Gunbird (World)",                                                MACHINE_SUPPORTS_SAVE )
GAME( 1994, gunbirdk,  gunbird,  gunbird,  gunbirdj,  psikyo_state, init_gunbird,  ROT270, "Psikyo",  "Gunbird (Korea)",                                                MACHINE_SUPPORTS_SAVE )
GAME( 1994, gunbirdj,  gunbird,  gunbird,  gunbirdj,  psikyo_state, init_gunbird,  ROT270, "Psikyo",  "Gunbird (Japan)",                                                MACHINE_SUPPORTS_SAVE )

GAME( 1994, btlkroad,  0,        gunbird,  btlkroad,  psikyo_state, init_gunbird,  ROT0,   "Psikyo",  "Battle K-Road",                                                  MACHINE_SUPPORTS_SAVE )
GAME( 1994, btlkroadk, btlkroad, gunbird,  btlkroadk, psikyo_state, init_gunbird,  ROT0,   "Psikyo",  "Battle K-Road (Korea)",                                          MACHINE_SUPPORTS_SAVE ) // game code is still multi-region, but sound rom appears to be Korea specific at least

GAME( 1995, s1945,     0,        s1945,    s1945,     psikyo_state, init_s1945,    ROT270, "Psikyo",  "Strikers 1945 (World)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1995, s1945a,    s1945,    s1945,    s1945a,    psikyo_state, init_s1945a,   ROT270, "Psikyo",  "Strikers 1945 (Japan / World)",                                  MACHINE_SUPPORTS_SAVE ) // Region dip - 0x0f=Japan, anything else=World
GAME( 1995, s1945j,    s1945,    s1945,    s1945j,    psikyo_state, init_s1945j,   ROT270, "Psikyo",  "Strikers 1945 (Japan)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1995, s1945n,    s1945,    s1945n,   s1945,     psikyo_state, init_gunbird,  ROT270, "Psikyo",  "Strikers 1945 (World, unprotected)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1995, s1945nj,   s1945,    s1945n,   s1945j,    psikyo_state, init_gunbird,  ROT270, "Psikyo",  "Strikers 1945 (Japan, unprotected)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1995, s1945k,    s1945,    s1945,    s1945j,    psikyo_state, init_s1945,    ROT270, "Psikyo",  "Strikers 1945 (Korea)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1995, s1945bl,   s1945,    s1945bl,  s1945bl,   psikyo_state, init_s1945bl,  ROT270, "bootleg", "Strikers 1945 (Hong Kong, bootleg)",                             MACHINE_SUPPORTS_SAVE )

GAME( 1996, tengai,    0,        s1945,    tengai,    psikyo_state, init_tengai,   ROT0,   "Psikyo",  "Tengai (World)",                                                 MACHINE_SUPPORTS_SAVE )
GAME( 1996, tengaij,   tengai,   s1945,    tengaij,   psikyo_state, init_tengai,   ROT0,   "Psikyo",  "Sengoku Blade: Sengoku Ace Episode II (Japan) / Tengai (World)", MACHINE_SUPPORTS_SAVE ) // Region dip - 0x0f=Japan, anything else=World
GAME( 1996, tengaibl,  tengai,   tengaibl, tengaibl,  psikyo_state, init_tengaibl, ROT0,   "Psikyo",  "Tengai (bootleg)",                                               MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
