// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/***************************************************************************

        Ilitek ILI9335 LCD controller

        TODO:
            Framerate control and front porch

***************************************************************************/

#include "emu.h"
#include "ili9335.h"

namespace {
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
}

// devices
DEFINE_DEVICE_TYPE(ILI9335, ili9335_device, "ili9335", "Ilitek ILI9335 LCD Controller")

ili9335_device::ili9335_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    device_t(mconfig, ILI9335, tag, owner, clock),

    m_lcdregs{0}, m_gram{0}, m_readlatch(0), m_writelatch(0), m_datalatch(0),
    m_currreg(0), m_currcol(0), m_currrow(0), m_height(320), m_width(240)
{
}

void ili9335_device::device_start()
{
    save_item(NAME(m_lcdregs));
    save_item(NAME(m_gram));

    save_item(NAME(m_readlatch));
    save_item(NAME(m_writelatch));

    save_item(NAME(m_currreg));
    save_item(NAME(m_currcol));
    save_item(NAME(m_currrow));

    save_item(NAME(m_datalatch));
}

void ili9335_device::device_reset()
{
    memset(m_lcdregs, 0x00, sizeof(m_lcdregs));
    memset(m_gram, 0x00, sizeof(m_gram));

    m_lcdregs[REG_ID] = 0x9335;

    m_readlatch = 0;
    m_writelatch = 0;

    m_currreg = 0;
    m_datalatch = 0;
}

void ili9335_device::control_write(uint8_t data) {
    m_currreg = (m_currreg << 8) | data;
    m_readlatch = 0;
    m_writelatch = 0;
}

uint8_t ili9335_device::control_read() {
    return 0;
}

void ili9335_device::data_write(uint8_t data) {
    m_datalatch = ((m_datalatch << 8) | data);

    if (m_currreg == REG_GRAMPTR) {
        m_writelatch++;

        if (m_lcdregs[REG_ENTRYMODE] & 0x8000 && m_writelatch >= 3) {
            // handle 18bpp mode
            m_writelatch = 0;
            if (m_lcdregs[REG_ENTRYMODE] & 0x4000) {
                writePixel_666(m_datalatch & 0x3FFFF);
            } else {
                writePixel_666_unpacked(m_datalatch);
            }
        } else if (!(m_lcdregs[REG_ENTRYMODE] & 0x8000) && m_writelatch >= 2) {
            // handle 16bpp mode
            m_writelatch = 0;
            writePixel_565((uint16_t)m_datalatch);
        }
    } else {
        if (m_writelatch)
            writeReg((uint16_t)m_datalatch);
        m_writelatch = !m_writelatch;
    }
}

uint8_t ili9335_device::data_read() {
    uint8_t data;

    if (m_currreg == REG_GRAMPTR) {
        // read a pixel. If the BGR bit is set, then swap
        // the red and blue channels regardless of whether 
        // they were already swapped when written to GRAM.

        uint32_t pixelRaw = getPixel(m_lcdregs[REG_CSRCOL], m_lcdregs[REG_CSRROW]);
        if (m_lcdregs[REG_ENTRYMODE] & 0x1000)
            pixelRaw = rgb666_swapBGR(pixelRaw);
    
        uint16_t pixel = rgb666_to_rgb565(pixelRaw);

        data = m_readlatch ? pixel & 0xFF : pixel >> 8;
    } else {
        uint16_t reg = m_lcdregs[m_currreg];
        data = m_readlatch ? reg & 0xFF : reg >> 8;
    }
    
    m_readlatch = !m_readlatch;

    return data;
}

