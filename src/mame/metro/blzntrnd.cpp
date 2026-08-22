// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

Blazing Tornado (c) 1994 Human Amusement (HUM-002-A-(B) PCB)
Grand Striker 2 (c) 1996 Human Amusement (HUM-003(A) PCB)

original driver from metro/metro.cpp by Luca Elia (l.elia@tin.it)


Main  CPU    :  MC68000

Video Chips  :  Imagetek I4220 071
                Konami 053936 PSAC2

Sound CPU    :  Z80

Sound Chips  :  YM2610 or YMF286K
                (YM2610 compatible)

Other        :  Memory Blitter

To Do:

-   For video related issues @see devices/video/imagetek_i4100.cpp
-   Most games in service mode, seem to require that you press start1&2 *exactly at once*
    in order to advance to the next screen (e.g. holding 1 then pressing 2 doesn't work).
-   Interrupt timing needs figuring out properly, having it incorrect
    causes scrolling glitches in some games.  Test cases Mouse Go Go
    title screen, GunMaster title screen.  Changing it can cause
    excessive slowdown in said games however.
-   Coin lockout;

driver modified by Hau
***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ymopn.h"
#include "video/imagetek_i4100.h"
#include "video/k053936.h"

#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>


namespace {

class blzntrnd_state : public driver_device
{
public:
	blzntrnd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ymsnd(*this, "ymsnd")
		, m_vdp(*this, "vdp2")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_soundlatch(*this, "soundlatch")
		, m_k053936(*this, "k053936")
		, m_k053936_ram(*this, "k053936_ram")
		, m_audiobank(*this, "audiobank")
	{ }

	void blzntrnd(machine_config &config);
	void gstrik2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void audiobank_w(u8 data);
	void k053936_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void ext_irq5_enable_w(int state);

	INTERRUPT_GEN_MEMBER(periodic_interrupt);
	void vblank_irq(int state);

	TILE_GET_INFO_MEMBER(k053936_get_tile_info);
	TILE_GET_INFO_MEMBER(k053936_gstrik2_get_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_gstrik2);
	DECLARE_VIDEO_START(blzntrnd);
	DECLARE_VIDEO_START(gstrik2);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym2610_device> m_ymsnd;
	required_device<imagetek_i4220_device> m_vdp;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<k053936_device> m_k053936;

	// memory pointers
	required_shared_ptr<u16> m_k053936_ram;

	required_memory_bank m_audiobank;

	// video-related
	tilemap_t *m_k053936_tilemap = nullptr;

	// misc
	bool m_ext_irq_enable = false;
};

/***************************************************************************


                                Interrupts


***************************************************************************/


INTERRUPT_GEN_MEMBER(blzntrnd_state::periodic_interrupt)
{
	m_vdp->set_irq(4);
}

// lev 2-7 (lev 1 seems sound related)
void blzntrnd_state::vblank_irq(int state)
{
//  logerror("%d %d %lld\n", state, m_screen->vpos(), m_screen->frame_number());

	if (state)
	{
		m_vdp->screen_eof(state);

		if (m_ext_irq_enable)
		{
			m_vdp->set_irq(5);
		}
	}
	else
	{
		m_vdp->clear_irq(5);
	}
}

void blzntrnd_state::ext_irq5_enable_w(int state)
{
	m_ext_irq_enable = state;
}

/***************************************************************************


                                    Video


***************************************************************************/

TILE_GET_INFO_MEMBER(blzntrnd_state::k053936_get_tile_info)
{
	const int code = m_k053936_ram[tile_index];

	tileinfo.set(0,
			code & 0x7fff,
			0xe,
			0);
}

TILE_GET_INFO_MEMBER(blzntrnd_state::k053936_gstrik2_get_tile_info)
{
	const int code = m_k053936_ram[tile_index];

	tileinfo.set(0,
			(code & 0x7fff) >> 2,
			0xe,
			0);
}

void blzntrnd_state::k053936_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_k053936_ram[offset]);
	m_k053936_tilemap->mark_tile_dirty(offset);
}

TILEMAP_MAPPER_MEMBER(blzntrnd_state::tilemap_scan_gstrik2)
{
	/* logical (col,row) -> memory offset */
	return ((row & 0x40) >> 6) | (col << 1) | ((row & 0x80) << 1) | ((row & 0x3f) << 9);
}

