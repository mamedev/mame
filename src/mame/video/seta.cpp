// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 0
        W           shows layer 1
        A           shows the sprites

        Keys can be used together!


                        [ 0, 1 Or 2 Scrolling Layers ]

    Each layer consists of 2 tilemaps: only one can be displayed at a
    given time (the games usually flip continuously between the two).
    The two tilemaps share the same scrolling registers.

        Layer Size:             1024 x 512
        Tiles:                  16x16x4 (16x16x6 in some games)
        Tile Format:

            Offset + 0x0000:
                            f--- ---- ---- ----     Flip X
                            -e-- ---- ---- ----     Flip Y
                            --dc ba98 7654 3210     Code

            Offset + 0x1000:

                            fedc ba98 765- ----     -
                            ---- ---- ---4 3210     Color

            The other tilemap for this layer (always?) starts at
            Offset + 0x2000.


                            [ 1024 Sprites ]

    Sprites are 16x16x4. They are just like those in "The Newzealand Story",
    "Revenge of DOH" etc (tnzs.c). Obviously they're hooked to a 16 bit
    CPU here, so they're mapped a bit differently in memory. Additionally,
    there are two banks of sprites. The game can flip between the two to
    do double buffering, writing to a bit of a control register(see below)


        Spriteram16_2 + 0x000.w

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc ba-- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0x400.w

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     X

        Spriteram16   + 0x000.w

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y



                            [ Floating Tilemap ]

    There's a floating tilemap made of vertical columns composed of 2x16
    "sprites". Each 32 consecutive "sprites" define a column.

    For column I:

        Spriteram16_2 + 0x800 + 0x40 * I:

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc b--- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0xc00 + 0x40 * I:

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     -

    Each column has a variable horizontal position and a vertical scrolling
    value (see also the Sprite Control Registers). For column I:


        Spriteram16   + 0x400 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y

        Spriteram16   + 0x408 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Low Bits Of X



                        [ Sprites Control Registers ]


        Spriteram16   + 0x601.b

                        7--- ----       0
                        -6-- ----       Flip Screen
                        --5- ----       0
                        ---4 ----       1 (Sprite Enable?)
                        ---- 3210       ???

        Spriteram16   + 0x603.b

                        7--- ----       0
                        -6-- ----       Sprite Bank
                        --5- ----       0 = Sprite Buffering (blandia,msgundam,qzkklogy)
                        ---4 ----       0
                        ---- 3210       Columns To Draw (1 is the special value for 16)

        Spriteram16   + 0x605.b

                        7654 3210       High Bit Of X For Columns 7-0

        Spriteram16   + 0x607.b

                        7654 3210       High Bit Of X For Columns f-8




***************************************************************************/

#include "emu.h"
#include "includes/seta.h"
#include "screen.h"

