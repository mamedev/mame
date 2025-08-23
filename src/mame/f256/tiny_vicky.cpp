// license:BSD-3-Clause
// copyright-holders:Daniel Tremblay
#include "emu.h"
#include "tiny_vicky.h"

#include "machine/ram.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(TINY_VICKY, tiny_vicky_video_device, "tiny_vicky", "F256K Tiny Vicky")

tiny_vicky_video_device::tiny_vicky_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock):
    device_t(mconfig, type, tag, owner, clock)
    , m_sof_irq_handler(*this)
    , m_sol_irq_handler(*this)
{
}

tiny_vicky_video_device::tiny_vicky_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
    tiny_vicky_video_device(mconfig, TINY_VICKY, tag, owner, clock)
{
}

rgb_t tiny_vicky_video_device::get_text_lut(uint8_t color_index, bool fg, bool gamma)
{
    uint8_t red =   m_iopage0_ptr[(fg ? 0x1800 : 0x1840) + color_index * 4 + 2];
    uint8_t green = m_iopage0_ptr[(fg ? 0x1800 : 0x1840) + color_index * 4 + 1];
    uint8_t blue =  m_iopage0_ptr[(fg ? 0x1800 : 0x1840) + color_index * 4 ];
    if (gamma)
    {
        blue = m_iopage0_ptr[blue];
        green = m_iopage0_ptr[0x400 + green];
        red = m_iopage0_ptr[0x800 + red];
    }
    return rgb_t(red, green, blue);
}

rgb_t tiny_vicky_video_device::get_lut_value(uint8_t lut_index, uint8_t pix_val, bool gamma)
{
    int lutAddress = 0xd000 - 0xc000 + (lut_index * 256 + pix_val) * 4;
    if (!gamma)
    {
        return rgb_t(m_iopage1_ptr[lutAddress + 2], m_iopage1_ptr[lutAddress + 1], m_iopage1_ptr[lutAddress]);
    }
    else
    {
        return rgb_t(m_iopage0_ptr[0x800 + m_iopage1_ptr[lutAddress + 2]],
                    m_iopage0_ptr[0x400 + m_iopage1_ptr[lutAddress + 1]],
                    m_iopage0_ptr[m_iopage1_ptr[lutAddress]]);
    }
}

