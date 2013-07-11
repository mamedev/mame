/***************************************************************************

TODO:
- It seems shadows can both affect underlying sprites and not. This is currently
  hardcoded in the drivers; there might be a control bit somewhere.
  Games requiring shadows to affect sprites behind them:
  - Surprise Attack (dark glass walls in level 3)
  - 88 Games (angle indicator in the long jump event)
  - Sunset Riders (bull's eye in the saloon cutscene)
  - TMNT 2 (lightbeam in level 4 cave)
  - Metamorphic Force (double! lightbeam just before the last boss)
  Games requiring shadows to NOT affect sprites behind them:
  - Asterix (Asterix's shadow would be over his feet otherwise)
  - X-Men is dubious, see enemies halfway through level 1 coming from above with
    boulders over their heads.

- scrollcontrol = 30 in Golfing Greats (leader board)

- detatwin: sprites are left on screen during attract mode


                      Emulated
                         |
                  board #|year    CPU      tiles        sprites  priority palette    other
                    -----|---- ------- ------------- ------------- ------ ------ ----------------
Hyper Crash         GX401 1985                   GX400
Twinbee             GX412*1985   68000           GX400
Yie Ar Kung Fu      GX407*1985    6809
Gradius / Nemesis   GX456*1985   68000           GX400
Shao-lins Road      GX477*1985    6809
Jail Break          GX507*1986 KONAMI-1          005849                   PROMs
Finalizer           GX523*1985 KONAMI-1          005885                   PROMs
Konami's Ping Pong  GX555*1985     Z80
Iron Horse          GX560*1986    6809           005885                   PROMs
Konami GT           GX561*1985   68000           GX400
Green Beret         GX577*1985     Z80           005849                   PROMs
Galactic Warriors   GX578*1985   68000           GX400
Salamander          GX587*1986   68000           GX400
WEC Le Mans 24      GX602*1986 2x68000
BAW / Black Panther GX604*1987   68000           GX400                    007593
Combat School /     GX611*1987    6309           007121(x2)               007327
  Boot Camp
Rock 'n Rage /      GX620*1986    6309 007342        007420               007327
  Koi no Hotrock
Mr Kabuki/Mr Goemon GX621*1986     Z80           005849
Jackal              GX631*1986    6809           005885(x2)               007327 007343 (address decoder)
Contra / Gryzor     GX633*1987    6309           007121(x2)               007593
Flak Attack         GX669*1987    6309           007121                   007327 007452
Devil World / Dark  GX687*1987 2x68000           TWIN16
  Adventure / Majuu no Oukoku
Double Dribble      GX690*1986  3x6809           005885(x2)               007327 007452
Kitten Kaboodle /   GX712*1988                   GX400                    007593 051550
  Nyan Nyan Panic
Chequered Flag      GX717*1988  052001               051960 051937(x2)           051316(x2) (roz) 051733 (protection)
Fast Lane           GX752*1987    6309           007121                          051733 (protection) 007801
Hot Chase           GX763*1988 2x68000                                           051316(x3) (roz) 007634 007635 007558 007557
Rack 'Em Up /       GX765*1987    6309 007342        007420               007327 007324
  The Hustler
Haunted Castle      GX768*1988  052001           007121(x2)               007327
Ajax / Typhoon      GX770*1987   6309+ 052109 051962 051960 051937  PROM  007327 051316 (roz)
                                052001
Labyrinth Runner /  GX771*1987    6309           007121                   007593 051733 (protection) 051550
  Trick Trap
Super Contra        GX775*1988  052001 052109 051962 051960 051937  PROM  007327
Battlantis          GX777*1987    6309 007342        007420               007327 007324
Vulcan Venture /    GX785*1988 2x68000           TWIN16
  Gradius 2
City Bomber         GX787*1987   68000           GX400                    007593 051550
Over Drive          GX789*1990 2x68000               053247 053246 053251 051316(x2) (roz) 053249 053250(x2) (road) 053252(*)
Hyper Crash         GX790 1987
Blades of Steel     GX797*1987    6309 007342        007420               007327 051733 (protection)
The Main Event      GX799*1988    6309 052109 051962 051960 051937  PROM
Missing in Action   GX808*1989   68000 052109 051962 051960 051937  PROM
Missing in Action J GX808*1989 2x68000           TWIN16
Crime Fighters      GX821*1989  052526 052109 051962 051960 051937  PROM
Special Project Y   GX857*1989    6309 052109 051962 051960 051937  PROM         052591 (protection)
'88 Games           GX861*1988  052001 052109 051962 051960 051937  PROM         051316 (roz)
Final Round /       GX870*1988 1x68000           TWIN16?
  Hard Puncher
Thunder Cross       GX873*1988  052001 052109 051962 051960 051937  PROM  007327 052591 (protection)
Aliens              GX875*1990  052526 052109 051962 051960 051937  PROM
Gang Busters        GX878*1988  052526 052109 051962 051960 051937  PROM
Devastators         GX890*1988    6309 052109 051962 051960 051937  PROM         007324 051733 (protection)
Bottom of the Ninth GX891*1989    6809 052109 051962 051960 051937  PROM         051316 (roz)
Cue Brick           GX903*1989   68000 052109 051962 051960 051937  PROM
Cue Brick           GX903*1989 2x68000           TWIN16
Punk Shot           GX907*1990   68000 052109 051962 051960 051937 053251
Ultraman            GX910*1991   68000 ------ ------ 051960 051937  PROM         051316(x3) (roz) 051550
Surprise Attack     GX911*1990  053248 052109 051962 053245 053244 053251
Lightning Fighters /GX939*1990   68000 052109 051962 053245 053244 053251
  Trigon
Gradius 3           GX945*1989 2x68000 052109 051962 051960 051937  PROM
Parodius            GX955*1990  053248 052109 051962 053245 053244 053251
TMNT                GX963*1989   68000 052109 051962 051960 051937  PROM
Block Hole          GX973*1989  052526 052109 051962 051960 051937  PROM
Escape Kids         GX975*1991  053248 052109 051962 053247 053246 053251        053252(*)
Rollergames         GX999*1991  053248 ------ ------ 053245 053244               051316 (roz) 053252(*)
Bells & Whistles /  GX060*1991   68000 052109 051962 053245 053244 053251        054000 (collision)
  Detana!! Twin Bee
Golfing Greats      GX061*1991   68000 052109 051962 053245 053244 053251        053936 (roz+)
TMNT 2              GX063*1991   68000 052109 051962 053245 053244 053251        053990 (protection) 051550
Sunset Riders       GX064*1991   68000 052109 051962 053245 053244 053251        054358
X-Men               GX065*1992   68000 052109 051962 053247 053246 053251        054539 (sound)
XEXEX               GX067*1991   68000 054157 054156 053247 053246 053251        053250?("road") 054338 (alpha blending) 054539 (sound)
Asterix             GX068*1992   68000 054157 054156 053245 053244 053251        054358
G.I. Joe            GX069*1992   68000 054157 054156 053247 053246 053251        054539 (sound)
The Simpsons        GX072*1991  053248 052109 051962 053247 053246 053251
Thunder Cross 2     GX073*1991   68000 052109 051962 051960 051937 053251        054000 (collision)
Vendetta /          GX081*1991  053248 052109 051962 053247 053246 053251        054000 (collision)
  Crime Fighters 2
Premier Soccer      GX101*1993   68000 052109 051962 053245 053244 053251        053936 (roz+) 054986
Hexion              GX122*1992     Z80                                           052591 (protection) 053252(*)
Entapous /          GX123*1993   68000 054157 054156 055673 053246 055555        053252(*) 054000 053936 (roz+)
  Gaiapolis
Mystic Warrior      GX128*1993   68000 054157 054156 055673 053246 055555        054338 (alpha blending) 053252(*) 054539(x2) (sound)
Cowboys of Moo Mesa GX151*1992   68000 054157 054156 053247 053246 053251        053252(*) 054338 (alpha blending) 053990 (protection)
Violent Storm       GX168*1993   68000 054157 054156 055673 053246 055555        054338 (alpha blending) 055550 054539(x2) (sound)
Monster Maulers /   GX170*1993   68000 054157 054156 055673 053246 055555        053252(*) 055550 054338 (alpha blending) 054539 (sound) 053936 (roz+)
  Ultimate Battler Dadandarn
Bucky 'O Hare       GX173*1992   68000 054157 054156 053247 053246 053251        054338 (alpha blending) 054539 (sound)
Potrio              GX174 1992
Lethal Enforcers    GX191*1992    6309 054157(x2) 054156 053245 053244(x2)       054000 054539 (sound)
Metamorphic Force   GX224*1993   68000 054157 054157 055673 053246 055555
Martial Champion    GX234*1993   68000 054157 054156 055673 053246 055555        053252(*) 054338 (alpha blending) 053990 054539 (sound)
Run and Gun         GX247*1993   68000 (TTL tilemap) 055673 053246               053253(x2) 053252(*) 053936 (roz+) 054539(x2) (sound)
Quiz Gakumon no     GX248*1993   68000 052109 051962 053245 053244 053251        053990 (protection) 051550 - same board as TMNT2
  Susume
Polygonet Commander GX305+1993   68020 (TTL tilemap)                             XC56156-40(3D DSP) 054009(x2) 054010(x2) 054539 (sound)
System GX (ver 1)   GX300*1993   68020 056832 054156 055673 053246 055555        054338 (alpha blending) 054539(x2) (sound) 053252(*) 053936 (optional on ROM board, roz+)
System GX (ver 2)   GX300*1995   68020 056832 058143 055673 058142 055555        058144 (alpha blending) 058141 (sound) 053252(*) 053936 (optional on ROM board, roz+)
Beatmania DJ Main   GX858+1996   68020 056832 058143 056766        055555        058144 (alpha blending) 058141 (sound) 053252(*)
Tail to Nose             *1989   68000          V-System                         051316 (roz)
F-1 Grand Prix           *1991 2x68000          V-System                         053936 (roz+)
F-1 Grand Prix Part II   *1992 2x68000          V-System                         053936 (roz+)
Lethal Crash Race        *1993   68000          V-System                         053936 (roz+)
Super Slams              *1995   68000          V-System                         053936 (roz+)
Blazing Tornado          *1991   68000            Metro                          053936 (roz+)
Dragonball Z 2           *1994   68000 054157 054156 053247 053246 053251(x2)    053936(x2) (roz+) 053252(*)


Notes:
* 053252 seems to be just a timing/interrupt controller (see Vendetta schematics).

- Old games use 051961 instead of 052109, it is an earlier version functionally
  equivalent (maybe 052109 had bugs fixed). The list always shows 052109 because
  the two are exchangeable and 052109's are found also on original boards whose
  schematics show a 051961.

- Starting with the version 2 System GX mainboard, the following chip substitutions took place.
  All "new" chips are equivalent to their older counterparts, but are in a smaller package (and
  presumably are made on a smaller process).  The exception is the 058141, which is equivalent
  to 2 54539s (and yet takes less board space than even 1).

  058141 = 054539 (x2) (2 sound chips in one)
  058142 = 053246 (sprites)
  058143 = 054156 (tiles)
  058144 = 054338 (alpha blending)



Status of the ROM tests in the emulated games:

Chequered Flag      pass
Ajax / Typhoon      pass
Super Contra        pass
Over Drive          pass
The Main Event      pass
Missing in Action   pass
Crime Fighters      pass
Special Project Y   pass
Konami 88           pass
Thunder Cross       pass
Aliens              pass
Gang Busters        pass
Devastators         pass
Bottom of the Ninth pass
Punk Shot           pass
Surprise Attack     fails D05-6 (052109) because it uses mirror addresses to
                    select banks, and supporting those addresses breaks the
                    normal game ;-(
Lightning Fighters  pass
Gradius 3           pass
Parodius            pass
TMNT                pass
Block Hole          pass
Escape Kids         pass
Rollergames         pass
Bells & Whistles    pass
Golfing Greats      pass
TMNT 2              pass
Sunset Riders       pass
X-Men               pass
The Simpsons        pass
Thunder Cross 2     pass
Xexex               pass
Asterix             pass
GiJoe               pass
Vendetta            pass
Premier Soccer      fails 16D 18D 18F (053936)
Hexion              pass
Run and Gun         fails 36M (053936) 2U 2Y 5U 5Y (sprites)
Quiz Gakumon no Susume  pass
Dragonball Z 2      fails


THE FOLLOWING INFORMATION IS PRELIMINARY AND INACCURATE. DON'T RELY ON IT.


005885
------
Some games use two of these in pair. Jackal even puts together the two 4bpp
tilemaps to form a single 8bpp one.
It manages sprites and 32x32 or 64x32 tilemap (only Double Dribble uses the
64x32 one).
The chip also generates clock and interrupt signals suitable for a 6809.
It uses 0x2000 bytes of RAM for the tilemaps and sprites, and an additional
0x100 bytes, maybe for scroll RAM and line buffers. The maximum addressable
ROM is 0x20000 bytes (addressed 16 bits at a time). Tile and sprite data both
come from the same ROM space. Double Dribble and Jackal have external circuitry
to extend the limits and use separated addressing spaces for sprites and tiles.
All games use external circuitry to reuse one or both the tile flip attributes
as an additional address bit.
Two 256x4 lookup PROMs are also used to increase the color combinations.
All tilemap / sprite priority handling is done internally and the chip exports
5 bits of color code, composed of 1 bit indicating tile or sprite, and 4 bits
of ROM data remapped through the PROM.

inputs:
- address lines (A0-A13)
- data lines (DB0-DB7)
- misc interface stuff
- data from the gfx ROMs (RDL0-RDL7, RDU0-RDU7)
- data from the tile lookup PROMs (VCD0-VCD3)
- data from the sprite lookup PROMs (OCD0-OCD3)

outputs:
- address lines for tilemap RAM (AX0-AX12)
- data lines for tilemap RAM (VO0-VO7)
- address lines for the small RAM (FA0-FA7)
- data lines for the small RAM (FD0-FD7)
- address lines for the gfx ROMs (R0-R15)
- address lines for the tile lookup PROMs (VCF0-VCF3, VCB0-VCB3)
- address lines for the sprite lookup PROMs (OCB0-OCB3, OCF0-OCF3)
- NNMI, NIRQ, NFIR, NCPE, NCPQ, NEQ for the main CPU
- misc interface stuff
- color code to be output on screen (COL0-COL4)


control registers
000:          scroll y
001:          scroll x (low 8 bits)
002: -------x scroll x (high bit)
     ----xxx- row/colscroll control
              000 = solid scroll (finalizr, ddribble bg)
              100 = solid scroll (jackal)
              001 = ? (ddribble fg)
              011 = colscroll (jackal high scores)
              101 = rowscroll (ironhors, jackal map)
003: ------xx high bits of the tile code
     -----x-- unknown (finalizr)
     ----x--- selects sprite buffer (and makes a copy to a private buffer?)
     --x----- unknown (ironhors)
     -x------ unknown (ironhors)
     x------- unknown (ironhors, jackal)
004: -------x nmi enable
     ------x- irq enable
     -----x-- firq enable
     ----x--- flip screen


K055550
-------

Protection chip which performs a memset() operation.

Used in Violent Storm and Ultimate Battler to clear VRAM between scenes, among
other things.  May also perform other functions since Violent Storm still isn't
happy...

Has word-wide registers as follows:

0: Count of units to transfer.  The write here triggers the transfer.
1-6: Unknown
7: Destination address, MSW
8: Destination address, LSW
9: Unknown
10: Size of transfer units, MSW
11: Size of transfer units, LSW
12: Unknown
13: Value to fill destination region with
14-15: Unknown



***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "video/k055555.h"

#include "devlegcy.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


#define XOR(a) WORD_XOR_BE(a)

// Newer Konami devices

// from video/gticlub.c



/*****************************************************************************/
/* Konami K001006 Custom 3D Texel Renderer chip (KS10081) */