/* note that drgnunit, stg and qzkklogy run on the same board, yet they need different alignment */
static const seta_state::game_offset game_offsets[] =
{
	// x offsets
	// "game",    { spr, spr_flip}, {tmap, tmap_flip}

	/* only sprites */
	{ "tndrcade",   {  0,  0 } },             // correct (start grid, wall at beginning of game)
	{ "tndrcadej",  {  0,  0 } },             // "
	{ "wits",       {  0,  0 } },             // unknown
	{ "thunderl",   {  0,  0 } },             // unknown
	{ "wiggie",     {  0,  0 } },             // some problems but they seem y co-ordinate related?
	{ "superbar",   {  0,  0 } },             // "
	{ "pairlove",   {  0,  0 } },             // unknown
	{ "blockcar",   {  0,  0 } },             // unknown
	{ "neobattl",   {  0,  0 } },             // correct (test grid)
	{ "umanclub",   {  0,  0 } },             // correct (test grid)
	{ "atehate",    {  0,  0 } },             // correct (test grid)
	{ "kiwame",     {  0,-16 } },             // correct (test grid)
	{ "krzybowl",   {  0,  0 } },             // correct (test grid)
	{ "orbs",       {  0,  0 } },             // unknown
	{ "keroppi",    {  0,  0 } },             // unknown

	/* 1 layer */
	{ "twineagl",   {  0,  0 }, {  0,  -3 } }, // unknown
	{ "downtown",   {  1,  0 }, { -1,   0 } }, // sprites correct (test grid), tilemap unknown but at least -1 non-flipped to fix glitches later in the game
	{ "downtown2",  {  1,  0 }, { -1,   0 } }, // "
	{ "downtownj",  {  1,  0 }, { -1,   0 } }, // "
	{ "downtownp",  {  1,  0 }, { -1,   0 } }, // "
	{ "usclssic",   {  1,  2 }, {  0,  -1 } }, // correct (test grid and bg)
	{ "calibr50",   { -1,  2 }, { -3,  -2 } }, // correct (test grid and roof in animation at beginning of game)
	{ "arbalest",   {  0,  1 }, { -2,  -1 } }, // correct (test grid and landing pad at beginning of game)
	{ "metafox",    {  0,  0 }, { 16, -19 } }, // sprites unknown, tilemap correct (test grid)
	{ "setaroul",   {  7,  0 }, {  5,   0 } }, // unknown (flipped offsets are unused: game handles flipping manually without setting the flip bit)
	{ "drgnunit",   {  2,  2 }, { -2,  -2 } }, // correct (test grid and I/O test)
	{ "jockeyc",    {  0,  0 }, { -2, 126 } }, // sprites correct? (bets), tilemap correct (test grid)
	{ "inttoote2",  {  0,  0 }, { -2, 126 } }, // "
	{ "inttoote",   {  0,  0 }, { -2,   0 } }, // "
	{ "stg",        {  0,  0 }, { -2,  -2 } }, // sprites correct? (panel), tilemap correct (test grid)
	{ "qzkklogy",   {  1,  1 }, { -1,  -1 } }, // correct (timer, test grid)
	{ "qzkklgy2",   {  0,  0 }, { -1,  -3 } }, // sprites unknown, tilemaps correct (test grid)

	/* 2 layers */
	{ "rezon",      {  0,  0 }, { -2,  -2 } }, // correct (test grid)
	{ "rezont",     {  0,  0 }, { -2,  -2 } }, // "
	{ "blandia",    {  0,  8 }, { -2,   6 } }, // correct (test grid, startup bg)
	{ "blandiap",   {  0,  8 }, { -2,   6 } }, // "
	{ "zingzip",    {  0,  0 }, { -1,  -2 } }, // sprites unknown, tilemaps correct (test grid)
	{ "eightfrc",   {  3,  4 }, {  0,   0 } }, // correct (test mode)
	{ "daioh",      {  0,  0 }, { -1,  -1 } }, // correct (test grid, planet)
	{ "daioha",     {  0,  0 }, { -1,  -1 } }, // "
	{ "daiohc",     {  0,  0 }, { -1,  -1 } }, // "
	{ "daiohp",     {  0,  0 }, { -1,  -1 } }, // "
	{ "msgundam",   {  0,  0 }, { -2,  -2 } }, // correct (test grid, banpresto logo)
	{ "msgundam1",  {  0,  0 }, { -2,  -2 } }, // "
	{ "oisipuzl",   {  1,  1 }, { -1,  -1 } }, // correct (test mode) flip screen not supported?
	{ "triplfun",   {  1,  1 }, { -1,  -1 } }, // "
	{ "wrofaero",   {  0,  0 }, {  0,   0 } }, // correct (test mode)
	{ "jjsquawk",   {  1,  1 }, { -1,  -1 } }, // correct (test mode)
	{ "jjsquawkb",  {  1,  1 }, { -1,  -1 } }, // "
	{ "kamenrid",   {  0,  0 }, { -2,  -2 } }, // correct (map, banpresto logo)
	{ "extdwnhl",   {  0,  0 }, { -2,  -2 } }, // correct (test grid, background images)
	{ "sokonuke",   {  0,  0 }, { -2,  -2 } }, // correct (game selection, test grid)
	{ "gundhara",   {  0,  0 }, {  0,   0 } }, // correct (test mode)
	{ "zombraid",   {  0,  0 }, { -2,  -2 } }, // correct for normal, flip screen not working yet
	{ "zombraidp",  {  0,  0 }, { -2,  -2 } }, // correct for normal, flip screen not working yet
	{ "zombraidpj", {  0,  0 }, { -2,  -2 } }, // correct for normal, flip screen not working yet
	{ "madshark",   {  0,  0 }, {  0,   0 } }, // unknown (wrong when flipped, but along y)
	{ "utoukond",   {  0,  0 }, { -2,   0 } }, // unknown (wrong when flipped, but along y)
	{ "crazyfgt",   {  0,  0 }, { -2,   0 } }, // wrong (empty background column in title screen, but aligned sprites in screen select)
	{ "magspeed",   {  0,  0 }, { -2,   0 } }, // floating tilemap maybe 1px off in test grid

	{ nullptr }
};