uint32_t tiny_vicky_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    if (m_running && m_iopage0_ptr)
    {
        uint8_t mcr = m_iopage0_ptr[0x1000];
        uint8_t mcr_h = m_iopage0_ptr[0x1001];
        m_cursor_counter++;
        if (m_cursor_counter > m_cursor_flash_rate *2)
        {
            m_cursor_counter = 0;
        }
        // if MCR=0 or MCR bit 7 is set, then video is disabled
        if (mcr != 0 && (mcr != 0x80))
        {
            // TODO: generate start of frame (SOF) interrupt
            m_sof_irq_handler(1);
            uint8_t border_reg = m_iopage0_ptr[0x1004];
            bool display_border = (border_reg & 0x1) > 0;
            bool enable_gamma = (mcr & 0x40) > 0;
            bool enable_graphics = (mcr & 0x4) > 0;
            uint16_t lines = (mcr_h & 1) == 0 ? 480 : 400;

            uint8_t border_x = 0;
            uint8_t border_y = 0;
            rgb_t border_color = rgb_t();
            if (display_border)
            {
                border_x = m_iopage0_ptr[0x1008];
                border_y = m_iopage0_ptr[0x1009];
            }
            uint32_t *topleft = (uint32_t *)bitmap.raw_pixptr(0);
            for (int y = 0; y < lines; y++)
            {
                // Check the Sart of Line registers
                uint8_t sol_reg = m_iopage0_ptr[0x1018];
                // 12-bit line
                uint16_t sol_line = m_iopage0_ptr[0x1018] + ((m_iopage0_ptr[0x101a] & 0xf) << 8);
                uint32_t *row = topleft + y * 800;
                if ((sol_reg & 1) != 0 && y == sol_line)
                {
                    // raise an interrupt
                    m_sol_irq_handler(1);
                }
                // border color can change during painting the screen
                if (display_border)
                {
                    if (!enable_gamma)
                    {
                        border_color = rgb_t(m_iopage0_ptr[0x1007], m_iopage0_ptr[0x1006], m_iopage0_ptr[0x1005]);
                    }
                    else
                    {
                        border_color = rgb_t(m_iopage0_ptr[0x800 + m_iopage0_ptr[0x1007]],
                                             m_iopage0_ptr[0x400 + m_iopage0_ptr[0x1006]],
                                             m_iopage0_ptr[m_iopage0_ptr[0x1005]]);
                    }
                }

                if (y < border_y || y >= lines - border_y)
                {
                    for (int x = 0; x < 640; x++)
                    {
                        row[x] = border_color;
                    }
                }
                else
                {
                    rgb_t background_color = rgb_t();
                    // only draw the even lines
                    if (enable_graphics && y % 2 == 0)
                    {
                        if (!enable_gamma)
                        {
                            background_color = rgb_t(m_iopage0_ptr[0x100f], m_iopage0_ptr[0x100e], m_iopage0_ptr[0x100d]);
                        }
                        else
                        {
                            background_color = rgb_t(m_iopage0_ptr[0x800 + m_iopage0_ptr[0x100f]],
                                                    m_iopage0_ptr[0x400 + m_iopage0_ptr[0x100e]],
                                                    m_iopage0_ptr[m_iopage0_ptr[0x100d]]);
                        }
                    }
                    // draw the border or background
                    if (y % 2 == 0)
                    {
                        for (int x = 0; x < 640; x++)
                        {
                            row[x] = x < border_x || x >= 640 - border_x ? border_color : background_color;
                        }
                        memcpy(row+800,row, 640*4);
                    }

                    if (enable_graphics)
                    {
                        if (y % 2 == 0)
                        {
                            // Tiny Vicky Layers for Bitmaps, Tilemaps and sprites
                            uint8_t layer_mgr0 = m_iopage0_ptr[0xd002 - 0xc000] & 0x7;
                            uint8_t layer_mgr1 = m_iopage0_ptr[0xd002 - 0xc000] >> 4;
                            uint8_t layer_mgr2 = m_iopage0_ptr[0xd003 - 0xc000] & 0x7;

                            // draw layers starting from the back
                            if ((mcr & 0x20) != 0)
                            {
                                draw_sprites(row, enable_gamma, 3, display_border, border_x, border_y, y, (uint16_t)640, lines / 2);
                            }
                            if ((mcr & 0x8) != 0 && layer_mgr2 < 3)
                            {
                                draw_bitmap(row, enable_gamma, layer_mgr2, display_border, background_color, border_x, border_y, y, (uint16_t)640);

                            }
                            if ((mcr & 0x10) != 0 && (layer_mgr2 > 3 && layer_mgr2 < 7))
                            {
                                draw_tiles(row, enable_gamma, layer_mgr2 & 3, display_border, border_x, y, (uint16_t)640);
                            }
                            if ((mcr & 0x20) != 0)
                            {
                                draw_sprites(row, enable_gamma, 2, display_border, border_x, border_y, y, (uint16_t)640, lines/2);
                            }
                            if ((mcr & 0x8) != 0 && layer_mgr1 < 3)
                            {
                                draw_bitmap(row, enable_gamma, layer_mgr1, display_border, background_color, border_x, border_y, y, (uint16_t)640);
                            }
                            if ((mcr & 0x10) != 0 && (layer_mgr1 > 3 && layer_mgr1 < 7))
                            {
                                draw_tiles(row, enable_gamma, layer_mgr1 & 3, display_border, border_x, y, (uint16_t)640);
                            }
                            if ((mcr & 0x20) != 0)
                            {
                                draw_sprites(row, enable_gamma, 1, display_border, border_x, border_y, y, (uint16_t)640, lines / 2);
                            }
                            if ((mcr & 0x8) != 0 && layer_mgr0 < 3)
                            {
                                draw_bitmap(row, enable_gamma, layer_mgr0, display_border, background_color, border_x, border_y, y, (uint16_t)640);
                            }
                            if ((mcr & 0x10) != 0 && (layer_mgr0 > 3 && layer_mgr0 < 7))
                            {
                                draw_tiles(row, enable_gamma, layer_mgr0 & 3, display_border, border_x, y, (uint16_t)640);
                            }
                            if ((mcr & 0x20) != 0)
                            {
                                draw_sprites(row, enable_gamma, 0, display_border, border_x, border_y, y, (uint16_t)640, lines/ 2);
                            }
                            // copy the odd line now
                            memcpy(row + 800, row, 640 * 4);
                        }
                    }
                    // Only display text in these cases
                    if ((mcr & 0x7) == 1 || (mcr & 0x7) == 3 || (mcr & 0x7) == 7)
                    {
                        draw_text(row, mcr, enable_gamma, border_x, border_y, y, (uint16_t)640, (uint16_t)lines);
                    }
                }
                draw_mouse(row, enable_gamma, y, (uint16_t)640, (uint16_t)lines);
            }
        }
    }

    return 0;
}