/***************************************************************************/
/*                                                                         */
/*                                  001006                                 */
/*                                                                         */
/***************************************************************************/

const device_type K001006 = &device_creator<k001006_device>;

k001006_device::k001006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K001006, "Konami 001006", tag, owner, clock, "k001006", __FILE__),
	m_screen(NULL),
	m_pal_ram(NULL),
	m_unknown_ram(NULL),
	m_addr(0),
	m_device_sel(0),
	m_palette(NULL)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k001006_device::device_config_complete()
{
	// inherit a copy of the static data
	const k001006_interface *intf = reinterpret_cast<const k001006_interface *>(static_config());
	if (intf != NULL)
	*static_cast<k001006_interface *>(this) = *intf;
	
	// or initialize to defaults if none provided
	else
	{
		m_gfx_region = "";
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001006_device::device_start()
{
	m_pal_ram = auto_alloc_array_clear(machine(), UINT16, 0x800);
	m_unknown_ram = auto_alloc_array_clear(machine(), UINT16, 0x1000);
	m_palette = auto_alloc_array_clear(machine(), UINT32, 0x800);

	save_pointer(NAME(m_pal_ram), 0x800*sizeof(UINT16));
	save_pointer(NAME(m_unknown_ram), 0x1000*sizeof(UINT16));
	save_pointer(NAME(m_palette), 0x800*sizeof(UINT32));
	save_item(NAME(m_device_sel));
	save_item(NAME(m_addr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001006_device::device_reset()
{
	m_addr = 0;
	m_device_sel = 0;
	memset(m_palette, 0, 0x800*sizeof(UINT32));
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ32_MEMBER( k001006_device::read )
{
	if (offset == 1)
	{
		switch (m_device_sel)
		{
			case 0x0b:      // CG Board ROM read
			{
				UINT16 *rom = (UINT16*)space.machine().root_device().memregion(m_gfx_region)->base();
				return rom[m_addr / 2] << 16;
			}
			case 0x0d:      // Palette RAM read
			{
				UINT32 addr = m_addr;

				m_addr += 2;
				return m_pal_ram[addr >> 1];
			}
			case 0x0f:      // Unknown RAM read
			{
				return m_unknown_ram[m_addr++];
			}
			default:
			{
				fatalerror("k001006_r, unknown device %02X\n", m_device_sel);
			}
		}
	}
	return 0;
}

WRITE32_MEMBER( k001006_device::write )
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_addr);
	}
	else if (offset == 1)
	{
		switch (m_device_sel)
		{
			case 0xd:   // Palette RAM write
			{
				int r, g, b, a;
				UINT32 index = m_addr;

				m_pal_ram[index >> 1] = data & 0xffff;

				a = (data & 0x8000) ? 0x00 : 0xff;
				b = ((data >> 10) & 0x1f) << 3;
				g = ((data >>  5) & 0x1f) << 3;
				r = ((data >>  0) & 0x1f) << 3;
				b |= (b >> 5);
				g |= (g >> 5);
				r |= (r >> 5);
				m_palette[index >> 1] = MAKE_ARGB(a, r, g, b);

				m_addr += 2;
				break;
			}
			case 0xf:   // Unknown RAM write
			{
			//  mame_printf_debug("Unknown RAM %08X = %04X\n", m_addr, data & 0xffff);
				m_unknown_ram[m_addr++] = data & 0xffff;
				break;
			}
			default:
			{
				mame_printf_debug("k001006_w: device %02X, write %04X to %08X\n", m_device_sel, data & 0xffff, m_addr++);
			}
		}
	}
	else if (offset == 2)
	{
		if (ACCESSING_BITS_16_31)
		{
			m_device_sel = (data >> 16) & 0xf;
		}
	}
}

UINT32 k001006_device::get_palette( int index )
{
	return m_palette[index];
}


/*****************************************************************************/
/* Konami K001005 Custom 3D Pixel Renderer chip (KS10071) */

/***************************************************************************/
/*                                                                         */
/*                                  001005                                 */
/*                                                                         */
/***************************************************************************/

#include "video/poly.h"
#include "cpu/sharc/sharc.h"

struct poly_extra_data
{
	UINT32 color;
	int texture_x, texture_y;
	int texture_page;
	int texture_palette;
	int texture_mirror_x;
	int texture_mirror_y;
};

struct k001005_state
{
	screen_device *screen;
	device_t *cpu;
	device_t *dsp;
	device_t *k001006_1;
	device_t *k001006_2;

	UINT8  *     texture;
	UINT16 *     ram[2];
	UINT32 *     fifo;
	UINT32 *     _3d_fifo;

	UINT32    status;
	bitmap_rgb32 *bitmap[2];
	bitmap_ind32 *zbuffer;
	rectangle cliprect;
	int    ram_ptr;
	int    fifo_read_ptr;
	int    fifo_write_ptr;
	int    _3d_fifo_ptr;

	int tex_mirror_table[4][128];

	int bitmap_page;

	poly_manager *poly;
	poly_vertex prev_v[4];
	int prev_poly_type;

	UINT8 *gfxrom;
};

static const int decode_x_gti[8] = {  0, 16, 2, 18, 4, 20, 6, 22 };
static const int decode_y_gti[16] = {  0, 8, 32, 40, 1, 9, 33, 41, 64, 72, 96, 104, 65, 73, 97, 105 };

static const int decode_x_zr107[8] = {  0, 16, 1, 17, 2, 18, 3, 19 };
static const int decode_y_zr107[16] = {  0, 8, 32, 40, 4, 12, 36, 44, 64, 72, 96, 104, 68, 76, 100, 108 };


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k001005_state *k001005_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == K001005);

	return (k001005_state *)downcast<k001005_device *>(device)->token();
}

INLINE const k001005_interface *k001005_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == K001005));
	return (const k001005_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static void k001005_render_polygons( device_t *device );

// rearranges the texture data to a more practical order
void k001005_preprocess_texture_data( UINT8 *rom, int length, int gticlub )
{
	int index;
	int i, x, y;
	UINT8 temp[0x40000];

	const int *decode_x;
	const int *decode_y;

	if (gticlub)
	{
		decode_x = decode_x_gti;
		decode_y = decode_y_gti;
	}
	else
	{
		decode_x = decode_x_zr107;
		decode_y = decode_y_zr107;
	}

	for (index = 0; index < length; index += 0x40000)
	{
		int offset = index;

		memset(temp, 0, 0x40000);

		for (i = 0; i < 0x800; i++)
		{
			int tx = ((i & 0x400) >> 5) | ((i & 0x100) >> 4) | ((i & 0x40) >> 3) | ((i & 0x10) >> 2) | ((i & 0x4) >> 1) | (i & 0x1);
			int ty = ((i & 0x200) >> 5) | ((i & 0x80) >> 4) | ((i & 0x20) >> 3) | ((i & 0x8) >> 2) | ((i & 0x2) >> 1);

			tx <<= 3;
			ty <<= 4;

			for (y = 0; y < 16; y++)
			{
				for (x = 0; x < 8; x++)
				{
					UINT8 pixel = rom[offset + decode_y[y] + decode_x[x]];

					temp[((ty + y) * 512) + (tx + x)] = pixel;
				}
			}

			offset += 128;
		}

		memcpy(&rom[index], temp, 0x40000);
	}
}

void k001005_swap_buffers( device_t *device )
{
	k001005_state *k001005 = k001005_get_safe_token(device);

	k001005->bitmap_page ^= 1;

	//if (k001005->status == 2)
	{
		k001005->bitmap[k001005->bitmap_page]->fill(device->machine().pens[0] & 0x00ffffff, k001005->cliprect);
		k001005->zbuffer->fill(0xffffffff, k001005->cliprect);
	}
}

READ32_DEVICE_HANDLER( k001005_r )
{
	k001005_state *k001005 = k001005_get_safe_token(device);

	switch(offset)
	{
		case 0x000:         // FIFO read, high 16 bits
		{
			UINT16 value = k001005->fifo[k001005->fifo_read_ptr] >> 16;
		//  mame_printf_debug("FIFO_r0: %08X\n", k001005->fifo_ptr);
			return value;
		}

		case 0x001:         // FIFO read, low 16 bits
		{
			UINT16 value = k001005->fifo[k001005->fifo_read_ptr] & 0xffff;
		//  mame_printf_debug("FIFO_r1: %08X\n", k001005->fifo_ptr);

			if (k001005->status != 1 && k001005->status != 2)
			{
				if (k001005->fifo_read_ptr < 0x3ff)
				{
					//k001005->dsp->execute().set_input_line(SHARC_INPUT_FLAG1, CLEAR_LINE);
					sharc_set_flag_input(k001005->dsp, 1, CLEAR_LINE);
				}
				else
				{
					//k001005->dsp->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
					sharc_set_flag_input(k001005->dsp, 1, ASSERT_LINE);
				}
			}
			else
			{
				//k001005->dsp->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
				sharc_set_flag_input(k001005->dsp, 1, ASSERT_LINE);
			}

			k001005->fifo_read_ptr++;
			k001005->fifo_read_ptr &= 0x7ff;
			return value;
		}

		case 0x11b:         // status ?
			return 0x8002;

		case 0x11c:         // slave status ?
			return 0x8000;

		case 0x11f:
			if (k001005->ram_ptr >= 0x400000)
			{
				return k001005->ram[1][(k001005->ram_ptr++) & 0x3fffff];
			}
			else
			{
				return k001005->ram[0][(k001005->ram_ptr++) & 0x3fffff];
			}

		default:
			//mame_printf_debug("k001005->r: %08X, %08X at %08X\n", offset, mem_mask, space.device().safe_pc());
			break;
	}
	return 0;
}

WRITE32_DEVICE_HANDLER( k001005_w )
{
	k001005_state *k001005 = k001005_get_safe_token(device);

	switch (offset)
	{
		case 0x000:         // FIFO write
		{
			if (k001005->status != 1 && k001005->status != 2)
			{
				if (k001005->fifo_write_ptr < 0x400)
				{
					//k001005->dsp->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
					sharc_set_flag_input(k001005->dsp, 1, ASSERT_LINE);
				}
				else
				{
					//k001005->dsp->execute().set_input_line(SHARC_INPUT_FLAG1, CLEAR_LINE);
					sharc_set_flag_input(k001005->dsp, 1, CLEAR_LINE);
				}
			}
			else
			{
				//k001005->dsp->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
				sharc_set_flag_input(k001005->dsp, 1, ASSERT_LINE);
			}

		//  mame_printf_debug("K001005 FIFO write: %08X at %08X\n", data, space.device().safe_pc());
			k001005->fifo[k001005->fifo_write_ptr] = data;
			k001005->fifo_write_ptr++;
			k001005->fifo_write_ptr &= 0x7ff;

			k001005->_3d_fifo[k001005->_3d_fifo_ptr++] = data;

			// !!! HACK to get past the FIFO B test (GTI Club & Thunder Hurricane) !!!
			if (k001005->cpu->safe_pc() == 0x201ee)
			{
				// This is used to make the SHARC timeout
				k001005->cpu->execute().spin_until_trigger(10000);
			}
			// !!! HACK to get past the FIFO B test (Winding Heat & Midnight Run) !!!
			if (k001005->cpu->safe_pc() == 0x201e6)
			{
				// This is used to make the SHARC timeout
				k001005->cpu->execute().spin_until_trigger(10000);
			}

			break;
		}

		case 0x100: break;

	//  case 0x10a:     poly_r = data & 0xff; break;
	//  case 0x10b:     poly_g = data & 0xff; break;
	//  case 0x10c:     poly_b = data & 0xff; break;

		case 0x11a:
			k001005->status = data;
			k001005->fifo_write_ptr = 0;
			k001005->fifo_read_ptr = 0;

			if (data == 2 && k001005->_3d_fifo_ptr > 0)
			{
				k001005_swap_buffers(device);
				k001005_render_polygons(device);
				poly_wait(k001005->poly, "render_polygons");
				k001005->_3d_fifo_ptr = 0;
			}
			break;

		case 0x11d:
			k001005->fifo_write_ptr = 0;
			k001005->fifo_read_ptr = 0;
			break;

		case 0x11e:
			k001005->ram_ptr = data;
			break;

		case 0x11f:
			if (k001005->ram_ptr >= 0x400000)
			{
				k001005->ram[1][(k001005->ram_ptr++) & 0x3fffff] = data & 0xffff;
			}
			else
			{
				k001005->ram[0][(k001005->ram_ptr++) & 0x3fffff] = data & 0xffff;
			}
			break;

		default:
			//mame_printf_debug("k001005->w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
			break;
	}

}

/* emu/video/poly.c cannot handle atm callbacks passing a device parameter */
#define POLY_DEVICE 0

#if POLY_DEVICE
static void draw_scanline( device_t *device, void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	float z = extent->param[0].start;
	float dz = extent->param[0].dpdx;
	UINT32 *fb = &destmap->pix32(scanline);
	UINT32 *zb = &k001005->zbuffer->pix32(scanline);
	UINT32 color = extra->color;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = (UINT32)z >> 16;

		if (iz <= zb[x])
		{
			if (color & 0xff000000)
			{
				fb[x] = color;
				zb[x] = iz;
			}
		}

		z += dz;
	}
}
#endif

