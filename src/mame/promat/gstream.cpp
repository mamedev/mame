// license:BSD-3-Clause
// copyright-holders:David Haywood
/********************************************************************

    G-Stream (c)2002 Oriental Soft Japan
    X2222 (prototype) (c)2000 Oriental Soft

    ---
    X2222 has corrupt boss graphics because the program roms we use don't match the sprite roms.
    --

    Hyperstone based hardware

    Simple Sprites (16x16x8bpp tiles)
    3 Simple Tilemaps (32x32x8bpp tiles)

    X2222 uses raw 16bpp palette data instead of 8bpp indexed colours.

    todo: sprite lag (sprites need buffering?)
          sprite wraparound is imperfect

    The following is confirmed on G-Stream only
    ---

    CPU:  E1-32XT
    Sound: 2x AD-65 (OKIM6295 clone)

    Xtals: 1.000Mhz (near AD-65a)
           16.000Mhz (near E1-32XT / u56.bin)

           27.000Mhz (near GFX Section & 54.000Mhz)
           54.000Mhz (near GFX Section & 27.000Mhz)

    notes:

    cpu #0 (PC=00002C34): unmapped program memory dword read from 515ECE48 & FFFFFFFF
     what is this, buggy code?

    Is there a problem with the OKI status reads for the AD-65 clone? the game reads
    the status register all the time, causing severe slowdown etc, might be related to
    bad OKI banking tho.

    ---

    Dump Notes:::

    G-Stream 2020, 2002 Oriental Soft Japan

    Shooter from Oriental soft, heavy influence from XII Stag
    as well as Trizeal. Three weapons may be carried and their
    power controlled by picking up capsules of a specific colour.

    Three capsule types:
    . Red   - Vulcan
    . Green - Missiles
    . Blue  - Laser

    Points are awarded by picking up medals which are released
    from shot down enemies. The medal value and appearance is
    increased as long as you don't miss any.

    When enough medals have been picked up a "void" bomb becomes
    available which when released (hold down main shot) will create
    a circular "void". Enemy bullets which hit this "void" are transformed
    into medals. If you place your ship inside this "void" it becomes
    invulnerable like in XII Stag.

    The game is powered by a HyperStone E1-32XT. A battery on the PCB
    saves hi-scores. Hi-Scores can be reset by toggling switch S2.

    Documentation:

    Name                 Size     CRC32
    ----------------------------------------
    g-stream_demo.jpg     195939  0x37984e02
    g-stream_title.jpg    152672  0xf7b9bfd3
    g-stream_pcb.jpg     2563664  0x5ec864f3

    Name           Size     CRC32        Chip Type
    ----------------------------------------------
    gs_prg_02.u197 2097152  0x2f8a6bea    27C160
    gs_gr_01.u120  2097152  0xb82cfab8    27C160
    gs_gr_02.u121  2097152  0x37e19cbd    27C160
    gs_gr_03.u125  2097152  0x1a3b2b11    27C160
    gs_gr_04.u126  2097152  0xa4e8906c    27C160
    gs_gr_05.u174  2097152  0xef283a73    27C160
    gs_gr_06.u175  2097152  0xd4e3a2b2    27C160
    gs_gr_07.u107  2097152  0x84e66fe1    27C160
    gs_gr_08.u109  2097152  0xabd0d6aa    27C160
    gs_gr_09.u180  2097152  0xf2c4fd77    27C160
    gs_gr_10.u182  2097152  0xd696d15d    27C160
    gs_gr_11.u108  2097152  0x946d71d1    27C160
    gs_gr_12.u110  2097152  0x94b56e4e    27C160
    gs_gr_13.u181  2097152  0x7daaeff0    27C160
    gs_gr_14.u183  2097152  0x6bd2a1e1    27C160
    gs_snd_01.u192  524288  0x79b64d3f    27C040
    gs_snd_02.u194  524288  0xe49ed92c    27C040
    gs_snd_03.u191  524288  0x2bfff4ac    27C040
    gs_snd_04.u193  524288  0xb259de3b    27C040
    gs_prg_01.u56   524288  0x0d0c6a38    27C040

    . Board supplied by Tormod
    . Board dumped by Tormod

+------------------------------------------------+
|        GS_SND_01   RAM4  16MHz     RAM1 S2 BT1 |
|  AD-65 GS_SND_02 GS_PRG_01 E1-32XT RAM1        |
| VOL    GS_SND_03                               |
|J AD-65 GS_SND_04                      GS_GR_07 |
|A    1MHz     GS_PRG_02 XC95288   RAM2 GS_GR_08 |
|M             GS_GR_01  RAM2           GS_GR_09 |
|M      RAM4   GS_GR_02   XC95288 27MHz GS_GR_10 |
|A      RAM4   GS_GR_03           54MHz GS_GR_11 |
|              GS_GR_04       S3        GS_GR_12 |
|    GAL       GS_GR_05       RAM3 RAM3 GS_GR_13 |
|    GAL       GS_GR_06       RAM3 RAM3 GS_GR_13 |
+------------------------------------------------+

  CPU: Hyperstone E1-32XT
Sound: OKI 6295 x 2 (rebadged as AD-65)
  OSC: 54.000MHz, 27.000MHz, 16.000MHz & 1.000MHz
Other: Sigma Xilinx XC95255 x 2

S2 is Toggle switch to reset high scores.
S3 is a 2 position dipswitch bank.
BT1 is 3.6V battery for high scores backup.

RAM1 is SEC KM416C1204CJ-6
RAM2 is SEC KM416C1002CJ-12
RAM3 is EliteMT LP621024DM-70LL
RAM4 is HMC HM6264LP-70

*********************************************************************/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class gstream_state : public driver_device
{
public:
	gstream_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki%u", 1)
		, m_workram(*this, "workram")
		, m_vram(*this, "vram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{
		m_toggle = 0;
	}

	void x2222(machine_config &config);
	void gstream(machine_config &config);

	void init_gstream();
	void init_x2222();

	int mirror_service_r();
	ioport_value gstream_mirror_r();
	int x2222_toggle_r();

private:
	/* devices */
	required_device<e132xt_device> m_maincpu;
	optional_device_array<okim6295_device, 2> m_oki;

	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_vram;
//  uint32_t *  m_nvram;    // currently this uses generic nvram handling

	/* video-related */
	uint32_t    m_scrollx[3];
	uint32_t    m_scrolly[3];

	/* misc */
	int       m_oki_bank[2];
	int       m_toggle;
	int       m_xoffset = 0;

	void vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Layer> void scrollx_w(uint16_t data);
	template<int Layer> void scrolly_w(uint16_t data);
	void gstream_oki_banking_w(uint32_t data);
	void gstream_oki_4040_w(uint32_t data);
	void x2222_sound_w(uint32_t data);
	uint32_t gstream_speedup_r();
	uint32_t x2222_speedup_r();
	uint32_t x2222_speedup2_r();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_bg(bitmap_rgb32 &bitmap, const rectangle &cliprect, int map, uint32_t* ram);
	void drawgfx_transpen_x2222(bitmap_rgb32 &dest, const rectangle &cliprect, gfx_element *gfx,gfx_element *gfx2,
	uint32_t code, int flipx, int flipy, int32_t destx, int32_t desty);

	void rearrange_sprite_data(uint8_t* ROM, uint32_t* NEW, uint32_t* NEW2);
	void rearrange_tile_data(uint8_t* ROM, uint32_t* NEW, uint32_t* NEW2);

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void gstream_32bit_map(address_map &map) ATTR_COLD;
	void gstream_io(address_map &map) ATTR_COLD;
	void x2222_32bit_map(address_map &map) ATTR_COLD;
	void x2222_io(address_map &map) ATTR_COLD;
};