VIDEO_START_MEMBER(blzntrnd_state,blzntrnd)
{
	m_k053936_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(blzntrnd_state::k053936_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 256, 512);
}

VIDEO_START_MEMBER(blzntrnd_state,gstrik2)
{
	m_k053936_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(blzntrnd_state::k053936_gstrik2_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(blzntrnd_state::tilemap_scan_gstrik2)), 16, 16, 128, 256);
}

u32 blzntrnd_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* TODO: bit 5 of reg 7 is off when ROZ is supposed to be disabled
	 * (Blazing Tornado title screen/character select/ending and Grand Striker 2 title/how to play transition)
	 */

	bitmap.fill(m_vdp->get_background_pen(), cliprect);
	m_k053936->zoom_draw(screen, bitmap, cliprect, m_k053936_tilemap, 0, 0, 1); // FIXME!!!
	m_vdp->draw_foreground(screen, bitmap, cliprect);

	return 0;
}

/***************************************************************************


                                Memory Maps


***************************************************************************/

/***************************************************************************
                            Blazing Tornado
***************************************************************************/

void blzntrnd_state::audiobank_w(u8 data)
{
	m_audiobank->set_entry(data & 0x07);
}

void blzntrnd_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xe000, 0xffff).ram();
}

void blzntrnd_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(blzntrnd_state::audiobank_w));
	map(0x40, 0x40).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw();
	map(0x80, 0x83).rw(m_ymsnd, FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}

void blzntrnd_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                             // ROM
	map(0x200000, 0x27ffff).m(m_vdp, FUNC(imagetek_i4220_device::v2_map));

	map(0x400000, 0x43ffff).ram().w(FUNC(blzntrnd_state::k053936_w)).share(m_k053936_ram);  // 053936
	map(0x500000, 0x500fff).w(m_k053936, FUNC(k053936_device::linectrl_w));      // 053936 line control
	map(0x600000, 0x60001f).w(m_k053936, FUNC(k053936_device::ctrl_w));          // 053936 control

	map(0xe00000, 0xe00001).portr("DSW0").nopw();                   // Inputs
	map(0xe00002, 0xe00003).portr("DSW1");                               //
	map(0xe00002, 0xe00003).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0xff00).cswidth(16); // To Sound CPU
	map(0xe00004, 0xe00005).portr("IN0");                                //
	map(0xe00006, 0xe00007).portr("IN1");                                //
	map(0xe00008, 0xe00009).portr("IN2");                                //
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************


                                Input Ports


***************************************************************************/


#define JOY_LSB(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_)

#define JOY_MSB(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_)

/***************************************************************************
                            Blazing Tornado
***************************************************************************/

static INPUT_PORTS_START( blzntrnd )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(      0x0007, "Beginner" )
	PORT_DIPSETTING(      0x0006, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0001, "Expert" )
	PORT_DIPSETTING(      0x0000, "Master" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00c0, 0x0000, "Control Panel" )         PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(      0x0000, "4 Players" )