#if POLY_DEVICE
static void draw_scanline_tex( device_t *device, void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	UINT8 *texrom = k001005->gfxrom + (extra->texture_page * 0x40000);
	device_t *pal_device = (extra->texture_palette & 0x8) ? k001005->k001006_2 : k001005->k001006_1;
	int palette_index = (extra->texture_palette & 0x7) * 256;
	float z = extent->param[0].start;
	float u = extent->param[1].start;
	float v = extent->param[2].start;
	float w = extent->param[3].start;
	float dz = extent->param[0].dpdx;
	float du = extent->param[1].dpdx;
	float dv = extent->param[2].dpdx;
	float dw = extent->param[3].dpdx;
	int texture_mirror_x = extra->texture_mirror_x;
	int texture_mirror_y = extra->texture_mirror_y;
	int texture_x = extra->texture_x;
	int texture_y = extra->texture_y;
	int x;

	UINT32 *fb = &destmap->pix32(scanline);
	UINT32 *zb = &k001005->zbuffer->pix32(scanline);

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = (UINT32)z >> 16;
		//int iu = u >> 16;
		//int iv = v >> 16;

		if (iz < zb[x])
		{
			float oow = 1.0f / w;
			UINT32 color;
			int iu, iv;
			int iiv, iiu, texel;

			iu = u * oow;
			iv = v * oow;

			iiu = texture_x + k001005->tex_mirror_table[texture_mirror_x][(iu >> 4) & 0x7f];
			iiv = texture_y + k001005->tex_mirror_table[texture_mirror_y][(iv >> 4) & 0x7f];
			texel = texrom[((iiv & 0x1ff) * 512) + (iiu & 0x1ff)];
			color = k001006_get_palette(pal_device, palette_index + texel);

			if (color & 0xff000000)
			{
				fb[x] = color;
				zb[x] = iz;
			}
		}

		u += du;
		v += dv;
		z += dz;
		w += dw;
	}
}
#endif