int gstream_state::x2222_toggle_r() // or the game hangs when starting, might be a status flag for the sound?
{
	m_toggle ^= 0xffff;
	return m_toggle;
}


int gstream_state::mirror_service_r()
{
	int result;

	/* PORT_SERVICE_NO_TOGGLE */
	result = (ioport("IN0")->read() & 0x8000) >> 15;

	return ~result;
}

ioport_value gstream_state::gstream_mirror_r()
{
	int result;

	/* IPT_COIN1 */
	result  = ((ioport("IN0")->read() & 0x200) >>  9) << 0;
	/* IPT_COIN2 */
	result |= ((ioport("IN1")->read() & 0x200) >>  9) << 1;
	/* IPT_START1 */
	result |= ((ioport("IN0")->read() & 0x400) >> 10) << 2;
	/* IPT_START2 */
	result |= ((ioport("IN1")->read() & 0x400) >> 10) << 3;
	/* PORT_SERVICE_NO_TOGGLE */
	result |= ((ioport("IN0")->read() & 0x8000) >> 15) << 6;

	return ~result;
}



void gstream_state::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

template<int Layer>
void gstream_state::scrollx_w(uint16_t data)
{
	m_scrollx[Layer] = data;
}

template<int Layer>
void gstream_state::scrolly_w(uint16_t data)
{
	m_scrolly[Layer] = data;
}

void gstream_state::gstream_32bit_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("workram"); // work ram
//  map(0x40000000, 0x40ffffff).ram(); // ?? lots of data gets copied here if present, but game runs without it??
	map(0x80000000, 0x80003fff).ram().w(FUNC(gstream_state::vram_w)).share("vram"); // video ram
	map(0x4E000000, 0x4e1fffff).rom().region("maindata", 0); // main game rom
	map(0x4f000000, 0x4f000003).w(FUNC(gstream_state::scrollx_w<2>)).umask32(0xffff0000).cswidth(32);
	map(0x4f200000, 0x4f200003).w(FUNC(gstream_state::scrolly_w<2>)).umask32(0xffff0000).cswidth(32);
	map(0x4f400000, 0x4f406fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x4f800000, 0x4f800003).w(FUNC(gstream_state::scrollx_w<0>)).umask32(0xffff0000).cswidth(32);
	map(0x4fa00000, 0x4fa00003).w(FUNC(gstream_state::scrolly_w<0>)).umask32(0xffff0000).cswidth(32);
	map(0x4fc00000, 0x4fc00003).w(FUNC(gstream_state::scrollx_w<1>)).umask32(0xffff0000).cswidth(32);
	map(0x4fe00000, 0x4fe00003).w(FUNC(gstream_state::scrolly_w<1>)).umask32(0xffff0000).cswidth(32);
	map(0xffc00000, 0xffc01fff).ram().share("nvram"); // Backup RAM
	map(0xfff80000, 0xffffffff).rom().region("maincpu", 0); // boot rom
}

