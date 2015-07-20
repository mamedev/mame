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

/* note that drgnunit, stg and qzkklogy run on the same board, yet they need different alignment */
static const game_offset game_offsets[] =
{
	/* only sprites */
	{ "tndrcade", {  0,  0 } },             // correct (start grid, wall at beginning of game)
	{ "tndrcadej",{  0,  0 } },             // "
	{ "wits",     {  0,  0 } },             // unknown
	{ "thunderl", {  0,  0 } },             // unknown
	{ "wiggie",   {  0,  0 } },             // some problems but they seem y co-ordinate related?
	{ "superbar", {  0,  0 } },             // "
	{ "pairlove", {  0,  0 } },             // unknown
	{ "blockcar", {  0,  0 } },             // unknown
	{ "neobattl", {  0,  0 } },             // correct (test grid)
	{ "umanclub", {  0,  0 } },             // correct (test grid)
	{ "atehate",  {  0,  0 } },             // correct (test grid)
	{ "kiwame",   {  0,-16 } },             // correct (test grid)
	{ "krzybowl", {  0,  0 } },             // correct (test grid)
	{ "orbs",     {  0,  0 } },             // unknown
	{ "keroppi",  {  0,  0 } },             // unknown

	/* 1 layer */
	{ "twineagl", {  0,  0 }, {  0, -3 } }, // unknown
	{ "downtown", {  1,  0 }, { -1,  0 } }, // sprites correct (test grid), tilemap unknown but at least -1 non-flipped to fix glitches later in the game
	{ "downtown2",{  1,  0 }, { -1,  0 } }, // "
	{ "downtownj",{  1,  0 }, { -1,  0 } }, // "
	{ "downtownp",{  1,  0 }, { -1,  0 } }, // "
	{ "usclssic", {  1,  2 }, {  0, -1 } }, // correct (test grid and bg)
	{ "calibr50", { -1,  2 }, { -3, -2 } }, // correct (test grid and roof in animation at beginning of game)
	{ "arbalest", {  0,  1 }, { -2, -1 } }, // correct (test grid and landing pad at beginning of game)
	{ "metafox",  {  0,  0 }, { 16,-19 } }, // sprites unknown, tilemap correct (test grid)
	{ "setaroul", {  0,  0 }, {  0,  0 } }, // unknown
	{ "drgnunit", {  2,  2 }, { -2, -2 } }, // correct (test grid and I/O test)
	{ "jockeyc",  {  0,  0 }, { -2,  0 } }, // sprites unknown, tilemap correct (test grid)
	{ "inttoote", {  0,  0 }, { -2,  0 } }, // "
	{ "inttootea",{  0,  0 }, { -2,  0 } }, // "
	{ "stg",      {  0,  0 }, { -2, -2 } }, // sprites correct? (panel), tilemap correct (test grid)
	{ "qzkklogy", {  1,  1 }, { -1, -1 } }, // correct (timer, test grid)
	{ "qzkklgy2", {  0,  0 }, { -1, -3 } }, // sprites unknown, tilemaps correct (test grid)

	/* 2 layers */
	{ "rezon",    {  0,  0 }, { -2, -2 } }, // correct (test grid)
	{ "rezont",   {  0,  0 }, { -2, -2 } }, // "
	{ "blandia",  {  0,  8 }, { -2,  6 } }, // correct (test grid, startup bg)
	{ "blandiap", {  0,  8 }, { -2,  6 } }, // "
	{ "zingzip",  {  0,  0 }, { -1, -2 } }, // sprites unknown, tilemaps correct (test grid)
	{ "eightfrc", {  3,  4 }, {  0,  0 } }, // correct (test mode)
	{ "daioh",    {  0,  0 }, { -1, -1 } }, // correct (test grid, planet)
	{ "daioha",   {  0,  0 }, { -1, -1 } }, // "
	{ "daiohc",   {  0,  0 }, { -1, -1 } }, // "
	{ "daiohp",   {  0,  0 }, { -1, -1 } }, // "
	{ "msgundam", {  0,  0 }, { -2, -2 } }, // correct (test grid, banpresto logo)
	{ "msgundam1",{  0,  0 }, { -2, -2 } }, // "
	{ "oisipuzl", {  1,  1 }, { -1, -1 } }, // correct (test mode) flip screen not supported?
	{ "triplfun", {  1,  1 }, { -1, -1 } }, // "
	{ "wrofaero", {  0,  0 }, {  0,  0 } }, // correct (test mode)
	{ "jjsquawk", {  1,  1 }, { -1, -1 } }, // correct (test mode)
	{ "jjsquawkb",{  1,  1 }, { -1, -1 } }, // "
	{ "kamenrid", {  0,  0 }, { -2, -2 } }, // correct (map, banpresto logo)
	{ "extdwnhl", {  0,  0 }, { -2, -2 } }, // correct (test grid, background images)
	{ "sokonuke", {  0,  0 }, { -2, -2 } }, // correct (game selection, test grid)
	{ "gundhara", {  0,  0 }, {  0,  0 } }, // correct (test mode)
	{ "zombraid", {  0,  0 }, { -2, -2 } }, // correct for normal, flip screen not working yet
	{ "zombraidp", {  0,  0 }, { -2, -2 } }, // correct for normal, flip screen not working yet
	{ "zombraidpj", {  0,  0 }, { -2, -2 } }, // correct for normal, flip screen not working yet
	{ "madshark", {  0,  0 }, {  0,  0 } }, // unknown (wrong when flipped, but along y)
	{ "utoukond", {  0,  0 }, { -2,  0 } }, // unknown (wrong when flipped, but along y)
	{ "crazyfgt", {  0,  0 }, { -2,  0 } }, // wrong (empty background column in title screen, but aligned sprites in screen select)
	{ "magspeed", {  0,  0 }, { -2,  0 } }, // floating tilemap maybe 1px off in test grid

	{ NULL }
};