static void k001005_render_polygons( device_t *device )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	int i, j;
#if POLY_DEVICE
	const rectangle &visarea = k001005->screen->visible_area();
#endif

//  mame_printf_debug("k001005->fifo_ptr = %08X\n", k001005->_3d_fifo_ptr);

	for (i = 0; i < k001005->_3d_fifo_ptr; i++)
	{
		if (k001005->_3d_fifo[i] == 0x80000003)
		{
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
//          poly_vertex v[4];
			int r, g, b, a;
			UINT32 color;
			int index = i;

			++index;

			for (j = 0; j < 4; j++)
			{
				int x, y;

				x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
				y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);
				++index;
#if POLY_DEVICE
				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = 0;  /* ??? */
#endif
			}

			++index;

			r = (k001005->_3d_fifo[index] >>  0) & 0xff;
			g = (k001005->_3d_fifo[index] >>  8) & 0xff;
			b = (k001005->_3d_fifo[index] >> 16) & 0xff;
			a = (k001005->_3d_fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			++index;

			extra->color = color;
#if POLY_DEVICE
			poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
			poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[2], &v[3]);
//          poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page],  &visarea, draw_scanline, 1, 4, v);
#endif
			i = index - 1;
		}
		else if (k001005->_3d_fifo[i] == 0x800000ae || k001005->_3d_fifo[i] == 0x8000008e ||
					k001005->_3d_fifo[i] == 0x80000096 || k001005->_3d_fifo[i] == 0x800000b6 ||
					k001005->_3d_fifo[i] == 0x8000002e || k001005->_3d_fifo[i] == 0x8000000e ||
					k001005->_3d_fifo[i] == 0x80000016 || k001005->_3d_fifo[i] == 0x80000036 ||
					k001005->_3d_fifo[i] == 0x800000aa || k001005->_3d_fifo[i] == 0x800000a8 ||
					k001005->_3d_fifo[i] == 0x800000b2)
		{
			// 0x00: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx    Command
			//
			// 0x01: xxxx---- -------- -------- --------    Texture palette
			// 0x01: -------- -------x xxxx---- --------    Texture page
			// 0x01: -------- -------- ----x-x- x-x-x-x-    Texture X / 8
			// 0x01: -------- -------- -----x-x -x-x-x-x    Texture Y / 8

			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
			poly_vertex v[4];
			int tx, ty;
			UINT32 color = 0;
			UINT32 header;
			UINT32 command;
			int num_verts = 0;
			int index = i;
			int poly_type = 0;

			command = k001005->_3d_fifo[index++];
			header = k001005->_3d_fifo[index++];

			for (j = 0; j < 4; j++)
			{
				INT16 u2, v2;
				int x, y, z;
				int end = 0;

				x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
				y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = k001005->_3d_fifo[index] & 0x4000;
				end = k001005->_3d_fifo[index] & 0x8000;
				++index;

				z = k001005->_3d_fifo[index];
				++index;

				if (end)
				{
					color = k001005->_3d_fifo[index];
					++index;

					u2 = (k001005->_3d_fifo[index] >> 16) & 0xffff;
					v2 = (k001005->_3d_fifo[index] >>  0) & 0xffff;
					++index;
				}
				else
				{
					u2 = (k001005->_3d_fifo[index] >> 16) & 0xffff;
					v2 = (k001005->_3d_fifo[index] >>  0) & 0xffff;
					++index;
				}

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = *(float*)(&z);
				v[j].p[3] = 1.0f / v[j].p[0];
				v[j].p[1] = u2 * v[j].p[3];
				v[j].p[2] = v2 * v[j].p[3];

				++num_verts;

				if (end)
					break;
			}

			ty = ((header & 0x400) >> 5) |
					((header & 0x100) >> 4) |
					((header & 0x040) >> 3) |
					((header & 0x010) >> 2) |
					((header & 0x004) >> 1) |
					((header & 0x001) >> 0);

			tx = ((header & 0x800) >> 6) |
					((header & 0x200) >> 5) |
					((header & 0x080) >> 4) |
					((header & 0x020) >> 3) |
					((header & 0x008) >> 2) |
					((header & 0x002) >> 1);

			extra->texture_x = tx * 8;
			extra->texture_y = ty * 8;

			extra->texture_page = (header >> 12) & 0x1f;
			extra->texture_palette = (header >> 28) & 0xf;

			extra->texture_mirror_x = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);
			extra->texture_mirror_y = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);

			extra->color = color;

			if (num_verts < 3)
			{
#if POLY_DEVICE
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, &k001005->prev_v[2], &v[0], &v[1]);
				if (k001005->prev_poly_type)
					poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, &k001005->prev_v[2], &k001005->prev_v[3], &v[0]);