/*      76-- ----
        --5- ----     Sound Enable
        ---4 ----     toggled in IRQ1 by many games, irq acknowledge?
                      [original comment for the above: ?? 1 in oisipuzl, sokonuke (layers related)]
        ---- 3---     Coin #1 Lock Out
        ---- -2--     Coin #0 Lock Out
        ---- --1-     Coin #1 Counter
        ---- ---0     Coin #0 Counter     */

// some games haven't the coin lockout device (blandia, eightfrc, extdwnhl, gundhara, kamenrid, magspeed, sokonuke, zingzip, zombraid)
void seta_state::seta_coin_counter_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	if (m_x1.found())
		m_x1->enable_w(BIT(data, 6));
}

void seta_state::seta_coin_lockout_w(u8 data)
{
	seta_coin_counter_w(data);

	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(1, !BIT(data, 3));
}


void seta_state::seta_vregs_w(u8 data)
{
	m_vregs = data;

	/* Partly handled in vh_screenrefresh:

	        76-- ----
	        --54 3---     Samples Bank (in blandia, eightfrc, zombraid)
	        ---- -2--
	        ---- --1-     Sprites Above Frontmost Layer
	        ---- ---0     Layer 0 Above Layer 1
	*/

	const int new_bank = (data >> 3) & 0x7;

	if (new_bank != m_samples_bank)
	{
		m_samples_bank = new_bank;
		if (m_x1_bank != nullptr)
			m_x1_bank->set_entry(m_samples_bank);
	}
}


/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

Offset + 0x0000:
                    f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc ba98 7654 3210     Code

Offset + 0x1000:

                    fedc ba98 765- ----     -
                    ---- ---- ---4 3210     Color


                      [ TileMaps Control Registers]

Offset + 0x0:                               Scroll X
Offset + 0x2:                               Scroll Y
Offset + 0x4:
                    fedc ba98 765- ----     -
                    ---- ---- ---4 ----     Tilemap color mode switch (used in blandia and the other games using 6bpp graphics)
                    ---- ---- ---- 3---     Tilemap Select (There Are 2 Tilemaps Per Layer)
                    ---- ---- ---- -21-     0 (1 only in eightfrc, when flip is on!)
                    ---- ---- ---- ---0     ?

***************************************************************************/

TILE_GET_INFO_MEMBER(seta_state::twineagl_get_tile_info)
{
	const u16 *vram = &m_vram[0][m_rambank[0] ? 0x1000 : 0];
	u16 code = vram[tile_index];
	const u16 attr = vram[tile_index + 0x800];
	if ((code & 0x3e00) == 0x3e00)
		code = (code & 0xc07f) | ((m_twineagl_tilebank[(code & 0x0180) >> 7] >> 1) << 7);
	SET_TILE_INFO_MEMBER(1, (code & 0x3fff), attr & 0x1f, TILE_FLIPXY((code & 0xc000) >> 14));
}