void gstream_state::gstream_oki_banking_w(uint32_t data)
{
/*
    ****OKI BANKING****
    Possibly perfect? works perfectly in-game, may not cover all possibilities.
    Needs code testing on PCB.

    The two OKIs can independently play music or samples, and have half of the
    sound ROMs to themselves. Sometimes the first OKI will play music, and some
    times it will play samples. The second OKI is the same way. The banks for
    both OKIs are laid out like so:

    BANK MUSIC SAMPLES
     0            X
     1     X
     2     X
     3     X

    Bank 0 is the same in both ROMs.

    The banking seems to be concerned with half-nibbles in the data byte. For
    OKI1, the half-nibbles are bits 76 and 32. For OKI2, 54 and 10.
    The bank's '2' bit is the first half-nibble's bottom bit bitwise AND with
    the inverse of its top bit. The bank's '1' bit is the second half-nibble's
    bottom bit bitwise AND with the top bit.

    This may or may not be correct, but further PCB tests are required, and it
    certainly sounds perfect in-game, without using any "hacks". */

/*
7654 3210
1010 1010 needs oki2 to be bank 0
1010 1011 needs oki2 to be bank 1
1010 1011 needs oki1 to be bank 0

7654 3210
1010 1111 needs oki1 to be bank 1
1010 1110 needs oki1 to be bank 1
1010 1110 needs oki2 to be bank 0

7654 3210
0110 0110 needs oki1 to be bank 2
0110 0110 needs oki2 to be bank 0

6and!7 & 2and3

4and!5 & 0and1

*/

	m_oki_bank[0] = ((BIT(data, 6) & ~BIT(data, 7)) << 1) | (BIT(data, 2) & BIT(data, 3));
	m_oki_bank[1] = ((BIT(data, 4) & ~BIT(data, 5)) << 1) | (BIT(data, 0) & BIT(data, 1));

	//popmessage("oki bank = %X\noki_1 = %X\noki_2 = %X\n",data, m_oki_bank[0], m_oki_bank[1]);

	m_oki[0]->set_rom_bank(m_oki_bank[0]);
	m_oki[1]->set_rom_bank(m_oki_bank[1]);
}

// Some clocking?
void gstream_state::gstream_oki_4040_w(uint32_t data)
{
	// data == 0 or data == 0x81
}

void gstream_state::gstream_io(address_map &map)
{
	map(0x4000, 0x4003).portr("IN0");
	map(0x4010, 0x4013).portr("IN1");
	map(0x4020, 0x4023).portr("IN2");    // extra coin switches etc
	map(0x4030, 0x4033).w(FUNC(gstream_state::gstream_oki_banking_w));    // oki banking
	map(0x4040, 0x4043).w(FUNC(gstream_state::gstream_oki_4040_w));   // some clocking?
	map(0x4053, 0x4053).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // music and samples
	map(0x4063, 0x4063).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // music and samples
}


void gstream_state::x2222_32bit_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("workram"); // work ram
	map(0x40000000, 0x403fffff).ram(); // ?? data gets copied here if present, but game runs without it??
	map(0x80000000, 0x80003fff).ram().w(FUNC(gstream_state::vram_w)).share("vram"); // video ram

	map(0x4fc00000, 0x4fc00003).w(FUNC(gstream_state::scrolly_w<1>)).umask32(0xffff0000).cswidth(32);
	map(0x4fd00000, 0x4fd00003).w(FUNC(gstream_state::scrollx_w<1>)).umask32(0xffff0000).cswidth(32);

	map(0x4fa00000, 0x4fa00003).w(FUNC(gstream_state::scrolly_w<2>)).umask32(0xffff0000).cswidth(32);
	map(0x4fb00000, 0x4fb00003).w(FUNC(gstream_state::scrollx_w<2>)).umask32(0xffff0000).cswidth(32);

	map(0x4fe00000, 0x4fe00003).w(FUNC(gstream_state::scrolly_w<0>)).umask32(0xffff0000).cswidth(32);
	map(0x4ff00000, 0x4ff00003).w(FUNC(gstream_state::scrollx_w<0>)).umask32(0xffff0000).cswidth(32);

	map(0xffc00000, 0xffc01fff).ram().share("nvram"); // Backup RAM (maybe)
	map(0xfff00000, 0xffffffff).rom().region("maincpu", 0); // boot rom
}

void gstream_state::x2222_sound_w(uint32_t data)
{
	// maybe sound in low 8-bits? but we have no samples anyway assuming it's an OKI
	if (data & 0xffffff00)
		printf("x2222_sound_w unk %08x\n", data);
}

void gstream_state::x2222_io(address_map &map)
{
	map(0x4000, 0x4003).portr("P1");
	map(0x4004, 0x4007).portr("P2");
	map(0x4008, 0x400b).portr("SYS");
	map(0x4010, 0x4013).portr("DSW");
	map(0x4028, 0x402b).w(FUNC(gstream_state::x2222_sound_w));
	map(0x4034, 0x4037).portr("IN4");
}

static INPUT_PORTS_START( gstream )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x7000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x8000, IP_ACTIVE_LOW )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x7000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(gstream_state, mirror_service_r)

	PORT_START("IN2")
	PORT_BIT( 0x004f, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(gstream_state, gstream_mirror_r)
	PORT_BIT( 0xffb0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( x2222 )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x0002, 0x0002, "IN1" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Free_Play ) ) // always 99 credits
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0006, 0x0004, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0018, "4" )
	PORT_DIPNAME( 0x00e0, 0x0000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0060, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x00a0, "5" )
	PORT_DIPSETTING(      0x00c0, "6" )
	PORT_DIPSETTING(      0x00e0, "7" )

	PORT_START("IN4")
	PORT_DIPNAME( 0x0001, 0x0001, "IN4" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(gstream_state, x2222_toggle_r)
INPUT_PORTS_END


static const gfx_layout layout16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,8*16) },
	16*16*8,
};


static const gfx_layout layout32x32 =
{
	32,32,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP32(0,8) },
	{ STEP32(0,8*32) },
	32*32*8,
};

static GFXDECODE_START( gfx_gstream )
	GFXDECODE_ENTRY( "gfx2", 0, layout32x32, 0x1000, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, layout32x32, 0x1400, 4 )
	GFXDECODE_ENTRY( "gfx4", 0, layout32x32, 0x1800, 4 )
	GFXDECODE_ENTRY( "gfx1", 0, layout16x16, 0,      32 )