//              if (k001005->prev_poly_type)
//                  poly_render_quad(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, &k001005->prev_v[2], &k001005->prev_v[3], &v[0], &v[1]);
//              else
//                  poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, &k001005->prev_v[2], &v[0], &v[1]);
#endif
				memcpy(&k001005->prev_v[0], &k001005->prev_v[2], sizeof(poly_vertex));
				memcpy(&k001005->prev_v[1], &k001005->prev_v[3], sizeof(poly_vertex));
				memcpy(&k001005->prev_v[2], &v[0], sizeof(poly_vertex));
				memcpy(&k001005->prev_v[3], &v[1], sizeof(poly_vertex));
			}
			else
			{
#if POLY_DEVICE
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, &v[0], &v[1], &v[2]);
				if (num_verts > 3)
					poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, &v[2], &v[3], &v[0]);
//              poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, num_verts, v);
#endif
				memcpy(k001005->prev_v, v, sizeof(poly_vertex) * 4);
			}

			k001005->prev_poly_type = poly_type;

			while ((k001005->_3d_fifo[index] & 0xffffff00) != 0x80000000 && index < k001005->_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
#if POLY_DEVICE
				int new_verts = 0;
#endif
				if (poly_type)
				{
					memcpy(&v[0], &k001005->prev_v[2], sizeof(poly_vertex));
					memcpy(&v[1], &k001005->prev_v[3], sizeof(poly_vertex));
				}
				else
				{
					memcpy(&v[0], &k001005->prev_v[1], sizeof(poly_vertex));
					memcpy(&v[1], &k001005->prev_v[2], sizeof(poly_vertex));
				}

				for (j = 2; j < 4; j++)
				{
					INT16 u2, v2;
					int x, y, z;
					int end = 0;

					x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
					y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = k001005->_3d_fifo[index] & 0x4000;
					end = k001005->_3d_fifo[index] & 0x8000;
					++index;

					z = k001005->_3d_fifo[index];
					++index;

					if (end)
					{
						color = k001005->_3d_fifo[index];
						++index;

						u2 = (k001005->_3d_fifo[index] >> 16) & 0xffff;
						v2 = (k001005->_3d_fifo[index] >>  0) & 0xffff;
						++index;
					}
					else
					{
						u2 = (k001005->_3d_fifo[index] >> 16) & 0xffff;
						v2 = (k001005->_3d_fifo[index] >>  0) & 0xffff;
						++index;
					}

					v[j].x = ((float)(x) / 16.0f) + 256.0f;
					v[j].y = ((float)(-y) / 16.0f) + 192.0f;
					v[j].p[0] = *(float*)(&z);
					v[j].p[3] = 1.0f / v[j].p[0];
					v[j].p[1] = u2 * v[j].p[3];
					v[j].p[2] = v2 * v[j].p[3];

#if POLY_DEVICE
					++new_verts;
#endif

					if (end)
						break;
				}

				extra->texture_x = tx * 8;
				extra->texture_y = ty * 8;

				extra->texture_page = (header >> 12) & 0x1f;
				extra->texture_palette = (header >> 28) & 0xf;

				extra->texture_mirror_x = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);
				extra->texture_mirror_y = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);

				extra->color = color;