void tiny_vicky_video_device::draw_text(uint32_t *row, uint8_t mcr, bool enable_gamma, uint8_t brd_x, uint8_t brd_y, uint16_t line, uint16_t x_res, uint16_t y_res)
{
    bool overlay = (mcr & 0x2) != 0;
    uint8_t mcrh = m_iopage0_ptr[0x1001] & 0x3f;
    bool double_x = (mcrh & 0x2) != 0;
    bool double_y = (mcrh & 0x4) != 0;
    bool use_font1 = (mcrh & 0x20) != 0;
    bool overlay_font = (mcrh & 0x10) != 0;
    int txt_line = ((double_y ? line / 2 : line) - brd_y) / MAME_F256_CHAR_HEIGHT;
    // Each character is defined by 8 bytes
    int font_line = ((double_y ? line / 2 : line) - brd_y) % MAME_F256_CHAR_HEIGHT;
    int txt_cols = double_x ? 40 : 80;

    // do cursor stuff
    int cursor_x = m_iopage0_ptr[0x1014] + (m_iopage0_ptr[0x1015] << 8);
    int cursor_y = m_iopage0_ptr[0x1016] + (m_iopage0_ptr[0x1017] << 8);

    // TODO: enabling/disabling cursor and flashing should be handled somewhere else... maybe in the top screen_update function.
    bool enable_cursor = (m_iopage0_ptr[0x1010] & 0x1) > 0;
    m_enable_cursor_flash = (m_iopage0_ptr[0x1010] & 0x8) == 0;
    // flash rate is the count of screen updates
    // 1s   = 60 ==> 00
    // 0.5s = 30 ==> 01
    // 0.25s= 15 ==> 10
    // 0.20s= 12 ==> 11
    m_cursor_flash_rate = 60;
    switch ((m_iopage0_ptr[0x1010] & 6) >> 1)
    {
        case 1:
            m_cursor_flash_rate = 30;
            break;
        case 2:
            m_cursor_flash_rate = 15;
            break;
        case 3:
            m_cursor_flash_rate = 12;
            break;
    }
    int screen_x = brd_x;
    // I'm assuming that if m_enable_cursor_flash is 0, then the cursor is always visible
    m_cursor_visible = enable_cursor && (m_enable_cursor_flash && (m_cursor_counter < m_cursor_flash_rate));
    // the loop should go to txt_cols - going to 80 causes a weird alias, but the machine works this way... so.
    for (int col = 0; col < 80; col++)
    {
        int x = col * MAME_F256_CHAR_WIDTH;
        if (x + brd_x > x_res - 1 - brd_x)
        {
            continue;
        }
        // int offset = 0;
        // if (col < (double_x ? 40 : 80))
        // {
        //     // offset is always based on 80 columns
        //     offset = 80 * txt_line + col;
        // }
        int offset = txt_cols * txt_line + col;
        // Each character will have foreground and background colors
        uint8_t character = m_iopage2_ptr[offset];
        uint8_t color = m_iopage3_ptr[offset];

        // Display the cursor - this replaces the text character
        if (cursor_x == col && cursor_y == txt_line && m_cursor_visible)
        {
            character = m_iopage0_ptr[0x1012];
        }

        uint8_t fg_color_index = (color & 0xf0) >> 4;
        uint8_t bg_color_index = (color & 0x0f);

        rgb_t fg_color = get_text_lut(fg_color_index, true, enable_gamma);
        rgb_t bg_color = get_text_lut(bg_color_index, false, enable_gamma);

        uint8_t value = m_iopage1_ptr[(use_font1 ? 0x800 : 0) + character * 8 + font_line];

        // For each bit in the font, set the foreground color - if the bit is 0 and overlay is set, skip it (keep the background)
        for (int b = 0x80; b > 0; b >>= 1)
        {
            if (double_x)
            {
                if ((value & b) != 0)
                {
                    row[screen_x] = fg_color;
                    row[screen_x + 1] = fg_color;
                }
                else if (!overlay || (overlay_font && (bg_color == 0)))
                {
                    row[screen_x] = bg_color;
                    row[screen_x + 1] = bg_color;
                }
                screen_x += 2;
            }
            else
            {
                if ((value & b) != 0)
                {
                    row[screen_x] = fg_color;
                }
                else if (!overlay || (overlay_font && (bg_color != 0)))
                {
                    row[screen_x] = bg_color;
                }
                screen_x++;
            }
        }
    }
}
void tiny_vicky_video_device::device_start()
{
}
void tiny_vicky_video_device::device_reset()
{

}
void tiny_vicky_video_device::draw_bitmap(uint32_t *row, bool enable_gamma, uint8_t layer, bool bkgrnd, rgb_t bgndColor, uint8_t borderXSize, uint8_t borderYSize, uint16_t line, uint16_t width)
{
    uint8_t reg = m_iopage0_ptr[(0xd100 - 0xc000) + layer * 8];
    // check if the bitmap is enabled
    if ((reg & 0x01) == 0)
    {
        return;
    }
    uint8_t lut_index = (reg >> 1) & 7;  // 8 possible LUTs
    constexpr int base_offset = 0xd101 - 0xc000;
    int bitmapAddress = (m_iopage0_ptr[base_offset + layer * 8] +
                         (m_iopage0_ptr[base_offset + 1 + layer * 8] << 8) +
                         (m_iopage0_ptr[base_offset + 2 + layer * 8] << 16)
                        ) & 0x3f'ffff;

    rgb_t color_val = 0;
    int offsetAddress = bitmapAddress + (line/2) * width/2;
    uint8_t pix_val = 0;

    for (int col = borderXSize; col < width - borderXSize; col += 2)
    {
        pix_val = m_videoram_ptr[offsetAddress + col/2 + 1];
        if (pix_val != 0)
        {
            color_val = get_lut_value(lut_index, pix_val, enable_gamma);
            row[col] = color_val;
            row[col + 1] = color_val;
        }
    }
}


