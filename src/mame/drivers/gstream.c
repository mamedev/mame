/********************************************************************

    G-Stream (c)2002 Oriental Soft Japan

    Hyperstone based hardware

    Simple Sprites (16x16x8bpp tiles)
    3 Simple Tilemaps (32x32x8bpp tiles)

    todo: sprite lag (sprites need buffering?)
          sound banking

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
Sound: OKI 6295 x 2 (rebaged as AD-65)
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

class gstream_state : public driver_device
{
public:
	gstream_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_oki_1(*this, "oki1"),
		  m_oki_2(*this, "oki2") ,
		m_workram(*this, "workram"),
		m_vram(*this, "vram"),
		m_paletteram(*this, "paletteram"){ }

	/* devices */
	required_device<e132xt_device> m_maincpu;
	required_device<okim6295_device> m_oki_1;
	required_device<okim6295_device> m_oki_2;

	/* memory pointers */
	required_shared_ptr<UINT32> m_workram;
	required_shared_ptr<UINT32> m_vram;
	required_shared_ptr<UINT32> m_paletteram;
//  UINT32 *  m_nvram;    // currently this uses generic nvram handling

	/* video-related */
	tilemap_t   *m_tilemap1;
	tilemap_t   *m_tilemap2;
	tilemap_t   *m_tilemap3;
	UINT32    m_tmap1_scrollx;
	UINT32    m_tmap2_scrollx;
	UINT32    m_tmap3_scrollx;
	UINT32    m_tmap1_scrolly;
	UINT32    m_tmap2_scrolly;
	UINT32    m_tmap3_scrolly;

	/* misc */
	int       m_oki_bank_0;
	int       m_oki_bank_1;

	DECLARE_WRITE32_MEMBER(gstream_palette_w);
	DECLARE_WRITE32_MEMBER(gstream_vram_w);
	DECLARE_WRITE32_MEMBER(gstream_tilemap1_scrollx_w);
	DECLARE_WRITE32_MEMBER(gstream_tilemap1_scrolly_w);
	DECLARE_WRITE32_MEMBER(gstream_tilemap2_scrollx_w);
	DECLARE_WRITE32_MEMBER(gstream_tilemap2_scrolly_w);
	DECLARE_WRITE32_MEMBER(gstream_tilemap3_scrollx_w);
	DECLARE_WRITE32_MEMBER(gstream_tilemap3_scrolly_w);
	DECLARE_WRITE32_MEMBER(gstream_oki_banking_w);
	DECLARE_WRITE32_MEMBER(gstream_oki_4040_w);
	DECLARE_READ32_MEMBER(gstream_speedup_r);
	DECLARE_CUSTOM_INPUT_MEMBER(gstream_mirror_service_r);
	DECLARE_CUSTOM_INPUT_MEMBER(gstream_mirror_r);
};



CUSTOM_INPUT_MEMBER(gstream_state::gstream_mirror_service_r)
{
	int result;

	/* PORT_SERVICE_NO_TOGGLE */
	result = (input_port_read(machine(), "IN0") & 0x8000) >> 15;

	return ~result;
}

CUSTOM_INPUT_MEMBER(gstream_state::gstream_mirror_r)
{
	int result;

	/* IPT_COIN1 */
	result  = ((input_port_read(machine(), "IN0") & 0x200) >>  9) << 0;
	/* IPT_COIN2 */
	result |= ((input_port_read(machine(), "IN1") & 0x200) >>  9) << 1;
	/* IPT_START1 */
	result |= ((input_port_read(machine(), "IN0") & 0x400) >> 10) << 2;
	/* IPT_START2 */
	result |= ((input_port_read(machine(), "IN1") & 0x400) >> 10) << 3;
	/* PORT_SERVICE_NO_TOGGLE */
	result |= ((input_port_read(machine(), "IN0") & 0x8000) >> 15) << 6;

	return ~result;
}


WRITE32_MEMBER(gstream_state::gstream_palette_w)
{
	COMBINE_DATA(&m_paletteram[offset]);

	palette_set_color_rgb(machine(), offset * 2, pal5bit(m_paletteram[offset] >> (0 + 16)),
									pal5bit(m_paletteram[offset] >> (6 + 16)),
									pal5bit(m_paletteram[offset] >> (11 + 16)));


	palette_set_color_rgb(machine(),offset * 2 + 1,pal5bit(m_paletteram[offset] >> (0)),
									pal5bit(m_paletteram[offset] >> (6)),
									pal5bit(m_paletteram[offset] >> (11)));
}