#if POLY_DEVICE
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, &v[0], &v[1], &v[2]);
				if (new_verts > 1)
					poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, &v[2], &v[3], &v[0]);
//              poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline_tex, 4, new_verts + 2, v);
#endif
				memcpy(k001005->prev_v, v, sizeof(poly_vertex) * 4);
			};

			i = index - 1;
		}
		else if (k001005->_3d_fifo[i] == 0x80000006 || k001005->_3d_fifo[i] == 0x80000026 ||
					k001005->_3d_fifo[i] == 0x80000020 || k001005->_3d_fifo[i] == 0x80000022)
		{
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
			poly_vertex v[4];
			int r, g, b, a;
			UINT32 color;
			int num_verts = 0;
			int index = i;
			int poly_type = 0;

			++index;

			for (j = 0; j < 4; j++)
			{
				int x, y, z;
				int end = 0;

				x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
				y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = k001005->_3d_fifo[index] & 0x4000;
				end = k001005->_3d_fifo[index] & 0x8000;
				++index;

				z = k001005->_3d_fifo[index];
				++index;

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = *(float*)(&z);

				++num_verts;

				if (end)
					break;
			}

			r = (k001005->_3d_fifo[index] >>  0) & 0xff;
			g = (k001005->_3d_fifo[index] >>  8) & 0xff;
			b = (k001005->_3d_fifo[index] >> 16) & 0xff;
			a = (k001005->_3d_fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			index++;

			extra->color = color;

#if POLY_DEVICE
			poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
			if (num_verts > 3)
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline, 1, &v[2], &v[3], &v[0]);
//          poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline, 1, num_verts, v);
#endif
			memcpy(k001005->prev_v, v, sizeof(poly_vertex) * 4);

			while ((k001005->_3d_fifo[index] & 0xffffff00) != 0x80000000 && index < k001005->_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(k001005->poly);
				int new_verts = 0;

				if (poly_type)
				{
					memcpy(&v[0], &k001005->prev_v[2], sizeof(poly_vertex));
					memcpy(&v[1], &k001005->prev_v[3], sizeof(poly_vertex));
				}
				else
				{
					memcpy(&v[0], &k001005->prev_v[1], sizeof(poly_vertex));
					memcpy(&v[1], &k001005->prev_v[2], sizeof(poly_vertex));
				}

				for (j = 2; j < 4; j++)
				{
					int x, y, z;
					int end = 0;

					x = ((k001005->_3d_fifo[index] >>  0) & 0x3fff);
					y = ((k001005->_3d_fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = k001005->_3d_fifo[index] & 0x4000;
					end = k001005->_3d_fifo[index] & 0x8000;
					++index;

					z = k001005->_3d_fifo[index];
					++index;

					v[j].x = ((float)(x) / 16.0f) + 256.0f;
					v[j].y = ((float)(-y) / 16.0f) + 192.0f;
					v[j].p[0] = *(float*)(&z);

					++new_verts;

					if (end)
						break;
				}

				r = (k001005->_3d_fifo[index] >>  0) & 0xff;
				g = (k001005->_3d_fifo[index] >>  8) & 0xff;
				b = (k001005->_3d_fifo[index] >> 16) & 0xff;
				a = (k001005->_3d_fifo[index] >> 24) & 0xff;
				color = (a << 24) | (r << 16) | (g << 8) | (b);
				index++;

				extra->color = color;

#if POLY_DEVICE
				poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
				if (new_verts > 1)
					poly_render_triangle(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[2], &v[3]);
//              poly_render_polygon(k001005->poly, k001005->bitmap[k001005->bitmap_page], &visarea, draw_scanline, 1, new_verts + 2, v);
#endif
				memcpy(k001005->prev_v, v, sizeof(poly_vertex) * 4);
			};

			i = index - 1;
		}
		else if (k001005->_3d_fifo[i] == 0x80000000)
		{
		}
		else if ((k001005->_3d_fifo[i] & 0xffffff00) == 0x80000000)
		{
			/*
			mame_printf_debug("Unknown polygon type %08X:\n", k001005->_3d_fifo[i]);
			for (j = 0; j < 0x20; j++)
			{
			    mame_printf_debug("  %02X: %08X\n", j, k001005->_3d_fifo[i + 1 + j]);
			}
			mame_printf_debug("\n");
			*/
		}
	}
}

void k001005_draw( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	int i, j;

	memcpy(&k001005->cliprect, &cliprect, sizeof(rectangle));

	for (j = cliprect.min_y; j <= cliprect.max_y; j++)
	{
		UINT32 *bmp = &bitmap.pix32(j);
		UINT32 *src = &k001005->bitmap[k001005->bitmap_page ^ 1]->pix32(j);

		for (i = cliprect.min_x; i <= cliprect.max_x; i++)
		{
			if (src[i] & 0xff000000)
			{
				bmp[i] = src[i];
			}
		}
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k001005 )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	const k001005_interface *intf = k001005_get_interface(device);
	int i, width, height;

	k001005->cpu = device->machine().device(intf->cpu);
	k001005->dsp = device->machine().device(intf->dsp);
	k001005->k001006_1 = device->machine().device(intf->k001006_1);
	k001005->k001006_2 = device->machine().device(intf->k001006_2);

	k001005->screen = device->machine().device<screen_device>(intf->screen);
	width = k001005->screen->width();
	height = k001005->screen->height();
	k001005->zbuffer = auto_bitmap_ind32_alloc(device->machine(), width, height);

	k001005->gfxrom = device->machine().root_device().memregion(intf->gfx_memory_region)->base();

	k001005->bitmap[0] = auto_bitmap_rgb32_alloc(device->machine(), k001005->screen->width(), k001005->screen->height());
	k001005->bitmap[1] = auto_bitmap_rgb32_alloc(device->machine(), k001005->screen->width(), k001005->screen->height());

	k001005->texture = auto_alloc_array(device->machine(), UINT8, 0x800000);

	k001005->ram[0] = auto_alloc_array(device->machine(), UINT16, 0x140000);
	k001005->ram[1] = auto_alloc_array(device->machine(), UINT16, 0x140000);

	k001005->fifo = auto_alloc_array(device->machine(), UINT32, 0x800);

	k001005->_3d_fifo = auto_alloc_array(device->machine(), UINT32, 0x10000);

	k001005->poly = poly_alloc(device->machine(), 4000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);

	for (i = 0; i < 128; i++)
	{
		k001005->tex_mirror_table[0][i] = i & 0x3f;
		k001005->tex_mirror_table[1][i] = i & 0x3f;
		k001005->tex_mirror_table[2][i] = ((i & 0x3f) >= 0x20) ? (0x1f - (i & 0x1f)) : i & 0x1f;
		k001005->tex_mirror_table[3][i] = ((i & 0x7f) >= 0x40) ? (0x3f - (i & 0x3f)) : i & 0x3f;
	}


	device->save_pointer(NAME(k001005->texture), 0x800000);
	device->save_pointer(NAME(k001005->ram[0]), 0x140000);
	device->save_pointer(NAME(k001005->ram[1]), 0x140000);
	device->save_pointer(NAME(k001005->fifo), 0x800);
	device->save_pointer(NAME(k001005->_3d_fifo), 0x10000);
	device->save_item(NAME(k001005->status));
	device->save_item(NAME(k001005->ram_ptr));
	device->save_item(NAME(k001005->fifo_read_ptr));
	device->save_item(NAME(k001005->fifo_write_ptr));
	device->save_item(NAME(k001005->_3d_fifo_ptr));
	device->save_item(NAME(k001005->bitmap_page));
	device->save_item(NAME(k001005->prev_poly_type));
	device->save_item(NAME(*k001005->bitmap[0]));
	device->save_item(NAME(*k001005->bitmap[1]));
	device->save_item(NAME(*k001005->zbuffer));

	// FIXME: shall we save poly as well?
}