void ili9335_device::writeReg(uint16_t val) {
    uint16_t mask = 0;

    switch (m_currreg) {
        case REG_ID:    // read only, bail out
            return;

        case REG_DRVOUTCTRL:
            mask = 0x0500;
            break;

        case REG_ENTRYMODE:
            mask = 0xD0B8;
            if (val & 0x80)
                resetCursorRegs(val, true, true);
            break;

        case REG_DATAFORMAT:
            mask = 0x0003;
            break;

        case REG_DISPCTRL1:
            mask = 0x313B;
            break;

        case REG_DISPCTRL2:
            mask = 0xFFFF;
            break;

        case REG_DISPCTRL3:
            mask = 0x073F;
            break;

        case REG_DISPCTRL4:
            mask = 0x000F;
            break;

        case REG_RGBDISPIC1:
            mask = 0x7133;
            break;

        case REG_RGBDISPIC2:
            mask = 0x001B;
            break;

        case REG_FRAMEMRK:
            mask = 0x01FF;
            break;

        case REG_PWRCTRL1:
            mask = 0x17F3;
            break;

        case REG_PWRCTRL2:
            mask = 0x0777;
            break;

        case REG_PWRCTRL3:
            mask = 0x008F;
            break;

        case REG_PWRCTRL4:
            mask = 0x1F00;
            break;

        case REG_CSRCOL:
            mask = 0x00FF;
            if (m_lcdregs[REG_ENTRYMODE] & 0x80) {
                resetCursorRegs(m_lcdregs[REG_ENTRYMODE], true, false);
                return;
            } else {
                m_lcdregs[REG_CSRROW] = m_currrow;
                m_currcol = val & mask;
            }
            break;

        case REG_CSRROW:
            mask = 0x01FF;
            if (m_lcdregs[REG_ENTRYMODE] & 0x80) {
                resetCursorRegs(m_lcdregs[REG_ENTRYMODE], false, true);
                return;
            } else {
                m_lcdregs[REG_CSRCOL] = m_currcol;
                m_currrow = val & mask;
            }
            break;

        case REG_PWRCTRL7:
            mask = 0x003F;
            break;

        case REG_FRCCTRL:
            // TODO: set framerate
            mask = 0x000F;
            break;

        case REG_GMCTRL1:
        case REG_GMCTRL2:
        case REG_GMCTRL3:
        case REG_GMCTRL4:
        case REG_GMCTRL6:
        case REG_GMCTRL7:
        case REG_GMCTRL8:
        case REG_GMCTRL9:
            mask = 0x0707;
            break;
            
        case REG_GMCTRL5:
        case REG_GMCTRL10:
            mask = 0x1F0F;
            break;

        case REG_WINH_ADDR_STA:
            mask = 0x00FF;
            if (m_lcdregs[REG_ENTRYMODE] & 0xA0)
                m_lcdregs[REG_CSRCOL] = val & mask;
            break;

        case REG_WINH_ADDR_END:
            mask = 0x00FF;
            if ((m_lcdregs[REG_ENTRYMODE] & 0xA0) == 0x80)
                m_lcdregs[REG_CSRCOL] = val & mask;
            break;

        case REG_WINV_ADDR_STA:
            mask = 0x01FF;
            if (m_lcdregs[REG_ENTRYMODE] & 0x90)
                m_lcdregs[REG_CSRROW] = val & mask;
            break;

        case REG_WINV_ADDR_END:
            mask = 0x01FF;
            if ((m_lcdregs[REG_ENTRYMODE] & 0x90) == 0x80)
                m_lcdregs[REG_CSRROW] = val & mask;
            break;

        case REG_GATESCANCTRL:
            mask = 0xBF3F;
            break;

        case REG_BIMGDISPCTRL:
            mask = 0x0007;
            break;

        case REG_VSCRCTRL:
        case REG_PARTIMG1DISPPOS:
        case REG_PARTIMG1STARTLN:
        case REG_PARTIMG1ENDLN:
        case REG_PARTIMG2DISPPOS:
        case REG_PARTIMG2STARTLN:
        case REG_PARTIMG2ENDLN:
            mask = 0x01FF;
            break;

        case REG_PANELINTCTRL1:
            mask = 0x031F;
            break;

        case REG_PANELINTCTRL2:
            mask = 0x0700;
            break;

        case REG_PANELINTCTRL4:
            mask = 0x0300;
            break;

        case REG_PANELINTCTRL5:
            mask = 0x0F00;
            break;

        case REG_OTP_VCM_PC:
            mask = 0x083F;
            break;

        case REG_OTP_VCM_SE:
            mask = 0xFF01;
            break;

        case REG_OTP_PID:
            mask = 0xFFFF;
            break;

        case REG_DEEPSBCTRL:
            mask = 0x0001;
            break;
    }

    val &= mask;
    m_lcdregs[m_currreg] = val;
}