template<int Layer>
TILE_GET_INFO_MEMBER(seta_state::get_tile_info)
{
	int gfx = 1 + Layer;
	const u16 *vram = &m_vram[Layer][m_rambank[Layer] ? 0x1000 : 0];
	const u16 *vctrl = m_vctrl[Layer];
	const u16 code = vram[tile_index];
	const u16 attr = vram[tile_index + 0x800];

	if (m_gfxdecode->gfx(gfx + ((vctrl[4/2] & 0x10) >> m_color_mode_shift)) != nullptr)
	{
		gfx += (vctrl[4/2] & 0x10) >> m_color_mode_shift;
	}
	else
	{
		popmessage("Missing Color Mode = 1 for Layer = %d. Contact MAMETesters.", Layer);
	}

	SET_TILE_INFO_MEMBER(gfx, m_tiles_offset + (code & 0x3fff), attr & 0x1f, TILE_FLIPXY((code & 0xc000) >> 14));
}


void seta_state::twineagl_tilebank_w(offs_t offset, u8 data)
{
	if (m_twineagl_tilebank[offset] != data)
	{
		m_twineagl_tilebank[offset] = data;
		machine().tilemap().mark_all_dirty();
	}
}


/* 2 layers */
VIDEO_START_MEMBER(seta_state,seta_2_layers)
{
	VIDEO_START_CALL_MEMBER(seta_no_layers);

	/* Each layer consists of 2 tilemaps: only one can be displayed
	   at any given time */

	/* layer 0 */
	m_tilemap[0] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seta_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS,
			16, 16, 64, 32);

	/* layer 1 */
	m_tilemap[1] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seta_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS,
			16, 16, 64, 32);

	m_tilemaps_flip = 0;
	m_color_mode_shift = 3;

	for (int layer = 0; layer < 2; layer++)
	{
		m_tilemap[layer]->set_transparent_pen(0);
	}
}

VIDEO_START_MEMBER(seta_state,oisipuzl_2_layers)
{
	VIDEO_START_CALL_MEMBER(seta_2_layers);
	m_tilemaps_flip = 1;

	// position kludges
	m_seta001->set_fg_yoffsets(-0x12, 0x0e);
}


/* 1 layer */
VIDEO_START_MEMBER(seta_state,seta_1_layer)
{
	VIDEO_START_CALL_MEMBER(seta_no_layers);

	/* Each layer consists of 2 tilemaps: only one can be displayed
	   at any given time */

	/* layer 0 */
	m_tilemap[0] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seta_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS,
			16, 16, 64, 32);

	m_color_mode_shift = 4;

	m_tilemap[0]->set_transparent_pen(0);
}

VIDEO_START_MEMBER(setaroul_state,setaroul_1_layer)
{
	VIDEO_START_CALL_MEMBER(seta_1_layer);

	// position kludges
	m_seta001->set_bg_yoffsets(0, -0x1);
	m_seta001->set_bg_xoffsets(0, 0x2);
}

VIDEO_START_MEMBER(jockeyc_state,jockeyc_1_layer)
{
	VIDEO_START_CALL_MEMBER(seta_1_layer);

	// position kludges
	m_seta001->set_fg_yoffsets(-0x12+8, 0x0e);
}

VIDEO_START_MEMBER(seta_state,twineagl_1_layer)
{
	VIDEO_START_CALL_MEMBER(seta_no_layers);

	/* Each layer consists of 2 tilemaps: only one can be displayed
	   at any given time */

	/* layer 0 */
	m_tilemap[0] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seta_state::twineagl_get_tile_info)), TILEMAP_SCAN_ROWS,
			16, 16, 64, 32);

	m_tilemap[0]->set_transparent_pen(0);
}

SETA001_SPRITE_GFXBANK_CB_MEMBER(seta_state::setac_gfxbank_callback)
{
	const int bank = (color & 0x06) >> 1;
	code = (code & 0x3fff) + (bank * 0x4000);

	return code;
}

