#ifndef MAME_SINCLAIR_SPECNEXT_ULA_H
#define MAME_SINCLAIR_SPECNEXT_ULA_H

#pragma once

#include "machine/ram.h"

class specnext_ula_device : public device_t, public device_gfx_interface
{

public:
	specnext_ula_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

    void set_raster_offset(std::pair<u16, u16> offset) { m_offset = { offset.first, offset.second }; }
    void set_global_transparent(u16 global_transparent) { m_global_transparent = global_transparent; }

    void ulanext_en_w(bool ulanext_en) { m_ports.ulanext_en = ulanext_en; }
    void ulanext_format_w(u8 ulanext_format) { m_ports.ulanext_format = ulanext_format; }
    void ulap_en_w(bool ulap_en) { m_ports.ulap_en = ulap_en; }

    void ula_clip_x1_w(u8 ula_clip_x1) { m_ports.ula_clip_x1 = ula_clip_x1; }
    void ula_clip_x2_w(u8 ula_clip_x2) { m_ports.ula_clip_x2 = ula_clip_x2; }
    void ula_clip_y1_w(u8 ula_clip_y1) { m_ports.ula_clip_y1 = ula_clip_y1; }
    void ula_clip_y2_w(u8 ula_clip_y2) { m_ports.ula_clip_y2 = ula_clip_y2; }
    void ula_scroll_x_w(u8 ula_scroll_x) { m_ports.ula_scroll_x = ula_scroll_x; }
    void ula_scroll_y_w(u8 ula_scroll_y) { m_ports.ula_scroll_y = ula_scroll_y; }
    void ula_fine_scroll_x_w (bool ula_fine_scroll_x) { m_ports.ula_fine_scroll_x = ula_fine_scroll_x; }

    void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flash);

protected:
	virtual void device_start() override;
	virtual void device_reset() override {};

    required_device<ram_device> m_ram;

private:
    std::pair<u16, u16> m_offset = { 0, 0 };
    u16 m_global_transparent;

    struct ports
    {
        bool ulanext_en;
        u8 ulanext_format;
        bool ulap_en;

        u8 ula_clip_x1;
        u8 ula_clip_x2;
        u8 ula_clip_y1;
        u8 ula_clip_y2;
        u8 ula_scroll_x;
        u8 ula_scroll_y;
        bool ula_fine_scroll_x;
        bool flash;
    } m_ports;
};

DECLARE_DEVICE_TYPE(SPECNEXT_ULA, specnext_ula_device)
#endif // MAME_SINCLAIR_SPECNEXT_ULA_H