WRITE32_MEMBER(gstream_state::gstream_vram_w)
{
	COMBINE_DATA(&m_vram[offset]);

	if (ACCESSING_BITS_24_31)
	{
		if (offset >= 0x000 / 4 && offset < 0x400 / 4)
		{
			m_tilemap1->mark_tile_dirty(offset - (0x000 / 4));
		}
		else if (offset >= 0x400 / 4 && offset < 0x800 / 4)
		{
			m_tilemap2->mark_tile_dirty(offset - (0x400 / 4));
		}
		else if (offset >= 0x800 / 4 && offset < 0xc00 / 4)
		{
			m_tilemap3->mark_tile_dirty(offset - (0x800 / 4));
		}
	}
}

WRITE32_MEMBER(gstream_state::gstream_tilemap1_scrollx_w)
{
	m_tmap1_scrollx = data;
}

WRITE32_MEMBER(gstream_state::gstream_tilemap1_scrolly_w)
{
	m_tmap1_scrolly = data;
}

WRITE32_MEMBER(gstream_state::gstream_tilemap2_scrollx_w)
{
	m_tmap2_scrollx = data;
}

WRITE32_MEMBER(gstream_state::gstream_tilemap2_scrolly_w)
{
	m_tmap2_scrolly = data;
}

WRITE32_MEMBER(gstream_state::gstream_tilemap3_scrollx_w)
{
	m_tmap3_scrollx = data;
}

WRITE32_MEMBER(gstream_state::gstream_tilemap3_scrolly_w)
{
	m_tmap3_scrolly = data;
}

static ADDRESS_MAP_START( gstream_32bit_map, AS_PROGRAM, 32, gstream_state )
	AM_RANGE(0x00000000, 0x003FFFFF) AM_RAM AM_SHARE("workram") // work ram
//  AM_RANGE(0x40000000, 0x40FFFFFF) AM_RAM // ?? lots of data gets copied here if present, but game runs without it??
	AM_RANGE(0x80000000, 0x80003FFF) AM_RAM_WRITE(gstream_vram_w) AM_SHARE("vram") // video ram
	AM_RANGE(0x4E000000, 0x4E1FFFFF) AM_ROM AM_REGION("user2",0) // main game rom
	AM_RANGE(0x4F000000, 0x4F000003) AM_WRITE(gstream_tilemap3_scrollx_w)
	AM_RANGE(0x4F200000, 0x4F200003) AM_WRITE(gstream_tilemap3_scrolly_w)
	AM_RANGE(0x4F400000, 0x4F406FFF) AM_RAM_WRITE(gstream_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x4F800000, 0x4F800003) AM_WRITE(gstream_tilemap1_scrollx_w)
	AM_RANGE(0x4FA00000, 0x4FA00003) AM_WRITE(gstream_tilemap1_scrolly_w)
	AM_RANGE(0x4FC00000, 0x4FC00003) AM_WRITE(gstream_tilemap2_scrollx_w)
	AM_RANGE(0x4FE00000, 0x4FE00003) AM_WRITE(gstream_tilemap2_scrolly_w)
	AM_RANGE(0xFFC00000, 0xFFC01FFF) AM_RAM AM_SHARE("nvram") // Backup RAM
	AM_RANGE(0xFFF80000, 0xFFFFFFFF) AM_ROM AM_REGION("user1",0) // boot rom
ADDRESS_MAP_END