GFXDECODE_END


static GFXDECODE_START( gfx_x2222 )
	GFXDECODE_ENTRY( "gfx2", 0, layout32x32, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx3", 0, layout32x32, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx4", 0, layout32x32, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx1", 0, layout16x16, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx1_lower", 0, layout16x16, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx2_lower", 0, layout32x32, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx3_lower", 0, layout32x32, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx4_lower", 0, layout32x32, 0, 0x80 )
GFXDECODE_END


void gstream_state::video_start()
{
}


// custom drawgfx function for x2222 to draw RGB data instead of indexed data, needed because our regular drawgfx and tilemap code don't support that
void gstream_state::drawgfx_transpen_x2222(bitmap_rgb32 &dest, const rectangle &cliprect, gfx_element *gfx,gfx_element *gfx2,
		uint32_t code, int flipx, int flipy, int32_t destx, int32_t desty)
{
	// use pen usage to optimize
	code %= gfx->elements();
	const pen_t *rgb = m_palette->pens(); // 16 bit BGR

	// render
	auto profile = g_profiler.start(PROFILER_DRAWGFX);

	int32_t destendx, destendy;
	int32_t srcx, srcy;
	int32_t dy;

	assert(dest.valid());
	assert(gfx != nullptr);
	assert(dest.cliprect().contains(cliprect));
	assert(code < gfx->elements());

	/* ignore empty/invalid cliprects */
	if (cliprect.empty())
		return;

	/* compute final pixel in X and exit if we are entirely clipped */
	destendx = destx + gfx->width() - 1;
	if (destx > cliprect.max_x || destendx < cliprect.min_x)
		return;

	/* apply left clip */
	srcx = 0;
	if (destx < cliprect.min_x)
	{
		srcx = cliprect.min_x - destx;
		destx = cliprect.min_x;
	}

	/* apply right clip */
	if (destendx > cliprect.max_x)
		destendx = cliprect.max_x;

	/* compute final pixel in Y and exit if we are entirely clipped */
	destendy = desty + gfx->height() - 1;
	if (desty > cliprect.max_y || destendy < cliprect.min_y)
		return;

	/* apply top clip */
	srcy = 0;
	if (desty < cliprect.min_y)
	{
		srcy = cliprect.min_y - desty;
		desty = cliprect.min_y;
	}

	/* apply bottom clip */
	if (destendy > cliprect.max_y)
		destendy = cliprect.max_y;

	/* apply X flipping */
	if (flipx)
		srcx = gfx->width() - 1 - srcx;

	/* apply Y flipping */
	dy = gfx->rowbytes();
	if (flipy)
	{
		srcy = gfx->height() - 1 - srcy;
		dy = -dy;
	}

	/* fetch the source data */
	const uint8_t *srcdata = gfx->get_data(code);
	const uint8_t *srcdata2 = gfx2->get_data(code);

	/* compute how many blocks of 4 pixels we have */
	uint32_t leftovers = (destendx + 1 - destx);

	/* adjust srcdata to point to the first source pixel of the row */
	srcdata += srcy * gfx->rowbytes() + srcx;
	srcdata2 += srcy * gfx->rowbytes() + srcx;

	/* non-flipped 16bpp case */
	if (!flipx)
	{
		/* iterate over pixels in Y */
		for (int32_t cury = desty; cury <= destendy; cury++)
		{
			uint32_t *destptr = &dest.pix(cury, destx);
			const uint8_t *srcptr = srcdata;
			const uint8_t *srcptr2 = srcdata2;
			srcdata += dy;
			srcdata2 += dy;

			/* iterate over leftover pixels */
			for (int32_t curx = 0; curx < leftovers; curx++)
			{
				uint32_t srcdata = (srcptr[0]);
				uint32_t srcdata2 = (srcptr2[0]);

				uint16_t full = (srcdata | (srcdata2 << 8));
				if (full != 0)
					destptr[0] = rgb[full];

				srcptr++;
				srcptr2++;
				destptr++;
			}
		}
	}

	/* flipped 16bpp case */
	else
	{
		/* iterate over pixels in Y */
		for (int32_t cury = desty; cury <= destendy; cury++)
		{
			uint32_t *destptr = &dest.pix(cury, destx);
			const uint8_t *srcptr = srcdata;
			const uint8_t *srcptr2 = srcdata2;

			srcdata += dy;
			srcdata2 += dy;

			/* iterate over leftover pixels */
			for (int32_t curx = 0; curx < leftovers; curx++)
			{
				uint32_t srcdata = (srcptr[0]);
				uint32_t srcdata2 = (srcptr2[0]);

				uint16_t full = (srcdata | (srcdata2 << 8));
				if (full != 0)
					destptr[0] = rgb[full];

				srcptr--;
				srcptr2--;
				destptr++;
			}
		}
	}
}

void gstream_state::draw_bg(bitmap_rgb32 &bitmap, const rectangle &cliprect, int map, uint32_t* ram )
{
	int scrollx;
	int scrolly;

	scrollx = m_scrollx[map]&0x1ff;
	scrolly = m_scrolly[map]&0x1ff;

	uint16_t basey = scrolly>>5;
	for (int y=0;y<13;y++)
	{
		uint16_t basex = scrollx>>5;
		for (int x=0;x<16;x++)
		{
			int vram_data = (ram[(basex&0x0f)+((basey&0x0f)*0x10)]) >> 16;
			int pal = (vram_data & 0xc000) >> 14;
			int code = (vram_data & 0x0fff);

			if (m_gfxdecode->gfx(map+5))
				drawgfx_transpen_x2222(bitmap,cliprect,m_gfxdecode->gfx(map),m_gfxdecode->gfx(map+5),code,0,0,(x*32)-(scrollx&0x1f)-m_xoffset,(y*32)-(scrolly&0x1f));
			else
				m_gfxdecode->gfx(map)->transpen(bitmap,cliprect,code,pal,0,0,(x*32)-(scrollx&0x1f)-m_xoffset,(y*32)-(scrolly&0x1f),0);

			basex++;
		}
		basey++;
	}
}

uint32_t gstream_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* The tilemaps and sprite are interleaved together.
	   Even Words are tilemap tiles
	   Odd Words are sprite data

	   Sprites start at the top of memory, tilemaps at
	   the bottom, but the areas can overlap

	   *What seems an actual game bug*
	   when a sprite ends up with a negative co-ordinate
	   a value of 0xfffffffe gets set in the sprite list.
	   this could corrupt the tile value as both words
	   are being set ?!

	   x2222 seems to set none of these bits but has other wraparound issues

	*/

	int i;

	//popmessage("(1) %08x %08x (2) %08x %08x (3) %08x %08x", m_scrollx[0], m_scrolly[0], m_scrollx[1], m_scrolly[1], m_scrollx[2], m_scrolly[2] );
	bitmap.fill(0,cliprect);


	draw_bg(bitmap, cliprect, 2, m_vram + 0x800/4);
	draw_bg(bitmap, cliprect, 1, m_vram + 0x400/4);
	draw_bg(bitmap, cliprect, 0, m_vram + 0x000/4); // move on top for x2222 , check

	int clk = 0;
	int clk_max = 432 * 262; // total usable cycle count for sprites; TODO : measure screen size, related to that?
	for (i = 0x0000 / 4; i < 0x4000 / 4; i += 4) // can't be drawable everything
	{
		clk += 8+128; // uses 8 cycle per RAM, 128 cycle per drawing tile
		if (clk >= clk_max)
			break;

		/* Upper bits are used by the tilemaps */
		int code = m_vram[i + 0] & 0xffff;
		int x = m_vram[i + 1] & 0x1ff;
		int y = m_vram[i + 2] & 0xff;
		int col = m_vram[i + 3] & 0x1f;

		if (m_gfxdecode->gfx(4))
		{
			drawgfx_transpen_x2222(bitmap, cliprect, m_gfxdecode->gfx(3), m_gfxdecode->gfx(4), code, 0, 0, x - m_xoffset, y);
			drawgfx_transpen_x2222(bitmap, cliprect, m_gfxdecode->gfx(3), m_gfxdecode->gfx(4), code, 0, 0, x - m_xoffset, y-0x100);
			drawgfx_transpen_x2222(bitmap, cliprect, m_gfxdecode->gfx(3), m_gfxdecode->gfx(4), code, 0, 0, x - m_xoffset - 0x200, y);
			drawgfx_transpen_x2222(bitmap, cliprect, m_gfxdecode->gfx(3), m_gfxdecode->gfx(4), code, 0, 0, x - m_xoffset - 0x200 , y-0x100);

		}
		else
		{
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, code, col, 0, 0, x - m_xoffset, y, 0);
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, code, col, 0, 0, x - m_xoffset, y-0x100, 0);
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, code, col, 0, 0, x - m_xoffset - 0x200, y, 0);
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, code, col, 0, 0, x - m_xoffset - 0x200, y-0x100, 0);

		}
	}
	return 0;
}