/*  ---- 3---       Coin #1 Lock Out
    ---- -2--       Coin #0 Lock Out
    ---- --1-       Coin #1 Counter
    ---- ---0       Coin #0 Counter     */

void seta_state::seta_coin_lockout_w(int data)
{
	static const char *const seta_nolockout[] =
	{ "blandia", "eightfrc", "extdwnhl", "gundhara", "kamenrid", "magspeed", "sokonuke", "zingzip", "zombraid", "zombraidp", "zombraidpj"};

	/* Only compute seta_coin_lockout when confronted with a new gamedrv */
	if (!m_coin_lockout_initialized)
	{
		m_coin_lockout_initialized = true;
		int i;

		m_coin_lockout = 1;
		for (i=0; i<ARRAY_LENGTH(seta_nolockout); i++)
		{
			if (strcmp(machine().system().name, seta_nolockout[i]) == 0 ||
				strcmp(machine().system().parent, seta_nolockout[i]) == 0)
			{
				m_coin_lockout = 0;
				break;
			}
		}
	}

	coin_counter_w      (machine(), 0, (( data) >> 0) & 1 );
	coin_counter_w      (machine(), 1, (( data) >> 1) & 1 );

	/* some games haven't the coin lockout device */
	if (    !m_coin_lockout )
		return;
	coin_lockout_w      (machine(), 0, ((~data) >> 2) & 1 );
	coin_lockout_w      (machine(), 1, ((~data) >> 3) & 1 );
}