/* NO layers, only sprites */
VIDEO_START_MEMBER(seta_state,seta_no_layers)
{
	m_tilemap[0] = nullptr;
	m_tilemap[1] = nullptr;

	m_tilemaps_flip = 0;

	m_global_offsets = game_offsets;
	while (m_global_offsets->gamename && strcmp(machine().system().name, m_global_offsets->gamename))
		m_global_offsets++;
	m_samples_bank = -1;    // set the samples bank to an out of range value at start-up
	if (m_x1_bank != nullptr)
		m_x1_bank->set_entry(0); // TODO : Unknown init

	// position kludges
	m_seta001->set_fg_xoffsets(m_global_offsets->sprite_offs[1], m_global_offsets->sprite_offs[0]);
	m_seta001->set_fg_yoffsets(-0x12, 0x0e);
	m_seta001->set_bg_yoffsets(0x1, -0x1);
	save_item(NAME(m_rambank));

	m_vregs = 0;
	save_item(NAME(m_vregs));
}

VIDEO_START_MEMBER(seta_state,kyustrkr_no_layers)
{
	VIDEO_START_CALL_MEMBER(seta_no_layers);

	// position kludges
	m_seta001->set_fg_yoffsets(-0x0a, 0x0e);
	m_seta001->set_bg_yoffsets(0x1, -0x1);
}


/***************************************************************************


                            Palette Init Functions


***************************************************************************/


/* 2 layers, 6 bit deep.

   The game can select to repeat every 16 colors to fill the 64 colors for the 6bpp gfx
   or to use the first 64 colors of the palette regardless of the color code!
*/
void seta_state::blandia_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			// layer 2-3
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x200 + ((color << 4) | (pen & 0x0f)));
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x200 + pen);

			// layer 0-1
			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x400 + ((color << 4) | (pen & 0x0f)));
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x400 + pen);
		}
	}

	// setup the colortable for the effect palette.
	// what are used for palette from 0x800 to 0xBFF?
	for (int i = 0; i < 0x2200; i++)
		palette.set_pen_indirect(0x2200 + i, 0x600 + (i & 0x1ff));
}


/* layers have 6 bits per pixel, but the color code has a 16 colors granularity,
   even if the low 2 bits are ignored (so there are only 4 different palettes) */
void seta_state::gundhara_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));

			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}


/* layers have 6 bits per pixel, but the color code has a 16 colors granularity */
void seta_state::jjsquawk_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x400 + (((color << 4) + pen) & 0x1ff)); // used by madshark
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x400 + (((color << 4) + pen) & 0x1ff));

			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x200 + (((color << 4) + pen) & 0x1ff)); // used by madshark
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x200 + (((color << 4) + pen) & 0x1ff));
		}
	}
}


// layer 0 is 6 bit per pixel, but the color code has a 16 colors granularity
void seta_state::zingzip_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x400 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0xc00 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}