void gstream_state::machine_start()
{
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_oki_bank));
}

void gstream_state::machine_reset()
{
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_scrollx[2] = 0;
	m_scrolly[0] = 0;
	m_scrolly[1] = 0;
	m_scrolly[2] = 0;
	m_oki_bank[0] = m_oki_bank[1] = 0;
}

void gstream_state::gstream(machine_config &config)
{
	/* basic machine hardware */
	E132XT(config, m_maincpu, 16000000*4); /* 4x internal multiplier */
	m_maincpu->set_addrmap(AS_PROGRAM, &gstream_state::gstream_32bit_map);
	m_maincpu->set_addrmap(AS_IO, &gstream_state::gstream_io);
	m_maincpu->set_vblank_int("screen", FUNC(gstream_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 240);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(gstream_state::screen_update));

	PALETTE(config, m_palette).set_format(palette_device::BGR_565, 0x1000 + 0x400 + 0x400 + 0x400); // sprites + 3 bg layers

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gstream);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.00); /* 1 Mhz? */

	OKIM6295(config, m_oki[1], 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.00); /* 1 Mhz? */
}

void gstream_state::x2222(machine_config &config)
{
	/* basic machine hardware */
	E132XT(config, m_maincpu, 16000000*4); /* 4x internal multiplier */
	m_maincpu->set_addrmap(AS_PROGRAM, &gstream_state::x2222_32bit_map);
	m_maincpu->set_addrmap(AS_IO, &gstream_state::x2222_io);
	m_maincpu->set_vblank_int("screen", FUNC(gstream_state::irq0_line_hold));

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 240);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(gstream_state::screen_update));

	PALETTE(config, m_palette, palette_device::BGR_565);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_x2222);

	// unknown sound hw (no sound roms dumped)

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.00); /* 1 Mhz? */
}