WRITE16_MEMBER(seta_state::seta_vregs_w)
{
	COMBINE_DATA(&m_vregs[offset]);
	switch (offset)
	{
		case 0/2:

/*      fedc ba98 76-- ----
        ---- ---- --5- ----     Sound Enable
        ---- ---- ---4 ----     toggled in IRQ1 by many games, irq acknowledge?
                                [original comment for the above: ?? 1 in oisipuzl, sokonuke (layers related)]
        ---- ---- ---- 3---     Coin #1 Lock Out
        ---- ---- ---- -2--     Coin #0 Lock Out
        ---- ---- ---- --1-     Coin #1 Counter
        ---- ---- ---- ---0     Coin #0 Counter     */
			if (ACCESSING_BITS_0_7)
			{
				seta_coin_lockout_w (data & 0x0f);
				if (m_x1 != NULL)
					m_x1->enable_w (data & 0x20);
				coin_counter_w(machine(), 0,data & 0x01);
				coin_counter_w(machine(), 1,data & 0x02);
			}
			break;

		case 2/2:
			if (ACCESSING_BITS_0_7)
			{
				int new_bank;

				/* Partly handled in vh_screenrefresh:

				        fedc ba98 76-- ----
				        ---- ---- --54 3---     Samples Bank (in blandia, eightfrc, zombraid)
				        ---- ---- ---- -2--
				        ---- ---- ---- --1-     Sprites Above Frontmost Layer
				        ---- ---- ---- ---0     Layer 0 Above Layer 1
				*/

				new_bank = (data >> 3) & 0x7;

				if (new_bank != m_samples_bank)
				{
					UINT8 *rom = memregion("x1snd")->base();
					int samples_len = memregion("x1snd")->bytes();
					int addr;

					m_samples_bank = new_bank;

					if (samples_len == 0x240000)    /* blandia, eightfrc */
					{
						addr = 0x40000 * new_bank;
						if (new_bank >= 3)  addr += 0x40000;

						if ( (samples_len > 0x100000) && ((addr+0x40000) <= samples_len) )
							memcpy(&rom[0xc0000],&rom[addr],0x40000);
						else
							logerror("PC %06X - Invalid samples bank %02X !\n", space.device().safe_pc(), new_bank);
					}
					else if (samples_len == 0x480000)   /* zombraid */
					{
						/* bank 1 is never explicitly selected, 0 is used in its place */
						if (new_bank == 0) new_bank = 1;
						addr = 0x80000 * new_bank;
						if (new_bank > 0) addr += 0x80000;

						memcpy(&rom[0x80000],&rom[addr],0x80000);
					}
				}

			}
			break;


		case 4/2:   // ?
			break;
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
                    fedc ba98 7654 3210     -
                    ---- ---- ---4 ----     Tilemap color mode switch (used in blandia and the other games using 6bpp graphics)
                    ---- ---- ---- 3---     Tilemap Select (There Are 2 Tilemaps Per Layer)
                    ---- ---- ---- -21-     0 (1 only in eightfrc, when flip is on!)
                    ---- ---- ---- ---0     ?

***************************************************************************/

inline void seta_state::twineagl_tile_info( tile_data &tileinfo, int tile_index, int offset )
{
	UINT16 *vram = m_vram_0 + offset;
	UINT16 code =   vram[ tile_index ];
	UINT16 attr =   vram[ tile_index + 0x800 ];
	if ((code & 0x3e00) == 0x3e00)
		code = (code & 0xc07f) | ((m_twineagl_tilebank[(code & 0x0180) >> 7] >> 1) << 7);
	SET_TILE_INFO_MEMBER(1, (code & 0x3fff), attr & 0x1f, TILE_FLIPXY((code & 0xc000) >> 14) );
}

TILE_GET_INFO_MEMBER(seta_state::twineagl_get_tile_info_0){ twineagl_tile_info(tileinfo, tile_index, 0x0000 ); }
TILE_GET_INFO_MEMBER(seta_state::twineagl_get_tile_info_1){ twineagl_tile_info(tileinfo, tile_index, 0x1000 ); }


inline void seta_state::get_tile_info( tile_data &tileinfo, int tile_index, int layer, int offset )
{
	int gfx = 1 + layer;
	UINT16 *vram = (layer == 0) ? m_vram_0 + offset : m_vram_2 + offset;
	UINT16 *vctrl = (layer == 0) ? m_vctrl_0 : m_vctrl_2;
	UINT16 code =   vram[ tile_index ];
	UINT16 attr =   vram[ tile_index + 0x800 ];

	if(m_gfxdecode->gfx(gfx + ((vctrl[ 4/2 ] & 0x10) >> m_color_mode_shift)) != NULL)
	{
		gfx += (vctrl[ 4/2 ] & 0x10) >> m_color_mode_shift;
	}
	else
	{
		popmessage("Missing Color Mode = 1 for Layer = %d. Contact MAMETesters.",layer);
	}

	SET_TILE_INFO_MEMBER(gfx, m_tiles_offset + (code & 0x3fff), attr & 0x1f, TILE_FLIPXY((code & 0xc000) >> 14) );
}

TILE_GET_INFO_MEMBER(seta_state::get_tile_info_0){ get_tile_info(tileinfo, tile_index, 0, 0x0000 ); }
TILE_GET_INFO_MEMBER(seta_state::get_tile_info_1){ get_tile_info(tileinfo, tile_index, 0, 0x1000 ); }
TILE_GET_INFO_MEMBER(seta_state::get_tile_info_2){ get_tile_info(tileinfo, tile_index, 1, 0x0000 ); }
TILE_GET_INFO_MEMBER(seta_state::get_tile_info_3){ get_tile_info(tileinfo, tile_index, 1, 0x1000 ); }


WRITE16_MEMBER(seta_state::seta_vram_0_w)
{
	COMBINE_DATA(&m_vram_0[offset]);
	if (offset & 0x1000)
		m_tilemap_1->mark_tile_dirty(offset & 0x7ff);
	else
		m_tilemap_0->mark_tile_dirty(offset & 0x7ff);
}

WRITE16_MEMBER(seta_state::seta_vram_2_w)
{
	COMBINE_DATA(&m_vram_2[offset]);
	if (offset & 0x1000)
		m_tilemap_3->mark_tile_dirty(offset & 0x7ff);
	else
		m_tilemap_2->mark_tile_dirty(offset & 0x7ff);
}

WRITE16_MEMBER(seta_state::twineagl_tilebank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		if (m_twineagl_tilebank[offset] != data)
		{
			m_twineagl_tilebank[offset] = data;
			machine().tilemap().mark_all_dirty();
		}
	}
}



/* 2 layers */
VIDEO_START_MEMBER(seta_state,seta_2_layers)
{
	VIDEO_START_CALL_MEMBER( seta_no_layers );

	/* Each layer consists of 2 tilemaps: only one can be displayed
	   at any given time */

	/* layer 0 */
	m_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seta_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS,
									16,16, 64,32 );

	m_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seta_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS,
									16,16, 64,32 );


	/* layer 1 */
	m_tilemap_2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seta_state::get_tile_info_2),this), TILEMAP_SCAN_ROWS,
									16,16, 64,32 );

	m_tilemap_3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seta_state::get_tile_info_3),this), TILEMAP_SCAN_ROWS,
									16,16, 64,32 );

	m_tilemaps_flip = 0;
	m_color_mode_shift = 3;

	m_tilemap_0->set_transparent_pen(0);
	m_tilemap_1->set_transparent_pen(0);
	m_tilemap_2->set_transparent_pen(0);
	m_tilemap_3->set_transparent_pen(0);
}