WRITE32_MEMBER(gstream_state::gstream_oki_banking_w)
{
	/* OKI BANKING  (still far from perfect, based on game behaviour)

    The two OKIs can independently play music or samples and are switched on the fly during game
    This is a preliminary table of the banks:

    BANK    MUSIC   SAMPLES
     0         X
     1    X
     2    X
     3    X
     4         X
     5    X
     6    X
     7    X

    Two nibbles are used in this handler: (data & 0xf) and ((data >> 4) & 0xf)
    The values for the first nibble are the followings and should map the 8 oki banks:
    - 0x6, 0x7, 0x9, 0xa, 0xb, 0xd, 0xe, 0xf
    The values for the second nibble are the followings and should probably be used too:
    - 0x6, 0x9, 0xa

    Same values are redudant, for example:
    level 2: data = 0x99
    level 6: data = 0x99
    (this means same background music for the two levels - it could be correct, though)

    Also with current implementation, using only (data & 0xf), we have to force some values
    manually, because the correspondent places in the table are already used

    Musics order is completely guessed but close to what the original PCB game should be */

	static const int bank_table_0[16] = { -1, -1, -1, -1, -1, -1, 0, 0, -1, 6, 0, 5, -1, 0, 0, 0 };
	static const int bank_table_1[16] = { -1, -1, -1, -1, -1, -1, 2, 2, -1, 0, 0, 4, -1, 1, 1, 1 };

	//popmessage("oki_0 banking value = %X\noki_1 banking value = %X\n",data & 0xf,(data >> 4) & 0xf);

	m_oki_bank_0 = bank_table_0[data & 0xf];
	m_oki_bank_1 = bank_table_1[data & 0xf];		// (data >> 4) & 0xf ??

	/* some values are already used in the table, so we force them manually */
	if ((data == 0x6f) || (data == 0x6e))
	{
		m_oki_bank_0 = 0;	// level 3b-5a samples
		m_oki_bank_1 = 6;		// level 3b-5a music
	}

	if (data == 0x9b)
	{
		m_oki_bank_0 = 7;		// level 7 music
		m_oki_bank_1 = 0;		// level 7 samples
	}

	if (data == 0x9f)
	{
		m_oki_bank_0 = 0;		// end sequence samples
		m_oki_bank_1 = 3;		// end sequence music
	}

	m_oki_1->set_bank_base(m_oki_bank_0 * 0x40000);
	m_oki_2->set_bank_base(m_oki_bank_1 * 0x40000);
}

WRITE32_MEMBER(gstream_state::gstream_oki_4040_w)
{
	// data == 0 or data == 0x81
}

static ADDRESS_MAP_START( gstream_io, AS_IO, 32, gstream_state )
	AM_RANGE(0x4000, 0x4003) AM_READ_PORT("IN0")
	AM_RANGE(0x4010, 0x4013) AM_READ_PORT("IN1")
	AM_RANGE(0x4020, 0x4023) AM_READ_PORT("IN2")	// extra coin switches etc
	AM_RANGE(0x4030, 0x4033) AM_WRITE(gstream_oki_banking_w)	// oki banking
	AM_RANGE(0x4040, 0x4043) AM_WRITE(gstream_oki_4040_w)	// ??
	AM_RANGE(0x4050, 0x4053) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x000000ff)	// music and samples
	AM_RANGE(0x4060, 0x4063) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x000000ff)	// music and samples
ADDRESS_MAP_END

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
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	PORT_CUSTOM_MEMBER(DEVICE_SELF, gstream_state,gstream_mirror_service_r, NULL)

	PORT_START("IN2")
	PORT_BIT( 0x004f, IP_ACTIVE_LOW, IPT_SPECIAL )	PORT_CUSTOM_MEMBER(DEVICE_SELF, gstream_state,gstream_mirror_r, NULL)
	PORT_BIT( 0xffb0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout layout16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24, 32,40,48,56, 64,72,80,88 ,96,104,112,120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*128,
};

static const gfx_layout layout32x32 =
{
	32,32,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,   8,   16,  24,  32,  40,  48,  56,
	  64,  72,  80,  88,  96,  104, 112, 120,
	  128, 136, 144, 152, 160, 168, 176, 184,
	  192, 200, 208, 216, 224, 232, 240, 248 },
	{ 0*256,  1*256,  2*256,  3*256,  4*256,  5*256,  6*256,  7*256,
	  8*256,  9*256,  10*256, 11*256, 12*256, 13*256, 14*256, 15*256,
      16*256, 17*256, 18*256, 19*256, 20*256, 21*256, 22*256, 23*256,
	  24*256, 25*256, 26*256, 27*256, 28*256, 29*256, 30*256, 31*256,
	},
	32*256,
};