ROM_START( gstream )
	ROM_REGION32_BE( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "gs_prg_01.u56", 0x000000, 0x080000, CRC(0d0c6a38) SHA1(a810bfc1c9158cccc37710d0ea7268e26e520cc2) )

	ROM_REGION32_BE( 0x200000, "maindata", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP( "gs_prg_02.u197", 0x000000, 0x200000, CRC(2f8a6bea) SHA1(c0a32838f4bd8599f09002139f87562db625c1c5) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* sprite tiles (16x16x8) */
	ROM_LOAD32_WORD( "gs_gr_07.u107", 0x000000, 0x200000, CRC(84e66fe1) SHA1(73d828714f9ed9baffdc06998f5bf3298396fe9c) )
	ROM_LOAD32_WORD( "gs_gr_11.u108", 0x000002, 0x200000, CRC(946d71d1) SHA1(516bd3f4d7f5bce59f0593ed6565114dbd5a4ef0) )
	ROM_LOAD32_WORD( "gs_gr_08.u109", 0x400000, 0x200000, CRC(abd0d6aa) SHA1(dd294bbdda05697df84247257f735ab51bc26ca3) )
	ROM_LOAD32_WORD( "gs_gr_12.u110", 0x400002, 0x200000, CRC(94b56e4e) SHA1(7c3877f993e575326dbd4c2e5d7570747277b20d) )
	ROM_LOAD32_WORD( "gs_gr_09.u180", 0x800000, 0x200000, CRC(f2c4fd77) SHA1(284c850688e3c0fd292a91a53e24fe3436dc4076) )
	ROM_LOAD32_WORD( "gs_gr_13.u181", 0x800002, 0x200000, CRC(7daaeff0) SHA1(5766d9a3a8c0931305424e0089108ce8df7dfe41) )
	ROM_LOAD32_WORD( "gs_gr_10.u182", 0xc00000, 0x200000, CRC(d696d15d) SHA1(85aaa5cdb35f3a8d3266bb8debec0558c860cb53) )
	ROM_LOAD32_WORD( "gs_gr_14.u183", 0xc00002, 0x200000, CRC(6bd2a1e1) SHA1(aedca91643f14ececc101a7708255ce9b1d70f68) )

	ROM_REGION( 0x400000, "gfx2", 0 )  /* bg tiles (32x32x8) */
	ROM_LOAD( "gs_gr_01.u120", 0x000000, 0x200000, CRC(b82cfab8) SHA1(08f0eaef5c927fb056c6cc9342e39f445aae9062) )
	ROM_LOAD( "gs_gr_02.u121", 0x200000, 0x200000, CRC(37e19cbd) SHA1(490ebb037fce09100ec4bba3f73ecdf101526641) )

	ROM_REGION( 0x400000, "gfx3", 0 )  /* bg tiles (32x32x8) */
	ROM_LOAD( "gs_gr_03.u125", 0x000000, 0x200000, CRC(1a3b2b11) SHA1(a4b1dc1a9709f8f8f2ab2190d7badc246caa540f) )
	ROM_LOAD( "gs_gr_04.u126", 0x200000, 0x200000, CRC(a4e8906c) SHA1(b285d7697cdaa62014cf65d09a19fcbd6a95bb98) )

	ROM_REGION( 0x400000, "gfx4", 0 )  /* bg tiles (32x32x8) */
	ROM_LOAD( "gs_gr_05.u174", 0x000000, 0x200000, CRC(ef283a73) SHA1(8b598facb344eac33138611abc141a2acb375983) )
	ROM_LOAD( "gs_gr_06.u175", 0x200000, 0x200000, CRC(d4e3a2b2) SHA1(4577c007172c718bf7ca55a8ccee5455c281026c) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "gs_snd_01.u192", 0x000000, 0x080000, CRC(79b64d3f) SHA1(b2166210d3a3b85b9ace90749a444c881f69d551) )
	ROM_LOAD( "gs_snd_02.u194", 0x080000, 0x080000, CRC(e49ed92c) SHA1(a3d7b3fe93a786a246acf2657d9056398c793078) )

	ROM_REGION( 0x100000, "oki2", 0 )
	ROM_LOAD( "gs_snd_03.u191", 0x000000, 0x080000, CRC(2bfff4ac) SHA1(cce1bb3c78b86722c926854c737f9589806012ba) )
	ROM_LOAD( "gs_snd_04.u193", 0x080000, 0x080000, CRC(b259de3b) SHA1(1a64f41d4344fefad5832332f1a7655e23f6b017) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "gstream.nv", 0x000000, 0x2000, CRC(895d724b) SHA1(97941102f94923220d9beb270939f0ad9a40fe0e) )
ROM_END




ROM_START( x2222 )
	ROM_REGION32_BE( 0x100000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "test.bin", 0x000000, 0x100000, CRC(6260421e) SHA1(095e955d029e98e024d4ec5c7f93a6d4845a92a0) ) // final version - but debug enabled, values on screen, maybe there's a flag in the ROM we can turn off?

	ROM_REGION32_BE( 0x0200000, "misc", 0 ) /* other code */
	ROM_LOAD( "test.hye", 0x000000, 0x0112dda, CRC(c1142b2f) SHA1(5807930820a53604013a6ac66e4d4ebe3628e1fc) ) // the above binary was built from this

	/* x2222 uses raw rgb16 data rather than 8bpp indexed, in order to use the same gfx decodes with a custom draw routine we arrange the data into 2 8bpp regions on init */
	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 )  /* sprite tiles (16x16x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx3", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	/* 2nd 8-bits */
	ROM_REGION( 0x800000, "gfx1_lower", ROMREGION_ERASE00 )  /* sprite tiles (16x16x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx2_lower", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx3_lower", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx4_lower", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x1000000, "sprites", 0 )  /* sprite tiles (16x16x16) */
	ROM_LOAD( "spr11.bin", 0x000000, 0x200000, CRC(1d15b444) SHA1(27ace509a7e4ec2e62453636acf444a861bb85ce) )
	ROM_LOAD( "spr21.bin", 0x200000, 0x1b8b00, CRC(1c392be2) SHA1(775882f588a8bef33a79fa4f25754a47dc82cb30) )
	ROM_LOAD( "spr12.bin", 0x400000, 0x200000, CRC(73225936) SHA1(50507c52b932198659e08d22d3c0a92e7c69e5ba) )
	ROM_LOAD( "spr22.bin", 0x600000, 0x1b8b00, CRC(cf7ebfa1) SHA1(c968dcf768e5598240f5a131414a5607899b4bef) )
	ROM_LOAD( "spr13.bin", 0x800000, 0x200000, CRC(52595c51) SHA1(a161a5f433aa7aa2f7824ea6b9b70d73ca63b62d) )
	ROM_LOAD( "spr23.bin", 0xa00000, 0x1b8b00, CRC(d894461e) SHA1(14dccfa8c762d928eaea0ac4cfff7d1272b69fdd) )
	ROM_LOAD( "spr14.bin", 0xc00000, 0x200000, CRC(f6cd6599) SHA1(170ea7a9a26fd8038df53fb333357766dabbe7c2) )
	ROM_LOAD( "spr24.bin", 0xe00000, 0x1b8b00, CRC(9542cb08) SHA1(d40c1f0b7d3e9deb12284c2f2c2df0ac43cb6cd2) )

	ROM_REGION( 0x400000, "bg1", 0 )  /* bg tiles (32x32x16) */
	ROM_LOAD16_BYTE( "bg31.bin", 0x000000, 0x11ac00, CRC(12e67bc2) SHA1(18618a8931af3b3aeab34fd50424a7ffb3da6458) )
	ROM_LOAD16_BYTE( "bg32.bin", 0x000001, 0x11ac00, CRC(95afa0da) SHA1(e534bc0874329475ce7efa836000fe29fc76c44c) )

	ROM_REGION( 0x400000, "bg2", 0 )  /* bg tiles (32x32x16) */
	ROM_LOAD16_BYTE( "bg21.bin", 0x000000, 0x1c8400, CRC(a10220f8) SHA1(9aa43a8e23cdf55d8623d2694b04971eaced9ba9) )
	ROM_LOAD16_BYTE( "bg22.bin", 0x000001, 0x1c8400, CRC(966f7c1d) SHA1(4699a3014c7e66d0dabd8d7982f43114b71181b7) )

	ROM_REGION( 0x400000, "bg3", 0 )  /* bg tiles (32x32x16) */
	ROM_LOAD16_BYTE( "bg11.bin", 0x000000, 0x1bc800, CRC(68975462) SHA1(7a2458a3d2465b727f4f5bf45685f35eb4885975) )
	ROM_LOAD16_BYTE( "bg12.bin", 0x000001, 0x1bc800, CRC(feef1240) SHA1(9eb123a19ade74d8b3ce4df0b04ca97c03fb9fdc) )

	// no idea what the sound hw is?
	ROM_REGION( 0x100000, "oki1", ROMREGION_ERASE00 )
	ROM_LOAD( "x2222_sound", 0x000000, 0x080000,NO_DUMP ) // probably an oki.. is there a sound cpu too?
ROM_END



ROM_START( x2222o )
	ROM_REGION32_BE( 0x100000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "older.bin", 0x080000, 0x080000, CRC(d12817bc) SHA1(2458f9d9020598a1646dfc848fddd323eebc5120) )

	ROM_REGION32_BE( 0x0200000, "misc", 0 ) /* other code */
	ROM_LOAD( "older.hye", 0x000000, 0x010892f, CRC(cf3a004e) SHA1(1cba64cfa235b9540f33a5ee0cc02dfd267e00fc) ) // this corresponds to the older.bin we're using, for reference

	/* x2222 uses raw rgb16 data rather than 8bpp indexed, in order to use the same gfx decodes with a custom draw routine we arrange the data into 2 8bpp regions on init */
	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 )  /* sprite tiles (16x16x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx3", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	/* 2nd 8-bits */
	ROM_REGION( 0x800000, "gfx1_lower", ROMREGION_ERASE00 )  /* sprite tiles (16x16x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx2_lower", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx3_lower", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x200000, "gfx4_lower", ROMREGION_ERASE00 )  /* bg tiles (32x32x8) */
	/* filled in at init*/

	ROM_REGION( 0x1000000, "sprites", 0 )  /* sprite tiles (16x16x16) */ // these sprite ROMs have the Boss tiles in the wrong location for the prototype program rom
	ROM_LOAD( "spr11.bin", 0x000000, 0x200000, BAD_DUMP CRC(1d15b444) SHA1(27ace509a7e4ec2e62453636acf444a861bb85ce) )
	ROM_LOAD( "spr21.bin", 0x200000, 0x1b8b00, BAD_DUMP CRC(1c392be2) SHA1(775882f588a8bef33a79fa4f25754a47dc82cb30) )
	ROM_LOAD( "spr12.bin", 0x400000, 0x200000, BAD_DUMP CRC(73225936) SHA1(50507c52b932198659e08d22d3c0a92e7c69e5ba) )
	ROM_LOAD( "spr22.bin", 0x600000, 0x1b8b00, BAD_DUMP CRC(cf7ebfa1) SHA1(c968dcf768e5598240f5a131414a5607899b4bef) )
	ROM_LOAD( "spr13.bin", 0x800000, 0x200000, BAD_DUMP CRC(52595c51) SHA1(a161a5f433aa7aa2f7824ea6b9b70d73ca63b62d) )
	ROM_LOAD( "spr23.bin", 0xa00000, 0x1b8b00, BAD_DUMP CRC(d894461e) SHA1(14dccfa8c762d928eaea0ac4cfff7d1272b69fdd) )
	ROM_LOAD( "spr14.bin", 0xc00000, 0x200000, BAD_DUMP CRC(f6cd6599) SHA1(170ea7a9a26fd8038df53fb333357766dabbe7c2) )
	ROM_LOAD( "spr24.bin", 0xe00000, 0x1b8b00, BAD_DUMP CRC(9542cb08) SHA1(d40c1f0b7d3e9deb12284c2f2c2df0ac43cb6cd2) )

	ROM_REGION( 0x400000, "bg1", 0 )  /* bg tiles (32x32x16) */
	ROM_LOAD16_BYTE( "bg31.bin", 0x000000, 0x11ac00, CRC(12e67bc2) SHA1(18618a8931af3b3aeab34fd50424a7ffb3da6458) )
	ROM_LOAD16_BYTE( "bg32.bin", 0x000001, 0x11ac00, CRC(95afa0da) SHA1(e534bc0874329475ce7efa836000fe29fc76c44c) )

	ROM_REGION( 0x400000, "bg2", 0 )  /* bg tiles (32x32x16) */
	ROM_LOAD16_BYTE( "bg21.bin", 0x000000, 0x1c8400, CRC(a10220f8) SHA1(9aa43a8e23cdf55d8623d2694b04971eaced9ba9) )
	ROM_LOAD16_BYTE( "bg22.bin", 0x000001, 0x1c8400, CRC(966f7c1d) SHA1(4699a3014c7e66d0dabd8d7982f43114b71181b7) )

	ROM_REGION( 0x400000, "bg3", 0 )  /* bg tiles (32x32x16) */
	ROM_LOAD16_BYTE( "bg11.bin", 0x000000, 0x1bc800, CRC(68975462) SHA1(7a2458a3d2465b727f4f5bf45685f35eb4885975) )
	ROM_LOAD16_BYTE( "bg12.bin", 0x000001, 0x1bc800, CRC(feef1240) SHA1(9eb123a19ade74d8b3ce4df0b04ca97c03fb9fdc) )

	// no idea what the sound hw is?
	ROM_REGION( 0x100000, "oki1", ROMREGION_ERASE00 )
	ROM_LOAD( "x2222_sound", 0x000000, 0x080000,NO_DUMP ) // probably an oki.. is there a sound cpu too?