VIDEO_START_MEMBER(seta_state,oisipuzl_2_layers)
{
	VIDEO_START_CALL_MEMBER(seta_2_layers);
	m_tilemaps_flip = 1;

	// position kludges
	m_seta001->set_fg_yoffsets( -0x12, 0x0e );
}


/* 1 layer */
VIDEO_START_MEMBER(seta_state,seta_1_layer)
{
	VIDEO_START_CALL_MEMBER( seta_no_layers );

	/* Each layer consists of 2 tilemaps: only one can be displayed
	   at any given time */

	/* layer 0 */
	m_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seta_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS,
									16,16, 64,32 );

	m_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seta_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS,
									16,16, 64,32 );

	m_color_mode_shift = 4;

	m_tilemap_0->set_transparent_pen(0);
	m_tilemap_1->set_transparent_pen(0);
}

VIDEO_START_MEMBER(seta_state,setaroul_1_layer)
{
	VIDEO_START_CALL_MEMBER(seta_1_layer);

	// position kludges
	m_seta001->set_fg_yoffsets( -0x12, 0x0e );
	m_seta001->set_bg_yoffsets( 0x1, -0x1 );
}

VIDEO_START_MEMBER(seta_state,twineagl_1_layer)
{
	VIDEO_START_CALL_MEMBER( seta_no_layers );

	/* Each layer consists of 2 tilemaps: only one can be displayed
	   at any given time */

	/* layer 0 */
	m_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seta_state::twineagl_get_tile_info_0),this), TILEMAP_SCAN_ROWS,
									16,16, 64,32 );

	m_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seta_state::twineagl_get_tile_info_1),this), TILEMAP_SCAN_ROWS,
									16,16, 64,32 );

	m_tilemap_0->set_transparent_pen(0);
	m_tilemap_1->set_transparent_pen(0);
}