//  PORT_DIPSETTING(      0x0040, "4 Players" )
	PORT_DIPSETTING(      0x0080, "1P & 2P Tag only" )
	PORT_DIPSETTING(      0x00c0, "1P & 2P vs only" )
	PORT_DIPNAME( 0x0300, 0x0300, "Half Continue" )         PORT_DIPLOCATION("SW4:1,2")
	PORT_DIPSETTING(      0x0000, "6C to start, 3C to continue" )
	PORT_DIPSETTING(      0x0100, "4C to start, 2C to continue" )
	PORT_DIPSETTING(      0x0200, "2C to start, 1C to continue" )
	PORT_DIPSETTING(      0x0300, "Disabled" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW4:3" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW4:4" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW4:5" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW4:6" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW4:7" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW4:8" ) // Not read in Service Mode

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, "CP Single" )         PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "2:00" )
	PORT_DIPSETTING(      0x0200, "2:30" )
	PORT_DIPSETTING(      0x0100, "3:00" )
	PORT_DIPSETTING(      0x0000, "3:30" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "CP Tag" )            PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "2:00" )
	PORT_DIPSETTING(      0x0800, "2:30" )
	PORT_DIPSETTING(      0x0400, "3:00" )
	PORT_DIPSETTING(      0x0000, "3:30" )
	PORT_DIPNAME( 0x3000, 0x3000, "Vs Single" )         PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "2:30" )
	PORT_DIPSETTING(      0x2000, "3:00" )
	PORT_DIPSETTING(      0x1000, "4:00" )
	PORT_DIPSETTING(      0x0000, "5:00" )
	PORT_DIPNAME( 0xc000, 0xc000, "Vs Tag" )            PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0xc000, "2:30" )
	PORT_DIPSETTING(      0x8000, "3:00" )
	PORT_DIPSETTING(      0x4000, "4:00" )
	PORT_DIPSETTING(      0x0000, "5:00" )

	PORT_START("IN0")
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("IN1")
	JOY_LSB(3, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	JOY_MSB(4, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0002, IP_ACTIVE_LOW)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END


/***************************************************************************
                            Grand Striker 2
***************************************************************************/

static INPUT_PORTS_START( gstrik2 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0003, 0x0003, "Player Vs Com" )         PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0003, "1:00" )
	PORT_DIPSETTING(      0x0002, "1:30" )
	PORT_DIPSETTING(      0x0001, "2:00" )
	PORT_DIPSETTING(      0x0000, "2:30" )
	PORT_DIPNAME( 0x000c, 0x000c, "1P Vs 2P" )          PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x000c, "0:45" )
	PORT_DIPSETTING(      0x0008, "1:00" )
	PORT_DIPSETTING(      0x0004, "1:30" )
	PORT_DIPSETTING(      0x0000, "2:00" )
	PORT_DIPNAME( 0x0030, 0x0030, "Extra Time" )            PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(      0x0030, "0:30" )
	PORT_DIPSETTING(      0x0020, "0:45" )
	PORT_DIPSETTING(      0x0010, "1:00" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7") // Does not in Service Mode
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Time Period" )           PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, "Sudden Death" )
	PORT_DIPSETTING(      0x0000, "Full" )
	PORT_DIPNAME( 0x0700, 0x0400, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0700, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:5") // Does not in Service Mode
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6") // Does not in Service Mode
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW4:8" )      // Does not in Service Mode

	PORT_START("DSW1")
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4,5")
	PORT_DIPSETTING(      0x001c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x0015, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x001f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0011, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0008, "4 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0016, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(      0x0000, "4 Coins/8 Credits" )
	PORT_DIPSETTING(      0x0009, "3 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0012, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x001b, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, "3 Coins/7 Credits" )
	PORT_DIPSETTING(      0x000e, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0001, "3 Coins/8 Credits" )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(      0x0013, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x00e0, "Same as Coin A" )
	PORT_DIPNAME( 0x0300, 0x0300, "Credits to Start" )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Credits to Continue" )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Continue" )          PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6") // Does not in Service Mode
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Playmode" )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "1 Credit for 1 Player" )
	PORT_DIPSETTING(      0x0000, "1 Credit for 2 Players" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN0")
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, UNUSED)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, UNUSED)

	PORT_START("IN1")
	// Not Used

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************


                            Graphics Layouts


***************************************************************************/


static const gfx_layout layout_053936_16 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8),STEP8(8*8*8*1,8) },
	{ STEP8(0,8*8),STEP8(8*8*8*2,8*8) },
	8*8*8*4
};

static GFXDECODE_START( gfx_blzntrnd )
	GFXDECODE_ENTRY( "roztiles", 0, gfx_8x8x8_raw,    0x0, 0x10 ) // [0] 053936 Tiles
GFXDECODE_END

static GFXDECODE_START( gfx_gstrik2 )
	GFXDECODE_ENTRY( "roztiles", 0, layout_053936_16, 0x0, 0x10 ) // [0] 053936 Tiles
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

void blzntrnd_state::machine_start()
{
	m_audiobank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
	save_item(NAME(m_ext_irq_enable));
}


