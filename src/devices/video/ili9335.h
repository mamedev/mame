// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/***************************************************************************

        Ilitek ILI9335 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_ILI9335_H
#define MAME_VIDEO_ILI9335_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ili9335_device

class ili9335_device : public device_t
{
public:
	// construction/destruction
	ili9335_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device interface
	void control_write(uint8_t data);
	uint8_t control_read();
	void data_write(uint8_t data);
	uint8_t data_read();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
    uint16_t m_lcdregs[256];
    uint8_t m_gram[(320 * 240 * 18) / 8];

    enum : uint8_t
    {
        REG_ID                 = 0x00,
        REG_DRVOUTCTRL         = 0x01,
        REG_LCDDRVCTRL         = 0x02,
        REG_ENTRYMODE          = 0x03,
        REG_DATAFORMAT         = 0x05,

        REG_DISPCTRL1          = 0x07,
        REG_DISPCTRL2          = 0x08,
        REG_DISPCTRL3          = 0x09,
        REG_DISPCTRL4          = 0x0A,

        REG_RGBDISPIC1         = 0x0C,
        REG_FRAMEMRK           = 0x0D,
        REG_RGBDISPIC2         = 0x0F,

        REG_PWRCTRL1           = 0x10,
        REG_PWRCTRL2           = 0x11,
        REG_PWRCTRL3           = 0x12,
        REG_PWRCTRL4           = 0x13,
        REG_PWRCTRL7           = 0x29,

        REG_CSRCOL             = 0x20,
        REG_CSRROW             = 0x21,
        REG_GRAMPTR            = 0x22,

        REG_FRCCTRL            = 0x2B,

        REG_GMCTRL1            = 0x30,
        REG_GMCTRL2            = 0x31,
        REG_GMCTRL3            = 0x32,
        REG_GMCTRL4            = 0x35,
        REG_GMCTRL5            = 0x36,
        REG_GMCTRL6            = 0x37,
        REG_GMCTRL7            = 0x38,
        REG_GMCTRL8            = 0x39,
        REG_GMCTRL9            = 0x3C,
        REG_GMCTRL10           = 0x3D,

        REG_WINH_ADDR_STA      = 0x50,
        REG_WINH_ADDR_END      = 0x51,
        REG_WINV_ADDR_STA      = 0x52,
        REG_WINV_ADDR_END      = 0x53,

        REG_GATESCANCTRL       = 0x60,
        REG_BIMGDISPCTRL       = 0x61,
        REG_VSCRCTRL           = 0x6A,

        REG_PARTIMG1DISPPOS    = 0x80,
        REG_PARTIMG1STARTLN    = 0x81,
        REG_PARTIMG1ENDLN      = 0x82,

        REG_PARTIMG2DISPPOS    = 0x83,
        REG_PARTIMG2STARTLN    = 0x84,
        REG_PARTIMG2ENDLN      = 0x85,

        REG_PANELINTCTRL1      = 0x90,
        REG_PANELINTCTRL2      = 0x92,
        REG_PANELINTCTRL4      = 0x95,
        REG_PANELINTCTRL5      = 0x97,

        REG_OTP_VCM_PC         = 0xA1,
        REG_OTP_VCM_SE         = 0xA2,
        REG_OTP_PID            = 0xA5,

        REG_DEEPSBCTRL         = 0xE6
    };

    uint8_t m_readlatch, m_writelatch;

    uint32_t m_datalatch;

    uint16_t m_currreg, m_currcol, m_currrow;

	uint16_t m_height, m_width;

    void writeReg(uint16_t val);
    bool updateCursorReg(uint16_t *cursor, uint16_t start, uint16_t end, uint8_t flag);
    void resetCursorRegs(uint16_t mode, bool resetX, bool resetY);

    uint32_t getGRAMData(uint32_t addr);
    uint32_t getPixel(uint16_t x, uint16_t y);
    uint16_t rgb666_to_rgb565(uint32_t pixel);
    uint32_t rgb666_swapBGR(uint32_t pixel);

    void writePixel_565(uint16_t pixel);
    void writePixel_666(uint32_t pixel);
    void writePixel_666_unpacked(uint32_t pixel);

    void drawInterlacedFrame(bitmap_rgb32 &bitmap);
    void drawPartialImage(bitmap_rgb32 &bitmap, uint16_t disp_pos, uint16_t start_line, uint16_t end_line);
    void plotPixel(bitmap_rgb32 &bitmap, uint32_t pixel18, uint8_t x, uint16_t y);
};

// device type definition
DECLARE_DEVICE_TYPE(ILI9335, ili9335_device)

#endif // MAME_VIDEO_ILI9335_H