static GFXDECODE_START( gstream )
	GFXDECODE_ENTRY( "gfx2", 0, layout32x32, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx1", 0, layout16x16, 0, 0x80 )
GFXDECODE_END



static TILE_GET_INFO( get_gs1_tile_info )
{
	gstream_state *state = machine.driver_data<gstream_state>();
	int tileno = (state->m_vram[tile_index + 0x000 / 4] & 0x0fff0000) >> 16;
	int palette = (state->m_vram[tile_index + 0x000 / 4] & 0xc0000000) >> 30;
	SET_TILE_INFO(0, tileno, palette + 0x10, 0);
}

static TILE_GET_INFO( get_gs2_tile_info )
{
	gstream_state *state = machine.driver_data<gstream_state>();
	int tileno = (state->m_vram[tile_index + 0x400 / 4] & 0x0fff0000) >> 16;
	int palette = (state->m_vram[tile_index + 0x400 / 4] & 0xc0000000) >> 30;
	SET_TILE_INFO(0, tileno + 0x1000, palette + 0x14, 0);
}


static TILE_GET_INFO( get_gs3_tile_info )
{
	gstream_state *state = machine.driver_data<gstream_state>();
	int tileno = (state->m_vram[tile_index + 0x800 / 4] & 0x0fff0000) >> 16;
	int palette = (state->m_vram[tile_index + 0x800 / 4] & 0xc0000000) >> 30;
	SET_TILE_INFO(0, tileno + 0x2000, palette + 0x18, 0);
}


static VIDEO_START(gstream)
{
	gstream_state *state = machine.driver_data<gstream_state>();
	state->m_tilemap1 = tilemap_create(machine, get_gs1_tile_info, tilemap_scan_rows, 32, 32, 16, 16);
	state->m_tilemap2 = tilemap_create(machine, get_gs2_tile_info, tilemap_scan_rows, 32, 32, 16, 16);
	state->m_tilemap3 = tilemap_create(machine, get_gs3_tile_info, tilemap_scan_rows, 32, 32, 16, 16);

	state->m_tilemap1->set_transparent_pen(0);
	state->m_tilemap2->set_transparent_pen(0);
}

static SCREEN_UPDATE_IND16(gstream)
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
   */

	gstream_state *state = screen.machine().driver_data<gstream_state>();
	int i;

	//popmessage("(1) %08x %08x (2) %08x %08x (3) %08x %08x", state->m_tmap1_scrollx, state->m_tmap1_scrolly, state->m_tmap2_scrollx, state->m_tmap2_scrolly, state->m_tmap3_scrollx, state->m_tmap3_scrolly );

	state->m_tilemap3->set_scrollx(0, state->m_tmap3_scrollx >> 16);
	state->m_tilemap3->set_scrolly(0, state->m_tmap3_scrolly >> 16);

	state->m_tilemap1->set_scrollx(0, state->m_tmap1_scrollx >> 16);
	state->m_tilemap1->set_scrolly(0, state->m_tmap1_scrolly >> 16);

	state->m_tilemap2->set_scrollx(0, state->m_tmap2_scrollx >> 16);
	state->m_tilemap2->set_scrolly(0, state->m_tmap2_scrolly >> 16);

	state->m_tilemap3->draw(bitmap, cliprect, 0, 0);
	state->m_tilemap2->draw(bitmap, cliprect, 0, 0);
	state->m_tilemap1->draw(bitmap, cliprect, 0, 0);

	for (i = 0x0000 / 4; i < 0x4000 / 4; i += 4)
	{
		/* Upper bits are used by the tilemaps */
		int code = state->m_vram[i + 0] & 0xffff;
		int x = state->m_vram[i + 1] & 0xffff;
		int y = state->m_vram[i + 2] & 0xffff;
		int col = state->m_vram[i + 3] & 0x1f;

		/* co-ordinates are signed */
		if (x & 0x8000) x -= 0x10000;
		if (y & 0x8000) y -= 0x10000;

		drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[1],code,col,0,0,x-2,y,0);
	}

	return 0;
}


static MACHINE_START( gstream )
{
	gstream_state *state = machine.driver_data<gstream_state>();

	state->save_item(NAME(state->m_tmap1_scrollx));
	state->save_item(NAME(state->m_tmap2_scrollx));
	state->save_item(NAME(state->m_tmap3_scrollx));
	state->save_item(NAME(state->m_tmap1_scrolly));
	state->save_item(NAME(state->m_tmap2_scrolly));
	state->save_item(NAME(state->m_tmap3_scrolly));
	state->save_item(NAME(state->m_oki_bank_0));
	state->save_item(NAME(state->m_oki_bank_1));
}