SETA001_SPRITE_GFXBANK_CB_MEMBER(seta_state::setac_gfxbank_callback)
{
	int bank    =   (color & 0x06) >> 1;
	code = (code & 0x3fff) + (bank * 0x4000);

	return code;
}

/* NO layers, only sprites */
VIDEO_START_MEMBER(seta_state,seta_no_layers)
{
	m_tilemap_0 = 0;
	m_tilemap_1 = 0;
	m_tilemap_2 = 0;
	m_tilemap_3 = 0;

	m_tilemaps_flip = 0;

	m_global_offsets = game_offsets;
	while (m_global_offsets->gamename && strcmp(machine().system().name, m_global_offsets->gamename))
		m_global_offsets++;
	m_samples_bank = -1;    // set the samples bank to an out of range value at start-up

	// position kludges
	m_seta001->set_fg_xoffsets(m_global_offsets->sprite_offs[1], m_global_offsets->sprite_offs[0]);
	m_seta001->set_fg_yoffsets( -0x12, 0x0e );
	m_seta001->set_bg_yoffsets( 0x1, -0x1 );
}



/***************************************************************************


                            Palette Init Functions


***************************************************************************/


/* 2 layers, 6 bit deep.

   The game can select to repeat every 16 colors to fill the 64 colors for the 6bpp gfx
   or to use the first 64 colors of the palette regardless of the color code!
*/
PALETTE_INIT_MEMBER(seta_state,blandia)
{
	int color, pen;

	for (color = 0; color < 0x20; color++)
	{
		for (pen = 0; pen < 0x40; pen++)
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
	for(int i = 0; i < 0x2200; i++)
	{
		palette.set_pen_indirect(0x2200 + i, 0x600 + (i & 0x1ff));
	}
}



/* layers have 6 bits per pixel, but the color code has a 16 colors granularity,
   even if the low 2 bits are ignored (so there are only 4 different palettes) */
PALETTE_INIT_MEMBER(seta_state,gundhara)
{
	int color, pen;

	for (color = 0; color < 0x20; color++)
		for (pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));

			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
}



/* layers have 6 bits per pixel, but the color code has a 16 colors granularity */
PALETTE_INIT_MEMBER(seta_state,jjsquawk)
{
	int color, pen;

	for (color = 0; color < 0x20; color++)
		for (pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x400 + (((color << 4) + pen) & 0x1ff)); // used by madshark
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x400 + (((color << 4) + pen) & 0x1ff));

			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x200 + (((color << 4) + pen) & 0x1ff)); // used by madshark
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x200 + (((color << 4) + pen) & 0x1ff));
		}
}


/* layer 0 is 6 bit per pixel, but the color code has a 16 colors granularity */
PALETTE_INIT_MEMBER(seta_state,zingzip)
{
	int color, pen;

	for (color = 0; color < 0x20; color++)
		for (pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x400 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0xc00 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
}

// color prom
PALETTE_INIT_MEMBER(seta_state,inttoote)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int x;
	for (x = 0; x < 0x200 ; x++)
	{
		int data = (color_prom[x*2] <<8) | color_prom[x*2+1];
		palette.set_pen_color(x, pal5bit(data >> 10),pal5bit(data >> 5),pal5bit(data >> 0));
	}
}