ROM_END


uint32_t gstream_state::gstream_speedup_r()
{
	if (m_maincpu->pc() == 0xc0001592)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_workram[0xd1ee0 / 4];
}


uint32_t gstream_state::x2222_speedup_r()
{
	if (m_maincpu->pc() == 0x22064)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_workram[0x7ffac / 4];
}

uint32_t gstream_state::x2222_speedup2_r()
{
	if (m_maincpu->pc() == 0x23f44)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_workram[0x84e3c / 4];
}


void gstream_state::init_gstream()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd1ee0, 0xd1ee3, read32smo_delegate(*this, FUNC(gstream_state::gstream_speedup_r)));

	m_xoffset = 2;
}


void gstream_state::rearrange_tile_data(uint8_t* ROM, uint32_t* NEW, uint32_t* NEW2)
{
	int i;
	for (i = 0; i < 0x80000; i++)
	{
		NEW[i]  = (ROM[(i * 8) + 0x000000] << 0) | (ROM[(i * 8) + 0x000001] << 8) | (ROM[(i * 8) + 0x000004] << 16) | (ROM[(i * 8) + 0x000005] << 24);
		NEW2[i] = (ROM[(i * 8) + 0x000002] << 0) | (ROM[(i * 8) + 0x000003] << 8) | (ROM[(i * 8) + 0x000006] << 16) | (ROM[(i * 8) + 0x000007] << 24);
	}
}