static MACHINE_RESET( gstream )
{
	gstream_state *state = machine.driver_data<gstream_state>();

	state->m_tmap1_scrollx = 0;
	state->m_tmap2_scrollx = 0;
	state->m_tmap3_scrollx = 0;
	state->m_tmap1_scrolly = 0;
	state->m_tmap2_scrolly = 0;
	state->m_tmap3_scrolly = 0;
	state->m_oki_bank_0 = 0;
	state->m_oki_bank_1 = 0;
}

static MACHINE_CONFIG_START( gstream, gstream_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", E132XT, 16000000*4)	/* 4x internal multiplier */
	MCFG_CPU_PROGRAM_MAP(gstream_32bit_map)
	MCFG_CPU_IO_MAP(gstream_io)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(gstream)
	MCFG_MACHINE_RESET(gstream)

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_STATIC(gstream)

	MCFG_PALETTE_LENGTH(0x1000 + 0x400 + 0x400 + 0x400) // sprites + 3 bg layers
	MCFG_GFXDECODE(gstream)

	MCFG_VIDEO_START(gstream)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", 1000000, OKIM6295_PIN7_HIGH) /* 1 Mhz? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_OKIM6295_ADD("oki2", 1000000, OKIM6295_PIN7_HIGH) /* 1 Mhz? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

ROM_START( gstream )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "gs_prg_01.u56", 0x000000, 0x080000, CRC(0d0c6a38) SHA1(a810bfc1c9158cccc37710d0ea7268e26e520cc2) )

	ROM_REGION32_BE( 0x200000, "user2", 0 ) /* Hyperstone CPU Code */
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

	ROM_REGION( 0xc00000, "gfx2", 0 )  /* bg tiles (32x32x8) */
	ROM_LOAD( "gs_gr_01.u120", 0x000000, 0x200000, CRC(b82cfab8) SHA1(08f0eaef5c927fb056c6cc9342e39f445aae9062) )
	ROM_LOAD( "gs_gr_02.u121", 0x200000, 0x200000, CRC(37e19cbd) SHA1(490ebb037fce09100ec4bba3f73ecdf101526641) )
	ROM_LOAD( "gs_gr_03.u125", 0x400000, 0x200000, CRC(1a3b2b11) SHA1(a4b1dc1a9709f8f8f2ab2190d7badc246caa540f) )
	ROM_LOAD( "gs_gr_04.u126", 0x600000, 0x200000, CRC(a4e8906c) SHA1(b285d7697cdaa62014cf65d09a19fcbd6a95bb98) )
	ROM_LOAD( "gs_gr_05.u174", 0x800000, 0x200000, CRC(ef283a73) SHA1(8b598facb344eac33138611abc141a2acb375983) )
	ROM_LOAD( "gs_gr_06.u175", 0xa00000, 0x200000, CRC(d4e3a2b2) SHA1(4577c007172c718bf7ca55a8ccee5455c281026c) )

	ROM_REGION( 0x200000, "oki1", 0 )
	ROM_LOAD( "gs_snd_01.u192", 0x000000, 0x080000, CRC(79b64d3f) SHA1(b2166210d3a3b85b9ace90749a444c881f69d551) )
	ROM_LOAD( "gs_snd_02.u194", 0x080000, 0x080000, CRC(e49ed92c) SHA1(a3d7b3fe93a786a246acf2657d9056398c793078) )
	ROM_LOAD( "gs_snd_03.u191", 0x100000, 0x080000, CRC(2bfff4ac) SHA1(cce1bb3c78b86722c926854c737f9589806012ba) )
	ROM_LOAD( "gs_snd_04.u193", 0x180000, 0x080000, CRC(b259de3b) SHA1(1a64f41d4344fefad5832332f1a7655e23f6b017) )

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_COPY( "oki1", 0, 0, 0x200000 )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "gstream.nv", 0x000000, 0x2000, CRC(895d724b) SHA1(97941102f94923220d9beb270939f0ad9a40fe0e) )
ROM_END

READ32_MEMBER(gstream_state::gstream_speedup_r)
{
	if (m_maincpu->state(STATE_GENPC) == 0xc0001592)
	{
		m_maincpu->eat_cycles(50);
	}

	return m_workram[0xd1ee0 / 4];
}

static DRIVER_INIT( gstream )
{
	gstream_state *state = machine.driver_data<gstream_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0xd1ee0, 0xd1ee3, read32_delegate(FUNC(gstream_state::gstream_speedup_r), state));
}


GAME( 2002, gstream, 0, gstream, gstream, gstream, ROT270, "Oriental Soft", "G-Stream G2020", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