void tiny_vicky_video_device::draw_sprites(uint32_t *row, bool enable_gamma, uint8_t layer, bool bkgrnd, uint8_t borderXSize, uint8_t borderYSize, uint16_t line, uint16_t width, uint16_t height)
{
    // There are 64 possible sprites to choose from.
    for (int s = 63; s > -1; s--)
    {
        int addr_sprite = 0xd900 - 0xc000 + s * 8;
        uint8_t reg = m_iopage0_ptr[addr_sprite];
        // if the set is not enabled, we're done.
        uint8_t sprite_layer = (reg & 0x18) >> 3;
        // if the sprite is enabled and the layer matches, then check the line
        if ((reg & 1) != 0 && layer == sprite_layer)
        {
            uint8_t sprite_size = 32;
            switch ((reg & 0x60) >> 5)
            {
                case 1:
                    sprite_size = 24;
                    break;
                case 2:
                    sprite_size = 16;
                    break;
                case 3:
                    sprite_size = 8;
                    break;
            }
            int posY = m_iopage0_ptr[addr_sprite + 6] + (m_iopage0_ptr[addr_sprite + 7] << 8) - 32;
            int actualLine = line / 2;
            if ((actualLine >= posY && actualLine < posY + sprite_size))
            {
                // TODO Fix this when Vicky II fixes the LUT issue
                uint8_t lut_index = ((reg & 6) >> 1);

                //int lut_address = 0xd000 - 0xc000 + lut_index * 0x400;
                //bool striding = (reg & 0x80) == 0x80;

                int sprite_address = (m_iopage0_ptr[addr_sprite + 1] +
                                     (m_iopage0_ptr[addr_sprite + 2] << 8) +
                                     (m_iopage0_ptr[addr_sprite + 3] << 16)) & 0x3f'ffff;
                int posX = m_iopage0_ptr[addr_sprite + 4] + (m_iopage0_ptr[addr_sprite + 5] << 8) - 32;
                posX *= 2;

                if (posX >= width || posY >= height || (posX + sprite_size) < 0 || (posY + sprite_size) < 0)
                {
                    continue;
                }
                int sprite_width = sprite_size;
                int xOffset = 0;
                // Check for sprite bleeding on the left-hand-side
                if (posX < borderXSize)
                {
                    xOffset = borderXSize - posX;
                    posX = borderXSize;
                    sprite_width = sprite_size - xOffset;
                    if (sprite_width == 0)
                    {
                        continue;
                    }
                }
                // Check for sprite bleeding on the right-hand side
                if (posX + sprite_size > width - borderXSize)
                {
                    sprite_width = width - borderXSize - posX;
                    if (sprite_width == 0)
                    {
                        continue;
                    }
                }

                rgb_t clrVal = 0;
                uint8_t pixVal = 0;

                int sline = actualLine - posY;
                int cols = sprite_size;
                if (posX + sprite_size*2 >= width - borderXSize)
                {
                    cols = width - borderXSize - posX;
                    cols /= 2;
                }
                for (int col = xOffset; col < xOffset + cols; col++)
                {
                    // Lookup the pixel in the tileset - if the value is 0, it's transparent
                    pixVal = m_videoram_ptr[sprite_address + col + sline * sprite_size];
                    if (pixVal != 0)
                    {
                        clrVal = get_lut_value(lut_index, pixVal, enable_gamma);
                        row[(col * 2) - xOffset + posX] = clrVal;
                        row[(col * 2) + 1 - xOffset + posX] = clrVal;
                    }
                }
            }
        }
    }
}