static DEVICE_RESET( k001005 )
{
	k001005_state *k001005 = k001005_get_safe_token(device);

	k001005->status = 0;
	k001005->ram_ptr = 0;
	k001005->fifo_read_ptr = 0;
	k001005->fifo_write_ptr = 0;
	k001005->_3d_fifo_ptr = 0;
	k001005->bitmap_page = 0;

	memset(k001005->prev_v, 0, sizeof(k001005->prev_v));
	k001005->prev_poly_type = 0;
}

static DEVICE_STOP( k001005 )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	poly_free(k001005->poly);
}

const device_type K001005 = &device_creator<k001005_device>;

k001005_device::k001005_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K001005, "Konami 001005", tag, owner, clock, "k001005", __FILE__)
{
	m_token = global_alloc_clear(k001005_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k001005_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001005_device::device_start()
{
	DEVICE_START_NAME( k001005 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001005_device::device_reset()
{
	DEVICE_RESET_NAME( k001005 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void k001005_device::device_stop()
{
	DEVICE_STOP_NAME( k001005 )(this);
}

/***************************************************************************/
/*                                                                         */
/*                                  001604                                 */
/*                                                                         */
/***************************************************************************/


struct k001604_state
{
	screen_device *screen;
	tilemap_t        *layer_8x8[2];
	tilemap_t        *layer_roz;
	int            gfx_index[2];

	UINT32 *       tile_ram;
	UINT32 *       char_ram;
	UINT32 *       reg;

	int            layer_size;
	int            roz_size;
	int            txt_mem_offset;
	int            roz_mem_offset;
};


#define K001604_NUM_TILES_LAYER0        16384
#define K001604_NUM_TILES_LAYER1        4096

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k001604_state *k001604_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == K001604);

	return (k001604_state *)downcast<k001604_device *>(device)->token();
}

INLINE const k001604_interface *k001604_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == K001604));
	return (const k001604_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static const gfx_layout k001604_char_layout_layer_8x8 =
{
	8, 8,
	K001604_NUM_TILES_LAYER0,
	8,
	{ 8,9,10,11,12,13,14,15 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128 },
	8*128
};

static const gfx_layout k001604_char_layout_layer_16x16 =
{
	16, 16,
	K001604_NUM_TILES_LAYER1,
	8,
	{ 8,9,10,11,12,13,14,15 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16, 9*16, 8*16, 11*16, 10*16, 13*16, 12*16, 15*16, 14*16 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256, 8*256, 9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256 },
	16*256
};


/* FIXME: The TILEMAP_MAPPER below depends on parameters passed by the device interface (being game dependent).
we might simplify the code, by passing the whole TILEMAP_MAPPER as a callback in the interface, but is it really worth? */

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_8x8_0_size0)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 128) + col + k001604->txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_8x8_0_size1)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 256) + col + k001604->txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_8x8_1_size0)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 128) + col + 64 + k001604->txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_8x8_1_size1)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 64 + k001604->txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_roz_128)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 128) + col + k001604->roz_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_roz_256)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 128 + k001604->roz_mem_offset;
}

TILE_GET_INFO_MEMBER(k001604_device::k001604_tile_info_layer_8x8)
{
	k001604_state *k001604 = k001604_get_safe_token(this);
	UINT32 val = k001604->tile_ram[tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = (val & 0x7fff);
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(k001604->gfx_index[0], tile, color, flags);
}

TILE_GET_INFO_MEMBER(k001604_device::k001604_tile_info_layer_roz)
{
	k001604_state *k001604 = k001604_get_safe_token(this);
	UINT32 val = k001604->tile_ram[tile_index];
	int flags = 0;
	int color = (val >> 17) & 0x1f;
	int tile = k001604->roz_size ? (val & 0x7ff) : (val & 0x1fff);

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tile += k001604->roz_size ? 0x800 : 0x2000;

	SET_TILE_INFO_MEMBER(k001604->gfx_index[k001604->roz_size], tile, color, flags);
}


void k001604_draw_back_layer( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	k001604_state *k001604 = k001604_get_safe_token(device);
	bitmap.fill(0, cliprect);

	if ((k001604->reg[0x60 / 4] & 0x40000000) == 0)
		return;

	int tile_size = k001604->roz_size ? 16 : 8;

	INT32 x  = (INT16)((k001604->reg[0x08] >> 16) & 0xffff);
	INT32 y  = (INT16)((k001604->reg[0x08] >>  0) & 0xffff);
	INT32 xx = (INT16)((k001604->reg[0x09] >>  0) & 0xffff);
	INT32 xy = (INT16)((k001604->reg[0x09] >> 16) & 0xffff);
	INT32 yx = (INT16)((k001604->reg[0x0a] >>  0) & 0xffff);
	INT32 yy = (INT16)((k001604->reg[0x0a] >> 16) & 0xffff);

	int pivotx = (INT16)((k001604->reg[0x00] >> 16) & 0xffff);
	int pivoty = (INT16)((k001604->reg[0x00] >>  0) & 0xffff);

	int startx  = ((x - pivotx) * 256) * 32;
	int starty  = ((y - pivoty) * 256) * 32;
	int incxx = (xx) * 32;
	int incxy = (-xy) * 32;
	int incyx = (-yx) * 32;
	int incyy = (yy) * 32;

	bitmap_ind16& pixmap = k001604->layer_roz->pixmap();

	// extract start/end points
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;
	int ey = cliprect.max_y;

	const rgb_t *clut = palette_entry_list_raw(bitmap.palette());

	int window_x, window_y, window_xmask, window_ymask;

	int layer_size = (k001604->reg[0x1b] >> 9) & 3;

	if (k001604->roz_size)
		window_x = ((k001604->reg[0x1b] >> 1) & 3) * 512;
	else
		window_x = ((k001604->reg[0x1b] >> 1) & 1) * 512;

	window_y = 0;

	switch (layer_size)
	{
		case 0: window_xmask = (128 * tile_size) - 1; break;
		case 2: window_xmask = (64 * tile_size) - 1; break;
		case 3: window_xmask = (32 * tile_size) - 1; break;
		default: fatalerror("k001604_draw_back_layer(): layer_size %d\n", layer_size); break;
	}

	window_ymask = pixmap.height() - 1;


	// loop over rows
	while (sy <= ey)
	{
		// initialize X counters
		int x = sx;
		UINT32 cx = startx;
		UINT32 cy = starty;

		UINT32 *dest = &bitmap.pix(sy, sx);

		// loop over columns
		while (x <= ex)
		{
			*dest = clut[pixmap.pix16(((cy >> 16) & window_ymask) + window_y, ((cx >> 16) & window_xmask) + window_x)];

			// advance in X
			cx += incxx;
			cy += incxy;
			x++;
			dest++;
		}

		// advance in Y
		startx += incyx;
		starty += incyy;
		sy++;
	}
}

void k001604_draw_front_layer( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	k001604->layer_8x8[0]->set_scrollx(-cliprect.min_x);
	k001604->layer_8x8[0]->set_scrolly(-cliprect.min_y);

	k001604->layer_8x8[1]->set_scrollx(-cliprect.min_x);
	k001604->layer_8x8[1]->set_scrolly(-cliprect.min_y);

	//k001604->layer_8x8[1]->draw(bitmap, cliprect, 0,0);
	k001604->layer_8x8[0]->draw(bitmap, cliprect, 0,0);
}

READ32_DEVICE_HANDLER( k001604_tile_r )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	return k001604->tile_ram[offset];
}