PALETTE_INIT_MEMBER(seta_state,setaroul)
{
	m_gfxdecode->gfx(0)->set_granularity(16);
	m_gfxdecode->gfx(1)->set_granularity(16);

	PALETTE_INIT_NAME(inttoote)(palette);
}

PALETTE_INIT_MEMBER(seta_state,usclssic)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int color, pen;
	int x;

	/* DECODE PROM */
	for (x = 0; x < 0x200 ; x++)
	{
		UINT16 data = (color_prom[x*2] <<8) | color_prom[x*2+1];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (x >= 0x100)
			palette.set_indirect_color(x + 0x000, color);
		else
			palette.set_indirect_color(x + 0x300, color);
	}

	for (color = 0; color < 0x20; color++)
		for (pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x200 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0xa00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
}


void seta_state::set_pens()
{
	offs_t i;

	for (i = 0; i < m_paletteram.bytes() / 2; i++)
	{
		UINT16 data = m_paletteram[i];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (m_palette->indirect_entries() != 0)
			m_palette->set_indirect_color(i, color);
		else
			m_palette->set_pen_color(i, color);
	}

	if(m_paletteram2 != NULL)
	{
		for (i = 0; i < m_paletteram2.bytes() / 2; i++)
		{
			UINT16 data = m_paletteram2[i];

			rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

			if (m_palette->indirect_entries() != 0)
				m_palette->set_indirect_color(i + m_paletteram.bytes() / 2, color);
			else
				m_palette->set_pen_color(i + m_paletteram.bytes() / 2, color);
		}
	}
}


void seta_state::usclssic_set_pens()
{
	offs_t i;

	for (i = 0; i < 0x200; i++)
	{
		UINT16 data = m_paletteram[i];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (i >= 0x100)
			m_palette->set_indirect_color(i - 0x100, color);
		else
			m_palette->set_indirect_color(i + 0x200, color);
	}
}