void tiny_vicky_video_device::draw_tiles(uint32_t *row, bool enable_gamma, uint8_t layer, bool bkgrnd, uint8_t borderXSize, uint16_t line, uint16_t width)
{
    // There are four possible tilemaps to choose from
    int addr_tile_addr = 0xd200 - 0xc000 + layer * 12;
    int reg = m_iopage0_ptr[addr_tile_addr];
    // if the set is not enabled, we're done.
    if ((reg & 0x01) == 00)
    {
        return;
    }
    bool smallTiles = (reg & 0x10) > 0;

    int tileSize = (smallTiles ? 8 : 16);
    int strideLine = tileSize * 16;
    uint8_t scrollMask = smallTiles ? 0xf : 0xf;  // Tiny Vicky bug: this should be 0xe

    int tilemapWidth = (m_iopage0_ptr[addr_tile_addr + 4] + (m_iopage0_ptr[addr_tile_addr + 5] << 8)) & 0x3ff;   // 10 bits
    //int tilemapHeight = VICKY.ReadWord(addrTileCtrlReg + 6) & 0x3ff;  // 10 bits
    int tilemapAddress = (m_iopage0_ptr[addr_tile_addr + 1] + (m_iopage0_ptr[addr_tile_addr + 2]  << 8) + (m_iopage0_ptr[addr_tile_addr + 3] << 16)) & 0x3f'ffff;

    // the tilemapWindowX is 10 bits and the scrollX is the lower 4 bits.  The IDE combines them.
    int tilemapWindowX = m_iopage0_ptr[addr_tile_addr + 8] + (m_iopage0_ptr[addr_tile_addr + 9] << 8);
    uint8_t scrollX = (tilemapWindowX & scrollMask) & scrollMask;
    // the tilemapWindowY is 10 bits and the scrollY is the lower 4 bits.  The IDE combines them.
    int tilemapWindowY = m_iopage0_ptr[addr_tile_addr + 10] + (m_iopage0_ptr[addr_tile_addr + 11] << 8);
    uint8_t scrollY = (tilemapWindowY & scrollMask) & scrollMask;
    if (smallTiles)
    {
        tilemapWindowX = ((tilemapWindowX & 0x3ff0)  >> 1) + scrollX + 8;
        tilemapWindowY = ((tilemapWindowY & 0x3ff0) >> 1) + scrollY;
    }
    else
    {
        tilemapWindowX = (tilemapWindowX & 0x3ff0) + scrollX;
        tilemapWindowY = (tilemapWindowY & 0x3ff0) + scrollY;
    }
    int tileXOffset = tilemapWindowX % tileSize;

    int tileRow = (line / 2 + tilemapWindowY) / tileSize;
    int tileYOffset = (line / 2 + tilemapWindowY) % tileSize;
    int maxX = width - borderXSize;

    // we always read tiles 0 to width/TILE_SIZE + 1 - this is to ensure we can display partial tiles, with X,Y offsets
    // TODO - variable length array: int tilemapItemCount = width / 2 / tileSize + 1;
    const uint8_t tilemapItemCount = 41;  // this is the maximum number of tiles in a line
    // The + 2 below is to take an FPGA bug in the F256Jr into account
    //int tlmSize = tilemapItemCount * 2 + 2;
    const uint8_t tlmSize = 84;
    uint8_t tiles[tlmSize];
    int tilesetOffsets[tilemapItemCount];

    // The + 2 below is to take an FPGA bug in the F256Jr into account
    memcpy(tiles, m_videoram_ptr + tilemapAddress + (tilemapWindowX / tileSize) * 2 + (tileRow + 0) * tilemapWidth * 2, tlmSize);

    // cache of tilesetPointers
    int tilesetPointers[8];
    int strides[8];
    for (int i = 0; i < 8; i++)
    {
        tilesetPointers[i] = (m_iopage0_ptr[0xd280 - 0xc000 + i * 4] + (m_iopage0_ptr[0xd280 - 0xc000 + i * 4 + 1] << 8) +
                             (m_iopage0_ptr[0xd280 - 0xc000 + i * 4 + 2] << 16)) & 0x3f'ffff;
        uint8_t tilesetConfig = m_iopage0_ptr[0xd280 - 0xc000 + i * 4 + 3];
        strides[i] = (tilesetConfig & 8) != 0 ? strideLine : tileSize;
    }
    for (int i = 0; i < tilemapItemCount; i++)
    {
        uint8_t tile = tiles[i * 2];
        uint8_t tilesetReg = tiles[i * 2 + 1];
        uint8_t tileset = tilesetReg & 7;

        // tileset
        int tilesetPointer = tilesetPointers[tileset];
        int strideX = strides[tileset];
        if (strideX == tileSize)
        {
            tilesetOffsets[i] = tilesetPointer + (tile % 16) * tileSize * tileSize + (tile / 16) * tileSize * tileSize * 16 + tileYOffset * tileSize;
        }
        else
        {
            tilesetOffsets[i] = tilesetPointer + ((tile / 16) * strideX * tileSize + (tile % 16) * tileSize) + tileYOffset * strideX;
        }
    }

    // alternate display style - avoids repeating the loop so often
    int startTileX = (borderXSize + tileXOffset) / tileSize;
    int endTileX = (width/2 - borderXSize + tileXOffset) / tileSize + 1;
    int startOffset = (borderXSize + tilemapWindowX) % tileSize;
    int x = borderXSize;
    //uint8_t tilepix[tileSize];
    rgb_t clr_val = 0;
    for (int t = startTileX; t < endTileX; t++)
    {
        // The (mode==0 ? 1 : 3) below is to take an FPGA but in the F256Jr into account
        uint8_t tilesetReg = tiles[t * 2 + 3];
        uint8_t lut_index = (tilesetReg & 0x38) >> 3;
        //int lutAddress = MemoryMap.GRP_LUT_BASE_ADDR - VICKY.StartAddress + lutIndex * 1024;
        //int tilesetOffsetAddress = tilesetOffsets[t];  // + startOffset
        //memcpy(tilepix, m_videoram_ptr + tilesetOffsets[t], tileSize);
        do
        {
            uint8_t pixVal = m_videoram_ptr[tilesetOffsets[t] + startOffset];  // tilepix[startOffset];
            if (pixVal > 0)
            {
                clr_val = get_lut_value(lut_index, pixVal, enable_gamma);
                row[x] = clr_val;
                row[x+1] = clr_val;
            }
            startOffset++;
            //tilesetOffsetAddress++;
            x+=2;
        } while (startOffset != tileSize && x < maxX);
        startOffset = 0;
        if (x == maxX)
        {
            break;
        }
    }
}