bool ili9335_device::updateCursorReg(uint16_t *cursor, uint16_t start, uint16_t end, uint8_t flag) {
    bool next;

    if (m_lcdregs[REG_ENTRYMODE] & flag) {
        next = *cursor >= end;
        *cursor = *cursor < end ? *cursor + 1 : start;
    } else {
        next = *cursor <= start;
        *cursor = *cursor > start ? *cursor - 1 : end;
    }

    return next;
}

void ili9335_device::resetCursorRegs(uint16_t mode, bool resetX, bool resetY) {
    if (resetX)
        m_lcdregs[REG_CSRCOL] = (mode & 0x10) ? m_lcdregs[REG_WINH_ADDR_STA] : m_lcdregs[REG_WINH_ADDR_END];

    if (resetY)
        m_lcdregs[REG_CSRROW] = (mode & 0x20) ? m_lcdregs[REG_WINV_ADDR_STA] : m_lcdregs[REG_WINV_ADDR_END];
}

inline uint32_t ili9335_device::getGRAMData(uint32_t addr) {
    uint16_t hi = (m_gram[addr] << 8) | m_gram[addr + 1];
    uint16_t lo = (m_gram[addr + 2] << 8) | m_gram[addr + 3];
    return hi << 16 | lo;
}

// On the ILI9335, pixels are stored in a bitpacked RGB666 framebuffer that is 
// 172,800 bytes large. To fetch a pixel, the data fetched from the framebuffer
// needs to be shifted and masked. The framebuffer's size leaves no empty space, so
// shifting starts like this:
// pixel (0, 0), shift = 6 (<<)
// pixel (1, 0), shift = 4 (<<)
// pixel (2, 0), shift = 2 (<<)
// pixel (3, 0), shift = 0 (<<)
// pixel (4, 0), shift = 6 (<<)
// and so on...
// of course, this shifting assumes ints are 24 bits large. This driver uses 32, so the
// result is shifted left an extra 8 bits.

inline uint32_t ili9335_device::getPixel(uint16_t x, uint16_t y) {
    uint32_t addr = (y*m_width + x) * 18;
    uint8_t shift = 14 - (addr & 0x7);
    return (getGRAMData(addr / 8) >> shift) & 0x03FFFF;
}

inline uint16_t ili9335_device::rgb666_to_rgb565(uint32_t pixel) {
    uint16_t blue = (pixel & 0x3E) >> 1;
    uint16_t green = (pixel & 0xFC0) >> 1;
    uint16_t red = (pixel & 0x3E000) >> 2;
    return red | green | blue;
}

inline uint32_t ili9335_device::rgb666_swapBGR(uint32_t pixel) {
    uint8_t blue = (pixel & 0x3F);
    uint8_t green = (pixel & 0xFC0) >> 6;
    uint8_t red = (pixel & 0x3F000) >> 12;
    return red | (green << 6) | (blue << 12);
}

void ili9335_device::writePixel_565(uint16_t pixel) {
    uint8_t blue = (pixel << 1) & 0x3E;
    uint8_t green = (pixel >> 5) & 0x3F;
    uint8_t red = (pixel >> 10) & 0x3E;

    // on the ILI9335, a rgb565 color is expanded to rgb666 by using the 
    // high bits of the red and blue channels to set the extra lower bit
    red |= (pixel >> 15);
    blue |= ((pixel & 0x10) == 0x10);

    writePixel_666(blue | (green << 6) | (red << 12));
}

