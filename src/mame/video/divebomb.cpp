// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Bennett
/*************************************************************************

    Kyuukoukabakugekitai - Dive Bomber Squad

*************************************************************************/

#include "emu.h"
#include "includes/divebomb.h"
#include "video/resnet.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(divebomb_state::get_fg_tile_info)
{
    UINT32 code = m_fgram[tile_index + 0x000];
    UINT32 attr = m_fgram[tile_index + 0x400];
    UINT32 colour = attr >> 4;

    code |= (attr & 0x3) << 8;

    SET_TILE_INFO_MEMBER(0, code, colour, 0);
}



/*************************************
 *
 *  K051316 callbacks
 *
 *************************************/

K051316_CB_MEMBER(divebomb_state::zoom_callback_1)
{
    *code |= (*color & 0x03) << 8;
    *color = 0 + ((roz_pal >> 4) & 3);
}


K051316_CB_MEMBER(divebomb_state::zoom_callback_2)
{
    *code |= (*color & 0x03) << 8;
    *color = 4 + (roz_pal & 3);
}



/*************************************
 *
 *  Video hardware handlers
 *
 *************************************/

WRITE8_MEMBER(divebomb_state::fgram_w)
{
    m_fgram[offset] = data;
    m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}


WRITE8_MEMBER(divebomb_state::rozcpu_wrap1_enable_w)
{
    roz1_wrap = !(data & 1);
}


WRITE8_MEMBER(divebomb_state::rozcpu_enable1_w)
{
    roz1_enable = !(data & 1);
}


WRITE8_MEMBER(divebomb_state::rozcpu_enable2_w)
{
    roz2_enable = !(data & 1);
}


WRITE8_MEMBER(divebomb_state::rozcpu_wrap2_enable_w)
{
    roz2_wrap = !(data & 1);
}


WRITE8_MEMBER(divebomb_state::rozcpu_pal_w)
{
    //.... ..xx  K051316 1 palette select
    //..xx ....  K051316 2 palette select

    roz_pal = data;
    
    m_k051316_2->mark_tmap_dirty();
    m_k051316_1->mark_tmap_dirty();
    
    if (data & 0xcc)
        logerror("rozcpu_port50_w %02x\n", data);
}



/*************************************
 *
 *  Video hardware init
 *
 *************************************/

void divebomb_state::decode_proms(const UINT8 * rgn, int size, int index, bool inv)
{
    static const int resistances[4] = { 2000, 1000, 470, 220 };

    double rweights[4], gweights[4], bweights[4];
    
    /* compute the color output resistor weights */
    compute_resistor_weights(0, 255, -1.0,
                             4, resistances, rweights, 0, 0,
                             4, resistances, gweights, 0, 0,
                             4, resistances, bweights, 0, 0);
    
    /* create a lookup table for the palette */
    for (UINT32 i = 0; i < size; ++i)
    {
        UINT32 rdata = rgn[i + size*2] & 0x0f;
        UINT32 r = combine_4_weights(rweights, BIT(rdata, 0), BIT(rdata, 1), BIT(rdata, 2), BIT(rdata, 3));
        
        UINT32 gdata = rgn[i + size] & 0x0f;
        UINT32 g = combine_4_weights(gweights, BIT(gdata, 0), BIT(gdata, 1), BIT(gdata, 2), BIT(gdata, 3));
        
        UINT32 bdata = rgn[i] & 0x0f;
        UINT32 b = combine_4_weights(bweights, BIT(bdata, 0), BIT(bdata, 1), BIT(bdata, 2), BIT(bdata, 3));
        
        if (!inv)
            m_palette->set_pen_color(index + i, rgb_t(r, g, b));
        else
            m_palette->set_pen_color(index + (i ^ 0xff), rgb_t(r, g, b));
    }
}


PALETTE_INIT_MEMBER(divebomb_state, divebomb)
{
    decode_proms(memregion("spr_proms")->base(), 0x100, 0x400 + 0x400 + 0x400, false);
    decode_proms(memregion("fg_proms")->base(), 0x400, 0x400 + 0x400, false);
    decode_proms(memregion("k051316_1_pr")->base(), 0x400, 0, true);
    decode_proms(memregion("k051316_2_pr")->base(), 0x400, 0x400, true);
}


VIDEO_START_MEMBER(divebomb_state,divebomb)
{
    m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(divebomb_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
    m_fg_tilemap->set_transparent_pen(0);
    m_fg_tilemap->set_scrolly(0, 16);
}



/*************************************
 *
 *  Main update
 *
 *************************************/

void divebomb_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
    const UINT8 *spriteram = m_spriteram;
    
    for (UINT32 i = 0; i < m_spriteram.bytes(); i += 4)
    {
        UINT32 sy = spriteram[i + 3];
        UINT32 sx = spriteram[i + 0];
        UINT32 code = spriteram[i + 2];
        UINT32 attr = spriteram[i + 1];
        
        code += (attr & 0x0f) << 8;
        
        UINT32 colour = attr >> 4;

        m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, colour, 0, 0, sx, sy, 0);
        m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, colour, 0, 0, sx, sy-256, 0);
    }
}


UINT32 divebomb_state::screen_update_divebomb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
    m_k051316_1->wraparound_enable(roz1_wrap);
    m_k051316_2->wraparound_enable(roz2_wrap);

    bitmap.fill(m_palette->black_pen(), cliprect);

    if (roz2_enable)
        m_k051316_2->zoom_draw(screen, bitmap, cliprect, 0, 0);
    
    if (roz1_enable)
        m_k051316_1->zoom_draw(screen, bitmap, cliprect, 0, 0);

    draw_sprites(bitmap, cliprect);

    m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

    return 0;
}