void tiny_vicky_video_device::draw_mouse(uint32_t *row, bool enable_gamma, uint16_t line, uint16_t width, uint16_t height)
{
    uint8_t mouse_reg = m_iopage0_ptr[0xd6e0 - 0xc000];

    bool MousePointerEnabled = (mouse_reg & 3) != 0;

    if (MousePointerEnabled)
    {
        int PosX = m_iopage0_ptr[0xd6e0 - 0xc000 + 2] + (m_iopage0_ptr[0xd6e0 - 0xc000 + 3] << 8);
        int PosY = m_iopage0_ptr[0xd6e0 - 0xc000 + 4] + (m_iopage0_ptr[0xd6e0 - 0xc000 + 5] << 8);
        if (line >= PosY && line < PosY + 16)
        {
            int ptr_addr = 0xcc00 - 0xc000;

            // Mouse pointer is a 16x16 icon
            int colsToDraw = PosX < width - 16 ? 16 : width - PosX;

            int mouse_line = line - PosY;
            for (int col = 0; col < colsToDraw; col++)
            {
                // Values are 0: transparent, 1:black, 255: white (gray scales)
                uint8_t pixel_index = m_iopage0_ptr[ptr_addr + mouse_line * 16 + col];
                rgb_t value;

                if (pixel_index != 0)
                {
                    if (!enable_gamma)
                    {
                        value = rgb_t(pixel_index, pixel_index, pixel_index);
                    }
                    else
                    {
                        value = rgb_t(m_iopage0_ptr[0x800 + pixel_index],
                                    m_iopage0_ptr[0x400 + pixel_index],
                                    m_iopage0_ptr[pixel_index]);
                    }
                    row[col + PosX] = value;
                }
            }
        }

    }
}