void ili9335_device::writePixel_666(uint32_t pixel) {
    if (m_lcdregs[REG_ENTRYMODE] & 0x1000)
        pixel = rgb666_swapBGR(pixel);

    uint16_t x = m_lcdregs[REG_CSRCOL];
    uint16_t y = m_lcdregs[REG_CSRROW];

    if (m_lcdregs[REG_DRVOUTCTRL] & 0x100)
        x = m_width - x - 1;

    uint32_t addr = ((y*m_width + x) * 18) / 8;
    uint8_t shift = 14 - (((y*m_width + x) * 18) % 8);
    uint32_t newPixel = (~(0x0003FFFF << shift) & getGRAMData(addr)) | (pixel << shift);

    for (int i = 0; i < 4; i++)
        m_gram[addr + (3 - i)] = (uint8_t)(newPixel >> 8*i);

    // "auto-increment" for the cursor row actually auto-decrements.
    if (m_lcdregs[REG_ENTRYMODE] & 0x08) {
        bool next = updateCursorReg(&m_lcdregs[REG_CSRROW], m_lcdregs[REG_WINV_ADDR_STA], m_lcdregs[REG_WINV_ADDR_END], 0x20);

        if (next)
            updateCursorReg(&m_lcdregs[REG_CSRCOL], m_lcdregs[REG_WINH_ADDR_STA], m_lcdregs[REG_WINH_ADDR_END], 0x10);
    } else {
        bool next = updateCursorReg(&m_lcdregs[REG_CSRCOL], m_lcdregs[REG_WINH_ADDR_STA], m_lcdregs[REG_WINH_ADDR_END], 0x10);

        if (next)
            updateCursorReg(&m_lcdregs[REG_CSRROW], m_lcdregs[REG_WINV_ADDR_STA], m_lcdregs[REG_WINV_ADDR_END], 0x20);
    }
}

void ili9335_device::writePixel_666_unpacked(uint32_t pixel) {
    uint8_t blue = (pixel >> 2) & 0x3F;
    uint8_t green = (pixel >> 10) & 0x3F;
    uint8_t red = (pixel >> 18) & 0x3F;
    writePixel_666(blue | (green << 6) | (red << 12));
}

uint32_t ili9335_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) {
    // clear screen
    rgb_t blank = m_lcdregs[REG_BIMGDISPCTRL] & 0x04 ? rgb_t::black() : rgb_t::white();

    // if the LCD is turned off, bail out after filling the bitmap with black pixels
    if (!(m_lcdregs[REG_DISPCTRL1] & 0x0002)) {
        bitmap.fill(rgb_t::black(), cliprect);
        return 0;
    }

    bitmap.fill(blank, cliprect);

    if (m_lcdregs[REG_DRVOUTCTRL] & 0x0400) {
        drawInterlacedFrame(bitmap);
    } else if (m_lcdregs[REG_DISPCTRL1] & 0x3000) {        
        // draw partial image 1
        if (m_lcdregs[REG_DISPCTRL1] & 0x1000)
            drawPartialImage(bitmap, m_lcdregs[REG_PARTIMG1DISPPOS], m_lcdregs[REG_PARTIMG1STARTLN], m_lcdregs[REG_PARTIMG1ENDLN]);
        
        // draw partial image 2
        if (m_lcdregs[REG_DISPCTRL1] & 0x2000)
            drawPartialImage(bitmap, m_lcdregs[REG_PARTIMG2DISPPOS], m_lcdregs[REG_PARTIMG2STARTLN], m_lcdregs[REG_PARTIMG2ENDLN]);
    } else if (m_lcdregs[REG_DISPCTRL1] & 0x0100) {
        // draw a normal frame
        int startLine = (m_lcdregs[REG_GATESCANCTRL] & 0x003F) * 8;
        int lineCount = (m_lcdregs[REG_GATESCANCTRL] & 0x3F00) >> 5;

        for (int y = startLine; y < startLine + lineCount + 8; y++)
            for (int x = 0; x < m_width; x++)
                plotPixel(bitmap, getPixel(x, y - startLine), x, y);
    }

    return 0;
}