// color prom
void seta_state::palette_init_RRRRRGGGGGBBBBB_proms(palette_device &palette) const
{
	const u8 *const color_prom = memregion("proms")->base();
	for (int x = 0; x < 0x200 ; x++)
	{
		const int data = (color_prom[x*2] << 8) | color_prom[x*2 + 1];
		palette.set_pen_color(x, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
	}
}

void setaroul_state::setaroul_palette(palette_device &palette) const
{
	m_gfxdecode->gfx(0)->set_granularity(16);
	m_gfxdecode->gfx(1)->set_granularity(16);

	palette_init_RRRRRGGGGGBBBBB_proms(palette);
}

void usclssic_state::usclssic_palette(palette_device &palette) const
{
	const u8 *const color_prom = memregion("proms")->base();

	// decode PROM
	for (int x = 0; x < 0x200; x++)
	{
		const u16 data = (color_prom[x*2] << 8) | color_prom[x*2 + 1];
		const rgb_t color(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (x >= 0x100)
			palette.set_indirect_color(x + 0x000, color);
		else
			palette.set_indirect_color(x + 0x300, color);
	}

	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x200 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0xa00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}


void seta_state::set_pens()
{
	for (int i = 0; i < m_paletteram[0].bytes() / 2; i++)
	{
		const u16 data = m_paletteram[0][i];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (m_palette->indirect_entries() != 0)
			m_palette->set_indirect_color(i, color);
		else
			m_palette->set_pen_color(i, color);
	}

	if (m_paletteram[1] != nullptr)
	{
		for (int i = 0; i < m_paletteram[1].bytes() / 2; i++)
		{
			const u16 data = m_paletteram[1][i];

			rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

			if (m_palette->indirect_entries() != 0)
				m_palette->set_indirect_color(i + m_paletteram[0].bytes() / 2, color);
			else
				m_palette->set_pen_color(i + m_paletteram[0].bytes() / 2, color);
		}
	}
}


void usclssic_state::usclssic_set_pens()
{
	for (int i = 0; i < 0x200; i++)
	{
		const u16 data = m_paletteram[0][i];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (i >= 0x100)
			m_palette->set_indirect_color(i - 0x100, color);
		else
			m_palette->set_indirect_color(i + 0x200, color);
	}
}


void seta_state::draw_tilemap_palette_effect(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tilemap, int scrollx, int scrolly, int gfxnum, int flipscreen)
{
	gfx_element *gfx_tilemap = m_gfxdecode->gfx(gfxnum);
	const bitmap_ind16 &src_bitmap = tilemap->pixmap();
	const int opaque_mask = gfx_tilemap->granularity() - 1;
	const int pixel_effect_mask = gfx_tilemap->colorbase() + (gfx_tilemap->colors() - 1) * gfx_tilemap->granularity();
	int p;

	const int width_mask = src_bitmap.width() - 1;
	const int height_mask = src_bitmap.height() - 1;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 *dest = &bitmap.pix16(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (!flipscreen)
			{
				p = src_bitmap.pix16((y + scrolly) & height_mask, (x + scrollx) & width_mask);
			}
			else
			{
				p = src_bitmap.pix16((y - scrolly - 256) & height_mask, (x - scrollx - 512) & width_mask);
			}

			// draw not transparent pixels
			if (p & opaque_mask)
			{
				// pixels with the last color are not drawn and the 2nd palette is added to the current bitmap color
				if ((p & pixel_effect_mask) == pixel_effect_mask)
				{
					dest[x] = m_palette->entries() / 2 + dest[x];
				}
				else
				{
					dest[x] = m_palette->pen(p);
				}
			}
		}
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/


/* For games without tilemaps */
u32 seta_state::screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	bitmap.fill(0x1f0, cliprect);

	m_seta001->draw_sprites(screen, bitmap,cliprect,0x1000);
	return 0;
}


/* For games with 1 or 2 tilemaps */
void seta_state::seta_layers_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_bank_size)
{
	const rectangle &visarea = screen.visible_area();
	const int vis_dimy = visarea.max_y - visarea.min_y + 1;

	// check tilemaps color modes

	for (int layer = 0; layer < 2; layer++)
	{
		if (m_tilemap[layer])
		{
			if (m_current_tilemap_mode[layer] != (m_vctrl[layer][4/2] & 0x10))
			{
				m_current_tilemap_mode[layer] = m_vctrl[layer][4/2] & 0x10;
				m_tilemap[layer]->mark_all_dirty();
			}
		}
	}

	const int flip = m_seta001->is_flipped() ^ m_tilemaps_flip;
	machine().tilemap().set_flip_all(flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

	int bank[2]{ 0, 0 }, x[2]{ 0, 0 }, y[2]{ 0, 0 };
	for (int layer = 0; layer < 2; layer++)
	{
		if (m_tilemap[layer])
		{
			x[layer]     =   m_vctrl[layer][0/2];
			y[layer]     =   m_vctrl[layer][2/2];
			bank[layer]  =   m_vctrl[layer][4/2];
			bank[layer]  =   (bank[layer] & 0x0008) ? 1 : 0; /*&& (bank[layer] & 0x0001)*/

			/* Select tilemap bank, Only one tilemap bank per layer is enabled */
			if (m_rambank[layer] != bank[layer])
			{
				m_rambank[layer] = bank[layer];
				m_tilemap[layer]->mark_all_dirty();
			}

			/* the hardware wants different scroll values when flipped */

			/*  bg x scroll      flip
			    metafox     0000 025d = 0, $400-$1a3 = $400 - $190 - $13
			    eightfrc    ffe8 0272
			                fff0 0260 = -$10, $400-$190 -$10
			                ffe8 0272 = -$18, $400-$190 -$18 + $1a      */

			x[layer] += 0x10 - m_global_offsets->tilemap_offs[flip ? 1 : 0];
			y[layer] -= (256 - vis_dimy)/2;
			if (flip)
			{
				x[layer] = -x[layer] - 512;
				y[layer] = y[layer] - vis_dimy;
			}

			m_tilemap[layer]->set_scrollx(0, x[layer]);
			m_tilemap[layer]->set_scrolly(0, y[layer]);
		}
		else
		{
			x[layer] = 0;
			y[layer] = 0;
		}
	}

	unsigned layers_ctrl = ~0U;
#ifdef MAME_DEBUG
	if (screen.machine().input().code_pressed(KEYCODE_Z))
	{   int msk = 0;
		if (screen.machine().input().code_pressed(KEYCODE_Q))   msk |= 1;
		if (screen.machine().input().code_pressed(KEYCODE_W))   msk |= 2;
		if (screen.machine().input().code_pressed(KEYCODE_A))   msk |= 8;
		if (msk != 0) layers_ctrl &= msk;

		if (m_tilemap[1])
			popmessage("VR:%02X L0:%04X L1:%04X",
				m_vregs, m_vctrl[0][4/2], m_vctrl[1][4/2]);
		else if (m_tilemap[0])    popmessage("L0:%04X", m_vctrl[0][4/2]);
	}
#endif

	bitmap.fill(0, cliprect);

	const int order = m_tilemap[1] ? m_vregs : 0;
	if (order & 1)  // swap the layers?
	{
		if (m_tilemap[1])
		{
			if (layers_ctrl & 2) m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		}

		if (order & 2)  // layer-sprite priority?
		{
			if (layers_ctrl & 8) m_seta001->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);

			if (order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1) m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
		}
		else
		{
			if (order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1) m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

			if (layers_ctrl & 8) m_seta001->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);
		}
	}
	else
	{
		if (layers_ctrl & 1) m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

		if (order & 2)  // layer-sprite priority?
		{
			if (layers_ctrl & 8) m_seta001->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);

			if ((order & 4) && m_paletteram[1] != nullptr)
			{
				draw_tilemap_palette_effect(bitmap, cliprect, m_tilemap[1], x[1], y[1], 2 + ((m_vctrl[1][4/2] & 0x10) >> m_color_mode_shift), flip);
			}
			else
			{
				if (order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (m_tilemap[1])
				{
					if (layers_ctrl & 2) m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
				}
			}
		}
		else
		{
			if ((order & 4) && m_paletteram[1] != nullptr)
			{
				draw_tilemap_palette_effect(bitmap, cliprect, m_tilemap[1], x[1], y[1], 2 + ((m_vctrl[1][4/2] & 0x10) >> m_color_mode_shift), flip);
			}
			else
			{
				if (order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (m_tilemap[1])
				{
					if (layers_ctrl & 2) m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
				}
			}

			if (layers_ctrl & 8) m_seta001->draw_sprites(screen,bitmap,cliprect,sprite_bank_size);
		}
	}

}

u32 seta_state::screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	seta_layers_update(screen, bitmap, cliprect, 0x1000);
	return 0;
}


u32 setaroul_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x0, cliprect);

	if (m_led & 0x80)
		seta_layers_update(screen, bitmap, cliprect, 0x800);

	return 0;
}

WRITE_LINE_MEMBER(setaroul_state::screen_vblank)
{
	// rising edge
	if (state)
		m_seta001->tnzs_eof();
}


u32 seta_state::screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	return screen_update_seta_layers(screen, bitmap, cliprect);
}


u32 usclssic_state::screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	usclssic_set_pens();
	return screen_update_seta_layers(screen, bitmap, cliprect);
}