void blzntrnd_state::blzntrnd(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &blzntrnd_state::main_map);
	m_maincpu->set_periodic_int(FUNC(blzntrnd_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	Z80(config, m_audiocpu, 16_MHz_XTAL/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &blzntrnd_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &blzntrnd_state::sound_io_map);

	// video hardware
	I4220(config, m_vdp, 26.666_MHz_XTAL);
	m_vdp->irq_cb().set_inputline(m_maincpu, M68K_IRQ_1);
	m_vdp->set_vblank_irq_level(0);
	m_vdp->set_blit_irq_level(3);
	m_vdp->set_spriteram_buffered(true); // sprites are 1 frame delayed
	m_vdp->ext_ctrl_0_cb().set(FUNC(blzntrnd_state::ext_irq5_enable_w));

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(58.2328); // VSync 58.2328Hz, HSync 15.32kHz
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1500));
	m_screen->set_size(392, 263);
	m_screen->set_visarea(0, 304-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(blzntrnd_state::screen_update));
	m_screen->screen_vblank().set(FUNC(blzntrnd_state::vblank_irq));

	MCFG_VIDEO_START_OVERRIDE(blzntrnd_state,blzntrnd)

	GFXDECODE(config, m_gfxdecode, "vdp2:palette", gfx_blzntrnd);

	K053936(config, m_k053936);
	m_k053936->set_offsets(-77, -21);

	// sound hardware
	// HUM-002 PCB Configuration : Stereo output with second speaker connector
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2610(config, m_ymsnd, 16_MHz_XTAL/2);
	m_ymsnd->irq_handler().set_inputline("audiocpu", 0);
	m_ymsnd->add_route(0, "speaker", 0.75, 0);
	m_ymsnd->add_route(0, "speaker", 0.75, 1);
	m_ymsnd->add_route(1, "speaker", 1.0, 0);
	m_ymsnd->add_route(2, "speaker", 1.0, 1);
}

void blzntrnd_state::gstrik2(machine_config &config)
{
	blzntrnd(config);
	m_gfxdecode->set_info(gfx_gstrik2);

	MCFG_VIDEO_START_OVERRIDE(blzntrnd_state,gstrik2)

	m_k053936->set_offsets(-77, -19);

	m_vdp->set_tmap_xoffsets(0,8,0);

	// HUM-003 PCB Configuration : Mono output only
	config.device_remove("speaker");
	SPEAKER(config, "mono").front_center();

	m_ymsnd->reset_routes();
	m_ymsnd->add_route(0, "mono", 0.75);
	m_ymsnd->add_route(1, "mono", 0.25);
	m_ymsnd->add_route(2, "mono", 0.25);
}


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

Blazing Tornado
(c)1994 Human

CPU:    68000-16
Sound:  Z80-8
    YMF286K (YM2610 compatible)
OSC:    16.0000MHz
    26.666MHz
Chips:  Imagetek I4220 071
    Konami 053936 (PSAC2)

***************************************************************************/