void ili9335_device::drawInterlacedFrame(bitmap_rgb32 &bitmap) {
    uint16_t start1 = m_lcdregs[REG_PARTIMG1STARTLN];
    uint16_t end1 = m_lcdregs[REG_PARTIMG1ENDLN];
    uint16_t disp1 = m_lcdregs[REG_PARTIMG1DISPPOS];

    uint16_t start2 = m_lcdregs[REG_PARTIMG2STARTLN];
    uint16_t end2 = m_lcdregs[REG_PARTIMG2ENDLN];
    uint16_t disp2 = m_lcdregs[REG_PARTIMG2DISPPOS];

    // bail out if partial image regs were set up incorrectly.
    if (start1 >= end1 || start2 >= end2)
        return;

    if (m_lcdregs[REG_DISPCTRL1] & 0x1000) {
        for (int line = 0; line < m_height / 2; line++) {
            uint16_t y = start1 + line;
            uint16_t dispY = ((disp1 + line) * 2) % m_height;

            for (int x = 0; x < m_width; x++)
                plotPixel(bitmap, getPixel(x, y), x, dispY);
        }
    }

    if (m_lcdregs[REG_DISPCTRL1] & 0x2000) {
        for (int line = 0; line < m_height / 2; line++) {
            uint16_t y = start2 + line;
            uint16_t dispY = ((disp2 + line) * 2) % m_height;

            for (int x = 0; x < m_width; x++)
                plotPixel(bitmap, getPixel(x, y), x, dispY + 1);
        }
    }
}

void ili9335_device::drawPartialImage(bitmap_rgb32 &bitmap, uint16_t disp_pos, uint16_t start_line, uint16_t end_line) {
    if (start_line >= end_line)
        return;
    
    for (int line = 0; line <= end_line - start_line; line++) {
        uint16_t y = disp_pos + line;
        if (y >= m_height)
            return;

        for (int x = 0; x < m_width; x++)
            plotPixel(bitmap, getPixel(x, start_line + line), x, y);
    }
}

void ili9335_device::plotPixel(bitmap_rgb32 &bitmap, uint32_t pixel18, uint8_t x, uint16_t y) {
    // bail out if the coords are out of bounds or the LCD is off
    if (y >= m_height || x >= m_width)
        return;

    // flip Y if the gate scan direction bit isn't set
    if (!(m_lcdregs[REG_GATESCANCTRL] & 0x8000))
        y = m_height - y - 1;

    // wrap Y around the screen if partial image mode is disabled and scrolling is enabled
    if ((m_lcdregs[REG_BIMGDISPCTRL] & 0x0002) && !(m_lcdregs[REG_DISPCTRL1] & 0x3000))
        y = (y + m_height - (m_lcdregs[REG_VSCRCTRL] % m_height)) % m_height;

    // bail out if the screen y coord is out of bounds
    if (y >= m_height)
        return;

    uint8_t b6 = pixel18 & 0x3F;
    uint8_t g6 = (pixel18 >> 6) & 0x3F;
    uint8_t r6 = (pixel18 >> 12) & 0x3F;
    
    uint8_t b8, g8, r8;
    if (m_lcdregs[REG_DISPCTRL1] & 0x08) {
        // use eight-color mode if it's enabled
        b8 = b6 & 0x20 ? 0xFF : 0x00;
        g8 = g6 & 0x20 ? 0xFF : 0x00;
        r8 = r6 & 0x20 ? 0xFF : 0x00;
    } else {
        b8 = (b6 << 2) | (b6 >> 4);
        g8 = (g6 << 2) | (g6 >> 4);
        r8 = (r6 << 2) | (r6 >> 4);
    }

    // if the color inversion bit is set, invert the RGB value when drawing
    rgb_t pixel = m_lcdregs[REG_BIMGDISPCTRL] & 0x0001 ? rgb_t(b8, g8, r8) : rgb_t(~b8, ~g8, ~r8);
    bitmap.pix(y, x) = pixel;
}