READ32_DEVICE_HANDLER( k001604_char_r )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	int set, bank;
	UINT32 addr;

	set = (k001604->reg[0x60 / 4] & 0x1000000) ? 0x100000 : 0;

	if (set)
		bank = (k001604->reg[0x60 / 4] >> 8) & 0x3;
	else
		bank = (k001604->reg[0x60 / 4] & 0x3);

	addr = offset + ((set + (bank * 0x40000)) / 4);

	return k001604->char_ram[addr];
}

READ32_DEVICE_HANDLER( k001604_reg_r )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	switch (offset)
	{
		case 0x54/4:    return space.machine().rand() << 16;
		case 0x5c/4:    return space.machine().rand() << 16 | space.machine().rand();
	}

	return k001604->reg[offset];
}

WRITE32_DEVICE_HANDLER( k001604_tile_w )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	int x/*, y*/;
	COMBINE_DATA(k001604->tile_ram + offset);

	if (k001604->layer_size)
	{
		x = offset & 0xff;
		/*y = offset / 256;*/
	}
	else
	{
		x = offset & 0x7f;
		/*y = offset / 128;*/
	}

	if (k001604->layer_size)
	{
		if (x < 64)
		{
			k001604->layer_8x8[0]->mark_tile_dirty(offset);
		}
		else if (x < 128)
		{
			k001604->layer_8x8[1]->mark_tile_dirty(offset);
		}
		else
		{
			k001604->layer_roz->mark_tile_dirty(offset);
		}
	}
	else
	{
		if (x < 64)
		{
			k001604->layer_8x8[0]->mark_tile_dirty(offset);
		}
		else
		{
			k001604->layer_8x8[1]->mark_tile_dirty(offset);
		}

		k001604->layer_roz->mark_tile_dirty(offset);
	}
}

WRITE32_DEVICE_HANDLER( k001604_char_w )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	int set, bank;
	UINT32 addr;

	set = (k001604->reg[0x60/4] & 0x1000000) ? 0x100000 : 0;

	if (set)
		bank = (k001604->reg[0x60 / 4] >> 8) & 0x3;
	else
		bank = (k001604->reg[0x60 / 4] & 0x3);

	addr = offset + ((set + (bank * 0x40000)) / 4);

	COMBINE_DATA(k001604->char_ram + addr);

	space.machine().gfx[k001604->gfx_index[0]]->mark_dirty(addr / 32);
	space.machine().gfx[k001604->gfx_index[1]]->mark_dirty(addr / 128);
}

WRITE32_DEVICE_HANDLER( k001604_reg_w )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	COMBINE_DATA(k001604->reg + offset);

	switch (offset)
	{
		case 0x8:
		case 0x9:
		case 0xa:
			//printf("K001604_reg_w %02X, %08X, %08X\n", offset, data, mem_mask);
			break;
	}

	if (offset != 0x08 && offset != 0x09 && offset != 0x0a /*&& offset != 0x17 && offset != 0x18*/)
	{
		//printf("K001604_reg_w (%d), %02X, %08X, %08X at %08X\n", chip, offset, data, mem_mask, space.device().safe_pc());
	}
}

const device_type K001604 = &device_creator<k001604_device>;

k001604_device::k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K001604, "Konami 001604", tag, owner, clock, "k001604", __FILE__)
{
	m_token = global_alloc_clear(k001604_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k001604_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001604_device::device_start()
{
	k001604_state *k001604 = k001604_get_safe_token(this);
	const k001604_interface *intf = k001604_get_interface(this);
	int roz_tile_size;

	k001604->layer_size = intf->layer_size;     // 0 -> width = 128 tiles, 1 -> width = 256 tiles
	k001604->roz_size = intf->roz_size;     // 0 -> 8x8, 1 -> 16x16

	k001604->txt_mem_offset = intf->txt_mem_offset;
	k001604->roz_mem_offset = intf->roz_mem_offset;

	k001604->gfx_index[0] = intf->gfx_index_1;
	k001604->gfx_index[1] = intf->gfx_index_2;

	k001604->char_ram = auto_alloc_array(machine(), UINT32, 0x200000 / 4);
	k001604->tile_ram = auto_alloc_array(machine(), UINT32, 0x20000 / 4);
	k001604->reg = auto_alloc_array(machine(), UINT32, 0x400 / 4);

	/* create tilemaps */
	roz_tile_size = k001604->roz_size ? 16 : 8;

	if (k001604->layer_size)
	{
		k001604->layer_8x8[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_8x8),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_8x8_0_size1),this), 8, 8, 64, 64);
		k001604->layer_8x8[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_8x8),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_8x8_1_size1),this), 8, 8, 64, 64);

		k001604->layer_roz = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_roz),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_roz_256),this), roz_tile_size, roz_tile_size, 128, 64);
	}
	else
	{
		k001604->layer_8x8[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_8x8),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_8x8_0_size0),this), 8, 8, 64, 64);
		k001604->layer_8x8[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_8x8),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_8x8_1_size0),this), 8, 8, 64, 64);

		k001604->layer_roz = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_roz),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_roz_128),this), roz_tile_size, roz_tile_size, 128, 64);
	}

	k001604->layer_8x8[0]->set_transparent_pen(0);
	k001604->layer_8x8[1]->set_transparent_pen(0);

	machine().gfx[k001604->gfx_index[0]] = auto_alloc(machine(), gfx_element(machine(), k001604_char_layout_layer_8x8, (UINT8*)&k001604->char_ram[0], machine().total_colors() / 16, 0));
	machine().gfx[k001604->gfx_index[1]] = auto_alloc(machine(), gfx_element(machine(), k001604_char_layout_layer_16x16, (UINT8*)&k001604->char_ram[0], machine().total_colors() / 16, 0));

	save_pointer(NAME(k001604->reg), 0x400 / 4);
	save_pointer(NAME(k001604->char_ram), 0x200000 / 4);
	save_pointer(NAME(k001604->tile_ram), 0x20000 / 4);

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001604_device::device_reset()
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	memset(k001604->char_ram, 0, 0x200000);
	memset(k001604->tile_ram, 0, 0x10000);
	memset(k001604->reg, 0, 0x400);
}