ROM_START( blzntrnd )
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000
	ROM_LOAD16_BYTE( "1k.bin", 0x000000, 0x80000, CRC(b007893b) SHA1(609363449c0218b8a38de72d37c66e6f3bb4f8cd) )
	ROM_LOAD16_BYTE( "2k.bin", 0x000001, 0x80000, CRC(ec173252) SHA1(652d70055d2799442beede1ae68e54551931068f) )
	ROM_LOAD16_BYTE( "3k.bin", 0x100000, 0x80000, CRC(1e230ba2) SHA1(ca96c82d57a6b5bacc1bfd2f7965503c2a6e162f) )
	ROM_LOAD16_BYTE( "4k.bin", 0x100001, 0x80000, CRC(e98ca99e) SHA1(9346fc0d419add23eaceb5843c505f3ffa69e495) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // Z80
	ROM_LOAD( "rom5.bin", 0x00000, 0x20000, CRC(7e90b774) SHA1(abd0eda9eababa1f7ab17a2f60534dcebda33c9c) )

	ROM_REGION( 0x1800000, "vdp2", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "rom142.bin", 0x0000000, 0x200000, CRC(a7200598) SHA1(f8168a94abc380308901303a69cbd15097019797) )
	ROM_LOAD64_WORD( "rom186.bin", 0x0000002, 0x200000, CRC(6ee28ea7) SHA1(b33bcbf16423999135d96a62bf25c6ff23031f2a) )
	ROM_LOAD64_WORD( "rom131.bin", 0x0000004, 0x200000, CRC(c77e75d3) SHA1(8ad716d4e37d6efe478a8e49feb4e68283310890) )
	ROM_LOAD64_WORD( "rom175.bin", 0x0000006, 0x200000, CRC(04a84f9b) SHA1(83aabbc1c7ab06b351168153335f3c2f91fba0e9) )
	ROM_LOAD64_WORD( "rom242.bin", 0x0800000, 0x200000, CRC(1182463f) SHA1(6fa2a0b3186a3542b43926e3f37714b78a890542) )
	ROM_LOAD64_WORD( "rom286.bin", 0x0800002, 0x200000, CRC(384424fc) SHA1(f89d43756bd38515a223fe4ffbed3a44c673ae28) )
	ROM_LOAD64_WORD( "rom231.bin", 0x0800004, 0x200000, CRC(f0812362) SHA1(9f8be51f60f7baf72f9de8352e4e13d730f85903) )
	ROM_LOAD64_WORD( "rom275.bin", 0x0800006, 0x200000, CRC(184cb129) SHA1(8ffb3cdc7e0d227b6f0a7962bc6d853c6b84c8d2) )
	ROM_LOAD64_WORD( "rom342.bin", 0x1000000, 0x200000, CRC(e527fee5) SHA1(e5de1e134d95aa7a48695183189924061482e3a3) )
	ROM_LOAD64_WORD( "rom386.bin", 0x1000002, 0x200000, CRC(d10b1401) SHA1(0eb75a283000a8b19a14177461b6f335c9d9dec2) )
	ROM_LOAD64_WORD( "rom331.bin", 0x1000004, 0x200000, CRC(4d909c28) SHA1(fb9bb824e518f67713799ed2c0159a7bd70f35c4) )
	ROM_LOAD64_WORD( "rom375.bin", 0x1000006, 0x200000, CRC(6eb4f97c) SHA1(c7f006230cbf10e706b0362eeed34655a3aef1a5) )

	ROM_REGION( 0x200000, "roztiles", 0 )   // 053936 gfx data
	ROM_LOAD( "rom9.bin", 0x000000, 0x200000, CRC(37ca3570) SHA1(3374c586bf84583fa33f2793c4e8f2f61a0cab1c) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   // Samples
	ROM_LOAD( "rom8.bin", 0x000000, 0x080000, CRC(565a4086) SHA1(bd5780acfa5affa8705acbfccb0af16bac8ed298) )

	ROM_REGION( 0x400000, "ymsnd:adpcma", 0 )  // Samples
	ROM_LOAD( "rom6.bin", 0x000000, 0x200000, CRC(8b8819fc) SHA1(5fd9d2b5088cb676c11d32cac7ba8c5c18e31b64) )
	ROM_LOAD( "rom7.bin", 0x200000, 0x200000, CRC(0089a52b) SHA1(d643ac122d62557de27f06ba1413ef757a45a927) )
ROM_END

/*

Grand Striker 2
Human Entertainment, 1996

PCB Layout
----------

HUM-003-(A)
|-----------------------------------------------------------------------|
|           YM3016 ROM8.22  ROM342.88  ROM386.87  ROM331.86  ROM375.85  |
|                                                                       |
| 6264  YM2610         ROM142.80  ROM186.79  ROM131.78  ROM175.77       |
|                                                                       |
|                  ROM7.27  ROM442.92  ROM486.91  ROM431.90  ROM475.89  |
|                                                                       |
|          PAL         ROM242.84  ROM286.83  ROM231.82  ROM275.81       |
|  SPRG.30                                                              |
|  PAL     Z80     ROM6.23                                              |
|                                                                       |
|J                                                                      |
|A                                                                      |
|M                                               |--------|             |
|M                   PRG2  PRG3                  |IMAGETEK|   6264      |
|A                                               |I4220   |             |
|                    PRG0  PRG1                  |--------|             |
|     16MHz  68000   62256  62256    26.666MHz                          |
|                                                                       |
|     DSW1                                                              |
|     DSW2   EPM7032         |------|  62256  62256                     |
|     DSW3            6116   |053936|  62256  62256                     |
|     DSW4            6116   |PSAC2 |                   PAL             |
|                            |------|                          ROM9.60  |
|-----------------------------------------------------------------------|

Notes:
       68000 clock: 16.000MHz
         Z80 clock: 8.000MHz
      YM2610 clock: 8.000MHz
             VSync: 58Hz
             HSync: 15.11kHz

TODO:
    HUM-002-A-(B) PCB set is also exists, but not dumped. it's blazing tornado conversion?
*/


// The MASK roms weren't dumped from this set, but it's safe to assume they're the same in this case
ROM_START( gstrik2 )
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000
	ROM_LOAD16_BYTE( "hum_003_g2f.rom1.u107", 0x000000, 0x80000, CRC(2712d9ca) SHA1(efa967de931728534a663fa1529e92003afbb3e9) )
	ROM_LOAD16_BYTE( "hum_003_g2f.rom2.u108", 0x000001, 0x80000, CRC(86785c64) SHA1(ef172d6e859a68eb80f7c127b61883d50eefb0fe) )
	ROM_LOAD16_BYTE( "prg2.109", 0x100000, 0x80000, CRC(ead86919) SHA1(eb9b68dff4e08d90ac90043c7f3021914caa007d) )
	ROM_LOAD16_BYTE( "prg3.110", 0x100001, 0x80000, CRC(e0b026e3) SHA1(05f75c0432efda3dec0372199382e310bb268fba) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // Z80
	ROM_LOAD( "sprg.30", 0x00000, 0x20000, CRC(aeef6045) SHA1(61b8c89ca495d3aac79e53413a85dd203db816f3) )

	ROM_REGION( 0x1000000, "vdp2", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "chr0.80", 0x0000000, 0x200000, CRC(f63a52a9) SHA1(1ad52bb3a051eaffe8fb6ba49d4fc1d0b6144156) )
	ROM_LOAD64_WORD( "chr1.79", 0x0000002, 0x200000, CRC(4110c184) SHA1(90ccb3d50eff7a655336cfa9c072f7213589e64c) )
	ROM_LOAD64_WORD( "chr2.78", 0x0000004, 0x200000, CRC(ddb4b9ee) SHA1(0e2c151c3690b9c3d298dda8842e283660d37386) )
	ROM_LOAD64_WORD( "chr3.77", 0x0000006, 0x200000, CRC(5ab367db) SHA1(adf8749451f4583f8e9e00ab61f3408d804a7265) )
	ROM_LOAD64_WORD( "chr4.84", 0x0800000, 0x200000, CRC(77d7ef99) SHA1(8f5cf72f5919fe9363e7549e0bb1b3ee633cec3b) )
	ROM_LOAD64_WORD( "chr5.83", 0x0800002, 0x200000, CRC(a4d49e95) SHA1(9789bacba7876100e0f0293f54c81def545ed068) )
	ROM_LOAD64_WORD( "chr6.82", 0x0800004, 0x200000, CRC(32eb33b0) SHA1(2ea06484ca326b44a35ee470343147a9d91d5626) )
	ROM_LOAD64_WORD( "chr7.81", 0x0800006, 0x200000, CRC(2d30a21e) SHA1(749e86b7935ef71556eaee4caf6f954634e9bcbf) )
	// not populated
//  ROM_LOAD64_WORD( "chr8.88", 0x1000000, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr9.87", 0x1000002, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr10.86", 0x1000004, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr11.85", 0x1000006, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr12.92", 0x1800000, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr13.91", 0x1800002, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr14.90", 0x1800004, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr15.89", 0x1800006, 0x200000, NO_DUMP )

	ROM_REGION( 0x200000, "roztiles", 0 )   // 053936 gfx data
	ROM_LOAD( "psacrom.60", 0x000000, 0x200000,  CRC(73f1f279) SHA1(1135b2b1eb4c52249bc12ee178340bbb202a94c8) )

	ROM_REGION( 0x200000, "ymsnd:adpcmb", 0 )   // Samples
	ROM_LOAD( "sndpcm-b.22", 0x000000, 0x200000, CRC(a5d844d2) SHA1(18d644545f0844e66aa53775b67b0a29c7b7c31b) )

	ROM_REGION( 0x400000, "ymsnd:adpcma", 0 )  // Samples
	ROM_LOAD( "sndpcm-a.23", 0x000000, 0x200000, CRC(e6d32373) SHA1(8a79d4ea8b27d785fffd80e38d5ae73b7cea7304) )
	// ROM7.27 not populated?
ROM_END

ROM_START( gstrik2j )
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000
	ROM_LOAD16_BYTE( "prg0.107", 0x000000, 0x80000, CRC(e60a8c19) SHA1(19be6cfcb60ede6fd4eb2e14914b174107c4b52d) )
	ROM_LOAD16_BYTE( "prg1.108", 0x000001, 0x80000, CRC(853f6f7c) SHA1(8fb9d7cd0390f620560a1669bb13f2033eed7c81) )
	ROM_LOAD16_BYTE( "prg2.109", 0x100000, 0x80000, CRC(ead86919) SHA1(eb9b68dff4e08d90ac90043c7f3021914caa007d) )
	ROM_LOAD16_BYTE( "prg3.110", 0x100001, 0x80000, CRC(e0b026e3) SHA1(05f75c0432efda3dec0372199382e310bb268fba) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // Z80
	ROM_LOAD( "sprg.30", 0x00000, 0x20000, CRC(aeef6045) SHA1(61b8c89ca495d3aac79e53413a85dd203db816f3) )

	ROM_REGION( 0x1000000, "vdp2", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "chr0.80", 0x0000000, 0x200000, CRC(f63a52a9) SHA1(1ad52bb3a051eaffe8fb6ba49d4fc1d0b6144156) )
	ROM_LOAD64_WORD( "chr1.79", 0x0000002, 0x200000, CRC(4110c184) SHA1(90ccb3d50eff7a655336cfa9c072f7213589e64c) )
	ROM_LOAD64_WORD( "chr2.78", 0x0000004, 0x200000, CRC(ddb4b9ee) SHA1(0e2c151c3690b9c3d298dda8842e283660d37386) )
	ROM_LOAD64_WORD( "chr3.77", 0x0000006, 0x200000, CRC(5ab367db) SHA1(adf8749451f4583f8e9e00ab61f3408d804a7265) )
	ROM_LOAD64_WORD( "chr4.84", 0x0800000, 0x200000, CRC(77d7ef99) SHA1(8f5cf72f5919fe9363e7549e0bb1b3ee633cec3b) )
	ROM_LOAD64_WORD( "chr5.83", 0x0800002, 0x200000, CRC(a4d49e95) SHA1(9789bacba7876100e0f0293f54c81def545ed068) )
	ROM_LOAD64_WORD( "chr6.82", 0x0800004, 0x200000, CRC(32eb33b0) SHA1(2ea06484ca326b44a35ee470343147a9d91d5626) )
	ROM_LOAD64_WORD( "chr7.81", 0x0800006, 0x200000, CRC(2d30a21e) SHA1(749e86b7935ef71556eaee4caf6f954634e9bcbf) )
	// not populated
//  ROM_LOAD64_WORD( "chr8.88", 0x1000000, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr9.87", 0x1000002, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr10.86", 0x1000004, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr11.85", 0x1000006, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr12.92", 0x1800000, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr13.91", 0x1800002, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr14.90", 0x1800004, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr15.89", 0x1800006, 0x200000, NO_DUMP )

	ROM_REGION( 0x200000, "roztiles", 0 )   // 053936 gfx data
	ROM_LOAD( "psacrom.60", 0x000000, 0x200000,  CRC(73f1f279) SHA1(1135b2b1eb4c52249bc12ee178340bbb202a94c8) )

	ROM_REGION( 0x200000, "ymsnd:adpcmb", 0 )   // Samples
	ROM_LOAD( "sndpcm-b.22", 0x000000, 0x200000, CRC(a5d844d2) SHA1(18d644545f0844e66aa53775b67b0a29c7b7c31b) )

	ROM_REGION( 0x400000, "ymsnd:adpcma", 0 )  // Samples
	ROM_LOAD( "sndpcm-a.23", 0x000000, 0x200000, CRC(e6d32373) SHA1(8a79d4ea8b27d785fffd80e38d5ae73b7cea7304) )
	// ROM7.27 not populated?
ROM_END


} // anonymous namespace

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1994, blzntrnd, 0,       blzntrnd, blzntrnd, blzntrnd_state, empty_init, ROT0, "Human Amusement", "Blazing Tornado", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1996, gstrik2,  0,       gstrik2,  gstrik2,  blzntrnd_state, empty_init, ROT0, "Human Amusement", "Grand Striker 2 (Europe and Oceania)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1996, gstrik2j, gstrik2, gstrik2,  gstrik2,  blzntrnd_state, empty_init, ROT0, "Human Amusement", "Grand Striker 2 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // priority between rounds