void seta_state::draw_tilemap_palette_effect(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tilemap, int scrollx, int scrolly, int gfxnum, int flipscreen)
{
	int y;
	gfx_element *gfx_tilemap = m_gfxdecode->gfx(gfxnum);
	const bitmap_ind16 &src_bitmap = tilemap->pixmap();
	int width_mask, height_mask;
	int opaque_mask = gfx_tilemap->granularity() - 1;
	int pixel_effect_mask = gfx_tilemap->colorbase() + (gfx_tilemap->colors() - 1) * gfx_tilemap->granularity();
	int p;

	width_mask = src_bitmap.width() - 1;
	height_mask = src_bitmap.height() - 1;

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dest = &bitmap.pix16(y);

		int x;
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if(!flipscreen)
			{
				p = src_bitmap.pix16((y + scrolly) & height_mask, (x + scrollx) & width_mask);
			}
			else
			{
				p = src_bitmap.pix16((y - scrolly - 256) & height_mask, (x - scrollx - 512) & width_mask);
			}

			// draw not transparent pixels
			if(p & opaque_mask)
			{
				// pixels with the last color are not drawn and the 2nd palette is added to the current bitmap color
				if((p & pixel_effect_mask) == pixel_effect_mask)
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
UINT32 seta_state::screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	bitmap.fill(0x1f0, cliprect);

	m_seta001->draw_sprites(screen, bitmap,cliprect,0x1000, 1);
	return 0;
}


/* For games with 1 or 2 tilemaps */
void seta_state::seta_layers_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_bank_size, int sprite_setac )
{
	int layers_ctrl = -1;
	int enab_0, enab_1, x_0, x_1=0, y_0, y_1=0;

	int order   =   0;
	int flip    =   m_seta001->is_flipped();

	const rectangle &visarea = screen.visible_area();
	int vis_dimy = visarea.max_y - visarea.min_y + 1;

	// check tilemaps color modes

	if(m_current_tilemap_mode[0] != (m_vctrl_0[ 4/2 ] & 0x10))
	{
		m_current_tilemap_mode[0] = m_vctrl_0[ 4/2 ] & 0x10;
		m_tilemap_0->mark_all_dirty();
		m_tilemap_1->mark_all_dirty();
	}

	if(m_tilemap_2 != NULL && m_tilemap_3 != NULL)
	{
		if(m_current_tilemap_mode[1] != (m_vctrl_2[ 4/2 ] & 0x10))
		{
			m_current_tilemap_mode[1] = m_vctrl_2[ 4/2 ] & 0x10;
			m_tilemap_2->mark_all_dirty();
			m_tilemap_3->mark_all_dirty();
		}
	}

	flip ^= m_tilemaps_flip;

	machine().tilemap().set_flip_all(flip ? (TILEMAP_FLIPX|TILEMAP_FLIPY) : 0 );

	x_0     =   m_vctrl_0[ 0/2 ];
	y_0     =   m_vctrl_0[ 2/2 ];
	enab_0  =   m_vctrl_0[ 4/2 ];

	/* Only one tilemap per layer is enabled! */
	m_tilemap_0->enable((!(enab_0 & 0x0008)) /*&& (enab_0 & 0x0001)*/ );
	m_tilemap_1->enable(( (enab_0 & 0x0008)) /*&& (enab_0 & 0x0001)*/ );

	/* the hardware wants different scroll values when flipped */

	/*  bg x scroll      flip
	    metafox     0000 025d = 0, $400-$1a3 = $400 - $190 - $13
	    eightfrc    ffe8 0272
	                fff0 0260 = -$10, $400-$190 -$10
	                ffe8 0272 = -$18, $400-$190 -$18 + $1a      */

	x_0 += 0x10 - m_global_offsets->tilemap_offs[flip ? 1 : 0];
	y_0 -= (256 - vis_dimy)/2;
	if (flip)
	{
		x_0 = -x_0 - 512;
		y_0 = y_0 - vis_dimy;
	}

	m_tilemap_0->set_scrollx(0, x_0);
	m_tilemap_1->set_scrollx(0, x_0);
	m_tilemap_0->set_scrolly(0, y_0);
	m_tilemap_1->set_scrolly(0, y_0);

	if (m_tilemap_2)
	{
		x_1     =   m_vctrl_2[ 0/2 ];
		y_1     =   m_vctrl_2[ 2/2 ];
		enab_1  =   m_vctrl_2[ 4/2 ];

		m_tilemap_2->enable((!(enab_1 & 0x0008)) /*&& (enab_1 & 0x0001)*/ );
		m_tilemap_3->enable(( (enab_1 & 0x0008)) /*&& (enab_1 & 0x0001)*/ );

		x_1 += 0x10 - m_global_offsets->tilemap_offs[flip ? 1 : 0];
		y_1 -= (256 - vis_dimy)/2;
		if (flip)
		{
			x_1 = -x_1 - 512;
			y_1 = y_1 - vis_dimy;
		}

		m_tilemap_2->set_scrollx(0, x_1);
		m_tilemap_3->set_scrollx(0, x_1);
		m_tilemap_2->set_scrolly(0, y_1);
		m_tilemap_3->set_scrolly(0, y_1);

		order   =   m_vregs[ 2/2 ];
	}


#ifdef MAME_DEBUG
if (screen.machine().input().code_pressed(KEYCODE_Z))
{   int msk = 0;
	if (screen.machine().input().code_pressed(KEYCODE_Q))   msk |= 1;
	if (screen.machine().input().code_pressed(KEYCODE_W))   msk |= 2;
	if (screen.machine().input().code_pressed(KEYCODE_A))   msk |= 8;
	if (msk != 0) layers_ctrl &= msk;

	if (m_tilemap_2)
		popmessage("VR:%04X-%04X-%04X L0:%04X L1:%04X",
			m_vregs[0], m_vregs[1], m_vregs[2], m_vctrl_0[4/2], m_vctrl_2[4/2]);
	else if (m_tilemap_0)    popmessage("L0:%04X", m_vctrl_0[4/2]);
}
#endif

	bitmap.fill(0, cliprect);

	if (order & 1)  // swap the layers?
	{
		if (m_tilemap_2)
		{
			if (layers_ctrl & 2)    m_tilemap_2->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 2)    m_tilemap_3->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		}

		if (order & 2)  // layer-sprite priority?
		{
			if (layers_ctrl & 8)        m_seta001->draw_sprites(screen, bitmap,cliprect,sprite_bank_size, sprite_setac);

			if(order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1)    m_tilemap_0->draw(screen, bitmap, cliprect, 0, 0);
			if (layers_ctrl & 1)    m_tilemap_1->draw(screen, bitmap, cliprect, 0, 0);
		}
		else
		{
			if(order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1)    m_tilemap_0->draw(screen, bitmap, cliprect, 0, 0);
			if (layers_ctrl & 1)    m_tilemap_1->draw(screen, bitmap, cliprect, 0, 0);

			if (layers_ctrl & 8)        m_seta001->draw_sprites(screen, bitmap,cliprect,sprite_bank_size, sprite_setac);
		}
	}
	else
	{
		if (layers_ctrl & 1)    m_tilemap_0->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		if (layers_ctrl & 1)    m_tilemap_1->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

		if (order & 2)  // layer-sprite priority?
		{
			if (layers_ctrl & 8)        m_seta001->draw_sprites(screen, bitmap,cliprect,sprite_bank_size, sprite_setac);

			if((order & 4) && m_paletteram2 != NULL)
			{
				if(m_tilemap_2->enabled())
				{
					draw_tilemap_palette_effect(bitmap, cliprect, m_tilemap_2, x_1, y_1, 2 + ((m_vctrl_2[ 4/2 ] & 0x10) >> m_color_mode_shift), flip);
				}
				else
				{
					draw_tilemap_palette_effect(bitmap, cliprect, m_tilemap_3, x_1, y_1, 2 + ((m_vctrl_2[ 4/2 ] & 0x10) >> m_color_mode_shift), flip);
				}
			}
			else
			{
				if(order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (m_tilemap_2)
				{
					if (layers_ctrl & 2)    m_tilemap_2->draw(screen, bitmap, cliprect, 0, 0);
					if (layers_ctrl & 2)    m_tilemap_3->draw(screen, bitmap, cliprect, 0, 0);
				}
			}
		}
		else
		{
			if((order & 4) && m_paletteram2 != NULL)
			{
				if(m_tilemap_2->enabled())
				{
					draw_tilemap_palette_effect(bitmap, cliprect, m_tilemap_2, x_1, y_1, 2 + ((m_vctrl_2[ 4/2 ] & 0x10) >> m_color_mode_shift), flip);
				}
				else
				{
					draw_tilemap_palette_effect(bitmap, cliprect, m_tilemap_3, x_1, y_1, 2 + ((m_vctrl_2[ 4/2 ] & 0x10) >> m_color_mode_shift), flip);
				}
			}
			else
			{
				if(order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (m_tilemap_2)
				{
					if (layers_ctrl & 2)    m_tilemap_2->draw(screen, bitmap, cliprect, 0, 0);
					if (layers_ctrl & 2)    m_tilemap_3->draw(screen, bitmap, cliprect, 0, 0);
				}
			}

			if (layers_ctrl & 8) m_seta001->draw_sprites(screen,bitmap,cliprect,sprite_bank_size, sprite_setac);
		}
	}

}

UINT32 seta_state::screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	seta_layers_update(screen, bitmap, cliprect, 0x1000, 1 );
	return 0;
}


UINT32 seta_state::screen_update_setaroul(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x0, cliprect);

	seta_layers_update(screen, bitmap, cliprect, 0x800, 1 );

	return 0;
}

void seta_state::screen_eof_setaroul(screen_device &screen, bool state)
{
	// rising edge
	if (state)
		m_seta001->tnzs_eof();
}



UINT32 seta_state::screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	return screen_update_seta_layers(screen, bitmap, cliprect);
}


UINT32 seta_state::screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	usclssic_set_pens();
	return screen_update_seta_layers(screen, bitmap, cliprect);
}


UINT32 seta_state::screen_update_inttoote(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* no palette to set */
	return screen_update_seta_layers(screen, bitmap, cliprect);
}