void gstream_state::rearrange_sprite_data(uint8_t* ROM, uint32_t* NEW, uint32_t* NEW2)
{
	int i;
	for (i = 0; i < 0x200000; i++)
	{
		NEW[i]  = (ROM[(i * 2) + 0xc00000] << 24) | (ROM[(i * 2) + 0x800000] << 16) | (ROM[(i * 2) + 0x400000] << 8) | (ROM[(i * 2) + 0x000000] << 0);
		NEW2[i] = (ROM[(i * 2) + 0xc00001] << 24) | (ROM[(i * 2) + 0x800001] << 16) | (ROM[(i * 2) + 0x400001] << 8) | (ROM[(i * 2) + 0x000001] << 0);
	}
}

void gstream_state::init_x2222()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x7ffac, 0x7ffaf, read32smo_delegate(*this, FUNC(gstream_state::x2222_speedup_r))); // older
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x84e3c, 0x84e3f, read32smo_delegate(*this, FUNC(gstream_state::x2222_speedup2_r))); // newer

	rearrange_sprite_data(memregion("sprites")->base(), (uint32_t*)memregion("gfx1")->base(), (uint32_t*)memregion("gfx1_lower")->base()  );
	rearrange_tile_data(memregion("bg1")->base(), (uint32_t*)memregion("gfx2")->base(), (uint32_t*)memregion("gfx2_lower")->base());
	rearrange_tile_data(memregion("bg2")->base(), (uint32_t*)memregion("gfx3")->base(), (uint32_t*)memregion("gfx3_lower")->base());
	rearrange_tile_data(memregion("bg3")->base(), (uint32_t*)memregion("gfx4")->base(), (uint32_t*)memregion("gfx4_lower")->base());

	m_xoffset = 0;
}

} // anonymous namespace


GAME( 2002, gstream, 0,     gstream, gstream, gstream_state, init_gstream, ROT270, "Oriental Soft Japan",    "G-Stream G2020",            MACHINE_SUPPORTS_SAVE )
GAME( 2000, x2222,   0,     x2222,   x2222,   gstream_state, init_x2222,   ROT270, "Oriental Soft / Promat", "X2222 (final debug?)",      MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
GAME( 2000, x2222o,  x2222, x2222,   x2222,   gstream_state, init_x2222,   ROT270, "Oriental Soft / Promat", "X2222 (5-level prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
