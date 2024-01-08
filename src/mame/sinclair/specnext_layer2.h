#ifndef MAME_SINCLAIR_SPECNEXT_LAYER2_H
#define MAME_SINCLAIR_SPECNEXT_LAYER2_H

#pragma once

#include "machine/ram.h"

class specnext_layer2_device : public device_t, public device_gfx_interface
{

public:
	specnext_layer2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

    void set_raster_offset(std::pair<u16, u16> offset) { m_offset = { offset.first, offset.second }; }
    void set_global_transparent(u16 global_transparent) { m_global_transparent = global_transparent; }

    void layer2_en_w(bool layer2_en) { m_ports.layer2_en = layer2_en; }
    void resolution_w(u8 resolution) { m_ports.resolution = resolution; }
    void palette_offset_w(u8 palette_offset) { m_ports.palette_offset = palette_offset; }
    void layer2_active_bank_w(u8 layer2_active_bank) { m_ports.layer2_active_bank = layer2_active_bank; }

    void scroll_x_w(u16 scroll_x) { m_ports.scroll_x = scroll_x; }
    void scroll_y_w(u8 scroll_y) { m_ports.scroll_y = scroll_y; }
    void clip_x1_w(u8 clip_x1) { m_ports.clip_x1 = clip_x1; }
    void clip_x2_w(u8 clip_x2) { m_ports.clip_x2 = clip_x2; }
    void clip_y1_w(u8 clip_y1) { m_ports.clip_y1 = clip_y1; }
    void clip_y2_w(u8 clip_y2) { m_ports.clip_y2 = clip_y2; }

    void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override;
	virtual void device_reset() override {};

    required_device<ram_device> m_ram;

private:
    std::pair<u16, u16> m_offset = { 0, 0 };
    u16 m_global_transparent;

    struct ports
    {
        bool layer2_en;
        u8 resolution : 2; // 00 = 256x192, 01 = 320x256, 1X = 640x256x4
        u8 palette_offset : 4;
        u8 layer2_active_bank : 7;

        u16 scroll_x : 9;
        u8 scroll_y;
        u8 clip_x1;
        u8 clip_x2;
        u8 clip_y1;
        u8 clip_y2;
    } m_ports;
};


DECLARE_DEVICE_TYPE(SPECNEXT_LAYER2, specnext_layer2_device)
#endif // MAME_SINCLAIR_SPECNEXT_LAYER2_H
