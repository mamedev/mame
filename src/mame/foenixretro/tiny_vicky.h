// license:BSD-3-Clause
// copyright-holders:Daniel Tremblay
#ifndef MAME_FOENIXRETRO_TINY_VICKY_H
#define MAME_FOENIXRETRO_TINY_VICKY_H

#define MAME_F256_CHAR_HEIGHT     8
#define MAME_F256_CHAR_WIDTH      8

//#include <atomic>

class tiny_vicky_video_device : public device_t
{
public:
    enum class model : uint8_t
    {
        LEGACY,
        CORE2X
    };

    tiny_vicky_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    void set_model(model value) { m_model = value; }

    // screen update
    uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
    void stop()
    {
        m_running = false;
        m_videoram_ptr = nullptr;
        m_iopage0_ptr = nullptr;
        m_iopage1_ptr = nullptr;
        m_iopage2_ptr = nullptr;
        m_iopage3_ptr = nullptr;
    }
    void set_videoram(uint8_t *videoram, uint32_t videoram_size, uint8_t *iopage0, uint8_t *iopage1, uint8_t *iopage2, uint8_t *iopage3)
    {
        m_videoram_ptr = videoram;
        m_videoram_size = videoram_size;
        m_iopage0_ptr = iopage0;
        m_iopage1_ptr = iopage1;
        m_iopage2_ptr = iopage2;
        m_iopage3_ptr = iopage3;
    }
    uint8_t memtext_lut_r(offs_t offset) const { return m_memtext_lut[offset & 0x1fff]; }
    void memtext_lut_w(offs_t offset, uint8_t data) { m_memtext_lut[offset & 0x1fff] = data; }
    uint8_t memtext_font_r(offs_t offset) const { return m_memtext_font[offset & 0x1fff]; }
    void memtext_font_w(offs_t offset, uint8_t data) { m_memtext_font[offset & 0x1fff] = data; }
    uint8_t sprite_r(offs_t offset, bool bank) const;
    void sprite_w(offs_t offset, uint8_t data, bool bank);
    uint8_t linedraw_r(offs_t offset) const;
    void linedraw_w(offs_t offset, uint8_t data);
    void start()
    {
        m_running = true;
    };
    auto sof_irq_handler()
    {
        return m_sof_irq_handler.bind();
    };
    auto sol_irq_handler()
    {
        return m_sol_irq_handler.bind();
    };
protected:
    tiny_vicky_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
    // device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
    // virtual void device_add_mconfig(machine_config &config) override;
private:
    // raster
    rgb_t get_text_lut(uint8_t color_index, bool fg, bool gamma);
    rgb_t get_memtext_lut(uint8_t color_index, bool fg, bool extended, bool gamma);
    rgb_t get_lut_value(uint8_t lut_index, uint8_t pix_val, bool gamma);
    void draw_text(uint32_t *row, uint8_t mcr, bool enable_gamma, uint8_t brd_x, uint8_t brd_y, uint16_t line, uint16_t x_res, uint16_t y_res, u64 frame_number);
    void draw_memtext(uint32_t *row, uint8_t mcr, bool enable_gamma, uint8_t brd_x, uint8_t brd_y, uint16_t line, uint16_t x_res, uint16_t y_res, u64 frame_number);
    void draw_bitmap(uint32_t *row, bool enable_gamma, uint8_t layer, bool bkgrnd, rgb_t bgndColor, uint8_t borderXSize, uint8_t borderYSize, uint16_t line, uint16_t width);
    void draw_sprites(uint32_t *row, bool enable_gamma, uint8_t layer, bool bkgrnd, uint8_t borderXSize, uint8_t borderYSize, uint16_t line, uint16_t width, uint16_t height);
    void draw_tiles(uint32_t *row, bool enable_gamma, uint8_t layer, bool bkgrnd, uint8_t borderXSize, uint16_t line, uint16_t width);
    void draw_mouse(uint32_t *row, bool enable_gamma, uint16_t line, uint16_t width, uint16_t height);
    void execute_linedraw();

    model m_model = model::LEGACY;
    bool m_running = false;
    devcb_write_line m_sof_irq_handler;
    devcb_write_line m_sol_irq_handler;
    uint8_t *m_videoram_ptr = nullptr; // Pointer to video RAM
    uint32_t m_videoram_size = 0;
    uint8_t *m_iopage0_ptr = nullptr;  // Pointer to IO Page 0
    uint8_t *m_iopage1_ptr = nullptr;  // Pointer to IO Page 1
    uint8_t *m_iopage2_ptr = nullptr;  // Pointer to IO Page 2
    uint8_t *m_iopage3_ptr = nullptr;  // Pointer to IO Page 3
    std::array<uint8_t, 0x2000> m_memtext_lut{};
    std::array<uint8_t, 0x2000> m_memtext_font{};
    std::array<uint8_t, 128 * 8> m_sprite_ram{};
    std::array<uint8_t, 8> m_linedraw_reg{};
    bool m_linedraw_complete = false;

    // cursor handling routines
    bool m_cursor_visible = false;
    bool m_enable_cursor_flash = true;
    uint8_t m_cursor_flash_rate = 60;


};

DECLARE_DEVICE_TYPE(TINY_VICKY, tiny_vicky_video_device)
#endif  // MAME_FOENIXRETRO_TINY_VICKY_H
